// TextClassificationCodes.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextClassificationCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextClassificationCodes

IMPLEMENT_DYNAMIC(CTextClassificationCodes, CRecordset)

CTextClassificationCodes::CTextClassificationCodes(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextClassificationCodes)
	m_ClassificationNumber = 0;
	m_ClassificationText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextClassificationCodes::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextClassificationCodes::GetDefaultSQL()
{
	return _T("[Classification Codes]");
}

void CTextClassificationCodes::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextClassificationCodes)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ClassificationNumber]"), m_ClassificationNumber);
	RFX_Text(pFX, _T("[ClassificationText]"), m_ClassificationText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextClassificationCodes diagnostics

#ifdef _DEBUG
void CTextClassificationCodes::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextClassificationCodes::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
