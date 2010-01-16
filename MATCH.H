//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef MatchGraph_H
#define MatchGraph_H

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#define	MAXWT  1000000L

/* the number of the blossom entered by edge e */
#define BEND(e) (BASE[(const int)END[(const int)e]])

/* the blossom matched with v's blossom */
#define BMATE(v) ((const int)BASE[(const int)END[(const int)MATE[(const int)v]]])

/* the blossom entered by the edge that links v's blossom */
#define BLINK(v) (BASE[(const int)END[(const int)LINK[(const int)v]]])

/* the edge e with it's direction reversed */
#define OPPEDGE(e) (((e - U) % 2L == 0) ? (e - 1L) : (e + 1L))

/* the slack of edge e */
#define SLACK(e) (Y[(const int)END[(const int)e]] + Y[(const int)END[(const int)OPPEDGE(e)]] - WEIGHT[(const int)e])

struct node_entry {
    long degree;
    unsigned long nlabel;
    struct edge_ent *adj_list;
    };
typedef struct node_entry *Graph;

struct edge_ent {
    long endpoint;		/* to node */
    long label;			/* cost of the arc */
    struct edge_ent *nextedge;	/* doubly-linked list next */
    struct edge_ent *prevedge;	/* double-linked list prev */
    struct edge_ent *otheredge	/* corresponding edge in the other direction */;
    };
typedef struct edge_ent *Edge;

#define Degree(n)    (graph[(const int)n].degree)
#define FirstEdge(n) (graph[(const int)n].adj_list)
#define NLabel(n)    (graph[(const int)n].nlabel)

#define EndPoint(e) (e->endpoint)
#define ELabel(e)   (e->label)
#define Other(e)    (e->otheredge)
#define NextEdge(e) (e->nextedge)

class MatchGraph
{
public:
    MatchGraph( const long size );
    MatchGraph( char *file );
    ~MatchGraph();
    void Weighted_Match( int maximize = 1 );
    void setNodeLabel( const long i, const unsigned long aNLabel )
    { graph[(const int)i].nlabel = aNLabel; }
    unsigned long getNodeLabel( const long i ) const
    { return graph[(const int)i].nlabel; }

    void AddEdge(long n, long m, long label);
    Edge FindEdge(long i, long j);
    int RemoveEdge(Edge edge);
    long NumEdges();
    void WriteGraph(char *file );
    long Mate(const long i) const { return MATE[(const int)i]; }
    long Size() const { return Degree(0); }
private:
    void init( const long size );
    void PAIR(long *outcome);
    void MERGE_PAIRS(long v);
    void LINK_PATH(long e);
    void INSERT_PAIR();
    void POINTER(long u, long v, long e);
    void SCAN(long x, long del);
    void SetUp();
    void SetStandard();
    void SET_MATCH_BOUNDS();
    void UNPAIR_ALL();
    void UNPAIR(long oldbase, long oldmate);
    void REMATCH(long firstmate, long e);
    void UNLINK(long oldbase);
    void Initialize(int maximize);
    void FreeUp();

    Graph	graph;

    long *A,*END,*WEIGHT,*NEXTPAIR;
    long *MATE,*LINK,*BASE,*NEXTVTX,*LASTVTX,*Y,*NEXT_D,*NEXTEDGE;

    long LAST_D, DELTA;

    long LASTEDGE[3];

    long DUMMYVERTEX, DUMMYEDGE;
    long U, V;

    long newbase, oldbase, nextbase, stopscan, pairpoint;
    long neighbor, nextpoint, newlast;
    long newmate, oldmate, oldfirst, firstmate, secondmate;
    long f, nextedge, nexte, nextu;

    long v,i,e;
};

#define INF	100000000L

#endif // MatchGraph_H
