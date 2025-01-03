/* elfp
 * ELF information dump
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "elfp.h"

#include "elfdump.h"

#define PCASE(C, S)                                                            \
	case(C):                                                                   \
		printf(S);                                                             \
		break

#define PADS(s, S) printf("%-" s "s", S);
#define PADT(s, T, N) printf("%-" s T, N);

#define PPADCASE(C, s, S)                                                      \
	case(C):                                                                   \
		PADS(s, S);                                                            \
		break

#define PH_SEP                                                                 \
	"\n----------------------------------------------------------------------" \
	"------\n"
#define SH_SEP                                                                 \
	"\n--------------------------------------------------------------------\n"

#define PHT_PAD "14"
#define SHT_PAD "20"

static void _ehDump(ELF_Header *header);
static void _phDump(ELF_PHEntry *ph, ELF_Class class, uint16_t num);
static void _shDump(ELF_SHEntry *sh, int nidx, ELF_Class class, uint16_t num);

static void _elfVersionDump(ELF_Version version);
static void _elfAddrDump(ELF_Class class, uint64_t addr);

static void _elfNoteDescDump(ELF_Note *note);
static void _elfNoteDescABIDump(ELF_Note *note);
static void _elfNoteDescBuildIDDump(ELF_Note *note);

static void _ehIdentDump(ELF_Ident *ident);
static void _ehTypeDump(ELF_Type type);
static void _ehMachineDump(ELF_Machine machine);
static void _ehFlagsDump(uint32_t flags);

static void _eiClassDump(ELF_Class class);
static void _eiEndiannessDump(ELF_Endianness endianness);
static void _eiABIDump(ELF_ABI abi);

static void _pheDump(ELF_PHEntry *ph, ELF_Class class);
static void _pheTypeDump(ELF_PH_Type type);
static void _pheFlagsDump(uint32_t flags);

static void _sheDump(ELF_SHEntry *sh, ELF_Class class);
static void _sheTypeDump(ELF_SH_Type type);
static void _sheFlagsDumpUpper(uint64_t flags);
static void _sheFlagsDumpLower(uint64_t flags);

void elfDump(ELF *elf, int flags) {
	ELF_Class class = elf->header.ident.class;

	printf("=== ELF DUMP ===\n\n");

	if( flags & ELF_DUMP_EH ) {
		_ehDump(&elf->header);
		printf("\n");
	}

	if( flags & ELF_DUMP_PH ) {
		_phDump(elf->ph, class, elf->header.progHeaderEntryNum);
		printf("\n");
	}

	if( flags & ELF_DUMP_SH ) {
		_shDump(elf->sh, elf->header.sectHeaderNameIndex, class,
			elf->header.sectHeaderEntryNum);
		printf("\n");
	}
}

static void _ehDump(ELF_Header *header) {
	printf("* Header:\n");
	_ehIdentDump(&header->ident);

	printf("├── Type: ");
	_ehTypeDump(header->type);

	printf("├── Machine: ");
	_ehMachineDump(header->machine);

	printf("├── Version: ");
	_elfVersionDump(header->version);

	printf("├── Entry-point: ");
	_elfAddrDump(header->ident.class, header->entryPointAddress);

	printf("\n├── Program Header table start offset: ");
	printf("%" PRIu64 " bytes from start of file\n", header->progHeaderOffset);

	printf("├── Section Header table start offset: ");
	printf("%" PRIu64 " bytes from start of file\n", header->sectHeaderOffset);

	printf("├── Flags: ");
	_ehFlagsDump(header->flags);

	printf("├── Entry Header size: ");
	printf("%" PRIu16 " bytes\n", header->headerSize);

	printf("├── Size of a Program Header entry: ");
	printf("%" PRIu16 " bytes\n", header->progHeaderEntrySize);

	printf("├── Number of Program Header entries: ");
	printf("%" PRIu16 "\n", header->progHeaderEntryNum);

	printf("├── Size of a Section Header entry: ");
	printf("%" PRIu16 " bytes\n", header->sectHeaderEntrySize);

	printf("├── Number of a Section Header entry: ");
	printf("%" PRIu16 "\n", header->sectHeaderEntryNum);

	printf("└── Index of the Section Header entry with names: ");
	printf("%" PRIu16 "\n", header->sectHeaderNameIndex);
}

static void _ehIdentDump(ELF_Ident *ident) {
	printf("├── Ident:\n");

	printf("├──── Class: ");
	_eiClassDump(ident->class);

	printf("├──── Endianness: ");
	_eiEndiannessDump(ident->endianness);

	printf("├──── Version: ");
	_elfVersionDump(ident->version);

	printf("├──── ABI: ");
	_eiABIDump(ident->abi);

	printf("├──── ABI Version: ");
	printf("%" PRIu8 "\n│\n", ident->abiVersion);
}

static void _eiClassDump(ELF_Class class) {
	switch( class ) {
		PCASE(ELF_CLASS_INVALID, "invalid\n");
		PCASE(ELF_CLASS_32_BIT, "32-bit\n");
		PCASE(ELF_CLASS_64_BIT, "64-bit\n");
	default:
		printf("Unknown class '%d'\n", class);
	}
}

static void _eiEndiannessDump(ELF_Endianness endianness) {
	switch( endianness ) {
		PCASE(ELF_ENDIAN_INVALID, "invalid\n");
		PCASE(ELF_ENDIAN_LITTLE_ENDIAN, "Little-endian\n");
		PCASE(ELF_ENDIAN_BIG_ENDIAN, "Big-endian\n");
	default:
		printf("Unknown endianness '%d'\n", endianness);
	}
}

static void _elfVersionDump(ELF_Version version) {
	printf(version == ELF_VERSION_CURRENT ? "1 (current)\n" : "invalid\n");
}

static void _elfAddrDump(ELF_Class class, uint64_t addr) {
	if( class == ELF_CLASS_32_BIT ) {
		printf("0x%08" PRIx32, (uint32_t)addr);
	} else {
		/* Assume 64-bit, even if class is invalid */
		printf("0x%016" PRIx64, addr);
	}
}

