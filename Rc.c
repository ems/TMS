//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "rc.h"

#ifndef PRODUCT
static const char *logFile = "c:\\windows\\tmslog.txt";
static long msgCount = 0;
static time_t logStartTime;

static void initLog(void)
{
	FILE	*pf = fopen( logFile, "w" );
	fclose( pf );

    msgCount = 0L;
    logStartTime = time(NULL);
}

char    szLogTmp[2048];
int logMsg( char *sz )
{
	FILE	*pf = fopen( logFile, "a" );
	if( !pf )
		return 0;

	fprintf( pf, "%ld\t%ld\t%s%s", ++msgCount, time(NULL) - logStartTime, sz,
                        sz[strlen(sz)-1] == '\n' ? "" : "\n");
	fclose( pf );
    return 1;
}
#endif

// Run cutter data lists.
Segment         *segmentList;
Block           *blockList;

Run             *runList;

Piece           *invalidPieceList;
Piece           *validPieceList;
Piece           *unexaminedPieceList;

/*****************************************************************
 * The Assert Handler is called from the Assert macro.  It prints
 * the filename and line number of where the assert happened, then
 * causes a divide by zero error to terminate execution.
 */
int     AssertHandler( file, lineno )
    char        *file;
    int         lineno;
{
  char outString[256];

  sprintf(outString, "AssertHandler: assert error in \"%s\" line %d.\n", file, lineno);
  MessageBeep(MB_ICONSTOP);
  MessageBox(NULL, outString, "TMS", MB_OK);

  return False;
}

void    RCFreeAll(void)
{
    segmentList = NULL;
    blockList = NULL;
    runList = NULL;
    unexaminedPieceList = NULL;
    invalidPieceList = NULL;
    validPieceList = NULL;

    FreeAll(MM_RUNS);
}

/*****************************************************************/
static  Block   *NewBlock(void)
{
    Block       *block = RCNew(Block, MM_RUNS);
    ListInsert( blockList, block );
    return block;
}

Piece   *NewPiece(void)
{
    Piece       *piece = RCNew(Piece, MM_RUNS);
    piece->status = BadStatus;
    return piece;
}

Run     *NewRun(void)
{
    Run *run = RCNew(Run, MM_RUNS);
    run->runType = grcp->runType;
    ListInsert( runList, run );
    return run;
}

/*****************************************************************/
Piece   *PieceRemove( Piece *piece )
{
    switch( piece->status )
    {
    case Unexamined:
        Assert( unexaminedPieceList );
        ListRemove( unexaminedPieceList, piece );
        break;

    case Valid:
        Assert( validPieceList );
        ListRemove( validPieceList, piece );
        break;

    case Invalid:
        Assert( invalidPieceList );
        ListRemove( invalidPieceList, piece );
        break;

    default:
        Assert( False );
        break;
    }
    return piece;
}

Piece   *PieceInsert( Piece *piece )
{
    switch( piece->status )
    {
    case Unexamined:
        ListInsert( unexaminedPieceList, piece );
        break;

    case Valid:
        ListInsert( validPieceList, piece );
        break;

    case Invalid:
        ListInsert( invalidPieceList, piece );
        break;

    default:
        Assert( False );
        break;
    }
    return piece;
}

#ifndef PRODUCT
/*****************************************************************/
static  char    *FormatNum( int size, char fill, long num )
{
    char        szNum[32];
    static      char szRet[32];
    int         len = sprintf( szNum, "%ld", num );
    memset( szRet, fill, size - len );
    strcpy( &szRet[size - len], szNum );
    return &szRet[0];
}
static  char    *FormatTime( char *szTime, cron_t time )
{
    cron_t      hour, min, sec;
    Boolean     am;

    hour = time / Hour(1);
    hour %= 24;
    min = (time % Hour(1)) / Minute(1);
    sec = time % Minute(1);
    if( hour < 12 )
        am = True;
    else
    {
        if( hour != 12 )
            hour -= 12;
        am = False;
    }
    szTime[0] = 0;
    strcat( szTime, FormatNum(2, ' ', hour) );
    strcat( szTime, FormatNum(2, '0', min) );
    strcat( szTime, ":" );
    strcat( szTime, FormatNum(2, '0', sec) );
    strcat( szTime, am ? "A" : "P" );
    return szTime;
}
static  char    *FormatTimeInterval( char *szTime, cron_t time )
{
    cron_t      hour, min, sec;

    hour = time / Hour(1);
    min = (time % Hour(1)) / Minute(1);
    sec = time % Minute(1);
    szTime[0] = 0;
    strcat( szTime, FormatNum(2, ' ', hour) );
    strcat( szTime, FormatNum(2, '0', min) );
    strcat( szTime, ":" );
    strcat( szTime, FormatNum(2, '0', sec) );
    return szTime;
}

