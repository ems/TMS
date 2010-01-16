// TextViolations.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextViolations.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextViolations

IMPLEMENT_DYNAMIC(CTextViolations, CRecordset)

CTextViolations::CTextViolations(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextViolations)
	m_ViolationIndex = 0;
	m_ViolationCategoryIndex = 0;
	m_ViolationNumber = 0;
	m_ViolationDuration = 0;
	m_ViolationAbsenceRelated = _T("");
	m_ViolationText = _T("");
	m_ViolationActionsIndexes = _T("");
	m_nFields = 7;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextViolations::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextViolations::GetDefaultSQL()
{
	return _T("[Violations]");
}

void CTextViolations::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextViolations)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ViolationIndex]"), m_ViolationIndex);
	RFX_Long(pFX, _T("[ViolationCategoryIndex]"), m_ViolationCategoryIndex);
	RFX_Long(pFX, _T("[ViolationNumber]"), m_ViolationNumber);
	RFX_Long(pFX, _T("[ViolationDuration]"), m_ViolationDuration);
	RFX_Text(pFX, _T("[ViolationAbsenceRelated]"), m_ViolationAbsenceRelated);
	RFX_Text(pFX, _T("[ViolationText]"), m_ViolationText);
	RFX_Text(pFX, _T("[ViolationActionsIndexes]"), m_ViolationActionsIndexes);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextViolations diagnostics

#ifdef _DEBUG
void CTextViolations::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextViolations::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
