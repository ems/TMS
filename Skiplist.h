//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef SKIPLIST_H
#define SKIPLIST_H

#define __MS_DOS__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <compare.h>

#pragma warning(disable : 4291)

// custom types for return values and modes of operation
// for the skip list.

// not in class due to a template bug in Borland C++.
        
typedef enum {SL_OK, SL_NOT_FOUND, SL_ERROR} slRetStatus;
typedef enum {SL_UNIQUE, SL_NON_UNIQUE} slOpType;

template <class T> class skipList;
template <class T> class skipListIter;

template <class T>
class skipNode
{
    friend class skipList<T>;
    friend class skipListIter<T>;
    
    private:
        skipNode (T *d, const int l) :
            nodeData (d), level (l) {}
        ~skipNode () {}

        // determines the number of levels within this node.
        
        int level;

        // The nodeData type is a parametric data type which must
        // have the == and the < operators defined.
        
        T *nodeData;

        // The series of next pointers to different levels of next nodes
        // are grown dynamically by the private new operator.  This must
        // be the last data member within this class for it to work
        // properly.
        
        skipNode<T> *next[1];

        void *operator new (size_t x, const int level)
        {
            void *ptr;
	    const int lev = level - 1; // all nodes have level >= 1 - compensate for this.
	    const unsigned size = x + sizeof (skipNode<T> *) * lev;

	    ptr = (void *) ::new char[size];

            return ptr;
        }

        void operator delete (void *buf) 
        {
	    if( buf != NULL )
	        ::delete [] (char *)buf;
	}
}; // skipNode<T>
#undef SkipNodeMemMgrLevels

template <class T>
class skipList
{
    // The iterator must be a friend of this class so that it can access
    // its update pointers, and also obtain valuable internal information
    // regarding its maximum level and etc.
    
friend class skipListIter<T>;
    
  public:

    // All of these constructors are necessary to be backwards compatible
    // with existing code.  The skipList is now adaptive in its height.
    skipList (Comparison<T> *func); // Default is SL_NON_UNIQUE
    skipList (Comparison<T> *func, const slOpType dup);

    // The following definitions are declared for backwards compatibility only...
    skipList (Comparison<T> *func, const int level); // Default is SL_NON_UNIQUE
    skipList (Comparison<T> *func, const int level, const slOpType dup);

    ~skipList ();           

    inline unsigned long entries (void) const;
    slRetStatus find (T *d, T **found = NULL) const;
    slRetStatus first( T **found ) const
    {
        return numEntries > 0 ? (*found = header->next[0]->nodeData, SL_OK)
                              : SL_NOT_FOUND;
    }
    slRetStatus insert (T *d);
    slRetStatus insertAll(const skipList<T> * sl);
    slRetStatus remove (T *d, T **found = NULL);
    int clear (void);
    int clearAndDelete (void);

    // "==" operator;  
    const int operator==(const skipList<T> & s) const {return (this == &s);}

  private:
    // random level generation with own random number generator.
    int randLevel (void);

    // function to init the skipList after the userMaxLevel
    // has been set.
    void init(Comparison<T> *func, const slOpType dup);

    // seed for randLevel;
#ifdef __MS_DOS__
    long seed;
#else
    unsigned long seed;
#endif // __MS_DOS__

    unsigned long numEntries;

    // The header node has no value except to point to the other nodes
    // that are one link away from it at all levels.
    skipNode<T> *header;

    // An array of pointers to the previous node of the current node so
    // that splicing and relink can be accomplished.  This array is
    // also used to traverse the list.  Hence update[x] represent a
    // pointer to the node at level x that requires its "next" pointer
    // to be updated, and update[0]->next[0] represents the current
    // node.  The update pointers are reset to the header when the
    // function first is used, and is affected when any of the
    // traversing functions are used.
    
    skipNode<T> **update;

    int userMaxLevel; // maximum level supplied by the user
    slOpType opFlag;  // a flag indicating whether duplicates are allowed

    // comparison function is required to compare between two type T.
    Comparison<T> *compFunc;

}; // skipList

// If the member function find is used all iterators relating to the skipList
// is reset to the result of the find.

template <class T>
class skipListIter
{
  public:

    skipListIter (skipList<T> *l) { list = l; cur = l->header; }
    ~skipListIter () {}
    
    slRetStatus first (T **found);
    slRetStatus findFirst (T *d, T **found = NULL);
    slRetStatus findFirstClosest (T *d, T **found = NULL);
    slRetStatus current (T **found) const;
    slRetStatus next (T **found);

