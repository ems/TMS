//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDTIMECHECK_H__74A6F5F5_9D0C_4FCF_A88D_88A5E77A1B5A__INCLUDED_)
#define AFX_ADDTIMECHECK_H__74A6F5F5_9D0C_4FCF_A88D_88A5E77A1B5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddTimecheck.h : header file
//

#define MAXTIMECHECKNODES 24

/////////////////////////////////////////////////////////////////////////////
// CAddTimecheck dialog

class CAddTimecheck : public CDialog
{
// Construction
public:
	CAddTimecheck(CWnd* pParent = NULL, long *pUdateRecordID = NULL, PDISPLAYINFO pDI = NULL);

// Dialog Data
	//{{AFX_DATA(CAddTimecheck)
	enum { IDD = IDD_ADDTIMECHECK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddTimecheck)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  PDISPLAYINFO m_pDI;
  long *m_pUpdateRecordID;
  long m_bInit;
  int  m_numNodes;
  int  m_NodePositionInTrip;
  long m_ROUTESrecordID;
  long m_SERVICESrecordID;
  long m_directionIndex;
  long m_TripNumber;
  long m_TRIPSrecordID;
  long m_NODESrecordIDs[MAXTIMECHECKNODES];
  long m_NodePositions[MAXTIMECHECKNODES];
  BOOL m_bFinishedInProgress;
  
  BOOL DisplayRSD(BOOL bLookupTrip, TRIPSDef *pTRIPS);
  void SetUpTextComboBox(GenericTextDef *pData, int numEntries, CComboBox *pComboBox);
  void SetUpTimepointsAndTimes(long NODESrecordID);
  void HighlightComboBoxValue(CComboBox *pComboBox, long searchValue);
  long GetComboBoxValue(CComboBox *pComboBox);

  CButton*   pButtonBYROUTE;
  CButton*   pButtonBYTRIPNUMBER;
  CComboBox* pComboBoxSPECIFICROUTE;
  CButton*   pButtonOUTBOUND;
  CButton*   pButtonINBOUND;
  CStatic*   pStaticSPECIFICSERVICETITLE;
  CComboBox* pComboBoxSPECIFICSERVICE;
  CStatic*   pStaticSPECIFICNODETITLE;
  CComboBox* pComboBoxSPECIFICNODE;
  CStatic*   pStaticSCHEDULEDTITLE;
  CComboBox* pComboBoxSCHEDULED;
  CStatic*   pStaticACTUALTITLE;
  CEdit*     pEditACTUAL00;
  CEdit*     pEditTRIPNUMBER;
  CStatic*   pStaticROUTE;
  CStatic*   pStaticDIRECTION;
  CStatic*   pStaticSERVICE;
  CStatic*   pStaticNODE[MAXTIMECHECKNODES];
  CStatic*   pStaticSCHEDULED[MAXTIMECHECKNODES];
  CEdit*     pEditACTUAL[MAXTIMECHECKNODES];
  CDTPicker* pDTPickerDATE;
  CComboBox* pComboBoxBUSTYPE;
  CComboBox* pComboBoxDRIVER;
  CComboBox* pComboBoxPASSENGERLOAD;
  CComboBox* pComboBoxWEATHER;
  CComboBox* pComboBoxTRAFFIC;
  CComboBox* pComboBoxOTHER;
  CComboBox* pComboBoxSUPERVISOR;
  CButton*   pButtonADD;

  
	// Generated message map functions
	//{{AFX_MSG(CAddTimecheck)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	afx_msg void OnAdd();
	afx_msg void OnFinished();
	afx_msg void OnClose();
	afx_msg void OnKillfocusTripnumber();
	afx_msg void OnByroute();
	afx_msg void OnBytripnumber();
	afx_msg void OnOutbound();
	afx_msg void OnInbound();
	afx_msg void OnEditchangeScheduled();
	afx_msg void OnSelchangeSpecificroute();
	afx_msg void OnSelchangeSpecificservice();
	afx_msg void OnSelchangeSpecificnode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDTIMECHECK_H__74A6F5F5_9D0C_4FCF_A88D_88A5E77A1B5A__INCLUDED_)
