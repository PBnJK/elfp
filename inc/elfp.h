#ifndef GUARD_ELFP_ELFP_H_
#define GUARD_ELFP_ELFP_H_

/* ELF format data structures
 *
 * I am aware of the /usr/include/elf.h header, which already specifies almost
 * all of the values used here. I just wanted to do it from scratch :-)
 *
 * Sources:
 * - Wikipedia..... https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
 * - ELF manpage... https://man7.org/linux/man-pages/man5/elf.5.html
 * - ELF spec...... http://www.skyfree.org/linux/references/ELF_Format.pdf
 * - ELF header.... /usr/include/elf.h
 * - binutils...... https://github.com/bminor/binutils-gdb/
 *
 * binutils specifically was *really* helpful for those obscure, seemingly
 * undocumented GNU note section types
 */

#include <stddef.h>
#include <stdint.h>

#include "util.h"

/* Enumeration of all possible EI_CLASS values
 * Represents whether the format is 32-bit or 64-bit
 */
typedef enum _ELF_Class {
	ELF_CLASS_INVALID = 0,
	ELF_CLASS_32_BIT = 1,
	ELF_CLASS_64_BIT = 2,
} ELF_Class;

/* Enumeration of all possible EI_DATA values
 * Represents whether the format is little or big endian
 */
typedef enum _ELF_Endianness {
	ELF_ENDIAN_INVALID = 0,
	ELF_ENDIAN_LITTLE_ENDIAN = 1,
	ELF_ENDIAN_BIG_ENDIAN = 2,
} ELF_Endianness;

/* Enumeration of all possible EI_VERSION values
 * Represents the ELF version. Always 1
 */
typedef enum _ELF_Version {
	ELF_VERSION_INVALID = 0,
	ELF_VERSION_CURRENT = 1,
} ELF_Version;

/* Enumeration of all possible EI_OSABI values
 * Represents the target OS ABI (Application Binary Interface)
 */
typedef enum _ELF_ABI {
	ELF_ABI_SYSTEM_V = 0,
	ELF_ABI_HP_UX = 1,
	ELF_ABI_NETBSD = 2,
	ELF_ABI_LINUX = 3,
	ELF_ABI_GNU_HURD = 4,
	ELF_ABI_SOLARIS = 6,
	ELF_ABI_AIX = 7,
	ELF_ABI_IRIX = 8,
	ELF_ABI_FREEBSD = 9,
	ELF_ABI_TRU64 = 10,
	ELF_ABI_MODESTO = 11,
	ELF_ABI_OPENBSD = 12,
	ELF_ABI_OPENVMS = 13,
	ELF_ABI_NONSTOP = 14,
	ELF_ABI_AROS = 15,
	ELF_ABI_FENIXOS = 16,
	ELF_ABI_CLOUD = 17,
	ELF_ABI_OPENVOS = 18,
	ELF_ABI_ARM_AEABI = 64,
	ELF_ABI_ARM = 97,
	ELF_ABI_STANDALONE = 255,
} ELF_ABI;

/* Structure representing the e_ident field
 * Serves as a means of identifying the ELF file
 */
typedef struct _ELF_Ident {
	ELF_Class class;
	ELF_Endianness endianness;
	ELF_Version version; /* Always 1 */
	ELF_ABI abi;

	char abiVersion; /* Further specifies the ABI version */
} ELF_Ident;

/* Enumeration of all possible e_type values
 * Represents the object file type
 */
typedef enum _ELF_Type {
	ELF_ET_NONE = 0,
	ELF_ET_RELOCATABLE = 1,
	ELF_ET_EXECUTABLE = 2,
	ELF_ET_DYNAMIC = 3,
	ELF_ET_CORE = 4,
	ELF_ET_LOOS = 0xFE00,
	ELF_ET_HIOS = 0xFEFF,
	ELF_ET_LOPROC = 0xFF00,
	ELF_ET_HIPROC = 0xFFFF,
} ELF_Type;

/* Enumeration of all possible e_machine values
 * Represents the machine this file is designed to run on (ISA)
 *
 * TODO: Complete the list
 */
