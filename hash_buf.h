
#ifndef hash_buf_H
#define hash_buf_H

#include <string>
#include <ctype.h>

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

typedef unsigned __int8		ub1;
typedef unsigned __int32	ub4;

#ifdef HASH_BUF_INLINING_OK

#define hash_buf_mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

inline ub4 hash_buf(register const ub1 *k,		/* the key */
					register ub4 length,		/* the length of the key */
					const ub4 initval )			/* the previous hash, or an arbitrary value */
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      hash_buf_mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((ub4)k[10]<<24);
   case 10: c+=((ub4)k[9]<<16);
   case 9 : c+=((ub4)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((ub4)k[7]<<24);
   case 7 : b+=((ub4)k[6]<<16);
   case 6 : b+=((ub4)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((ub4)k[3]<<24);
   case 3 : a+=((ub4)k[2]<<16);
   case 2 : a+=((ub4)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   hash_buf_mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}

#undef hash_buf_mix

#else // !HASH_BUF_INLINING_OK

inline ub4 hash_buf( register const ub1 *key, register ub4 len, ub4 initval )
{
	register		ub4   hash = 0x9e3779b9 + initval;
	register const	ub1	  *keyEnd = key + len;
	while( key != keyEnd )
	{
		hash += *key++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
} 

#endif // HASH_BUF_INLINING_OK

inline ub4 hash_buf_icase( register const ub1 *key, register ub4 len, ub4 initval )
{
	register		ub4   hash = 0x9e3779b9 + initval;
	register const	ub1	  *keyEnd = key + len;
	while( key != keyEnd )
	{
		hash += tolower(*key++);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
} 

inline ub4	hash_buf( const char *k, const ub4 initval = 0 )		{ return hash_buf( (const ub1 *)k, strlen(k), initval ); }
inline ub4	hash_buf( const std::string &s, const ub4 initval = 0 )	{ return hash_buf( (const ub1 *)s.c_str(), s.size(), initval ); }
inline ub4	hash_buf_icase( const char *k, const ub4 initval = 0 )			{ return hash_buf_icase( (const ub1 *)k, strlen(k), initval ); }
inline ub4	hash_buf_icase( const std::string &s, const ub4 initval = 0 )	{ return hash_buf_icase( (const ub1 *)s.c_str(), s.size(), initval ); }
inline ub4	hash_buf( const int i, const ub4 initval = 0 )			{ return hash_buf( (const ub1 *)&i, sizeof(int), initval ); }
inline ub4	hash_buf( const unsigned int i, const ub4 initval = 0 )	{ return hash_buf( (const ub1 *)&i, sizeof(unsigned int), initval ); }
inline ub4	hash_buf( const long l, const ub4 initval = 0 )			{ return hash_buf( (const ub1 *)&l, sizeof(long), initval ); }
inline ub4	hash_buf( const unsigned long l, const ub4 initval = 0 ){ return hash_buf( (const ub1 *)&l, sizeof(unsigned long), initval ); }
inline ub4	hash_buf( const double d, const ub4 initval = 0 )		{ return hash_buf( (const ub1 *)&d, sizeof(double), initval ); }
inline ub4	hash_buf( const void *v, const ub4 initval = 0 )		{ return hash_buf( (const ub1 *)&v, sizeof(void *), initval ); }
inline ub4	hash_buf( const bool b, const ub4 initval = 0 )			{ return hash_buf( (const ub1 *)&b, sizeof(bool), initval ); }

template <	typename T0, typename T1 >
inline ub4 multi_hash( const T0 &v0, const T1 &v1 )
{
	ub4	h = 0;
	h = hash_buf( v0, h );
	h = hash_buf( v1, h );
	return h;	
}

template <	typename T0, typename T1, typename T2 >
inline ub4	multi_hash( const T0 &v0, const T1 &v1, const T2 &v2 )
{
	ub4	h = 0;
	h = hash_buf( v0, h );
	h = hash_buf( v1, h );
	h = hash_buf( v2, h );
	return h;	
}

template <	typename T0, typename T1, typename T2, typename T3 >
inline ub4	multi_hash( const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3 )
{
	ub4	h = 0;
	h = hash_buf( v0, h );
	h = hash_buf( v1, h );
	h = hash_buf( v2, h );
	h = hash_buf( v3, h );
	return h;	
}

template <	typename T0, typename T1, typename T2, typename T3, typename T4 >
inline ub4	multi_hash( const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4 )
{
	ub4	h = 0;
	h = hash_buf( v0, h );
	h = hash_buf( v1, h );
	h = hash_buf( v2, h );
	h = hash_buf( v3, h );
	h = hash_buf( v4, h );
	return h;	
}

template <	typename T0, typename T1, typename T2, typename T3, typename T4, typename T5 >
inline ub4	multi_hash( const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5 )
{
	ub4	h = 0;
	h = hash_buf( v0, h );
	h = hash_buf( v1, h );
	h = hash_buf( v2, h );
	h = hash_buf( v3, h );
	h = hash_buf( v4, h );
	h = hash_buf( v5, h );
	return h;	
}

#endif // hash_buf_H