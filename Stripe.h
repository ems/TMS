#ifndef Stripe_H
#define Stripe_H

#include "tod.h"

class Stripe
{
public:
	struct ILocTime
	{
		ILocTime( const size_t aILoc = 0, const tod_t aT = -1 ) : iLoc(aILoc), t(aT) {}

		size_t	iLoc;	// Index of location in idLocations array (see CISroute).
		tod_t	t;		// Time if event.
	};
	typedef std::vector<ILocTime>	ILocTimeVector;

	struct ILocTimeCmp
	{
		bool operator()( const ILocTime &loc1, const ILocTime &loc2 ) const
		{
			return loc1.iLoc < loc2.iLoc ? true
				:  loc2.iLoc < loc1.iLoc ? false
				:  loc1.t < loc2.t;
		}
	};
	struct ILocTimeCmpLoc
	{
		bool operator()( const ILocTime &loc1, const ILocTime &loc2 ) const
		{ return loc1.iLoc < loc2.iLoc; }
	};

	void	push( const size_t iLoc, const tod_t t )
	{
		if( !iLocTimes.empty() )
		{
			assert( iLocTimes.back().iLoc < iLoc );
			assert( iLocTimes.back().t <= t );
		}
		iLocTimes.push_back( ILocTime(iLoc, t) );
	}

	void	appendAllButFirst( const Stripe &c )
	{
		iLocTimes.reserve( iLocTimes.size() + c.iLocTimes.size() - 1 );
		std::copy( c.iLocTimes.begin() + 1, c.iLocTimes.end(), std::back_inserter(iLocTimes) );
	}

	size_t	getLocsInCommon( const Stripe &c ) const
	{
		std::vector<size_t>	locs;
		locs.reserve( iLocTimes.size() );
		register ILocTimeVector::const_iterator	i, iEnd;

		for( i = iLocTimes.begin(), iEnd = iLocTimes.end(); i != iEnd; ++i )
			locs.push_back( i->iLoc );

		std::sort( locs.begin(), locs.end() );

		register size_t iCount = 0;
		for( i = c.iLocTimes.begin(), iEnd = c.iLocTimes.end(); i != iEnd; ++i )
		{
			std::vector<size_t>::const_iterator iFound = std::lower_bound(locs.begin(), locs.end(), i->iLoc);
			if( iFound != locs.end() && *iFound == i->iLoc )
				++iCount;
		}
		return iCount;
	}

	bool	onlyFirstLocInCommon( const Stripe &c ) const
	{
		std::vector<size_t>	locs;
		locs.reserve( iLocTimes.size() );
		register ILocTimeVector::const_iterator	i, iEnd;

		for( i = iLocTimes.begin(), iEnd = iLocTimes.end(); i != iEnd; ++i )
			locs.push_back( i->iLoc );

		std::sort( locs.begin(), locs.end() );

		register bool firstInCommon = false;
		register size_t iCount = 0;
		for( i = c.iLocTimes.begin(), iEnd = c.iLocTimes.end(); i != iEnd; ++i )
		{
			std::vector<size_t>::const_iterator iFound = std::lower_bound(locs.begin(), locs.end(), i->iLoc);
			if( iFound != locs.end() && *iFound == i->iLoc )
			{
				++iCount;
				if( i == c.iLocTimes.begin() )
					firstInCommon = true;
			}
			if( iCount > 2 )
				break;
		}
		return firstInCommon && iCount == 1;
	}

	size_t	getFirstILoc() const { return iLocTimes[0].iLoc; }
	tod_t	getFirstTime() const { return iLocTimes[0].t; }
	size_t	getLastILoc() const { return iLocTimes.back().iLoc; }
	tod_t	getLastTime() const { return iLocTimes.back().t; }

private:
	bool	getCommonLocationPrivate( size_t &iLoc1, size_t &iLoc2, const Stripe &s ) const
	{
		if( iLocTimes.empty() || s.iLocTimes.empty() )
			return false;

		// Take a wild guess - we will be right for the most common case.
		if( iLocTimes[0].iLoc == s.iLocTimes[0].iLoc )
		{
			iLoc1 = iLoc2 = 0;
			return true;
		}

		if( iLocTimes.back().iLoc < s.iLocTimes[0].iLoc )
			return false;

		if( s.iLocTimes.back().iLoc < iLocTimes[0].iLoc )
			return false;

		// Find the best place to start in the smaller stripe.
		ILocTimeVector::const_iterator found = std::lower_bound( iLocTimes.begin(), iLocTimes.end(), s.iLocTimes[0], ILocTimeCmpLoc() );
		iLoc1 = found - iLocTimes.begin();
		if( found->iLoc == s.iLocTimes[0].iLoc )
		{
			iLoc2 = 0;
			return true;
		}
		for( const size_t iLoc1End = iLocTimes.size(); iLoc1 < iLoc1End && iLocTimes[iLoc1].iLoc <= s.iLocTimes.back().iLoc; ++iLoc1 )
		{
			found = std::lower_bound( s.iLocTimes.begin(), s.iLocTimes.end(), iLocTimes[iLoc1], ILocTimeCmpLoc() );
			if( found != s.iLocTimes.end() )
			{
				iLoc2 = found - s.iLocTimes.begin();
				return true;
			}
		}
		return false;
	}

public:
	bool	getCommonLocation( size_t &iLoc1, size_t &iLoc2, const Stripe &s ) const
	{
		return iLocTimes.size() < s.iLocTimes.size()
			? getCommonLocationPrivate(iLoc1, iLoc2, s) : s.getCommonLocationPrivate(iLoc2, iLoc1, *this);
	}

	bool	operator<( const Stripe &s ) const
	{
		if( iLocTimes.empty() )
			return !s.iLocTimes.empty();

		// Check if the input is off the back.
		if( iLocTimes.back().iLoc < s.iLocTimes[0].iLoc )
			return iLocTimes.back().t < s.iLocTimes[0].t;

		// Check if this is off the back.
		if( s.iLocTimes.back().iLoc < iLocTimes[0].iLoc )
			return iLocTimes[0].t < s.iLocTimes.back().t;

		// Check for a common location.
		size_t	iLoc1, iLoc2;
		if( getCommonLocation(iLoc1, iLoc2, s) )
			return iLocTimes[iLoc1].t < s.iLocTimes[iLoc2].t;

		// At this point, the stripes land in a hole of each other.
		// Find the closest location to the first gap and compare the times there.
		if( iLocTimes[0].t < s.iLocTimes[0].t )
		{
			ILocTimeVector::const_iterator i = std::lower_bound( iLocTimes.begin(), iLocTimes.end(), s.iLocTimes[0], ILocTimeCmpLoc() );
			if( i->iLoc > s.iLocTimes[0].iLoc )
				--i;
			return i->t < s.iLocTimes[0].t;
		}
		else
		{
			ILocTimeVector::const_iterator i = std::lower_bound( s.iLocTimes.begin(), s.iLocTimes.end(), iLocTimes[0], ILocTimeCmpLoc() );
			if( i->iLoc > iLocTimes[0].iLoc )
				--i;
			return iLocTimes[0].t < i->t;
		}
		return false;
	}

	ILocTimeVector	iLocTimes;
};

typedef std::vector<Stripe>	StripeVector;

#endif // Stripe_H