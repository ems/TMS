//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <match.H>

/* Assign a pointer link to a vertex.  Edge e joins a vertex in blossom */
/* u to a linked vertex. */

void MatchGraph::POINTER (long u, long v, long e)

{   long i, del;

#ifdef DEBUG
    printf("Pointer u,v,e=%d %d %d-%d\n",u,v,END[OPPEDGE(e)],END[e]);
#endif

    LINK[(int)u] = -DUMMYEDGE;
    NEXTVTX[(int)LASTVTX[(int)u]] = DUMMYVERTEX;
    NEXTVTX[(int)LASTVTX[(int)v]] = DUMMYVERTEX;

    if (LASTVTX[(int)u] != u) {
	i = MATE[(int)NEXTVTX[(int)u]];
	del = -SLACK(i) / 2L;
	}
    else del = LAST_D;

    i = u;
    while (i != DUMMYVERTEX) {
	Y[(int)i] += del;
	NEXT_D[(int)i] += del;
	i = NEXTVTX[(int)i];
	}
    if (LINK[(int)v] < 0) {
	LINK[(int)v] = e;
	NEXTPAIR[(int)DUMMYEDGE] = DUMMYEDGE;
	SCAN (v, DELTA);
	return;
	}
    else {
	LINK[(int)v] = e;
	return;
	}
}


/* Scan each vertex in the blossom whose base is x */

void MatchGraph::SCAN (long x, long del)

{   long u, del_e;

#ifdef DEBUG
    printf("Scan del=%d x=%d\n",del,x);
#endif

    newbase = BASE[(int)x];
    stopscan = NEXTVTX[(int)LASTVTX[(int)x]];
    while (x != stopscan) {
	Y[(int)x] += del;
	NEXT_D[(int)x] = LAST_D;
	pairpoint = DUMMYEDGE;
	e = A[(int)x];
	while (e != 0) {
	    neighbor = END[(int)e];
	    u = BASE[(int)neighbor];
	    if (LINK[(int)u] < 0) {
		if (LINK[BMATE (u)] < 0 || LASTVTX[(int)u] != u) {
		    del_e = SLACK (e);
		    if (NEXT_D[(int)neighbor] > del_e) {
			NEXT_D[(int)neighbor] = del_e;
			NEXTEDGE[(int)neighbor] = e;
			}
		    }
		}
	    else if (u != newbase) {
		INSERT_PAIR();
		}
	    e = A[(int)e];
	    }
	x = NEXTVTX[(int)x];
	}
    NEXTEDGE[(int)newbase] = NEXTPAIR[(int)DUMMYEDGE];
}


