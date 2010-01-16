//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

#define RCFLAGS_NONE         0
#define RCFLAGS_ACCESSIBLE  (1<<0)
#define RCFLAGS_NODEGAR     (1<<1)
#define RCFLAGS_PULLOUT     (1<<2)
#define RCFLAGS_PULLIN      (1<<3)
#define MAXSPANENTRIES      100

typedef struct RUNTRIPDATAStruct
{
  long assignedToNODESrecordID;
  long blockNumber;
  long blockPOGNODESrecordID;
  long blockPIGNODESrecordID;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long PATTERNNAMESrecordID;
  long BUSTYPESrecordID;
  char flags;
} RUNTRIPDATADef;

typedef struct RUNTRIPStruct
{
  RUNTRIPDATADef from[MAXPIECES];
  RUNTRIPDATADef to[MAXPIECES];
} RUNTRIPDef;

typedef struct SpanCheckStruct
{
  long ATime;
  long BTime;
  char ALabel;
  char BLabel;
} SpanCheckDef;

long GetClosestGarage(long, RUNTRIPDef *, long *, long *, long, long, int, int);
#define CLOSEST_FLAG_BEATGARAGEBEFORETIME 0x0001
#define CLOSEST_FLAG_REVERSE              0x0002
BOOL GetPieceTravel(BOOL, int, int, int, int, RUNTRIPDef *, PROPOSEDRUNDef *, COSTDef *, long *, long *);
long GetPaidTravel(int, long);
BOOL IsGarage(long);
long GetGarageTravel(long, PROPOSEDRUNPIECESDef *, PREMIUMDef *, RUNTRIPDef *);
void FillRUNTRIP(RUNTRIPDef *, PROPOSEDRUNDef *, int, BOOL);

long RunCoster(PROPOSEDRUNDef *pPR, long cutAsRuntype, COSTDef *pCOST)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  RUNTRIPDef RUNTRIP;
  SpanCheckDef SpanCheck[MAXSPANENTRIES];
  float distance;
  long actualBreak;
  long bump;
  long longest;
  long onTime;
  long offTime;
  long pieceSize;
  long breakTime;
  long travelTime;
  long actualSpreadTime;
  long platformTime;
  long sumOfTimes;
  long bestTime;
  long bestTime1;
  long bestTime2;
  long bestTime3;
  long bestTime4;
  long contigTime;
  long minutesThatDontCount;
  long startLocation;
  long endLocation;
  long dwellTime;
  long dwellTime1;
  long dwellTime2;
  long paidTravel;
  long tempLong;
  long deadheadTime;
  long startTime;
  long endTime;
  long equivalentTravelTime;
  long startOfBreak;
  long endOfBreak;
  long *pTime[MAXPREMIUMS][MAXPIECES];
  long junkHeap;
  long minLayNotBefore;
  long previousLastNode;
  long previousRouteRecordID;
  long previousServiceRecordID;
  long previousPatternNamesRecordID;
  long currentSpanStart;
  long dailyOvertime;
  long spreadOvertime;
  long totalDontIncorporate;
  BOOL bGotMinLayover;
  BOOL bJustContinued;
  BOOL bDoIt;
  BOOL bAlreadyPaid[MAXPIECES];
  BOOL bCheckRunMinBreak, bGotRunMinBreak;
  BOOL bCheckRunMaxBreak, bGotRunMaxBreak;
  BOOL bCheckContigTime;
  BOOL bFound;
  BOOL bOKToPayStartEnd;
  BOOL bOKToPayBC;
  BOOL bSameNode;
  BOOL bStillLooking;
  BOOL bPaidBreak[MAXPIECES];
  BOOL bValidTravel;
  BOOL bPassedTest[MAXPREMIUMS][MAXPIECES];
  int  nI;
  int  nJ;
  int  nK;
  int  nL;
  int  rcode2;
  int  OTIndex;
  int  lastPiece;
  int  serviceIndex;
  int  nType;
  int  nSlot;
  int  longestBreakStartIndex;
  int  numEntries;

  int keyNumber;
//
//  Set up the index into the node labels string
//
  serviceIndex = (int)(m_ServiceNumber - 1);
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
//
//  Establish the last piece of the run
//
  lastPiece = pPR->numPieces - 1;
//
//  Clean out pCOST
//
  memset(pCOST, 0x00, sizeof(COSTDef));
//
//  And the node references in TRAVEL too
//
  for(nI = 0; nI < MAXPIECES; nI++)
  {
    pCOST->TRAVEL[nI].startNODESrecordID = NO_RECORD;
    pCOST->TRAVEL[nI].startAPointNODESrecordID = NO_RECORD;
    pCOST->TRAVEL[nI].endBPointNODESrecordID = NO_RECORD;
    pCOST->TRAVEL[nI].endNODESrecordID = NO_RECORD;
  }
//
//  Send it back if it's just a placeholder
//
  if(cutAsRuntype != NO_RUNTYPE && cutAsRuntype != UNCLASSIFIED_RUNTYPE)
  {
    nType = (short int)LOWORD(cutAsRuntype);
    nSlot = (short int)HIWORD(cutAsRuntype);

    if(RUNTYPE[nType][nSlot].flags & RTFLAGS_PLACEHOLDER)
    {
      pCOST->runtype = cutAsRuntype;
      return(pCOST->runtype);
    }
  }
//
//  If necessary, figure out the runtype
//
  LoadString(hInst, TEXT_081, runcosterReason, sizeof(runcosterReason));
//
//  If the runtype was passed in and we're not checking the integrity
//  of the runtype, then there's no point in determining it
//
  if(cutAsRuntype != NO_RUNTYPE &&
        cutAsRuntype != UNCLASSIFIED_RUNTYPE &&
        !(CUTPARMS.flags & CUTPARMSFLAGS_CHECKRUNTYPE))
  {
    pCOST->runtype = cutAsRuntype;
  }
//
//  It isn't/wasn't, so let's see what it is
//
  else
  {
    pCOST->runtype = UNCLASSIFIED_RUNTYPE;
//
//  Loop through all the runtypes
//
    for(nType = 0; nType < NUMRUNTYPES; nType++)
    {

//
//  If we've found a match, leave
//
      if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
        break;
//
//  Loop through all the slots
//
      for(nSlot = 0; nSlot < NUMRUNTYPESLOTS; nSlot++)
      {
//
//  If we're not considering this runtype, continue on through
//
        if(m_bCheckUnderConsideration)
        {
          if(!CUTPARMS.runtypes[nType][nSlot])
          {
            continue;
          }
        }
//
//  Again, if we've found a match, leave
//
        if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
          break;
//
//  If this runtype is in use, and we've got the right number of pieces, then check it out
//
        if(RUNTYPE[nType][nSlot].flags & RTFLAGS_INUSE &&
              RUNTYPE[nType][nSlot].numPieces == pPR->numPieces)
        {
//
//  See if we have to check minimum and maximum breaks, and the contiguous piece time(s)
//
          bCheckRunMinBreak = RUNTYPE[nType][nSlot].minBreakOf != NO_TIME && pPR->numPieces > 2;
          bGotRunMinBreak = FALSE;
          bCheckRunMaxBreak = RUNTYPE[nType][nSlot].maxBreakOf != NO_TIME && pPR->numPieces > 2;
          bGotRunMaxBreak = FALSE;
          bCheckContigTime = RUNTYPE[nType][nSlot].maxContig != NO_TIME && pPR->numPieces > 2;
          contigTime = 0;
//
//  Loop through the pieces
//
          for(nK = 0; nK < pPR->numPieces; nK++)
          {
//
//  Assume we're still looking, and that we haven't found anything
//
            bStillLooking = TRUE;
            if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
              break;
//
//  Match the minimum and maximum on and off times to this runtype's properties
//
            onTime = pPR->piece[nK].fromTime;
            offTime = pPR->piece[nK].toTime;
            if(RUNTYPE[nType][nSlot].PIECE[nK].minOnTime != NO_TIME &&
                  onTime < RUNTYPE[nType][nSlot].PIECE[nK].minOnTime)
              break;
            if(RUNTYPE[nType][nSlot].PIECE[nK].maxOnTime != NO_TIME &&
                  onTime > RUNTYPE[nType][nSlot].PIECE[nK].maxOnTime)
              break;
            if(RUNTYPE[nType][nSlot].PIECE[nK].minOffTime != NO_TIME &&
                  offTime < RUNTYPE[nType][nSlot].PIECE[nK].minOffTime)
              break;
            if(RUNTYPE[nType][nSlot].PIECE[nK].maxOffTime != NO_TIME &&
                  offTime > RUNTYPE[nType][nSlot].PIECE[nK].maxOffTime)
              break;
//
//  Now look at the minimum and maximum piece size
//
            pieceSize = pPR->piece[nK].toTime - pPR->piece[nK].fromTime;
            if(RUNTYPE[nType][nSlot].PIECE[nK].minPieceSize != NO_TIME &&
                  pieceSize < RUNTYPE[nType][nSlot].PIECE[nK].minPieceSize)
              break;
            if(RUNTYPE[nType][nSlot].PIECE[nK].maxPieceSize != NO_TIME &&
                  pieceSize > RUNTYPE[nType][nSlot].PIECE[nK].maxPieceSize)
              break;
//
//  Except that last piece, check the minimum and maximum break sizes
//
            if(nK != lastPiece)
            {
              breakTime = pPR->piece[nK + 1].fromTime - pPR->piece[nK].toTime;
              if(RUNTYPE[nType][nSlot].PIECE[nK].minBreakTime != NO_TIME &&
                    breakTime < RUNTYPE[nType][nSlot].PIECE[nK].minBreakTime)
                break;
              if(RUNTYPE[nType][nSlot].PIECE[nK].maxBreakTime != NO_TIME &&
                    breakTime > RUNTYPE[nType][nSlot].PIECE[nK].maxBreakTime)
                break;
//
//  See if we've encountered the minimum and maximum breaks.  This differs subtley from
//  the above check, 'cause here we're checking on the run as a whole, as opposed to a piece
//
              if(bCheckRunMinBreak && !bGotRunMinBreak && breakTime > RUNTYPE[nType][nSlot].minBreakOf)
                bGotRunMinBreak = TRUE;
              if(bCheckRunMaxBreak && !bGotRunMaxBreak && breakTime < RUNTYPE[nType][nSlot].maxBreakOf)
                bGotRunMaxBreak = TRUE;
            }
//
//  Ok - check the contiguous times of the pieces.  This is sort of a half-assed check that
//  can be (and usually is) refuted later on in this routine, but it's a starting point to see
//  if any blatant violations exist at the outset.  Simply put, if there's work strung together
//  one right after the other, then this check will allow us to check the validity of those
//  contiguous pieces now, rather than later (when we've undoubtedly chewed a fair number of cycles)
//
            if(bCheckContigTime)
            {
              if(nK == lastPiece)
              {
                if(contigTime += pieceSize > RUNTYPE[nType][nSlot].maxContig)
                  break;
              }
              else
              {
                if(pPR->piece[nK].toTime == pPR->piece[nK + 1].fromTime)
                {
                  if(contigTime += pieceSize > RUNTYPE[nType][nSlot].maxContig)
                    break;
                }
                else
                {
                  contigTime = 0;  // Start again
                }
              }
            }
            bStillLooking = FALSE;  // Set this in case we're exiting the loop
          }
//
//  If we found one, the the minimum and maximum breaks on the run
//
          if(!bStillLooking)
          {
            if(bCheckRunMinBreak && !bGotRunMinBreak)
              continue;
            if(bCheckRunMaxBreak && !bGotRunMaxBreak)
              continue;
//
//  Also ensure we haven't exceeded the maximum (or fallen short of the minimum) platform time
//
            for(platformTime = 0, nK = 0; nK < pPR->numPieces; nK++)
            {
              platformTime += pPR->piece[nK].toTime - pPR->piece[nK].fromTime;
            }
            if(platformTime <= RUNTYPE[nType][nSlot].maxPlatformTime &&
                  platformTime >= RUNTYPE[nType][nSlot].minPlatformTime)  // Bingo
            {
              pCOST->runtype = MAKELONG(nType, nSlot);
            }
          }
        }
      }
    }
  }
//
//  Set out the runtype type and slot indexes
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    nType = (short int)LOWORD(pCOST->runtype);
    nSlot = (short int)HIWORD(pCOST->runtype);
  }
  else
  {
    nType = nSlot = NO_RECORD;  // We'd rather crash than have it point to the wrong thing
    LoadString(hInst, TEXT_092, runcosterReason, sizeof(runcosterReason));
  }
//
//  Put the platform time into pCOST
//
  for(pCOST->TOTAL.platformTime = 0, nI = 0; nI < pPR->numPieces; nI++)
  {
    pCOST->PIECECOST[nI].platformTime = pPR->piece[nI].toTime - pPR->piece[nI].fromTime;
    pCOST->TOTAL.platformTime += pCOST->PIECECOST[nI].platformTime;
  }
//
//  And the spread time too
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE &&
        RUNTYPE[nType][nSlot].maxSpreadTime != NO_TIME &&
        pPR->piece[lastPiece].toTime - pPR->piece[0].fromTime >
              RUNTYPE[nType][nSlot].maxSpreadTime && cutAsRuntype == NO_RUNTYPE)
  {
    LoadString(hInst, TEXT_083, tempString, TEMPSTRING_LENGTH);
    sprintf(runcosterReason, tempString, RUNTYPE[nType][nSlot].localName);
    pCOST->runtype = UNCLASSIFIED_RUNTYPE;
  }
//
//  Fill the RUNTRIP data structure so we can avoid excessive btrieve calls later in the code
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE &&
        RUNTYPE[nType][nSlot].flags & RTFLAGS_CREWONLY)
  {
    FillRUNTRIP(&RUNTRIP, pPR, serviceIndex, TRUE);
  }
  else
  {
    FillRUNTRIP(&RUNTRIP, pPR, serviceIndex, FALSE);
  }
//
//  Set up the pTime pointers
//
  for(nI = 0; nI < m_numPremiums; nI++)
  {
    for(nJ = 0; nJ < pPR->numPieces; nJ++)
    {
      if(PREMIUM[nI].formattedDataType == TMSDATA_REPORTTIME)
      {
        pTime[nI][nJ] = &(pCOST->PIECECOST[nJ].reportTime);
      }
      else if(PREMIUM[nI].formattedDataType == TMSDATA_TURNINTIME)
      {
        pTime[nI][nJ] = &(pCOST->PIECECOST[nJ].turninTime);
      }
      else if(PREMIUM[nI].formattedDataType == TMSDATA_MEAL ||
            PREMIUM[nI].formattedDataType == TMSDATA_PAIDBREAKS)
      {
        pTime[nI][nJ] = &(pCOST->PIECECOST[nJ].paidMeal);
      }
      else
      {
        pTime[nI][nJ] = &junkHeap;
      }
    }
  }
//
//  Loop through the premiums
//
  if(bUseDynamicTravels && !bUseCISPlan)
  {
    numTravelInstructions = 0;
  }
  minutesThatDontCount = 0;

