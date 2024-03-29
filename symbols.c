/********************************************************************
 * File: symbols.c
 *
 * Symbols functions.  This file has functions for symbols mgmt
 *  (such as translating addresses to function names with
 *  addr2line) and also connectivity matrix functions to keep
 *  the function call trace counts.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"
#include "symbols.h"

func_t functions[MAX_FUNCTIONS];
unsigned int totals[MAX_FUNCTIONS];
unsigned int calls[MAX_FUNCTIONS][MAX_FUNCTIONS];
char imageName[50];

void
initSymbol(char *image)
{
	int from, to;

	strncpy(imageName, image, sizeof(imageName) - 1);
	imageName[sizeof(imageName) - 1] = '\0';

	for (from = 0; from < MAX_FUNCTIONS; from++) {
		functions[from].address = 0;
		functions[from].funcName[0] = 0;
		totals[from] = 0;

		for (to = 0; to < MAX_FUNCTIONS; to++)
			calls[from][to] = 0;
	}
}

int
lookupSymbol(unsigned int address)
{
	int index;

	for (index = 0; index < MAX_FUNCTIONS; index++) {
		if (functions[index].address == 0)
			break;

		if (functions[index].address == address)
			return index;
	}

	return 0;
}

int
translateFunctionFromSymbol(unsigned int address, char *func)
{
	FILE *p;
	char line[100];
	int i;

	snprintf(line, sizeof(line), "addr2line -e %s -f -s 0x%x", imageName, address);

	p = popen(line, "r");

	if (p == NULL)
		return 0;

	fread(line, 99, 1, p);

	for (i = 0; i < strlen(line); i++) {
		if ((line[i] == 0x0d) || (line[i] == 0x0a)) {
			func[i] = 0;
			break;
		} else {
			func[i] = line[i];
		}
	}
	pclose(p);

	return 1;
}

void
addSymbol(unsigned int address)
{
	int index;

	for (index = 0; index < MAX_FUNCTIONS; index++) {
		if (functions[index].address == address)
			return;

		if (functions[index].address == 0)
			break;
	}

	if (index < MAX_FUNCTIONS) {
		functions[index].address = address;
		translateFunctionFromSymbol(address, functions[index].funcName);
	}
}

void
addCallTrace(unsigned int address)
{
	if (stackNumElems())
		calls[lookupSymbol(stackTop())][lookupSymbol(address)]++;
}

void
emitSymbols(void)
{
	int from, to;
	FILE *fp;

	fp = fopen("graph.dot", "w");
	if (fp == NULL) {
		printf("Couldn't open graph.dot\n");
		exit(0);
	}
	fprintf(fp, "digraph %s {\n\n", imageName);

	/* Identify node shapes */
	for (from = 0; from < MAX_FUNCTIONS; from++) {
		if (functions[from].address == 0)
			break;

		for (to = 0; to < MAX_FUNCTIONS; to++) {
			if (functions[to].address == 0)
				break;

			if (calls[from][to])
				totals[from]++;
		}

		fprintf(fp, "  %s [shape=%s]\n",
			totals[from] ? "rectangle" : "ellipse",
			functions[from].funcName);
	}

	/* Emit call graph */
	for (from = 0; from < MAX_FUNCTIONS; from++) {
		if (functions[from].address == 0)
			break;

		for (to = 0; to < MAX_FUNCTIONS; to++) {
			if (calls[from][to]) {
				fprintf(fp, "  %s -> %s [label=\"%d call%s\" fontsize=\"10\"]\n",
				    functions[from].funcName, functions[to].funcName,
				    calls[from][to], calls[from][to] > 1 ? "s" : "");
			}
			if (functions[to].address == 0)
				break;
		}
	}

	fprintf(fp, "\n}\n");

	fclose(fp);
}
