/* elfp
 * ELF parser
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fault.h"
#include "util.h"

#include "elfp.h"

/* A 32-bit ELF header is at least 52 bytes long
 * Let's make that our cut-off point (even though it could be larger)
 */
#define SMALLEST_POSSIBLE_ELF 52

/* Small utility for warning users about invalid values */
#define WARN_INVALID(S, V, I)                                                  \
	WARN("ELF has unknown or invalid " S " %d\n", (V));                        \
	(V) = (I)

/* Small utility for validating values */
#define CHECK(V, M, S, I)                                                      \
	do {                                                                       \
		if( (V) > (M) ) {                                                      \
			WARN_INVALID(S, (V), (I));                                         \
		}                                                                      \
	} while( 0 )

/* Read 8-bit data */
#define READ8() utilRead8(fp)

/* Read 16-bit data, endianness dependent */
#define READ16() utilRead16(ident->endianness == ELF_ENDIAN_LITTLE_ENDIAN, fp)

/* Read 32-bit data, endianness dependent */
#define READ32() utilRead32(ident->endianness == ELF_ENDIAN_LITTLE_ENDIAN, fp)

/* Read 32/64-bit data, class and endianness dependent */
#define READ64()                                                               \
	ident->class == ELF_CLASS_32_BIT                                           \
		? utilRead32(ident->endianness == ELF_ENDIAN_LITTLE_ENDIAN, fp)        \
		: utilRead64(ident->endianness == ELF_ENDIAN_LITTLE_ENDIAN, fp)

static bool _parseEntryHeader(ELF_Header *header, FP *fp);
static bool _parseElfIdent(ELF_Ident *ident, FP *fp);

static void _parseNoteSection(void **data, uint64_t addr, FP *fp);

static bool _parseProgHeaders(ELF *elf, FP *fp);
static bool _parseProgHeaderEntry(ELF_PHEntry *ph, ELF_Ident *ident, FP *fp);

static bool _parseSectHeaders(ELF *elf, FP *fp);
static bool _parseSectHeaderEntry(ELF_SHEntry *sh, ELF_Ident *ident, FP *fp);

static void _parseSHEStringTable(ELF_SHEntry *sh, FP *fp);

static void _freePHEntry(ELF_PHEntry *ph);
static void _freeSHEntry(ELF_SHEntry *sh);

ELF *elfParseFile(const char *FILEPATH) {
	FP *fp = utilReadFile(FILEPATH);

	return elfParse(fp);
}

ELF *elfParse(FP *fp) {
	if( fp->size < SMALLEST_POSSIBLE_ELF ) {
		ERR("file too small: can't possibly be an ELF file\n");
		return NULL;
	}

	ELF *elf = malloc(sizeof(*elf));
	if( elf == NULL ) {
		FATAL("an error occurred while allocating memory\n");
	}

	if( !_parseEntryHeader(&elf->header, fp) ) {
		elfFree(elf);
		return NULL;
	}

	if( !_parseProgHeaders(elf, fp) ) {
		elfFree(elf);
		return NULL;
	}

	if( !_parseSectHeaders(elf, fp) ) {
		elfFree(elf);
		return NULL;
	}

	utilFreeFile(fp);
	return elf;
}

/* Parses the Entry Header */
static bool _parseEntryHeader(ELF_Header *header, FP *fp) {
	if( fp->data[0] != 0x7F || fp->data[1] != 'E' || fp->data[2] != 'L'
		|| fp->data[3] != 'F' ) {
		ERR("file is not a valid ELF binary (wrong magic)\n");
		return false;
	}

	fp->data += 4;

	if( !_parseElfIdent(&header->ident, fp) ) {
		return false;
	}

	ELF_Ident *ident = &header->ident;

	header->type = READ16();
	if( header->type > ELF_ET_CORE && header->type < ELF_ET_LOOS ) {
		WARN_INVALID("type", header->type, header->type);
	}

	header->machine = READ16();

	header->version = READ32();
	if( header->version != ELF_VERSION_CURRENT ) {
		WARN_INVALID("version", header->version, ELF_VERSION_INVALID);
	}

	header->entryPointAddress = READ64();
	header->progHeaderOffset = READ64();
	header->sectHeaderOffset = READ64();

	header->flags = READ32();

	header->headerSize = READ16();

	header->progHeaderEntrySize = READ16();
	header->progHeaderEntryNum = READ16();

	header->sectHeaderEntrySize = READ16();
	header->sectHeaderEntryNum = READ16();
	header->sectHeaderNameIndex = READ16();

	return true;
}

/* Parses the ident section of the Entry Header */
static bool _parseElfIdent(ELF_Ident *ident, FP *fp) {
	ident->class = READ8();
	CHECK(ident->class, ELF_CLASS_64_BIT, "class", ELF_CLASS_INVALID);

	ident->endianness = READ8();
	CHECK(ident->endianness, ELF_ENDIAN_BIG_ENDIAN, "endianness",
		ELF_ENDIAN_INVALID);

	ident->version = READ8();
	if( ident->version != ELF_VERSION_CURRENT ) {
		WARN_INVALID("version", ident->version, ELF_VERSION_INVALID);
	}

	ident->abi = READ8();
	ident->abiVersion = READ8();

	/* Skip over padding */
	fp->data += 7;

	return true;
}

