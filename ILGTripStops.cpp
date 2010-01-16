// ILGTripStops.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGTripStops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGTripStops

IMPLEMENT_DYNAMIC(CILGTripStops, CRecordset)

CILGTripStops::CILGTripStops(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGTripStops)
	m_TripID = 0;
	m_StopNum = 0;
	m_CrossingTimeInMinutes = 0;
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGTripStops::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGTripStops::GetDefaultSQL()
{
	return _T("[TripStops]");
}

void CILGTripStops::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGTripStops)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[TripID]"), m_TripID);
	RFX_Long(pFX, _T("[StopNum]"), m_StopNum);
	RFX_Date(pFX, _T("[CrossingTimeInTime]"), m_CrossingTimeInTime);
	RFX_Long(pFX, _T("[CrossingTimeInMinutes]"), m_CrossingTimeInMinutes);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGTripStops diagnostics

#ifdef _DEBUG
void CILGTripStops::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGTripStops::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
