//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef TimeInterval_H
#define TimeInterval_H

#define	tiMin	(INT_MIN / 64)
#define	tiMax	(INT_MAX / 64)

//--------------------------------------------------------------------------
// The TimeInterval class is an inclusive representation.  For example,
// the time interval 0, 1, 2, 3, 4, 5 would be represented as min=0, max=5
// and would have a duration of 6.
// An empty time interval is represented by min > max.

class TimeInterval
{
public:
	TimeInterval( const tod_t aMin, const tod_t aMax ) { set(aMin, aMax); }
	TimeInterval( const tod_t aT ) { set(aT, aT); }
	TimeInterval() { reset(); }
	void reset()
	{
		m_min = tiMax;
		m_max = tiMin;
	}
	TimeInterval &operator|=( const TimeInterval &t )
	{
		if( t.m_min < m_min ) m_min = t.m_min;
		if( t.m_max > m_max ) m_max = t.m_max;
		return *this;
	}
	TimeInterval &operator|=( const tod_t t )
	{
		if( t < m_min ) m_min = t;
		if( t > m_max ) m_max = t;
		return *this;
	}
	TimeInterval &operator=( const TimeInterval &t )
	{
		m_min = t.m_min;
		m_max = t.m_max;
		return *this;
	}
	int operator==( const TimeInterval &ti ) const
	{ return m_min == ti.m_min && m_max == ti.m_max; }
	int operator!=( const TimeInterval &ti ) const
	{ return !operator==(ti); }
	int contains( const tod_t t ) const
	{ return m_min <= t && t <= m_max; }
	int contains( const TimeInterval &ti ) const	// A time interval contains itself.
	{ return m_min <= ti.m_min && ti.m_max <= m_max; }
	int intersects( const TimeInterval &ti ) const
	{ return !(ti.m_max < m_min || m_max < ti.m_min); }
	tod_t overlap( const TimeInterval &ti ) const
	{
		return intersects(ti)
			? (m_max < ti.m_max ? m_max : ti.m_max) - (m_min > ti.m_min ? m_min : ti.m_min) + 1  // Compensate for the inclusive representation.
			: 0;
	}
	TimeInterval getIntersect( const TimeInterval &ti ) const
	{
		TimeInterval	i;
		if( intersects(ti) )
		{
			i.m_min = (m_min > ti.m_min ? m_min : ti.m_min);
			i.m_max = (m_max < ti.m_max ? m_max : ti.m_max);
		}
		return i;
	}
	int operator!() const	// returns True if the interval is empty.
	{ return m_min > m_max; }
	int operator()() const	// returns True if the interval is not empty.
	{ return m_min < m_max; }

	int operator<( const TimeInterval &t ) const { return m_max < t.m_min; }
	int operator>( const TimeInterval &t ) const { return t.m_max < m_min; }

	int operator<( const tod_t t ) const { return m_max < t; }
	int operator>( const tod_t t ) const { return t < m_min; }

	tod_t	tmin() const { return m_min; }
	tod_t	tmax() const { return m_max; }

	tod_t	duration() const { return m_max - m_min + 1; } // Compensate for the inclusive representation.

	void	set( const tod_t aMin, const tod_t aMax )
	{
		if( aMin <= aMax )  { m_min = aMin; m_max = aMax; }
		else				{ m_min = aMax; m_max = aMin; }
	}
private:
	tod_t	m_min, m_max;
};

#endif // TimeInterval_H
