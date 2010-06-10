//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#pragma warning (disable: 4786 4290)

#include <fstream>
#include <strstream>
#include <string>
#include <stdlib.h>
#include <limits.h>
#include "TFormat.h"

using namespace std;

extern "C" {
#define externC
#include "TMSHeader.h"
#undef externC
}

#include "RosterImprove.h"

static const int DaysInWeek = 7;

static int  combinedIsValid(const COMBINEDDef &c)
{
  for(register int d = 0; d < DaysInWeek; d++)
  {
    if(c.RUNSrecordID[d] != NO_RECORD)
    {
      return TRUE;
    }
  }
  return FALSE;
}

static long getNumFrozen()
{
	register long numFrozen = 0L;
	
	register int i;
	for(i = 0; i < m_numCOMBINED; i++)
	{
		if(m_pCOMBINED[i].frozen)
			numFrozen++;
	}

	return numFrozen;
}

static inline int lt7(register int i)
{ return i >= 7 ? i - 7 : i; }

static int	ScoreCombined(const COMBINEDDef &c)
{
	register int d;
	for(d = 0; d < 7; d++)
		if(c.RUNSrecordID[(d-1) < 0 ? 6 : (d-1)] == NO_RECORD && c.RUNSrecordID[d] != NO_RECORD)
			break;
	const int dStart = d;

	int score = 0, cScore = 4;
	int prev = 0, cur;
	for(d = 0; d < 7; d++)
	{
		cur = (c.RUNSrecordID[lt7(d+dStart)] != NO_RECORD);
		if(prev && cur)
			cScore <<= 1;
		else if(prev && !cur)
		{
			score += cScore - 1;
			cScore = 4;
		}

		prev = cur;
	}

#ifdef WorkdayBias
	if(c.RUNSrecordID[4] == NO_RECORD)  // Friday off is good.
		score++;
#endif
	if(c.RUNSrecordID[5] == NO_RECORD)  // Saturday off is better.
		score += 2;
	if(c.RUNSrecordID[6] == NO_RECORD)  // Sunday off is best.
		score += 3;

	return score;
}

static int	CmpCombined(const void *v1, const void *v2)
{
	COMBINEDDef	&cd1 = *(COMBINEDDef *)v1;
	COMBINEDDef	&cd2 = *(COMBINEDDef *)v2;
	register int	j, d, c1 = 0, c2 = 0;

	// Compare by length of roster.
	for(j = 0; j < 7; j++)
	{
		if(cd1.RUNSrecordID[j] != NO_RECORD)
			c1++;
		if(cd2.RUNSrecordID[j] != NO_RECORD)
			c2++;
	}
	if(c1 != c2)
		return c2 - c1;

	// Compare the score of the roster.
	c1 = ScoreCombined(cd1);
	c2 = ScoreCombined(cd2);
	if(c1 != c2)
		return c2 - c1;

	// Compare by which days are worked first.
	const int startDay = 0;
	for(j = 0; j < 7; j++)
	{
		d = j + startDay;
		if(d >= 7)
			d -= 7;
		if((cd1.RUNSrecordID[d] != NO_RECORD) != (cd2.RUNSrecordID[d] != NO_RECORD))
		    return cd1.RUNSrecordID[d] != NO_RECORD ? 1 : -1;
	}

	// Compare by run ids starting at Monday.
	for(j = 0; j < 7; j++)
		if(cd1.RUNSrecordID[j] != NO_RECORD && cd1.RUNSrecordID[j] != cd2.RUNSrecordID[j])
			return cd1.RUNSrecordID[j] < cd2.RUNSrecordID[j] ? -1 : 1;

	return 0;
}

struct HashEntry
{
	HashEntry() : runID(NO_RECORD), flags(0), runIndex(-1), serviceIndex(-1) {}
	long	runID, flags;
	int		runIndex, serviceIndex;
};

static	int	hashSize = 0;
static	HashEntry *ht = NULL;

inline int	hashValue(register long runID, long flags)
{
	// Make sure to scramble the runID a little.
	return (int)((runID * 5 + flags == 0 ? 0 : 1) % hashSize);
}

