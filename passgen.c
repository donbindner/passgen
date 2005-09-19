/* Copyright (C) 2001 Donald J Bindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>

#define VERSION "0.3"

extern char *optarg;

#define PASS_LEN 255
char password[PASS_LEN+1];

int freq[256];
unsigned sum = 0;
unsigned mask;

FILE *randf;

unsigned generate_mask( int arg ) {
    unsigned i = 32768;
    int flag = 0;

    while( i ) {
	if( i&arg ) flag++;

	if( flag ) {
	    arg |= i;
	}

	i >>= 1;
    }
    return arg;
}

void init_empty( void ) {
    int i;

    for( i = 0; i < 256; i++ ) freq[i] = 0;
    sum = 0;
}

void init_addlower( int weight ) {
    int i;

    assert( weight >= 0 && weight < 32768 );

    for( i = 'a'; i <= 'z'; i++ ) {
	freq[i] += weight;
	sum += weight;
    }

    mask = generate_mask( sum-1 );
}

void init_addupper( int weight ) {
    int i;

    assert( weight >= 0 && weight < 32768 );

    for( i = 'A'; i <= 'Z'; i++ ) {
	freq[i] += weight;
	sum += weight;
    }

    mask = generate_mask( sum-1 );
}

void init_adddigit( int weight ) {
    int i;

    assert( weight >= 0 && weight < 32768 );

    for( i = '0'; i <= '9'; i++ ) {
	freq[i] += weight;
	sum += weight;
    }

    mask = generate_mask( sum-1 );
}

void init_addsymbol( int weight ) {
    assert( weight >= 0 && weight < 32768 );
    
    /* symbol */
    freq['~'] += weight; sum += weight;
    freq['`'] += weight; sum += weight;
    freq['!'] += weight; sum += weight;
    freq['@'] += weight; sum += weight;
    freq['#'] += weight; sum += weight;
    freq['$'] += weight; sum += weight;
    freq['%'] += weight; sum += weight;
    freq['^'] += weight; sum += weight;
    freq['&'] += weight; sum += weight;
    freq['*'] += weight; sum += weight;
    freq['('] += weight; sum += weight;
    freq[')'] += weight; sum += weight;
    freq['-'] += weight; sum += weight;
    freq['_'] += weight; sum += weight;
    freq['='] += weight; sum += weight;
    freq['+'] += weight; sum += weight;
    freq['['] += weight; sum += weight;
    freq['{'] += weight; sum += weight;
    freq[']'] += weight; sum += weight;
    freq['}'] += weight; sum += weight;
    freq['\\'] += weight; sum += weight;
    freq['|'] += weight; sum += weight;
    freq[';'] += weight; sum += weight;
    freq[':'] += weight; sum += weight;
    freq['\''] += weight; sum += weight;
    freq['"'] += weight; sum += weight;
    freq[','] += weight; sum += weight;
    freq['<'] += weight; sum += weight;
    freq['.'] += weight; sum += weight;
    freq['>'] += weight; sum += weight;
    freq['/'] += weight; sum += weight;
    freq['?'] += weight; sum += weight;

    mask = generate_mask( sum-1 );
}

void init_english( void ) {
    int i;

    init_empty();

    /* upper and lower in English-like frequency */
    freq['a'] =  856;  freq['A'] = freq['a']/3;
    freq['b'] =  139;  freq['B'] = freq['b']/3;
    freq['c'] =  279;  freq['C'] = freq['c']/3;
    freq['d'] =  378;  freq['D'] = freq['d']/3;
    freq['e'] = 1304;  freq['E'] = freq['e']/3;
    freq['f'] =  289;  freq['F'] = freq['f']/3;
    freq['g'] =  199;  freq['G'] = freq['g']/3;
    freq['h'] =  528;  freq['H'] = freq['h']/3;
    freq['i'] =  627;  freq['I'] = freq['i']/3;
    freq['j'] =   13;  freq['J'] = freq['j']/3;
    freq['k'] =   42;  freq['K'] = freq['k']/3;
    freq['l'] =  339;  freq['L'] = freq['l']/3;
    freq['m'] =  249;  freq['M'] = freq['m']/3;
    freq['n'] =  707;  freq['N'] = freq['n']/3;
    freq['o'] =  797;  freq['O'] = freq['o']/3;
    freq['p'] =  199;  freq['P'] = freq['p']/3;
    freq['q'] =   12;  freq['Q'] = freq['r']/3;
    freq['r'] =  677;  freq['R'] = freq['r']/3;
    freq['s'] =  607;  freq['S'] = freq['s']/3;
    freq['t'] = 1045;  freq['T'] = freq['t']/3;
    freq['u'] =  249;  freq['U'] = freq['u']/3;
    freq['v'] =   92;  freq['V'] = freq['v']/3;
    freq['w'] =  149;  freq['W'] = freq['w']/3;
    freq['x'] =   17;  freq['X'] = freq['x']/3;
    freq['y'] =  199;  freq['Y'] = freq['y']/3;
    freq['z'] =    8;  freq['Z'] = freq['z']/3;
    
    /* initialze the sum */
    for( i = 0; i < 256; i++ ) sum += freq[i];

    mask = generate_mask( sum-1 );
}

