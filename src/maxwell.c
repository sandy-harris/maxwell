/*
	A daemon to get entropy from timer randomness
	and feed it to random(4)

	It borrows some code from Folkert van Heusden's
	timer entropy daemon http://www.vanheusden.com/te/
	but the two programs are quite different.

	License is GPL v2, the same as the earlier code.

	If anyone needs it under another Open Source
	license, they can contact me.

	This is version 2, 2016
	The main changes from v1, 2012 are
		different command-line options
		constants LO_CLAIM & HI_CLAIM
		(instead of a different prime for each option)
		moving actual output from main() to do1k()
		random choice of delay in do1k()

	Sandy Harris, sandyinchina@gmail.com
*/

#include "maxwell.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <syslog.h>

#include <linux/random.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

int good_opt(char *) ;
int good_num(char *) ;

void usage() ;

/* minimum number of loops */
#define MIN_LOOPS 3

/* entropy claims */
#define LO_CLAIM 16
#define HI_CLAIM 30

/*
	global variables
	set in main()
	used by do1k(), log_error() and error_exit()
*/
char *prog_name ;
int user, demon, output, loops, mix, mul ;

/*
	primes taken from
	http://primes.utm.edu/lists/small/1000.txt

	used as microsecond delay values
*/
unsigned primes[] = {89, 97, 101, 103, 107} ;	// median 101, mean 99.4
unsigned smallp[] = {43, 47,  53,  57,  59} ;	// median  53, mean 51.8

/*
	constants borrowed from SHA-1
	reference used, RFC 3174

	used as initialisation constants
	for the entropy-collecting variable
*/
u32 sha_c[] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0} ;

/*
	constant for multiplications

	11, 37 and 71 are 3, 5 and 7 mod 8
	so they all give some mixing in the low bits
	and they each do it differently
*/
#define MUL (71+(37<<12)+(11<<19))

/*
	produce one K bit of output
	32 32-bit words
*/
void do1k( unsigned delay, unsigned claim )
{
	int i, j, k, ret ;
	unsigned a, b ;

	// fprintf( stderr, "do1k delay %d claim %d mix %d loops %d\n", delay, claim, mix, loops) ;

	for( i = 0 ; i < 32 ; i++ )	{
		if( mix )
			// initialise with a somewhat random constant
			a = sha_c[t5()] ;
		else
			a = 0 ;
		// main loop; sample & mix
		for( j = 0 ; j < loops ; j++)	{
			// get 16 samples for entropy
			for( k = 0 ; k < 16 ; k++ )	{
				usleep(delay) ;
				// mix in a sample
				// multiplication spreads bits out
				a += ( t31() * mul ) ;
				// rotate left by two bits
				b = a >> 30 ;
				a = (a << 2) | b ;
			}
			if( loops > 1 )	{
				// rotate by one bit between loops
				b = a >> 31 ;
				a = (a << 1) | b ;
			}
		}
		// mix thoroughly
		if( mix )
			a = qht(a) ;
		// output 32 bits
		if( (ret = write( output, &a, 4)) != 4)
			error_exit("write to output file failed") ;
		// update entropy estimate
		if( demon )
			// a failure here is logged
			// but does not stop the program
			// it does no real harm
			if( (ioctl(output, RNDADDTOENTCNT, claim)) == -1)
				message("ioctl() failed") ;
	}
}