    skipListIter<T> &operator = ( const skipListIter<T> &itr )
    {
	list = itr.list;
	cur = itr.cur;
	return *this;
    }

    int operator == ( const skipListIter<T> &itr ) const
    {
	return list == itr.list && cur == itr.cur;
    }

  private:
    skipList<T> *list;
    skipNode<T> *cur;
		
}; // skipListIter

template <class T>
inline unsigned long skipList<T>::entries (void) const
{
    return numEntries;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
skipList<T>::skipList (Comparison<T> *func,
		       const int level, const slOpType dup)

[begindesc
Purpose:    Construct a skipList instance with a particular maximum level
            and denote's whether the list should contain duplicates or not.
enddesc]
endproto]
============================================================================*/

template <class T>
void skipList<T>::init(Comparison<T> *func, const slOpType dup)
{
    header = new (userMaxLevel) skipNode<T>(NULL, userMaxLevel);
    update = new skipNode<T> * [userMaxLevel];

    for (int i = 0; i < userMaxLevel; i++)
    {
        header->next[i] = NULL;
        update[i] = header;
    }
    
    opFlag = dup;
    seed = 1L;
    compFunc = func;
    numEntries = 0L;
}

enum { userMaxLevelInit = 3 };

template <class T>
skipList<T>::skipList (Comparison<T> *func)
{
    userMaxLevel = userMaxLevelInit;
    init(func, SL_NON_UNIQUE);
}

template <class T>
skipList<T>::skipList (Comparison<T> *func, const slOpType dup)
{
    userMaxLevel = userMaxLevelInit;
    init(func, dup);
}

template <class T>
skipList<T>::skipList (Comparison<T> *func, const int /* level */)
{
    userMaxLevel = userMaxLevelInit;
    init(func, SL_NON_UNIQUE);
}

template <class T>
skipList<T>::skipList (Comparison<T> *func, const int /* level */, const slOpType dup)
{
    userMaxLevel = userMaxLevelInit;
    init(func, dup);
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
skipList<T>::~skipList ()

[begindesc
Purpose:    Remove the extra memory allocated for the header and the update
            pointers.
enddesc]
endproto]
============================================================================*/

template <class T>
skipList<T>::~skipList ()
{
    clear ();
    delete header;
    delete [] update;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
int skipList<T>::randLevel (void)

[begindesc
Purpose:    Determine a random level for a new node.  This version is set
            so that the distribution is as follows:

                the probability of a node having level n, where p = 1/4 is

                (1 - p) * (p)^(n - 1) * 100% yeilding:
                
                75.00% probability for having level 1 node.
                18.75% probability for having level 2 node.
                 4.69% probability for having level 3 node.
                 1.17% probability for having level 4 node.
                 0.29% probability for having level 5 node.
                 0.07% probability for having level 6 node.
                 0.02% probability for having level 7 node.

Return:     Returns the number of level.
enddesc]
endproto]
============================================================================*/

