// TextAbsenceReasons.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextAbsenceReasons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextAbsenceReasons

IMPLEMENT_DYNAMIC(CTextAbsenceReasons, CRecordset)

CTextAbsenceReasons::CTextAbsenceReasons(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextAbsenceReasons)
	m_ReasonNumber = 0;
	m_ReasonPointer = 0;
	m_ReasonCountsTowardUnpaidTime = _T("");
	m_ReasonText = _T("");
	m_ReasonPayrollCode = _T("");
	m_nFields = 5;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextAbsenceReasons::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextAbsenceReasons::GetDefaultSQL()
{
	return _T("[Absence Reasons]");
}

void CTextAbsenceReasons::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextAbsenceReasons)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ReasonNumber]"), m_ReasonNumber);
	RFX_Long(pFX, _T("[ReasonPointer]"), m_ReasonPointer);
	RFX_Text(pFX, _T("[ReasonCountsTowardUnpaidTime]"), m_ReasonCountsTowardUnpaidTime);
	RFX_Text(pFX, _T("[ReasonText]"), m_ReasonText);
	RFX_Text(pFX, _T("[ReasonPayrollCode]"), m_ReasonPayrollCode);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextAbsenceReasons diagnostics

#ifdef _DEBUG
void CTextAbsenceReasons::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextAbsenceReasons::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
