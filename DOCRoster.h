#ifndef DOCRoster_H
#define DOCRoster_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits.h>

class DOCweekPattern
{
	// Monday is 0, Sunday is 6.
public:
	DOCweekPattern( const int aPattern = 0 ) { setPattern(aPattern); }
	
	bool isOn( const int d ) const { return (pattern & (1<<d)) != 0; }
	bool isOff( const int d ) const { return (pattern & (1<<d)) == 0; }
	
	unsigned int getOnDays() const { return onDays; }
	unsigned int getOffDays() const { return 7 - onDays; }
	
	bool getConsecutiveDaysOff() const { return consecutiveDaysOff; }
	bool getWeekendOff() const { return (pattern >> 5) == 0; }
	bool getWeekendDayOff() const { return (pattern >> 5) != 3; }
	bool getWorkWeekendDay() const { return !getWeekendOff(); }
	
	bool getConsecutiveIfAllWeekdayDaysOff() const
	{ return getConsecutiveDaysOff() || getWeekendDayOff(); }
	
	int	getPreference() const { return preference & ((1<<12)-1); }
	
	void setPattern( const int aPattern );
	unsigned int	getPattern() const { return pattern; }

	unsigned int	getInversePattern() const { return pattern ^ 127; }
	
	bool operator<( const DOCweekPattern &wp ) const
	{
		return preference > wp.preference ? true :
		preference < wp.preference ? false :
		pattern < wp.pattern;
	}

	bool hasDaysOff( const unsigned daysOffPattern ) const { return (daysOffPattern & pattern) == 0; }

	std::ostream &printOn( std::ostream &os ) const;
	
protected:
	unsigned	int pattern:7;		// pattern of this roster.
	unsigned	int onDays:3;		// on days of this roster.
	unsigned 	int consecutiveDaysOff:1;	// flag indicating this roster has consecutive days off.
	unsigned 	int preference:21;		// preference of this roster.
};

struct DOCweekendDayOffFirstCmp
{
	bool operator()( const DOCweekPattern &wp1, const DOCweekPattern &wp2 ) const
	{
		return
			wp1.getWeekendDayOff() && !wp2.getWeekendDayOff() ? true :
		!wp1.getWeekendDayOff() &&  wp2.getWeekendDayOff() ? false :
		wp1 < wp2;
	}
};

struct DOCweekendDayOffLastCmp
{
	bool operator()( const DOCweekPattern &wp1, const DOCweekPattern &wp2 ) const
	{
		return
			!wp1.getWeekendDayOff() &&  wp2.getWeekendDayOff() ? true :
		wp1.getWeekendDayOff() && !wp2.getWeekendDayOff() ? false :
		wp1 < wp2;
	}
};

inline std::ostream &operator<<( std::ostream &os, const DOCweekPattern &wp ) { return wp.printOn(os); }

//----------------------------------------------------------------------------------

class DOCweekPatterns
{
public:
	DOCweekPatterns()
	{
		for( register int i = 0; i < 128; ++i )
			wp[i].setPattern(i);
	}
	
	const DOCweekPattern &operator[]( const int i ) const { return wp[i]; }
	void	sort()	{ std::sort( &wp[0], &wp[128] ); }
	
private:
	DOCweekPattern	wp[128];
};

//----------------------------------------------------------------------------------

class DOCweekPatternCount : public DOCweekPattern
{
public:
	DOCweekPatternCount( const int aCount = 0 ) { set(aCount); }
	
	void set( const int aCount ) { count = aCount; }
	int	getCount() const { return count; }

	operator int() const { return count; }
	void operator++() { ++count; }
	void operator--() { --count; }
	
	DOCweekPatternCount &operator=( const DOCweekPattern &wp )
    { DOCweekPattern::operator=(wp); return *this; }
	DOCweekPatternCount &operator=( const DOCweekPatternCount &wpc )
    { DOCweekPattern::operator=(wpc); count = wpc.count; return *this; }
	
