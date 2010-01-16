//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef insectc_H
#define insectc_H
typedef int coord_t;

/* return values */
#define DONT_INTERSECT 0
#define DO_INTERSECT   1
#define COLLINEAR      2

int lines_intersect( const coord_t x1, const coord_t y1, const coord_t x2, const coord_t y2,
					const coord_t x3, const coord_t y3, const coord_t x4, const coord_t y4,
					coord_t &x, coord_t &y);

#endif // insectc_H