/*****************************************************************/
void    PrintSegment( Segment *segment )
{
    char szStart[64], szEnd[64];

    FormatTimeInterval( szStart, segment->start );
    FormatTimeInterval( szEnd, segment->end );

    TM5( 1, "  Block %ld %ld %s %s %ld\n",
        segment->block,
        segment->ifrom,
        szStart,
        szEnd,
        segment->ito );
}
void    PrintPiece( Piece *piece )
{
    Segment *segment;

    TM( 1, "\n\n****Piece****\n" );
    ForAllPieceSegmentsDo( piece, segment )
        PrintSegment( segment );
}

void    PrintPieceList( Piece *pieceList )
{
    Piece *piece;

    ForAllListDo( pieceList, piece )
        PrintPiece( piece );
}

void    PrintRun( Run *run )
{
    Piece       *piece;

    TM1( 1, "\n&&&RunType  : %ld\n", run->runType );
    ForAllListDo( run->pieceList, piece )
        PrintPiece( piece );
}
static void     PrintRuns( void )
{
    Run         *run;

    TM( 1, "Valid runs:\n\n" );
    ForAllListDo( runList, run )
        PrintRun( run );
}
#endif // PRODUCT
/*****************************************************************/

#define CmpCode(a,b)    (int)((a)>(b) ? 1 : (a)<(b) ? -1 : 0)

/*****************************************************************/
static int     CmpSegments( Segment **pseg1, Segment **pseg2 )
{
#define Compare(a,b)    if( (a)!=(b) ) return CmpCode(a,b)
    Compare( (*pseg1)->block, (*pseg2)->block );
    Compare( (*pseg1)->start, (*pseg2)->start );
    Compare( (*pseg1)->iFromIndex, (*pseg2)->iFromIndex );
    return 0;
}
static  int     CmpPieces( Piece **ppiece1, Piece **ppiece2 )
{
    return CmpSegments(&(*ppiece1)->segmentStart,
                       &(*ppiece2)->segmentStart);
}

static  int     CmpPieceStart( Piece **ppiece1, Piece **ppiece2 )
{
    return CmpCode( (*ppiece1)->segmentStart->start, (*ppiece2)->segmentStart->start );
}

static  int     CmpPieceLength( Piece **ppiece1, Piece **ppiece2 )
{
    return CmpCode(ForwardPieceLength((*ppiece1)->segmentStart, (*ppiece1)->segmentEnd),
                   ForwardPieceLength((*ppiece2)->segmentStart, (*ppiece2)->segmentEnd));
}

static  int     CmpRuns( Run **prun1, Run **prun2 )
{
    // Sort runs by type and start time.
    Compare( (*prun1)->runType, (*prun2)->runType );
    return CmpCode((*prun1)->pieceList->segmentStart->start,
                   (*prun2)->pieceList->segmentStart->start );
}

// The SortDList macro is used to call the privateSortDList function
// with the correct casting.  The reason why this is done this way
// is to avoid needing another sort function for every structure type.

typedef int (*CmpFunc)(Segment **, Segment **);

#define SortDList(pHead, comp) \
        privateSortDList( (Segment **)pHead, \
                          (CmpFunc)comp )

static  CmpFunc sortCmpFunc = NULL;  // This saves stack space.

//---------------------------------------------------------------------
// The following routines implement a merge sort that sorts doubly-
// linked lists without allocating extra memory.  This is only
// slightly slower than a call to qsort.
//
static  Segment *splitDList( Segment *head )
{
    Segment *headNew = NULL;
    int putOnNewList = 0;
    Segment *pseg, *psegNext;

    // Put every second element on the new list.
    for( pseg = head;
         pseg != NULL;
         pseg = psegNext, putOnNewList ^= TRUE )
    {
        psegNext = pseg->next;
        if( putOnNewList )
        {
            // Take the element off the old list...
            if( pseg->prev )
                pseg->prev->next = pseg->next;
            if( pseg->next )
                pseg->next->prev = pseg->prev;

            // and put it on the new list.
            pseg->prev = NULL;
            if( headNew )
                headNew->prev = pseg;
            pseg->next = headNew;
            headNew = pseg;
        }
    }

    return headNew;
}

