#ifndef GUARD_ELFP_UTIL_H_
#define GUARD_ELFP_UTIL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Structure representing an open file
 * Used for passing data around
 */
typedef struct _FP {
	char *_start; /* Points to the start of the allocated memory segment */
	char *data; /* Actual data pointer that should be used */
	size_t size; /* Size of the file */
} FP;

/* Reads the file at 'FILEPATH' and returns its contents
 * Returns NULL on failure
 */
FP *utilReadFile(const char *FILEPATH);

/* Frees a file pointer returned by 'utilReadFile' */
void utilFreeFile(FP *fp);

uint8_t utilRead8(FP *fp);
uint16_t utilRead16(bool le, FP *fp);
uint32_t utilRead32(bool le, FP *fp);
uint64_t utilRead64(bool le, FP *fp);

#endif // !GUARD_ELFP_UTIL_H_