//
//  Figure out the longest break time - that's lunch
//
  longest = NO_TIME;
  if(pPR->numPieces == 1)
  {
    longestBreakStartIndex = 0;
  }
  else
  {
    for(nJ = 0; nJ < lastPiece; nJ++)
    {
      breakTime = pPR->piece[nJ + 1].fromTime - pPR->piece[nJ].toTime;
      if(breakTime > longest)
      {
        longest = breakTime;
        longestBreakStartIndex = nJ + 1;
      }
    }
  }

  totalDontIncorporate = 0;
  for(nI = 0; nI < m_numPremiums; nI++)
  {
//
//  Don't bother with this premium if it's not what we're looking at
//
    if(PREMIUM[nI].runtype != UNCLASSIFIED_RUNTYPE && PREMIUM[nI].runtype != pCOST->runtype)
    {
      continue;
    }
//
//  Apply any costs to the run
//
//  Find out where the time goes
//
//  Set bAlreadyPaid to FALSE for all the pieces
//
    for(nJ = 0; nJ < pPR->numPieces; nJ++)
    {
      bAlreadyPaid[nJ] = FALSE;
    }
//
//  Set bPassedTest to TRUE for all the pieces
//
    for(nJ = 0; nJ < pPR->numPieces; nJ++)
    {
      bPassedTest[nI][nJ] = TRUE;
    }
//
//  Piece number
//
    if(PREMIUM[nI].forPieceNumber != NO_RECORD)
    {
      if(PREMIUM[nI].forPieceNumber == 0)
      {
        for(nJ = 0; nJ < pPR->numPieces; nJ++)
        {
          bPassedTest[nI][nJ] = (longestBreakStartIndex == nJ);
        }
      }
      else
      {
        for(nJ = 0; nJ < pPR->numPieces; nJ++)
        {
          bPassedTest[nI][nJ] = (PREMIUM[nI].forPieceNumber == nJ + 1);
        }
      }
    }
//
//  Route
//
    if(PREMIUM[nI].ROUTESrecordID != NO_RECORD)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(!bPassedTest[nI][nJ])
        {
          continue;
        }
        bPassedTest[nI][nJ] = (PREMIUM[nI].ROUTESrecordID = RUNTRIP.from[nJ].RGRPROUTESrecordID);
      }
    }
//
//  Service
//
    if(PREMIUM[nI].SERVICESrecordID != NO_RECORD)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(!bPassedTest[nI][nJ])
        {
          continue;
        }
        bPassedTest[nI][nJ] = (PREMIUM[nI].SERVICESrecordID == RUNTRIP.from[nJ].SGRPSERVICESrecordID);
      }
    }
//
//  Before time
//
    if(PREMIUM[nI].beforeTime != NO_TIME)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(!bPassedTest[nI][nJ])
        {
          continue;
        }
        bPassedTest[nI][nJ] = (pPR->piece[nJ].fromTime <= PREMIUM[nI].beforeTime);
      }
    }
//
//  Assigned to Garage
//
    if(PREMIUM[nI].assignedToNODESrecordID != NO_RECORD)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(!bPassedTest[nI][nJ])
        {
          continue;
        }
        bPassedTest[nI][nJ] =
              (PREMIUM[nI].assignedToNODESrecordID == RUNTRIP.from[nJ].assignedToNODESrecordID);
      }
    }
