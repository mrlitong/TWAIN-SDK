// twaintestDoc.cpp : implementation of the CTwaintestDoc class
//

#include "stdafx.h"
#include "twaintest.h"
#include "twaintestDoc.h"
#include "Mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTwaintestDoc

IMPLEMENT_DYNCREATE(CTwaintestDoc, CDocument)

BEGIN_MESSAGE_MAP(CTwaintestDoc, CDocument)
	//{{AFX_MSG_MAP(CTwaintestDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTwaintestDoc construction/destruction

CTwaintestDoc::CTwaintestDoc()
{
	// TODO: add one-time construction code here

}

CTwaintestDoc::~CTwaintestDoc()
{
}

BOOL CTwaintestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTwaintestDoc serialization

void CTwaintestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTwaintestDoc diagnostics

#ifdef _DEBUG
void CTwaintestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTwaintestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTwaintestDoc commands

