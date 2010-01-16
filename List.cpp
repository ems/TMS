//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "List.h"

MMinitMax( DListBase::Element, 1024 );
MMinitMax( SListBase::Element, 1024 );

void DListBase::clear()
{
	register Element *e, *eNext;
	for( e = head; e != NULL; e = eNext )
	{
		eNext = e->next;
		delete e;
	}
	head = NULL;
	numElements = 0;
}

DListBase &DListBase::operator=( const DListBase &list )
{
	if( this != &list )
	{
		clear();
		register Element *e, *eNew;
		Element *tail = NULL;

		for( e = list.head; e != NULL; e = e->next )
		{
			eNew = new Element;
			eNew->data = e->data;
			eNew->addToListTail( head, tail );
		}
		numElements = list.numElements;
	}
	return *this;
}

void SListBase::clear()
{
	register Element *e, *eNext;
	for( e = head; e != NULL; e = eNext )
	{
		eNext = e->next;
		delete e;
	}
	head = NULL;
	numElements = 0;
}

SListBase &SListBase::operator=( const SListBase &list )
{
	if( this != &list )
	{
		clear();
		register Element *e, *eNew;
		Element *tail = NULL;

		for( e = list.head; e != NULL; e = e->next )
		{
			eNew = new Element;
			eNew->data = e->data;
			eNew->addToListTail( head, tail );
		}
		numElements = list.numElements;
	}
	return *this;
}
