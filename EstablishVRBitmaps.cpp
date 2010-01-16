//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "CutRuns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void EstablishVRBitmaps(CImageList *pImageList, VRCOLORSDef *pVRC)
{
  CBitmap bm;
  int nI, nJ;
 
  BYTE bits[15][16][4];  // = 960

//
//  VRCOLORS defaults are in the following order:
//
//      Red (Illegal)
//      Green (Cut)
//      Frozen (Blue)
//      Uncut (Yellow)
//
//  Colors
//
  COLORREF cRed = RGB(255, 0, 0);
  COLORREF cGreen = RGB(0, 255, 0);
  COLORREF cBlue = RGB(0, 0, 255);
  COLORREF cYellow = RGB(255, 255, 0);
   
  pVRC->illegal = GetPrivateProfileInt(userName, "VRColorIllegal", cRed, TMSINIFile);
  pVRC->cut = GetPrivateProfileInt(userName, "VRColorCut", cGreen, TMSINIFile);
  pVRC->frozen = GetPrivateProfileInt(userName, "VRColorFrozen", cBlue, TMSINIFile);
  pVRC->uncut = GetPrivateProfileInt(userName, "VRColorUncut", cYellow, TMSINIFile);

  bm.LoadBitmap(IDR_VRCOLORBUTTON);

  BITMAP BM;
  BM.bmBits = &bits;
  bm.GetBitmap(&BM);
  bm.GetBitmapBits(960, &bits);
//
//  Illegal runs
//
  BYTE bR = GetRValue(pVRC->illegal);
  BYTE bG = GetGValue(pVRC->illegal);
  BYTE bB = GetBValue(pVRC->illegal);
  for(nI = 2; nI < 13; nI++)
  {
    for(nJ = 2; nJ < 14; nJ++)
    {
      bits[nI][nJ][0] = bB;
      bits[nI][nJ][1] = bG;
      bits[nI][nJ][2] = bR;
      bits[nI][nJ][3] = 0;
    }
  }

  bm.SetBitmapBits(960, &bits);
  pImageList->Add(&bm, RGB(192, 192, 192));
//
//  Cut runs
//
  bR = GetRValue(pVRC->cut);
  bG = GetGValue(pVRC->cut);
  bB = GetBValue(pVRC->cut);
  for(nI = 2; nI < 13; nI++)
  {
    for(nJ = 2; nJ < 14; nJ++)
    {
      bits[nI][nJ][0] = bB;
      bits[nI][nJ][1] = bG;
      bits[nI][nJ][2] = bR;
      bits[nI][nJ][3] = 0;
    }
  }
  bm.SetBitmapBits(960, &bits);
  pImageList->Add(&bm, RGB(192, 192, 192));
//
//  Frozen runs
//
  bR = GetRValue(pVRC->frozen);
  bG = GetGValue(pVRC->frozen);
  bB = GetBValue(pVRC->frozen);
  for(nI = 2; nI < 13; nI++)
  {
    for(nJ = 2; nJ < 14; nJ++)
    {
      bits[nI][nJ][0] = bB;
      bits[nI][nJ][1] = bG;
      bits[nI][nJ][2] = bR;
      bits[nI][nJ][3] = 0;
    }
  }
  bm.SetBitmapBits(960, &bits);
  pImageList->Add(&bm, RGB(192, 192, 192));
//
//  Uncut runs
//
  bR = GetRValue(pVRC->uncut);
  bG = GetGValue(pVRC->uncut);
  bB = GetBValue(pVRC->uncut);
  for(nI = 2; nI < 13; nI++)
  {
    for(nJ = 2; nJ < 14; nJ++)
    {
      bits[nI][nJ][0] = bB;
      bits[nI][nJ][1] = bG;
      bits[nI][nJ][2] = bR;
      bits[nI][nJ][3] = 0;
    }
  }
  bm.SetBitmapBits(960, &bits);
  pImageList->Add(&bm, RGB(192, 192, 192));

  bm.DeleteObject();
}

//
//  EstablishBPBitmaps() - Set up the bitmaps for the Blocks Properties image list
//

