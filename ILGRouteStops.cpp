// ILGRouteStops.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGRouteStops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGRouteStops

IMPLEMENT_DYNAMIC(CILGRouteStops, CRecordset)

CILGRouteStops::CILGRouteStops(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGRouteStops)
	m_RouteAbbr = _T("");
	m_RouteDirectionAbbr = _T("");
	m_PatternAbbr = _T("");
	m_StopNumber = 0;
	m_StopAbbr = _T("");
	m_TimePointAbbr = _T("");
	m_DistanceFromLast = _T("");
	m_CompassDirection = 0;
	m_nFields = 8;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGRouteStops::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGRouteStops::GetDefaultSQL()
{
	return _T("[RouteStops]");
}

void CILGRouteStops::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGRouteStops)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[RouteAbbr]"), m_RouteAbbr);
	RFX_Text(pFX, _T("[RouteDirectionAbbr]"), m_RouteDirectionAbbr);
	RFX_Text(pFX, _T("[PatternAbbr]"), m_PatternAbbr);
	RFX_Long(pFX, _T("[StopNumber]"), m_StopNumber);
	RFX_Text(pFX, _T("[StopAbbr]"), m_StopAbbr);
	RFX_Text(pFX, _T("[TimePointAbbr]"), m_TimePointAbbr);
	RFX_Text(pFX, _T("[DistanceFromLast]"), m_DistanceFromLast);
	RFX_Long(pFX, _T("[CompassDirection]"), m_CompassDirection);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGRouteStops diagnostics

#ifdef _DEBUG
void CILGRouteStops::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGRouteStops::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
