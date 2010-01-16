//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <match.H>

/* Process an edge linking two linked vertices */
/* Note: global variable v set to the base of one end of the linking edge */

void MatchGraph::PAIR (long *outcome)
{
    long u, w, temp;

#ifdef DEBUG
    printf("Pair v=%d\n",v);
#endif

    e = NEXTEDGE[(int)v];
    while (SLACK(e) != 2L*DELTA)
	e = NEXTPAIR[(int)e];
    w = BEND (e);
    LINK[BMATE (w)] = -e;
    u = BMATE (v);
    while (LINK[(int)u] != -e) {
	LINK[(int)u] = -e;
	if (MATE[(int)w] != DUMMYEDGE) {
	    temp = v;
	    v = w;
	    w = temp; }
	v = BLINK (v);
	u = BMATE (v);
	}
    if (u == DUMMYVERTEX && v != w) {
	*outcome = 1;
	return;
    }
    newlast = v;
    newbase = v;
    oldfirst = NEXTVTX[(int)v];
    LINK_PATH (e);
    LINK_PATH (OPPEDGE (e));
    NEXTVTX[(int)newlast] = oldfirst;
    if (LASTVTX[(int)newbase] == newbase)
	LASTVTX[(int)newbase] = newlast;
    NEXTPAIR[(int)DUMMYEDGE] = DUMMYEDGE;
    MERGE_PAIRS (newbase);
    i = NEXTVTX[(int)newbase];
    do {
	MERGE_PAIRS (i);
	i = NEXTVTX[(int)LASTVTX[(int)i]];
	SCAN (i, 2L*DELTA - SLACK(MATE[(int)i]));
	i = NEXTVTX[(int)LASTVTX[(int)i]];
    } while (i != oldfirst);
    *outcome = 0;
}


/* merges a subblossom's pair list into a new blossom's pair list */
/* v is the base of the previously unlinked subblossom */
/* Note: global variable newbase set to the base of the new blossom */
/* 	called with NEXTPAIR[DUMMYEDGE] pointing to the first edge */
/*		on newbase's pair list */

void MatchGraph::MERGE_PAIRS (long v)
{
#ifdef DEBUG
    printf("Merge Pairs v=%d\n",v);
#endif

    NEXT_D[(int)v] = LAST_D;
    pairpoint = DUMMYEDGE;
    f = NEXTEDGE[(int)v];
    while (f != DUMMYEDGE)
    {
	e = f;
	neighbor = END[(int)e];
	f = NEXTPAIR[(int)f];
	if (BASE[(int)neighbor] != newbase)
	    INSERT_PAIR();
    }
}


/* links the unlinked vertices in the path P(END[e],newbase) */
/* Note: global variable newbase is set to the base vertex of the new blossom */
/*		newlast is set to the last vertex in newbase's current blossom*/

void MatchGraph::LINK_PATH (long e)
{
    long u;

#ifdef DEBUG
    printf("Link Path e=%d-%d\n", END[OPPEDGE(e)], END[e]);
#endif

    v = BEND (e);
    while (v != newbase) {
	u = BMATE (v);
	LINK[(int)u] = OPPEDGE (e);
	NEXTVTX[(int)newlast] = v;
	NEXTVTX[(int)LASTVTX[(int)v]] = u;
	newlast = LASTVTX[(int)u];
	i = v;
	BASE[(int)i] = newbase;
	i = NEXTVTX[(int)i];
	while (i != DUMMYVERTEX) {
	    BASE[(int)i] = newbase;
	    i = NEXTVTX[(int)i];
	}
	e = LINK[(int)v];
	v = BEND (e);
    }
}


/* Update a blossom's pair list. */
/* Note: called with global variable e set to the edge to be inserted. */
/*			neighbor set to the vertex at the end of e */
/*			pairpoint set to the next pair on the pair list */

void MatchGraph::INSERT_PAIR ()
{
    long del_e;

#ifdef DEBUG
    printf("Insert Pair e=%d-%d\n",END[OPPEDGE(e)],END[e]);
#endif

    del_e = SLACK(e)/2L;
    nextpoint = NEXTPAIR[(int)pairpoint];

    while (END[(int)nextpoint] < neighbor) {
	pairpoint = nextpoint;
	nextpoint = NEXTPAIR[(int)nextpoint];
    }
    if (END[(int)nextpoint] == neighbor) {
	if (del_e >= SLACK (nextpoint)/2)
	    return;
	nextpoint = NEXTPAIR[(int)nextpoint];
    }
    NEXTPAIR[(int)pairpoint] = e;
    pairpoint = e;
    NEXTPAIR[(int)e] = nextpoint;
    if (NEXT_D[(int)newbase] > del_e)
	NEXT_D[(int)newbase] = del_e;
}