//
//  Bustype - split up for reability
//
    if(PREMIUM[nI].bustype != NO_RECORD)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(!bPassedTest[nI][nJ])
        {
          continue;
        }
        if(RUNTRIP.from[nJ].BUSTYPESrecordID == NO_RECORD)
        {
          bPassedTest[nI][nJ] = FALSE;
        }
        else
        {
          if(PREMIUM[nI].bustype == PREMIUMS_ANYACCESSIBLE)
          {
            if(!(RUNTRIP.from[nJ].flags & RCFLAGS_ACCESSIBLE))
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
          else
          {
            if(PREMIUM[nI].BUSTYPESrecordID != RUNTRIP.from[nJ].BUSTYPESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
        }
      }
    }
//
//  On
//
    if(PREMIUM[nI].onVerb != NO_RECORD)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(!bPassedTest[nI][nJ])
          continue;
//
//  On Pullout
//
        if(PREMIUM[nI].onVerb == PREMIUMS_PULLOUT)
        {
//
//  Is it a pullout?
//
          if(!(RUNTRIP.from[nJ].flags & RCFLAGS_PULLOUT))
          {
            bPassedTest[nI][nJ] = FALSE;
          }
//
//  Yes - Specific timepoint?
//
          else
          {
            if(PREMIUM[nI].onNoun == PREMIUMS_TIMEPOINT &&
                  (RUNTRIP.from[nJ].blockPOGNODESrecordID != PREMIUM[nI].nounNODESrecordID))
            {
              bPassedTest[nI][nJ] = FALSE;
            }
//
//  Fall through - check pullout record against start node
//
            if(RUNTRIP.from[nJ].blockPOGNODESrecordID != pPR->piece[nJ].fromNODESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
        }  // On pullout
//
//  On Pullin
//
        else if(PREMIUM[nI].onVerb == PREMIUMS_PULLIN)
        {
//
//  Is it a pullin?
//
          if(!(RUNTRIP.to[nJ].flags & RCFLAGS_PULLIN))
          {
            bPassedTest[nI][nJ] = FALSE;
          }
//
//  Yes - Specific timepoint?
//
          else
          {
            if(PREMIUM[nI].onNoun == PREMIUMS_TIMEPOINT &&
                  (RUNTRIP.to[nJ].blockPIGNODESrecordID != PREMIUM[nI].nounNODESrecordID))
            {
              bPassedTest[nI][nJ] = FALSE;
            }
//
//  Fall through - check pullout record against start node
//
            if(RUNTRIP.from[nJ].blockPIGNODESrecordID != pPR->piece[nJ].toNODESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
        }  // On pullin
//
//  On Start Relief
//
        else if(PREMIUM[nI].onVerb == PREMIUMS_STARTRELIEF)
        {
//
//  At any Garage?
//
          if(PREMIUM[nI].onNoun == PREMIUMS_ANYGARAGE)
          {
            if(!(RUNTRIP.from[nJ].flags & RCFLAGS_PULLOUT))
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
//
//  At any non-garage?
//
          else if(PREMIUM[nI].onNoun == PREMIUMS_ANYNONGARAGE)
          {
            if(RUNTRIP.from[nJ].flags & RCFLAGS_PULLOUT &&
                  RUNTRIP.from[nJ].blockPOGNODESrecordID == pPR->piece[nJ].fromNODESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
//
//  Specific timepoint?
//
          else if(PREMIUM[nI].onNoun == PREMIUMS_TIMEPOINT)
          {
            if(pPR->piece[nJ].fromNODESrecordID != PREMIUM[nI].nounNODESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
        }  // On Start Relief
//
//  On End Relief
//
        else if(PREMIUM[nI].onVerb == PREMIUMS_ENDRELIEF)
        {
//
//  At any Garage?
//
          if(PREMIUM[nI].onNoun == PREMIUMS_ANYGARAGE)
          {
            if(!(RUNTRIP.to[nJ].flags & RCFLAGS_PULLIN))
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
//
//  At any non-garage?
//
          else if(PREMIUM[nI].onNoun == PREMIUMS_ANYNONGARAGE)
          {
            if(RUNTRIP.to[nJ].flags & RCFLAGS_PULLIN &&
                  RUNTRIP.from[nJ].blockPIGNODESrecordID == pPR->piece[nJ].toNODESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
//
//  Specific Timepoint?
//
          else if(PREMIUM[nI].onNoun == PREMIUMS_TIMEPOINT)
          {
            if(pPR->piece[nJ].toNODESrecordID != PREMIUM[nI].nounNODESrecordID)
            {
              bPassedTest[nI][nJ] = FALSE;
            }
          }
        }  // On End Relief
//
//  Finish at - handled later
//
        else if(PREMIUM[nI].onVerb == PREMIUMS_FINISHAT)
        {
          bPassedTest[nI][nJ] = FALSE;
        }
      }
    }  // On
//
//  Give the premium if the piece passed the tests
//
    for(nJ = 0; nJ < pPR->numPieces; nJ++)
    {
      bAlreadyPaid[nJ] = bPassedTest[nI][nJ];
      if(bPassedTest[nI][nJ])
      {
        *pTime[nI][nJ] += PREMIUM[nI].time;
        if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
        {
          minutesThatDontCount += PREMIUM[nI].time;
        }
      }
    }
//
//  The break: longestBreakStartIndex set prior to cycling through the premiums
//
//  Is it paid at a flat rate?
//
    if(PREMIUM[nI].mealsPaid == PAIDMEAL_YES && pPR->numPieces > 1)
    {
      pCOST->PIECECOST[longestBreakStartIndex].paidMeal += PREMIUM[nI].time;
      if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
        minutesThatDontCount += PREMIUM[nI].time;
//
//  See if there's anything else to add to a paid break
//
      if(pCOST->runtype != NO_RUNTYPE && pCOST->runtype != UNCLASSIFIED_RUNTYPE)
      {
        if(RUNTYPE[nType][nSlot].flags & RTFLAGS_PAIDBREAK ||
              RUNTYPE[nType][nSlot].flags & RTFLAGS_LARGESTPAIDTHROUGHWHEN)
        {
          actualBreak = pPR->piece[longestBreakStartIndex + 1].fromTime - pPR->piece[longestBreakStartIndex].toTime;
          if(RUNTYPE[nType][nSlot].flags & RTFLAGS_PAIDBREAK)
          {
            if(actualBreak >= RUNTYPE[nType][nSlot].paidFrom &&
                  actualBreak <= RUNTYPE[nType][nSlot].paidTo)
            {
              pCOST->PIECECOST[longestBreakStartIndex].paidMeal += actualBreak - PREMIUM[nI].time;
              if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
                minutesThatDontCount += actualBreak - PREMIUM[nI].time;
            }
          }
          else
          {
            if(actualBreak < RUNTYPE[nType][nSlot].paidToWhen)
            {
              pCOST->PIECECOST[longestBreakStartIndex].paidMeal += actualBreak - PREMIUM[nI].time;
              if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
                minutesThatDontCount += actualBreak - PREMIUM[nI].time;
            }
          }
        }
      }
    }
//
//  Piece travel
//
    if(PREMIUM[nI].pieceTravel != BCTRAVEL_NA &&
          PREMIUM[nI].pieceTravel != NO_RECORD && pPR->numPieces > 1)
    {
      if(pCOST->runtype == UNCLASSIFIED_RUNTYPE)
      {
        goto DontApplyPremium;
      }
//
//  Are we going to pay it?
//
      bOKToPayBC = TRUE;
      if((PREMIUM[nI].payWhen & PAYWHEN_ABCDFLAG) && pPR->numPieces == 2)
      {
        if(NodesEquivalent(pPR->piece[0].fromNODESrecordID, pPR->piece[0].toNODESrecordID, &equivalentTravelTime) &&
              NodesEquivalent(pPR->piece[1].fromNODESrecordID, pPR->piece[1].toNODESrecordID, &equivalentTravelTime))

        {
          bOKToPayBC = FALSE;
        }
      }
//
//  Loop through all the pieces
//
      if(bOKToPayBC)
      {
        for(nJ = 0; nJ < lastPiece; nJ++)
        {
          bSameNode = NodesEquivalent(pPR->piece[nJ].toNODESrecordID,
                pPR->piece[nJ + 1].fromNODESrecordID, &equivalentTravelTime);
//
//  Do we do a lookup?
//
          if(bSameNode && (PREMIUM[nI].pieceTravel == BCTRAVEL_BTOCDIRECT || nJ != longestBreakStartIndex - 1))
          {
            travelTime = 0;
            bValidTravel = TRUE;
//
//  If the two nodes are identical and they're equivalenced to the garage, 
//  then look up the equivalent travel time between the node and the garage
//
            if(pPR->piece[nJ].toNODESrecordID == pPR->piece[nJ + 1].fromNODESrecordID)
            {
              bestTime = NO_TIME;
              for(nL = 0; nL < numGaragesInGarageList; nL++)
              {
                if(pPR->piece[nJ].toNODESrecordID != garageList[nL] &&
                      NodesEquivalent(pPR->piece[nJ].toNODESrecordID,
                            garageList[nL], &equivalentTravelTime))
                {
                  pCOST->TRAVEL[nJ].endNODESrecordID = garageList[nL];
                  pCOST->TRAVEL[nJ].endBPointNODESrecordID = pPR->piece[nJ].toNODESrecordID;
                  pCOST->TRAVEL[nJ].endBPointTime = pPR->piece[nJ].fromTime;
                  pCOST->TRAVEL[nJ].endTravelTime = equivalentTravelTime;
                  pCOST->TRAVEL[nJ].endDwellTime = 0;
                  travelTime += equivalentTravelTime;
                  pCOST->TRAVEL[nJ + 1].startNODESrecordID = pPR->piece[nJ + 1].fromNODESrecordID;
                  pCOST->TRAVEL[nJ + 1].startTravelTime = equivalentTravelTime;
                  pCOST->TRAVEL[nJ + 1].startDwellTime = 0;
                  pCOST->TRAVEL[nJ + 1].startAPointNODESrecordID = garageList[nL];
                  pCOST->TRAVEL[nJ + 1].startAPointTime = pPR->piece[nJ + 1].toTime;
                  travelTime += equivalentTravelTime;
                  break;
                }
              }
            }
//
//  They weren't identical - check on the equivalent travel time
//
            else if(equivalentTravelTime != 0)
            {
              for(bFound = FALSE, nL = 0; nL < numGaragesInGarageList; nL++)
              {
                if(pPR->piece[nJ].toNODESrecordID == garageList[nL])
                {
                  bFound = TRUE;
                  break;
                }
              }
              if(!bFound)
              {
                pCOST->TRAVEL[nJ].endNODESrecordID = pPR->piece[nJ].toNODESrecordID;
                pCOST->TRAVEL[nJ].endBPointNODESrecordID = pPR->piece[nJ].toNODESrecordID;
                pCOST->TRAVEL[nJ].endBPointTime = pPR->piece[nJ].fromTime;
                pCOST->TRAVEL[nJ].endTravelTime = equivalentTravelTime;
                pCOST->TRAVEL[nJ].endDwellTime = 0;
                travelTime += equivalentTravelTime;
              }
              for(bFound = FALSE, nL = 0; nL < numGaragesInGarageList; nL++)
              {
                if(pPR->piece[nJ + 1].fromNODESrecordID == garageList[nL])
                {
                  bFound = TRUE;
                  break;
                }
              }
              if(!bFound)
              {
                pCOST->TRAVEL[nJ + 1].startNODESrecordID =
                      pPR->piece[nJ + 1].fromNODESrecordID;
                pCOST->TRAVEL[nJ + 1].startTravelTime = equivalentTravelTime;
                pCOST->TRAVEL[nJ + 1].startDwellTime = 0;
                pCOST->TRAVEL[nJ + 1].startAPointNODESrecordID =
                      pPR->piece[nJ + 1].fromNODESrecordID; 
                pCOST->TRAVEL[nJ + 1].startAPointTime = pPR->piece[nJ + 1].toTime;
                travelTime += equivalentTravelTime;
              }
            }
          }
          else
          {
            bValidTravel = GetPieceTravel(FALSE, nI, nJ, nType, nSlot, &RUNTRIP, pPR, pCOST, &travelTime, &dwellTime);
            travelTime += equivalentTravelTime;
            if(!bValidTravel)
            {
              pCOST->runtype = UNCLASSIFIED_RUNTYPE;
              goto DontApplyPremium;
            }
          }
//
//  Add it in 
//
          paidTravel = GetPaidTravel(nI, travelTime);
          if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
            minutesThatDontCount += paidTravel;
//
//  Was it valid?
//
          if(!bValidTravel)
          {
            pCOST->runtype = UNCLASSIFIED_RUNTYPE;
            goto DontApplyPremium;
          }
        }
      }
    }
//
//  Start/End Travel payments
//
    if(PREMIUM[nI].startEnd != NO_RECORD)
    {
      travelTime = 0;
//
//  Are we going to pay it?
//
      bOKToPayStartEnd = TRUE;
      if((PREMIUM[nI].paidTravel & TRAVELPAID_ABCDFLAG) && pPR->numPieces == 2)
      {
        if(NodesEquivalent(pPR->piece[0].fromNODESrecordID, pPR->piece[0].toNODESrecordID, &equivalentTravelTime) &&
              NodesEquivalent(pPR->piece[1].fromNODESrecordID, pPR->piece[1].toNODESrecordID, &equivalentTravelTime))
        {
          bOKToPayStartEnd = FALSE;
        }
      }
//
//  Start/End at Closest Garage
//
      if(PREMIUM[nI].startLocation == STARTEND_STARTCLOSEST)
      {
//
//  Find the travel from the closest garage to the first
//  point and from the last point to the closest garage
//
        long flags = CLOSEST_FLAG_BEATGARAGEBEFORETIME | CLOSEST_FLAG_REVERSE;

        bestTime1 = GetClosestGarage(flags, &RUNTRIP, &dwellTime1,  &startLocation,
               pPR->piece[0].fromNODESrecordID, pPR->piece[0].fromTime, nI, 0);
        if(PREMIUM[nI].startEnd == STARTEND_SAME)
        {
          flags = 0;
          bestTime2 = GetClosestGarage(flags, &RUNTRIP, &dwellTime2, &endLocation, 
                  pPR->piece[lastPiece].toNODESrecordID, pPR->piece[lastPiece].toTime, nI, lastPiece);
        }
        else
        {
          bestTime2 = NO_TIME;  // Fall through 'til later
          endLocation = NO_RECORD;
        }
//
//  Both start and end are NO_TIME it's infeasible
//
        if(bestTime1 == NO_TIME && bestTime2 == NO_TIME)
        {
          startLocation = NO_RECORD;
          bestTime = 0;
          dwellTime = 0;
        }
//
//  If either but not both are NO_TIME, choose 
//  the one that didn't come back as NO_TIME
//
        else if((bestTime1 == NO_TIME && bestTime2 != NO_TIME) ||
              (bestTime1 != NO_TIME && bestTime2 == NO_TIME))
        {
          if(bestTime1 == NO_TIME)
          {
            startLocation = endLocation;
            bestTime = bestTime2;
            dwellTime = dwellTime2;
          }
          else
          {
            bestTime = bestTime1;
            dwellTime = dwellTime1;
          }
          if(NodesEquivalent(startLocation,
                pPR->piece[0].fromNODESrecordID, &equivalentTravelTime))
          {
            dwellTime = 0;
            travelTime = equivalentTravelTime;
          }
          else
          {
            travelTime = bestTime;
          }
        }
//
//  Neither were no time, so choose the better
//
        else
        {
          if(NodesEquivalent(startLocation, endLocation, &equivalentTravelTime))
          {
            dwellTime = dwellTime1;
            travelTime = bestTime1;
          }
          else
          {
            bestTime3 = GetGarageTravel(startLocation,
                  &pPR->piece[lastPiece], &PREMIUM[nI], &RUNTRIP);
            bestTime4 = GetGarageTravel(endLocation,
                  &pPR->piece[0], &PREMIUM[nI], &RUNTRIP);
            if(bestTime3 == NO_TIME && bestTime4 == NO_TIME)
            {
              startLocation = NO_RECORD;
              bestTime = 0;
              dwellTime = 0;
            }
            else if(bestTime3 == NO_TIME && bestTime4 != NO_TIME)
            {
              bestTime = bestTime2;
              dwellTime = dwellTime2;
            }
            else if(bestTime3 != NO_TIME && bestTime4 == NO_TIME)
            {
              bestTime = bestTime1;
              dwellTime = dwellTime1;
            }
            else
            {
              if(bestTime1 + bestTime3 < bestTime2 + bestTime4)
              {
                bestTime = bestTime1 + bestTime3;
                dwellTime = dwellTime1;
              }
              else
              {
                startLocation = endLocation;
                bestTime = bestTime2 + bestTime4;
                dwellTime = dwellTime2;
              }
            }
            travelTime = bestTime;
          }
        }
//
//  See which one was better
//
        if(bOKToPayStartEnd)
        {
          pCOST->TRAVEL[0].startNODESrecordID = startLocation;
          pCOST->TRAVEL[0].startTravelTime = travelTime;
          pCOST->TRAVEL[0].startDwellTime = dwellTime;
          pCOST->TRAVEL[0].startAPointNODESrecordID = pPR->piece[0].fromNODESrecordID; 
          pCOST->TRAVEL[0].startAPointTime = pPR->piece[0].fromTime;
          if(PREMIUM[nI].paidTravel & TRAVELPAID_START)
          {
            pCOST->TRAVEL[0].flags |= TRAVEL_FLAGS_STARTPAID;
          }
        }
      }
//
//  The others...
//
      else
      {
//
//  Start anywhere
//
//  This is an issue if the start and end locations have to be the same,
//  and the bus pulls in on piece[lastPiece] - ie there might be no way
//  to get him back to the original location.  Therefore, while he may
//  indeed start "anywhere", we'll have to get him there from the garage
//  in the first place.
//
        bDoIt = FALSE;
        if(PREMIUM[nI].startLocation == STARTEND_STARTANYWHERE)
        {
          if(PREMIUM[nI].startEnd == STARTEND_SAME && RUNTRIP.to[lastPiece].flags & RCFLAGS_PULLIN)
          {
            bDoIt = TRUE;
            GCTData.fromNODESrecordID = pPR->piece[lastPiece].toNODESrecordID;
          }
        }
//
//  Start at POG
//
        else if(PREMIUM[nI].startLocation == STARTEND_STARTPOG)
        {
          if(pPR->piece[0].fromNODESrecordID != RUNTRIP.from[0].blockPOGNODESrecordID)
          {
            bDoIt = TRUE;
            GCTData.fromNODESrecordID = RUNTRIP.from[0].blockPOGNODESrecordID;
          }
        }
//
//  Start at PIG
//
        else if(PREMIUM[nI].startLocation == STARTEND_STARTPIG)
        {
          if(pPR->piece[0].fromNODESrecordID != RUNTRIP.from[0].blockPIGNODESrecordID)
          {
            bDoIt = TRUE;
            GCTData.fromNODESrecordID = RUNTRIP.from[0].blockPIGNODESrecordID;
          }
        }
//
//  Start at a particular timepoint
//
        else
        {
          if(pPR->piece[0].fromNODESrecordID != PREMIUM[nI].startNODESrecordID)
          {
            bDoIt = TRUE;
            GCTData.fromNODESrecordID = PREMIUM[nI].startNODESrecordID;
          }
        }
//
//  There's the possibility we don't need to check...
//
        dwellTime = 0;
        if(!bDoIt)
        {
          startLocation = pPR->piece[0].fromNODESrecordID;
          travelTime = 0;
        }
        else
        {
          startLocation = GCTData.fromNODESrecordID;
          travelTime = GetGarageTravel(startLocation, &pPR->piece[0], &PREMIUM[nI], &RUNTRIP);
          if(bOKToPayStartEnd)
          {
            pCOST->TRAVEL[0].startNODESrecordID = startLocation;
            pCOST->TRAVEL[0].startTravelTime = travelTime;
            pCOST->TRAVEL[0].startDwellTime = dwellTime;
            pCOST->TRAVEL[0].startAPointNODESrecordID = pPR->piece[0].fromNODESrecordID; 
            pCOST->TRAVEL[0].startAPointTime = pPR->piece[0].fromTime;
            if(PREMIUM[nI].paidTravel & TRAVELPAID_START)
            {
              pCOST->TRAVEL[0].flags |= TRAVEL_FLAGS_STARTPAID;
            }
          }
        }
      }
//
//  Did we get anything?
//
      if(travelTime == NO_TIME)
      {
        LoadString(hInst, TEXT_084, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString,
              NodeAndNode(GCTData.fromNODESrecordID, pPR->piece[0].fromNODESrecordID));
        pCOST->runtype = UNCLASSIFIED_RUNTYPE;
        goto DontApplyPremium;
      }
//
//  We know at this point that if there was travel, it's at least feasible
//
      if(bOKToPayStartEnd)
      {
        pCOST->TRAVEL[0].startTravelTime = travelTime;
        if(travelTime != 0 && (PREMIUM[nI].paidTravel & TRAVELPAID_START))
        {
          paidTravel = GetPaidTravel(nI, travelTime);
          pCOST->PIECECOST[0].dwellTime = dwellTime;
          if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
            minutesThatDontCount += paidTravel;
        }
      }
//
//  Now do the travel from the last point of the run to wherever
//
      travelTime = 0;
      dwellTime = 0;
      bDoIt = FALSE;
//
//  End at Closest Garage (if startEnd isn't STARTEND_SAME)
//
      if(PREMIUM[nI].endLocation == STARTEND_ENDCLOSEST &&
            PREMIUM[nI].startEnd != STARTEND_SAME)
      {
        long flags = 0;

        bestTime = GetClosestGarage(flags, &RUNTRIP, &dwellTime, &endLocation, 
              pPR->piece[lastPiece].toNODESrecordID, pPR->piece[lastPiece].toTime, nI, nJ);
        if(bestTime == NO_TIME)
          bestTime = 0;
        if(NodesEquivalent(endLocation,
              pPR->piece[lastPiece].toNODESrecordID, &equivalentTravelTime))
        {
          dwellTime = 0;
          travelTime = equivalentTravelTime;
        }
        else
        {
          travelTime = bestTime;
        }
        if(bOKToPayStartEnd)
        {
          pCOST->TRAVEL[lastPiece].endNODESrecordID = endLocation;
          pCOST->TRAVEL[lastPiece].endBPointNODESrecordID = pPR->piece[lastPiece].toNODESrecordID;
          pCOST->TRAVEL[lastPiece].endBPointTime = pPR->piece[lastPiece].toTime;
          pCOST->TRAVEL[lastPiece].endTravelTime = travelTime;
          pCOST->TRAVEL[lastPiece].endDwellTime = dwellTime;
          if(PREMIUM[nI].paidTravel & TRAVELPAID_END)
          {
            pCOST->TRAVEL[lastPiece].flags |= TRAVEL_FLAGS_ENDPAID;
          }
        }
      }
//
//  Otherwise, figure it out
//
      else
      {
        if(PREMIUM[nI].startEnd == STARTEND_SAME)
        {
          if(pPR->piece[lastPiece].toNODESrecordID != startLocation)
          {
            bDoIt = TRUE;
            GCTData.toNODESrecordID = startLocation;
          }
//
//  End at POG
//
          else if(PREMIUM[nI].endLocation == STARTEND_ENDPOG)
          {
            if(pPR->piece[lastPiece].toNODESrecordID != RUNTRIP.to[lastPiece].blockPOGNODESrecordID)
            {
              bDoIt = TRUE;
              GCTData.toNODESrecordID = RUNTRIP.to[lastPiece].blockPOGNODESrecordID;
            }
          }
//
//  End at PIG
//
          else if(PREMIUM[nI].endLocation == STARTEND_ENDPIG)
          {
            if(pPR->piece[lastPiece].toNODESrecordID != RUNTRIP.to[lastPiece].blockPIGNODESrecordID)
            {
              bDoIt = TRUE;
              GCTData.toNODESrecordID = RUNTRIP.to[lastPiece].blockPIGNODESrecordID;
            }
          }
//
//  End at a particular timepoint
//
          else if(PREMIUM[nI].endLocation == STARTEND_STARTNODE)
          {
            if(pPR->piece[lastPiece].toNODESrecordID != PREMIUM[nI].endNODESrecordID)
            {
              bDoIt = TRUE;
              GCTData.toNODESrecordID = PREMIUM[nI].endNODESrecordID;
            }
          }
        }
//
//  There's the possibility we don't need to check...
//
        if(bDoIt)
        {
          if(NodesEquivalent(pPR->piece[lastPiece].toNODESrecordID,
                GCTData.toNODESrecordID, &equivalentTravelTime))
            travelTime = equivalentTravelTime;
          else if(PREMIUM[nI].payTravelTime == WORKRULES_TIMEENTERED)
            travelTime = PREMIUM[nI].time;
          else if(PREMIUM[nI].payTravelTime == WORKRULES_ASINCONNECTIONS)
          {
            GCTData.fromNODESrecordID = pPR->piece[lastPiece].toNODESrecordID;
            GCTData.fromROUTESrecordID = RUNTRIP.to[lastPiece].RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = RUNTRIP.to[lastPiece].SGRPSERVICESrecordID;
            GCTData.fromPATTERNNAMESrecordID = RUNTRIP.to[lastPiece].PATTERNNAMESrecordID;
            GCTData.toROUTESrecordID = RUNTRIP.to[lastPiece].RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = RUNTRIP.to[lastPiece].SGRPSERVICESrecordID;
            GCTData.toPATTERNNAMESrecordID = RUNTRIP.to[lastPiece].PATTERNNAMESrecordID;
            GCTData.timeOfDay = pPR->piece[lastPiece].toTime;
            travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
          }
          else
          {
            if(bUseCISPlan)
            {
              if(RUNTRIP.to[lastPiece].SGRPSERVICESrecordID == NO_RECORD)
              {
                travelTime = NO_TIME;
              }
              else
              {
                CISplanReliefConnect(&startTime, &endTime, &dwellTime, 0,
                      pPR->piece[lastPiece].toNODESrecordID, GCTData.toNODESrecordID,
                      pPR->piece[lastPiece].toTime, TRUE,
                      RUNTRIP.to[lastPiece].SGRPSERVICESrecordID);
                if(startTime == -25 * 60 * 60)
                {
                  travelTime = NO_TIME;
                }
                else
                {
                  travelTime =  (endTime - startTime) + dwellTime;
                }
              }
            }
            else
              travelTime = GetDynamicTravelTime(FALSE, pPR->piece[lastPiece].toNODESrecordID,
                    GCTData.toNODESrecordID, RUNTRIP.to[lastPiece].SGRPSERVICESrecordID,
                    pPR->piece[lastPiece].toTime, &dwellTime);
            if(travelTime == NO_TIME &&
                  (PREMIUM[nI].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC))
            {
              if(travelTime != NO_TIME)
              {
                if(bUseDynamicTravels && !bUseCISPlan)
                  numTravelInstructions--;
              }
              GCTData.fromNODESrecordID = pPR->piece[lastPiece].toNODESrecordID;
              GCTData.fromROUTESrecordID = RUNTRIP.to[lastPiece].RGRPROUTESrecordID;
              GCTData.fromSERVICESrecordID = RUNTRIP.to[lastPiece].SGRPSERVICESrecordID;
              GCTData.fromPATTERNNAMESrecordID = RUNTRIP.to[lastPiece].PATTERNNAMESrecordID;
              GCTData.toROUTESrecordID = RUNTRIP.to[lastPiece].RGRPROUTESrecordID;
              GCTData.toSERVICESrecordID = RUNTRIP.to[lastPiece].SGRPSERVICESrecordID;
              GCTData.toPATTERNNAMESrecordID = RUNTRIP.to[lastPiece].PATTERNNAMESrecordID;
              GCTData.timeOfDay = pPR->piece[lastPiece].toTime;
              travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
            }
          }
          if(bOKToPayStartEnd)
          {
            pCOST->TRAVEL[lastPiece].endBPointNODESrecordID = pPR->piece[lastPiece].toNODESrecordID;
            pCOST->TRAVEL[lastPiece].endBPointTime = pPR->piece[lastPiece].toTime;
            pCOST->TRAVEL[lastPiece].endTravelTime = travelTime;
            pCOST->TRAVEL[lastPiece].endDwellTime = dwellTime;
            pCOST->TRAVEL[lastPiece].endNODESrecordID = GCTData.toNODESrecordID;
            if(PREMIUM[nI].paidTravel & TRAVELPAID_END)
            {
              pCOST->TRAVEL[lastPiece].flags |= TRAVEL_FLAGS_ENDPAID;
            }
          }
        }
      }
//
//  Did we get anything?
//
      if(travelTime == NO_TIME)
      {
        LoadString(hInst, TEXT_084, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString,
              NodeAndNode(pPR->piece[lastPiece].toNODESrecordID, GCTData.toNODESrecordID));
        pCOST->runtype = UNCLASSIFIED_RUNTYPE;
        goto DontApplyPremium;
      }
//
//  We know at this point that if there was travel, it's at least feasible
//
      if(bOKToPayStartEnd)
      {
        pCOST->TRAVEL[lastPiece].endTravelTime = travelTime;
        if(PREMIUM[nI].paidTravel & TRAVELPAID_END)
        {
          paidTravel = GetPaidTravel(nI, travelTime);
          pCOST->PIECECOST[lastPiece].dwellTime += dwellTime;
          if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
            minutesThatDontCount += paidTravel;
        }
      }
//
//  If travel is included in the spread time, check it out here
//
      if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
      {
        if(pCOST->TRAVEL[lastPiece].endTravelTime != 0 || pCOST->TRAVEL[0].startTravelTime != 0)
        {
          if(RUNTYPE[nType][nSlot].flags & RTFLAGS_TRAVELCOUNTS
                && (pPR->piece[lastPiece].toTime + pCOST->TRAVEL[lastPiece].endTravelTime) -
                   (pPR->piece[0].fromTime - pCOST->TRAVEL[0].startTravelTime) > RUNTYPE[nType][nSlot].maxSpreadTime)
          {
            LoadString(hInst, TEXT_099, tempString, TEMPSTRING_LENGTH);
            strcpy(runcosterReason, tempString);
            pCOST->runtype = UNCLASSIFIED_RUNTYPE;
            goto DontApplyPremium;
          }
        }
      }
    }
//
//  Loop through
//
    continue;
//
//  Deduction
//
    DontApplyPremium:
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        if(bAlreadyPaid[nJ])
        {
          *pTime[nI][nJ] -= PREMIUM[nI].time;
          if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
            minutesThatDontCount -= PREMIUM[nI].time;
        }
      }
  }
//
//  Special for "finish at".  This occurs when he lands at a particular 
//  timepoint at the end of a piece.  We don't have this information
//  filled in on the pCOST->TRAVEL structure until after the first pass
//
  for(nI = 0; nI < m_numPremiums; nI++)
  {
    if(PREMIUM[nI].onVerb == PREMIUMS_FINISHAT)
    {
      for(nJ = 0; nJ < pPR->numPieces; nJ++)
      {
        bDoIt = FALSE;
//
//  At any Garage?
//
        if(PREMIUM[nI].onNoun == PREMIUMS_ANYGARAGE)
        {
          if((RUNTRIP.to[nJ].flags & RCFLAGS_PULLIN) ||
                (IsGarage(pCOST->TRAVEL[nJ].endNODESrecordID)))
          {
            bDoIt = TRUE;
          }
        }
//
//  At any non-garage?
//
        else if(PREMIUM[nI].onNoun == PREMIUMS_ANYNONGARAGE)
        {
          if(!(RUNTRIP.to[nJ].flags & RCFLAGS_PULLIN) ||
                !(IsGarage(pCOST->TRAVEL[nJ].endNODESrecordID)))
          {
            bDoIt = TRUE;
          }
        }
//
//  Specific Timepoint?
//
        else if(PREMIUM[nI].onNoun == PREMIUMS_TIMEPOINT)
        {
          if(pPR->piece[nJ].toNODESrecordID == PREMIUM[nI].nounNODESrecordID)
          {
            bDoIt = TRUE;
          }
        }
//
//  Give the premium if the piece passed the tests
//
        if(bDoIt)
        {
          *pTime[nI][nJ] += PREMIUM[nI].time;
          if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT))
          {
            minutesThatDontCount += PREMIUM[nI].time;
          }
        }
      }  // nJ
    }  // On Finish at
  }  // nI

//
//  Check to see if these premiums adversely affected the break time
//
  totalDontIncorporate = 0;
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    for(nI = 0; nI < m_numPremiums; nI++)
    {
      if(PREMIUM[nI].runtype != UNCLASSIFIED_RUNTYPE && PREMIUM[nI].runtype != pCOST->runtype)
      {
        continue;
      }
//      if(!bAlreadyPaid[nI])
//      {
//        continue;
//      }
      if(pPR->numPieces > 1 && (PREMIUM[nI].flags & PREMIUMFLAGS_DONTINCORPORATE) && bPassedTest[nI][1])
      {
        totalDontIncorporate += PREMIUM[nI].time;
        if(longest - totalDontIncorporate < RUNTYPE[nType][nSlot].PIECE[0].minBreakTime)
        {
          LoadString(hInst, ERROR_370, tempString, TEMPSTRING_LENGTH);
          strcpy(runcosterReason, tempString);
          pCOST->runtype = UNCLASSIFIED_RUNTYPE;
          break;
        }
      }
    }
  }
//
//  Check the piece sizes with any travel
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    for(nJ = 0; nJ < pPR->numPieces; nJ++)
    {
      nL = (pPR->piece[nJ].toTime + pCOST->TRAVEL[nJ].endTravelTime) - 
            (pPR->piece[nJ].fromTime - pCOST->TRAVEL[nJ].startTravelTime);
      if(nL > RUNTYPE[nType][nSlot].PIECE[nJ].maxPieceSize &&
            (RUNTYPE[nType][nSlot].PIECE[nJ].flags & PIECEFLAGS_TRAVELINCLUDEDINMAX))
      {
        LoadString(hInst, TEXT_100, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString, nJ + 1);
        pCOST->runtype = UNCLASSIFIED_RUNTYPE;
        break;
      }
    }
  }
//
//  Now check to see if there is a penalty in any of the
//  premiums for exceeding a certain piece size (plat only)
//
//
//  Piece size
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    for(nI = 0; nI < m_numPremiums; nI++)
    {
      if(PREMIUM[nI].withPieceSize != NO_RECORD && PREMIUM[nI].withPieceSize != 0)
      {
        for(nJ = 0; nJ < pPR->numPieces; nJ++)
        {
          if(PREMIUM[nI].forPieceNumber != NO_RECORD && PREMIUM[nI].forPieceNumber != nJ + 1)
            continue;
          nL = pPR->piece[nJ].toTime - pPR->piece[nJ].fromTime;
          if(nL > PREMIUM[nI].withPieceSize)
          {
            pCOST->TOTAL.paidBreak += PREMIUM[nI].time;
            break;
          }
        }
      }
    }
  }

//
//  Verify the minimum and maximum breaks
//
//  20050929 Assume the breaks are ok if we're coming in with a specific runtype
//
  if(cutAsRuntype == NO_RUNTYPE)
  {
    if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
    {
      if(RUNTYPE[nType][nSlot].minBreakOf != NO_TIME && 
            RUNTYPE[nType][nSlot].maxBreakOf != NO_TIME &&
            pPR->numPieces > 2)
      {
        bStillLooking = TRUE;
        for(nJ = 0; nJ < lastPiece; nJ++)
        {
          startOfBreak = pPR->piece[nJ].toTime + pCOST->TRAVEL[nJ].endTravelTime;
          endOfBreak = pPR->piece[nJ + 1].fromTime - pCOST->TRAVEL[nJ + 1].startTravelTime;
          breakTime = endOfBreak - startOfBreak;
          if(breakTime >= RUNTYPE[nType][nSlot].minBreakOf &&
                breakTime <= RUNTYPE[nType][nSlot].maxBreakOf)
          {
            bStillLooking = FALSE;
            break;
          }
        }
        if(bStillLooking)
        {
          LoadString(hInst, TEXT_155, tempString, TEMPSTRING_LENGTH);
          strcpy(runcosterReason, tempString);
          pCOST->runtype = UNCLASSIFIED_RUNTYPE;
        }
      }
    }
  }
//
//  Pay any breaks that need paying and deduct any premiums that were
//  paid when the "dont pay if paying intervening time" flag was set
//
  for(nJ = 0; nJ < lastPiece; nJ++)
  {
    bPaidBreak[nJ] = FALSE;
  }
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE && pPR->numPieces > 1)
  {
    longest = NO_TIME;
    for(nJ = 0; nJ < lastPiece; nJ++)
    {
      startOfBreak = pPR->piece[nJ].toTime + pCOST->TRAVEL[nJ].endTravelTime;
      endOfBreak = pPR->piece[nJ + 1].fromTime - pCOST->TRAVEL[nJ + 1].startTravelTime;
      if(pCOST->PIECECOST[nJ + 1].reportTime != 0)
      {
        if(endOfBreak - pCOST->PIECECOST[nJ + 1].reportTime < startOfBreak)
        {
          pCOST->PIECECOST[nJ + 1].reportTime = 0;
        }
      }
      endOfBreak = pPR->piece[nJ + 1].fromTime - pCOST->TRAVEL[nJ + 1].startTravelTime - pCOST->PIECECOST[nJ + 1].reportTime;
      breakTime = endOfBreak - startOfBreak;
      if(breakTime > longest)
      {
        longest = breakTime;
        nK = nJ;
      }
    }
    for(nJ = 0; nJ < lastPiece; nJ++)
    {
      startOfBreak = pPR->piece[nJ].toTime + pCOST->TRAVEL[nJ].endTravelTime + pCOST->PIECECOST[nJ].turninTime;
      endOfBreak = pPR->piece[nJ + 1].fromTime - pCOST->TRAVEL[nJ + 1].startTravelTime - pCOST->PIECECOST[nJ + 1].reportTime;
      breakTime = endOfBreak - startOfBreak;
      if(breakTime > 0)
      {
        if(nJ == nK)
        {
          if(RUNTYPE[nType][nSlot].flags & RTFLAGS_PAIDBREAK)
          {
            if(RUNTYPE[nType][nSlot].paidFrom != NO_TIME &&
                  RUNTYPE[nType][nSlot].paidTo != NO_TIME)
            {
              if(RUNTYPE[nType][nSlot].paidFrom == 0)
              {
                if(breakTime > RUNTYPE[nType][nSlot].paidTo)
                {
                  breakTime = RUNTYPE[nType][nSlot].paidTo;
                }
              }
              else
              {
                if(breakTime >= RUNTYPE[nType][nSlot].paidFrom &&
                      breakTime <= RUNTYPE[nType][nSlot].paidTo)
                {
                  breakTime -= RUNTYPE[nType][nSlot].paidFrom - 60; // + 60;
                  if(breakTime < 0)
                  {
                    breakTime = 0;
                  }
                } 
                else
                {
                  breakTime = 0;
                }
              }
            }
          }
          else if((RUNTYPE[nType][nSlot].flags & RTFLAGS_LARGESTPAIDTHROUGHWHEN))
          {
            if(RUNTYPE[nType][nSlot].paidToWhen != NO_TIME)
            {
//              if(breakTime < RUNTYPE[nType][nSlot].paidToWhen)
//              {
//                breakTime -= RUNTYPE[nType][nSlot].paidToWhen;
//                if(breakTime < 0)
//                  breakTime = 0;
//              } 
//              else
//                breakTime = 0;
              if(breakTime >= RUNTYPE[nType][nSlot].paidToWhen)
              {
                breakTime = 0;
              }
            }
          }
          else
          {
            continue;
          }
        }
        else
        {
          if(RUNTYPE[nType][nSlot].flags & RTFLAGS_SMALLERPAID)
          {
            if(RUNTYPE[nType][nSlot].paidFromSmaller != NO_TIME &&
                  RUNTYPE[nType][nSlot].paidToSmaller != NO_TIME)
            {
              if(breakTime >= RUNTYPE[nType][nSlot].paidFromSmaller &&
                    breakTime <= RUNTYPE[nType][nSlot].paidToSmaller)
              {
                breakTime = breakTime - RUNTYPE[nType][nSlot].paidFromSmaller - 60; // + 60;
              }
              else
              {
                breakTime = 0;
              }
            }
          }
          else if(RUNTYPE[nType][nSlot].flags & RTFLAGS_SMALLERPAIDTHROUGHWHEN)
          {
            if(RUNTYPE[nType][nSlot].paidToWhenSmaller != NO_TIME)
            {
//              if(breakTime < RUNTYPE[nType][nSlot].paidToWhenSmaller)
//              {
//                breakTime = breakTime - RUNTYPE[nType][nSlot].paidToWhenSmaller;
//                if(breakTime < 0)
//                  breakTime = 0;
//              }
//              else
//                breakTime = 0;
              if(breakTime >= RUNTYPE[nType][nSlot].paidToWhenSmaller)
              {
                breakTime = 0;
              }
            }
          }
          else
          {
            continue;
          }
        }
        pCOST->PIECECOST[nJ].paidBreak = breakTime;
        pCOST->TOTAL.paidBreak += breakTime;
        bPaidBreak[nJ] = TRUE;
      }
    }
    pCOST->TOTAL.premiumTime += pCOST->TOTAL.paidBreak;
  }
//
//  Deduct the premium if necessay
//
  for(nJ = 0; nJ < lastPiece; nJ++)
  {
    if(!bPaidBreak[nJ])
    {
      continue;
    }
    for(nI = 0; nI < m_numPremiums; nI++)
    {
      if(PREMIUM[nI].flags & PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING)
      {
        *pTime[nI][nJ + 1] -= PREMIUM[nI].time;
        if(*pTime[nI][nJ + 1] < 0)
        {
          *pTime[nI][nJ + 1] = 0;
        }
      }
    }
  }
//
//  See if a layover is required,
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    if(RUNTYPE[nType][nSlot].flags & RTFLAGS_MINLAY)
    {
//
//  Build up the list of from and to times
//
      numEntries = 0;
      for(nI = 0; nI < pPR->numPieces; nI++)
      {
        SpanCheck[numEntries].ATime = pPR->piece[nI].fromTime - pCOST->TRAVEL[nI].startTravelTime -
              pCOST->TRAVEL[nI].startDwellTime - pCOST->PIECECOST[nI].reportTime;
        TRIPSKey0.recordID = pPR->piece[nI].fromTRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        keyNumber = (TRIPS.dropback.RGRPROUTESrecordID == NO_RECORD) ? 2 : 3;
        rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        previousLastNode = NO_RECORD;
        while(rcode2 == 0 &&
              numEntries < MAXSPANENTRIES)
        {
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          if(previousLastNode != NO_RECORD)
          {
            SpanCheck[numEntries].ATime = GTResults.firstNodeTime;
          }

          NODESKey0.recordID = GTResults.firstNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          SpanCheck[numEntries].ALabel = NODES.reliefLabels[serviceIndex];
          if(SpanCheck[numEntries].ALabel == ' ')
          {
            SpanCheck[numEntries].ALabel = '\0';
          }
          NODESKey0.recordID = GTResults.lastNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          SpanCheck[numEntries].BLabel = NODES.reliefLabels[serviceIndex];
          if(SpanCheck[numEntries].BLabel == ' ')
          {
            SpanCheck[numEntries].BLabel = '\0';
          }
          SpanCheck[numEntries].BTime = GTResults.lastNodeTime;
          if(previousLastNode != NO_RECORD)
          {
            if(GTResults.firstNODESrecordID != previousLastNode)
            {
              GCTData.fromPATTERNNAMESrecordID = previousPatternNamesRecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = previousRouteRecordID;
              GCTData.fromSERVICESrecordID = previousServiceRecordID;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = previousLastNode;
              GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
              GCTData.timeOfDay = SpanCheck[numEntries - 1].BTime;
              deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(deadheadTime != NO_TIME)
              {
                SpanCheck[numEntries - 1].BTime += deadheadTime;
              }
            }
          }
          numEntries++;
          if(TRIPS.recordID == pPR->piece[nI].toTRIPSrecordID)
          {
            break;
          }
          previousLastNode = GTResults.lastNODESrecordID;
          previousRouteRecordID = TRIPS.ROUTESrecordID;
          previousServiceRecordID = TRIPS.SERVICESrecordID;
          previousPatternNamesRecordID = TRIPS.PATTERNNAMESrecordID;
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        }
        if(numEntries > 0)
        {
          SpanCheck[numEntries - 1].BTime = pPR->piece[nI].toTime + pCOST->TRAVEL[nI].endTravelTime +
                  pCOST->TRAVEL[nI].endDwellTime + pCOST->PIECECOST[nI].turninTime;
        }
      }
//
//  Ok, all the spanCheck times have been recorded.  Go through all
//  of them to see if we've got the break. minLayTime is the minimum
//  layover time that must occur.  minLaySpan is used to tell the runcutter
//  that minLayTime seconds must occur every minLaySpan minutes.
//
//  When minLaySpan is NO_TIME this is a no-brainer.
//  Just check to see if we've got the break at least once, and that we can break there
//
      bGotMinLayover = FALSE;
      bJustContinued = FALSE;
      minLayNotBefore = SpanCheck[0].ATime;
      if(RUNTYPE[nType][nSlot].minLayNotBefore != NO_TIME)
      {
        minLayNotBefore += RUNTYPE[nType][nSlot].minLayNotBefore;
      }
      if(RUNTYPE[nType][nSlot].minLaySpan == NO_TIME)
      {
        for(nI = 0; nI < numEntries - 1; nI++)
        {
          if(SpanCheck[nI].BTime < minLayNotBefore && RUNTYPE[nType][nSlot].minLayNotBefore != NO_TIME)
          {
            continue;
          }
          if(SpanCheck[nI + 1].ATime - SpanCheck[nI].BTime >= RUNTYPE[nType][nSlot].minLayTime &&
                (RUNTYPE[nType][nSlot].minLayLabel[0] == '\0' || SpanCheck[nI + 1].ALabel == RUNTYPE[nType][nSlot].minLayLabel[0]) &&
                (RUNTYPE[nType][nSlot].minLayLabel[0] == '\0' || SpanCheck[nI].BLabel == RUNTYPE[nType][nSlot].minLayLabel[0]))
          {
            bGotMinLayover = TRUE;
            break;
          }
        }
      }
//
//  Not a no-brainer.  Keep track of all the breaks.
//  When a break exceeds minLayTime, reset the span
//  start to the start of the current trip.
//
      if(!bGotMinLayover)
      {
        currentSpanStart = SpanCheck[0].ATime;
        bGotMinLayover = TRUE;
        bJustContinued = TRUE;
        for(nI = 0; nI < numEntries - 1; nI++)
        {
          if(SpanCheck[nI].BTime < minLayNotBefore && RUNTYPE[nType][nSlot].minLayNotBefore != NO_TIME)
          {
            continue;
          }
          if(RUNTYPE[nType][nSlot].minLayLabel[0] != '\0')
          {
            if(SpanCheck[nI + 1].ALabel != RUNTYPE[nType][nSlot].minLayLabel[0] ||
                  SpanCheck[nI].BLabel != RUNTYPE[nType][nSlot].minLayLabel[0])
            {
              continue;
            }
          }
          if(SpanCheck[nI].BTime - currentSpanStart > RUNTYPE[nType][nSlot].minLaySpan)
          {
            bGotMinLayover = FALSE;
            break;
          }
          else
          {
            if(nI <= numEntries -1)
            {
              if(SpanCheck[nI + 1].ATime - SpanCheck[nI].BTime >= RUNTYPE[nType][nSlot].minLayTime)
              {
                currentSpanStart = SpanCheck[nI + 1].ATime;
                minLayNotBefore = currentSpanStart + RUNTYPE[nType][nSlot].minLayNotBefore;
                bJustContinued = FALSE;
              }
            }
          }
        }
      }
      if(!bGotMinLayover || bJustContinued)
      {
        nI = (RUNTYPE[nType][nSlot].minLaySpan == NO_TIME) ? TEXT_152 : TEXT_255;
        LoadString(hInst, nI, tempString, TEMPSTRING_LENGTH);
        strcpy(runcosterReason, tempString);
        pCOST->runtype = UNCLASSIFIED_RUNTYPE;
      }
    }
  }
//
//  Sum everything up
//
  for(nJ = 0; nJ < pPR->numPieces; nJ++)
  {
    if(nJ == 0 && nJ == pPR->numPieces - 1)
    {
      if(pCOST->TRAVEL[nJ].flags & TRAVEL_FLAGS_STARTPAID)
      {
        pCOST->PIECECOST[nJ].premiumTime += pCOST->TRAVEL[nJ].startTravelTime;
      }
      if(pCOST->TRAVEL[nJ].flags & TRAVEL_FLAGS_ENDPAID)
      {
        pCOST->PIECECOST[nJ].premiumTime += pCOST->TRAVEL[nJ].endTravelTime;
      }
    }
    else
    {
      pCOST->PIECECOST[nJ].premiumTime += pCOST->TRAVEL[nJ].startTravelTime;
      pCOST->PIECECOST[nJ].premiumTime += pCOST->TRAVEL[nJ].endTravelTime;
    }
    pCOST->PIECECOST[nJ].premiumTime += pCOST->PIECECOST[nJ].reportTime;
    pCOST->PIECECOST[nJ].premiumTime += pCOST->PIECECOST[nJ].turninTime;
    pCOST->PIECECOST[nJ].premiumTime += pCOST->PIECECOST[nJ].paidMeal;
    pCOST->TOTAL.premiumTime += pCOST->PIECECOST[nJ].premiumTime;
    pCOST->TOTAL.reportTime += pCOST->PIECECOST[nJ].reportTime;
    pCOST->TOTAL.turninTime += pCOST->PIECECOST[nJ].turninTime;
  }
//
//  See if we have to bring up individual pieces to a minimum pay time
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    for(nJ = 0; nJ < pPR->numPieces; nJ++)
    {
      if(RUNTYPE[nType][nSlot].PIECE[nJ].minPayTime != NO_TIME)
      {
        tempLong = pCOST->PIECECOST[nJ].platformTime + pCOST->PIECECOST[nJ].premiumTime; 
        if(tempLong < RUNTYPE[nType][nSlot].PIECE[nJ].minPayTime)
        {
          BOOL bApply = TRUE;
          if(RUNTYPE[nType][nSlot].PIECE[nJ].flags & PIECEFLAGS_PAYONLYAPPLIES)
          {
            if(!((RUNTRIP.from[nJ].flags & RCFLAGS_NODEGAR) ||
                  IsGarage(pCOST->TRAVEL[nJ].startNODESrecordID)))
            {
              bApply = FALSE;
            }
          }
          if(pCOST->PIECECOST[nJ].paidBreak > 0)  // If we're paying through, don't bring this up
          {
            bApply = FALSE;
          }
          if(bApply)
          {
            pCOST->TOTAL.makeUpTime += (RUNTYPE[nType][nSlot].PIECE[nJ].minPayTime - tempLong);
          }
        }
      }
    }
//
//  See if we have to bring this guy up to min pay before adding premiums (!!!)
//
    if(RUNTYPE[nType][nSlot].flags & RTFLAGS_BRINGUPTO &&
          pCOST->TOTAL.platformTime < RUNTYPE[nType][nSlot].minPayTime)
      pCOST->TOTAL.makeUpTime = RUNTYPE[nType][nSlot].minPayTime - pCOST->TOTAL.platformTime;
//
//  Add it into the the premium time total
//
    pCOST->TOTAL.premiumTime += pCOST->TOTAL.makeUpTime;
    minutesThatDontCount += pCOST->TOTAL.makeUpTime;
  }
//
//  Any overtime?
//
  dailyOvertime = 0;
  sumOfTimes = 0;
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    sumOfTimes = pCOST->TOTAL.platformTime + pCOST->TOTAL.makeUpTime +
          pCOST->TOTAL.premiumTime - minutesThatDontCount;
    OTIndex = RUNTYPE[nType][nSlot].flags & RTFLAGS_4DAY ? 0 : 1;
    if(sumOfTimes > OVERTIME.dailyAfter[OTIndex][serviceIndex] &&
          OVERTIME.dailyAfter[OTIndex][serviceIndex] != NO_TIME)
    {
      dailyOvertime = (long)((sumOfTimes - OVERTIME.dailyAfter[OTIndex][serviceIndex]) *
            (OVERTIME.dailyRate[OTIndex][serviceIndex] - 1));
//
//  Pay overtime minute by minute (default)
//
      if(OVERTIME.payMinuteByMinute)
      {
      }
//
//  Pay overtime in "bounded" mode
//
      else if(!(OVERTIME.flags & OTF_ROUNDUPTONEXT))
      {
        if(OVERTIME.roundedBounded != 0)
        {
          dailyOvertime = (long)((((dailyOvertime + 120) / 
                     OVERTIME.roundedBounded) * OVERTIME.roundedBounded) *
                (OVERTIME.dailyRate[OTIndex][serviceIndex] - 1));
        }
      }
      bump = dailyOvertime % 60;
      if(bump != 0 && dailyOvertime != 0 && !(OVERTIME.flags & OTF_KEEPHALF))
      {
        if(OVERTIME.flags & OTF_ROUNDUP)
        {
          dailyOvertime += (60 - bump);
        }
        else
        {
          dailyOvertime -= bump;
        }
      }
    }
  }
