
#ifndef BinSearch_H
#define BinSearch_H

// Search an ordered array with binary search.  If the item is not found,
// a pointer is returned to next greatest element.
// Allows the key to be a different type than the elements in the array.
template <typename T, typename K, typename Cmp>
inline T	*binSearch( const T *first, const T *last, const K &e, const Cmp &cmp )
{
	register const T *left = first - 1, *right = last, *mid;
	register unsigned long n;
	while( (n = right - left) > 1 )
	{
		mid = left + (n >> 1);
		if( cmp(*mid, e) )
			left = mid;
		else
			right = mid;
	}
	// Return the search point at the position the element should be if it
	// is not found.  This requires one final check.
	return const_cast<T *>(left < first || cmp(*left, e) ? left + 1 : left);
}

// Wrapper for searching by iterator.
template <typename Itr, typename K, typename Cmp>
inline Itr	binSearchItr( Itr first, Itr last, const K &e, const Cmp &cmp )
{
	register Itr left = first - 1, right = last, mid;
	register size_t n;
	while( (n = right - left) > 1 )
	{
		mid = left + (n >> 1);
		if( cmp(*mid, e) )
			left = mid;
		else
			right = mid;
	}
	// Return the search point at the position the element should be if it
	// is not found.  This requires one final check.
	return (left < first || cmp(*left, e) ? left + 1 : left);
}

#endif // BinSearch_H