static  Segment *mergeDList( Segment *h1, Segment *h2 )
{
    Segment *headNew = NULL;
    Segment *tailNew = NULL;
    Segment *psegNext;
    Segment **ph;

    // Add the smallest element to the output list until one of the lists run out.
    while( h1 && h2 )
    {
        int ret = (*sortCmpFunc)(&h1, &h2);
        ph = (ret <= 0) ? &h1 : &h2;
        psegNext = (*ph)->next;  // Save the next pointer for this list.

        if( tailNew == NULL )
        {
            (*ph)->prev = (*ph)->next = NULL;
            headNew = (*ph);
            tailNew = (*ph);
        }
        else
        {
            (*ph)->next = NULL;
            (*ph)->prev = tailNew;
            tailNew->next = (*ph);
            tailNew = (*ph);
        }
        (*ph) = psegNext;
    }

    // Add the leftover list to the end of the output list.
    ph = ((h1) ? &h1 : ((h2) ? &h2 : NULL));
    if( ph != NULL )
        if( tailNew == NULL )
            headNew = (*ph);
        else
        {
            (*ph)->prev = tailNew;
            tailNew->next = (*ph);
        }

    return headNew;
}

static  Segment *sortDList( Segment *head )
{
    Segment *headNew;

    if( head == NULL || head->next == NULL )
        return head; // Nothing to do

    // First split this list into 2 parts.
    headNew = splitDList( head );

    // Sort each part.
    head    = sortDList( head );
    headNew = sortDList( headNew );

    // Merge the 2 sorted parts back together again.
    return mergeDList( head, headNew );
}

static void     privateSortDList( Segment **plistHead,
                                  CmpFunc compare )
{
    sortCmpFunc = compare;
    *plistHead = sortDList( *plistHead );
}
/****************************************************************/
static  void    InitPieces(void)
{
    Piece       *piece;
    Block       *block;
    int         numPieces = 0;

    // Initially make all pieces as large as blocks.
    ForAllListDo( blockList, block )
    {
        piece = NewPiece();
        piece->status = Unexamined;
        ListInsert( unexaminedPieceList, piece );

        // Splice the segments onto the run list/
        piece->segmentStart = block->segmentStart;
        piece->segmentEnd = block->segmentEnd;
        numPieces++;
    }
}
/****************************************************************/
static  void    InitBlocks(void)
{
    Block       *block = NULL;
    Segment     *segment;
    int         numBlocks = 0;

    ForAllListDo( segmentList, segment )
    {
        if( block == NULL ||
              block->segmentEnd->block != segment->block ||
              block->segmentEnd->end != segment->start)
        {
            block = NewBlock();
            block->segmentStart = segment;
            block->blockNumber = segment->block;
            numBlocks++;
        }
        block->segmentEnd = segment;
    }
}

/****************************************************************/
static  void    BestStraightCut(void)
{
    SortDList( &unexaminedPieceList, CmpPieceLength );

    while( unexaminedPieceList != NULL )
        StraightCutDecide(unexaminedPieceList);
}

void InitSegments(int fGetUnmatched)
{
    int                 i;
    RELIEFPOINTSDef *potpiece, *potpieceNext;
    Segment         *segment;

    segmentList = NULL;

    // Run through the array looking for available pairs of relief points
    // in the same block.
    //
    // Use those pairs to make segments.

    for( i = 0; i < m_numRELIEFPOINTS - 1; i++ )
    {
        if( m_pRELIEFPOINTS[i].blockNumber != m_pRELIEFPOINTS[i+1].blockNumber )
            continue;

        if( (IsFromReliefPointMatched(m_pRELIEFPOINTS[i]) &&
             IsToReliefPointMatched(m_pRELIEFPOINTS[i+1]) ? fGetUnmatched : !fGetUnmatched) )
            continue;

        // Relief points that are not in the same block and are
        // not matched can be used to form pieces.
        potpiece     = &m_pRELIEFPOINTS[i];
        potpieceNext = &m_pRELIEFPOINTS[i+1];
        segment = RCNew(Segment, MM_RUNS);
        segment->block = potpiece->blockNumber;
        segment->ifrom = potpiece->NODESrecordID;
        segment->ito   = potpieceNext->NODESrecordID;
        segment->start = potpiece->time;
        segment->end   = potpieceNext->time;
        segment->iFromIndex = i;
        segment->iToIndex   = i+1;

        ListInsert( segmentList, segment );
    }
    SortDList( &segmentList, CmpSegments );
}

