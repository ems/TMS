
#ifndef BinPackRuncut_H
#define BinPackRuncut_H

#pragma warning (disable: 4786)

#ifdef min
#undef min
#undef max
#endif

#include <vector>
#include <algorithm>
#include <map>
#include <list>
#include <set>
#include <iostream>
#include "hash_buf.h"
#include "multi_cmp.h"
#include "HashObj.h"

typedef long tod_t;
typedef unsigned long ident_t;

namespace BinPackRuncut
{

typedef std::vector<size_t>	SizeTVector;

class Block;

struct Relief
{
	ident_t	nodeID;
	tod_t	t;
	float	lat, lng;
	ident_t	iRef;

	bool	operator<( const Relief &r ) const
	{ return t < r.t ? true : r.t < t ? false : nodeID < r.nodeID; }

	SizeTVector	children;

	bool	head, tail;
};
typedef std::vector<Relief>	ReliefVector;

struct Piece
{
	Piece( Block *aB = NULL, const size_t aI = 10000, const size_t aJ = 10000 ) : b(aB), i(aI), j(aJ) {}

	Block	*b;
	size_t	i, j;

	Relief	&first() const;
	Relief	&last() const;
	tod_t	length() const;

	bool	empty() const { return i == j; }

	bool	overlaps( const Piece &p ) const
	{
		const Piece &x = i < p.i ? *this : p;
		const Piece &y = i < p.i ? p : *this;
		return y.i < x.j;
	}
	bool	operator<( const Piece &p ) const;
	bool	operator==( const Piece &p ) const	{ return b == p.b && i == p.i && j == p.j; }

	size_t	hash( size_t h = 0 ) const
	{
		h = hash_buf( (void *)b, h );
		h = hash_buf( i, h );
		h = hash_buf( j, h );
		return h;
	}
};
typedef std::list<Piece>		PieceList;
typedef std::vector<Piece>		PieceVector;

class Block
{
public:
	friend struct Piece;
	friend class Runcut;

	Block( const ident_t aID ) : id(aID) {}

	void	append( const tod_t t, const ident_t nodeID, const ident_t iRef, const float lat = 0.0, const float lng = 0.0 )
	{
		Relief	r;
		r.nodeID = nodeID;
		r.iRef = iRef;
		r.t = t;
		r.lat = lat;
		r.lng = lng;
		r.head = r.tail = false;
		relief.push_back( r );
	}

	void	resetHeadTail()
	{
		for( register ReliefVector::iterator r = relief.begin(), rEnd = relief.end(); r != rEnd; ++r )
			r->head = r->tail = false;
		relief[0].tail = true;
		relief.back().head = true;
	}

	const Relief	&operator[]( const size_t i ) const { return relief[i]; }

	tod_t	getTime( const size_t i ) const						{ return relief[i].t; }
	tod_t	pieceLength( const size_t i, const size_t j ) const	{ return getTime(j) - getTime(i); }
	tod_t	pieceLength( const Piece &p ) const					{ return getTime(p.j) - getTime(p.i); }

	tod_t	workTime( const size_t i, const size_t j ) const	{ return relief[j].t - relief[i].t; }

	void	resetPartitions( const tod_t pieceSizeMin, const tod_t pieceSizeMax );

	void	removeOverlapping( const Piece &piece );

	int		cmp( const Block &b ) const
	{
		return MC::multi_cmp(	id,			MC::less, b.id,
								relief[0],	MC::less, b.relief[0] );
	}
	bool	operator<( const Block &b ) const	{ return cmp(b) < 0; }

	ident_t			id;
protected:
	ReliefVector	relief;
	PieceList		pieces;

	void	partitionRecursive( const tod_t pieceSizeMin, const tod_t pieceSizeMax, const size_t i );
};
typedef std::list<Block>	BlockList;

inline Relief	&Piece::first() const { return b->relief[i]; }
inline Relief	&Piece::last()	const { return b->relief[j]; }
inline tod_t	Piece::length() const { return b ? last().t - first().t : 0; }
inline bool	Piece::operator<( const Piece &p ) const
{
	return MC::multi_cmp( first().t,	MC::less,		p.first().t,
						  length(),		MC::greater,	p.length(),
						  b,			MC::less,		p.b ) < 0;
}
struct Time
{
	Time( const tod_t aT = 0 ) : t(aT) {}