//
//  And put in the spread overtime
//
  actualSpreadTime = pPR->piece[lastPiece].toTime + pCOST->PIECECOST[lastPiece].turninTime +
         pCOST->TRAVEL[lastPiece].endTravelTime -
         (pPR->piece[0].fromTime - pCOST->TRAVEL[0].startTravelTime - pCOST->PIECECOST[0].reportTime);
  spreadOvertime = 0;
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE &&
        RUNTYPE[nType][nSlot].flags & RTFLAGS_PAYSPDOT)
  {
    if(OVERTIME.spreadAfter[serviceIndex] != NO_TIME &&
          pCOST->runtype != UNCLASSIFIED_RUNTYPE && pPR->numPieces > 1)
    {
//
//  Not sure why this was here - someone will complain later, I suppose
//
//      if(sumOfTimes > OVERTIME.dailyAfter[OTIndex][serviceIndex])
//      {
//        actualSpreadTime -= (sumOfTimes - OVERTIME.dailyAfter[OTIndex][serviceIndex]);
//      }
      if(actualSpreadTime > OVERTIME.spreadAfter[serviceIndex])
      {
        spreadOvertime = (long)((actualSpreadTime - OVERTIME.spreadAfter[serviceIndex]) *
              (OVERTIME.spreadRate[serviceIndex] - 1));
//
//  Pay spread overtime minute by minute (default)
//
        if(OVERTIME.payMinuteByMinute)
        {
        }
//
//  Pay spread overtime by rounding up the next specified interval
//
        else if(OVERTIME.flags & OTF_ROUNDUPTONEXT)
        {
          if(OVERTIME.roundedBounded != NO_TIME && OVERTIME.roundedBounded != 0)
          {
            long roundedTo = OVERTIME.roundedBounded * 60;

            if(spreadOvertime % roundedTo != 0)
              spreadOvertime += (roundedTo - (spreadOvertime % roundedTo));
          }
        }
//
//  Pay spread overtime in "bounded" mode
//
        else
        {
          if(OVERTIME.roundedBounded != 0)
          {
            spreadOvertime = (long)((((spreadOvertime + 120) / 
                       OVERTIME.roundedBounded) * OVERTIME.roundedBounded) *
                  (OVERTIME.spreadRate[serviceIndex] - 1));
          }
        }
        bump = spreadOvertime % 60;
        if(bump != 0 && spreadOvertime != 0 && !(OVERTIME.flags & OTF_KEEPHALF))
        {
          if(OVERTIME.flags & OTF_ROUNDUP)
          {
            spreadOvertime += (60 - bump);
          }
          else
          {
            spreadOvertime -= bump;
          }
        }
//
//  Adjust it if we're paying makeup before spread
//
        if(!(OVERTIME.flags & OTF_PAYMAKEUPBEFORESPREAD))
        {
          bump = spreadOvertime % 60;  // Do any necessary rounding
          if(bump != 0 && !(OVERTIME.flags & OTF_KEEPHALF))
          {
            if(OVERTIME.flags & OTF_ROUNDUP)
              spreadOvertime += (60 - bump);
            else
              spreadOvertime -= bump;
          }
        }
      }
    }
  }
