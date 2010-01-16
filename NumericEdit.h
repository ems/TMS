/**
 *	NumericEdit.h - main header
 *	
 *	Written By Gurmeet S. Kochar <gomzygotit@hotmail.com>
 *	Copyright (c) 2004. All rights reserved.
 *
 *	This code may be used in compiled form in any way you desire 
 *	(including commercial use). The code may be redistributed 
 *	unmodified by any means PROVIDING it is not sold for profit 
 *	without the authors written consent, and providing that this 
 *	notice and the authors name and all copyright notices remains 
 *	intact. However, this file and the accompanying source code may 
 *	not be hosted on a website or bulletin board without the authors 
 *	written permission.
 *	
 *	This file is provided "AS IS" with no expressed or implied warranty.
 *	The author accepts no liability for any damage/loss of business that
 *	this product may cause.
 */
#ifndef __NUMERICEDIT_H__
#define __NUMERICEDIT_H__

#if _MSC_VER > 1000
#pragma once
#endif	// _MSC_VER > 1000

class CNumericEdit : public CEdit
{
private:
	/**
	 *	Added By Gurmeet S. Kochar (Mar 04, 2004)
	 */
	struct _LocaleInfo
	{
		TCHAR	chDecimalSymbol;	// character used for decimal separator
		TCHAR	chNegationSymbol;	// character used for negative sign

		_LocaleInfo()
		{
			//
			// you MAY initialize these variables to 
			// any other value you like, for e.g., zero
			//
			chDecimalSymbol = _T('.');	// reasonable default!
			chNegationSymbol = _T('-');	// reasonable default!

			CString	strT;	// temporary string
			/**
			 *	retrieve the symbol used as the decimal separator
			 */
			int	iResult = ::GetLocaleInfo(LOCALE_USER_DEFAULT, 
										  LOCALE_SDECIMAL, 
										  strT.GetBufferSetLength(2), 2);
			strT.ReleaseBuffer();
			if (iResult)	chDecimalSymbol = strT[0];

			/**
			 *	retrieve the symbol used as the negative sign
			 */
			iResult = ::GetLocaleInfo(LOCALE_USER_DEFAULT, 
									  LOCALE_SNEGATIVESIGN, 
									  strT.GetBufferSetLength(2), 2);
			strT.ReleaseBuffer();
			if (iResult)	chNegationSymbol = strT[0];
		}
	};

// Constructors
public:
	CNumericEdit() : CEdit() { 
		m_bAllowNegativeValues = true;
		m_chDigitsAfterDecimal = 2;	// change this to any value between 0 and 9!!!
	}

// Overridables
public:
	virtual BOOL PreTranslateMessage(MSG *pMSG);

// Message Handler(s)
private:
	/**
	 *	Added By Gurmeet S. Kochar (Mar 04, 2004)
	 */
	afx_msg	LRESULT OnPaste(WPARAM wParam, LPARAM lParam);

// Helpers
private:
	/**
	 *	Added By Gurmeet S. Kochar (Mar 04, 2004)
	 */
	CString GetClipboardText(void) const;

// Attributes
public:
	bool	m_bAllowNegativeValues;	// determines whether negative values are allowed
	TCHAR	m_chDigitsAfterDecimal;	// number of digits to allow after decimal
private:
	/**
	 *	Added By Gurmeet S. Kochar (Mar 04, 2004)
	 */
	static const _LocaleInfo	DefUserLocale;
	// Generated message map functions
protected:
	//{{AFX_MSG(CNumericEdit)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif	// !__NUMERICEDIT_H__