void EstablishBPBitmaps(CImageList *pImageList, VRCOLORSDef *pVRC)
{
  CBitmap bm;
  COLORREF cThisColor;
  int nI, nJ, endJ;
  int leftSide, rightSide;
 
  BYTE bits[15][16][4];  // = 960
  BYTE bR;
  BYTE bG;
  BYTE bB;

//
//  VRCOLORS defaults are in the following order:
//
//      Red (Illegal)
//      Green (Cut)
//      Frozen (Blue)
//      Uncut (Yellow)
//
//  Colors
//
  COLORREF cRed = RGB(255, 0, 0);
  COLORREF cGreen = RGB(0, 255, 0);
  COLORREF cBlue = RGB(0, 0, 255);
  COLORREF cYellow = RGB(255, 255, 0);
   
  pVRC->illegal = GetPrivateProfileInt(userName, "VRColorIllegal", cRed, TMSINIFile);
  pVRC->cut = GetPrivateProfileInt(userName, "VRColorCut", cGreen, TMSINIFile);
  pVRC->frozen = GetPrivateProfileInt(userName, "VRColorFrozen", cBlue, TMSINIFile);
  pVRC->uncut = GetPrivateProfileInt(userName, "VRColorUncut", cYellow, TMSINIFile);

  bm.LoadBitmap(IDR_BPCOLORBUTTON);

  BITMAP BM;
  BM.bmBits = &bits;
  bm.GetBitmap(&BM);
  bm.GetBitmapBits(960, &bits);
//
//  Loop through the two sides
//
  for(leftSide = 0; leftSide < 5; leftSide++)
  {
    if(leftSide == BITMAP_ILLEGALRUN)
      cThisColor = pVRC->illegal;
    else if(leftSide == BITMAP_LEGALRUN)
      cThisColor = pVRC->cut;
    else if(leftSide == BITMAP_FROZENRUN)
      cThisColor = pVRC->frozen;
    else if(leftSide == BITMAP_UNCUT)
      cThisColor = pVRC->uncut;
    else
      cThisColor = RGB(255, 255, 255);
//
//  Get the RGB
//
    bR = GetRValue(cThisColor);
    bG = GetGValue(cThisColor);
    bB = GetBValue(cThisColor);
//
//  Put the color on the left side
//
    endJ = 11;
    for(nI = 2; nI <= 11; nI++)
    {
      for(nJ = 2; nJ <= endJ; nJ++)
      {
        bits[nI][nJ][0] = bB;
        bits[nI][nJ][1] = bG;
        bits[nI][nJ][2] = bR;
        bits[nI][nJ][3] = 0;
      }
      endJ--;
    }
//
//  Now do the right side
//
    for(rightSide = 0; rightSide < 5; rightSide++)
    {
      if(rightSide == BITMAP_ILLEGALRUN)
        cThisColor = pVRC->illegal;
      else if(rightSide == BITMAP_LEGALRUN)
        cThisColor = pVRC->cut;
      else if(rightSide == BITMAP_FROZENRUN)
        cThisColor = pVRC->frozen;
      else if(rightSide == BITMAP_UNCUT)
        cThisColor = pVRC->uncut;
      else
        cThisColor = RGB(255, 255, 255);
//
//  Get the RGB
//
      bR = GetRValue(cThisColor);
      bG = GetGValue(cThisColor);
      bB = GetBValue(cThisColor);
//
//  Put the color on the right side
//
      endJ = 13;
      for(nI = 3; nI <= 12; nI++)
      {
        for(nJ = 13; nJ >= endJ; nJ--)
        {
          bits[nI][nJ][0] = bB;
          bits[nI][nJ][1] = bG;
          bits[nI][nJ][2] = bR;
          bits[nI][nJ][3] = 0;
        }
        endJ--;
      }
//
//  Add the bitmap to the image list
//
      bm.SetBitmapBits(960, &bits);
      pImageList->Add(&bm, RGB(192, 192, 192));
    }
  }

  bm.DeleteObject();
}