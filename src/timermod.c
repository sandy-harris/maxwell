#include "maxwell.h"
#include <time.h>
#include <sys/time.h>

/*
	Some code here is a bit sloppy and might
	fail on a 16-bit machine. I declare several
	variables as unsigned then assign items of
	type time_t or even long into them.

	To extract bits from the timer, we use
	timer modulo 5, 7, 31 or 255

	This better than just selecting some
	of the low bits with a mask or with
	a modulo operation using a power of
	two. It lets all input bits affect
	the output.

	We do not need larger moduli because
	we do not expect a timer sample to
	have more than a few bits of
	entropy.

	On the other hand, if there are
	sometimes two bits of entropy,
	then mod 2 or mod 3 would lose
	some, and mod 4 uses only low
	bits so it would lose any that
	are in high bits. mod 5 or mod 7
	are then better

	mod 31 or mod 255 are suitable if
	more entropy is expected
*/

/*
	struct has sec and nsec
	or sec and usec
	so multiplier is a million
	or a billion
	
	if you want total time
	(B * sec) + nsec
	(M * sec) + usec
*/
#define B 1000000000
#define M 1000000

/*
	Generic timer modulo m
	using a hi-resolution timer

	Because of the nanosecond resolution
	this is preferable to the gmod()
	routine below
*/
unsigned tmod(m)
{
	struct timespec t ;
	int ret ;
	unsigned x ;
	// use monotonic clock, which not even root can reset
	if( (ret = clock_gettime(CLOCK_MONOTONIC,&t)) == - 1)	{
		fprintf(stderr,"timer mod: clock read fails\n") ;
	}
	/*
	we want (B * sec) + nsec
	but multiplication or addition
	might overflow

	extra modulo operations
	guarantee no overflow
	*/
	x = (B%m) * (t.tv_sec%m) ;
	x += t.tv_nsec ;
	return( x % m ) ;
}

/*
	Generic timer modulo m
	using gettimeofday()
	microsecond timer

	This, and its derivatives g5() etc,
	are mostly used for testing;
	if a microsecond timer has entropy,
	then we can be confident that a
	nanosecond timer does

	Could also be used on a system that
	lacks the realtime clocks
*/
unsigned gmod(m)
{
	struct timeval t ;
	int ret ;
	unsigned x ;
	if( (ret = gettimeofday(&t,NULL)) == - 1)	{
		fprintf(stderr,"timer mod: clock read fails\n") ;
	}
	/*
	we want (M * sec) + usec
	but multiplication or addition
	might overflow

	extra modulo operations
	guarantee no overflow
	*/
	x = (M%m) * (t.tv_sec%m) ;
	x += t.tv_usec ;
	return( x % m ) ;
}


/*
	these return timer modulo something
	they could likely be done as macros
*/

u32 t5(){ return( tmod(5) ) ; }
u32 g5(){ return( gmod(5) ) ; }

u32 t255(){ return( tmod(255) ) ; }
u32 g255(){ return( gmod(255) ) ; }

/*
	these are not quite timer modulo x
	return a value in range 1 to x
	rather than 0 to x-1 for modulo
	always non-zero
*/
u32 t7(){ return( tmod(7)+1 ) ; }
u32 g7(){ return( gmod(7)+1 ) ; }

u32 t31(){ return( tmod(31)+1 ) ; }
u32 g31(){ return( gmod(31)+1 ) ; }

/*
	functions to take the parity
	of a timer
*/
unsigned tpar()
{
	struct timespec t ;
	int ret ;
	unsigned x ;
	// use monotonic clock, which not even root can reset
	if( (ret = clock_gettime(CLOCK_MONOTONIC,&t)) == - 1)	{
		fprintf(stderr,"timer mod: clock read fails\n") ;
	}
	x = parity(t.tv_sec ^ t.tv_nsec ) ;
	return( x ) ;
}

unsigned gpar()
{
	struct timeval t ;
	int ret ;
	unsigned x ;
	if( (ret = gettimeofday(&t,NULL)) == - 1)	{
		fprintf(stderr,"timer mod: clock read fails\n") ;
	}
	x = parity(t.tv_sec ^ t.tv_usec ) ;
	return( x ) ;
}

/*
	brute force solution
*/
unsigned parity(unsigned x)
{
	unsigned p ;
	for( p = 0 ; x ; x >>= 1)
		p ^= (x&1) ;
	return(p) ;
}
