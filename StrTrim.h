
#ifndef StrTrim_H
#define StrTrim_H

template <typename S>
inline S trim( const S &s )
{
	register S::const_iterator nsBegin = s.begin(), cEnd = s.end();
	while( nsBegin != cEnd && isspace(*nsBegin) )
		++nsBegin;

	register S::const_iterator nsEnd = cEnd;
	while( nsEnd != nsBegin && isspace(*(nsEnd-1)) )
		--nsEnd;

	if( nsBegin == nsEnd )
		return S();

	if( nsBegin == s.begin() && nsEnd == cEnd )
		return s;

	return s.substr(nsBegin-s.begin(), nsEnd-nsBegin);
}

template <typename S>
inline void trimInPlace( S &s )
{
	register S::iterator nsBegin = s.begin(), cEnd = s.end();
	while( nsBegin != cEnd && isspace(*nsBegin) )
		++nsBegin;

	register S::iterator nsEnd = cEnd;
	while( nsEnd != nsBegin && isspace(*(nsEnd-1)) )
		--nsEnd;

	if( nsEnd != cEnd )				s.erase( nsEnd, s.end() );
	if( nsBegin != s.begin() )		s.erase( s.begin(), nsBegin );
}

#endif // StrTrim_H