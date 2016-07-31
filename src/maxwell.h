#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned u32 ;

/*
	constant for multiplications

	11, 37 and 71 are 3, 5 and 7 mod 8
	so they all give some mixing in the low bits
	and they each do it differently
*/
#define MUL (71+(37<<12)+(11<<19))

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