template <class T>
int skipList<T>::randLevel (void)
{

    // self random number generator (can't trust any random generator on any
    // platforms.  -- same as rand(3)

#ifdef __MS_DOS__
    static const unsigned long multiplier = 22695573L;
    static const unsigned long increment = 1L;
#else
    static const unsigned long m = 2147483647;
    static const unsigned long q = 127773;

    static const unsigned int a = 16807;
    static const unsigned int r = 2836;

    register int test;
#endif // __MS_DOS__

    int rnum;
    int i;

    for (i = 0; i < userMaxLevel - 1; i++)
    {
#ifdef __MS_DOS__
        seed = multiplier * seed + increment;
        rnum = (seed >> 16) & 0x0FFFFL;
#else
	test = (int) (a*(seed % q) - r*(seed / q));
	seed = (test > 0 ? test : test + m);
        rnum = (int) seed;
#endif
        if (rnum & (3UL << i))
            break;
    }

    return i + 1;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
slRetStatus skipList<T>::find (T *d, T **found) const

[begindesc
Purpose:    Find a node that has a data element that is equal to the
            parameter d, and sets up the update pointers to point to
            the last nodes (at all levels) that are less than the requested
            data element d.

Return:     If found, returns SL_OK and sets found to the found data
            element within the list, else returns SL_NOT_FOUND, and
            found is either NULL, if the list is exhausted, or found points
            to the closest data element (not exceeding).
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipList<T>::find (T *d, T **found) const
{
    // the return element is always set to null if the element is not
    // found or if the list is empty.

    if (found != NULL)
        *found = (T *) NULL;

    if (numEntries <= 0L)
        return SL_NOT_FOUND;

    skipNode<T> *p = header;
    
    // traverse the levels from top to bottom
    for (int level = userMaxLevel - 1; level >= 0; level--)
    {
        // traverse the links in a level
        while (p->next[level] != NULL)
        {
            // if the next node is less than the wanted object then
            // continue with the link.
            
            if (compFunc->compare ((*(p->next[level]->nodeData)), (*d)) < 0)
                p = p->next[level];
            else
                break;
        }
        update[level] = p;
    } // end for

    // even if found is not successful, will will return the data element
    // that is the closest (but not exceeding) the search element (d)

    if (found != NULL)
        *found = p->nodeData;

    p = update[0]->next[0];
    
    if (p != NULL && compFunc->compare ((*(p->nodeData)), (*d)) == 0)
    {
        if (found != NULL)
            *found = p->nodeData;
        return SL_OK;
    }

    return SL_NOT_FOUND;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
slRetStatus skipList<T>::insert (T *d)

[begindesc
Purpose:    Insert a data element into the list, and sets the update
            pointers.

Return:     If successful returns SL_OK, else SL_ERROR.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipList<T>::insert (T *d)
{
    // find the place to insert the new item and check if it unique.
    if (find (d) == SL_OK && opFlag == SL_UNIQUE)
        return SL_ERROR;

    // create a new skip node to put into the list.
    int level = randLevel ();
    skipNode<T> *newNode = new (level) skipNode<T> (d, level);

    // splice in the new node at all "relevant" levels
    level--;
    do
    {
        newNode->next[level] = update[level]->next[level];
        update[level]->next[level] = newNode;
    } while (--level >= 0);

    // Check if we have exceeded the performance of a skiplist of
    // this size (is numEntries > 4**userMaxLevel == 2**(userMaxLevel*2))
    // Actually, we check for numEntries to be 1/4 the maximum elements
    // the list can take so that some taller nodes can be built in advance.
    // This minimizes the skewing of the list and improves performance
    // slightly.
    if( ++numEntries > (unsigned long)(1L<<((userMaxLevel-1)<<1L)) )
    {
	// Performance of the skiplist is starting to degrade
	// because there are too many items in it.

	// Increase the height of the skiplist.
	const int userMaxLevelNew = userMaxLevel + 1;
	skipNode<T> *headerNew = new (userMaxLevelNew) skipNode<T>(NULL, userMaxLevelNew);
	skipNode<T> **updateNew = new skipNode<T> * [userMaxLevelNew];
	
	for (int i = 0; i < userMaxLevel; i++)
	{
	    headerNew->next[i] = header->next[i];
	    updateNew[i] = headerNew;
	}
	headerNew->next[userMaxLevel] = NULL;
	updateNew[userMaxLevel] = headerNew;

	delete header;
	delete [] update;

	header = headerNew;
	update = updateNew;
	userMaxLevel = userMaxLevelNew;
    }

    return SL_OK;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
slRetStatus skipList<T>::insertAll (skipList<T> &sl)

[begindesc
Purpose:    To assign all the contents of one skiplist to another.
	    This can be used to re-sort items already contained in one
	    skipList by inserting them into another skipList with a
	    different comparison class.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipList<T>::insertAll (const skipList<T> *sl)
{
    if( sl == this )
	return SL_OK;

    clear ();
    skipNode<T> *p = sl->header;
    for( skipNode<T> *n = p->next[0]; n != NULL; n = n->next[0] )
	this->insert(n->nodeData);

    return SL_OK;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
slRetStatus skipList<T>::remove (T *d, T **found)

[begindesc
Purpose:    Remove a data element into the list, and sets the update
            pointers.  Effectively moving to the next node.

Return:     If successful returns SL_OK, else SL_NOT_FOUND.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipList<T>::remove (T *d, T **found)
{
    if (find (d, found) != SL_OK)
        return SL_NOT_FOUND;

    skipNode<T> *p = update[0]->next[0];
    int level = p->level - 1;
    do
    {
        update[level]->next[level] = p->next[level];
    } while (--level >= 0);
    delete p;
	
    --numEntries;

    return SL_OK;
}

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
int skipList<T>::clear (void)

[begindesc
Purpose:    remove all nodes

Return:     number of nodes removed
enddesc]
endproto]
============================================================================*/

template <class T>
int skipList<T>::clear (void)
{
    skipNode<T> *p = header;
    skipNode<T> *n = p->next[0];
    int count = 0;
    
    while (n != NULL)
    {
	p = n;
	n = n->next[0];
	delete p;
	count++;
    }
    
    for (int i = 0; i < userMaxLevel; i++)
    {
        header->next[i] = NULL;
        update[i] = header;
    }
	
    numEntries = 0L;
    return count;
}

//----------------------------------------------------------------------------
// clearAndDelete will only work with elements that have a public destructor
// which is not always the case.
//----------------------------------------------------------------------------

/*=cdoc[skipList<T>]==========================================================
[beginproto

template <class T>
int skipList<T>::clearAndDelete (void)

[begindesc
Purpose:    remove all nodes and delete its elements

Return:     number of nodes removed
enddesc]
endproto]
============================================================================*/

template <class T>
int skipList<T>::clearAndDelete (void)
{
    skipNode<T> *p = header;
    skipNode<T> *n = p->next[0];
    int count = 0;

    while (n != NULL)
    {
	p = n;
	n = n->next[0];
	delete p->nodeData;
	delete p;
	count++;
    }
    
    for (int i = 0; i < userMaxLevel; i++)
    {
        header->next[i] = NULL;
        update[i] = header;
    }

    numEntries = 0L;
    return count;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*=cdoc[skipListIter<T>]=======================================================
[beginproto

template <class T>
slRetStatus skipListIter<T>::current (T **found) const

[begindesc
Purpose:    Returns the current node.  If find is used before this, then the
            current will indicate the found element.

Return:     If found, returns SL_OK, else returns SL_NOT_FOUND.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipListIter<T>::current (T **found) const
{
    skipNode<T> *f;
    
    if (cur != NULL && (f = cur->next[0]) != NULL)
    {
        *found = f->nodeData;
        return SL_OK;
    }
    return SL_NOT_FOUND;
}

/*=cdoc[skipListIter<T>]=======================================================
[beginproto

template <class T>
slRetStatus skipListIter<T>::next (T **found)

[begindesc
Purpose:    Returns the next node and advances the lowest level of
            update pointers.

Return:     If found, returns SL_OK, else returns SL_NOT_FOUND.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipListIter<T>::next (T **found)
{
    skipNode<T> *f, *s;

    if (cur != NULL
	&& (f = cur->next[0]) != NULL
	&& (s = f->next[0]) != NULL)
    {
        *found = s->nodeData;
	cur = f;
        return SL_OK;
    }
    return SL_NOT_FOUND;
}

/*=cdoc[skipListIter<T>]=======================================================
[beginproto

template <class T>
slRetStatus skipListIter<T>::first (T **found)

[begindesc
Purpose:    Returns the first node.

Return:     If found, returns SL_OK, else returns SL_NOT_FOUND.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipListIter<T>::first (T **found)
{
    cur = list->header;

    if (cur->next[0] != NULL)
    {
        *found = cur->next[0]->nodeData;
        return SL_OK;
    }
    return SL_NOT_FOUND;
}

/*=cdoc[skipListIter<T>]=======================================================
[beginproto

template <class T>
slRetStatus skipListIter<T>::findFirst (T*d, T **found)

[begindesc
Purpose:    Returns the first node matching the given key d.
	    This allows the iterator to move forward from this
	    initial found key a bit like a database cursor.

Return:     If found, returns SL_OK, else returns SL_NOT_FOUND.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipListIter<T>::findFirst (T* d, T **found)
{
    slRetStatus ret = list->find( d, found );
    cur = list->update[0];

    return ret;
}


/*=cdoc[skipListIter<T>]=======================================================
[beginproto

template <class T>
slRetStatus skipListIter<T>::findFirstClosest (T* d, T **found)

[begindesc
Purpose:    Returns the closest node to given d.
	    This allows the iterator to move forward from this
	    initial found key a bit like a database cursor.

Return:     If found, returns SL_OK, else returns SL_NOT_FOUND.
enddesc]
endproto]
============================================================================*/

template <class T>
slRetStatus skipListIter<T>::findFirstClosest (T* d, T **found)
{
    T *firstFound;

    slRetStatus ret = list->find( d, &firstFound );

    if( ret == SL_OK )
    {
        if( found )
            *found = firstFound;
    }
    else if( firstFound != NULL )
        ret = list->find( firstFound, found );
            
    cur = list->update[0];

    return ret;
}

#endif // SKIPLIST_H
