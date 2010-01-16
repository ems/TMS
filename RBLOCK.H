//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef rblock_H
#define rblock_H

class LPGen;
class RunChunk;
class RBlock;

class ReliefPoint
{
public:
    ReliefPoint( const tod_t aT, const long aRi = -1L );
    ~ReliefPoint();

    void addToList( ReliefPoint *&head, ReliefPoint *&tail )
    {
	if( head == NULL )
	    head = this;
	else
	    tail->next = this;
	tail = this;
	next = NULL;
    }
    void addToPieceList( ReliefPoint *&head )
    {
	if( head != NULL )
	    head->prevPiece = this;
	nextPiece = head;
	head = this;
	prevPiece = NULL;
    }
    void removeFromPieceList( ReliefPoint *&head )
    {
	if( prevPiece != NULL )
	    prevPiece->nextPiece = nextPiece;
	else
	    head = nextPiece;
	if( nextPiece != NULL )
	    nextPiece->prevPiece = prevPiece;
    }
    double getBoundaryCost( const tod_t multiple ) const;
    long getBlockNum() const;
    ReliefPoint	*from() const { return pred; }
    ReliefPoint *to() { return this; }
    tod_t	getT() const { return t; }

private:
    tod_t	t;		// the time of this relief point

public:
    // Index into lpgen chunks structure
    // Will be NULL if this piece does not correspond to a valid work piece.
    RunChunk    *i;

    int		done;		// a flag indicating that we are done.

    ReliefPoint	*pred;		// pointer to RP predecessor.
    double	cost;		// cost of going to the RP predecessor.

    ReliefPoint	*next;		// next relief point in this block.

    ReliefPoint	*nextPiece;	// a pointer to the next piece of this type.
    ReliefPoint *prevPiece;

    RBlock	*owner;
    long        ri;             // index of this relief point in the array.
};

class RBlock
{
public:
    RBlock( const long aNum = 0L );
    ~RBlock();

    void addToList( RBlock *&head, RBlock *&tail )
    {
	if( head == NULL )
	    head = this;
	else
	    tail->next = this;
	tail = this;
	next = NULL;
    }
    void addRelief( const tod_t t, const long nodeID = -1L )
    {
	// Only add relief points with unique times.
	if( tail == NULL || tail->getT() != t )
	{
	    ReliefPoint *rp = new ReliefPoint(t, nodeID);
	    rp->owner = this;
	    rp->addToList(head, tail);
	}
    }

    double solve( LPGen &g, const tod_t minLeftoverPiece, const tod_t multiple );
    void putBack( LPGen &g );

    void	removeDuplicateReliefs();

    double	cost;		// cost of the heuristic cut of this block.
    ReliefPoint	*head, *tail;
    long	num;		// block number

    RBlock	*next;
};

ostream &operator<<( ostream &os, const RBlock &rb );

inline    long ReliefPoint::getBlockNum() const { return owner->num; }

#endif // rblock_H
