//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include <rundef.H>
#include <stdio.h>

const char *asTime( tod_t s )
{
    int	hour, minute, second;
    hour = (int)(s / (60L * 60L));
    s -= hour * 60L * 60L;
    minute = (int)(s / 60L);
    s -= minute * 60L;
    second = (int)s;

    static	char	sz[16];
    sprintf( sz, "%02d%02d", hour, minute );
    return sz;
}
