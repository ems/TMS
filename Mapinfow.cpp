// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
} // extern "C"

#include "mapinfow.h"


/////////////////////////////////////////////////////////////////////////////
// DMapInfo properties

/////////////////////////////////////////////////////////////////////////////
// DMapInfo operations

LPDISPATCH DMapInfo::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x60010000, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMapInfo::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x60010001, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

CString DMapInfo::GetName()
{
	CString result;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString DMapInfo::GetFullName()
{
	CString result;
	InvokeHelper(0x60010003, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString DMapInfo::GetVersion()
{
	CString result;
	InvokeHelper(0x60010004, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

BOOL DMapInfo::GetVisible()
{
	BOOL result;
	InvokeHelper(0x60010005, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void DMapInfo::SetVisible(BOOL bNewValue)
{
	static BYTE BASED_CODE parms[] =
		VTS_BOOL;
	InvokeHelper(0x60010005, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long DMapInfo::GetLastErrorCode()
{
	long result;
	InvokeHelper(0x60010007, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void DMapInfo::SetLastErrorCode(long nNewValue)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x60010007, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CString DMapInfo::GetLastErrorMessage()
{
	CString result;
	InvokeHelper(0x60010009, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMapInfo::GetMBApplications()
{
	LPDISPATCH result;
	InvokeHelper(0x6001000a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

void DMapInfo::Do(LPCTSTR command)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x6001000b, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 command);
}

CString DMapInfo::Eval(LPCTSTR expression)
{
	CString result;
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x6001000c, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		expression);
	return result;
}

void DMapInfo::RunCommand(LPCTSTR command)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x6001000d, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 command);
}

void DMapInfo::RunMenuCommand(short id)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x6001000e, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 id);
}

LPUNKNOWN DMapInfo::DataObject(long windowID)
{
	LPUNKNOWN result;
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x6001000f, DISPATCH_METHOD, VT_UNKNOWN, (void*)&result, parms,
		windowID);
	return result;
}

void DMapInfo::SetCallback(LPDISPATCH callbackobject)
{
	static BYTE BASED_CODE parms[] =
		VTS_DISPATCH;
	InvokeHelper(0x60010010, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 callbackobject);
}


/////////////////////////////////////////////////////////////////////////////
// DMBApplications properties

/////////////////////////////////////////////////////////////////////////////
// DMBApplications operations

LPDISPATCH DMBApplications::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x60010000, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMBApplications::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x60010001, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

long DMBApplications::GetCount()
{
	long result;
	InvokeHelper(0x60010002, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMBApplications::Item(const VARIANT& index)
{
	LPDISPATCH result;
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x0, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&index);
	return result;
}

LPUNKNOWN DMBApplications::Get_NewEnum()
{
	LPUNKNOWN result;
	InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&result, NULL);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// DMapBasicApplication properties

/////////////////////////////////////////////////////////////////////////////
// DMapBasicApplication operations

LPDISPATCH DMapBasicApplication::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x60010000, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMapBasicApplication::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x60010001, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

CString DMapBasicApplication::GetName()
{
	CString result;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString DMapBasicApplication::GetFullName()
{
	CString result;
	InvokeHelper(0x60010003, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMapBasicApplication::GetMBGlobals()
{
	LPDISPATCH result;
	InvokeHelper(0x60010004, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

void DMapBasicApplication::Do(LPCTSTR command)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x60010005, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 command);
}

CString DMapBasicApplication::Eval(LPCTSTR expression)
{
	CString result;
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x60010006, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		expression);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// DMBGlobals properties

/////////////////////////////////////////////////////////////////////////////
// DMBGlobals operations

LPDISPATCH DMBGlobals::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x60010000, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMBGlobals::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x60010001, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

long DMBGlobals::GetCount()
{
	long result;
	InvokeHelper(0x60010002, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMBGlobals::Item(const VARIANT& index)
{
	LPDISPATCH result;
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x0, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&index);
	return result;
}

LPUNKNOWN DMBGlobals::Get_NewEnum()
{
	LPUNKNOWN result;
	InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&result, NULL);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// DMBGlobal properties

/////////////////////////////////////////////////////////////////////////////
// DMBGlobal operations

LPDISPATCH DMBGlobal::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x60010000, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

LPDISPATCH DMBGlobal::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x60010001, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

CString DMBGlobal::GetName()
{
	CString result;
	InvokeHelper(0x60010002, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString DMBGlobal::GetType()
{
	CString result;
	InvokeHelper(0x60010003, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString DMBGlobal::GetValue()
{
	CString result;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

void DMBGlobal::SetValue(LPCTSTR lpszNewValue)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x0, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 lpszNewValue);
}
