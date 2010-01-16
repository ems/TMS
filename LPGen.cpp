//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

//#define WRITE_LP
//#define PRINT_MIN_WORK
//#define PRINT_BLOCK_INFO
//#define PRINT_GET_COST_STATUS
//#define PRINT_RUN_PATTERN_TOTAL
//#define PRINT_RUN_PATTERNS

extern "C"
{
#include <TMSHeader.h>
#include <lpkit.h>
}

#ifndef Min
#define Min(a,b)        (((a)<(b))?(a):(b))
#endif
#ifndef Max
#define Max(a,b)        (((a)>(b))?(a):(b))
#endif

#include <cover.h>
#include <lpgen.H>
#include <math.h>
#include <iomanip>

using namespace std;

LPGen::LPGen()
{
    rdHead = rdTail = NULL;
    rbHead = rbTail = NULL;

    chunkIndex = NULL;
    chunkElements = NULL;
    chunkIndexSize = 0;

    tShortest = -1;
    tLongest  = -1;

    minDrivers = 0.0;
    maxDrivers = 50000.0;
}

LPGen::~LPGen()
{
    // Delete the chunk index structure.
    for( long i = 0; i < chunkIndexSize; i++ )
        delete [] chunkIndex[i];
    delete [] chunkIndex;
    delete [] chunkElements;

    // Delete the run definitions.
    RunDef	*rd, *rdNext;
    for( rd = rdHead; rd != NULL; rd = rdNext )
    {
	rdNext = rd->next;
	delete rd;
    }

    // Delete the known blocks.
    RBlock	*rb, *rbNext;
    for( rb = rbHead; rb != NULL; rb = rbNext )
    {
	rbNext = rb->next;
	delete rb;
    }
}

RunChunk *LPGen::find( tod_t chunkStart, tod_t chunkLength ) const
{
    // Assume here that start and length are already correct multiples.

    // First check if the length is in range.
    if( chunkLength < tShortest || chunkLength > tLongest )
        return NULL;

    // Use binary search to find the chunk in the list.
    RunChunk search( 1, chunkStart, chunkLength );
    const long i = (chunkStart - start) / multiple;
    if( i < 0 || i >= chunkIndexSize )
        return NULL;
    RunChunk **chunks = chunkIndex[i];
    long  left  = 0;
    long  right = chunkElements[i] - 1;
    long  middle;

    while( left < right )
    {
	middle = (left + right) / 2;
	if( *chunks[middle] < search )
	    left = middle + 1;
	else
	    right = middle;
    }
    if( left < chunkElements[i] && *chunks[left] == search )
        return chunks[left];
    return NULL;
}

double LPGen::getCost( tod_t start, tod_t end ) const
{
    start = nearestMult(start, multiple);
    end   = nearestMult(end,   multiple);

    double	workCost;
    RunChunk    *found = find(start, end - start);
    if( found != NULL )
    {
	    workCost = found->workCount;

	    // Scan through all the equivalent chunks and sum the work.
	    while( (found+1) != NULL && *found == *(found+1) )
	        workCost += (++found)->workCount;
	    workCost = -2.0 * workCost + 1.0;
    }
    else
	workCost = INFINITY;

#ifdef PRINT_GET_COST_STATUS
    cout << "cost: " << asTime(start) << '\t';
    cout << asTime(end) << '\t';
    cout << asTime(end - start) << '\t';
    cout << workCost << '\n';
#endif
    return workCost;
}

void LPGen::decCount( RunChunk *&found, tod_t start, tod_t end )
{
    start = nearestMult(start, multiple);
    end   = nearestMult(end,   multiple);

    found = find(start, end-start);
    if( !found )
	return;

    // Try to find the best chunk to decrement.
    double bestMax = found->workCount;

    // Scan through all the equivalent chunks.
    RunChunk *foundCur = found;
    while( (foundCur + 1) != NULL && *foundCur == *(foundCur+1) )
	if( (++foundCur)->workCount > bestMax )
	    bestMax = (found = foundCur)->workCount;

    // Consume the best one we found.
    found->workCount -= 1.0;
}

