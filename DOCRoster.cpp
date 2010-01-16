#include "DOCRoster.h"

std::ostream	&DOCweekPattern::printOn( std::ostream &os ) const
{
	for( register int d = 0; d < 7; ++d )
		os << ((pattern & (1<<d)) ? 'X' : '.');
	return os;
}

void	DOCweekPattern::setPattern( const int aPattern )
{
	pattern = (aPattern & 127);
	
	register	int	d, b;
	
	// Count the number of on days.
	for( onDays = 0, b = pattern; b != 0; b &= (b-1) )
		++onDays;
	
	// Check if there are consecutive days off in this pattern.
	consecutiveDaysOff = false;
	
	if( getOffDays() > 0 )
    {
		register int mask = (1<<getOffDays()) - 1;
		
		for( d = 0; d < 7; ++d )
		{
			// Test whether the days off are consecutive.
			if( !(mask & pattern) )
			{
				consecutiveDaysOff = true;
				break;
			}
			// Circularly shift the mask.
			if( (mask <<= 1) & 128 )
			{
				mask |= 1;
				mask &= 127;
			}
		}
    }
	
	// Compute a roster preference based on its attributes.
	preference = onDays * 16;
	if( getWeekendOff() && getConsecutiveDaysOff() )	preference += 8;
	else if(	getWeekendOff() )						preference += 4;
	else if(	getConsecutiveDaysOff() )				preference += 2;
	preference += getWeekendDayOff();
}

int DOCweekPatternCounts::setPatterns( const unsigned int daysOff,
									  const bool consecutiveDO,
									  const bool consecutiveIfAllWeekdayDO,
									  const bool extrasOnWeekends )
{
	const bool occurAnywhereDO = !(consecutiveDO || consecutiveIfAllWeekdayDO);
	
	reset();
	const DOCweekPatterns	wps;		// All possible weekday patterns.
	register int i, j = 0;
	for( i = 0; i < 128; ++i )
    {
		if( wps[i].getOffDays() != daysOff )
			continue;
		if( !occurAnywhereDO )
		{
			if( consecutiveDO && !wps[i].getConsecutiveDaysOff() )
				continue;
			if( consecutiveIfAllWeekdayDO && !wps[i].getConsecutiveIfAllWeekdayDaysOff() )
				continue;
		}
		wp[j++] = wps[i];
    }
	wp[j].setPattern(0);			// terminate the list of allowable patterns.
	
	if( extrasOnWeekends )
		std::sort( &wp[0], &wp[j], DOCweekendDayOffFirstCmp() );
	else
		std::sort( &wp[0], &wp[j], DOCweekendDayOffLastCmp() );
	
	return j;
}

void DOCweekPatternCounts::setExtrasOnWeekends( const bool extrasOnWeekends )
{
	register int j;
	for( j = 0; wp[j].getPattern(); ++j )
		continue;

	if( extrasOnWeekends )
		std::sort( &wp[0], &wp[j], DOCweekendDayOffFirstCmp() );
	else
		std::sort( &wp[0], &wp[j], DOCweekendDayOffLastCmp() );
}

void DOCweekPatternCounts::excludeDaysOffPattern( const unsigned int daysOffPattern )
{
	register int i, j;
	for( i = j = 0; wp[i]; ++i )
	{
		if( !wp[i].hasDaysOff(daysOffPattern) )
			wp[j++] = wp[i];
	}
	wp[j].setPattern(0);
}

#define	ForAllDays( d )		for( d = 0; d < 7; ++d )

int	DOCroster::getBest( SFrame &s, int *outWork, DOCweekPatternCounts &wpc )
{
	register	int	d;
	
	const	int	onDays = wpc[0].getOnDays();
	int	iBest = -1;
	register	double	value, valueBest = 0.0;
	for( register int i = 0; wpc[i].getPattern(); ++i )
    {
		if( s[i] )		// Skip previously used work patterns.
			continue;
		
		const	DOCweekPatternCount	&wpcCur = wpc[i];
		
		value = 0.0;
		ForAllDays( d )
		{
			if( outWork[d] > 0 )
			{
				if( wpcCur.isOn(d) )
					value += (double)outWork[d] * (double)outWork[d];
			}
			else
			{
				if( wpcCur.isOn(d) )
					break;
			}
		}
		
		// Skip invalid patterns.
		if( d != 7 )
			continue;
		
		if( value > valueBest )
		{
			iBest = i;
			valueBest = value;
		}
    }
	return iBest;
}

