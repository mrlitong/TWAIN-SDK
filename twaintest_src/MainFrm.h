// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__2C017508_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
#define AFX_MAINFRM_H__2C017508_4DC7_11D3_915F_E5DF02F24121__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTwaintestDoc;

#include "TwainCpp.h"

class CMainFrame : public CMDIFrameWnd,public CTwain
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
	void SetAcquireDoc(CTwaintestDoc *pDoc);
	void SetImage(HANDLE hBitmap,int bits);
	void CopyImage(HANDLE hBitmap,TW_IMAGEINFO& info);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileSelectsource();
	afx_msg void OnUpdateFileSelectsource(CCmdUI* pCmdUI);
	afx_msg void OnFileAcquire();
	afx_msg void OnUpdateFileAcquire(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CTwaintestDoc *m_pDoc;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__2C017508_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