typedef enum _ELF_Machine {
	ELF_EM_NONE = 0,
	ELF_EM_WE32100 = 1,
	ELF_EM_SPARC = 2,
	ELF_EM_I386 = 3,
	ELF_EM_M68K = 4,
	ELF_EM_M88K = 5,
	ELF_EM_IMCU = 6,
	ELF_EM_I860 = 7,
	ELF_EM_MIPS = 8,
	ELF_EM_S370 = 9,
	ELF_EM_MIPS_RS3000_LE = 10,
	ELF_EM_PARISC = 15,
	ELF_EM_VPP500 = 17,
	ELF_EM_V8PLUS = 18,
	ELF_EM_I960 = 19,
	ELF_EM_POWERPC = 20,
	ELF_EM_POWERPC64 = 21,
	ELF_EM_S390 = 22,
	ELF_EM_SPU = 23,
	ELF_EM_V800 = 36,
	ELF_EM_FR20 = 37,
	ELF_EM_RH32 = 38,
	ELF_EM_RCE = 39,
	ELF_EM_ARM = 40,
	ELF_EM_ALPHA = 41,
	ELF_EM_SUPERH = 42,
	ELF_EM_SPARCV9 = 43,
	ELF_EM_TRICORE = 44,
	ELF_EM_ARC = 45,
	ELF_EM_H8_300 = 46,
	ELF_EM_H8_300H = 47,
	ELF_EM_H8S = 48,
	ELF_EM_H8_500 = 49,
	ELF_EM_IA_64 = 50,
	ELF_EM_MIPS_X = 51,
	ELF_EM_COLDFIRE = 52,
	ELF_EM_M68HC12 = 53,
	ELF_EM_MMA = 54,
	ELF_EM_PCP = 55,
	ELF_EM_NCPU = 56,
	ELF_EM_NDR1 = 57,
	ELF_EM_STARCORE = 58,
	ELF_EM_ME16 = 59,
	ELF_EM_ST100 = 60,
	ELF_EM_TINYJ = 61,
	ELF_EM_X86_64 = 62,
	ELF_EM_DSP = 63,
	ELF_EM_PDP10 = 64,
	ELF_EM_PDP11 = 65,
	ELF_EM_FX66 = 66,
	ELF_EM_ST9PLUS = 67,
	ELF_EM_ST7 = 68,
	ELF_EM_M68HC16 = 69,
	ELF_EM_M68HC11 = 70,
	ELF_EM_M68HC08 = 71,
	ELF_EM_M68HC05 = 72,
	ELF_EM_SVX = 73,
	ELF_EM_ST19 = 74,
	ELF_EM_VAX = 75,
} ELF_Machine;

/* Structure representing the ELF Entry Header */
typedef struct _ELF_Header {
	ELF_Ident ident;
	ELF_Type type;
	ELF_Machine machine;
	ELF_Version version; /* Always 1 */

	uint64_t entryPointAddress; /* Entry-point of the process */
	uint64_t progHeaderOffset; /* Program header offset */
	uint64_t sectHeaderOffset; /* Section header offset */

	uint32_t flags; /* Target architecture dependent field */

	uint16_t headerSize; /* Size of this header */

	uint16_t progHeaderEntrySize; /* Size of an entry in the program header */
	uint16_t progHeaderEntryNum; /* Number of program header entries */
	uint16_t sectHeaderEntrySize; /* Size of an entry in the section header */
	uint16_t sectHeaderEntryNum; /* Number of section header entries */
	uint16_t sectHeaderNameIndex; /* Index of the section names entry on the
									 section header */
} ELF_Header;

/* Enumeration of all possible p_type values
 * Represents the Program Header's type
 */
typedef enum _ELF_PH_Type {
	ELF_PHT_NULL = 0,
	ELF_PHT_LOAD = 1,
	ELF_PHT_DYNAMIC = 2,
	ELF_PHT_INTERP = 3,
	ELF_PHT_NOTE = 4,
	ELF_PHT_SHLIB = 5,
	ELF_PHT_PHDR = 6,
	ELF_PHT_TLS = 7,
	ELF_PHT_LOOS = 0x60000000,
	ELF_PHT_GNU_EH_FRAME = 0x6474E550,
	ELF_PHT_GNU_STACK = 0x6474E551,
	ELF_PHT_GNU_RELRO = 0x6474E552,
	ELF_PHT_GNU_PROPERTY = 0x6474E553,
	ELF_PHT_GNU_SFRAME = 0x6474E554,
	ELF_PHT_SUNBSS = 0x6FFFFFA,
	ELF_PHT_SUNSTACK = 0x6FFFFFB,
	ELF_PHT_HIOS = 0x6FFFFFFF,
	ELF_PHT_LOPROC = 0x70000000,
	ELF_PHT_HIPROC = 0x7FFFFFFF,
} ELF_PH_Type;

typedef enum _ELF_NT {
	ELF_NT_GNU_ABI = 1,
	ELF_NT_GNU_HWCAP = 2,
	ELF_NT_GNU_BUILDID = 3
} ELF_NT;