inline void	DOCroster::consume( int *outWork, DOCweekPatternCount &wpc )
{
	if( wpc.isOn(0) ) --outWork[0];
	if( wpc.isOn(1) ) --outWork[1];
	if( wpc.isOn(2) ) --outWork[2];
	if( wpc.isOn(3) ) --outWork[3];
	if( wpc.isOn(4) ) --outWork[4];
	if( wpc.isOn(5) ) --outWork[5];
	if( wpc.isOn(6) ) --outWork[6];
	++wpc;
}

inline void	DOCroster::release( int *outWork, DOCweekPatternCount &wpc )
{
	if( wpc.isOn(0) ) ++outWork[0];
	if( wpc.isOn(1) ) ++outWork[1];
	if( wpc.isOn(2) ) ++outWork[2];
	if( wpc.isOn(3) ) ++outWork[3];
	if( wpc.isOn(4) ) ++outWork[4];
	if( wpc.isOn(5) ) ++outWork[5];
	if( wpc.isOn(6) ) ++outWork[6];
	--wpc;
}

void	DOCroster::solve( int &workTotal, int &extraMax, int &extraTotal, DOCweekPatternCounts &wpc )
{
	// Find the best roster given the week patterns.
	int	backtrackCount = 0;
	const int backtrackCountMax = (1<<15);
	
	register int d, i;
	
	// Initialize the pattern counts.
	unsigned int onDaysMin = 7;
	for( i = 0; wpc[i].getPattern(); ++i )
	{
		wpc[i].set( 0 );
		if( wpc[i].getOnDays() < onDaysMin )
			onDaysMin = wpc[i].getOnDays();
	}
	if( i == 0 )
		return;	// Nothing to do.
	const	int	iPatternMax = i;
	
	// Keep track of the best week pattern count we have found so far.
	DOCweekPatternCounts	wpcBest;
	wpcBest.assign( wpc, iPatternMax );
	
	workTotal = 0;
	ForAllDays( d )
	{
		outWork[d] = inWork[d];
		workTotal += inWork[d];
	}
	
	int	remainingWork = workTotal, remainingWorkBest = workTotal, extraMaxBest = workTotal;
	
	// Declare a position stack so we can backtrack.
	// Remember: a stack frame can be empty, or an index to a pattern.
	SFrame	*stack = new SFrame[workTotal / onDaysMin + 2];
	int	iStack = 0;
	stack[0].reset();
	
	for( ;; )
    {
		// If this stack frame is not empty, release its current pattern before getting a new one.
		// This will only occur if we are backtracking to this frame.
		if( stack[iStack].valid() )
		{
			remainingWork += wpc[stack[iStack]()].getOnDays();
			release( outWork, wpc[stack[iStack]()] );
		}
		
		// Get the best pattern that has not been chosen before for this stack frame.
		const int iBest = getBest(stack[iStack], outWork, wpc);
		
		if( iBest < 0 )	// Check if a pattern was found.  If not:
		{
			if( iStack == 0 )	// Fathomed all possibilities - no solution found.
				break;
			
			// Backtrack.
			--iStack;
			if( ++backtrackCount > backtrackCountMax )
				break;
		}
		else			// We found a valid roster.
		{
			// Continue the search.
			stack[iStack] = iBest;						// Save this choice on the stack.
			consume( outWork, wpc[iBest] );				// Consume the work of this pattern.
			remainingWork -= wpc[iBest].getOnDays();	// Update the remaining work count.
			
			// Compute extraMax.
			extraMax = 0; ForAllDays( d ) { if( outWork[d] > extraMax ) extraMax = outWork[d]; }
			
			// Check if we need to save the best solution so far.
			if( remainingWork < remainingWorkBest ||
				(remainingWork == remainingWorkBest && extraMax < extraMaxBest) )
			{
				wpcBest.assign( wpc, iPatternMax );
				remainingWorkBest = remainingWork;
				extraMaxBest = extraMax;
				
				if( remainingWorkBest == 0 )	// Quit early if we find a perfect solution.
					break;
			}
			
			stack[++iStack].reset();	// Initialize the next stack frame to empty.
		}
    }
	
	// Free the committment stack.
	delete [] stack;
	
	// Restore the best solution we found.
	wpc.assign( wpcBest, iPatternMax );
	
	// Compute the extra stats.
	extraMax = 0;
	extraTotal = 0;
	ForAllDays( d )
		outWork[d] = inWork[d];
	
	for( i = 0; i < iPatternMax; ++i )
		if( wpc[i].getCount() > 0 )
		{
			ForAllDays( d )
				if( wpc[i].isOn(d) )
					outWork[d] -= wpc[i].getCount();
		}
		
	ForAllDays( d )
	{
		if( outWork[d] > extraMax )
			extraMax = outWork[d];
		extraTotal += outWork[d];
	}
}