//
//  And the actual spread time
//
//  pCOST->spreadTime = actualSpreadTime;
  pCOST->spreadTime = pPR->piece[lastPiece].toTime + pCOST->PIECECOST[lastPiece].turninTime +
         pCOST->TRAVEL[lastPiece].endTravelTime -
         (pPR->piece[0].fromTime -
         pCOST->TRAVEL[0].startTravelTime - pCOST->PIECECOST[0].reportTime);

//
//  And see which of spread/dialy overtime gets paid
//
  if(OVERTIME.flags & OTF_PAYLOWER)
  {
    if(spreadOvertime < dailyOvertime)
    {
      if(!(OVERTIME.flags & OTF_PAYMAKEUPBEFORESPREAD))
      {
        pCOST->spreadOvertime = spreadOvertime;
        pCOST->TOTAL.payTime += pCOST->spreadOvertime;
        pCOST->TOTAL.premiumTime += pCOST->spreadOvertime;
      }
    }
    else
    {
      pCOST->TOTAL.overTime = dailyOvertime;
    }
  }
  else if(OVERTIME.flags & OTF_PAYHIGHER)
  {
    if(spreadOvertime > dailyOvertime)
    {
      if(!(OVERTIME.flags & OTF_PAYMAKEUPBEFORESPREAD))
      {
        pCOST->spreadOvertime = spreadOvertime;
        pCOST->TOTAL.payTime += pCOST->spreadOvertime;
        pCOST->TOTAL.premiumTime += pCOST->spreadOvertime;
      }
    }
    else
    {
      pCOST->TOTAL.overTime = dailyOvertime;
    }
  }
  else if(OVERTIME.flags & OTF_PAYBOTH)
  {
    if(!(OVERTIME.flags & OTF_PAYMAKEUPBEFORESPREAD))
    {
      if(pCOST->spreadTime > OVERTIME.spreadAfter[serviceIndex])
      {
//        spreadOvertime = (long)((pCOST->spreadTime - OVERTIME.spreadAfter[serviceIndex]) *
//               (OVERTIME.spreadRate[serviceIndex] - 1));
        pCOST->spreadOvertime = spreadOvertime;
        pCOST->TOTAL.payTime += pCOST->spreadOvertime;
        pCOST->TOTAL.premiumTime += pCOST->spreadOvertime;
      }
    }
    pCOST->TOTAL.overTime = dailyOvertime;
  }