typedef enum _ELF_NT_GNU_ABI_Type {
	ELF_NT_GNU_ABI_LINUX = 0,
	ELF_NT_GNU_ABI_HURD = 1,
	ELF_NT_GNU_ABI_SOLARIS = 2,
	ELF_NT_GNU_ABI_FREEBSD = 3,
	ELF_NT_GNU_ABI_NETBSD = 4,
	ELF_NT_GNU_ABI_SYLLABLE = 5,
	ELF_NT_GNU_ABI_NACL = 6,
} ELF_NT_GNU_ABI_Type;

typedef struct _ELF_Note {
	uint32_t namesz;
	uint32_t descsz;
	uint32_t type;
	char *name;
	char *desc;
} ELF_Note;

/* p_flags values
 * Represents a Program Header entry's flags
 */
#define ELF_PHF_X 1
#define ELF_PHF_W 2
#define ELF_PHF_R 4

/* Structure representing an entry in the ELF Program Header */
typedef struct _ELF_PHEntry {
	ELF_PH_Type type;
	uint32_t flags;

	uint64_t offset;
	uint64_t virtualAddr;
	uint64_t physicalAddr;
	uint64_t fileSize;
	uint64_t memSize;
	uint64_t align;

	void *data;
} ELF_PHEntry;

typedef enum _ELF_SH_Type {
	ELF_SHT_NULL = 0,
	ELF_SHT_PROGBITS = 1,
	ELF_SHT_SYMTAB = 2,
	ELF_SHT_STRTAB = 3,
	ELF_SHT_RELOC_A = 4,
	ELF_SHT_HASH = 5,
	ELF_SHT_DYNAMIC = 6,
	ELF_SHT_NOTE = 7,
	ELF_SHT_NOBITS = 8,
	ELF_SHT_RELOC = 9,
	ELF_SHT_SHLIB = 10,
	ELF_SHT_DYNSYM = 11,
	ELF_SHT_INIT_ARRAY = 14,
	ELF_SHT_FINI_ARRAY = 15,
	ELF_SHT_PREINIT_ARRAY = 16,
	ELF_SHT_GROUP = 17,
	ELF_SHT_SYMTAB_EXT = 18,
	ELF_SHT_RELR = 19,

	ELF_SHT_LOOS = 0x60000000,
	ELF_SHT_HIOS = 0x6FFFFFFF,

	ELF_SHT_LOPROC = 0x70000000,
	ELF_SHT_HIPROC = 0x7FFFFFFF,
} ELF_SH_Type;

/* Also part of ELF_SH_Type, but C doesn't allow enum mebers larger than the
 * int type...
 */
#define ELF_SHT_LOUSER 0x80000000
#define ELF_SHT_HIUSER 0x8FFFFFFF

/* s_flags values
 * Represents a Section Header entry's flags
 */
#define ELF_SHF_WRITE 1
#define ELF_SHF_ALLOC 2
#define ELF_SHF_EXEC 4
#define ELF_SHF_MERGE 0x10
#define ELF_SHF_STRINGS 0x20
#define ELF_SHF_INFO 0x40
#define ELF_SHF_LINK_ORDER 0x80
#define ELF_SHF_OS_NONCONFORMING 0x100
#define ELF_SHF_GROUP 0x200
#define ELF_SHF_TLS 0x400
#define ELF_SHF_ORDERERD 0x4000000
#define ELF_SHF_EXCLUDE 0x8000000
#define ELF_SHF_OS 0x0FF00000
#define ELF_SHF_PROC 0xF0000000

/* Structure representing an entry in the ELF Section Header */
typedef struct _ELF_SHEntry {
	uint32_t nameIdx;

	ELF_SH_Type type;
	uint64_t flags;

	uint64_t addr;
	uint64_t offset;
	uint64_t size;

	uint32_t link;
	uint32_t info;

	uint64_t addrAlign;
	uint64_t entrySize;

	void *data;
} ELF_SHEntry;

/* Structure representing an ELF file */
typedef struct _ELF {
	ELF_Header header;
	ELF_PHEntry *ph;
	ELF_SHEntry *sh;
} ELF;

/* Opens a file and parses into an ELF structure */
ELF *elfParseFile(const char *FILENAME);

/* Parses a sequence of bytes into an ELF structure */
ELF *elfParse(FP *fp);

/* Frees an allocated ELF file */
void elfFree(ELF *elf);

#endif // !GUARD_ELFP_ELFP_H_
