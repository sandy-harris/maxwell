#include "maxwell.h"
#include <assert.h>
#include <string.h>

void one_output() ;

int g, q ; // flags

int main(int argc, char **argv)
{
	int i, limit, forever ;
	// set defaults
	g = 0 ;
	q = 0 ;
	forever = 0 ;
	limit = 256*1024 ;	// 256 K 32-bit -> one megabyte
	for( argc--, argv++ ; argc ; argc--, argv++ )	{
		if( !strcmp(*argv, "-g") )
			g = 1 ;
		if( !strcmp(*argv, "-f") )
			forever = 1 ;
	}
	if( forever )
		// infinite loop
		for( ; ; )
			one_output() ;
	else
		// a megabyte
		for( i = 0 ; i < limit ; i++ )
			one_output() ;
	exit(0) ;
}

void one_output()
{
	int i ;
	unsigned x, y ;
	// minimum delay
	usleep(1) ;
	// for each bit position
	for( i = 0, y = 0 ; i < 32 ; i++, y <<= 1 )	{
		// get samples
		if( g )
			x = gpar() ;	// microsecond timer
		else	x = tpar() ;	// nanosecond
		y |= x ;
	}
	// output four bytes
	write( 1, &y, 4 ) ;
}
