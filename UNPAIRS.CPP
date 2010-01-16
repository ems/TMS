//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <match.H>

/* Expands a blossom.  Fixes up LINK and MATE. */

void MatchGraph::UNPAIR (long oldbase, long oldmate)

{   long e, newbase, u;

#ifdef DEBUG
    printf("Unpair oldbase, oldmate=%d %d\n",oldbase, oldmate);
#endif

    UNLINK (oldbase);
    newbase = BMATE (oldmate);
    if (newbase != oldbase) {
	LINK [(int)oldbase] = -DUMMYEDGE;
	REMATCH (newbase, MATE[(int)oldbase]);
	if (f == LASTEDGE[1])
	    LINK[(int)secondmate] = -LASTEDGE[2];
	else LINK[(int)secondmate] = -LASTEDGE[1];
	}
    e = LINK[(int)oldmate];
    u = BEND (OPPEDGE (e));
    if (u == newbase) {
	POINTER (newbase, oldmate, e);
	return;
	}
    LINK[BMATE (u)] = -e;
    do {
	e = -LINK[(int)u];
	v = BMATE (u);
	POINTER (u, v, -LINK[(int)v]);
	u = BEND (e);
    } while (u != newbase);
    e = OPPEDGE (e);
    POINTER (newbase, oldmate, e);
}


/* changes the matching along an alternating path */
/* firstmate is the first base vertex on the path */
/* edge e is the new matched edge for firstmate   */

void MatchGraph::REMATCH (long firstmate, long e)
{
#ifdef DEBUG
     printf("Rematch firstmate=%d e=%d-%d\n",firstmate, END[OPPEDGE(e)], END[e]);
#endif

    MATE[(int)firstmate] = e;
    nexte = -LINK[(int)firstmate];
    while (nexte != DUMMYEDGE) {
	e = nexte;
	f = OPPEDGE (e);
	firstmate = BEND (e);
	secondmate = BEND (f);
	nexte = -LINK[(int)firstmate];
	LINK[(int)firstmate] = -MATE[(int)secondmate];
	LINK[(int)secondmate] = -MATE[(int)firstmate];
	MATE[(int)firstmate] = f;
	MATE[(int)secondmate] = e;
	}
}


/* unlinks subblossoms in a blossom.  oldbase is the base of the blossom to */
/* be unlinked. */

void MatchGraph::UNLINK (long oldbase)
{
    int k, j=1;

#ifdef DEBUG
    printf("Unlink oldbase=%d\n",oldbase);
#endif

    i = NEXTVTX[(int)oldbase];
    newbase = NEXTVTX[(int)oldbase];
    nextbase = NEXTVTX[(int)LASTVTX[(int)newbase]];
    e = LINK[(int)nextbase];
 UL2:
    do {
	nextedge = OPPEDGE (LINK[(int)newbase]);
	for (k=1; k <= 2; ++k) {
	    LINK[(int)newbase] = -LINK[(int)newbase];
	    BASE[(int)i] = newbase;
	    i = NEXTVTX[(int)i];
	    while (i != nextbase) {
		BASE[(int)i] = newbase;
		i = NEXTVTX[(int)i];
	    }
	    newbase = nextbase;
	    nextbase = NEXTVTX[(int)LASTVTX[(int)newbase]];
	}
    } while (LINK[(int)nextbase] == nextedge);
    if (j==1) {
	LASTEDGE[1] = nextedge;
	j++;
	nextedge = OPPEDGE (e);
	if (LINK[(int)nextbase] == nextedge) {
	    goto UL2;
	}
    }
    LASTEDGE[2] = nextedge;

    if (BASE[(int)LASTVTX[(int)oldbase]] == oldbase)
	NEXTVTX[(int)oldbase] = newbase;
    else {
	NEXTVTX[(int)oldbase] = DUMMYVERTEX;
	LASTVTX[(int)oldbase] = oldbase;
    }
}


