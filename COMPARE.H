//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef COMPARE_H
#define COMPARE_H

template <class T>
class Comparison {
public:
	virtual ~Comparison(){}
	virtual int compare(const T&, const T&) = 0;
protected:
	Comparison(){}
};

#endif // COMPARE_H

