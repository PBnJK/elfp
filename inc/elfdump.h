#ifndef GUARD_ELFP_ELFDUMP_H_
#define GUARD_ELFP_ELFDUMP_H_

#include "elfp.h"

#define ELF_DUMP_EH 1 /* Dump entry header */
#define ELF_DUMP_PH 2 /* Dump program headers */
#define ELF_DUMP_SH 4 /* Dump section headers */
#define ELF_DUMP_ALL (ELF_DUMP_EH | ELF_DUMP_PH | ELF_DUMP_SH)

/* Dumps an ELF's content */
void elfDump(ELF *elf, int flags);

#endif // !GUARD_ELFP_ELFDUMP_H_