static	void	IndexFromRunIDProlog()
{
  register int  nI;
  register int  nJ;
  register int	nK;

  hashSize = 0;
//
//  Determine the size of the hash table.
//
  nI = 0;
  while(m_pRRLIST[nI].SERVICESrecordID != NO_RECORD)
  {
	  hashSize += m_pRRLIST[nI].numRuns;
	  nI++;
  }
  hashSize = hashSize * 5 + 3;

// Make sure hashSize is a prime number.
  bool isPrime;
  do
  {
	  isPrime = true;
	  if((hashSize & 1) == 0)
		  hashSize++;
	  for(nJ = 3; nJ < (hashSize >> 1) - 1; nJ++)
		if(hashSize % nJ == 0)
	    {
		  hashSize += 2;
		  isPrime = false;
		  break;
	    }
  } while(!isPrime);

  // Allocate and initialize the hash table.
  ht = new HashEntry[hashSize];

  // Hash all the runID's into the table.
  nI = 0;
  while(m_pRRLIST[nI].SERVICESrecordID != NO_RECORD)
  {
	  for(nJ = 0; nJ < m_pRRLIST[nI].numRuns; nJ++)
	  {
		  nK = hashValue(m_pRRLIST[nI].pData[nJ].RUNSrecordID, m_pRRLIST[nI].pData[nJ].flags);
		  while(ht[nK].runID != NO_RECORD)
		  {
			  if(++nK == hashSize)
				  nK = 0;
		  }
		  ht[nK].runID = m_pRRLIST[nI].pData[nJ].RUNSrecordID;
		  ht[nK].flags = m_pRRLIST[nI].pData[nJ].flags;
		  ht[nK].runIndex = nJ;
		  ht[nK].serviceIndex = nI;
	  }
	  nI++;
  }
}

static int IndexFromRunID(int *serviceIndex, long runID, long flags)
{
    register int  nK = hashValue(runID, flags);
	//	while(ht[nK].runID != runID && ht[nK].runID != NO_RECORD)// && ht[nK].flags != flags)
	//		if(++nK == hashSize)
	//			nK = 0;
	while( ht[nK].runID != runID || ht[nK].flags != flags )
	{
//		if( ht[nK].runID == NO_RECORD )
//			break;

		if( ++nK == hashSize )
			nK = 0;
	}
	if(ht[nK].runID == NO_RECORD)
		return NO_RECORD;
	*serviceIndex = ht[nK].serviceIndex;
	return ht[nK].runIndex;
}

static	void	IndexFromRunIDEpilog()
{
	delete [] ht;
	ht = NULL;
	hashSize = 0;
}

static int		compressFixedRosters()
{
    // Fix up the m_pCOMBINED array to add in our new rosters.
    // First, compress all the fixed runs to the beginning.
    register int i, iDest = 0;
    for(i = 0; i < m_numCOMBINED; i++)
    {
        if(!combinedIsValid(m_pCOMBINED[i]))
            continue;
        if(m_pCOMBINED[i].frozen)
        {
            if(iDest != i)
                memcpy(&m_pCOMBINED[iDest], &m_pCOMBINED[i], sizeof(m_pCOMBINED[0]));
            iDest++;
        }
    }

	return iDest;
}

