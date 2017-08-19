// twaintestView.h : interface of the CTwaintestView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TWAINTESTVIEW_H__2C01750E_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
#define AFX_TWAINTESTVIEW_H__2C01750E_4DC7_11D3_915F_E5DF02F24121__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTwaintestView : public CScrollView
{
protected: // create from serialization only
	CTwaintestView();
	DECLARE_DYNCREATE(CTwaintestView)

// Attributes
public:
	CTwaintestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTwaintestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTwaintestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTwaintestView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in twaintestView.cpp
inline CTwaintestDoc* CTwaintestView::GetDocument()
   { return (CTwaintestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TWAINTESTVIEW_H__2C01750E_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