/* Loads the contents of a note */
static void _parseNoteSection(void **data, uint64_t addr, FP *fp) {
	ELF_Note *note = malloc(sizeof(*note));

	memcpy(&note->namesz, fp->_start + addr, 4);
	memcpy(&note->descsz, fp->_start + addr + 4, 4);
	memcpy(&note->type, fp->_start + addr + 8, 4);

	note->name = malloc(note->namesz);
	memcpy(note->name, fp->_start + addr + 12, note->namesz);

	if( note->descsz > 0 ) {
		char *descAddr = (fp->_start + addr + 16);

		note->desc = malloc(note->descsz);
		memcpy(note->desc, descAddr, note->descsz);
	} else {
		note->desc = NULL;
	}

	*data = note;
}

/* Parses the Program Header */
static bool _parseProgHeaders(ELF *elf, FP *fp) {
	fp->data = fp->_start + elf->header.progHeaderOffset;
	elf->ph = malloc(sizeof(*elf->ph) * elf->header.progHeaderEntryNum);

	for( uint16_t i = 0; i < elf->header.progHeaderEntryNum; ++i ) {
		char *start = fp->data;

		if( !_parseProgHeaderEntry(&elf->ph[i], &elf->header.ident, fp) ) {
			return false;
		}

		fp->data = start + elf->header.progHeaderEntrySize;
	}

	return true;
}

/* Parses an entry in the Program Header */
static bool _parseProgHeaderEntry(ELF_PHEntry *ph, ELF_Ident *ident, FP *fp) {
	ph->type = READ32();

	if( ident->class == ELF_CLASS_64_BIT ) {
		ph->flags = READ32();
	}

	ph->offset = READ64();
	ph->virtualAddr = READ64();
	ph->physicalAddr = READ64();
	ph->fileSize = READ64();
	ph->memSize = READ64();

	if( ident->class == ELF_CLASS_32_BIT ) {
		ph->flags = READ32();
	}

	ph->align = READ32();

	switch( ph->type ) {
	case ELF_PHT_INTERP:
		ph->data = malloc(ph->fileSize);
		strncpy(ph->data, fp->_start + ph->virtualAddr, ph->fileSize);
		break;
	case ELF_PHT_NOTE:
		_parseNoteSection(&ph->data, ph->offset, fp);
		break;
	default:
		ph->data = NULL;
		break;
	}

	return true;
}

/* Parses the Section Header */
static bool _parseSectHeaders(ELF *elf, FP *fp) {
	fp->data = fp->_start + elf->header.sectHeaderOffset;
	elf->sh = malloc(sizeof(*elf->sh) * elf->header.sectHeaderEntryNum);

	for( uint16_t i = 0; i < elf->header.sectHeaderEntryNum; ++i ) {
		char *start = fp->data;

		if( !_parseSectHeaderEntry(&elf->sh[i], &elf->header.ident, fp) ) {
			return false;
		}

		fp->data = start + elf->header.sectHeaderEntrySize;
	}

	return true;
}

/* Parses an entry in the Section Header */
static bool _parseSectHeaderEntry(ELF_SHEntry *sh, ELF_Ident *ident, FP *fp) {
	sh->nameIdx = READ32();
	sh->type = READ32();

	sh->flags = READ64();
	sh->addr = READ64();
	sh->offset = READ64();
	sh->size = READ64();

	sh->link = READ32();
	sh->info = READ32();

	sh->addrAlign = READ64();
	sh->entrySize = READ64();

	switch( sh->type ) {
	case ELF_SHT_STRTAB:
		_parseSHEStringTable(sh, fp);
		break;
	case ELF_SHT_NOTE:
		_parseNoteSection(&sh->data, sh->offset, fp);
		break;
	default:
		sh->data = NULL;
	}

	return true;
}

/* Loads the contents of a string table */
static void _parseSHEStringTable(ELF_SHEntry *sh, FP *fp) {
	sh->data = malloc(sh->size);
	memcpy(sh->data, fp->_start + sh->offset, sh->size);
}

void elfFree(ELF *elf) {
	for( uint16_t i = 0; i < elf->header.progHeaderEntryNum; ++i ) {
		_freePHEntry(&elf->ph[i]);
	}

	for( uint16_t i = 0; i < elf->header.sectHeaderEntryNum; ++i ) {
		_freeSHEntry(&elf->sh[i]);
	}

	free(elf->ph);
	free(elf->sh);
	free(elf);
}

/* Frees a Program Header entry */
static void _freePHEntry(ELF_PHEntry *ph) {
	if( ph->data == NULL ) {
		return;
	}

	switch( ph->type ) {
	case ELF_PHT_NOTE: {
		ELF_Note *note = ph->data;
		free(note->name);
		free(note->desc);
	} break;
	default:
		break;
	}

	free(ph->data);
}

/* Frees a Section Header entry */
static void _freeSHEntry(ELF_SHEntry *sh) {
	if( sh->data == NULL ) {
		return;
	}

	switch( sh->type ) {
	case ELF_SHT_NOTE: {
		ELF_Note *note = sh->data;
		free(note->name);
		free(note->desc);
	} break;
	default:
		break;
	}

	free(sh->data);
}
