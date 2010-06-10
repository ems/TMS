
#ifndef StrTrim_H
#define StrTrim_H

template <typename S>
inline S trim( const S &s )
{
	register typename S::const_iterator nsBegin = s.begin(), cEnd = s.end();
	while( nsBegin != cEnd && isspace(*nsBegin) )
		++nsBegin;

	register typename S::const_iterator nsEnd = cEnd;
	while( nsEnd != nsBegin && isspace(*(nsEnd-1)) )
		--nsEnd;

	if( nsBegin == nsEnd )
		return S();

	if( nsBegin == s.begin() && nsEnd == cEnd )
		return s;

	return s.substr(nsBegin-s.begin(), nsEnd-nsBegin);
}

template <typename S>
inline S &trimInPlace( S &s )
{
	register typename S::iterator nsBegin = s.begin(), cEnd = s.end();
	while( nsBegin != cEnd && isspace(*nsBegin) )
		++nsBegin;

	register typename S::iterator nsEnd = cEnd;
	while( nsEnd != nsBegin && isspace(*(nsEnd-1)) )
		--nsEnd;

	if( nsEnd != cEnd )				s.erase( nsEnd, s.end() );
	if( nsBegin != s.begin() )		s.erase( s.begin(), nsBegin );

	return s;
}

template <typename S>
inline void trimQuotesInPlace( S &s )
{
	trimInPlace( s );
	const size_t len = s.size();
	if( len > 1 && (s[0] == '"' || s[0] == '\'') && s[len-1] == s[0] )
		s = s.substr( 1, len-2 );
}

#endif // StrTrim_H