float lg( float arg ) {
/* returns log base 2 */
    return log(arg)/log(2.0);
}

void print_help( void ) {
    printf( "Usage: passgen [OPTIONS]\n" );
    printf( "OPTIONS\n" );
    printf( " -b bits      specify required entropy in bits (default 48)\n" );
    printf( " -c count     number of passwords to produce\n" );
    printf( " -d[weight]   include digits\n" );
    printf( " -e           use the default english-like probabilities\n" );
    printf( " -l[weight]   include lower case letters\n" );
    printf( " -p           paranoid: use /dev/random instead of /dev/urandom\n" );
    printf( " -q           quiet: do not report entropy\n" );
    printf( " -s[weight]   include symbols\n" );
    printf( " -u[weight]   include upper case letters\n" );
    printf( "Version %s by Donald J. Bindner\n", VERSION );
}

char random_char() {
    int i;
    unsigned ran, count;

    while( 1 ) {
	if( fread( &ran, sizeof( ran ), 1 , randf ) == 0 ) {
	    printf( "Could not read from random source.\n" );
	    exit( 1 );
	}

	/* mod into our set (works right because mask is 2^k - 1) */
	ran &= mask;

	count = 0;
	for( i = 0; i < 256; i++ ) {
	    count += freq[i];
	    if( count > ran ) {
		return (char)i;
	    }
	}
    }
}


