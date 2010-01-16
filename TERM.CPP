//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include <match.H>

/* updates numerical bounds for linking paths. */
/* called with LAST_D set to the bound on DELTA for the next search */

void MatchGraph::SET_MATCH_BOUNDS ()
{
    long del;

    for (v=1; v <= U; ++v) {
	if (LINK[(int)v] < 0 || BASE[(int)v] != v) {
	    NEXT_D[(int)v] = LAST_D;
	    continue;
	}
	LINK[(int)v] = -LINK[(int)v];
	i = v;
	while (i != DUMMYVERTEX) {
	    Y[(int)i] -= DELTA;
	    i = NEXTVTX[(int)i];
	}
	f = MATE[(int)v];
	if (f != DUMMYEDGE) {
	    i = BEND(f);
	    del = SLACK(f);
	    while (i != DUMMYVERTEX) {
		Y[(int)i] -= del;
		i = NEXTVTX[(int)i];
	    }
	}
	NEXT_D[(int)v] = LAST_D;
    }
}


/* undoes all blossoms to get the final matching */

void MatchGraph::UNPAIR_ALL ()
{
    long u;

    for (v=1; v <= U; ++v) {
	if (BASE[(int)v] != v || LASTVTX[(int)v] == v)
	    continue;
	nextu = v;
	NEXTVTX[(int)LASTVTX[(int)nextu]] = DUMMYVERTEX;
	while (1) {
	    u = nextu;
	    nextu = NEXTVTX[(int)nextu];
	    UNLINK (u);
	    if (LASTVTX[(int)u] != u) {
		f = (LASTEDGE[2] == OPPEDGE(e)) ? LASTEDGE[1] : LASTEDGE[2];
		NEXTVTX[(int)LASTVTX[(int)BEND(f)]] = u;
	    }
	    newbase = BMATE (BMATE(u));
	    if (newbase != DUMMYVERTEX && newbase != u) {
		LINK[(int)u] = -DUMMYEDGE;
		REMATCH (newbase, MATE[(int)u]);
	    }
	    while (LASTVTX[(int)nextu] == nextu && nextu != DUMMYVERTEX)
		nextu = NEXTVTX[(int)nextu];
	    if (LASTVTX[(int)nextu] == nextu && nextu == DUMMYVERTEX)
		break;
	}
    }
}


