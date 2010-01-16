// TextStopFlags.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextStopFlags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextStopFlags

IMPLEMENT_DYNAMIC(CTextStopFlags, CRecordset)

CTextStopFlags::CTextStopFlags(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextStopFlags)
	m_StopFlagNumber = 0;
	m_StopFlagText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextStopFlags::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextStopFlags::GetDefaultSQL()
{
	return _T("[Stop Flags]");
}

void CTextStopFlags::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextStopFlags)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[StopFlagNumber]"), m_StopFlagNumber);
	RFX_Text(pFX, _T("[StopFlagText]"), m_StopFlagText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextStopFlags diagnostics

#ifdef _DEBUG
void CTextStopFlags::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextStopFlags::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
