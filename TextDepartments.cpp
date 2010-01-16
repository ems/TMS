// TextDepartments.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextDepartments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextDepartments

IMPLEMENT_DYNAMIC(CTextDepartments, CRecordset)

CTextDepartments::CTextDepartments(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextDepartments)
	m_DepartmentNumber = 0;
	m_DepartmentText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextDepartments::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextDepartments::GetDefaultSQL()
{
	return _T("[Departments]");
}

void CTextDepartments::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextDepartments)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[DepartmentNumber]"), m_DepartmentNumber);
	RFX_Text(pFX, _T("[DepartmentText]"), m_DepartmentText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextDepartments diagnostics

#ifdef _DEBUG
void CTextDepartments::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextDepartments::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
