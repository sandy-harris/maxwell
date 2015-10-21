#include "maxwell.h"

#define LIMIT (256*1024)	// a megabyte of output

#define MUL (71+(37<<12)+(11<<19))

int main( int argc, char **argv)
{
	u32 a, b, x ;
	int i, j, k ;
	// test maxwell's mixing
	// x is an overall counter
	for( i = 0, x = 0 ; i < LIMIT ; i++ )	{
		a = 0 ;
		for( j = 0; j < 3 ; j++ )	{
			// increment x as well as k
			for( k = 0 ; k < 16 ; k++, x++ )	{
				// all the sampling replaced
				// by the counter x
				b = (x % 31) + 1 ;
				// mixing is like maxwell
				a += b * MUL ;
				// rotate left by two bits
				b = a >> 30 ;
				a = (a << 2) | b ;
			}
			// rotate left one bit
			b = a >> 31 ;
			a = (a << 1) | b ;
		}
		// write to stdout
		write(1, &a, 4) ;
	}
	exit(0) ;
}