static	int	cmpChunks( const void *v1, const void *v2 )
{
    const RunChunk *rc1 = *((const RunChunk * const *)v1);
    const RunChunk *rc2 = *((const RunChunk * const *)v2);

    return *rc1 < *rc2 ? -1
	:  *rc1 > *rc2 ?  1
	: 0;
}

class blockStartPair
{
friend class LPGen;

    blockStartPair() { n = 0; row = -1; }

    size_t	n;		// number of blocks starting at this time.
    int		row;
};

inline void set_rh_permuted( lprec *lp, int row, double value )
{
    set_rh( lp, row, value - 1.0/200.0 + drand48()/100.0 );
}

class PQPair
{
friend class LPGen;

    PQPair() { start = 0; end = 0; n = 0; }

    tod_t	start, end;
    size_t	n;		// number of small pq pairs that must exist at this time.
    int		row;
};

long LPGen::getNumPatterns( tod_t aStart, tod_t aEnd, const tod_t aMultiple )
{
    // Initialize
    multiple = aMultiple;

    start            = lesserMult( aStart, multiple );
    end              = greaterMult(  aEnd, multiple );

    //--------------------------------------------------------------------
    // Count all the run patterns for the definitions.
    RunDef	*rd;
    long	totalPatterns = 0;

    // Set the count-only flag.
    RunDef::countOnly = 1;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
        // Only generate patterns for up to 2 piece runs - try to match
        // the multi-piece runs later.
        if( rd->getNumWorking() <= 2 )
  	         totalPatterns += rd->generatePatterns( start, end, multiple );
    }
    // Clear the count-only flag.
    RunDef::countOnly = 0;
    return totalPatterns;
}

static	void	get_rhs( lprec *lp, REAL *rhsRet )
{
	register int row;
	rhsRet[0] = 0;
	if(lp->scaling_used)
		for( row = 1; row <= lp->rows; row++ )
			if(lp->ch_sign[row])
				rhsRet[row] = -lp->orig_rh[row] / lp->scale[row];
			else
				rhsRet[row] = lp->orig_rh[row] / lp->scale[row];
	else
		for( row = 1; row <= lp->rows; row++ )
			if(lp->ch_sign[row])
				rhsRet[row] = -lp->orig_rh[row];
			else
				rhsRet[row] = lp->orig_rh[row];
}

static int coverSolve( lprec *lp, const int tiMin )
{
	Cover	cover( lp->columns+1, tiMin+1 );
	REAL	*col = new REAL[lp->rows+1];
	unsigned int rhsFirst, rhsLast;
	register unsigned int r, c;

	// First, find the ranges of the non-zero rhs.
	get_rhs( lp, col );
	for( rhsFirst = 1; rhsFirst < (unsigned int)tiMin; rhsFirst++ )
		if( (unsigned int)(col[rhsFirst] + 0.5) != 0 )
			break;
	for( rhsLast = (unsigned int)tiMin-1; rhsLast > 1; rhsLast-- )
		if( (unsigned int)(col[rhsLast] + 0.5) != 0 )
			break;

	// Populate the cover with the rhs, costs and matrix.
	for( r = rhsFirst; r <= rhsLast; r++ )
		cover.addRow( r, (unsigned int)(col[r] + 0.5) );

	for( c = 1; c <= (unsigned int)lp->columns; c++ )
	{
		get_column( lp, c, col );
		cover.addColumn( c, (unsigned int)(col[0] + 0.5) );
		for( r = rhsFirst; r <= rhsLast; r++ )
			if( col[r] != 0.0 )
				cover.addMatrix( c, r );
	}

	// Solve the cover.
	const int status = cover.solve() ? INFEASIBLE : OPTIMAL;
	if( status == OPTIMAL )
	{
		// Install the solution back into the lp.
		for( c = 1; c <= (unsigned int)lp->columns; c++ )
			get_x(lp, c) = (REAL)cover.getValue(c);
		get_obj_val(lp) = (REAL)cover.getCost();
	}

	// Cleanup
	delete [] col;

	return status;
}