/**************************************************************/
//
// This routine attempts to split very long invalid runs into
// valid piece sizes suitable to be combined into 2-piece runs.
//
static void SplitInvalidPieces(void)
{
    while( unexaminedPieceList != NULL )
        SplitPiece( unexaminedPieceList );
}
static void PerformTwoPieceRunCut(void)
{
    SplitInvalidPieces();
    MatchPieces();
}


//
// This routine installs a runID into the potential piece
// array.
//
static void InstallRun( Run *run, long runNumber )
{
    Piece       *piece;
    Segment     *segment;
    int         pieceNumber = 1;

	SortDList( &run->pieceList, CmpPieceStart );
    ForAllPiecesDo( run, piece )
    {
        ForAllPieceSegmentsDo( piece, segment )
        {
            m_pRELIEFPOINTS[segment->iFromIndex].start.recordID = NO_RECORD;
            m_pRELIEFPOINTS[segment->iFromIndex].start.runNumber = runNumber;
            m_pRELIEFPOINTS[segment->iFromIndex].start.pieceNumber = pieceNumber;

            m_pRELIEFPOINTS[segment->iToIndex].end.recordID = NO_RECORD;
            m_pRELIEFPOINTS[segment->iToIndex].end.runNumber = runNumber;
            m_pRELIEFPOINTS[segment->iToIndex].end.pieceNumber = pieceNumber;
        }
        pieceNumber++;
    }
}

static void InstallRuns(void)
{
    Run *run;
    ForAllListDo( runList, run )
         InstallRun( run, m_GlobalRunNumber++ );
}

static void ReinitializeLeftoverPieces(void)
{
    register Piece *piece;

    // Make sure everything is on the invalid piece list.
    while( (piece = unexaminedPieceList) != NULL )
        ChangePieceStatus( piece, Invalid );
    while( (piece = validPieceList) != NULL )
        ChangePieceStatus( piece, Invalid );

    // Of the remaining unvalid pieces, make sure they are as large
    // as possible.
    // This means that some pieces may have to be merged together.
    SortDList( &invalidPieceList, CmpPieces );

	// Merge consecutive pieces.
	// We first put them on the Valid list, then on the Unexamined list
	// to ensure the end up in the correct order.
	while( (piece = invalidPieceList) != NULL )
	{
		Assert( piece->segmentStart->block == piece->segmentEnd->block );
		Assert( piece->segmentStart->start < piece->segmentEnd->end );

		while( piece->next != NULL &&
				piece->segmentEnd->block	== piece->next->segmentStart->block &&
				piece->segmentEnd->end		== piece->next->segmentStart->start )
		{
			piece->segmentEnd = piece->next->segmentEnd;
			PieceRemove( piece->next );
		}

		Assert( piece->segmentStart->block == piece->segmentEnd->block );
		Assert( piece->segmentStart->start < piece->segmentEnd->end );

        ChangePieceStatus( piece, Valid );
	}

	while( (piece = validPieceList) != NULL )
		ChangePieceStatus( piece, Unexamined );
}

static void PerformStraightRunCut(void)
{
    BestStraightCut();
}

static	void	CombineConsecutiveTwoPiece( Run *runList )
{
	register Run	*r;
	register Piece	*p1, *p2;

	for( r = runList; r != NULL; r = r->next )
	{
		if( r->pieceList == NULL || r->pieceList->next == NULL )
			continue;

		// Check for consecutive pieces.
		p1 = r->pieceList; p2 = r->pieceList->next;
		if( p1->segmentEnd->end == p2->segmentStart->start &&
			p1->segmentEnd->block == p2->segmentStart->block )
		{
			ListRemove( r->pieceList, p2 );
			p1->segmentEnd = p2->segmentEnd;
		}
	}
}

// I use a pointer to the RunCutParameters structures
// because in the future the runcutter may have to
// keep track of multiple parameters sets, and the
// code can switch between them by just changing the
// grcp and lrcp pointers.
static  RunCutParameters        runCutParameters;
RunCutParameters                *grcp = &runCutParameters;

