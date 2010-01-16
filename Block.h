#if !defined(AFX_BLOCK_H__78422C0A_F663_4F24_B7C8_92A1EDE051EC__INCLUDED_)
#define AFX_BLOCK_H__78422C0A_F663_4F24_B7C8_92A1EDE051EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Block.h : header file
//

typedef struct PARMSStruct
{
  BOOL bStandard;
  BOOL bDropback;
  BOOL bInterlining;
  BOOL bServiceWrap;
  BOOL bDeadheading;
  BOOL bDeadheadAll;
  long deadheadMins;
} PARMSDef;

#define TRIPDATA_FLAG_PARTNER           0x0001
#define TRIPDATA_FLAG_SERVICEWRAP_PLUS  0x0002
#define TRIPDATA_FLAG_SERVICEWRAP_MINUS 0x0004

typedef struct TripDataStruct
{
  long flags;
  long RGRPROUTESrecordID;
  char szRGRPRouteName[ROUTES_NAME_LENGTH + 1];
  long TRIPSrecordID;
  long ROUTESrecordID;
  char szRouteName[ROUTES_NAME_LENGTH + 1];
  long SERVICESrecordID;
  char szServiceName[SERVICES_NAME_LENGTH + 1];
  long PATTERNNAMESrecordID;
  char szPatternName[PATTERNNAMES_NAME_LENGTH + 1];
  long BUSTYPESrecordID;
  char szBustypeName[BUSTYPES_NAME_LENGTH + 1];
  long startNODESrecordID;
  char szStartNodeAbbrName[NODES_ABBRNAME_LENGTH + 1];
  long startTime;
  long endTime;
  long endNODESrecordID;
  char szEndNodeAbbrName[NODES_ABBRNAME_LENGTH + 1];
  long layoverMin;
  long layoverMax;
  long blockNumber;
  long lay;
  long dhd;
  int  directionIndex;
  char szDirectionAbbrName[DIRECTIONS_ABBRNAME_LENGTH + 1];
  int  indexIntoTripData;
} TripDataDef;

/////////////////////////////////////////////////////////////////////////////
// CBlock dialog

class CBlock : public CDialog
{
// Construction
public:
	CBlock(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBlock)
	enum { IDD = IDD_BLOCK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlock)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListCtrl* pListCtrlTRIPS;
  CListCtrl* pListCtrlBLOCKS;
  CListCtrl* pListCtrlPARTNERS;
  CButton*   pButtonDEADHEADING;
  CButton*   pButtonINTERLINING;
  CButton*   pButtonSERVICEWRAP;
  CButton*   pButtonDEADHEADALL;
  CEdit*     pEditDEADHEADMINS;
  CButton*   pButtonDEADHEADTHOSE;

  PDISPLAYINFO m_pDI;
  int          m_maxTrips;
  TripDataDef* m_pTripData;
  TripDataDef* m_pBlockData;
  PARMSDef*    m_pPARMS;
  BLOCKSDef*   m_pTRIPSChunk;
  LAYOVERDef*  m_pLayover;
  BOOL         m_bInitializing;
  BOOL         m_bDeadheading;
  BOOL         m_bServiceWrap;
  BOOL         m_bInterlining;
  HMENU        m_hMenu1;
  HMENU        m_hMenu2;
  HMENU        m_hMenu3;
  long         m_lastBlockNumber;
  int          m_numTrips;
  int          m_numBlockedTrips;

  void DrawTrips(int indexToDisplay);
  void DrawBlocks(BOOL bRebuild, int indexToDisplay);
  void DrawPartners(int blockedTripIndex);
  void ClearPartners(void);
  int  GetIndex(CListCtrl* pLC, int item);
  int  GetIndex(CListCtrl* pLC, int item, int* selection);
  void FreeMem();
  void EstablishDhdAndLay(long blockNumber);
  void ParameterChange();

	// Generated message map functions
	//{{AFX_MSG(CBlock)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDeadheadall();
	afx_msg void OnDeadheading();
	afx_msg void OnDeadheadthose();
	afx_msg void OnInterlining();
	afx_msg void OnDblclkTrips(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickBlocks(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkBlocks(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickBlocks(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkPartners(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLayover();
	afx_msg void OnSave();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnNewblock();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnUnhookblock();
	afx_msg void OnRenumber();
	afx_msg void OnSummary();
	afx_msg void OnTaketrip();
	afx_msg void OnChangeDeadheadmins();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnServicewrap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCK_H__78422C0A_F663_4F24_B7C8_92A1EDE051EC__INCLUDED_)
