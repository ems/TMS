//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef __STDC__
#define __STDC__     1
#endif

#if __STDC__ || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif

/* Decide.c */
extern void StraightCutDecide P_((Piece *piece));
/* Match.c */
extern void MatchPieces P_((void));
/* Split.c */
extern void SplitPiece P_((Piece *piece));
/* StrcutB.c */
extern Segment *StraightFindBackward P_((Segment *segmentEnd, Segment *segmentStart, cron_t *pcost));
extern Segment *StraightFindBackwardCut P_((Segment *segmentEnd, Segment *segmentStart, StraightCostFunc feasibleCF, StraightCostFunc desirableCF, cron_t *pcost));
extern Segment *StraightFindBackwardSplit P_((Segment *segmentEnd, Segment *segmentStart, cron_t *pcost));
extern cron_t EvaluateBackwardStrategy P_((Piece *piece));
extern Run *BackwardCutRun P_((Piece *piece, Segment *segment));
extern Run *StraightCutBackward P_((Piece *piece));
/* StrcutF.c */
extern cron_t StraightFeasiblePieceCoster P_(( Segment *start, Segment *end ));
extern cron_t StraightDesirablePieceCoster P_(( Segment *start, Segment *end ));
extern Segment *StraightFindForwardCut P_((Segment *segmentStart, Segment *segmentEnd, StraightCostFunc feasibleCF, StraightCostFunc desirableCF, cron_t *pcost));
extern Segment *StraightFindForward P_((Segment *segmentStart, Segment *segmentEnd, cron_t *pcost));
extern Segment *StraightFindForwardSplit P_((Segment *segmentStart, Segment *segmentEnd, cron_t *pcost));
extern cron_t EvaluateForwardStrategy P_((Piece *piece));
extern Run *ForwardCutRun P_((Piece *piece, Segment *segment));
extern Run *StraightCutForward P_((Piece *piece));
extern int GetLookaheadCutRange P_((Segment *, Segment *));
/* rc.c */
extern Segment *segmentList;
extern Block *blockList;
extern Run *runList;
extern Piece *invalidPieceList;
extern Piece *validPieceList;
extern Piece *unexaminedPieceList;
extern int AssertHandler P_((char *file, int lineno));
extern Piece *PieceRemove P_((Piece *piece));
extern Piece *PieceInsert P_((Piece *piece));
extern Piece *NewPiece(void);
extern void PrintSegment P_((Segment *segment));
extern void PrintRun P_((Run *run));
extern void PrintPiece P_((Piece *piece));
extern void ComputeRunStatistics P_((Run *run));

#if defined(__WIN32__)
extern cron_t scan1_ P_((cron_t *nb1, cron_t *n, double *sup, cron_t *basis, cron_t *mem, cron_t *ka, cron_t *kb, cron_t *sm, cron_t *tma, cron_t *tmb, double *y1, double *y2, double *dplus, double *dminus, cron_t *m1));
#else
extern cron_t scan1_ P_((cron_t huge *nb1, cron_t huge *n, double huge *sup, cron_t huge *basis, cron_t huge *mem, cron_t huge *ka, cron_t huge *kb, cron_t huge *sm, cron_t huge *tma, cron_t huge *tmb, double huge *y1, double huge *y2, double huge *dplus, double huge *dminus, cron_t huge *m1));
#endif
extern Run *NewRun(void);

extern cron_t PieceCoster( Piece rgPiece[], int numPieces );
extern long UnconstrainedPieceCoster( Piece rgPiece[], int numPieces, cron_t *pCost );
extern void PerformTripperRunCut(void);
extern void PerformIllegalRunCut(void);
extern void PerformTwoPieceCleanup(void);
extern void RCFreeAll(void);

// This is the public version of the runcut improver.
// It works on the RELIEFPOINTS array - returns the number of improvements.
// The total amount of cost reduction is returned in "savings".
extern int RuncutImprove( long *savings );

#undef P_
