#ifndef FastRand_H
#define FastRand_H

#define CARRY_METHOD
#ifdef CARRY_METHOD
// Concatenation of following two 16-bit multiply with carry generators
// x(n)=a*x(n-1)+carry mod 2^16 and y(n)=b*y(n-1)+carry mod 2^16,
// number and carry packed within the same 32 bit integer.

class FastRand
{
protected:
	// Use any pair of non-equal numbers from this list for "a" and "b"
    // 18000 18030 18273 18513 18879 19074 19098 19164 19215 19584       
    // 19599 19950 20088 20508 20544 20664 20814 20970 21153 21243       
    // 21423 21723 21954 22125 22188 22293 22860 22938 22965 22974       
    // 23109 23124 23163 23208 23508 23520 23553 23658 23865 24114       
    // 24219 24660 24699 24864 24948 25023 25308 25443 26004 26088       
    // 26154 26550 26679 26838 27183 27258 27753 27795 27810 27834       
    // 27960 28320 28380 28689 28710 28794 28854 28959 28980 29013       
    // 29379 29889 30135 30345 30459 30714 30903 30963 31059 31083
	enum { a = 18000, b = 30903 };

public:
	FastRand( unsigned int seed1 = 0, unsigned int seed2 = 0 ) { seed(seed1, seed2); }

	void seed( unsigned int seed1 = 0, unsigned int seed2 = 0 )
    {
	   if (seed1) SEED_X = seed1; else SEED_X = 521288629;
	   if (seed2) SEED_Y = seed2; else SEED_Y = 362436069;
	}

	void init() { seed(); }

	unsigned int rand()
	{
		// return a random integer in the range of [0,4294967296)
		SEED_X = ((unsigned int)a)*(SEED_X&65535) + (SEED_X>>16);
		SEED_Y = ((unsigned int)b)*(SEED_Y&65535) + (SEED_Y>>16);
		return (SEED_X<<16) + (SEED_Y&65535);
	}

	unsigned int operator()( const unsigned n ) { return (unsigned int)(drand() * n); }
	
	double drand()
	{
		// return a uniformly distributed random number in the range of [0,1)
		SEED_X = ((unsigned int)a)*(SEED_X&65535) + (SEED_X>>16);
		SEED_Y = ((unsigned int)b)*(SEED_Y&65535) + (SEED_Y>>16);
		return ((SEED_X<<16) + (SEED_Y&65535)) / 4294967296.0;
	}
	
	double drand( const double lb, const double ub )
	{
		// return a uniformly distributed random number in the range of [lb,ub)
		SEED_X = ((unsigned int)a)*(SEED_X&65535) + (SEED_X>>16);
		SEED_Y = ((unsigned int)b)*(SEED_Y&65535) + (SEED_Y>>16);
		return lb + (ub - lb) * (((SEED_X<<16) + (SEED_Y&65535)) / 4294967296.0);
	}

protected:
	unsigned int SEED_X, SEED_Y;
};

#else

// Matsumoto method.
#include <limits.h>

class FastRand
{
protected:
	enum { N = 25, M = 7 };

public:
	FastRand()
	{
		k = 0;
		mag01[0] = 0x0; mag01[1] = 0x8ebfd028; // this is magic vector `a' - don't change!
		x[ 0] = 0x95f24dab; x[ 1] = 0x0b685215; x[ 2] = 0xe76ccae7; x[ 3] = 0xaf3ec239; x[ 4] = 0x715fad23;
		x[ 5] = 0x24a590ad; x[ 6] = 0x69e4b5ef; x[ 7] = 0xbf456141; x[ 8] = 0x96bc1b7b; x[ 9] = 0xa7bdf825;
		x[10] = 0xc1de75b7; x[11] = 0x8858a9c9; x[12] = 0x2da87693; x[13] = 0xb657f9dd; x[14] = 0xffdc8a9f;
		x[15] = 0x8121da71; x[16] = 0x8b823ecb; x[17] = 0x885d05f5; x[18] = 0x4e20cd47; x[19] = 0x5a9ad5d9;
		x[20] = 0x512c0c03; x[21] = 0xea857ccd; x[22] = 0x4cc1d30f; x[23] = 0x8891a8a1; x[24] = 0xa6b7aadb;
	}

	unsigned int rand()
	{
		// return a random integer in the range of [0,4294967296)
		if( k == N )
			nextWords();

		register unsigned int y = x[k++];
		y ^= (y << 7) & 0x2b5b2500; /* s and b, magic vectors */
		y ^= (y << 15) & 0xdb8b0000; /* t and c, magic vectors */
#if UINT_MAX != 0xffffffff
		y &= 0xffffffff;
#endif
		y ^= (y >> 16);
		return y;
	}

	unsigned int operator()( const unsigned n ) { return rand() % n; }
	
	// return a uniformly distributed random number in the range of [0,1)
	double drand()
	{
		if( k == N )
			nextWords();

		register unsigned int y = x[k++];
		y ^= (y << 7) & 0x2b5b2500; /* s and b, magic vectors */
		y ^= (y << 15) & 0xdb8b0000; /* t and c, magic vectors */
#if UINT_MAX != 0xffffffff
		y &= 0xffffffff;
#endif
		y ^= (y >> 16);
		return (double) y / (unsigned int) 0xffffffff;
	}

	// return a uniformly distributed random number in the range of [lb,ub)
	double drand( const double lb, const double ub ) { return lb + (ub - lb) * drand(); }

protected:
	void	nextWords()
	{
		// generate N words at one time
		register int kk;
		for( kk = 0; kk < N-M; kk++)
			x[kk] = x[kk+M] ^ (x[kk] >> 1) ^ mag01[x[kk] % 2];
		for( ; kk < N; kk++)
			x[kk] = x[kk+(M-N)] ^ (x[kk] >> 1) ^ mag01[x[kk] % 2];
		k = 0;
	}

	int	k;
	unsigned int x[N];
	unsigned int mag01[2];
};

#endif // CARRY_METHOD

#endif // FastRand_H