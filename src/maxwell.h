#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

typedef uint32_t u32 ;

/*
	constant for multiplications

	11, 37, 71 and 137 are 3, 5, 7 and 9 mod 16
	so they all give some mixing in the low bits
	and they each do it differently. Then the
	shifts make them do it in different places.

	These get multiplied by a value taken mod 31
	so each of the shifted constants can affect
	a different number of bits. e.g. the 1 can
	affect only 5 bits, but 71 can give a value
	up to 31*71 and affect up to 11 bits.
*/
#define MUL (71+(37<<8)+(137<<15)+(11<<19)+(1<<25))

/* forward declarations of functions */

u32 qht(u32) ;

int got_clock(void) ;

u32 t5(void) ;
u32 t7(void) ;
u32 t31(void) ;
u32 t255(void) ;

u32 g5(void) ;
u32 g7(void) ;
u32 g31(void) ;
u32 g255(void) ;