/********************************************************************
 * File: stack.h
 *
 * Simple stack implementation header.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#ifndef __STACK_H
#define __STACK_H

__BEGIN_DECLS
void stackInit(void);
int stackNumElems(void);
unsigned int stackTop(void);
void stackPush(unsigned int value);
unsigned int stackPop(void);
__END_DECLS

#endif
