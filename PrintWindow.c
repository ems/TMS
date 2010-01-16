//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

HPALETTE GetSystemPalette()
{
  HDC hDC;
  HPALETTE hPal;
  LPLOGPALETTE lpLogPal; 

//
//  Get a DC for the desktop
//
  hDC = GetDC(NULL); 
//
//  Check to see if we are running in a palette-based video mode.
//
  if(!(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE))
  {
    ReleaseDC(NULL, hDC);
    return NULL;
  } 
//
//  Allocate memory for the palette
//
  lpLogPal = (LPLOGPALETTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY)); 
  if(lpLogPal == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return NULL;
  }
//
//  Initialize
//
  lpLogPal->palVersion = 0x300;
  lpLogPal->palNumEntries = 256; 
//
//  Copy the current system palette into the logical palette
//
  GetSystemPaletteEntries(hDC, 0, 256, (LPPALETTEENTRY)(lpLogPal->palPalEntry)); 
//
//  Create the palette
//
  hPal = CreatePalette(lpLogPal); 
//
//  Clean up
//
  TMSHeapFree(lpLogPal);
  ReleaseDC(NULL, hDC); 
    
  return hPal; 
} 

//
//  Create a 24-bit-per-pixel surface
//
HBITMAP Create24BPPDIBSection(HDC hDC, int iWidth, int iHeight)
{
  BITMAPINFO bmi;
  HBITMAP hbm;
  LPBYTE pBits; 

//
//  Initialize to 0
//
  ZeroMemory(&bmi, sizeof(bmi)); 
//
// Initialize the header.
//
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = iWidth;
  bmi.bmiHeader.biHeight = iHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB; 
//
//  Create the surface
//
  hbm = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0); 
  
  return(hbm); 
} 

