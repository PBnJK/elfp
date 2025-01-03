/* elfp
 * Entry point
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elfdump.h"
#include "elfp.h"

#include "fault.h"

static void _usage(void) {
	printf("usage: elfp [OPTIONS] (file)\n");
	printf("       -a, --all....... Print all information\n");
	printf("       -h, --header.... Print the Entry Header\n");
	printf("       -p, --program... Print the Program Header\n");
	printf("       -s, --section... Print the Section Header\n");
}

#define NEXT()                                                                 \
	--argc;                                                                    \
	++argv

static bool _isOpt(char *argv[], char argS, char *argL) {
	char *cmd = *argv;
	if( *cmd != '-' ) {
		fprintf(stderr, "unexpected %s\n", cmd);
		exit(EXIT_FAILURE);
	}

	if( cmd[1] == argS && cmd[2] == '\0' ) {
		return true;
	}

	if( cmd[1] == '-' && cmd[2] != '\0' ) {
		return strcmp(cmd + 2, argL) == 0;
	}

	return false;
}

#define CHECK(S, L) if( _isOpt(argv, (S), (L)) )
#define EXPECT(B)                                                              \
	do {                                                                       \
		if( argc == 1 ) {                                                      \
			fprintf(stderr, "expected " B "\n");                               \
			exit(EXIT_FAILURE);                                                \
		}                                                                      \
		NEXT();                                                                \
	} while( false )

int main(int argc, char *argv[]) {
	if( argc < 2 ) {
		ERR("must specify a file as input\n\n");
		_usage();
		exit(EXIT_FAILURE);
	}

	char *file = NULL;
	int flags = 0;

	NEXT();
	while( argc > 0 ) {
		if( *argv[0] != '-' ) {
			file = *argv;
			break;
		}

		CHECK('a', "all") {
			flags = ELF_DUMP_ALL;
		}
		else CHECK('h', "header") {
			flags |= ELF_DUMP_EH;
		}
		else CHECK('p', "program") {
			flags |= ELF_DUMP_PH;
		}
		else CHECK('s', "section") {
			flags |= ELF_DUMP_SH;
		}
		else {
			ERR("unknown option '%s'\n\n", *argv);
			_usage();
			exit(EXIT_FAILURE);
		}

		NEXT();
	}

	if( file == NULL ) {
		ERR("must specify a file as input\n\n");
		_usage();
		exit(EXIT_FAILURE);
	}

	if( flags == 0 ) {
		ERR("must specify what information to print\n\n");
		_usage();
		exit(EXIT_FAILURE);
	}

	ELF *elf = elfParseFile("./elfp");
	if( elf == NULL ) {
		return EXIT_FAILURE;
	}

	elfDump(elf, flags);

	elfFree(elf);

	return EXIT_SUCCESS;
}
