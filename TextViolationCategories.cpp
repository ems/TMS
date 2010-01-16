// TextViolationCategories.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextViolationCategories.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextViolationCategories

IMPLEMENT_DYNAMIC(CTextViolationCategories, CRecordset)

CTextViolationCategories::CTextViolationCategories(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextViolationCategories)
	m_CategoryNumber = 0;
	m_CategoryText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextViolationCategories::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextViolationCategories::GetDefaultSQL()
{
	return _T("[Violation Categories]");
}

void CTextViolationCategories::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextViolationCategories)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[CategoryNumber]"), m_CategoryNumber);
	RFX_Text(pFX, _T("[CategoryText]"), m_CategoryText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextViolationCategories diagnostics

#ifdef _DEBUG
void CTextViolationCategories::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextViolationCategories::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
