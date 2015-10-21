#include "maxwell.h"
#include <assert.h>
#include <string.h>

#define OUTPUTS (128*1024)	// 128 k samples

int main(int argc, char **argv)
{
	int i, g, x ;
	if( (argc == 2) && !strcmp(argv[1], "-g") )
		g = 1 ;
	else	g = 0 ;
	for( i = 0 ; i < OUTPUTS ; i++ )	{
		usleep(97) ;		//default delay
		// get sample
		if( g )
			x = g31() ;	// microsecond timer
		else	x = t31() ;	// nanosecond
		// check it is in range
		assert( x > 0) ;
		assert( x < 32) ;
		// output as byte
		putchar( x ) ;
	}
	exit(0) ;
}
