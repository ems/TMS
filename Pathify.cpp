#pragma warning (disable: 4786)

#include <set>
#include <map>
#include <vector>

#include "Cis.h"

typedef std::multiset<ident_t>    SetID;

struct PathifyNode
{
    ident_t    id;
    SetID    in, out;

    PathifyNode( const ident_t aID ) : id(aID) {}

    void connect( PathifyNode *nTo )
    {
        out.insert( nTo->id );
        nTo->in.insert( id );

    }
    void    disconnect( PathifyNode *nTo )
    {
        out.erase( out.lower_bound(nTo->id) );
        nTo->in.erase( nTo->in.lower_bound(id) );
    }

    bool    isolated() const { return in.empty() && out.empty(); }
};

typedef std::map<ident_t, PathifyNode *>    PathifyGraph;

void    pathify( PathifyConnections &pc )
{
    PathifyGraph    g;

    // Create a graph corresponding to the connections.
    register PathifyConnections::iterator c, cEnd;
    for( c = pc.begin(), cEnd = pc.end(); c != cEnd; ++c )
    {
        PathifyGraph::iterator n;
        if( (n = g.find(c->first)) == g.end() )
            n = g.insert( std::make_pair(c->first, new PathifyNode(c->first) )).first;
        PathifyNode    *nFrom = n->second;
        if( (n = g.find(c->second)) == g.end() )
            n = g.insert( std::make_pair(c->second, new PathifyNode(c->second) )).first;
        PathifyNode    *nTo = n->second;

        nFrom->connect( nTo );
    }

    // Traverse the graph to create long paths.
    c = pc.begin();
    while( !g.empty() )
    {
        // See if there is a "natural" root node (no incoming connections).
        register PathifyGraph::iterator n, nEnd;
        for( n = g.begin(), nEnd = g.end(); n != nEnd; ++n )
        {
            if( n->second->in.empty() )
                break;
        }

        if( n == g.end() )
        {
            // All nodes are in a cycle.  Pick any one as a starting point.
            n = g.begin();
        }

        // Follow this path as far as possible.  As we remove the connections as we go, we will never
        // get stuck in a cycle.
        PathifyGraph::iterator nNext;
        while( !n->second->out.empty() )
        {
            nNext = g.find( *n->second->out.begin() );

            // Record the path arc.
            PathifyNode    *nFrom = n->second, *nTo = nNext->second;
            c->first = nFrom->id;
            c->second = nTo->id;
            ++c;

            // Remove the connection.
            nFrom->disconnect( nTo );

            // Remove the entire node if it is isolated.
            if( n->second->isolated() )
            {
                delete n->second;
                g.erase( n );
            }

            // Keep following the path.
            n = nNext;
        }

        // Remove the last node if it is isolated.
        if( n->second->isolated() )
        {
            delete n->second;
            g.erase( n );
        }
    }

    pc.erase( c, pc.end() );
}
