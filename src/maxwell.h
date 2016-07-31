#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned u32 ;

/*
	constant for multiplications

	11, 37, 71 and 41 are 3, 5, 7 and 9 mod 16
	so they all give some mixing in the low bits
	and they each do it differently

	These get multiplied by a value taken mod 31
	so each of the shifted constants can affect
	a different number of bits. e.g. the 1 can
	affect only 5 bits, but 71 can give a value
	up to 31*71 and affect up to 11 bits.
*/
#define MUL (71+(37<<8)+(41<<15)+(11<<19)+(1<<25))

/* forward declarations of functions */

u32 qht(u32) ;
u32 idea(u32,u32) ;

u32 t5() ;
u32 t7() ;
u32 t31() ;
u32 t255() ;

u32 g5() ;
u32 g7() ;
u32 g31() ;
u32 g255() ;

u32 gpar() ;
u32 tpar() ;

u32 parity(u32) ;

void error_exit(const char *) ;
void message(const char *) ;
