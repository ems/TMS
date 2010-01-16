//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef cover_H
#define cover_H

#include "HashFI.h"

typedef unsigned int ident_t;

class Cover;
class Column;
class Row;
class IncreasingContrib;

typedef HashFI< Column *, Column * > columnHT;
typedef HashFI< ident_t, Column * > idColumnHT;
typedef HashFI< ident_t, Row * > idRowHT;
typedef HashFI< Row *, Row * > rowHT;

#define ForAllInCollection( c, i ) \
	for( i = c.begin(); !(i == c.end()); ++i )

class Row
{
private:
	friend class Cover;
	friend class Column;

	Row( ident_t aID, const unsigned int b )
	{
		id = aID;
		bOriginal = b;
		init();
	}
	void	init() { bCurrent = 0; solutionColumns.clear(); }
	void	cover( Column *c );
	void	expose( Column *c );

	unsigned int	bOriginal, bCurrent;
	ident_t	id;

	int	isUnderCovered() const { return bCurrent < bOriginal; }
	int	isCovered() const { return bCurrent == bOriginal; }
	int isOverCovered() const { return bCurrent > bOriginal; }

	columnHT	solutionColumns;
};

class Column
{
private:
	friend class Cover;
	friend class Row;
	friend class IncreasingContrib;

	Column( ident_t aID, unsigned int aCost = 1 )
	{
		id = aID;
		cost = aCost;
	}

	void	addRow( Row *r )  { rows.insertKey(r); }

	void	setCost( unsigned int aCost ) { cost = aCost; }
	unsigned int getCost() const { return cost; }

	ident_t	id;
	unsigned int cost;
	int	value;

	double	contribution();
	double	lastContribution;

	rowHT	rows;
};

inline void Row::cover( Column *c )
{
	bCurrent++;
	if( c->value == 1 ) solutionColumns.insertKey(c);
}
inline void Row::expose( Column *c )
{	
	bCurrent--;
	if( c->value == 0 ) solutionColumns.remove(c);
}

class Cover
{
public:
	Cover( const size_t numColumns = 0, const size_t numRows = 0 )
		: columns(numColumns), rows(numRows), solution(numRows)
	{
		cost = 0;
	}

	void	addColumn( ident_t id, unsigned int cost = 1 )
	{ columns.insert( id, new Column(id, cost) ); }
	void	addRow( ident_t id, unsigned int b )
	{ rows.insert( id, new Row(id, b) ); }
	void	addMatrix( ident_t colID, ident_t rowID )
	{
		idColumnHT::iterator c = columns.find(colID);
		if( !(c == columns.end()) )
		{
			idRowHT::iterator r = rows.find(rowID);
			if( !(r == rows.end()) )
				c.data()->addRow( r.data() );
		}
	}
	
	// returns 1 if optimal, 0 if infeasible
	int	solve();

	unsigned int getValue( const id )
	{
		idColumnHT::iterator c = columns.find(id);
		return c == columns.end() ? 0 : c.data()->value;
	}
	unsigned int getCost() const { return cost; }

private:
	int	solveCover( const unsigned int NMax );

	idColumnHT		columns;
	idRowHT			rows;

	columnHT		solution;
	unsigned int	cost;
};

#endif // cover_H