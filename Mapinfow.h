// Machine generated IDispatch driver class(es) created with MFCDSPWZ tool.
/////////////////////////////////////////////////////////////////////////////
// DMapInfo wrapper class

class DMapInfo : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type
	// method 'AddRef' not emitted because of invalid return type
	// method 'Release' not emitted because of invalid return type
	LPDISPATCH GetApplication();
	LPDISPATCH GetParent();
	CString GetName();
	CString GetFullName();
	CString GetVersion();
	BOOL GetVisible();
	void SetVisible(BOOL bNewValue);
	long GetLastErrorCode();
	void SetLastErrorCode(long nNewValue);
	CString GetLastErrorMessage();
	LPDISPATCH GetMBApplications();
	void Do(LPCTSTR command);
	CString Eval(LPCTSTR expression);
	void RunCommand(LPCTSTR command);
	void RunMenuCommand(short id);
	LPUNKNOWN DataObject(long windowID);
	void SetCallback(LPDISPATCH callbackobject);
};
/////////////////////////////////////////////////////////////////////////////
// DMBApplications wrapper class

class DMBApplications : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type
	// method 'AddRef' not emitted because of invalid return type
	// method 'Release' not emitted because of invalid return type
	LPDISPATCH GetApplication();
	LPDISPATCH GetParent();
	long GetCount();
	LPDISPATCH Item(const VARIANT& index);
	LPUNKNOWN Get_NewEnum();
};
/////////////////////////////////////////////////////////////////////////////
// DMapBasicApplication wrapper class

class DMapBasicApplication : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type
	// method 'AddRef' not emitted because of invalid return type
	// method 'Release' not emitted because of invalid return type
	LPDISPATCH GetApplication();
	LPDISPATCH GetParent();
	CString GetName();
	CString GetFullName();
	LPDISPATCH GetMBGlobals();
	void Do(LPCTSTR command);
	CString Eval(LPCTSTR expression);
};
/////////////////////////////////////////////////////////////////////////////
// DMBGlobals wrapper class

class DMBGlobals : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type
	// method 'AddRef' not emitted because of invalid return type
	// method 'Release' not emitted because of invalid return type
	LPDISPATCH GetApplication();
	LPDISPATCH GetParent();
	long GetCount();
	LPDISPATCH Item(const VARIANT& index);
	LPUNKNOWN Get_NewEnum();
};
/////////////////////////////////////////////////////////////////////////////
// DMBGlobal wrapper class

class DMBGlobal : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type
	// method 'AddRef' not emitted because of invalid return type
	// method 'Release' not emitted because of invalid return type
	LPDISPATCH GetApplication();
	LPDISPATCH GetParent();
	CString GetName();
	CString GetType();
	CString GetValue();
	void SetValue(LPCTSTR lpszNewValue);
};

// ADDED FOR INTEGRATED MAPPING SUPPORT
EXTERN DMapInfo mapinfo;
#include "mapbasic.h"
// END OF ADDITION FOR INTEGRATED MAPPING SUPPORT
