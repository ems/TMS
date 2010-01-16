//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef LPFILE_H
#define LPFILE_H

//#define KEEP_FILES_HERE
#if defined(UNIX) || defined(KEEP_FILES_HERE)
#define probFileName "lpprob.txt"
#define solFileName  "lpsol.txt"
#else
#define probFileName "c:\\windows\\lpprob.txt"
#define solFileName  "c:\\windows\\lpsol.txt"
//#define probFileName "c:\\devel\\tms\\src\\lpprob.txt"
//#define solFileName  "c:\\devel\\tms\\src\\lpsol.txt"
#endif

#endif // LPFILE_H