static void _elfNoteDescDump(ELF_Note *note) {
	if( strcmp(note->name, "GNU") == 0 ) {
		switch( note->type ) {
		case ELF_NT_GNU_ABI:
			_elfNoteDescABIDump(note);
			break;
		case ELF_NT_GNU_BUILDID:
			_elfNoteDescBuildIDDump(note);
			break;
		default:
			printf("Unknown GNU note type '%d'", note->type);
		}
	} else {
		printf("Unknown");
	}
}

static void _elfNoteDescABIDump(ELF_Note *note) {
	printf("Expects ");

	uint32_t *data = (uint32_t *)note->desc;

	const uint32_t OS = data[0];
	const uint32_t MAJOR = data[1];
	const uint32_t MINOR = data[2];
	const uint32_t PATCH = data[3];

	switch( OS ) {
		PCASE(ELF_NT_GNU_ABI_LINUX, "Linux");
		PCASE(ELF_NT_GNU_ABI_HURD, "GNU Hurd");
		PCASE(ELF_NT_GNU_ABI_SOLARIS, "Solaris");
		PCASE(ELF_NT_GNU_ABI_FREEBSD, "FreeBSD");
		PCASE(ELF_NT_GNU_ABI_SYLLABLE, "Syllable");
		PCASE(ELF_NT_GNU_ABI_NACL, "NaCl");
	default:
		printf("unknown OS '%d'", OS);
	}

	printf(", ABI v%" PRIu32 ".%" PRIu32 ".%" PRIu32, MAJOR, MINOR, PATCH);
}

static void _elfNoteDescBuildIDDump(ELF_Note *note) {
	printf("Build ID: ");
	for( uint32_t i = 0; i < note->descsz; ++i ) {
		printf("%02x", (unsigned char)note->desc[i]);
	}
}

