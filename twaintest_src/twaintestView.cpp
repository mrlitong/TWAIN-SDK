// twaintestView.cpp : implementation of the CTwaintestView class
//

#include "stdafx.h"
#include "twaintest.h"

#include "twaintestDoc.h"
#include "twaintestView.h"
#include "Mainfrm.h"
#include "InputDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define DISTANCE(r1,g1,b1,r2,g2,b2) \
	    (long) (3L*(long)((r1)-(r2))*(long)((r1)-(r2)) + \
		    4L*(long)((g1)-(g2))*(long)((g1)-(g2)) + \
		    2L*(long)((b1)-(b2))*(long)((b1)-(b2)))


void Create24Bit(CDIB& source,CDIB& dest);
/////////////////////////////////////////////////////////////////////////////
// CTwaintestView

IMPLEMENT_DYNCREATE(CTwaintestView, CScrollView)

BEGIN_MESSAGE_MAP(CTwaintestView, CScrollView)
	//{{AFX_MSG_MAP(CTwaintestView)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTwaintestView construction/destruction

CTwaintestView::CTwaintestView()
{
	// TODO: add construction code here
}

CTwaintestView::~CTwaintestView()
{
}

BOOL CTwaintestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTwaintestView drawing

void CTwaintestView::OnDraw(CDC* pDC)
{
	CTwaintestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(pDoc->m_Dib.IsValid())
	{
		pDoc->m_Dib.BitBlt(pDC->m_hDC,0,0,pDoc->m_Dib.Width(),pDoc->m_Dib.Height(),0,0);
	}
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CTwaintestView printing

BOOL CTwaintestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTwaintestView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTwaintestView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTwaintestView diagnostics

#ifdef _DEBUG
void CTwaintestView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTwaintestView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CTwaintestDoc* CTwaintestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTwaintestDoc)));
	return (CTwaintestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTwaintestView message handlers

void CTwaintestView::OnInitialUpdate() 
{
	SetScrollSizes(MM_TEXT,CSize(100,100));
	CScrollView::OnInitialUpdate();
	OnUpdate(NULL,0,NULL);
}



void CTwaintestView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
CTwaintestDoc *pDoc = GetDocument();
	CSize size;
	size = CSize(pDoc->m_Dib.Width(),pDoc->m_Dib.Height());
	SetScrollSizes(MM_TEXT,size);
}

