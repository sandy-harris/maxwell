/*
	A daemon to get entropy from timer randomness
	and feed it to random(4)

	It borrows some code from Folkert van Heusden's
	timer entropy demon http://www.vanheusden.com/te/
	but the two programs are quite different.
	
	License is GPL v2, the same as the earlier code.

	If anyone needs it under another Open Source
	license, they can contact me.

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

/*
	minimum number of loops
*/
#define MIN_LOOPS 3

/*
	global variables
	set in main()
	used by log_error() and error_exit()
*/
char *prog_name ;
int user, demon ;

/*
	primes taken from
	http://primes.utm.edu/lists/small/1000.txt

	used as microsecond delay values
*/
unsigned primes[] = {41,43,97,101,103,107,109,113,127} ;

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

int main( int argc, char **argv)
{
	unsigned a, b, p, delay, limit, loops ;
	int i, j, out, output, foreground, ret, claim, mul, mix ;
	char *u, *v ;

	/*
		defaults
		might be overidden by options
	*/
	p = 0 ;			// no paranioa, three loops
	delay = primes[2] ;
	limit = 0 ;		// loop forever
	claim = 30 ;		// entropy claim per output
	/*
		defaults
		always apply except in testing
	*/
	demon = 1 ;
	mix = 1 ;
	mul = MUL ;
	/*
		default unless overriden by demon=1
		output to standard out
	*/
	output = 1 ;
	foreground = 0 ;

	/* argument processing */
	prog_name = *argv ;
	switch(argc)	{
		// no options, use defaults
		case 1:
			break ;
		// one, to four name/value pairs
		// no check for repeated options
		// so if same option twice, 2nd is used
		case 9:
		case 7:
		case 5:
		case 3:
			for( argc--, argv++ ; argc ; argc -= 2, argv += 2)	{
				u = argv[0] ;
				v = argv[1] ;
				if( good_opt(u) && good_num(v) )
					a = atoi(v) ;
				else	usage() ;
				if( !strcmp(u,"-p") )
					p = a ;
				else if( !strcmp(u,"-d") )
					delay = a ;
				else if( !strcmp(u,"-s") )
					limit = a ;
				else if( !strcmp(u,"-c") )
					if( a <=32 )
						claim = a ;
					else
						claim = 32 ;
				else
					usage() ;
			}
			break ;
		// one of the standalone options
		case 2:
			// get the option value
			i = good_opt(argv[1]) ;
			// fprintf(stderr, "option -> %c\n", i) ;

			// digit -> loops with no mixing
			if( isdigit(i) )	{
				demon = 0 ;
				limit = 1024 ;
				mix = 0 ;
				mul = 1 ;
				loops = (i -'0') ;
			}
			else switch(i)	{
				// faster variants
				// reduce claim so random(4) accepts
				// inputs faster
				case 'f' :
					delay = primes[0] ;
					claim = 16 ;
					limit = 8 ;
					break ;
				case 'g' :
					delay = primes[1] ;
					claim = 16 ;
					limit = 16 ;
					break ;
				// as -g but don't daemonize
				case 'G' :
					delay = primes[1] ;
					claim = 16 ;
					limit = 16 ;
					foreground = 1;
					break ;
				// for the paranoids
				// -x, -y -z do more loops
				case 'x' :
					delay = primes[3] ;
					p = 2 ;
					break ;
				case 'y' :
					delay = primes[4] ;
					p = 3 ;
					break ;
				case 'z' :
					delay = primes[5] ;
					p = 4 ;
					break ;
				// options for testing
				case 'm' :
					// one megabit
					limit = 1024 ;
					demon = 0 ;
					break ;
				case 't' :
					// zero delay, faster test
					delay = 0 ;
					demon = 0 ;
					break ;
				default:
					usage() ;
					break ;
			}
			break ;
		default:
			usage() ;
			break ;
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
		fprintf(stderr, "%s: exception is 'maxwell -[t|m|0-9]' for testing\n", prog_name) ;
		exit(1) ;
	}

	// not testing and root
	if( demon )	{
		if( (output=open("/dev/random", O_WRONLY)) == -1)
			error_exit("failed to open /dev/random") ;
		if( !foreground && daemon(-1, -1) == -1)
			error_exit("failed to become daemon process") ;
		openlog( prog_name, (LOG_CONS|LOG_PID), LOG_DAEMON) ;
	}

	// calculate loops per output
		loops = MIN_LOOPS + 2*p ;

	// limit is given in K bits
	// but we need the number of 32-bit chunks
	if( limit )
		limit *= 32 ;

	message( "maxwell(8) v 1.2" );

	for( out = 0 ; (limit == 0) || (out < limit) ; out++) {
		if( mix )
			// initialise with a somewhat random constant
			a = sha_c[t5()] ;
		else
			a = 0 ;
		// main loop; sample & mix
		for( i = 0 ; i < loops ; i++)	{
			// get 16 samples for entropy 
			for( j = 0 ; j < 16 ; j++ )	{
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
		if( (ret = write(output, &a, 4)) != 4)
			error_exit("write to /dev/random failed") ;
		// update entropy estimate
		if( demon )
			// a failure here is logged
			// but does not stop the program
			// it does no real harm
			if( (ioctl(output, RNDADDTOENTCNT, claim)) == -1)
				message("ioctl() failed") ;
	}
	message( "shutting down" );
	exit(0) ;
}

void error_exit(const char *reason)
{
	if( demon )		// running as root
		syslog(LOG_ERR, "%s", reason) ;
	else
		fprintf(stderr, "%s: %s\n", prog_name, reason) ;
	exit(-1) ;
}

void message(const char *reason)
{
	if( demon )
		syslog(LOG_INFO, "%s", reason) ;
	// else
	//   do nothing
}

void usage()
{
	fprintf(stderr, "usage: %s [-p <number>] [-d <number>] [-s <number>] [-c number]\n", prog_name) ;
	fprintf(stderr, "usage: %s [-f|-g|-m|-t|-x|-y|-z|-<digit>]\n", prog_name) ;
	exit(-1) ;
}

/*
	is this a legal single character option?

	no program-specific checks
	just is it '-' followed by a single letter or digit
	if so, return the letter or digit
*/
int good_opt(char *p)
{
	if( (p[0] == '-') && isalnum(p[1]) && (p[2] == '\0') )
		return(p[1]) ;
	else
		return(0) ;
}

/*
	allows numbers 0 - 999, no + - or decimal point
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
	return( (i>0) && (i<=3) ) ;
}