static void _eiABIDump(ELF_ABI abi) {
	switch( abi ) {
		PCASE(ELF_ABI_SYSTEM_V, "Unix System V\n");
		PCASE(ELF_ABI_HP_UX, "HP-UX\n");
		PCASE(ELF_ABI_NETBSD, "NetBSD\n");
		PCASE(ELF_ABI_LINUX, "Linux\n");
		PCASE(ELF_ABI_GNU_HURD, "GNU Hurd\n");
		PCASE(ELF_ABI_SOLARIS, "Oracle Solaris\n");
		PCASE(ELF_ABI_AIX, "AIX\n");
		PCASE(ELF_ABI_IRIX, "IRIX\n");
		PCASE(ELF_ABI_FREEBSD, "FreeBSD\n");
		PCASE(ELF_ABI_TRU64, "Tru64 UNIX\n");
		PCASE(ELF_ABI_MODESTO, "Novell Modesto\n");
		PCASE(ELF_ABI_OPENBSD, "OpenBSD\n");
		PCASE(ELF_ABI_OPENVMS, "OpenVMS\n");
		PCASE(ELF_ABI_NONSTOP, "NonStop Kernel\n");
		PCASE(ELF_ABI_AROS, "AROS Research Operating System\n");
		PCASE(ELF_ABI_FENIXOS, "FenixOS\n");
		PCASE(ELF_ABI_CLOUD, "Nuxi CloudABI\n");
		PCASE(ELF_ABI_OPENVOS, "Stratus Technologies OpenVOS\n");
		PCASE(ELF_ABI_ARM_AEABI, "ARM AEABI\n");
		PCASE(ELF_ABI_ARM, "ARM\n");
		PCASE(ELF_ABI_STANDALONE, "Standalone (embedded)\n");
	default:
		printf("Unknown ABI '%d'\n", abi);
	}
}

static void _ehTypeDump(ELF_Type type) {
	switch( type ) {
		PCASE(ELF_ET_NONE, "None\n");
		PCASE(ELF_ET_RELOCATABLE, "Relocatable\n");
		PCASE(ELF_ET_EXECUTABLE, "Executable\n");
		PCASE(ELF_ET_DYNAMIC, "Dynamic (shared)\n");
		PCASE(ELF_ET_CORE, "Core\n");
	default:
		if( type >= ELF_ET_LOOS && type <= ELF_ET_HIOS ) {
			printf("OS specific\n");
		} else if( type >= ELF_ET_LOPROC && type <= ELF_ET_HIPROC ) {
			printf("Processor specific\n");
		} else {
			printf("Unknown type '%d'\n", type);
		}
	}
}

