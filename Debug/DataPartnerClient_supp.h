//	DataPartnerClient_supp.h : supplement header file for DataPartnerClient.dll
//	Author: DLL to Lib version 3.00
//	Date: Wednesday, May 06, 2009
//	Description: The declaration of the DataPartnerClient.dll's entry-point function.
//	Prototype: BOOL WINAPI xxx_DllMain(HINSTANCE hinstance, DWORD fdwReason, LPVOID lpvReserved);
//	Parameters: 
//		hinstance
//		  Handle to current instance of the application. Use AfxGetInstanceHandle()
//		  to get the instance handle if your project has MFC support.
//		fdwReason
//		  Specifies a flag indicating why the entry-point function is being called.
//		lpvReserved 
//		  Specifies further aspects of DLL initialization and cleanup. Should always
//		  be set to NULL;
//	Comment: Please see the help document for detail information about the entry-point 
//		 function
//	Homepage: http://www.binary-soft.com
//	Technical Support: support@binary-soft.com
/////////////////////////////////////////////////////////////////////

#if !defined(D2L_DATAPARTNERCLIENT_SUPP_H__735C5F28_1E4C_1218_3BC7_376148683030__INCLUDED_)
#define D2L_DATAPARTNERCLIENT_SUPP_H__735C5F28_1E4C_1218_3BC7_376148683030__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __cplusplus
extern "C" {
#endif


#include <windows.h>

/* This is DataPartnerClient.dll's entry-point function. You should call it to do necessary
 initialization and finalization. */

BOOL WINAPI DATAPARTNERCLIENT_DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);


#ifdef __cplusplus
}
#endif

#endif // !defined(D2L_DATAPARTNERCLIENT_SUPP_H__735C5F28_1E4C_1218_3BC7_376148683030__INCLUDED_)