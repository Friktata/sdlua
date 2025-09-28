#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOWER_BASE	0x40
#define LOWER_TOP	0x5B
#define UPPER_BASE	0x60
#define UPPER_TOP	0x7B
#define TOGGLE_BIT	0x20

void printbyte(char byte) {
	if (
		(byte > LOWER_BASE && byte < LOWER_TOP) || 
		(byte > UPPER_BASE && byte < UPPER_TOP)
	)
		fprintf(stdout, "%c", TOGGLE_BIT ^ byte);
}

int main(int argc, char **argv) {
	if (argc == 1) {
		fprintf(stderr, "No input?!\n");
		exit(EXIT_FAILURE);
	}

	for (int arg = 1; arg < argc; arg++) {
		for (int byte = 0; byte < strlen(argv[arg]); byte++)
			printbyte(argv[arg][byte]);
	}

	fprintf(stdout, "\n");

	return EXIT_SUCCESS;
}