static void _ehMachineDump(ELF_Machine machine) {
	switch( machine ) {
		PCASE(ELF_EM_NONE, "No machine specified\n");
		PCASE(ELF_EM_WE32100, "AT&T WE 32100\n");
		PCASE(ELF_EM_SPARC, "SPARC\n");
		PCASE(ELF_EM_I386, "Intel 386\n");
		PCASE(ELF_EM_M68K, "Motorola 68K\n");
		PCASE(ELF_EM_M88K, "Motorola 88K\n");
		PCASE(ELF_EM_IMCU, "Intel MCU\n");
		PCASE(ELF_EM_I860, "Intel 80860\n");
		PCASE(ELF_EM_MIPS, "MIPS\n");
		PCASE(ELF_EM_S370, "IBM System/370\n");
		PCASE(ELF_EM_MIPS_RS3000_LE, "MIPS RS3000 Little-endian\n");
		PCASE(ELF_EM_PARISC, "Hewlett-Packard PA-RISC\n");
		PCASE(ELF_EM_VPP500, "Fujitsu VPP500\n");
		PCASE(ELF_EM_V8PLUS, "SPARC V8+\n");
		PCASE(ELF_EM_I960, "Intel 80960\n");
		PCASE(ELF_EM_POWERPC, "PowerPC\n");
		PCASE(ELF_EM_POWERPC64, "PowerPC (64-bit)\n");
		PCASE(ELF_EM_S390, "IBM S390/S390x\n");
		PCASE(ELF_EM_SPU, "IBM SPU/SPC\n");
		PCASE(ELF_EM_V800, "NEC V800\n");
		PCASE(ELF_EM_FR20, "Fujitsu FR20\n");
		PCASE(ELF_EM_RH32, "TRW RH-32\n");
		PCASE(ELF_EM_RCE, "Motorola RCE\n");
		PCASE(ELF_EM_ARM, "ARM\n");
		PCASE(ELF_EM_ALPHA, "Digital Alpha\n");
		PCASE(ELF_EM_SUPERH, "Hitachi SuperH\n");
		PCASE(ELF_EM_SPARCV9, "SPARC V9\n");
		PCASE(ELF_EM_TRICORE, "Siemens TriCore\n");
		PCASE(ELF_EM_ARC, "Argonaut RISC Core\n");
		PCASE(ELF_EM_H8_300, "Hitachi H8/300\n");
		PCASE(ELF_EM_H8_300H, "Hitachi H8/300H\n");
		PCASE(ELF_EM_H8S, "Hitachi H8S\n");
		PCASE(ELF_EM_H8_500, "Hitachi H8/500\n");
		PCASE(ELF_EM_IA_64, "Itanium IA-64\n");
		PCASE(ELF_EM_MIPS_X, "Stanford MIPS-X\n");
		PCASE(ELF_EM_COLDFIRE, "Motorola ColdFire\n");
		PCASE(ELF_EM_M68HC12, "Motorola 68HC12\n");
		PCASE(ELF_EM_MMA, "Fujitsu MMA Multimedia Accelerator\n");
		PCASE(ELF_EM_PCP, "Siemens PCP\n");
		PCASE(ELF_EM_NCPU, "Sony nCPU\n");
		PCASE(ELF_EM_NDR1, "Denso NDR1\n");
		PCASE(ELF_EM_STARCORE, "Motorola Star*Core\n");
		PCASE(ELF_EM_ME16, "Toyota ME16\n");
		PCASE(ELF_EM_ST100, "STMicroelectronics ST100\n");
		PCASE(ELF_EM_TINYJ, "Advanced Logic Corp. TinyJ\n");
		PCASE(ELF_EM_X86_64, "AMD x86-64\n");
		PCASE(ELF_EM_DSP, "Sony DSP\n");
		PCASE(ELF_EM_PDP10, "Digital Equipment Corp. PDP-10\n");
		PCASE(ELF_EM_PDP11, "Digital Equipment Corp. PDP-11\n");
		PCASE(ELF_EM_FX66, "Siemens FX66\n");
		PCASE(ELF_EM_ST9PLUS, "STMicroelectronics ST9+ 8/16-bit\n");
		PCASE(ELF_EM_ST7, "STMicroelectronics ST7 8-bit\n");
		PCASE(ELF_EM_M68HC16, "Motorola M68CH16\n");
		PCASE(ELF_EM_M68HC11, "Motorola M68CH11\n");
		PCASE(ELF_EM_M68HC08, "Motorola M68CH08\n");
		PCASE(ELF_EM_M68HC05, "Motorola M68CH05\n");
		PCASE(ELF_EM_SVX, "Silicon Graphics SVx\n");
		PCASE(ELF_EM_ST19, "STMicroelectronics ST19 8-bit\n");
		PCASE(ELF_EM_VAX, "Digital Equipment Corp. VAX\n");
	default:
		printf("Unknown machine %d\n", machine);
	}
}

static void _ehFlagsDump(uint32_t flags) {
	/* TODO: Do boring flag cross-referencing... */
	printf("%" PRIu32 "\n", flags);
}

static void _phDump(ELF_PHEntry *ph, ELF_Class class, uint16_t num) {
	printf("* Program Header entries\n");
	printf("No.   Type          Offset             Virtual addr.      Physical "
		   "addr.\n");
	printf("                    File size          Memory size        Flags "
		   "Align");
	printf(PH_SEP);

	for( uint16_t i = 0; i < num; ++i ) {
		printf("%-5" PRIu16 " ", i);
		_pheDump(&ph[i], class);
	}
}

static void _pheDump(ELF_PHEntry *ph, ELF_Class class) {
	_pheTypeDump(ph->type);

	_elfAddrDump(class, ph->offset);
	printf(" ");

	_elfAddrDump(class, ph->virtualAddr);
	printf(" ");

	_elfAddrDump(class, ph->physicalAddr);
	printf("\n                    ");

	printf("%-18" PRIu64 " ", ph->fileSize);
	printf("%-18" PRIu64 " ", ph->memSize);

	_pheFlagsDump(ph->flags);
	printf("   0x%-10" PRIx64, ph->align);

	if( ph->type == ELF_PHT_INTERP ) {
		printf(" (requests interpreter %s)", (char *)ph->data);
	}

	if( ph->type == ELF_PHT_NOTE ) {
		ELF_Note *note = ph->data;
		printf(" Note (%s): ", note->name);
		_elfNoteDescDump(note);
	}

	printf(PH_SEP);
}

