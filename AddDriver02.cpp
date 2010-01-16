//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDriver02.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "TMS.h"
#include "AddDialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDriver02 property page

IMPLEMENT_DYNCREATE(CAddDriver02, CPropertyPage)

CAddDriver02::CAddDriver02() : CPropertyPage(CAddDriver02::IDD)
{
	//{{AFX_DATA_INIT(CAddDriver02)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAddDriver02::~CAddDriver02()
{
}

void CAddDriver02::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDriver02)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDriver02, CPropertyPage)
	//{{AFX_MSG_MAP(CAddDriver02)
	ON_BN_CLICKED(ADDDRIVER_HIREDATEBOX, OnHiredatebox)
	ON_BN_CLICKED(ADDDRIVER_TRANSITHIREDATEBOX, OnTransithiredatebox)
	ON_BN_CLICKED(ADDDRIVER_FULLTIMEDATEBOX, OnFulltimedatebox)
	ON_BN_CLICKED(ADDDRIVER_PROMOTIONDATEBOX, OnPromotiondatebox)
	ON_BN_CLICKED(ADDDRIVER_SENDATEBOX, OnSendatebox)
	ON_BN_CLICKED(ADDDRIVER_TERMINATIONDATEBOX, OnTerminationdatebox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDriver02 message handlers

BOOL CAddDriver02::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  pButtonHIREDATEBOX = (CButton *)GetDlgItem(ADDDRIVER_HIREDATEBOX);
  pDTPickerHIREDATE = (CDTPicker *)GetDlgItem(ADDDRIVER_HIREDATE);
  pButtonTRANSITHIREDATEBOX = (CButton *)GetDlgItem(ADDDRIVER_TRANSITHIREDATEBOX);
  pDTPickerTRANSITHIREDATE = (CDTPicker *)GetDlgItem(ADDDRIVER_TRANSITHIREDATE);
  pButtonFULLTIMEDATEBOX = (CButton *)GetDlgItem(ADDDRIVER_FULLTIMEDATEBOX);
  pDTPickerFULLTIMEDATE = (CDTPicker *)GetDlgItem(ADDDRIVER_FULLTIMEDATE);
  pButtonPROMOTIONDATEBOX = (CButton *)GetDlgItem(ADDDRIVER_PROMOTIONDATEBOX);
  pDTPickerPROMOTIONDATE = (CDTPicker *)GetDlgItem(ADDDRIVER_PROMOTIONDATE);
  pButtonSENDATEBOX = (CButton *)GetDlgItem(ADDDRIVER_SENDATEBOX);
  pDTPickerSENDATE = (CDTPicker *)GetDlgItem(ADDDRIVER_SENDATE);
  pButtonTERMINATIONDATEBOX = (CButton *)GetDlgItem(ADDDRIVER_TERMINATIONDATEBOX);
  pDTPickerTERMINATIONDATE = (CDTPicker *)GetDlgItem(ADDDRIVER_TERMINATIONDATE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddDriver02::OnHiredatebox() 
{
  if(pButtonHIREDATEBOX->GetCheck())
  {
    SetDateField(pDTPickerHIREDATE);
  }
  else
  {
    ClearDateField(pDTPickerHIREDATE);
  }
}

void CAddDriver02::OnTransithiredatebox() 
{
  if(pButtonTRANSITHIREDATEBOX->GetCheck())
  {
    SetDateField(pDTPickerTRANSITHIREDATE);
  }
  else
  {
    ClearDateField(pDTPickerTRANSITHIREDATE);
  }
}

void CAddDriver02::OnFulltimedatebox() 
{
  if(pButtonFULLTIMEDATEBOX->GetCheck())
  {
    SetDateField(pDTPickerFULLTIMEDATE);
  }
  else
  {
    ClearDateField(pDTPickerFULLTIMEDATE);
  }
}

void CAddDriver02::OnPromotiondatebox() 
{
  if(pButtonPROMOTIONDATEBOX->GetCheck())
  {
    SetDateField(pDTPickerPROMOTIONDATE);
  }
  else
  {
    ClearDateField(pDTPickerPROMOTIONDATE);
  }
}

void CAddDriver02::OnSendatebox() 
{
  if(pButtonSENDATEBOX->GetCheck())
  {
    SetDateField(pDTPickerSENDATE);
  }
  else
  {
    ClearDateField(pDTPickerSENDATE);
  }
}

void CAddDriver02::OnTerminationdatebox() 
{
  if(pButtonTERMINATIONDATEBOX->GetCheck())
  {
    SetDateField(pDTPickerTERMINATIONDATE);
  }
  else
  {
    ClearDateField(pDTPickerTERMINATIONDATE);
  }
}

void CAddDriver02::ClearDateField(CDTPicker *pDTP)
{
  COleVariant v;

  long day = 1;
  long month = 1;
  long year = 1900;

  v = year;
  pDTP->SetYear(v);
  v = month;
  pDTP->SetMonth(v);
  v = day;
  pDTP->SetDay(v);

  pDTP->EnableWindow(FALSE);
}

void CAddDriver02::SetDateField(CDTPicker *pDTP)
{
  COleVariant v;

  CTime time = CTime::GetCurrentTime();

  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTP->SetYear(v);
  v = month;
  pDTP->SetMonth(v);
  v = day;
  pDTP->SetDay(v);

  pDTP->EnableWindow(TRUE);
}