void LPGen::solveLP( tod_t aStart, tod_t aEnd,
		     const tod_t aMinLeftoverPiece,
		     const tod_t aMultiple )
{
    // Initialize
    multiple = aMultiple;

    start            = lesserMult( aStart, multiple );
    end              = greaterMult(  aEnd, multiple );
    minLeftoverPieceSave = aMinLeftoverPiece;
    minLeftoverPiece = greaterMult( aMinLeftoverPiece, multiple );

    if( rbHead == NULL || rdHead == NULL )
	    return;

    tod_t t;

#define ti(t)	((int)(((t) - start) / multiple + 1))
    const int tiMin = ti(end);
    const int tiMax = tiMin + 1;
    
    //--------------------------------------------------------------------
    // Count the number of unique block starting times.
    const int tiBlockStartC = tiMax + 1;
    RBlock	*rb;
    const tod_t blockHorizon = 48L * 60L * 60L;
    blockStartPair	*bsPair = new blockStartPair[blockHorizon / multiple + 1];
    for( rb = rbHead; rb != NULL; rb = rb->next )
    {
	    const tod_t  tStart = Max(nearestMult(rb->head->getT(), multiple), start);
	    blockStartPair *bs = &bsPair[tStart / multiple];
	    bs->n++;
    }
    // Assign a matrix row number of this starting blocks constraint.
    int bsCount = 0;
    for( t = 0; t < blockHorizon; t += multiple )
	    if( bsPair[t / multiple].n > 0 )
	    {
	        blockStartPair *bs = &bsPair[t / multiple];
	        bs->row = tiBlockStartC + bsCount;
	        bsCount++;
	    }

    //--------------------------------------------------------------------
    // Count the number of possible small blocks from p to q.
    const int tiPQStartC = tiBlockStartC + bsCount;
    int		pqC = 0;
    for( t = start; t < end; t += multiple )
    {
	    tod_t	v;
	    for( v = t + multiple;
	         v < end && v - t < minLeftoverPiece;
	         v += multiple )
	        pqC++;
    }
    const int pqCount = pqC;
    pqC = 0;
    PQPair	*pqPair = new PQPair[pqCount];
    for( t = start; t < end; t += multiple )
    {
		tod_t	v;
	    for( v = t + multiple;
	         v < end && v - t < minLeftoverPiece;
	         v += multiple )
	    {
	        PQPair	*pq = &pqPair[pqC];
	        pq->start = t;
	        pq->end = v;
	        pq->row = tiPQStartC + pqC;
	        pqC++;
	    }
    }

    const int rows = tiPQStartC + pqCount;

    lprec *lp = make_lp( rows-1, 0 );
    set_minim( lp );

    // add 1 for the cost (row 0)
#if !defined(__WIN32__)
    REAL huge	*colBuf = (REAL huge *)new REAL [rows + 1];
#else
    REAL *colBuf = (REAL *)new REAL [rows + 1];
#endif

    //--------------------------------------------------------------------
    // Collect the work requirments for each block.
    unsigned long r;
    for( r = 0; r <= (unsigned long)rows; r++ )
	    colBuf[r] = 0.0;
    for( rb = rbHead; rb != NULL; rb = rb->next )
    {
	    const tod_t  tStart = Max(nearestMult(rb->head->getT(), multiple), start);
	    const tod_t  tEnd   = Min(nearestMult(rb->tail->getT(), multiple), end);
	    for( t = tStart; t < tEnd; t += multiple )
	        colBuf[ti(t)]++;
    }
    // Set the minimum work constraints.
#ifdef PRINT_MIN_WORK
    cout << "Minimum work requirements by time of day\n";
#endif
    for( t = start; t < end; t += multiple )
    {
	    set_rh_permuted( lp, ti(t), colBuf[ti(t)] );
	    set_constr_type( lp, ti(t), GE );
#ifdef PRINT_MIN_WORK
	    cout << asTime(t) << '\t' << colBuf[ti(t)] << '\n';
#endif
#ifdef WRITE_LP
	    char rName[25];
	    sprintf( rName, "mw%d", t );
	    set_row_name( lp, ti(t), rName );
#endif
    }
    // Set the max/min available drivers constraints.
    set_rh_permuted( lp, tiMin, minDrivers );
    set_constr_type( lp, tiMin, GE );
    set_rh_permuted( lp, tiMax, maxDrivers );
    set_constr_type( lp, tiMax, LE );
#ifdef WRITE_LP
    set_row_name( lp, tiMax, "maxDrivers" );
    set_row_name( lp, tiMin, "minDrivers" );
#endif

    //--------------------------------------------------------------------
    // Set the "start of each block" constraints.
    // These ensure that the number of pieces starting at any
    // one point in time is GE the number of blocks starting
    // at that time.
    for( t = 0; t < blockHorizon; t += multiple )
	if( bsPair[t / multiple].n > 0 )
	{
	    blockStartPair *bs = &bsPair[t / multiple];
	    set_rh_permuted( lp, bs->row, bs->n );
	    set_constr_type( lp, bs->row, GE );

#ifdef WRITE_LP
	    char rName[25];
	    sprintf( rName, "stb%d", t );
	    set_row_name( lp, bs->row, rName );
#endif
	}

    //--------------------------------------------------------------------
    // Set the small block constraints.
    // First, collect the number of small blocks between the pq times of the day.
    if( pqCount > 0 )
    {
	    for( rb = rbHead; rb != NULL; rb = rb->next )
	    {
	        const tod_t  tStart = Max(nearestMult(rb->head->getT(), multiple), start);
	        const tod_t  tEnd   = Min(nearestMult(rb->tail->getT(), multiple), end);
	        for( pqC = (int)(((tStart-pqPair[0].start) / multiple)
			         * (minLeftoverPiece / multiple - 1));
				 pqC < pqCount && pqPair[pqC].end <= tEnd;
				 pqC++ )
				pqPair[pqC].n++;
	    }
	    // Now set the RHS requirements.
	    for( pqC = 0; pqC < pqCount; pqC++ )
	    {
	        PQPair	*pq = &pqPair[pqC];
	        set_rh_permuted( lp, pq->row, pq->n );
	        set_constr_type( lp, pq->row, GE );

#ifdef WRITE_LP
	        char rName[25];
	        sprintf( rName, "sb%d", pqC );
	        set_row_name( lp, pq->row, rName );
#endif
	      }
    }
    
    //--------------------------------------------------------------------
    // Generate all the run patterns for the definitions.
    // Doing this here avoids lots of memory fragmentation.
    RunDef	*rd;
    size_t	totalPatterns = 0;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
        totalPatterns += rd->generatePatterns( start, end, multiple );
#ifdef PRINT_RUN_PATTERNS
	      cout << *rd;
#endif
    }