static void _pheTypeDump(ELF_PH_Type type) {
	switch( type ) {
		PPADCASE(ELF_PHT_NULL, PHT_PAD, "Unused");
		PPADCASE(ELF_PHT_LOAD, PHT_PAD, "Loadable");
		PPADCASE(ELF_PHT_DYNAMIC, PHT_PAD, "Dynamic");
		PPADCASE(ELF_PHT_INTERP, PHT_PAD, "Interpreter");
		PPADCASE(ELF_PHT_NOTE, PHT_PAD, "Note");
		PPADCASE(ELF_PHT_SHLIB, PHT_PAD, "Reserved");
		PPADCASE(ELF_PHT_PHDR, PHT_PAD, "Prog Header");
		PPADCASE(ELF_PHT_TLS, PHT_PAD, "TLS");
		PPADCASE(ELF_PHT_GNU_EH_FRAME, PHT_PAD, "GNU EH Frame");
		PPADCASE(ELF_PHT_GNU_STACK, PHT_PAD, "GNU Stack");
		PPADCASE(ELF_PHT_GNU_RELRO, PHT_PAD, "GNU Read-only");
		PPADCASE(ELF_PHT_GNU_PROPERTY, PHT_PAD, "GNU Property");
		PPADCASE(ELF_PHT_GNU_SFRAME, PHT_PAD, "GNU Stackframe");
		PPADCASE(ELF_PHT_SUNBSS, PHT_PAD, "Sun BSS");
		PPADCASE(ELF_PHT_SUNSTACK, PHT_PAD, "Sun Stack");
	default:
		if( type >= ELF_PHT_LOOS && type <= ELF_PHT_HIOS ) {
			PADS(PHT_PAD, "OS");
		} else if( type >= ELF_PHT_LOPROC && type <= ELF_PHT_HIPROC ) {
			PADS(PHT_PAD, "Processor");
		} else {
			printf("Unknown %" PRIu32 " ", type);
		}
	}
}

static void _pheFlagsDump(uint32_t flags) {
	printf("%c", (flags & ELF_PHF_R) ? 'R' : ' ');
	printf("%c", (flags & ELF_PHF_W) ? 'W' : ' ');
	printf("%c", (flags & ELF_PHF_X) ? 'X' : ' ');
}

static void _shDump(ELF_SHEntry *sh, int nidx, ELF_Class class, uint16_t num) {
	printf("* Section Header entries\n");
	printf("No.   Name             Type                Flags1 Offset\n");
	printf("      Entry Size       Link Info Align     Flags2 Address");
	printf(SH_SEP);

	char *strtab = sh[nidx].data;

	for( uint16_t i = 0; i < num; ++i ) {
		printf("%-5" PRIu16 " ", i);

		ELF_SHEntry *she = &sh[i];
		char *str = &strtab[she->nameIdx];
		if( *str == '\0' ) {
			printf("No name          ");
		} else if( strlen(str) > 13 ) {
			printf("%.*s... ", 13, str);
		} else {
			printf("%-17s", str);
		}

		_sheDump(she, class);
	}

	printf("Flags key:\n");
	printf(
		"W: Write    S: Strings           G: Section group o: OS-specific\n");
	printf("A: Allocate I: Info link         T: TLS           p: "
		   "Processor-specific\n");
	printf("X: Execute  L: Link order        O: Ordered\n");
	printf("M: Merge    N: OS non-conforming E: Exclude\n");
}

static void _sheDump(ELF_SHEntry *sh, ELF_Class class) {
	_sheTypeDump(sh->type);
	_sheFlagsDumpUpper(sh->flags);
	_elfAddrDump(class, sh->offset);

	printf("\n      ");

	printf("%016" PRIu64 " ", sh->entrySize);
	printf("%04" PRIu32 " ", sh->link);
	printf("%04" PRIu32 " ", sh->info);
	printf("%08" PRIu64 "  ", sh->addrAlign);

	_sheFlagsDumpLower(sh->flags);
	_elfAddrDump(class, sh->addr);

	if( sh->type == ELF_SHT_NOTE ) {
		ELF_Note *note = sh->data;
		printf(" Note (%s): ", note->name);
		_elfNoteDescDump(note);
	}

	printf(SH_SEP);
}

