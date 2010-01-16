/* N-cubed weighted matching */
/* Implementation of H. Gabow's Ph.D. thesis, Stanford Univ. 1973 */
/* Written by Edward Rothberg  7/85 */
/* For complete details, please refer to the original paper */

/* Send either a Euclidean graph or an adjacency list graph. */
/* Returns an array, the ith entry being the mate of vertex 'i'. */
/* A zero entry indicates an unmatched vertex. */

/* To add new type, see readgraph.c */

#include <match.H>

void MatchGraph::Weighted_Match (int maximize)
{
    long g, j, w, outcome, loop=1;

    /* set up internal data structure */
    SetUp();

    Initialize(maximize);

    for(;;) {
	/* printf("Augment #%d\n",loop++); */
	DELTA = 0;
	for (v=1; v<=U; ++v)
	    if (MATE[(int)v] == DUMMYEDGE)
		POINTER (DUMMYVERTEX, v, DUMMYEDGE);
	for(;;) {
	    i = 1;
	    for (j=2; j<=U; ++j)
		if (NEXT_D[(int)i] > NEXT_D[(int)j])
		    i = j;
	    DELTA = NEXT_D[(int)i];
	    if (DELTA == LAST_D)
		goto done;
	    v = BASE[(int)i];
	    if (LINK[(int)v] >= 0) {
		PAIR (&outcome);
		if (outcome == 1)
		    break;
	    }
	    else {
		w = BMATE (v);
		if (LINK[(int)w] < 0) {
		    POINTER (v, w, OPPEDGE(NEXTEDGE[(int)i]));
		}
		else UNPAIR (v, w);
	    }
	}

	LAST_D -=DELTA;
	SET_MATCH_BOUNDS();
	g = OPPEDGE(e);
	REMATCH (BEND(e), g);
	REMATCH (BEND(g), e);
    }

 done:
    SET_MATCH_BOUNDS();
    UNPAIR_ALL();
    for (i=1; i<=U;++i)
    {
	MATE[(int)i] = END[(int)MATE[(int)i]];
	if (MATE[(int)i]==DUMMYVERTEX)
	    MATE[(int)i]=0;
    }

    FreeUp();
}


void MatchGraph::Initialize(int maximize)
{
    long i, allocsize, max_wt= -MAXWT, min_wt=MAXWT;

    DUMMYVERTEX = U+1;
    DUMMYEDGE = U+2*V+1;
    END[(int)DUMMYEDGE] = DUMMYVERTEX;

    for (i=U+2; i<=U+2*V; i+=2)
    {
	if (WEIGHT[(int)i] > max_wt)
	    max_wt = WEIGHT[(int)i];
	if (WEIGHT[(int)i] < min_wt)
	    min_wt = WEIGHT[(int)i];
    }
    if (!maximize)
    {
	if (U%2!=0)
	{
	    printf("Must have an even number of vertices to do a\n");
	    printf("minimum complete matching.\n");
	    exit(0);
	}
	max_wt += 2L;		/* Don't want all zero weight */
#ifndef SHOULD_NOT_BE_NECESSARY_BUT_SEEMS_TO_WORK
	max_wt *= 2L;
#endif
	for (i=U+1; i<=U+2*V; i++)
	    WEIGHT[(int)i] = max_wt-WEIGHT[(int)i];
	max_wt = max_wt-min_wt;
    }
    LAST_D = max_wt/2L;

    if( MATE ) delete [] MATE;
    allocsize = U + 2L;
    MATE     = new long[(size_t)allocsize];
    LINK     = new long[(size_t)allocsize];
    BASE     = new long[(size_t)allocsize];
    NEXTVTX  = new long[(size_t)allocsize];
    LASTVTX  = new long[(size_t)allocsize];
    Y        = new long[(size_t)allocsize];
    NEXT_D   = new long[(size_t)allocsize];
    NEXTEDGE = new long[(size_t)allocsize];
    allocsize = U + 2L*V + 2L;
    NEXTPAIR = new long[(size_t)allocsize];

    for (i = 0; i <= U+1; ++i) {
		MATE[(int)i] = DUMMYEDGE;
		NEXTEDGE[(int)i] = DUMMYEDGE;
		NEXTVTX[(int)i] = 0;
		LINK[(int)i] = -DUMMYEDGE;
		BASE[(int)i] = i;
		LASTVTX[(int)i] = i;
		Y[(int)i] = LAST_D;
		NEXT_D[(int)i] = LAST_D;
    }
}

void MatchGraph::FreeUp()
{
    // Free everything but MATE
    delete [] LINK;
    delete [] BASE;
    delete [] NEXTVTX;
    delete [] LASTVTX;
    delete [] Y;
    delete [] NEXT_D;
    delete [] NEXTEDGE;
    delete [] NEXTPAIR;
    delete [] A;
    delete [] END;
    delete [] WEIGHT;
}