#ifdef NOT_NECESSARY
#include "crew.H"
#include "roster.H"
#include "workunit.H"
static	void	makeRosterQuickAndDirty(const int maxWorkDays)
{
    WorkUnitWeek        wuw(true);
    RosterWorkRunCmp    rosterWorkRunCmp;
    long                numFrozen = 0L;
    slRoster            longRosterList(&rosterWorkRunCmp, SL_UNIQUE);
    slRoster            shortRosterList(&rosterWorkRunCmp, SL_UNIQUE);
    slRoster            rosterList(&rosterWorkRunCmp, SL_UNIQUE);
    int                 dayOffCur = 0;

    // Get all the work that needs to be done on each day of the week.
    register int i, d;
    for(i = 0; i < m_numCOMBINED; i++)
    {
        if(m_pCOMBINED[i].frozen)
        {
            numFrozen++;
            continue;
        }
        if(!combinedIsValid(m_pCOMBINED[i]))
            continue;
        for(d = 0; d < DaysInWeek; d++)
        {
            if(m_pCOMBINED[i].RUNSrecordID[d] == NO_RECORD)
                continue;

            int runIndex, serviceIndex;
            runIndex = IndexFromRunID(&serviceIndex,
                  m_pCOMBINED[i].RUNSrecordID[d], m_pCOMBINED[i].flags[d]);

            WorkUnit *wu = new WorkUnit(m_pCOMBINED[i].RUNSrecordID[d],
                                         runIndex, serviceIndex);
            wuw.getWorkUnitsForDay(d).insert(wu);
        }
    }

    for(;;)
    {
        // Take a number of passes at combining rosters.
        for(;;)
        {
            // Attempt to combine as many runs into rosters as possible.
            // Find the first unallocated workday.
            for(d = 0; d < DaysInWeek; d++)
                if(wuw.getWorkUnitsForDay(d).entries() > 0)
                    break;
            // If there is no more work, we are done.
            if(d >= DaysInWeek)
                break;

            Roster *r = new Roster;
            WorkUnit *wuLast = wuw.removeFirst(d);
            WorkUnit *wuFirst = wuLast;
            r->setRun(d, wuLast);

            for(d++; d < DaysInWeek; d++)
            {
                if(wuw.getWorkUnitsForDay(d).entries() == 0)
                    continue;

                // Scan all the runs on the next day.
                // Combine with the lowest cost run.

                long costBest = LONG_MAX;
                slWorkUnitIter wuIter(&wuw.getWorkUnitsForDay(d));
                slRetStatus   ret;
                WorkUnit      *wu, *wuBest = NULL;
                for(ret = wuIter.first(&wu); ret == SL_OK; ret = wuIter.next(&wu))
                {
                    long costNew = RosterCoster(wuLast->getRunIndex(), wuLast->getServiceIndex(),
                                                 wu->getRunIndex(), wu->getServiceIndex());
                    if(costNew < 0)  // Ignore infeasible combinations.
                        continue;
                    if(RosterCoster(wu->getRunIndex(), wu->getServiceIndex(),
                                      wuFirst->getRunIndex(), wuFirst->getServiceIndex())
                                      < 0)
                        continue;
                    if(costBest > costNew)
                    {
                        costBest = costNew;
                        wuBest = wu;
                        if(costBest == 0L)  // Don't argue with perfection.
                            break;
                    }
                }
                if(wuBest != NULL)
                {
                    wuw.getWorkUnitsForDay(d).remove(wuBest);
                    r->setRun(d, wuBest);
                    wuLast = wuBest;
                }
            }

            // Put this combined roster on the appropriate list.
            const int workDays = r->workDays();
            if(workDays < maxWorkDays)
                shortRosterList.insert(r);
            else if(workDays > maxWorkDays)
                longRosterList.insert(r);
            else
                rosterList.insert(r);
        }

        if(longRosterList.entries() == 0)
        {
            // No more rosters can be combined.
            // Just put all the remaining short rosters on the valid
            // roster list.
            slRosterIter   ri(&shortRosterList);
            Roster         *r;
            while(ri.first(&r) == SL_OK)
            {
                shortRosterList.remove(r);
                rosterList.insert(r);
            }
            break;
        }

        // Cyclically assign holidays to all the long rosters.
        slRosterIter    riLong(&longRosterList);
        Roster          *r;
        while(riLong.first(&r) == SL_OK)
        {
            // Take this run off the long list.
            longRosterList.remove(r);

            // Add holidays to the run based on the current holiday.
            // This code will handle any combination of workdays
            // and day off allocations.
            const int workDays = r->workDays();
            for(int off = workDays - maxWorkDays; off > 0; off--)
            {
                // Find a working day in this roster.
                while(r->isHoliday(dayOffCur))
                    dayOffCur = (dayOffCur + 1) % DaysInWeek;

                // Put the run on this day back on the list of
                // available work and mark this day as a holiday
                // in the current roster.
                wuw.getWorkUnitsForDay(dayOffCur).insert(r->getRun(dayOffCur));
                r->setRun(dayOffCur, NULL);

                dayOffCur = (dayOffCur + 1) % DaysInWeek;
            }

            // Put the fixed-up valid run on the valid list.
            rosterList.insert(r);
        }

        // Throw all the short rosters back into the pool for the
        // next pass.
        slRosterIter    riShort(&shortRosterList);
        while(riShort.first(&r) == SL_OK)
        {
            shortRosterList.remove(r);
            for(int d = 0; d < DaysInWeek; d++)
                if(r->getRun(d) != NULL)
                   wuw.getWorkUnitsForDay(d).insert(r->getRun(d));
            delete r;
        }
    }

    Assert(shortRosterList.entries() == 0);
    Assert(longRosterList.entries() == 0);

    // Fix up the COMBINED array to add in our new rosters.
    // First, compress all the fixed runs to the beginning.
    long startNumCombined = m_numCOMBINED = compressFixedRosters();

    // Initialize a histogram of workdays.
    int hist[7];
    for(d = 0; d < DaysInWeek; d++)
        hist[d] = 0;

    // Copy the newly combined rosters into the COMBINED array.
    slRosterIter   riValid(&rosterList);
    Roster         *r;
    while(riValid.first(&r) == SL_OK)
    {
      if(m_numCOMBINED >= m_maxCOMBINED)
      {
        m_maxCOMBINED += 512;
        m_pCOMBINED = (COMBINEDDef *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, m_pCOMBINED, sizeof(COMBINEDDef) * m_maxCOMBINED); 
        if(m_pCOMBINED == NULL)
        {
          AllocationError(__FILE__, __LINE__, TRUE);
          break;
        }
      }
    		COMBINEDDef &c = m_pCOMBINED[m_numCOMBINED++];
        c.frozen = TRUE;
        for(d = 0; d < DaysInWeek; d++)
            c.RUNSrecordID[d] = (r->isWorking(d) ? r->getRunID(d) : NO_RECORD);

        // Update the histogram counters.
        hist[r->workDays()]++;

        // Delete this roster.
        rosterList.remove(r);
        delete r;
    }

	// Sort the new rosters.
	qsort(&m_pCOMBINED[startNumCombined], m_numCOMBINED - startNumCombined, sizeof(m_pCOMBINED[0]), CmpCombined);
	for(; startNumCombined < m_numCOMBINED; startNumCombined++)
		m_pCOMBINED[startNumCombined].rosterNumber = GlobalRosterNumber++;
}
#endif // NOT_NECESSARY

