/********************************************************************
 * File: trace.c
 *
 * main function for the pvtrace utility.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "symbols.h"
#include "stack.h"

int
main(int argc, char *argv[])
{
	FILE *tracef;
	char type;
	unsigned int address;

	if (argc != 2) {
		printf("Usage: pvtrace <image>\n\n");
		exit(-1);
	}
	initSymbol(argv[1]);
	stackInit();

	tracef = fopen("trace.txt", "r");

	if (tracef == NULL) {
		printf("Can't open trace.txt\n");
		exit(-1);
	}
	while (!feof(tracef)) {
		fscanf(tracef, "%c0x%x\n", &type, &address);
		switch (type) {
		case 'E':
			/* Function Entry */
			addSymbol(address);
			addCallTrace(address);
			stackPush(address);
			break;
		case 'X':
			/* Function Exit */
			(void) stackPop();
			break;
		}
	}

	emitSymbols();
	fclose(tracef);

	return 0;
}
