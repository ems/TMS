// ILGTimeTable.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGTimeTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGTimeTable

IMPLEMENT_DYNAMIC(CILGTimeTable, CRecordset)

CILGTimeTable::CILGTimeTable(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGTimeTable)
	m_TimeTableVersionName = _T("");
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGTimeTable::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGTimeTable::GetDefaultSQL()
{
	return _T("[TimeTable]");
}

void CILGTimeTable::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGTimeTable)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[TimeTableVersionName]"), m_TimeTableVersionName);
	RFX_Date(pFX, _T("[ActivationDate]"), m_ActivationDate);
	RFX_Date(pFX, _T("[DeactivationDate]"), m_DeactivationDate);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGTimeTable diagnostics

#ifdef _DEBUG
void CILGTimeTable::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGTimeTable::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
