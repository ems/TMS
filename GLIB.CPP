//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
extern "C"
{
#include "TMSHeader.h"
}

#include <match.H>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string>

using namespace std;

MatchGraph::MatchGraph(const long size)
{
    init( size );
}

void MatchGraph::init( const long size )
{
    long i;

    graph = new struct node_entry[(size_t)(size+1)];
    for (i=0; i<=size; i++)
	{
		Degree(i) = 0;
		FirstEdge(i) = NULL;
		NLabel(i) = 0;
    }
    Degree(0) = size;

    MATE     = NULL;
    LINK     = NULL;
    BASE     = NULL;
    NEXTVTX  = NULL;
    LASTVTX  = NULL;
    Y        = NULL;
    NEXT_D   = NULL;
    NEXTEDGE = NULL;
    NEXTPAIR = NULL;
}

void MatchGraph::AddEdge ( long n, long m, long label )
{
    Edge edge1,edge2;

    edge1 = (Edge) malloc(2*sizeof(struct edge_ent));
    if(edge1 == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      return;
    }
    edge2 = edge1 + 1;

    edge1->label = label;
    edge1->endpoint = m;
    edge1->otheredge = edge2;
    edge1->prevedge = NULL;
    edge1->nextedge = graph[(const int)n].adj_list;
    if (edge1->nextedge != NULL)
		edge1->nextedge->prevedge = edge1;
    graph[(const int)n].adj_list = edge1;
    graph[(const int)n].degree++;

    edge2->label = label;
    edge2->endpoint = n;
    edge2->otheredge = edge1;
    edge2->prevedge = NULL;
    edge2->nextedge = graph[(const int)m].adj_list;
    if (edge2->nextedge != NULL)
		edge2->nextedge->prevedge = edge2;
    graph[(const int)m].adj_list = edge2;
    graph[(const int)m].degree++;
}

Edge MatchGraph::FindEdge(long i, long j)
{
    Edge edge;

    for( edge = graph[(int)i].adj_list; edge != NULL; edge = edge->nextedge )
	if( edge->endpoint == j )
	    return edge;
    return NULL;
}

int MatchGraph::RemoveEdge( Edge edge )
{
    Edge other;
    long i,j;

    if (edge==NULL) return(0);
    other = edge->otheredge;
    i = other->endpoint;
    j = edge->endpoint;
    graph[(int)i].degree--; graph[(int)j].degree--;
    if (edge->prevedge == NULL) {
	graph[(int)i].adj_list = edge->nextedge;
	if (edge->nextedge != NULL)
	    edge->nextedge->prevedge = NULL;
    }
    else if (edge->nextedge == NULL)
	(edge->prevedge)->nextedge = NULL;
    else {
	(edge->nextedge)->prevedge = edge->prevedge;
	(edge->prevedge)->nextedge = edge->nextedge;
    }
    if (other->prevedge == NULL) {
	graph[(int)j].adj_list = other->nextedge;
	if (other->nextedge != NULL)
	    other->nextedge->prevedge = NULL;
    }
    else if (other->nextedge == NULL)
	(other->prevedge)->nextedge = NULL;
    else {
	(other->nextedge)->prevedge = other->prevedge;
	(other->prevedge)->nextedge = other->nextedge;
    }
    free((edge < other) ? edge : other);
    return(1);
}

long MatchGraph::NumEdges()
{
    long i,size,edges;

    edges = 0;
    size = Degree(0);
    for (i=1; i<=size; i++)
	edges += Degree(i);
    edges /= 2;
    return(edges);
}

/* Graph I/O routines */

MatchGraph::MatchGraph(char *file)
{
    FILE *fp;
    char szIn[1024];
    long degree, elabel, adj_node, i;

    if (file[0] == '\0') fp = stdin;
    else fp = fopen(file,"r");
    if (fp==NULL) {
	printf("ReadGraph: file %s can't be opened\n",file);
	exit(0);
    }
    fgets( szIn, sizeof(szIn), fp );
    const long size = atol( szIn );

    init( size );

    for (i = 1; i <= size; ++i)
    {
	fgets( szIn, sizeof(szIn), fp );
	degree = atol( szIn );
	while( degree-- > 0 )
	{
	    fgets( szIn, sizeof(szIn), fp );
            char *pch = strtok( szIn, " \t\n" );
            adj_node = atol( pch );
            pch = strtok( NULL, " \t\n" );
            elabel = atol( pch );
	    if (i < adj_node)
		AddEdge (i,adj_node,elabel);
	}
    }
    fclose(fp);
}

void MatchGraph::WriteGraph (char *file)
{
    FILE *fp;
    long size, i,j;
    Edge p;

    if (file[0] == '\0') fp = stdout;
    else fp = fopen(file,"w");
    if (fp==NULL) {
	printf("WriteGraph: file %s can't be opened\n",file);
	exit(0);
    }
    size = Degree(0);
    fprintf(fp,"%ld\n",size);

    for (i = 1; i <= size; i++) {
	fprintf(fp,"%ld\n",Degree(i) );
	p = FirstEdge(i);
	for (j = 1; j <= Degree(i); ++j) {
	    fprintf(fp,"%ld %ld\n",EndPoint(p),ELabel(p));
	    p = NextEdge(p);
	}
    }
    fclose(fp);
}

MatchGraph::~MatchGraph()
{
    delete [] MATE;

    long size = Degree(0);
    for (i = 1; i <= size; i++)
	while( graph[(int)i].degree > 0 )
	    RemoveEdge( FirstEdge(i) );
    delete [] graph;
}