#include <fstream>
#include <iomanip>
static	void	makeRosterReport( const int maxWorkDays, const long minPaidWork, const long maxPaidWork )
{
	static int count = 0;
	char	szFile[128];
	sprintf( szFile, "c:\\RosterReport_%d.txt", ++count );
	std::ofstream	ofs(szFile);

	long	workUnderTotal = 0, workOverTotal = 0;
	long	workMinReserve = 0, workMaxReserve = 0;

    register int i, d;
    for(i = 0; i < m_numCOMBINED; i++)
    {
        if(!combinedIsValid(m_pCOMBINED[i]))
            continue;

		ofs << std::setw(4) << m_pCOMBINED[i].rosterNumber << ' ';
		ofs << (m_pCOMBINED[i].frozen ? "Frozen   " : "Unfrozen ");
		long	costTotal = 0L, paidWorkTotal = 0L;
		bool	feasible = true;
		int		workDays = 0;
        for(d = 0; d < DaysInWeek; d++)
        {
            if(m_pCOMBINED[i].RUNSrecordID[d] == NO_RECORD)
			{
				ofs << " + XXXX";
                continue;
			}
			++workDays;

			int dFrom = (d == 0 ? 6 : d - 1);
			while( m_pCOMBINED[i].RUNSrecordID[dFrom] == NO_RECORD )
				dFrom = (dFrom == 0 ? 6 : dFrom - 1);

            int runIndexFrom, serviceIndexFrom;
            runIndexFrom = IndexFromRunID(&serviceIndexFrom, m_pCOMBINED[i].RUNSrecordID[dFrom], m_pCOMBINED[i].flags[dFrom]);

            int runIndexTo, serviceIndexTo;
            runIndexTo = IndexFromRunID(&serviceIndexTo, m_pCOMBINED[i].RUNSrecordID[d], m_pCOMBINED[i].flags[d]);

			paidWorkTotal += m_pRRLIST[serviceIndexTo].pData[runIndexTo].payTime;

			long cost;
			if( ((dFrom + 1) % 7) == d )
				cost = RosterCoster( runIndexFrom, serviceIndexFrom, runIndexTo, serviceIndexTo );
			else
				cost = RosterCosterNonConsecutive( runIndexFrom, serviceIndexFrom, runIndexTo, serviceIndexTo );

			if( cost == NO_RECORD )
				feasible = false;
			else
				costTotal += cost;

			ofs << (cost == NO_RECORD ? " - " : " + ") << std::setw(4) << m_pCOMBINED[i].RUNSrecordID[d];
        }

		if( workDays != maxWorkDays )
		{
			ofs << " Infeasible Work Days";
		}
		else if( feasible )
		{
			ofs << " cost=" << std::setw(5) << costTotal << " " << (const char *)tFormat(paidWorkTotal);
			if( paidWorkTotal < minPaidWork )
			{
				ofs << " SHORT=" << (const char *)tFormat(minPaidWork - paidWorkTotal);
				workUnderTotal += minPaidWork - paidWorkTotal;
			}
			else if( paidWorkTotal > maxPaidWork )
			{
				ofs << " LONG=" << (const char *)tFormat(paidWorkTotal - maxPaidWork);
				workOverTotal += paidWorkTotal - maxPaidWork;
			}
			else
			{
				workMinReserve += paidWorkTotal - minPaidWork;
				workMaxReserve += maxPaidWork - paidWorkTotal;
			}
		}
		else
			ofs << " Infeasible Run Combination";

		ofs << std::endl;
    }

	ofs << std::endl;
	ofs << "workUnderTotal=" << (const char *)tFormat(workUnderTotal) << std::endl;
	ofs << "workMinReserve=" << (const char *)tFormat(workMinReserve) << std::endl;
	ofs << std::endl;
	ofs << "workOverTotal=" << (const char *)tFormat(workOverTotal) << std::endl;
	ofs << "workMaxReserve=" << (const char *)tFormat(workMaxReserve) << std::endl;
}

