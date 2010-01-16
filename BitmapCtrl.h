//
// BitmapCtrl.h : header file
//

#pragma once
/////////////////////////////////////////////////////////////////////////////
// CBitmapCtrl window

class CBitmapCtrl : public CWnd
{
public:
	void SetBitmap(HBITMAP hBmp);
	void DrawBitmap();
	CRect rcClient;
	int scrollbar_width;
	int x_offset;
	int y_offset;
	CSize image_size;
	HBITMAP hBitmap;
		

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
public:
	void LoadFile(CString filename);
	virtual ~CBitmapCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBitmapCtrl)
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CDC memDC;
	CBitmap* pBitmap;
	CBitmap* pOldBitmap;
};

/////////////////////////////////////////////////////////////////////////////
