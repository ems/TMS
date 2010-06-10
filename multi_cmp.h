
#ifndef multi_cmp_H
#define multi_cmp_H

#include <string>

namespace MC
{

enum MCmp { less = 1, greater = -1 };

// Default comparison.
template < typename T >
struct cmp
{
	int	operator()( const T &x, const T &y ) const
	{ return x < y ? -1 : y < x ? 1 : 0; }
};

// Specialized comparisons for integer-like and string data types.
template <> struct cmp<int>
{ int operator()( const int x, const int y ) const { return y - x; } };

template <> struct cmp<short>
{ int operator()( const short x, const short y ) const { return (int)y - (int)x; } };

template <> struct cmp<unsigned short>
{ int operator()( const unsigned short x, const unsigned short y ) const { return (int)y - (int)x; } };

template <> struct cmp<char>
{ int operator()( const char x, const char y ) const { return (int)y - (int)x; } };

template <> struct cmp<unsigned char>
{ int operator()( const unsigned char x, const unsigned char y ) const { return (int)y - (int)x; } };

template <> struct cmp<const char *>
{ int operator()( const char *x, const char *y ) const { return strcmp(x, y); } };

template <> struct cmp<const bool>
{ int operator()( const bool x, const bool y ) const { return (int)y - (int)x; } };

template <> struct cmp<const std::string>
{ int operator()( const std::string &x, const std::string &y ) const { return strcmp(x.c_str(), y.c_str()); } };

template < typename T0 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0 )
{
	return op0 * cmp<T0>()(x0, y0)); 
}

// Multi-field comparisons.
template < typename T0, typename T1 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	return 0;
}

template < typename T0, typename T1, typename T2 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1,
						const T2 &x2, const MCmp op2, const T2 &y2 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	if(ret = cmp<T2>()(x2, y2)) return op2 * ret;
	return 0;
}

template < typename T0, typename T1, typename T2, typename T3 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1,
						const T2 &x2, const MCmp op2, const T2 &y2,
						const T3 &x3, const MCmp op3, const T3 &y3 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	if(ret = cmp<T2>()(x2, y2)) return op2 * ret;
	if(ret = cmp<T3>()(x3, y3)) return op3 * ret;
	return 0;
}

template < typename T0, typename T1, typename T2, typename T3, typename T4 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1,
						const T2 &x2, const MCmp op2, const T2 &y2,
						const T3 &x3, const MCmp op3, const T3 &y3,
						const T4 &x4, const MCmp op4, const T4 &y4 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	if(ret = cmp<T2>()(x2, y2)) return op2 * ret;
	if(ret = cmp<T3>()(x3, y3)) return op3 * ret;
	if(ret = cmp<T4>()(x4, y4)) return op4 * ret;
	return 0;
}

template < typename T0, typename T1, typename T2, typename T3, typename T4, typename T5 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1,
						const T2 &x2, const MCmp op2, const T2 &y2,
						const T3 &x3, const MCmp op3, const T3 &y3,
						const T4 &x4, const MCmp op4, const T4 &y4,
						const T5 &x5, const MCmp op5, const T5 &y5 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	if(ret = cmp<T2>()(x2, y2)) return op2 * ret;
	if(ret = cmp<T3>()(x3, y3)) return op3 * ret;
	if(ret = cmp<T4>()(x4, y4)) return op4 * ret;
	if(ret = cmp<T5>()(x5, y5)) return op5 * ret;
	return 0;
}

template < typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1,
						const T2 &x2, const MCmp op2, const T2 &y2,
						const T3 &x3, const MCmp op3, const T3 &y3,
						const T4 &x4, const MCmp op4, const T4 &y4,
						const T5 &x5, const MCmp op5, const T5 &y5,
						const T6 &x6, const MCmp op6, const T5 &y6 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	if(ret = cmp<T2>()(x2, y2)) return op2 * ret;
	if(ret = cmp<T3>()(x3, y3)) return op3 * ret;
	if(ret = cmp<T4>()(x4, y4)) return op4 * ret;
	if(ret = cmp<T5>()(x5, y5)) return op5 * ret;
	if(ret = cmp<T6>()(x6, y6)) return op6 * ret;
	return 0;
}

template < typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
inline int multi_cmp(	const T0 &x0, const MCmp op0, const T0 &y0,
						const T1 &x1, const MCmp op1, const T1 &y1,
						const T2 &x2, const MCmp op2, const T2 &y2,
						const T3 &x3, const MCmp op3, const T3 &y3,
						const T4 &x4, const MCmp op4, const T4 &y4,
						const T5 &x5, const MCmp op5, const T5 &y5,
						const T6 &x6, const MCmp op6, const T5 &y6,
						const T7 &x7, const MCmp op7, const T5 &y7 )
{
	int ret;
	if(ret = cmp<T0>()(x0, y0)) return op0 * ret;
	if(ret = cmp<T1>()(x1, y1)) return op1 * ret;
	if(ret = cmp<T2>()(x2, y2)) return op2 * ret;
	if(ret = cmp<T3>()(x3, y3)) return op3 * ret;
	if(ret = cmp<T4>()(x4, y4)) return op4 * ret;
	if(ret = cmp<T5>()(x5, y5)) return op5 * ret;
	if(ret = cmp<T6>()(x6, y6)) return op6 * ret;
	if(ret = cmp<T7>()(x7, y7)) return op7 * ret;
	return 0;
}

} // namespace MC



#endif // multi_cmp_H