//
//  Make sure we haven't blown over the maximum time for this runtype,
//  or bring it up to the minimum pay if necessary
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    sumOfTimes = pCOST->TOTAL.platformTime + pCOST->TOTAL.makeUpTime +
        pCOST->TOTAL.premiumTime + pCOST->TOTAL.overTime;
//
//  Round the pay up the next specified interval
//
    if(OVERTIME.flags & OTF_ROUNDUPTONEXT)
    {
      if(dailyOvertime > 0)
      {
        if(OVERTIME.roundedBounded != NO_TIME && OVERTIME.roundedBounded != 0)
        {
          long roundedTo = OVERTIME.roundedBounded * 60;
        
          if(sumOfTimes % roundedTo != 0)
          {
            pCOST->TOTAL.overTime += (roundedTo - (sumOfTimes % roundedTo));
            sumOfTimes += (roundedTo - (sumOfTimes % roundedTo));
          }
        }
      }
    }
    if(sumOfTimes < RUNTYPE[nType][nSlot].minPayTime)
    {
      pCOST->TOTAL.makeUpTime += RUNTYPE[nType][nSlot].minPayTime - sumOfTimes;
      pCOST->TOTAL.premiumTime += pCOST->TOTAL.makeUpTime;
    }
//    else if(RUNTYPE[nType][nSlot].flags & RTFLAGS_BRINGUPTO)
//    {
//      pCOST->TOTAL.premiumTime += pCOST->TOTAL.makeUpTime;
//    }
    if(OVERTIME.flags & OTF_PAYMAKEUPBEFORESPREAD)
    {
      pCOST->spreadOvertime = spreadOvertime;
      pCOST->TOTAL.payTime += pCOST->spreadOvertime;
      pCOST->TOTAL.premiumTime += pCOST->spreadOvertime;
    }
    pCOST->TOTAL.payTime = pCOST->TOTAL.platformTime +
          pCOST->TOTAL.premiumTime + pCOST->TOTAL.overTime;
    if(pCOST->TOTAL.payTime > RUNTYPE[nType][nSlot].maxPayTime && cutAsRuntype == NO_RUNTYPE)
    {
      LoadString(hInst, TEXT_089, tempString, TEMPSTRING_LENGTH);
      sprintf(runcosterReason, tempString, RUNTYPE[nType][nSlot].localName);
      pCOST->runtype = UNCLASSIFIED_RUNTYPE;
    }
  }
//
//  One last check - is what we're paying at least equal to the minimum pay time for the run?
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    if(pCOST->TOTAL.payTime < RUNTYPE[nType][nSlot].minPayTime)
    {
      pCOST->TOTAL.makeUpTime += RUNTYPE[nType][nSlot].minPayTime - pCOST->TOTAL.payTime;
      pCOST->TOTAL.payTime += RUNTYPE[nType][nSlot].minPayTime - pCOST->TOTAL.payTime;
    }
  }
//
//  Do any necessary rounding
//
  if(pCOST->runtype != UNCLASSIFIED_RUNTYPE)
  {
    bump = pCOST->TOTAL.payTime % 60;
    if(bump != 0 && !(OVERTIME.flags & OTF_KEEPHALF))
    {
      if(OVERTIME.flags & OTF_ROUNDUP)
        pCOST->TOTAL.payTime += (60 - bump);
      else
        pCOST->TOTAL.payTime -= bump;
    }
  }

  return(pCOST->runtype);
}

long GetClosestGarage(long flags, RUNTRIPDef *pRT, long *pDwellTime,
      long *pClosestGarageRecordID, long fromNode, long fromTime, int nPrem, int piece)
{
  GetConnectionTimeDef GCTData;
  float distance;
  BOOL bAtGarage;
  BOOL bBeAtGarageBeforeTime = flags & CLOSEST_FLAG_BEATGARAGEBEFORETIME;
  BOOL bReverse = flags & CLOSEST_FLAG_REVERSE;
  long travelTime;
  long bestTime;
  long startTime;
  long endTime;
  long equivalentTravelTime;
  long testTime;
  int  nI;

  bestTime = NO_TIME;
  bAtGarage = FALSE;
  *pDwellTime = 0;
  *pClosestGarageRecordID = NO_RECORD;

  for(nI = 0; nI < numGaragesInGarageList; nI++)
  {
    GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
    GCTData.toPATTERNNAMESrecordID = NO_RECORD;
    GCTData.fromROUTESrecordID = pRT->to[piece].RGRPROUTESrecordID;
    GCTData.fromSERVICESrecordID = pRT->to[piece].SGRPSERVICESrecordID;
    GCTData.toROUTESrecordID = pRT->to[piece].RGRPROUTESrecordID;
    GCTData.toSERVICESrecordID = pRT->to[piece].SGRPSERVICESrecordID;
    if(bReverse)
    {
      GCTData.toNODESrecordID = fromNode;
      GCTData.fromNODESrecordID = garageList[nI];
    }
    else
    {
      GCTData.fromNODESrecordID = fromNode;
      GCTData.toNODESrecordID = garageList[nI];
    }
    GCTData.timeOfDay = fromTime;
    if((travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance)) != NO_TIME)
    {
      if(bestTime == NO_TIME || travelTime < bestTime)
      {
        *pClosestGarageRecordID = garageList[nI];
        bestTime = travelTime;
        if(fromNode == garageList[nI])
          bAtGarage = TRUE;
      }
    }
  }

//
//  We know the closest garage at this point, see if bestTime has to be adjusted
//
  if(bestTime != NO_TIME)
  {
    if(PREMIUM[nPrem].payTravelTime == WORKRULES_TIMEENTERED)
      bestTime = PREMIUM[nPrem].time;
    else if(PREMIUM[nPrem].payTravelTime == WORKRULES_ASPERSCHEDULE ||
          PREMIUM[nPrem].payTravelTime == WORKRULES_ASPERSCHEDULEDWELL)
    {
      if(!bAtGarage)
      {
        if(bUseCISPlan)
        {
          if(bReverse)
          {
            if(NodesEquivalent(fromNode, *pClosestGarageRecordID, &equivalentTravelTime))
            {
              bestTime = equivalentTravelTime;
            }
            else
            {
              CISplanReliefConnect(&startTime, &endTime, pDwellTime, 0, *pClosestGarageRecordID, 
                    fromNode, fromTime, !bBeAtGarageBeforeTime,
                    pRT->to[0].SGRPSERVICESrecordID);
              if(startTime == -25 * 60 * 60)
              {
                bestTime = NO_TIME;
              }
              else
              {
                bestTime =  (endTime - startTime) + *pDwellTime;
              }
            }
          }
          else
          {
            if(NodesEquivalent(fromNode, *pClosestGarageRecordID, &equivalentTravelTime))
            {
              bestTime = equivalentTravelTime;
            }
            else
            {
              CISplanReliefConnect(&startTime, &endTime, pDwellTime, 0, fromNode,
                    *pClosestGarageRecordID, fromTime, !bBeAtGarageBeforeTime,
                    pRT->to[0].SGRPSERVICESrecordID);
              if(startTime == -25 * 60 * 60)
              {
                bestTime = NO_TIME;
              }
              else
              {
                bestTime =  (endTime - startTime) + *pDwellTime;
              }
            }
          }
        }
        else
        {
          if(bReverse)
          {
            testTime = GetDynamicTravelTime(bBeAtGarageBeforeTime, *pClosestGarageRecordID, 
                  fromNode, pRT->to[piece].SGRPSERVICESrecordID, fromTime, pDwellTime);
            if(testTime < bestTime && testTime != NO_TIME)
            {
              bestTime = testTime;
            }
          }
          else
          {
            testTime = GetDynamicTravelTime(bBeAtGarageBeforeTime, fromNode,
                  *pClosestGarageRecordID, pRT->to[piece].SGRPSERVICESrecordID, fromTime, pDwellTime);
            if(testTime < bestTime && testTime != NO_TIME)
            {
              bestTime = testTime;
            }
          }
        }
      }
    }
  }

  return(bestTime);
}

BOOL GetPieceTravel(BOOL bRecursive, int nPrem, int nPce, int nType, int nSlot,
      RUNTRIPDef *pRT, PROPOSEDRUNDef *pPR, COSTDef *pCOST, long *pTravelTime, long *pDwellTime)
{
  GetConnectionTimeDef GCTData;
  float distance;
  int  nMethod;
  int  nNxtPce = nPce + 1;
  int  nI;
  int  travelInBreak;
  int  payWhen;
  BOOL bFound;
  long BNode;
  long CNode;
  long minBreakTime;
  long BToInt;
  long intermediateNode;
  long sum;
  long travelTime[2];
  long startTime;
  long endTime;
  long equivalentTravelTime;
  long travelChosen;
  long totalTravel;
  long largestBreakTime;
  long largestBreakPieceBefore;
  BOOL bRecordOnAB;
  BOOL bForceTravelDirect;
  BOOL bValidTravel;
//
//  Zero out the travel and wait times
//
  *pTravelTime = 0;
  *pDwellTime = 0;
//
//  If we're here on a bogus PREMIUM call, leave
//
  if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_NA)
  {
    LoadString(hInst, ERROR_241, runcosterReason, sizeof(runcosterReason));
    return(FALSE);
  }
//
//  First off, figure out how we're travelling.
//
//  When bRecursive is TRUE, it means we're checking
//  on feasible travel from within this routine.
//
  if(bRecursive)
  {
    travelInBreak = FALSE;
    payWhen = FALSE;
  }
  else
  {
    travelInBreak = PREMIUM[nPrem].travelInBreak;
    payWhen = PREMIUM[nPrem].payWhen;
  }
//
//  And where from and where to
//
  BNode =  pPR->piece[nPce].toNODESrecordID;
  CNode = pPR->piece[nNxtPce].fromNODESrecordID;
//
//  And if this is a multi-piece run and we're looking at a break
//  that isn't the larger (or largest), then travel him direct only 
//  when it isn't feasible to travel him to the intermediate point
//  and out again.
//
  bForceTravelDirect = FALSE;
//
//  Don't go through this on a recursive call
//
  if(!bRecursive)
  {
    if(RUNTYPE[nType][nSlot].numPieces > 2)
    {
      if(PREMIUM[nPrem].pieceTravel != BCTRAVEL_BTOCDIRECT)
      {
        largestBreakTime = NO_TIME;
        largestBreakPieceBefore = 0;
        for(nI = 0; nI < RUNTYPE[nType][nSlot].numPieces - 1; nI++)
        {
          if(pPR->piece[nI + 1].fromTime - pPR->piece[nI].toTime > largestBreakTime)
          {
            largestBreakTime = pPR->piece[nI + 1].fromTime - pPR->piece[nI].toTime;
            largestBreakPieceBefore = nI;
          }
        }
        if(largestBreakPieceBefore != nPce)
        {
          if(!(PREMIUM[nPrem].flags & PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE))
          {
            bForceTravelDirect = TRUE;
          }
          else
          {
            bValidTravel = GetPieceTravel(TRUE, nPrem, nPce, nType, nSlot, pRT, pPR, pCOST, pTravelTime, pDwellTime);
            if(!bValidTravel || *pTravelTime + *pDwellTime > pPR->piece[nPce + 1].fromTime - pPR->piece[nPce].toTime)
            {
              bForceTravelDirect = TRUE;
            }
          }
        }
      }
    }
  }
  
//
//  BCTRAVEL_BTOCDIRECT
//
  if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTOCDIRECT || bForceTravelDirect)
  {
    nMethod = BCTRAVEL_BTOCDIRECT;
//
//  If the B Point is equivalenced to a garage, change over
//  to BCTRAVEL_BTONODE and make the intermediate point the garage.
//
    for(nI = 0; nI < numGaragesInGarageList; nI++)
    {
      if(BNode == garageList[nI])
        continue;
      if(NodesEquivalent(BNode, garageList[nI], &equivalentTravelTime))
      {
        intermediateNode = garageList[nI];
        nMethod = BCTRAVEL_BTONODE;
        BToInt = equivalentTravelTime;
        break;
      }
    }
  }
//
//  Now the intermediate crap - determine the intermediate point
//
  else
  {
    nMethod = BCTRAVEL_BTONODE;
    BToInt = NO_TIME;
//
//  Intermediate: Closest garage
//
    if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTOCLOSESTTOC)
    {
      long flags = CLOSEST_FLAG_BEATGARAGEBEFORETIME;

      GetClosestGarage(TRUE, pRT, pDwellTime, &intermediateNode, 
            pPR->piece[nPce].toNODESrecordID, pPR->piece[nPce].fromTime, nPrem, nPce);
    }
//
//  Intermediate: BNode's POG
//
    else if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTOBPOGTOC)
    {
      intermediateNode = pRT->from[nPce].blockPOGNODESrecordID;
      if(intermediateNode == NO_RECORD)
      {
        LoadString(hInst, TEXT_087, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString, nPce);
        return(FALSE);
      }
    }
//
//  Intermediate: BNode's PIG
//
    else if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTOBPIGTOC)
    {
      intermediateNode = pRT->from[nPce].blockPIGNODESrecordID;
      if(intermediateNode == NO_RECORD)
      {
        LoadString(hInst, TEXT_088, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString, nPce);
        return(FALSE);
      }
    }
//
//  Intermediate: CNode's POG
//
    else if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTOCPOGTOC)
    {
      intermediateNode = pRT->from[nNxtPce].blockPOGNODESrecordID;
      if(intermediateNode == NO_RECORD)
      {
        LoadString(hInst, TEXT_087, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString, nPce);
        return(FALSE);
      }
    }
//
//  Intermediate: CNode's PIG
//
    else if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTOCPIGTOC)
    {
      intermediateNode = pRT->from[nNxtPce].blockPIGNODESrecordID;
      if(intermediateNode == NO_RECORD)
      {
        LoadString(hInst, TEXT_088, tempString, TEMPSTRING_LENGTH);
        sprintf(runcosterReason, tempString, nPce);
        return(FALSE);
      }
    }
//
//  Intermediate: Specific node
//
    else if(PREMIUM[nPrem].pieceTravel == BCTRAVEL_BTONODE)
    {
      intermediateNode = PREMIUM[nPrem].payBtoNODESrecordID;
      if(PREMIUM[nPrem].flags & PREMIUMFLAGS_DONTTRAVELBEQC)
      {
        if(NodesEquivalent(BNode, CNode, &equivalentTravelTime))
        {
          nMethod = BCTRAVEL_BTOCDIRECT;
        }
      }
    }
//
//  Fall through - bad value for PREMIUM[].pieceTravel
//
    else
    {
      LoadString(hInst, TEXT_261, tempString, TEMPSTRING_LENGTH);
      sprintf(runcosterReason, tempString, nPrem);
      return(FALSE);
    }
  }
