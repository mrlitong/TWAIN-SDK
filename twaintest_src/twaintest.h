// twaintest.h : main header file for the TWAINTEST application
//

#if !defined(AFX_TWAINTEST_H__2C017504_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
#define AFX_TWAINTEST_H__2C017504_4DC7_11D3_915F_E5DF02F24121__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
/////////////////////////////////////////////////////////////////////////////
// CTwaintestApp:
// See twaintest.cpp for the implementation of this class
//

class CTwaintestApp : public CWinApp
{
public:
	CTwaintestApp();
	~CTwaintestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTwaintestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTwaintestApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TWAINTEST_H__2C017504_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