	int	hour() const	{ return int(t / (60*60)); }
	int min() const		{ return int((t / 60) % 60); }
	int	sec() const		{ return int(t % 60); }

	tod_t	t;
};
inline std::ostream &operator<<( std::ostream &os, const Time &t )
{
	char	sz[32];
	sprintf( sz, "%02d%02d", t.hour(), t.min(), t.sec() );
	return os << sz;
}
inline std::ostream &operator<<( std::ostream &os, const Piece &p )
{
	return os << p.b->id << ": (" << p.first().nodeID << ' ' << Time(p.first().t) << ','
								  << p.last ().nodeID << ' ' << Time(p.last ().t)
								  << " [" << Time(p.last().t - p.first().t) << "]"
								  << ')';
}

class Run
{
public:
	Run() : cost(0) {}

	tod_t	workTime() const { return piece[0].length() + piece[1].length(); }
	tod_t	payTime( const tod_t payTimeMin )
	{
		const tod_t wt = workTime();
		return wt < payTimeMin ? payTimeMin : wt;
	}
	tod_t	spreadTime() const
	{
		return	piece[0].empty() ? piece[1].length() :
				piece[1].empty() ? piece[0].length() :
				piece[1].last().t - piece[0].first().t;
	}
	tod_t	breakTime() const
	{
		return piece[0].empty() || piece[1].empty() ? 0 : piece[1].first().t - piece[0].last().t;
	}

	double		cost;
	Piece		piece[2];

	int		cmp( const Run &r ) const
	{
		return MC::multi_cmp(	piece[0], MC::less, r.piece[0],
								piece[1], MC::less, r.piece[1] );
	}
	bool	operator==( const Run &r ) const { return cmp(r) == 0; }
	bool	operator<( const Run &r ) const { return cmp(r) < 0; }

	size_t		hash() const	{ return piece[1].hash( piece[0].hash() ); }
};
typedef std::list<Run>	RunList;
typedef std::set<Run>	RunSet;

inline std::ostream &operator<<( std::ostream &os, const Run &r )
{
	return os << r.cost << ": " << r.piece[0] << " - " << r.piece[1];
}

struct PieceChooser
{
	virtual Piece &operator()( Piece &firstPiece, PieceVector &secondPieces ) = 0;
};

struct NodeID
{
	NodeID( const ident_t aID ) : id(aID) {}
	ident_t	id;

	bool	operator==( const NodeID &n ) const { return id == n.id; }

	size_t	hash() const { return hash_buf(id); }
};

class Runcut
{
public:
	Runcut() :
		callback(NULL), callbackData(NULL),
		checkFeasibleCallback(NULL), checkFeasibleCallbackData(NULL)
	{ resetParameters(); }

	Block	&newBlock( const ident_t id )
	{
		blocks.push_back( Block(id) );
		return blocks.back();
	}

	void	setEquivalent( const ident_t x, const ident_t y, const tod_t t )	{ equivalences.insert( IDPair(x, y), t ); }

	void	resetParameters()
	{
		tCur = 0;

		startEndMustMatch = false;
		const tod_t	hour = 60*60;
		pieceSizeMin	=  2 * hour;
		pieceSizeMax	=  6 * hour;

		workPayMin		=  8 * hour;

		platformMin		=  6 * hour;
		platformMax		=  9 * hour;

		breakMin		=      hour / 2;
		breakMax		=  4 * hour;

		spreadMax		= 12 * hour;

		regularCostPerHour				= 25.0;
		additionalPenaltyCostPerHour	= 20.0;
	}

	bool	startEndMustMatch;
	tod_t	pieceSizeMin;		// Length of shortest piece.
	tod_t	pieceSizeMax;		// Length of longest piece.

	tod_t	workPayMin;			// Minimum paid work.

	tod_t	platformMin;		// Minimum total work time.
	tod_t	platformMax;		// Maximum total work time.