int main( int argc, char **argv)
{
	unsigned a, b, delay, limit, *ptr ;
	int i, p, out, fast, halt ;
	char *u, *v ;

	/*
		defaults
		might be overidden by options
	*/
	p = 0 ;			// no paranioa, three loops
	halt = 0 ;		// loop forever
	fast = 4 ;		// 4 K bits done quickly

	/*
		defaults
		always apply except in testing
	*/
	demon = 1 ;
	mix = 1 ;
	mul = MUL ;
	loops = MIN_LOOPS ;
	/*
		default unless overriden by demon=1
		output to standard out
	*/
	output = 1 ;

	/* argument processing */
	prog_name = *argv ;
	for( argc--, argv++ ; (argc > 0) && (*argv != NULL) ; argc--, argv++ )	{
		u = *argv ;
		if( u[0] != '-' )
			usage() ;
		switch( u[1] )	{
			case '0' :
				message( "-0 option asks for 0 output, makes no sense") ;
				usage() ;
			// any digit but zero
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				v = u + 1 ;
				loops = atoi(v) ;
				// no mixing for test output
				mix = 0 ;
				mul = 1 ;
				// output to stdout
				// demon = 0 ;
				break ;
			// output to stdout
			case 's':
				demon = 0 ;
				break ;
			// paranoia level
			case 'p' :
				v = argv[1] ;
				if( (v == NULL) || !good_num(v) )
					usage() ;
				p = atoi(v) ;
				// extra increment to skip over numeric argument
				argc-- ; argv++ ;
				break ;
			// initial fast data in K bits
			case 'f' :
				v = argv[1] ;
				if( (v == NULL) || !good_num(v) )
					usage() ;
				fast = atoi(v) ;
				argc-- ; argv++ ;
				break ;
			// halt after K bits
			case 'h' :
				v = argv[1] ;
				if( (v == NULL) || !good_num(v) )
					usage() ;
				halt = atoi(v) ;
				argc-- ; argv++ ;
				break ;
			default:
				usage() ;
		}
	}

	// documented in getuid(1) as "always successful"
	// so do not bother checking for failure
	user = geteuid() ;

	/*
		set up various things
	*/

	// not testing and not root: fail
	if( demon && (user != 0) )	{
		fprintf(stderr, "%s: must be root to run maxwell(8)\n", prog_name) ;
		fprintf(stderr, "%s: updates kernel entropy count, so root only\n", prog_name) ;
		fprintf(stderr, "%s: exception is 'maxwell -s' for testing\n", prog_name) ;
		exit(1) ;
	}

	// not testing and root
	if( demon )	{
		if( (output=open("/dev/random", O_WRONLY)) == -1)
			error_exit("failed to open /dev/random") ;
		if( daemon(-1, -1) == -1)
			error_exit("failed to become daemon process") ;
		openlog( prog_name, (LOG_CONS|LOG_PID), LOG_DAEMON) ;
	}

	message( "maxwell(8) v 2" );

	// first loop, fast output to fill random(4) pool
	// uses smaller prime and default loops == MIN_LOOPS for speed
	// -p option has no effect here; not needed when claiming only LO_CLAIM
	// halt == 0 means run forever
	// fprintf( stderr, "fast loop: p %d loops %d halt %d fast %d output %d demon %d\n", p, loops, halt, fast, output, demon ) ;
	for( i = 0 ; (i < fast) && ((halt == 0) || (i < halt)) ; i++ )
		do1k( smallp[t5()], LO_CLAIM ) ;

	// main loop, halt == 0 means run forever
	// -p takes efect here
	loops = MIN_LOOPS + 2*p ;
	// fprintf( stderr, "main loop: p %d loops %d halt %d fast %d output %d demon %d\n", p, loops, halt, fast, output, demon ) ;
	for( /* continue with same i */ ; (halt == 0) || (i < halt) ; i++ )
		do1k( primes[t5()], HI_CLAIM ) ;

	message( "shutting down" );
	exit(0) ;
}

void error_exit(const char *reason)
{
	if( demon )		// running as root
		syslog(LOG_ERR, "%s", reason) ;
	else
		fprintf(stderr, "%s: error: %s\n", prog_name, reason) ;
	exit(-1) ;
}

void message(const char *reason)
{
	if( demon )
		syslog(LOG_INFO, "%s", reason) ;
	// else
	// do nothing
}

void usage()
{
	fprintf(stderr, "usage: %s [-<digit>] [-s] [-p <number>] [-h <number>] [-f <number>]\n", prog_name) ;
	fprintf(stderr, "usage: %s <digit> must be in range 1-9, <number> 0-99\n", prog_name) ;
	exit(-1) ;
}

/*
	allows numbers 0 - 99, no + - or decimal point
*/
int good_num(char *p)
{
	int i ;
	for( i = 0 ; *p ; p++, i++ )	{
		if( isdigit(*p)	)
			// do nothing
			 ;
		else
			return(0) ;
	}
	return( (i>0) && (i<=2) ) ;
}