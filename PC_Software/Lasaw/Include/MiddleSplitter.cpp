// MiddleSplitter.cpp : implementation file
//

#include "stdafx.h"
#include "MiddleSplitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMiddleSplitter

IMPLEMENT_DYNCREATE(CMiddleSplitter, CSplitterWnd)

CMiddleSplitter::CMiddleSplitter()
{
	m_cxSplitter = 4;    //must >=4 ,拖动时拖动条的宽度
	m_cySplitter = 4;    
	m_cxBorderShare = 0; //按下鼠标时拖动条的偏移量
	m_cyBorderShare = 0;  
	m_cxSplitterGap= 1;  //splitter拖动条的宽度   
	m_cySplitterGap= 1;  
}

CMiddleSplitter::~CMiddleSplitter()
{
}


BEGIN_MESSAGE_MAP(CMiddleSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CMiddleSplitter)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiddleSplitter drawing

void CMiddleSplitter::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CMiddleSplitter diagnostics

#ifdef _DEBUG
void CMiddleSplitter::AssertValid() const
{
	CSplitterWnd::AssertValid();
}

void CMiddleSplitter::Dump(CDumpContext& dc) const
{
	CSplitterWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMiddleSplitter message handlers

void CMiddleSplitter::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
//	if(point.y<250||point.y>500)
		return;
//	CSplitterWnd::OnLButtonDown(nFlags, point);
}

void CMiddleSplitter::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
//	if(point.y<250||point.y>500)
		return;
//	CSplitterWnd::OnMouseMove(nFlags, point);
}


BOOL CMiddleSplitter::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
// 	CBrush backBrush(RGB(165,204,211)); 
// 	CBrush* pOldBrush = pDC->SelectObject(&backBrush); 
// 	CRect rect; 
// 	pDC->GetClipBox(&rect);  
// 	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY); 
// 	pDC->SelectObject(pOldBrush); 
// 	backBrush.DeleteObject();
// 	DeleteObject(pOldBrush);
// 	return TRUE;
	return CSplitterWnd::OnEraseBkgnd(pDC);
}
