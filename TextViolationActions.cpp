// TextViolationActions.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextViolationActions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextViolationActions

IMPLEMENT_DYNAMIC(CTextViolationActions, CRecordset)

CTextViolationActions::CTextViolationActions(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextViolationActions)
	m_ActionNumber = 0;
	m_ActionText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextViolationActions::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextViolationActions::GetDefaultSQL()
{
	return _T("[Violation Actions]");
}

void CTextViolationActions::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextViolationActions)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ActionNumber]"), m_ActionNumber);
	RFX_Text(pFX, _T("[ActionText]"), m_ActionText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextViolationActions diagnostics

#ifdef _DEBUG
void CTextViolationActions::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextViolationActions::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