	tod_t	breakMin;			// Minimum break.
	tod_t	breakMax;			// Maximum break.

	tod_t	spreadMax;			// Maximum spread between pieces.

	double		regularCostPerHour;
	double		additionalPenaltyCostPerHour;

	struct ChooseGreedy : public PieceChooser
	{
		Piece	&operator()( Piece &firstPiece, PieceVector &secondPieces )
		{
			register	size_t	i, iBest = 0, iMax = secondPieces.size();
			for( i = 1; i < iMax; ++i )
				if( secondPieces[i] < secondPieces[iBest] )
					iBest = i;
			return secondPieces[iBest];
		}
	};

	void solve()	{ coreSolve( ChooseGreedy() ); }

	tod_t		tCur;	// Current position in the day of the cut.
	RunList		runs;
	PieceList	uncut;
	double		cost;
	tod_t		uncutTotal;
	tod_t		workTotal;

	double		getWorkCutRatio() const { return (double)uncutTotal / (double)workTotal; }
	size_t		getNumRuns() const { return runs.size(); }

	typedef bool	(*Callback)( void * );
	void		setCallback( Callback aCallback, void *aData = NULL ) { callback = aCallback; callbackData = aData; }
	bool		keepGoing() const { return callback ? (*callback)(callbackData) : true; }


	typedef bool	(*CheckFeasibleCallback)( void *, Run &r, Runcut &rc );
	void		setFeasibleCallback( CheckFeasibleCallback aCheckFeasibleCallback, void *aData = NULL )
	{ checkFeasibleCallback = aCheckFeasibleCallback; checkFeasibleCallbackData = aData; }

	HashObj<NodeID, int>	infeasibleFirst, infeasibleLast;

	virtual	size_t	getIteration() const { return 0; }

protected:
	Callback	callback;
	void		*callbackData;

	CheckFeasibleCallback	checkFeasibleCallback;
	void		*checkFeasibleCallbackData;

	bool	checkFeasible( const Piece &first, const Piece &second )
	{
		if( !checkFeasibleCallback )
			return true;
		Run	r;
		r.piece[0] = first;
		r.piece[1] = second;
		return (*checkFeasibleCallback)(checkFeasibleCallbackData, r, *this);
	}

	BlockList	blocks;
	BlockList	emptyBlocks;

	size_t		choicesMax;
	void coreSolve( PieceChooser &pieceChooser, const size_t numChoices = 1 );

	double	runCost( const Run &r ) const
	{
		const	tod_t	W = r.workTime();
		double	cost = regularCostPerHour * (W < workPayMin ? workPayMin : W)/(60.0*60.0);
		const	tod_t	S = r.spreadTime();
		if( S > spreadMax )
			cost += additionalPenaltyCostPerHour * (S - spreadMax)/(60.0*60.0);
		return cost;
	}

	void	findBestSecondPieces( PieceVector &secondPieces, const Piece &firstPiece );
	void	removeOverlapping( const Piece &p );
	bool	hasOverlap( const Piece &p );

	void	getUncutWork();

	struct IDPair
	{
		IDPair( const ident_t aX = -1, const ident_t aY = -1 ) : x(aX), y(aY) {}
		ident_t	x, y;

		int	cmp( const IDPair &p ) const
		{
			return MC::multi_cmp( x, MC::less, p.x,
								  y, MC::less, p.y );
		}
		bool operator==( const IDPair &p ) const { return cmp(p) == 0; }
		bool operator<( const IDPair &p ) const { return cmp(p) < 0; }

		size_t	hash() const	{ return multi_hash(x, y); }
	};
	typedef HashObj<IDPair, tod_t>	Equivalences;
	Equivalences	equivalences;

	bool	compatibleLocations( const ident_t x, const ident_t y ) const
	{ return x == y || equivalences.find(IDPair(x, y)) != equivalences.end(); }

	tod_t	getConnectionTime( const ident_t x, const ident_t y )
	{
		Equivalences::iterator i = equivalences.find(IDPair(x,y));
		if( i == equivalences.end() )
			return x == y ? 0 : -1;
		return i.value();
	}
};

} // namespace BinPackRuncut
#endif // BinPackRuncut_H