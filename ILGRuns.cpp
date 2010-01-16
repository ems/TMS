// ILGRuns.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGRuns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGRuns

IMPLEMENT_DYNAMIC(CILGRuns, CRecordset)

CILGRuns::CILGRuns(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGRuns)
	m_RunNum = 0;
	m_BlockNum = 0;
	m_ServiceAbbr = _T("");
	m_PieceNum = 0;
	m_StartTimeInMinutes = 0;
	m_EndTimeInMinutes = 0;
	m_nFields = 8;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGRuns::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGRuns::GetDefaultSQL()
{
	return _T("[Runs]");
}

void CILGRuns::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGRuns)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[RunNum]"), m_RunNum);
	RFX_Long(pFX, _T("[BlockNum]"), m_BlockNum);
	RFX_Text(pFX, _T("[ServiceAbbr]"), m_ServiceAbbr);
	RFX_Long(pFX, _T("[PieceNum]"), m_PieceNum);
	RFX_Long(pFX, _T("[StartTimeInMinutes]"), m_StartTimeInMinutes);
	RFX_Long(pFX, _T("[EndTimeInMinutes]"), m_EndTimeInMinutes);
	RFX_Date(pFX, _T("[StartTimeInTime]"), m_StartTimeInTime);
	RFX_Date(pFX, _T("[EndTimeInTime]"), m_EndTimeInTime);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGRuns diagnostics

#ifdef _DEBUG
void CILGRuns::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGRuns::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
