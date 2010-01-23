//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#define cron_t          long    /* must be large enough to store the largest time */
#define flow_t          long    /* must be large enough hold the maximum number of trips */
#define locindex_t      long    /* must be large enough to hold the number of locations */

#define InvalidTime     ((cron_t)-1)

#define Second(x)       ((cron_t)(x))
#define Minute(x)       (Second(x) * Second(60))
#define Hour(x)         (Second(x) * Second(60) * Second(60))

#define InfiniteCost    Hour(1000)
/***************************************************************************/
/* define a bunch of useful macros for debugging, convenience etc. */
#define True    1
#define False   0

#define Boolean         int
#define EOS             '\0'
#define Unbounded       ((1<<sizeof(flow_t)*8-1)-1)

#define Min(a,b)        ((a)<(b)?(a):(b))
#define Max(a,b)        ((a)>(b)?(a):(b))
#define Abs(a)          ((a)<0?-(a):(a))
#define MkStr(s)        strcpy(malloc(strlen(s)+1),(s))
#define Number(a)       (sizeof(a)/sizeof((a)[0]))

// These defines control messaging and Asserts.
#define PRODUCT
//#undef PRODUCT
#define NOLOG
#define KEEPASSERTS

#define FFNameMatch(f,m)        True
#ifndef PRODUCT
    extern char szLogTmp[2048];
    extern int logMsg( char *sz );
#   define TExec(grbMask,w)     do{ if(FFNameMatch(__FILE__,(grbMask))){w;} } while(0)
#ifdef NOLOG
#   define TM(grbMask,w1)
#   define TM1(grbMask,w1,w2)
#   define TM2(grbMask,w1,w2,w3)
#   define TM3(grbMask,w1,w2,w3,w4)
#   define TM4(grbMask,w1,w2,w3,w4,w5)
#   define TM5(grbMask,w1,w2,w3,w4,w5,w6)
#   define TM6(grbMask,w1,w2,w3,w4,w5,w6,w7)
#else
#   define TPrint(grbMask,w)        (FFNameMatch(__FILE__,(grbMask))?((w),True):False)
#   define TM(grbMask,w1)           TPrint((grbMask),(sprintf(szLogTmp,(w1)),logMsg(szLogTmp)))
#   define TM1(grbMask,w1,w2)       TPrint((grbMask),(sprintf(szLogTmp,(w1),(w2)),logMsg(szLogTmp)))
#   define TM2(grbMask,w1,w2,w3)        TPrint((grbMask),(sprintf(szLogTmp,(w1),(w2),(w3)),logMsg(szLogTmp)))
#   define TM3(grbMask,w1,w2,w3,w4) TPrint((grbMask),(sprintf(szLogTmp,(w1),(w2),(w3),(w4)),logMsg(szLogTmp)))
#   define TM4(grbMask,w1,w2,w3,w4,w5)  TPrint((grbMask),(sprintf(szLogTmp,(w1),(w2),(w3),(w4),(w5)),logMsg(szLogTmp)))
#   define TM5(grbMask,w1,w2,w3,w4,w5,w6)   TPrint((grbMask),(sprintf(szLogTmp,(w1),(w2),(w3),(w4),(w5),(w6)),logMsg(szLogTmp)))
#   define TM6(grbMask,w1,w2,w3,w4,w5,w6,w7)    TPrint((grbMask),(sprintf(szLogTmp,(w1),(w2),(w3),(w4),(w5),(w6),(w7)),logMsg(szLogTmp)))
#endif
#   define  Assert( fCond )         ( !(fCond)?AssertHandler(__FILE__,__LINE__):True)
#else
#   define TExec(grbMask,w1)
#   define TM(grbMask,w1)
#   define TM1(grbMask,w1,w2)
#   define TM2(grbMask,w1,w2,w3)
#   define TM3(grbMask,w1,w2,w3,w4)
#   define TM4(grbMask,w1,w2,w3,w4,w5)
#   define TM5(grbMask,w1,w2,w3,w4,w5,w6)
#   define TM6(grbMask,w1,w2,w3,w4,w5,w6,w7)
#ifndef KEEPASSERTS
#       define  Assert( fCond )
#else
static int assertEffect = 0;        // This keeps the compiler from complaining.
#   define  Assert( fCond ) \
        (assertEffect = ( !(fCond)?AssertHandler(__FILE__,__LINE__):True))
#endif
#endif

/*****************************************************************/
#define Head(L) (L)
#define Next(k) (k)->next
#define Prev(k) (k)->prev

#define IsListNull( L ) ((L) == NULL)

#define ListInsert( L, x ) \
    do { Next(x) = Head(L); \
    if( Head(L) != NULL ) Prev(Head(L)) = (x); \
    Head(L) = (x); \
    Prev(x) = NULL; } while(0)

#define ListRemove( L, x ) \
    do { if( Prev(x) != NULL )       Next(Prev(x)) = Next(x); \
    else                        Head(L) = Next(x); \
    if( Next(x) != NULL )       Prev(Next(x)) = Prev(x); } while(0)