static  RunCutLookaheadParameters runCutLookaheadParameters;
RunCutLookaheadParameters		  *lrcp = &runCutLookaheadParameters;

static void InitRunCutParameters(void)
{
	grcp->fSmartLookahead 		= TRUE;
	grcp->fTwoPieceCanBeOne		= FALSE;
	
	grcp->platformTimeTarget  = Hour(8);
	grcp->platformTimeMin     = Hour(6)+Minute(30);
	grcp->platformTimeMax     = Hour(9)+Minute(30);
	
	grcp->unmatchedPieceCost  = Hour(10);
	
	grcp->minLeftover         = Hour(1);
	
	grcp->minPieceSize        = Hour(1);
	grcp->desPieceSize        = Hour(8);
	grcp->maxPieceSize        = Hour(10);
	
	grcp->minPieceSize1 = grcp->minPieceSize2 = Hour(3);
	grcp->desPieceSize1 = grcp->desPieceSize2 = Hour(4);
	grcp->maxPieceSize1 = grcp->maxPieceSize2 = Hour(6);
	grcp->maxSpread		   = Hour(48);
	
	grcp->startRunBeforeTime  = Hour(48)-Second(1);
	grcp->endRunAfterTime     = Hour(0);
	grcp->runType             = 0L;
	
    // These are set based on conditions in the particular block.
	lrcp->minPieceSize			= grcp->minPieceSize;
	lrcp->desPieceSize			= grcp->desPieceSize;
	lrcp->maxPieceSize			= grcp->maxPieceSize;
}