//
//  Print the entire contents (including the non-client
//  area) of the specified window to the default printer
//
BOOL PrintWindowToDC(HWND hWnd, char *pszTitle)
{
  DIBSECTION ds;
  HPALETTE   hPal; 
  PRINTDLG   pdlg; 
  DEVMODE    *pDM;
  HBITMAP hbm;
  DOCINFO di;
  float fScaleX;
  float fScaleY;
  float fLogPelsX1; 
  float fLogPelsY1; 
  float fLogPelsX2; 
  float fLogPelsY2; 
  RECT  rc;
  HDC   hdcPrinter;
  HDC   hdcMemory;
  HDC   hdcWindow;
  int   cWidthPels;
  int   cHeightPels;
  int   xLeft;
  int   yTop;
  int   iWidth;
  int   iHeight;
//
//  Do we have a valid window?
//
  if(!IsWindow(hWnd))
    return FALSE; 
//
//  Get a HDC for the default printer and force it to landscape
//
  memset(&pdlg, 0x00, sizeof(PRINTDLG));
  pdlg.lStructSize = sizeof(PRINTDLG);
  pdlg.Flags = PD_RETURNDC | PD_RETURNDEFAULT;
  if(!PrintDlg(&pdlg))
    return FALSE;
  hdcPrinter = pdlg.hDC;
  if(!hdcPrinter)
    return FALSE; 
  pDM = (DEVMODE *)GlobalLock(pdlg.hDevMode);
  pDM->dmFields = DM_ORIENTATION;
  pDM->dmOrientation = DMORIENT_LANDSCAPE;
  ResetDC(hdcPrinter, pDM);
  GlobalUnlock(pdlg.hDevMode);
//
//  Get the HDC for the entire window
//
  hdcWindow = GetWindowDC(hWnd); 
//
//  Get the rectangle bounding the window
//
  GetWindowRect(hWnd, &rc); 
//
//  Adjust coordinates to client area
//
  OffsetRect(&rc, -rc.left, -rc.top); 
//
//  Get the resolution of the printer device
//
  iWidth  = GetDeviceCaps(hdcPrinter, HORZRES);
  iHeight = GetDeviceCaps(hdcPrinter, VERTRES); 
//
//  Create the intermediate drawing surface at window resolution
//
  hbm = Create24BPPDIBSection(hdcWindow, rc.right, rc.bottom);
  if(!hbm)
  {
    DeleteDC(hdcPrinter);
    ReleaseDC(hWnd, hdcWindow);
    return FALSE;
  } 
//
//  Prepare the surface for drawing
//
  hdcMemory = CreateCompatibleDC(hdcWindow);
  SelectObject(hdcMemory, hbm); 
//
//  Get the current system palette
//
  hPal = GetSystemPalette(); 
//
//  If a palette was returned
//
  if(hPal)  
  {
    SelectPalette(hdcWindow, hPal, FALSE);  // Apply the palette to the source DC
    RealizePalette(hdcWindow); 
    SelectPalette(hdcMemory, hPal, FALSE);  // Apply the palette to the destination DC.
    RealizePalette(hdcMemory);
  } 
//
//  Copy the window contents to the memory surface
//
  BitBlt(hdcMemory, 0, 0, rc.right, rc.bottom, hdcWindow, 0, 0, SRCCOPY); 
//
//  Prepare the DOCINFO
//
  ZeroMemory(&di, sizeof(di));
  di.cbSize = sizeof(di);
  di.lpszDocName = pszTitle; 
//
//  Initialize the print job
//
  if(StartDoc(hdcPrinter, &di) > 0)
  { 
//
//  Prepare to send a page
//
    if(StartPage(hdcPrinter) > 0)
    { 
//
//  Retrieve the information describing the surface
//
      GetObject(hbm, sizeof(DIBSECTION), &ds); 
//
//  Retrieve the number of pixels-per-logical-inch in the 
//  horizontal and vertical directions for the display upon which 
//  the bitmap was created. 
//
      fLogPelsX1 = (float) GetDeviceCaps(hdcWindow, LOGPIXELSX); 
      fLogPelsY1 = (float) GetDeviceCaps(hdcWindow, LOGPIXELSY); 
 
//
// Retrieve the number of pixels-per-logical-inch in the 
// horizontal and vertical directions for the printer upon which 
// the bitmap will be printed. 
// 
      fLogPelsX2 = (float) GetDeviceCaps(pdlg.hDC, LOGPIXELSX); 
      fLogPelsY2 = (float) GetDeviceCaps(pdlg.hDC, LOGPIXELSY); 
// 
// Determine the scaling factors required to print the bitmap and 
// retain its original proportions. 
// 
      if(fLogPelsX1 > fLogPelsX2) 
        fScaleX = (fLogPelsX1 / fLogPelsX2); 
      else
        fScaleX = (fLogPelsX2 / fLogPelsX1); 
 
      if(fLogPelsY1 > fLogPelsY2) 
        fScaleY = (fLogPelsY1 / fLogPelsY2); 
      else
        fScaleY = (fLogPelsY2 / fLogPelsY1); 
// 
//  Compute the coordinates of the upper left corner of the bitmap. 
// 
      cWidthPels = GetDeviceCaps(pdlg.hDC, HORZRES); 
      xLeft = ((cWidthPels / 2) - ((int) (((float) rc.right)  * fScaleX)) / 2); 
      cHeightPels = GetDeviceCaps(pdlg.hDC, VERTRES); 
      yTop = ((cHeightPels / 2) - ((int) (((float) rc.bottom)  * fScaleY)) / 2); 
//
//  And fit it to the paper if necessary
//
      StretchDIBits(hdcPrinter,
            xLeft, yTop, (int)((float)rc.right *  fScaleX), (int)((float)rc.bottom * fScaleY),
            0, 0, rc.right, rc.bottom,
            ds.dsBm.bmBits, (LPBITMAPINFO)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY); 
//
//  Let the driver know the page is done
//
      EndPage(hdcPrinter);
    } 
//
//  Let the driver know the document is done
//
    EndDoc(hdcPrinter);
  } 
//
//  Clean up the created objects
//
  DeleteDC(hdcPrinter);
  DeleteDC(hdcMemory);
  ReleaseDC(hWnd, hdcWindow);
  DeleteObject(hbm);
  if(hPal)
    DeleteObject(hPal);

  return TRUE;
}
