// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CF1ObjPos wrapper class

class CF1ObjPos : public COleDispatchDriver
{
public:
	CF1ObjPos() {}		// Calls COleDispatchDriver default constructor
	CF1ObjPos(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CF1ObjPos(const CF1ObjPos& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	float GetStartRow();
	float GetStartCol();
	float GetEndRow();
	float GetEndCol();
	float GetRows();
	float GetCols();
};