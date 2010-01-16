// TextPayrollLayout.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextPayrollLayout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextPayrollLayout

IMPLEMENT_DYNAMIC(CTextPayrollLayout, CRecordset)

CTextPayrollLayout::CTextPayrollLayout(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextPayrollLayout)
	m_ID = 0;
	m_Field_Name = _T("");
	m_Position = 0;
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = snapshot;
}


CString CTextPayrollLayout::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextPayrollLayout::GetDefaultSQL()
{
	return _T("[Payroll Layout]");
}

void CTextPayrollLayout::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextPayrollLayout)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ID]"), m_ID);
	RFX_Text(pFX, _T("[Field Name]"), m_Field_Name);
	RFX_Long(pFX, _T("[Position]"), m_Position);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextPayrollLayout diagnostics

#ifdef _DEBUG
void CTextPayrollLayout::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextPayrollLayout::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
