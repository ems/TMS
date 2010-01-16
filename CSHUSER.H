/***************************************************************************
 *
 * File: CSHUSER.H
 *
 * Descripton: ContextHelp Initialization Definition File
 *
 * Notes:
 *
 ****************************************************************************
 *
 * Copyright (c) 1995, 1996 - Blue Sky Software Corp. - All Rights Reserved
 *
 ***************************************************************************/

#ifndef CSHUSER_H
#define CSHUSER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SC__
 #define WINAPI 
#endif

//**************************************************************************

// User Application Initialization Functions

BOOL WINAPI CSHInitialize (LPSTR a_pHelpFile, BOOL bContextMenu);
BOOL WINAPI CSHRegisterDialog (HWND a_hDialog, long a_UniqueID);
typedef BOOL (WINAPI *fnCSHREGISTERDIALOG) (HWND a_hDialog, long a_UniqueID);

// Special Advanced Functionality Routines									  
BOOL WINAPI CSHContextMenu(HWND hDlg, HWND hWndPoint, LPPOINT pPt);
BOOL WINAPI CSHContextHelp(HWND hDlg, HWND hCtrl, int iContextType);

#ifdef __cplusplus
 #define CSHHelpEnabled(_pCshOn) ::SendMessage(::GetFocus(), (UINT)RegisterWindowMessage("WM_CSHHASHELP"), (WPARAM)(0), (LPARAM)(LPINT)_pCshOn)
#else
 #define CSHHelpEnabled(_pCshOn) SendMessage(GetFocus(), (UINT)RegisterWindowMessage("WM_CSHHASHELP"), (WPARAM)(0), (LPARAM)(LPINT)_pCshOn)
#endif

#define CSH_DLLNAME             "CSH.DLL"
#define CSH_INITFUNCNAME        "CSHInitialize"
#define CSH_REGISTERFUNCNAME    "CSHRegisterDialog"

//**************************************************************************

#ifdef __cplusplus
}
#endif

#endif // CSHUSER_H