#define encodeRunIndexServiceIndex(r, s)		((r) * servicesMax + (s))
#define decodeRunIndex(c)						((c) / servicesMax)
#define decodeServiceIndex(c)					((c) % servicesMax)

static	void	makeRosterSmartSubproblem(const int maxWorkDays, const unsigned int *patterns, const bool preferExtrasOnWeekend = false,
										const bool equalize = false,
										const long minWorkSeconds = 0, const long maxWorkSeconds = 200L*24*60*60,
										const int maxNightWork = 7,
										const bool includeInfeasible = true )
{
	const long	servicesMax = SERVICES.number + 1;

	RosterImprove		r;

	// Pass in the work patterns.
	{
		for( register unsigned int p = 0; p < 128 && patterns[p] != 0; ++p )
			r.setAllowedWorkPattern( patterns[p] );
	}
	r.setMinMaxPaidWork( minWorkSeconds, maxWorkSeconds );
	r.setEqualize( equalize );
	r.setMaxNightWork( maxNightWork );

    long                numFrozen = 0L;

    // Get all the work that needs to be done on each day of the week.
    register int i, d;
    for(i = 0; i < m_numCOMBINED; i++)
    {
        if(m_pCOMBINED[i].frozen)
        {
            numFrozen++;
            continue;
        }
        if(!combinedIsValid(m_pCOMBINED[i]))
            continue;
        for(d = 0; d < DaysInWeek; d++)
        {
            if(m_pCOMBINED[i].RUNSrecordID[d] == NO_RECORD)
                continue;

            int runIndex, serviceIndex;
            runIndex = IndexFromRunID(&serviceIndex, m_pCOMBINED[i].RUNSrecordID[d], m_pCOMBINED[i].flags[d]);

			// Check if night work.
			bool isNightWork = false;
			if( ROSTERPARMS.nightWorkAfter > 0 )
			{
        if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_STARTSAFTER)
        {
  				if( m_pRRLIST[serviceIndex].pData[runIndex].onTime > ROSTERPARMS.nightWorkAfter )
  					isNightWork = true;
        }
        else
        {
  				if( m_pRRLIST[serviceIndex].pData[runIndex].offTime > ROSTERPARMS.nightWorkAfter )
  					isNightWork = true;
        }
			}
			r.newWork( (RosterImprove::DayOfWeek)d,
						encodeRunIndexServiceIndex(runIndex, serviceIndex),
						m_pRRLIST[serviceIndex].pData[runIndex].payTime,
						isNightWork );
        }
    }

	// Create all the possible run combination connections.
	for( d = 0; d < 7; ++d )
	{
		RosterImprove::WorkIDVector	work;
		r.getWork( (RosterImprove::DayOfWeek)d, work );
		for( register RosterImprove::WorkIDVector::iterator w1 = work.begin(), w1End = work.end(); w1 != w1End; ++w1 )
		{
			RosterImprove::WorkIDVector	work2;

			// Create the connections to the next consecutive day.
			register int d2 = d + 1; if( d2 >= 7 ) d2 -= 7;
			r.getWork( (RosterImprove::DayOfWeek)d2, work2 );
			for( register RosterImprove::WorkIDVector::iterator w2 = work2.begin(), w2End = work2.end(); w2 != w2End; ++w2 )
			{
				const long cost = RosterCoster( decodeRunIndex(*w1), decodeServiceIndex(*w1),
												decodeRunIndex(*w2), decodeServiceIndex(*w2) );
				if( cost != NO_RECORD )
					r.newConnection( (RosterImprove::DayOfWeek)d, *w1, (RosterImprove::DayOfWeek)d2, *w2, cost );
			}

			// Create the connections to the remaining nonconsecutive days.
			const int dOffsetMax = 7 - maxWorkDays + 1;
			for( register int dOffset = 2; dOffset <= dOffsetMax; ++dOffset )
			{
				d2 = d + dOffset; if( d2 >= 7 ) d2 -= 7;
				r.getWork( (RosterImprove::DayOfWeek)d2, work2 );
				for( register RosterImprove::WorkIDVector::iterator w2 = work2.begin(), w2End = work2.end(); w2 != w2End; ++w2 )
				{
					const long cost = RosterCosterNonConsecutive(decodeRunIndex(*w1), decodeServiceIndex(*w1),
																 decodeRunIndex(*w2), decodeServiceIndex(*w2) );
					if( cost != NO_RECORD )
						r.newConnection( (RosterImprove::DayOfWeek)d, *w1, (RosterImprove::DayOfWeek)d2, *w2, cost );
				}
			}
		}
	}