static void _sheTypeDump(ELF_SH_Type type) {
	switch( type ) {
		PPADCASE(ELF_SHT_NULL, SHT_PAD, "NULL");
		PPADCASE(ELF_SHT_PROGBITS, SHT_PAD, "Program data");
		PPADCASE(ELF_SHT_SYMTAB, SHT_PAD, "Symbol table");
		PPADCASE(ELF_SHT_STRTAB, SHT_PAD, "String table");
		PPADCASE(ELF_SHT_RELOC_A, SHT_PAD, "Reloc (addends)");
		PPADCASE(ELF_SHT_HASH, SHT_PAD, "Symbol hash table");
		PPADCASE(ELF_SHT_DYNAMIC, SHT_PAD, "Dynlink info");
		PPADCASE(ELF_SHT_NOTE, SHT_PAD, "Notes");
		PPADCASE(ELF_SHT_NOBITS, SHT_PAD, "BSS");
		PPADCASE(ELF_SHT_RELOC, SHT_PAD, "Reloc (no addends)");
		PPADCASE(ELF_SHT_SHLIB, SHT_PAD, "Reserved");
		PPADCASE(ELF_SHT_DYNSYM, SHT_PAD, "Dyn linker symbols");
		PPADCASE(ELF_SHT_INIT_ARRAY, SHT_PAD, "Constructors");
		PPADCASE(ELF_SHT_FINI_ARRAY, SHT_PAD, "Destructors");
		PPADCASE(ELF_SHT_PREINIT_ARRAY, SHT_PAD, "Pre-constructors");
		PPADCASE(ELF_SHT_GROUP, SHT_PAD, "Section group");
		PPADCASE(ELF_SHT_SYMTAB_EXT, SHT_PAD, "Ext section indices");
		PPADCASE(ELF_SHT_RELR, SHT_PAD, "RELR");
	default:
		if( type >= ELF_SHT_LOOS && type <= ELF_SHT_HIOS ) {
			PADS(SHT_PAD, "OS");
		} else if( type >= ELF_SHT_LOPROC && type <= ELF_SHT_HIPROC ) {
			PADS(SHT_PAD, "Processor");
		} else if( type >= ELF_SHT_LOUSER && type <= ELF_SHT_HIUSER ) {
			PADS(SHT_PAD, "User");
		} else {
			printf("Unknown %" PRIu32 " ", type);
		}
	}
}

static void _sheFlagsDumpUpper(uint64_t flags) {
	printf("%c", (flags & ELF_SHF_WRITE) ? 'W' : ' ');
	printf("%c", (flags & ELF_SHF_ALLOC) ? 'A' : ' ');
	printf("%c", (flags & ELF_SHF_EXEC) ? 'X' : ' ');
	printf("%c", (flags & ELF_SHF_MERGE) ? 'M' : ' ');
	printf("%c", (flags & ELF_SHF_STRINGS) ? 'S' : ' ');
	printf("%c", (flags & ELF_SHF_INFO) ? 'I' : ' ');
	printf("%c", (flags & ELF_SHF_LINK_ORDER) ? 'L' : ' ');
}

static void _sheFlagsDumpLower(uint64_t flags) {
	printf("%c", (flags & ELF_SHF_OS_NONCONFORMING) ? 'N' : ' ');
	printf("%c", (flags & ELF_SHF_GROUP) ? 'G' : ' ');
	printf("%c", (flags & ELF_SHF_TLS) ? 'T' : ' ');
	printf("%c", (flags & ELF_SHF_ORDERERD) ? 'O' : ' ');
	printf("%c", (flags & ELF_SHF_EXCLUDE) ? 'E' : ' ');
	printf("%c", (flags & ELF_SHF_OS) ? 'o' : ' ');
	printf("%c", (flags & ELF_SHF_PROC) ? 'p' : ' ');
}
