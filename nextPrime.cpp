//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "nextPrime.h"

#define WORD_BIT	32

#if WORD_BIT == 64
#  define WORD_SHIFT	6
#  define WORD_MASK		63
#elif WORD_BIT == 32
#  define WORD_SHIFT	5
#  define WORD_MASK		31
#elif WORD_BIT == 16
#  define WORD_SHIFT	4
#  define WORD_MASK		15
#endif

static	unsigned int *bits = NULL;
static	size_t numBits = 0;

inline void setBit( const size_t i )
{ bits[i>>WORD_SHIFT] |= (1U << (i & WORD_MASK)); }

inline void clearBit( const size_t i )
{ bits[i>>WORD_SHIFT] &= ~(1U << (i & WORD_MASK)); }

inline unsigned int checkBit( const size_t i )
{ return bits[i>>WORD_SHIFT] & (1U << (i & WORD_MASK)); }

inline unsigned int checkPrime( const size_t i )
{ return checkBit( i>>1U ); }

static void	buildPrimeMap()
{
  numBits = ((numBits + WORD_MASK) & ~WORD_MASK);	// round up to the next bit multiple.
  size_t	iBitMax = (numBits >> WORD_SHIFT);
  
  operator delete( bits );
  bits = (unsigned int *)operator new( iBitMax * sizeof(bits[0]) );
  
  register size_t i, j, k;
  for( i = 0; i < iBitMax; i++ )
    bits[i] = ~0U; // set all the bits.
  
  // Do sieve, but only for the odd numbers.
  clearBit( 0 );
  for( i = 1; i < numBits; i++ )
  {
    if( checkBit(i) )
	{
		k = (i << 1) + 1;
		for( j = i + k; j < numBits; j += k )
			clearBit( j );
	}
  }
}

static inline int	isPrime( const size_t num )
{
  // This prime checking routine is really only good for small primes.
  // We maintain a bitmap of the prime numbers so we can check them quickly.
  if( num >= (numBits<<1) )
    {
      numBits = (num<<1);
      buildPrimeMap();
    }

  return (num & 1) && (checkPrime(num) != 0);
}

#ifdef FIXLATER
int      keyCollectionBase::isPrime( const size_t num )
{
    // Check 2 - a special case.
    if( num <= 3 )
      return num != 1;

    // If the number is a multiple of 2, reject it.
    if( (num & 1) == 0 )
        return 0;

    // Compute the sum of odd numbers as an approximation of the square
    // root of num.  This avoids computing the sqrt.
    // Initialize to the first 3 odd numbers as we are starting at 3.
    register size_t sumOdd = 1 + 3 + 5;
    register size_t i;
    for( i = 3; ; i += 2 )
    {
        if( num % i == 0 )
            return 0;

        // Add in the next 2 odd numbers since we are incrementing by 2
        // each iteration.
        sumOdd += ((i<<1) + 1) + (((i+1)<<1) + 1);
        if( sumOdd > num )
            break;
    }
    return 1;
}
#endif

size_t   nextPrime( register size_t num )
{
  // Check for even numbers.
  if( (num & 1) == 0 )
    num++;

  // Find a non-zero word in the bit array.
  // This helps us find a fast starting point to search from.
  register size_t i, newNum, bitMax = (numBits >> WORD_SHIFT);
  for( i = (num>>(WORD_SHIFT+1)); i < bitMax; i++ )
	  if( bits[i] )
		break;
  newNum = (i << (WORD_SHIFT+1)) + 1;
  if( newNum > num )
	  num = newNum;

  // Keep checking odd numbers until we find a prime.
  while( !isPrime(num) )
    num += 2;
  return num;
}