#ifdef PRINT_RUN_PATTERN_TOTAL
    cerr << "Generated " << totalPatterns << " patterns in total.\n";
#endif

    // Fill in the contributions of all the possible shifts.
    size_t	id = 0;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
		RunPattern	*rp;
		for( rp = rd->pHead; rp != NULL; rp = rp->getNext() )
		{
			RunChunk	*rc;

			rp->id = id++;

			//--------------------------------------------------------
			IdealRunPiece pieces[16];
			int p = 0;
			for( rc = rp->head; rc != NULL; rc = rc->next )
			{
				pieces[p].start = rc->start;
				pieces[p].length = rc->length;
				p++;
			}
			double patternCost;
			if( rp->owner->isPenalty )
				patternCost = 10L*60L*60L;
			else
			{
				patternCost = IdealRunCoster(rp->owner->runType, pieces, p);
				if( patternCost < 0.0 )
					patternCost = INFINITY;
			}

			// Permute the cost slighly to avoid degeneracy.
			patternCost += drand48() / 100.0 - 1.0/200.0;

			colBuf[0] = patternCost;

			// Initialize the constraint column for this variable.
			for( r = 1; r < (unsigned long)rows; r++ )
	    		colBuf[r] = 0.0;

			for( rc = rp->head; rc != NULL; rc = rc->next )
			{
				// Fill in the contribution for each working chunk.
				const tod_t tStart = nearestMult( rc->start, multiple );
				const tod_t tEnd   = nearestMult( rc->start + rc->length, multiple );
				for( t = tStart > start ? tStart : start;
					 t < tEnd && t < end;
					 t += multiple )
					colBuf[ti(t)] = 1.0;

				// Add a contribution if this chunk starts at the same time
				// as a block.
				if( bsPair[tStart / multiple].n > 0 )
					colBuf[bsPair[tStart / multiple].row] = 1.0;

				// Add the contribution of this chunk to all the pq small blocks
				if( pqCount > 0 )
				{
					for( pqC = (int)(((tStart-pqPair[0].start) / multiple)
							 * (minLeftoverPiece / multiple - 1));
						 pqC < pqCount && pqPair[pqC].end <= tEnd;
						 pqC++ )
						colBuf[pqPair[pqC].row] = 1.0;
				}
			}
			// Add the max and min contribution.
			colBuf[tiMin] = 1.0;
			colBuf[tiMax] = 1.0;

			add_column( lp, colBuf );
		}
    }

    // Cleanup the lp generation stuff.
    delete [] bsPair;
    delete [] pqPair;
    delete [] colBuf; colBuf = NULL;