#define	ProvideDefault(x, d)	(x != NO_TIME ? x : d)
/****************************************************************/
void AutomaticRunCut(short int wmId)
{
	extern void RecursiveRuncut(void);
	extern void ChainsawRuncut(void);
	
	if(wmId == IDRECURSIVE)
		RecursiveRuncut();
	else
	{
		int         i, j;
		int         cutTwoPieceBefore = FALSE;
		long        savings;
		
		InitRunCutParameters();
		//
		//  Make sure memory pool and lists are initialized
		//
		RCFreeAll();
		InitSegments(1);
		
#ifndef PRODUCT
		TM( 1, "Initial Pieces:\n" );
		PrintPieceList( unexaminedPieceList );
#endif
		
		InitBlocks();
		InitPieces();
		
		if( CUTPARMS.cutRuns )
			grcp->endRunAfterTime = CUTPARMS.endTime;
		else
			grcp->startRunBeforeTime = CUTPARMS.startTime;
		
		grcp->fSmartLookahead = (CUTPARMS.flags & CUTPARMSFLAGS_SMART);
		
		for( i = 0; i < NUMRUNTYPES; i++ )
		{
			for( j = 0; j < NUMRUNTYPESLOTS; j++ )
			{
				if( (CUTPARMS.runtypes[i][j]) &&
					(RUNTYPE[i][j].flags & RTFLAGS_INUSE))
				{
					grcp->platformTimeMin    = ProvideDefault(RUNTYPE[i][j].minPayTime, 8*60*60);
					grcp->platformTimeTarget = ProvideDefault(RUNTYPE[i][j].desPayTime, 8*60*60);
					grcp->platformTimeMax    = ProvideDefault(RUNTYPE[i][j].maxPayTime, 10*60*60);
					grcp->minLeftover        = ProvideDefault(CUTPARMS.minLeftover, 60*60);
					grcp->runType            = MAKELONG(i,j);
					
					switch( RUNTYPE[i][j].numPieces )
					{
					case 1:
						grcp->fTwoPieceCanBeOne = TRUE;
						grcp->minPieceSize = ProvideDefault(RUNTYPE[i][j].PIECE[0].minPieceSize, 1*60*60);
						grcp->desPieceSize = ProvideDefault(RUNTYPE[i][j].PIECE[0].desPieceSize, 8*60*60);
						grcp->maxPieceSize = ProvideDefault(RUNTYPE[i][j].PIECE[0].maxPieceSize, 10*60*60);
						grcp->maxSpread    = ProvideDefault(RUNTYPE[i][j].maxSpreadTime, 24*60*60);
						if(grcp->desPieceSize < grcp->minPieceSize ||
							grcp->desPieceSize > grcp->maxPieceSize ||
							grcp->desPieceSize == NO_TIME)
							grcp->desPieceSize = (grcp->minPieceSize + grcp->maxPieceSize) / 2;
						if(i == TRIPPER_INDEX)
							PerformTripperRunCut();
						else if(i == ILLEGAL_INDEX)
							PerformIllegalRunCut();
						else
							PerformStraightRunCut();
						ReinitializeLeftoverPieces();
						break;
						//
						//  Consider all possible two piece cuts simultaneosly.
						//  This means that we only need to loop throught the 2-piece
						//  run types onece.
						//
					case 2:
						if( cutTwoPieceBefore )
							break;
						cutTwoPieceBefore = TRUE;
						//
						//  This gets a bit tricky on what to do here since I do
						//  not know beforehand where the piece is going to go
						//  until after I assign it - but I need to prepare probable
						//  pieces beforehand.  I don't know what to do about this
						//  if the specified pieces are of different sizes.
						//
						grcp->minPieceSize =
							(ProvideDefault(RUNTYPE[i][j].PIECE[0].minPieceSize, 4*60*60) +
							ProvideDefault(RUNTYPE[i][j].PIECE[1].minPieceSize, 4*60*60)) / 2;
						
						if(RUNTYPE[i][j].PIECE[0].desPieceSize == NO_TIME ||
							RUNTYPE[i][j].PIECE[1].desPieceSize == NO_TIME)
						{
							grcp->desPieceSize =
								(RUNTYPE[i][j].PIECE[0].minPieceSize +
								RUNTYPE[i][j].PIECE[0].maxPieceSize) / 2;
						}
						else
						{
							grcp->desPieceSize =
								(RUNTYPE[i][j].PIECE[0].desPieceSize +
								RUNTYPE[i][j].PIECE[1].desPieceSize) / 2;
						}
						grcp->maxPieceSize =
							(ProvideDefault(RUNTYPE[i][j].PIECE[0].maxPieceSize, 8*60*60) +
							ProvideDefault(RUNTYPE[i][j].PIECE[1].maxPieceSize, 8*60*60)) / 2;
						if(grcp->desPieceSize < grcp->minPieceSize ||
							grcp->desPieceSize > grcp->maxPieceSize ||
							grcp->desPieceSize == NO_TIME)
						{
							grcp->desPieceSize = (grcp->minPieceSize + grcp->maxPieceSize) / 2;
						}
						grcp->maxSpread = ProvideDefault(RUNTYPE[i][j].maxSpreadTime, 24*60*60);
						//
						//  Store multi-piece info.
						//
						grcp->desPieceSize1 = ProvideDefault(RUNTYPE[i][j].PIECE[0].desPieceSize, 4*60*60);
						grcp->minPieceSize1 = ProvideDefault(RUNTYPE[i][j].PIECE[0].minPieceSize, 1*60*60);
						grcp->maxPieceSize1 = ProvideDefault(RUNTYPE[i][j].PIECE[0].maxPieceSize, 8*60*60);
						grcp->desPieceSize2 = ProvideDefault(RUNTYPE[i][j].PIECE[1].desPieceSize, 4*60*60);
						grcp->minPieceSize2 = ProvideDefault(RUNTYPE[i][j].PIECE[1].minPieceSize, 1*60*60);
						grcp->maxPieceSize2 = ProvideDefault(RUNTYPE[i][j].PIECE[1].maxPieceSize, 8*60*60);
						
						StatusBarText("Performing Two Piece Run Cut...");
						PerformTwoPieceRunCut();
						ReinitializeLeftoverPieces();

						/*
						StatusBarText("Two Piece Cleanup...");
						PerformTwoPieceCleanup();
						ReinitializeLeftoverPieces();
						*/
						break;
						
					default:  // don't know how to cut this run type !!!
						break;
					}
				}
			}
		}
		//
		//  Combine any consecutive 2-piece runs together.
		//
		if( grcp->fTwoPieceCanBeOne )
			CombineConsecutiveTwoPiece( runList );
		//
		//  Make sure the runs are sorted - the leads to predictable run numbers
		//
		SortDList( &runList, CmpRuns );
		
		InstallRuns();
		
#ifndef PRODUCT
		PrintRuns();
		TM( 1, "Leftover Pieces:\n" );
		PrintPieceList( unexaminedPieceList );
#endif
		
		RCFreeAll();
		
		//
		//  Try to improve the runs that we have cut.
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE)
			RuncutImprove( &savings );
	}
}
