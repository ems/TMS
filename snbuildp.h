//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif
#define SNzeroCostLayover	1
void	SNbuildProlog();
void	SNbuildAddTrip( TRIPINFODef *pt );
void	SNbuildAddDeadhead( long startNode, long endNode,
							tod_t start, tod_t end, tod_t dur );
void	SNbuildAddWaits( int zeroCostLayovers );
void	SNbuildEpilog( tod_t vehicleMin, tod_t vehicleMax );
#ifdef __cplusplus
} // extern "C"
#endif