    bool operator<( const DOCweekPatternCount &wpc ) const { return count < wpc.count; }
    bool operator>( const DOCweekPatternCount &wpc ) const { return count > wpc.count; }
	
private:
	int	count;
};

class DOCweekPatternCounts
{
public:
	DOCweekPatternCounts() { reset(); }
	
	void	reset()
	{
		for( register int i = 0; i < 128; ++i )
		{
			wp[i].setPattern(i);
			wp[i].set(0);
		}
	}
	
	DOCweekPatternCounts &assign( DOCweekPatternCounts &wpc, int iMax = 128 )
	{
		if( iMax > 128 )
			iMax = 128;
		for( register int i = 0; i < iMax; ++i )
			wp[i] = wpc[i];
		return *this;
	}
	
	const DOCweekPatternCount &operator[]( const int i ) const { return wp[i]; }
	DOCweekPatternCount &operator[]( const int i ) { return wp[i]; }

	int	setPatterns(  const unsigned int  daysOff,
		      const bool consecutiveDO = false,
			  const bool consecutiveIfAllWeekdayDO = false,
			  const bool extrasOnWeekends = false );		// Returns number of patterns.

	void setExtrasOnWeekends( const bool extrasOnWeekends );

	void excludeDaysOffPattern( const unsigned daysOffPattern );	// Used for excluding days off patterns;
	
	void  sortDecreasingFrequency()
    { std::sort(&wp[0],&wp[128],std::greater<DOCweekPatternCount>()); }
	
	void  sortIncreasingFrequency()
    { std::sort(&wp[0],&wp[getNumValid()],std::less<DOCweekPatternCount>()); }
	
private:
	DOCweekPatternCount	wp[128];
	int	getNumValid() const
	{
		register int i;
		for( i = 0; wp[i].getPattern(); ++i )
			continue;
		return i;
	}
};

//----------------------------------------------------------------------------------

class DOCroster
{
public:
	DOCroster() { for( register d = 0; d < 7; ++d ) inWork[d] = 0; }
	
	int	operator[]( const int i ) const { return inWork[i]; }
	int	&operator[]( const int i ) { return inWork[i]; }
	
	int	leftOver( const int i ) const { return outWork[i]; }
	
	void	solve( int &workTotal, int &extraMax, int &extraTotal, DOCweekPatternCounts &wpc );
	
private:
	struct	SFrame
	{
		enum
		{
#if   UINT_MAX == 0xffff
			wordShift = 4, wordMask = 15,
#elif UINT_MAX == 0xffffffff
			wordShift = 5, wordMask = 31,
#else
			wordShift = 6, wordMask = 63,
#endif
			usedSize = (128 >> wordShift),	// There can be a max of 128 roster patterns for one week.
		};

		bool	valid() const { return i >= 0; }
		void	reset()
		{
			i = -1;

			used[0] = 0;
			used[1] = 0;
#if UINT_MAX <= 0xffffffff
			used[2] = 0;
			used[3] = 0;
#endif
#if UINT_MAX == 0xffff
			used[4] = 0;
			used[5] = 0;
			used[6] = 0;
			used[7] = 0;
#endif
		}

		SFrame	&operator=( const int aI )
		{ i = aI; used[i >> wordShift] |= (1<<(i & wordMask)); return *this; } // flag use of index.

		int		operator()() const { return i; }

		bool	operator[]( const int u ) const
		{ return (used[u >> wordShift] & (1<<(u & wordMask))) != 0; }

		int		i;						// Current index in the week pattern.
		unsigned int used[usedSize];	// Bit array of previously used indexes - this allows us to backtrack to a new unique index
	};
	
	int		getBest( SFrame &s, int *outWork, DOCweekPatternCounts &wpc );
	void	consume( int *outWork, DOCweekPatternCount &wpc );
	void	release( int *outWork, DOCweekPatternCount &wpc );
	int		inWork[7], outWork[7];
};

#endif // DOCRoster_H
