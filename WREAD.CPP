//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <match.H>

/* set up data structures for weighted match */

/* to add a new type, add new case in SetUp() and a Set_X() routine */

void MatchGraph::SetUp ()
{
    long i,allocsize;

    U = Degree(0);
    V = NumEdges();

    allocsize = U+2*V+2;
    A      = new long[(size_t)allocsize];
    END    = new long[(size_t)allocsize];
    WEIGHT = new long[(size_t)allocsize];
    for (i=0; i<allocsize; i++)
	A[(int)i]=END[(int)i]=WEIGHT[(int)i]=0L;

    SetStandard();
}

void MatchGraph::SetStandard()
{
    long elabel, adj_node, i, j;
    long u, v, currentedge;
    Edge edge;

    currentedge = U+2L;
    for (i=1L; i<=U; ++i) {
	edge = FirstEdge(i);
	for (j = 1L; j <= Degree(i); ++j) {
	    adj_node = EndPoint(edge);
	    if (i < adj_node) {
		elabel = ELabel(edge)*2L;
		WEIGHT[(int)(currentedge-1L)] = WEIGHT[(int)currentedge] = 2L*elabel;
		END[(int)(currentedge-1L)] = i;
		END[(int)currentedge] = adj_node;
		if (A[(int)i] == 0)
		    A[(int)i] = currentedge;
		else {
		    u = i;
		    v = A[(int)i];
		    while (v != 0) {
			if (END[(int)v] > adj_node)
			    break;
			u = v;
			v = A[(int)v];
		    }
		    A[(int)u] = currentedge;
		    A[(int)currentedge] = v;
		}
		u = adj_node;
		v = A[(int)u];
		while (v != 0) {
		    u = v;
		    v = A[(int)v];
		}
		A[(int)u] = currentedge - 1L;
		currentedge += 2L;
	    }
	    edge = NextEdge(edge);
	}
    }
}


