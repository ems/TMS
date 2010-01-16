// ILGRoutes.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGRoutes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGRoutes

IMPLEMENT_DYNAMIC(CILGRoutes, CRecordset)

CILGRoutes::CILGRoutes(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGRoutes)
	m_RouteAbbr = _T("");
	m_RouteName = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGRoutes::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGRoutes::GetDefaultSQL()
{
	return _T("[Routes]");
}

void CILGRoutes::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGRoutes)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[RouteAbbr]"), m_RouteAbbr);
	RFX_Text(pFX, _T("[RouteName]"), m_RouteName);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGRoutes diagnostics

#ifdef _DEBUG
void CILGRoutes::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGRoutes::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