//	{
//		{
//			std::ofstream	ofs("c:\\RosterImproveData.txt");
//			r.write( ofs );
//		}
//	}

	r.solve();

    // Fix up the COMBINED array to add in our new rosters.
    // First, compress all the fixed rosters to the beginning.
	long startNumCombined = m_numCOMBINED = compressFixedRosters();

	// Then, copy in the new optimized rosters.
	const RosterImprove::RosterVec	&rosters = r.getRosters();
	for( i = 0; i < rosters.size(); ++i )
	{
    if(m_numCOMBINED >= m_maxCOMBINED)
    {
      m_maxCOMBINED += 512;
      m_pCOMBINED = (COMBINEDDef *)HeapReAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, m_pCOMBINED, sizeof(COMBINEDDef) * m_maxCOMBINED); 
      if(m_pCOMBINED == NULL)
      {
        AllocationError(__FILE__, __LINE__, TRUE);
        break;
      }
    }
		COMBINEDDef &c = m_pCOMBINED[m_numCOMBINED++];
		c.frozen = (includeInfeasible || rosters[i]->isFeasible() ? TRUE : FALSE);
		c.rosterNumber = NO_RECORD;

        for(d = 0; d < DaysInWeek; ++d)
        {
			RosterImprove::Work *work = rosters[i]->work[d];
			c.RUNSrecordID[d] = (work ? m_pRRLIST[decodeServiceIndex(work->id)].pData[decodeRunIndex(work->id)].RUNSrecordID : NO_RECORD);
			c.flags[d] = (work ? m_pRRLIST[decodeServiceIndex(work->id)].pData[decodeRunIndex(work->id)].flags : 0);
        }
        c.DRIVERSrecordID = NO_RECORD;
        c.COMMENTSrecordID = NO_RECORD;

		// Set the flags on the roster to enable better analysis.
		// c.flags = 0;
		// if( rosters[i]->getViolatesMinPaidWork() )
		// 	c.flags |= AAA;
		// if( rosters[i]->getViolatesMaxPaidWork() )
		// 	c.flags |= BBB;
		// if( rosters[i]->getViolatesWorkDays() )
		// 	c.flags |= CCC;
		// if( rosters[i]->getViolatesPatterns() )
		// 	c.flags |= DDD;
		// if( rosters[i]->getContainsInvalidCombinations() )
		// 	c.flags |= EEE;		
	}

	// Sort the new rosters.
	qsort(&m_pCOMBINED[startNumCombined], m_numCOMBINED - startNumCombined, sizeof(m_pCOMBINED[0]), CmpCombined);
	for(; startNumCombined < m_numCOMBINED; startNumCombined++)
	{
		if( m_pCOMBINED[startNumCombined].frozen )
			m_pCOMBINED[startNumCombined].rosterNumber = GlobalRosterNumber++;
	}
}

