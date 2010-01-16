// TextRoadSupervisors.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextRoadSupervisors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextRoadSupervisors

IMPLEMENT_DYNAMIC(CTextRoadSupervisors, CRecordset)

CTextRoadSupervisors::CTextRoadSupervisors(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextRoadSupervisors)
	m_RoadSupNumber = 0;
	m_RoadSupName = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextRoadSupervisors::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextRoadSupervisors::GetDefaultSQL()
{
	return _T("[Road Supervisors]");
}

void CTextRoadSupervisors::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextRoadSupervisors)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[RoadSupNumber]"), m_RoadSupNumber);
	RFX_Text(pFX, _T("[RoadSupName]"), m_RoadSupName);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextRoadSupervisors diagnostics

#ifdef _DEBUG
void CTextRoadSupervisors::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextRoadSupervisors::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
