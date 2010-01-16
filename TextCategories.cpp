// TextCategories.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextCategories.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextCategories

IMPLEMENT_DYNAMIC(CTextCategories, CRecordset)

CTextCategories::CTextCategories(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextCategories)
	m_CategoryNumber = 0;
	m_CategoryText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextCategories::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextCategories::GetDefaultSQL()
{
	return _T("[Categories]");
}

void CTextCategories::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextCategories)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[CategoryNumber]"), m_CategoryNumber);
	RFX_Text(pFX, _T("[CategoryText]"), m_CategoryText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextCategories diagnostics

#ifdef _DEBUG
void CTextCategories::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextCategories::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