#define DeclareListPointers(t)  t *next; t *prev

/*****************************************************************/

// Global parameters for runcutting.
typedef struct RunCutParametersStruct
{
         // If fSmartLookahead is TRUE, then the cutters will
         // divide the total remaining block by the desired piece size and
         // cut smaller or larger pieces to maximize the possibility that
         // the entire block can be cut without invalid leftovers.
         // If fSmartLookahead if FALSE, then the cutters will
         // cut as close as possible to the user-provided desired piece size
         // leaving whatever leftover occurs.
    // This applies to both 1-piece and 2-piece runs.
    int     fSmartLookahead;

	int		fTwoPieceCanBeOne;	// indicates that a two piece can be joined to a one piece.

    cron_t platformTimeMin;
    cron_t platformTimeTarget;
    cron_t platformTimeMax;

    cron_t unmatchedPieceCost; // used for 2 piece runs.

    cron_t minLeftover;        // smallest piece size to leave over.

    cron_t minPieceSize;       // used for 1 piece runs and 2 piece runs for the matcher.
    cron_t desPieceSize;
    cron_t maxPieceSize;

	cron_t minPieceSize1;	   // used for 2 piece runs for the heuristic.
	cron_t desPieceSize1;
	cron_t maxPieceSize1;
	cron_t minPieceSize2;
	cron_t desPieceSize2;
	cron_t maxPieceSize2;

	cron_t maxSpread;

    cron_t startRunBeforeTime; // cut window parameters.
    cron_t endRunAfterTime;

    long   runType;            // requested run type to cut.
} RunCutParameters;
extern  RunCutParameters *grcp;

// Local parameters for runcutting.
typedef struct RunCutLookaheadParametersStruct
{
     cron_t minPieceSize;
     cron_t desPieceSize;
     cron_t maxPieceSize;
} RunCutLookaheadParameters;
extern RunCutLookaheadParameters *lrcp;

typedef struct Segment Segment;
typedef struct Piece Piece;
typedef struct Block Block;
typedef struct Run Run;

struct Segment
{
    DeclareListPointers(Segment); /* this must be first */
    /* Segments are always maintained in block and time order. */

    long        block;          /* block, route of segment */
    locindex_t  ifrom;          /* index of from information */
    locindex_t  ito;            /* index of to information */
    cron_t      start;          /* start time of segment */
    cron_t      end;            /* end time of segment */
    int         iFromIndex, iToIndex; /* indexes into POTPIECE array */
};

typedef enum { Unexamined, Valid, Invalid, Allocated, BadStatus } PieceStatus;

struct Piece
{
    DeclareListPointers(Piece); /* this must be first! */

    Segment *segmentStart;      /* the first segment in this piece */
    Segment *segmentEnd;        /* the last segment in this piece */

    PieceStatus status;
};

#define PieceLength( p ) ((p)->segmentEnd->end - (p)->segmentStart->start)

#define ForAllPieceSegmentsDo( p, s ) \
        for( (s) = (p)->segmentStart; \
             (s)!=NULL && (s)->prev!=(p)->segmentEnd; \
             (s) = (s)->next )

#define ForAllBackwardPieceSegmentsDo( p, s ) \
        for( (s) = (p)->segmentEnd; \
             (s)!=NULL && (s)->next!=(p)->segmentStart; \
             (s) = (s)->prev )

struct Block
{
    DeclareListPointers(Block); /* this must be first! */

    long        blockNumber;    /* the number of this block */

    Segment     *segmentStart;  /* the first segment in this block */
    Segment     *segmentEnd;    /* the last segment in this block */
};

/* By definition, all runs are valid. */
struct Run
{
    DeclareListPointers(Run);   /* this must be first! */

    long        runType;        /* the runcode of this run */
	long		runNumber;		/* the number of this run (used by RuncutImprove) */

    Piece       *pieceList;     /* the pieces of this run */
};

#define ForAllListDo(l, e) for( (e)=l; (e) != NULL; (e)=(e)->next )

#define ForAllPiecesDo(r, p) for((p)=(r)->pieceList;(p)!=NULL;(p)=(p)->next)

#define ChangePieceStatus( piece, stat ) \
    do { if( (piece)->status != (stat) ) { \
         PieceRemove(piece); \
         (piece)->status = (stat); \
         PieceInsert(piece); } \
    } while(0)

/*****************************************************************/

#ifndef PRODUCT
#define ForwardPieceLength( s1, s2 )    (Assert((s2)->end>=(s1)->start),(s2)->end-(s1)->start)
#define BackwardPieceLength( s1, s2 )   (Assert((s1)->end>=(s2)->start),(s1)->end-(s2)->start)
#else
#define ForwardPieceLength( s1, s2 )    ((s2)->end - (s1)->start)
#define BackwardPieceLength( s1, s2 )   ((s1)->end - (s2)->start)
#endif

#define RCNew(type, pool) \
    (type *)PoolCalloc(1, sizeof(type), pool)

typedef cron_t (*StraightCostFunc)( Segment *start, Segment *end );

#include "rcproto.h"
