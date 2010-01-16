// ILGRouteDirection.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGRouteDirection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGRouteDirection

IMPLEMENT_DYNAMIC(CILGRouteDirection, CRecordset)

CILGRouteDirection::CILGRouteDirection(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGRouteDirection)
	m_RouteDirectionAbbr = _T("");
	m_RouteDirectionName = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGRouteDirection::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGRouteDirection::GetDefaultSQL()
{
	return _T("[RouteDirection]");
}

void CILGRouteDirection::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGRouteDirection)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[RouteDirectionAbbr]"), m_RouteDirectionAbbr);
	RFX_Text(pFX, _T("[RouteDirectionName]"), m_RouteDirectionName);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGRouteDirection diagnostics

#ifdef _DEBUG
void CILGRouteDirection::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGRouteDirection::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
