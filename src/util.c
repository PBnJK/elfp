/* elfp
 * Utilities
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fault.h"

#include "util.h"

#define SHF(B, N) (((B) & 0xFF) << (N))

FP *utilReadFile(const char *FILEPATH) {
	FILE *file = fopen(FILEPATH, "rb");
	if( file == NULL ) {
		ERR("couldn't open the file at '%s'\n", FILEPATH);
		return NULL;
	}

	FP *fp = malloc(sizeof(*fp));
	if( fp == NULL ) {
		ERR("failed to allocate file pointer for file at '%s", FILEPATH);
	}

	fseek(file, 0L, SEEK_END);
	fp->size = ftell(file);
	rewind(file);

	fp->_start = malloc(fp->size + 1);
	if( fp->_start == NULL ) {
		ERR("failed to allocate buffer for file at '%s'\n", FILEPATH);
		return NULL;
	}

	const size_t BYTES_READ = fread(fp->_start, sizeof(char), fp->size, file);
	if( BYTES_READ < fp->size ) {
		ERR("couldn't read the file at '%s'\n", FILEPATH);
		return NULL;
	}

	fp->_start[BYTES_READ] = '\0';
	fp->data = fp->_start;

	fclose(file);

	return fp;
}

void utilFreeFile(FP *fp) {
	free(fp->_start);
	fp->data = NULL;

	free(fp);
	fp = NULL;
}

uint8_t utilRead8(FP *fp) {
	return *fp->data++;
}

uint16_t utilRead16(bool le, FP *fp) {
	uint16_t lo, hi;

	if( le ) {
		hi = utilRead8(fp);
		lo = utilRead8(fp);
	} else {
		lo = utilRead8(fp);
		hi = utilRead8(fp);
	}

	return (lo << 8) | hi;
}

uint32_t utilRead32(bool le, FP *fp) {
	uint32_t a, b, c, d;

	if( le ) {
		d = utilRead8(fp);
		c = utilRead8(fp);
		b = utilRead8(fp);
		a = utilRead8(fp);
	} else {
		a = utilRead8(fp);
		b = utilRead8(fp);
		c = utilRead8(fp);
		d = utilRead8(fp);
	}

	return SHF(a, 24) | SHF(b, 16) | SHF(c, 8) | (d & 0xFF);
}

uint64_t utilRead64(bool le, FP *fp) {
	uint64_t a, b, c, d, e, f, g, h;

	if( le ) {
		h = utilRead8(fp);
		g = utilRead8(fp);
		f = utilRead8(fp);
		e = utilRead8(fp);
		d = utilRead8(fp);
		c = utilRead8(fp);
		b = utilRead8(fp);
		a = utilRead8(fp);
	} else {
		a = utilRead8(fp);
		b = utilRead8(fp);
		c = utilRead8(fp);
		d = utilRead8(fp);
		e = utilRead8(fp);
		f = utilRead8(fp);
		g = utilRead8(fp);
		h = utilRead8(fp);
	}

	return SHF(a, 56) | SHF(b, 48) | SHF(c, 40) | SHF(d, 32) | SHF(e, 24)
		| SHF(f, 16) | SHF(g, 8) | (h & 0xFF);
}
