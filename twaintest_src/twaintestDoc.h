// twaintestDoc.h : interface of the CTwaintestDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TWAINTESTDOC_H__2C01750C_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
#define AFX_TWAINTESTDOC_H__2C01750C_4DC7_11D3_915F_E5DF02F24121__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dib.h"

class CTwaintestDoc : public CDocument
{
protected: // create from serialization only
	CTwaintestDoc();
	DECLARE_DYNCREATE(CTwaintestDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTwaintestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTwaintestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTwaintestDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CDIB m_Dib;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TWAINTESTDOC_H__2C01750C_4DC7_11D3_915F_E5DF02F24121__INCLUDED_)
