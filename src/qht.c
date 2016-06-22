#include "maxwell.h"

/*
	Quasi-Hadamard transform
	My own invention

	Goal is to mix a 32-bit object
	so that each output bit depends
	on every input bit

	Underlying primitive is IDEA
	multiplication which mixes
	a pair of 16-bit objects

	This is analogous to the
	pseudo-Hadamard transform
	(PHT) originally from the
	SAFER cipher, later in
	Twofish and others

	Conceptually, a two-way PHT
	on a,b is:

	x = a + b
	y = a + 2b
	a = x
	b = y

	This is reversible; it loses
	no information. Each output
	word depends on both inputs.

	A PHT can be implemented as

	a += b
	b += a

	which is faster and avoids
	using intermediate variables

	QHT is the same thing using
	IDEA multiplication instead
	of addition, calculating a*b
	and a*b^2 instead of a+b and
	a+2b

	IDEA multiplication operates
	on 16-bit chunks and makes
	every output bit depend on
	all input bits. Therefore
	QHT is close to an ideal
	mixer for 32-bit words.
*/

u32 qht(u32 x)
{
	u32 a, b ;
	a = x >> 16 ;		// high 16 bits
	b = x & 0xffff ;	// low 16
	a = idea(a,b) ;		// a *= b
	b = idea(a,b) ;		// b *= a
	return( (a<<16) | b) ;
}

/*
	IDEA multiplication
	borrowed from the IDEA cipher
*/
#define	MAX (1<<16)
#define MOD (MAX+1)

u32 idea(u32 a, u32 b)
{
	u32 x ;
	// make sure they are in range
	a %= MOD ;
	b %= MOD ;
	// special cases
	if( (a == 0) && (b == 0))
		return(1) ;
	else if( a == 0)
		return(MOD - b) ;
	else if( b == 0)
		return(MOD - a) ;
	// non-special
	else	{
		x = (a*b) % MOD ;
		if(x == MAX)
			return(0) ;
		else	return(x) ;
	}
}
