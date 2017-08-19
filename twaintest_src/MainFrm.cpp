// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "twaintest.h"

#include "MainFrm.h"
#include "TwainTestDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define HIWORD(l)   ((WORD) (((DWORD) (l) >> 16) & 0xFFFF))  
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_SELECTSOURCE, OnFileSelectsource)
	ON_UPDATE_COMMAND_UI(ID_FILE_SELECTSOURCE, OnUpdateFileSelectsource)
	ON_COMMAND(ID_FILE_ACQUIRE, OnFileAcquire)
	ON_UPDATE_COMMAND_UI(ID_FILE_ACQUIRE, OnUpdateFileAcquire)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pDoc = NULL;	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

//<<<<<<<TWAIN>>>>>>>>
	/* Try to load TWAIN now */

	InitTwain(m_hWnd);
	if(!IsValidDriver())
	{
		AfxMessageBox("Unable to load Twain Driver.");
	}
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
//<<<<<<<TWAIN>>>>>>>>
	ProcessMessage(*pMsg);
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}


void CMainFrame::SetAcquireDoc(CTwaintestDoc *pDoc)
{
	m_pDoc = pDoc;
}


BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CMDIFrameWnd::OnCommand(wParam, lParam);
}

static unsigned char masktable[] = { 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01 };

void Create24Bit(CDIB& source,CDIB& dest)
{
int pal;
	dest.Create(source.Width(),source.Height(),24);
	pal = source.GetPaletteSize();
BYTE palet[768];
	for(int i=0; i < pal; i++)
	{
		COLORREF col = source.PaletteColor(i);
		palet[i*3+2] = GetRValue(col);
		palet[i*3+1] = GetGValue(col);
		palet[i*3+0] = GetBValue(col);
	}
int j;
BYTE *src,*dst,*ptr;
	for(i=0; i < source.Height(); i++)
	{
		src = source.GetLinePtr(i);
		dst = dest.GetLinePtr(i);
		ptr = dst;
		int index;
		for(j=0; j < source.Width(); j++,ptr+=3)
		{
			switch(pal)
			{
			case 2:
				if(src[j>>3] & masktable[j&7])
				{
					index = 1;
				}
				else
				{
					index = 0;
				}
				break;
			case 16:
				if(j & 1)
				{
					index = src[j>>1] & 0x0f;
				}
				else
				{
					index = (src[j>>1] >> 4) & 0x0f;
				}
				break;
			case 256:
				index = src[j];
				break;
			}
			ASSERT(index < pal);
			memcpy(ptr,palet+index*3,3);
		}
		index = (ptr - dst)/3;
		ASSERT(index <= source.Width());
	}
		
}

void CMainFrame::SetImage(HANDLE hBitmap,int bits)
{
	CWinApp *pApp = AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	CMultiDocTemplate *pTemplate = (CMultiDocTemplate *)pApp->GetNextDocTemplate(pos);
	CTwaintestDoc *pDoc =  (CTwaintestDoc *)pTemplate->OpenDocumentFile(NULL);
	if(pDoc)
	{
		CDIB dib;
		dib.CreateFromHandle(hBitmap,bits);
		if(bits == 24)  pDoc->m_Dib = dib;
		else Create24Bit(dib,pDoc->m_Dib);
		pDoc->UpdateAllViews(NULL);
	}
}

//<<<<<<<TWAIN>>>>>>>>
void CMainFrame::CopyImage(HANDLE hBitmap,TW_IMAGEINFO& info)
{
	SetImage(hBitmap,info.BitsPerPixel);
}

void CMainFrame::OnFileSelectsource() 
{
	// TODO: Add your command handler code here
	SelectSource();
}

void CMainFrame::OnUpdateFileSelectsource(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsValidDriver());	
}

void CMainFrame::OnFileAcquire() 
{
	Acquire(TWCPP_ANYCOUNT);
}

void CMainFrame::OnUpdateFileAcquire(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(SourceSelected());	
}



void CMainFrame::OnClose() 
{
	ReleaseTwain();
	CMDIFrameWnd::OnClose();
}
