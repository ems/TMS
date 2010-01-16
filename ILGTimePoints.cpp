// ILGTimePoints.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGTimePoints.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGTimePoints

IMPLEMENT_DYNAMIC(CILGTimePoints, CRecordset)

CILGTimePoints::CILGTimePoints(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGTimePoints)
	m_TimePointAbbr = _T("");
	m_TimePointName = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGTimePoints::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGTimePoints::GetDefaultSQL()
{
	return _T("[TimePoints]");
}

void CILGTimePoints::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGTimePoints)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[TimePointAbbr]"), m_TimePointAbbr);
	RFX_Text(pFX, _T("[TimePointName]"), m_TimePointName);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGTimePoints diagnostics

#ifdef _DEBUG
void CILGTimePoints::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGTimePoints::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