int main( int argc, char *argv[] ) {
    char config_name[256];
    char line[256];
    int count;
    int lo, hi, fr;
    FILE *f;
    char *rand_fname = "/dev/urandom";
    int target = 48;	/* number of bits we want to generate */
    float bits = 0.0;	/* how many bits we have made so far */
    int set_size, base_length;
    long int seed;
    float size_bits;
    int opt;
    int i, l, n, num_pass=1;
    int english_flag=1, quiet_flag=0;
    unsigned lower_weight=0, upper_weight=0, digit_weight=0,
      symbol_weight=0;

    
    /* find our config file */
    config_name[0] = '\0';
    strncat( config_name, getenv( "HOME" ), sizeof( config_name ) -1 );
    strncat( config_name, "/.passgen",
	    sizeof( config_name ) - strlen( config_name ) - 1 );

    f = fopen( config_name, "r" );
    if( f != NULL ) {
	init_empty();
	count = 0;
	while( fgets( line, sizeof( line ), f )) {
	    count++;

	    /* skip empty lines */
	    if( line[0] == '\0' ) continue;
	    
	    /* parse line */
	    lo = (int)line[0];
	    if( line[1] == ':' ) {
		freq[lo] = strtol( line+2, NULL, 10 );
	    } else if( line[1] == '-' ) {
		hi = (int)line[2];
		if( line[3] != ':' ) {
		    printf( "Error in %s on line %d.\n", config_name, count );
		    exit( 1 );
		}
		fr = strtol( line+4, NULL, 10 );
		for( i = lo; i <= hi; i++ ) freq[i] = fr;
	    } else {
		printf( "Error in %s on line %d.\n", config_name, count );
		exit( 1 );
	    }
	}
	/* initialze the sum */
	for( i = 0; i < 256; i++ ) sum += freq[i];

	/* Set the mask if appropriate */
	if( sum ) {
	    english_flag = 0;
	    mask = generate_mask( sum-1 );
	}

	fclose( f );
    }

    /* process the options */
    while(( opt = getopt( argc, argv, "b:c:d::ehl::pqs::u::" )) != -1 ) {
	switch( opt ) {
	    case 'b':
		target = strtol( optarg, NULL, 10 );
		if( target < 1 || target > 1024 ) {
		    printf( "Bad value:  0 < bits <= 1024 required\n" );
		    exit( 1 );
		}
		break;
	    case 'c':
		num_pass = strtol( optarg, NULL, 10 );
		break;
	    case 'd':
		english_flag = 0;
		if( optarg ) {
		    digit_weight = (unsigned)strtol( optarg, NULL, 10 );
		    if( !digit_weight ) digit_weight = 1;
		} else digit_weight = 1;
		break;
	    case 'e':
		english_flag = 1;
		break;
	    case 'h':
		print_help();
		exit( 0 );
	    case 'l':
		english_flag = 0;
		if( optarg ) {
		    lower_weight = (unsigned)strtol( optarg, NULL, 10 );
		    if( !lower_weight ) lower_weight = 1;
		} else lower_weight = 1;
		break;
	    case 'p':
		rand_fname = "/dev/random";
		break;
	    case 'q':
		quiet_flag = 1;
		break;
	    case 's':
		english_flag = 0;
		if( optarg ) {
		    symbol_weight = (unsigned)strtol( optarg, NULL, 10 );
		    if( !symbol_weight ) symbol_weight = 1;
		} else symbol_weight = 1;
		break;
	    case 'u':
		english_flag = 0;
		if( optarg ) {
		    upper_weight = (unsigned)strtol( optarg, NULL, 10 );
		    if( !upper_weight ) upper_weight = 1;
		} else upper_weight = 1;
		break;
	    case ':':
	    case '?':
		exit( 1 );
	    default:
		printf( "Unexpected option: %c\n", opt );
		exit( 1 );
	}
    }
    
    /* Let's initialize our probabilities--either from options or
     * or ~/.passgen (above) or english-like defaults that author prefers */
    if( english_flag ) {
	init_english();
	init_adddigit( 200 );
	init_addsymbol( 50 );

	/* Eliminate easily confused characters */
	freq['I'] = freq['1'] = freq['l'] = 0;
	freq['0'] = freq['O'] = 0;
    } else if( lower_weight || upper_weight || digit_weight || symbol_weight ) {
	init_empty();
	init_addlower( lower_weight );
	init_addupper( upper_weight );
	init_adddigit( digit_weight );
	init_addsymbol( symbol_weight );
    }

    /* compute base password length from set size */
    set_size = 0;
    for( i = 0; i < 256; i++ ) {
	if( freq[i] ) set_size++;
    }
    size_bits = lg( (float)set_size );
    base_length = target/size_bits+0.5;

    /* open random source for reading */
    if( NULL == (randf = fopen( rand_fname, "r" ))) {
	printf( "Could not open %s\n", rand_fname );
	exit( 1 );
    };

    /* seed random number generator -- used only for making choices*/
    /* not for generating new characters */
    if( fread( &seed, sizeof( seed ), 1 , randf ) == 0 ) {
	printf( "Could not read from random source.\n" );
	exit( 1 );
    }
    srand48( seed );

    for( n = 0; n < num_pass; n++ ) {
	bits = 0.0;
	l = 0;

	/* fill up password of base_length characters */
	assert( base_length < PASS_LEN );
	for( l = 0; l < base_length; l++ ) {
	    i = random_char();
	    password[l] = i;
	    bits += -lg((float)freq[i] / (float)sum);
	}

	/* keep outputting characters until we have enough entropy */
	while( l < PASS_LEN ) {
	    if( drand48() < pow( 2.0, bits-target)) break;

	    i = random_char();
	    password[l] = i;
	    bits += -lg((float)freq[i] / (float)sum);
	    l++;
	}
	password[l] = '\0';

	if( quiet_flag ) {
	    printf( "%s\n", password );
	} else {
	    printf( "%s (entropy = %.1f bits)\n", password, bits );
	}
    }

    exit( 0 );
}
