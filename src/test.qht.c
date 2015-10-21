#include "maxwell.h"

#define LIMIT (32*1024)	// a megabit of output

int main( int argc, char **argv)
{
	u32 a ;
	int i;
	// test outer loop of maxwell
	for( i = 0 ; i < LIMIT ; i++ )	{
		// all the sampling replaced
		a = qht(i) ;
		// write to stdout
		write(1, &a, 4) ;
	}
	exit(0) ;
}