static	void	makeRosterSmart(const int maxWorkDays, const unsigned int *patterns, const bool preferExtrasOnWeekend = false,
								const bool equalize = false,
								const long minWorkSeconds = 0, const long maxWorkSeconds = 200L*24*60*60,
								const int maxNightWork = 7 )
{
    long	numFrozen = getNumFrozen(), numFrozenLast;

	do
	{
		numFrozenLast = numFrozen;
		makeRosterSmartSubproblem( maxWorkDays, patterns, preferExtrasOnWeekend, equalize, minWorkSeconds, maxWorkSeconds, maxNightWork, false );
	} while( (numFrozen = getNumFrozen()) != numFrozenLast );

	// Make sure the following line is uncommented if you want infeasible rosters created.
//	makeRosterSmartSubproblem( maxWorkDays, patterns, preferExtrasOnWeekend, equalize, minWorkSeconds, maxWorkSeconds, true );
}

extern "C" {

void makeRoster(int maxWorkDays, unsigned int *patterns, BOOL preferExtrasOnWeekends, BOOL equalizeWork, long minWorkSeconds, long maxWorkSeconds, int maxNightWork )
{
	IndexFromRunIDProlog();

	// Use all patterns if we are passed nothing.
	if( !*patterns )
	{
		unsigned int *pCur = patterns;
		for( unsigned int p = 0; p < 128; ++p )
			if( bitCount(p) == maxWorkDays )
				*pCur++ = p;
		*pCur = 0;
	}

//	makeRosterReport( maxWorkDays, minWorkSeconds, maxWorkSeconds );
	makeRosterSmart( maxWorkDays, patterns, preferExtrasOnWeekends != 0, equalizeWork != 0, minWorkSeconds, maxWorkSeconds, maxNightWork );
//	makeRosterReport( maxWorkDays, minWorkSeconds, maxWorkSeconds );

	IndexFromRunIDEpilog();
}

void SortRenumberRosters()
{
	qsort(m_pCOMBINED, m_numCOMBINED, sizeof(m_pCOMBINED[0]), CmpCombined);
	for(GlobalRosterNumber = 1; GlobalRosterNumber <= m_numCOMBINED; GlobalRosterNumber++)
		(m_pCOMBINED-1)[GlobalRosterNumber].rosterNumber = GlobalRosterNumber;
}

} // extern "C"
