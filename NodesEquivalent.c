#include "TMSHeader.h"

//
//  See if the CFLAG_EQUIV flag is set for this node pair
//
BOOL NodesEquivalent(long fromNODESrecordID, long toNODESrecordID, long *pEquivalentTravelTime)
{
  int rcode2;

  *pEquivalentTravelTime = 0;

  if(fromNODESrecordID == toNODESrecordID)
    return(TRUE);  // Taken out because it wouldn't look up A to A to get the "travel" time

  CONNECTIONSKey1.fromNODESrecordID = fromNODESrecordID;
  CONNECTIONSKey1.toNODESrecordID = toNODESrecordID;
  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  while(rcode2 == 0 &&
        CONNECTIONS.fromNODESrecordID == fromNODESrecordID &&
        CONNECTIONS.toNODESrecordID == toNODESrecordID)
  {
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
    {
      *pEquivalentTravelTime = CONNECTIONS.connectionTime;
      return(TRUE);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  }
  CONNECTIONSKey1.fromNODESrecordID = toNODESrecordID;
  CONNECTIONSKey1.toNODESrecordID = fromNODESrecordID;
  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  while(rcode2 == 0 &&
        CONNECTIONS.fromNODESrecordID == toNODESrecordID &&
        CONNECTIONS.toNODESrecordID == fromNODESrecordID)
  {
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT &&
          CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY)
    {
      *pEquivalentTravelTime = CONNECTIONS.connectionTime;
      return(TRUE);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  }

  return(FALSE);
}