#if !defined(WRITE_LP)
    FILE *pf = fopen("runcut.lp", "w");
    write_LP( lp, pf );
    fclose( pf );
#endif

    // Solve the lp - (this may take awhile...)
    auto_scale( lp );
    lp->trace = 0;
    const int solveStat = /* ::solve(lp); */ coverSolve(lp, tiMin);
    if( solveStat != OPTIMAL )
    {
	    // something went wrong!!!
	    // (most likely an
	    // Clean up the lp.
	    delete_lp( lp );
	    lp = NULL;

	    // Set the cost to infinity.
	    idealRuncutCost = INFINITY;
	    return;
    }

#ifdef MAKE_INTEGER
    // Go for an integer solution for the amgibuous values.
    for( ;; )
    {
	    // First, find the biggest fractional value.
	    long idBest = -1;
	    long valueBest = -1;
	    for( id = 1; id <= lp->columns; id++ )
	        if( get_x(lp,id) != (long)get_x(lp, id) )
		        if( valueBest < get_x(lp,id) )
		        {
		            idBest = id;
		            valueBest = (long)get_x(lp,id) + 1;
		        }

	    if( idBest < 0 )
	        break;

	    // Then, round that value up.
	    set_lowbo( lp, (int)idBest, (REAL)valueBest );
	    const int solveRet = ::solve(lp);
	    if( solveRet != OPTIMAL )
	    {
	        set_lowbo( lp, (int)idBest, (REAL)(valueBest - 1) );
	        set_upbo( lp, (int)idBest, (REAL)(valueBest - 1) );
	        ::solve(lp);
	    }
    }
