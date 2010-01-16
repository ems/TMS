#if !defined(AFX_RIDERSHIP_H__6F3363FD_5FD3_4A65_9D75_8BFA93F1FD31__INCLUDED_)
#define AFX_RIDERSHIP_H__6F3363FD_5FD3_4A65_9D75_8BFA93F1FD31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ridership.h : header file
//
typedef struct RowByRowStruct
{
  long  ROUTESrecordID;
  long  TRIPSrecordID;
  long  NODESrecordID;
  int   nodePositionInTrip;
  float fareValue[4];
} RowByRowDef;

/////////////////////////////////////////////////////////////////////////////
// CRidership dialog

class CRidership : public CDialog
{
// Construction
public:
	CRidership(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRidership)
	enum { IDD = IDD_RIDERSHIP };
	CNumericEdit	m_ctlAdjustments;
	CNumericEdit	m_ctlMileageOut;
	CNumericEdit	m_ctlMileageIn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRidership)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CDTPicker* pDTPickerDATE;
  CComboBox* pComboBoxRUNNUMBER;
  CComboBox* pComboBoxBUS;
  CComboBox* pComboBoxACTUALDRIVER;
  CStatic*   pStaticROSTEREDDRIVER;
  CSDIGrid*  pSDIGridGRID;
  CListCtrl* pListCtrlLIST;
  CStatic*   pStaticTOTALS;
  CNumericEdit*     pNumericEditMILEAGEOUT;
  CNumericEdit*     pNumericEditMILEAGEIN;
  CStatic*   pStaticTOTALMILEAGE;
  CEdit*     pEditNOTES;

  long m_ROSTERDivisionInEffect;
  long m_RUNSDivisionInEffect;
  int  m_Today;
  long m_DateToUse;
  long m_SERVICESrecordIDInEffect;
  long m_numRows;
  int  m_numTrips;
  int  m_IndexToFTBR[500];
  RowByRowDef m_RowByRow[500];
  float m_MileageOut, m_MileageIn;
  float m_CashAdjustments;
  long  m_RUNSrecordID;
  long  m_RunNumber;
  long  m_ActualDRIVERSrecordID;
  long  m_RosteredDRIVERSrecordID;
  BOOL  m_bNeedToUpdate;

  void ZeroOutList();


	// Generated message map functions
	//{{AFX_MSG(CRidership)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDate();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnSelchangeRunnumber();
	afx_msg void OnCalculate();
	afx_msg void OnAccept();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RIDERSHIP_H__6F3363FD_5FD3_4A65_9D75_8BFA93F1FD31__INCLUDED_)