//
//  Now that we know how, let's do it
//
  minBreakTime = RUNTYPE[nType][nSlot].PIECE[nPce].minBreakTime;
  if(minBreakTime == NO_TIME)
    minBreakTime = 0;
//
//  nMethod: BTOCDIRECT
//
//  All have to be evaluated twice in order to determine which is the best
//  way of actually travelling this guy: B->C then break, or break then B->C.
//
//  The first evaluated travel sits in travelTime[0], the second in travelTime[1]
//
  if(nMethod == BCTRAVEL_BTOCDIRECT)
  {
//
//  Actual travel calculated as hard-wired time
//
    if(PREMIUM[nPrem].payTravelTime == WORKRULES_TIMEENTERED)
    {
      travelTime[0] = PREMIUM[nPrem].time;
      travelTime[1] = PREMIUM[nPrem].time;
    }
//
//  Actual travel calculated as per the connections table
//
    else if(PREMIUM[nPrem].payTravelTime == WORKRULES_ASINCONNECTIONS)
    {
      GCTData.fromNODESrecordID = BNode;
      GCTData.fromROUTESrecordID = pRT->to[nPce].RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = pRT->to[nPce].SGRPSERVICESrecordID;
      GCTData.fromPATTERNNAMESrecordID = pRT->to[nPce].PATTERNNAMESrecordID;
      GCTData.toNODESrecordID = CNode;
      GCTData.toROUTESrecordID = pRT->from[nNxtPce].RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = pRT->from[nNxtPce].SGRPSERVICESrecordID;
      GCTData.toPATTERNNAMESrecordID = pRT->from[nNxtPce].PATTERNNAMESrecordID;
      GCTData.timeOfDay = pPR->piece[nPce].toTime;
      travelTime[0] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
      GCTData.timeOfDay = pPR->piece[nNxtPce].fromTime;
      travelTime[1] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
    }
//
//  Actual travel calculated by scheduled or scheduled + dwell
//
    else
    {
//
//  Use the trip planner
//
      if(bUseCISPlan)
      {
        travelTime[0] = 0;
        travelTime[1] = 0;
        if(pPR->piece[nNxtPce].fromTime != NO_TIME)
        {
          CISplanReliefConnect(&startTime, &endTime, pDwellTime, 0, BNode, CNode,
                pPR->piece[nPce].toTime, TRUE, pRT->to[nPce].SGRPSERVICESrecordID);
          if(startTime == -25 * 60 * 60)
            travelTime[0] = NO_TIME;
          else
            travelTime[0] =  (endTime - startTime) + *pDwellTime;
          CISplanReliefConnect(&startTime, &endTime, pDwellTime, 0, BNode, CNode,
                pPR->piece[nNxtPce].fromTime, FALSE, pRT->to[nPce].SGRPSERVICESrecordID);
          if(startTime == -25 * 60 * 60)
            travelTime[1] = NO_TIME;
          else
            travelTime[1] = ((endTime - startTime) + *pDwellTime);
        }
      }
//
//  Or use dynamic travels
//
      else
      {
        travelTime[0] = GetDynamicTravelTime(FALSE, BNode, CNode,
              pRT->to[nPce].SGRPSERVICESrecordID, pPR->piece[nPce].toTime, pDwellTime);
        travelTime[1] = GetDynamicTravelTime(TRUE, BNode, CNode,
              pRT->to[nPce].SGRPSERVICESrecordID, pPR->piece[nNxtPce].fromTime, pDwellTime);
      }
//
//  Fall through to "Use conn if no sched"
//
      if(PREMIUM[nPrem].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
      {
        for(nI = 0; nI < 2; nI++)
        {
          if(travelTime[nI] == NO_TIME)
          {
            GCTData.fromNODESrecordID = BNode;
            GCTData.fromROUTESrecordID = pRT->to[nPce].RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = pRT->to[nPce].SGRPSERVICESrecordID;
            GCTData.fromPATTERNNAMESrecordID = pRT->to[nPce].PATTERNNAMESrecordID;
            GCTData.toNODESrecordID = CNode;
            GCTData.toROUTESrecordID = pRT->from[nNxtPce].RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = pRT->from[nNxtPce].SGRPSERVICESrecordID;
            GCTData.toPATTERNNAMESrecordID = pRT->from[nNxtPce].PATTERNNAMESrecordID;
            if(nI == 0)
            {
              GCTData.timeOfDay = pPR->piece[nPce].toTime;
              travelTime[nI] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
            }
            else
            {
              GCTData.timeOfDay = pPR->piece[nNxtPce].toTime;
              travelTime[nI] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
            }
            if(bUseDynamicTravels && !bUseCISPlan)
              numTravelInstructions--;
          }
        }
      }
    }
//
//  If there's no travel time, set up the reason and leave
//
    if(travelTime[0] == NO_TIME && travelTime[1] == NO_TIME)
    {
      LoadString(hInst, TEXT_084, tempString, TEMPSTRING_LENGTH);
      sprintf(runcosterReason, tempString, NodeAndNode(BNode, CNode));
      return(FALSE);
    }
    if(payWhen & PAYWHEN_DIRECT || bForceTravelDirect)
    {
      *pTravelTime = travelTime[0];
//
//  Also, figure out how this guy is actually going to travel.  There's a flag that
//  can get set on the BCTravel dialog that will force the issue - i.e. put the travel
//  on the AB piece (at the end) as opposed to having the break at the B point (if applicable)
//  and then travelling him out to C just prior to his C departure time.
//
      bRecordOnAB = FALSE;
      if(PREMIUM[nPrem].flags & PREMIUMFLAGS_RECORDONAB)
      {
        for(bFound = FALSE, nI = 0; nI < numGaragesInGarageList; nI++)
        {
          if(garageList[nI] == CNode)
          {
            bFound = TRUE;
            break;
          }
        }
        if(bFound)
          bRecordOnAB = TRUE;
      }
//
//  Or, if the travel is better on the AB piece, put it there
//
      if(!bRecordOnAB && travelTime[0] < travelTime[1])
        bRecordOnAB = TRUE;
//
//  If we're forcing the issue, then put the travel at the end of the AB piece
//
      if(bRecordOnAB)
      {
        travelChosen = travelTime[0];
        pCOST->TRAVEL[nPce].endBPointNODESrecordID = BNode;
        pCOST->TRAVEL[nPce].endBPointTime = pPR->piece[nPce].toTime;
        pCOST->TRAVEL[nPce].endNODESrecordID = CNode;
        pCOST->TRAVEL[nPce].endTravelTime = travelTime[0];
        pCOST->TRAVEL[nPce].endDwellTime = *pDwellTime;
      }
//
//  We're not forcing the issue.  This means that we can evaluate possibly two potential
//  travel scenarios and pick the best one.  (Break at B travel to C, or travel B->C then
//  break)
//
      else
      {
        travelChosen = travelTime[1];
        pCOST->TRAVEL[nNxtPce].startNODESrecordID = BNode;
        pCOST->TRAVEL[nNxtPce].startAPointNODESrecordID = CNode;
        pCOST->TRAVEL[nNxtPce].startAPointTime = pPR->piece[nNxtPce].fromTime;
        pCOST->TRAVEL[nNxtPce].startTravelTime = travelTime[1];
        pCOST->TRAVEL[nNxtPce].startDwellTime = *pDwellTime;
      }
//
//  If the break plus the travel times exceed the interval, it's illegal unless the
//  break time is included in the travel.  Note that the sum calculation figures out
//  the longest of the travel and break times in just such a case.
//
      if(bForceTravelDirect)
      {
        sum = travelChosen;
      }
      else
      {
        if(*pDwellTime >= minBreakTime ||
              pPR->piece[nNxtPce].fromTime - travelChosen - pPR->piece[nPce].toTime >= minBreakTime)
        {
          if(pPR->piece[nNxtPce].fromTime - travelChosen - pPR->piece[nPce].toTime < 0)
            sum = travelChosen;
          else
            sum = 0;
        }
        else
        {
          sum = max(travelChosen, minBreakTime) + (travelInBreak ? 0 : min(minBreakTime, travelChosen));
        }
      }
      if(pPR->piece[nPce].toTime + sum <= pPR->piece[nNxtPce].fromTime)
        return(TRUE);
      else
      {
        if(!bRecursive)
        {
          LoadString(hInst, TEXT_086, runcosterReason, sizeof(runcosterReason));
        }
        return(FALSE);
      }
    }
  }
//
//  Finished B->C Direct
//
//  Now do B->C with an intermediate point
//
  else
  {
//
//  B to intermediate.  If BToInt is NO_TIME, it means that there
//  wasn't an equivalence between B and a Garage under BCDIRECT.  If there was,
//  we would have set BToInt as the equivalentTravelTime earlier. 
//
    if(BToInt != NO_TIME)
    {
      travelTime[0] = BToInt;
    }
//
//  Are they equivalent?
//
    else if(NodesEquivalent(BNode, intermediateNode, &equivalentTravelTime))
    {
      travelTime[0] = equivalentTravelTime;
    }
//
//  Get the time
//
    else
    {
//
//  Set up GCTData in case we fall through to "Use conn if no sched"
//
      GCTData.fromNODESrecordID = BNode;
      GCTData.fromROUTESrecordID = pRT->to[nPce].RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = pRT->to[nPce].SGRPSERVICESrecordID;
      GCTData.fromPATTERNNAMESrecordID = pRT->to[nPce].PATTERNNAMESrecordID;
      GCTData.toNODESrecordID = intermediateNode;
      GCTData.toROUTESrecordID = pRT->from[nNxtPce].RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = pRT->from[nNxtPce].SGRPSERVICESrecordID;
      GCTData.toPATTERNNAMESrecordID = pRT->from[nNxtPce].PATTERNNAMESrecordID;
      GCTData.timeOfDay = pPR->piece[nPce].toTime;
//
//  B->Int: Actual travel calculated as hard-wired time
//
      if(PREMIUM[nPrem].payTravelTime == WORKRULES_TIMEENTERED)
      {
        travelTime[0] = PREMIUM[nPrem].time;
      }
//
//  B->Int: Actual travel calculated as per the connections table
//
      else if(PREMIUM[nPrem].payTravelTime == WORKRULES_ASINCONNECTIONS)
      {
        travelTime[0] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
      }
//
//  B->Int: Actual travel calculated by scheduled or scheduled + dwell
//
      else
      {
       *pDwellTime = 0;
        if(bUseCISPlan)
        {
          CISplanReliefConnect(&startTime, &endTime, pDwellTime, 0,
                BNode, intermediateNode, pPR->piece[nPce].toTime,
                TRUE, pRT->from[nNxtPce].SGRPSERVICESrecordID);
          if(startTime == -25 * 60 * 60)
            travelTime[0] = NO_TIME;
          else
            travelTime[0] =  (endTime - startTime) + *pDwellTime;
        }
        else
        {
          travelTime[0] = GetDynamicTravelTime(FALSE, BNode, intermediateNode,
                pRT->from[nNxtPce].SGRPSERVICESrecordID, pPR->piece[nPce].toTime, pDwellTime);
        }
//
//  B->Int: Fall through to "Use conn if no sched"
//
        if(PREMIUM[nPrem].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
        {
          if(travelTime[0] == NO_TIME)
          {
            travelTime[0] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
            if(bUseDynamicTravels && !bUseCISPlan)
              numTravelInstructions--;
          }
        }
      }
    }
//
//  Directly or indirectly, we now have a travel time
//
//  Or do we?
//
    if(travelTime[0] == NO_TIME)
    {
      LoadString(hInst, TEXT_084, tempString, TEMPSTRING_LENGTH);
      sprintf(runcosterReason, tempString, NodeAndNode(BNode, intermediateNode));
      return(FALSE);
    }
//
//  Yes we do - did we blow past the start of the next piece?
//  Note that we'll save the travel in break stuff for the next pass.
//
    else if(pPR->piece[nPce].toTime + travelTime[0] > pPR->piece[nNxtPce].fromTime)
    {
      if(!bRecursive)
      {
        LoadString(hInst, TEXT_086, runcosterReason, sizeof(runcosterReason));
      }
      return(FALSE);
    }
//
//  Save the travel belonging to the first piece (B->Int)
//
    if(BNode != intermediateNode)
    {
      pCOST->TRAVEL[nPce].endBPointNODESrecordID = BNode;
      pCOST->TRAVEL[nPce].endBPointTime = pPR->piece[nPce].toTime;
      pCOST->TRAVEL[nPce].endNODESrecordID = intermediateNode;
      pCOST->TRAVEL[nPce].endTravelTime = travelTime[0];
      pCOST->TRAVEL[nPce].endDwellTime = *pDwellTime;
    }
//
//  Now go from the intermediate point to C
//
//
//  Are they equivalent?
//
    if(NodesEquivalent(intermediateNode, CNode, &equivalentTravelTime))
    {
      travelTime[1] = equivalentTravelTime;
    }
//
//  Nope - get the time
//
//  Set up GCTData in case we fall through to "Use conn if no sched"
//
    else
    {
      GCTData.fromNODESrecordID = intermediateNode;
      GCTData.fromROUTESrecordID = pRT->to[nPce].RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = pRT->to[nPce].SGRPSERVICESrecordID;
      GCTData.fromPATTERNNAMESrecordID = pRT->to[nPce].PATTERNNAMESrecordID;
      GCTData.toNODESrecordID = CNode;
      GCTData.toROUTESrecordID = pRT->from[nNxtPce].RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = pRT->from[nNxtPce].SGRPSERVICESrecordID;
      GCTData.toPATTERNNAMESrecordID = pRT->from[nNxtPce].PATTERNNAMESrecordID;
      GCTData.timeOfDay = pPR->piece[nNxtPce].fromTime;
//
//  Int->C: Actual travel calculated as hard-wired time
//
      if(PREMIUM[nPrem].payTravelTime == WORKRULES_TIMEENTERED)
        travelTime[1] = PREMIUM[nPrem].time;
      else
      {
//
//  Int->C: Actual travel calculated as per the connections table
//
        if(PREMIUM[nPrem].payTravelTime == WORKRULES_ASINCONNECTIONS)
        {
          travelTime[1] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
        }
//
//  Int->C: Actual travel calculated by scheduled or scheduled + dwell
//
        else
        {
          if(NodesEquivalent(intermediateNode, CNode, &equivalentTravelTime))
            travelTime[1] = equivalentTravelTime;
          else
          {
            if(bUseCISPlan)
            {
              CISplanReliefConnect(&startTime, &endTime, pDwellTime, 0,
                    intermediateNode, CNode, pPR->piece[nNxtPce].fromTime,
                    FALSE, pRT->from[nNxtPce].SGRPSERVICESrecordID);
              if(startTime == -25 * 60 * 60)
                travelTime[1] = NO_TIME;
              else
                travelTime[1] =  (endTime - startTime) + *pDwellTime;
            }
            else
            {
              travelTime[1] = GetDynamicTravelTime(TRUE, intermediateNode, CNode,
                    pRT->from[nNxtPce].SGRPSERVICESrecordID,
                    pPR->piece[nNxtPce].fromTime, pDwellTime);
            }
//
//  Int->C: Fall through to "Use conn if no sched"
//
            if(PREMIUM[nPrem].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
            {
              if(travelTime[1] == NO_TIME)
              {
                travelTime[1] = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
                if(bUseDynamicTravels && !bUseCISPlan)
                  numTravelInstructions--;
              }
            }
          }
        }
      }
    }
//
//  Check the intermediate to C travel
//
    if(travelTime[1] == NO_TIME)
    {
      LoadString(hInst, TEXT_084, tempString, TEMPSTRING_LENGTH);
      sprintf(runcosterReason, tempString, NodeAndNode(intermediateNode, CNode));
      return(FALSE);
    }
//
//  Now check the sum of the B to intermediate and intermediate to C travels
//
    totalTravel = travelTime[0] + travelTime[1];
    sum = max(totalTravel, minBreakTime) +
          (PREMIUM[nPrem].travelInBreak ? 0: min(minBreakTime, totalTravel));
    if(pPR->piece[nPce].toTime + sum > pPR->piece[nNxtPce].fromTime)
    {
      if(!bRecursive)
      {
        LoadString(hInst, TEXT_086, runcosterReason, sizeof(runcosterReason));
      }
      return(FALSE);
    }
    pCOST->TRAVEL[nNxtPce].startNODESrecordID = intermediateNode;
    pCOST->TRAVEL[nNxtPce].startAPointTime = pPR->piece[nNxtPce].fromTime;
    pCOST->TRAVEL[nNxtPce].startAPointNODESrecordID = CNode;
    pCOST->TRAVEL[nNxtPce].startTravelTime = travelTime[1];
    pCOST->TRAVEL[nNxtPce].startDwellTime = *pDwellTime;
    *pTravelTime = totalTravel;
  }
  return(TRUE);
}

char *NodeAndNode(long firstNODESrecordID, long secondNODESrecordID)
{
  static char   outString[128];
  char   andString[16];
  char   nodeName[max(NODES_ABBRNAME_LENGTH + 1, 36)];
  int    rcode2;

  if((NODESKey0.recordID = firstNODESrecordID) == NO_RECORD)
    LoadString(hInst, TEXT_093, nodeName, sizeof(nodeName));
  else
  {
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(nodeName, NODES_ABBRNAME_LENGTH);
  }
  strcpy(outString, nodeName);

  LoadString(hInst, TEXT_091, andString, sizeof(andString));
  strcat(outString, andString);

  if((NODESKey0.recordID = secondNODESrecordID) == NO_RECORD)
    LoadString(hInst, TEXT_093, nodeName, sizeof(nodeName));
  else
  {
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(nodeName, NODES_ABBRNAME_LENGTH);
  }
  strcat(outString, nodeName);

  return &outString[0];
}

long GetPaidTravel(int nI, long travelTime)
{
  long paidTravel = 0;
  
  if(PREMIUM[nI].payHow == WORKRULES_PAYACTUAL)
    paidTravel = travelTime;
  else if(PREMIUM[nI].payHow == WORKRULES_PAYFLAT)
    paidTravel = PREMIUM[nI].payHowMinutes;
  else if(PREMIUM[nI].payHow == WORKRULES_PAYPERCENTAGE)
  {
    paidTravel = (long)(travelTime * PREMIUM[nI].payHowPercent);
    paidTravel -= paidTravel % 60;
  }
  else if(PREMIUM[nI].payHow == WORKRULES_PAYSTAGGERED)
  {
    if(travelTime < PREMIUM[nI].payHowMinutes)
    {
      paidTravel = (long)(travelTime * PREMIUM[nI].payHowPercent);
      paidTravel -= paidTravel % 60;
    }
    else
    {
      paidTravel = travelTime;
    }
  }

  return(paidTravel);
}

BOOL IsGarage(long NODESrecordID)
{
  int nI;

  for(nI = 0; nI < numGaragesInGarageList; nI++)
  {
    if(garageList[nI] == NODESrecordID)
      return(TRUE);
  }

  return(FALSE);
}

long GetGarageTravel(long startLocation, PROPOSEDRUNPIECESDef *pPiece,
      PREMIUMDef *pPremium, RUNTRIPDef *pRuntrip)
{
  GetConnectionTimeDef GCTData;
  float distance;
  long travelTime;
  long dwellTime;
  long startTime;
  long endTime;
  long equivalentTravelTime;

  if(NodesEquivalent(startLocation, pPiece->fromNODESrecordID, &equivalentTravelTime))
    travelTime = equivalentTravelTime;
  else if(pPremium->payTravelTime == WORKRULES_TIMEENTERED)
    travelTime = pPremium->time;
  else if(pPremium->payTravelTime == WORKRULES_ASINCONNECTIONS)
  {
    GCTData.fromROUTESrecordID = pRuntrip->from[0].RGRPROUTESrecordID;
    GCTData.fromSERVICESrecordID = pRuntrip->from[0].SGRPSERVICESrecordID;
    GCTData.fromPATTERNNAMESrecordID = pRuntrip->from[0].PATTERNNAMESrecordID;
    GCTData.fromNODESrecordID = startLocation;
    GCTData.toROUTESrecordID = pRuntrip->from[0].RGRPROUTESrecordID;
    GCTData.toSERVICESrecordID = pRuntrip->from[0].SGRPSERVICESrecordID;
    GCTData.toPATTERNNAMESrecordID = pRuntrip->from[0].PATTERNNAMESrecordID;
    GCTData.toNODESrecordID = pPiece->fromNODESrecordID;
    GCTData.timeOfDay = pPiece->fromTime;
    travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
  }
  else
  {
    if(bUseCISPlan)
    {
      if(pRuntrip->from[0].SGRPSERVICESrecordID == NO_RECORD)
      {
        travelTime = NO_TIME;
      }
      else
      {
        CISplanReliefConnect(&startTime, &endTime, &dwellTime, 0, startLocation,
              pPiece->fromNODESrecordID, pPiece->fromTime, FALSE,
              pRuntrip->from[0].SGRPSERVICESrecordID);
        if(startTime == -25 * 60 * 60)
        {
          travelTime = NO_TIME;
        }
        else
        {
          travelTime =  (endTime - startTime) + dwellTime;
        }
      }
    }
    else 
      travelTime = GetDynamicTravelTime(TRUE, startLocation,
            pPiece->fromNODESrecordID, pRuntrip->from[0].SGRPSERVICESrecordID,
            pPiece->fromTime, &dwellTime);
    if(travelTime == NO_TIME &&
         (pPremium->flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC))
    {
      if(travelTime != NO_TIME)
      {
        if(bUseDynamicTravels && !bUseCISPlan)
          numTravelInstructions--;
      }
      GCTData.fromROUTESrecordID = pRuntrip->from[0].RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = pRuntrip->from[0].SGRPSERVICESrecordID;
      GCTData.fromPATTERNNAMESrecordID = pRuntrip->from[0].PATTERNNAMESrecordID;
      GCTData.fromNODESrecordID = startLocation;
      GCTData.toROUTESrecordID = pRuntrip->from[0].RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = pRuntrip->from[0].SGRPSERVICESrecordID;
      GCTData.toPATTERNNAMESrecordID = pRuntrip->from[0].PATTERNNAMESrecordID;
      GCTData.toNODESrecordID = pPiece->fromNODESrecordID;
      GCTData.timeOfDay = pPiece->fromTime;
      travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
    }
  }

  return(travelTime);
}

//
//  FillRUNTRIP - Set up the RUNTRIP data structure
//
void FillRUNTRIP(RUNTRIPDef *pRT, PROPOSEDRUNDef *pPR, int serviceIndex, BOOL bCrewOnly)
{
  BLOCKSDef *pTRIPSChunk;
  BOOL bFound;
  int keyNumber;
  int nI;
  int nJ;
  int rcode2;
//
//  Common stuff
//
  for(nI = 0; nI < pPR->numPieces; nI++)
  {
//
//  From trip: Node data - Start node
//
    pRT->from[nI].flags = RCFLAGS_NONE;
    for(bFound = FALSE, nJ = 0; nJ < numGaragesInGarageList; nJ++)
    {
      if(pPR->piece[nI].fromNODESrecordID == garageList[nJ])
      {
        bFound = TRUE;
        break;
      }
    }
    if(bFound)
      pRT->from[nI].flags |= RCFLAGS_NODEGAR;
//
//  To trip: Node data - End node
//
    pRT->to[nI].flags = RCFLAGS_NONE;
    for(bFound = FALSE, nJ = 0; nJ < numGaragesInGarageList; nJ++)
    {
      if(pPR->piece[nI].toNODESrecordID == garageList[nJ])
      {
        bFound = TRUE;
        break;
      }
    }
    if(bFound)
      pRT->to[nI].flags |= RCFLAGS_NODEGAR;
  }
//
//  Crew only runs
//
  if(bCrewOnly)
  {
    for(nI = 0; nI < pPR->numPieces; nI++)
    {
      pRT->from[nI].assignedToNODESrecordID = NO_RECORD;
      pRT->from[nI].blockNumber = NO_RECORD;
      pRT->from[nI].blockPOGNODESrecordID = NO_RECORD;
      pRT->from[nI].blockPIGNODESrecordID = NO_RECORD;
      pRT->from[nI].RGRPROUTESrecordID = NO_RECORD;
      pRT->from[nI].SGRPSERVICESrecordID = NO_RECORD;
      pRT->from[nI].PATTERNNAMESrecordID = NO_RECORD;
      pRT->from[nI].BUSTYPESrecordID = NO_RECORD;
      pRT->to[nI].assignedToNODESrecordID = NO_RECORD;
      pRT->to[nI].blockNumber = NO_RECORD;
      pRT->to[nI].blockPOGNODESrecordID = NO_RECORD;
      pRT->to[nI].blockPIGNODESrecordID = NO_RECORD;
      pRT->to[nI].RGRPROUTESrecordID = NO_RECORD;
      pRT->to[nI].SGRPSERVICESrecordID = NO_RECORD;
      pRT->to[nI].PATTERNNAMESrecordID = NO_RECORD;
      pRT->to[nI].BUSTYPESrecordID = NO_RECORD;
    }
    return;
  }
//
//  Regular runs
//
  for(nI = 0; nI < pPR->numPieces; nI++)
  {
//
//  From trip
//
    TRIPSKey0.recordID = pPR->piece[nI].fromTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    keyNumber = (TRIPS.dropback.RGRPROUTESrecordID == NO_RECORD) ? 2 : 3;
    pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
//
//  From trip: Trip/block data
//
    pRT->from[nI].assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
    pRT->from[nI].blockNumber = pTRIPSChunk->blockNumber;
    pRT->from[nI].blockPOGNODESrecordID = pTRIPSChunk->POGNODESrecordID;
    pRT->from[nI].blockPIGNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
    pRT->from[nI].RGRPROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
    pRT->from[nI].SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
    pRT->from[nI].PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
//
//  From trip: Block POG
//
    if(pRT->from[nI].blockPOGNODESrecordID != NO_RECORD)
      pRT->from[nI].flags |= RCFLAGS_PULLOUT;
    else
    {
      TRIPSKey2.assignedToNODESrecordID = pRT->from[nI].assignedToNODESrecordID;
      TRIPSKey2.RGRPROUTESrecordID = pRT->from[nI].RGRPROUTESrecordID;
      TRIPSKey2.SGRPSERVICESrecordID = pRT->from[nI].SGRPSERVICESrecordID;
      TRIPSKey2.blockNumber = pRT->from[nI].blockNumber;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
      if(rcode2 != 0 || pTRIPSChunk->POGNODESrecordID == NO_RECORD)
        pRT->from[nI].blockPOGNODESrecordID = NO_RECORD;
      else
        pRT->from[nI].blockPOGNODESrecordID = pTRIPSChunk->POGNODESrecordID;
    }
//
//  From trip: Block PIG
//
    if(pRT->from[nI].blockPIGNODESrecordID != NO_RECORD)
      pRT->from[nI].flags |= RCFLAGS_PULLIN;
    {
      TRIPSKey2.assignedToNODESrecordID = pRT->from[nI].assignedToNODESrecordID;
      TRIPSKey2.RGRPROUTESrecordID = pRT->from[nI].RGRPROUTESrecordID;
      TRIPSKey2.SGRPSERVICESrecordID = pRT->from[nI].SGRPSERVICESrecordID;
      TRIPSKey2.blockNumber = pRT->from[nI].blockNumber + 1;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETLESSTHAN, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      if(rcode2 != 0 || pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
        pRT->from[nI].blockPIGNODESrecordID = NO_RECORD;
      else
        pRT->from[nI].blockPIGNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
    }
//
//  From trip: Bustype data
//
    if(TRIPS.BUSTYPESrecordID == NO_RECORD)
      pRT->from[nI].BUSTYPESrecordID = NO_RECORD;
    else
    {
      BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
      btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
      pRT->from[nI].BUSTYPESrecordID = BUSTYPES.recordID;
      if(BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE)
        pRT->from[nI].flags |= RCFLAGS_ACCESSIBLE;
    }
//
//  To trip
//
    TRIPSKey0.recordID = pPR->piece[nI].toTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  To trip: Trip/block data
//
    pRT->to[nI].assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
    pRT->to[nI].blockNumber = TRIPS.standard.blockNumber;
    pRT->to[nI].blockPOGNODESrecordID = TRIPS.standard.POGNODESrecordID;
    pRT->to[nI].blockPIGNODESrecordID = TRIPS.standard.PIGNODESrecordID;
    pRT->to[nI].RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
    pRT->to[nI].SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
    pRT->to[nI].PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
//
//  To trip: Block POG
//
    if(pRT->to[nI].blockPOGNODESrecordID != NO_RECORD)
      pRT->to[nI].flags |= RCFLAGS_PULLOUT;
    else
    {
      TRIPSKey2.assignedToNODESrecordID = pRT->to[nI].assignedToNODESrecordID;
      TRIPSKey2.RGRPROUTESrecordID = pRT->to[nI].RGRPROUTESrecordID;
      TRIPSKey2.SGRPSERVICESrecordID = pRT->to[nI].SGRPSERVICESrecordID;
      TRIPSKey2.blockNumber = pRT->to[nI].blockNumber;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      if(rcode2 != 0 || pTRIPSChunk->POGNODESrecordID == NO_RECORD)
        pRT->to[nI].blockPOGNODESrecordID = NO_RECORD;
      else
        pRT->to[nI].blockPOGNODESrecordID = pTRIPSChunk->POGNODESrecordID;
    }
//
//  To trip: Block PIG
//
    if(pRT->to[nI].blockPIGNODESrecordID != NO_RECORD)
      pRT->to[nI].flags |= RCFLAGS_PULLIN;
    {
      TRIPSKey2.assignedToNODESrecordID = pRT->to[nI].assignedToNODESrecordID;
      TRIPSKey2.RGRPROUTESrecordID = pRT->to[nI].RGRPROUTESrecordID;
      TRIPSKey2.SGRPSERVICESrecordID = pRT->to[nI].SGRPSERVICESrecordID;
      TRIPSKey2.blockNumber = pRT->to[nI].blockNumber + 1;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETLESSTHAN, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      if(rcode2 != 0 || TRIPS.standard.PIGNODESrecordID == NO_RECORD)
        pRT->to[nI].blockPIGNODESrecordID = NO_RECORD;
      else
        pRT->to[nI].blockPIGNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
    }
//
//  To trip: Bustype data
//
    if(TRIPS.BUSTYPESrecordID == NO_RECORD)
      pRT->to[nI].BUSTYPESrecordID = NO_RECORD;
    else
    {
      BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
      btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
      pRT->to[nI].BUSTYPESrecordID = BUSTYPES.recordID;
      if(BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE)
        pRT->to[nI].flags |= RCFLAGS_ACCESSIBLE;
    }
  }
}