#endif

    idealRuncutCost = get_obj_val(lp);

    // Collect the solution from the lp and temporarily
    // put them on our own list for convenience.
    RunPattern *rpHead = NULL, *rpTail = NULL;

    id = 0;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
	    RunPattern	*rp, *rpNext;
	    for( rp = rd->pHead; rp != NULL; rp = rpNext )
	    {
	        rpNext = rp->getNext();

	        // Store the number of times this pattern should be used.
	        rp->workCount = get_x(lp, ++id);

	        // Store this possible pattern on our list.
	        rp->addToList( rpHead, rpTail );
	        RunChunk	*rc;
	        for( rc = rp->head; rc != NULL; rc = rc->next )
	        {
		        rc->workCount = rp->workCount;
		        rc->owner = rp;
	        }
	    }
	    // At this point, this run def does not own its patterns anymore.
	    rd->pHead = rd->pTail = NULL;
    }

    // Clean up the lp.
    delete_lp( lp );
    lp = NULL;

    // At this point:
    // - all the used run patterns have been placed on our own list.
    // - each run pattern and its component chunks have their workCount set
    //   to the desired number of chunks of that type.
    // - all the run definitions have been deleted.
    // - the lp has been removed.
    // Build an index so we can find the chunks faster.
    chunkIndexSize = (long)((end - start) / (double)multiple + 1);
    chunkIndex = new RunChunk ** [ chunkIndexSize ];
    chunkElements = new long [ chunkIndexSize ];
    long i;
    for( i = 0L; i < chunkIndexSize; i++ )
    {
	    chunkIndex[i] = NULL;
	    chunkElements[i] = 0;
    }

    // First, count up how many chunks occur in each time time bucket.
    // Also, record the min and max length of chunks to speed up searches.
    RunPattern *rp;
    RunChunk *rc;
    tShortest = 24L * 60L * 60L * 3L;
    tLongest = 0;
    for( rp = rpHead; rp != NULL; rp = rp->getNext() )
	    for( rc = rp->head; rc != NULL; rc = rc->next )
	    {
	        if( rc->length < tShortest ) tShortest = rc->length;
	        if( rc->length > tLongest )  tLongest  = rc->length;
	        chunkElements[(rc->start - start) / multiple]++;
	    }

    // Then, allocate space for each list chunk.
    // Reserve room for a NULL at the end of the array.
    for( i = 0L; i < chunkIndexSize; i++ )
    {
	    chunkIndex[i] = new RunChunk * [chunkElements[i]+1];
	    chunkIndex[i][chunkElements[i]] = NULL;
	    chunkElements[i] = 0;
    }

    // Now, assign all the chunks into the chunk index array.
    for( rp = rpHead; rp != NULL; rp = rp->getNext() )
	    for( rc = rp->head; rc != NULL; rc = rc->next )
	    {
	        const long bi = (rc->start - start) / multiple;
	        chunkIndex[bi][chunkElements[bi]++] = rc;
	    }

    // Sort by size so we can find them quickly.
    for( i = 0L; i < chunkIndexSize; i++ )
	    qsort( chunkIndex[i], chunkElements[i], sizeof(chunkIndex[i][0]), cmpChunks );

    // Make all the rundef's own their patterns again.
    RunPattern	*rpNext;
    for( rp = rpHead; rp != NULL; rp = rpNext )
    {
	    rpNext = rp->getNext();
	    rp->addToList( rp->owner->pHead, rp->owner->pTail );
    }
}

void	LPGen::cutBlocks()
{
    // Heuristically cut up the blocks - do this a few times to
    // ensure that we converge as close as possible to the ideal.
    RBlock	*rb;
    double	cost, costLast;

    long	numBlocks = 0L, i, j;

    // First, get an initial feasible solution.
    for( rb = rbHead; rb != NULL; rb = rb->next )
    {
	    rb->solve( *this, minLeftoverPieceSave, multiple );
	    numBlocks++;
    }

    RBlock	**blocks = new RBlock * [numBlocks];
    for( i = 0, rb = rbHead; rb != NULL; rb = rb->next, i++ )
	    blocks[i] = rb;

    // Keep refining the solution until the total cost no longer changes.
    srand48( 0xED );
    cost = INFINITY;
    do
    {
#ifdef PRINT_BLOCK_INFO
	    cout << "Recutting blocks...\n";
#endif
	    // Randomize the blocks to avoid cycles.
	    for( i = 0; i < numBlocks; i++ )
	    {
	        j = (long)(drand48() * numBlocks);
	        RBlock	*t = blocks[i];
	        blocks[i] = blocks[j];
	        blocks[j] = t;
	    }

	    costLast = cost;
	    cost = 0.0;
	    for( i = 0; i < numBlocks; i++ )
	    {
	        blocks[i]->putBack( *this );
	        cost += blocks[i]->solve( *this, minLeftoverPieceSave, multiple );
	    }
    } while( cost < costLast );

    // Cleanup.
    delete [] blocks;

#ifdef PRINT_BLOCK_INFO
    cout << "Cut blocks:\n";
    for( rb = rbHead; rb != NULL; rb = rb->next )
	    cout << *rb;

    cout << "Remaining Ideal Chunks:\n";
    double	stddev = 0.0;
    long c;
    for( c = 0; c < numChunks; c++ )
	    if( chunks[c]->workCount != 0.0 )
	    {
	        cout << c << '\t' << setw(6) << setprecision(8)
		     << (chunks[c]->workCount) << *chunks[c];
	        stddev += chunks[c]->workCount * chunks[c]->workCount;
	    }
    cout << "stddev: " << sqrt(stddev/(double)numChunks) << '\n';
    cout << '\n';
#endif
}
