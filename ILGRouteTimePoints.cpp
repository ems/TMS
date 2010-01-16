// ILGRouteTimePoints.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGRouteTimePoints.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGRouteTimePoints

IMPLEMENT_DYNAMIC(CILGRouteTimePoints, CRecordset)

CILGRouteTimePoints::CILGRouteTimePoints(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGRouteTimePoints)
	m_RouteAbbr = _T("");
	m_RouteDirectionAbbr = _T("");
	m_TimePointAbbr = _T("");
	m_RouteSequence = 0;
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGRouteTimePoints::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGRouteTimePoints::GetDefaultSQL()
{
	return _T("[RouteTimePoints]");
}

void CILGRouteTimePoints::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGRouteTimePoints)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[RouteAbbr]"), m_RouteAbbr);
	RFX_Text(pFX, _T("[RouteDirectionAbbr]"), m_RouteDirectionAbbr);
	RFX_Text(pFX, _T("[TimePointAbbr]"), m_TimePointAbbr);
	RFX_Long(pFX, _T("[RouteSequence]"), m_RouteSequence);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGRouteTimePoints diagnostics

#ifdef _DEBUG
void CILGRouteTimePoints::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGRouteTimePoints::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
