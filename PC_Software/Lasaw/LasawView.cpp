
// LasawView.cpp : CLasawView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Lasaw.h"
#endif

#include "LasawDoc.h"
#include "LasawView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLasawView

IMPLEMENT_DYNCREATE(CLasawView, CView)

BEGIN_MESSAGE_MAP(CLasawView, CView)
END_MESSAGE_MAP()

// CLasawView 构造/析构

CLasawView::CLasawView()
{
	// TODO:  在此处添加构造代码

}

CLasawView::~CLasawView()
{
}

BOOL CLasawView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CLasawView 绘制

void CLasawView::OnDraw(CDC* /*pDC*/)
{
	CLasawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  在此处为本机数据添加绘制代码
}


// CLasawView 诊断

#ifdef _DEBUG
void CLasawView::AssertValid() const
{
	CView::AssertValid();
}

void CLasawView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLasawDoc* CLasawView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLasawDoc)));
	return (CLasawDoc*)m_pDocument;
}
#endif //_DEBUG


// CLasawView 消息处理程序
