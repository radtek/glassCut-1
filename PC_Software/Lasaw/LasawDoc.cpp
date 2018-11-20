
// LasawDoc.cpp : CLasawDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Lasaw.h"
#endif
#include "MainFrm.h"

#include "LasawDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLasawDoc

IMPLEMENT_DYNCREATE(CLasawDoc, CDocument)

BEGIN_MESSAGE_MAP(CLasawDoc, CDocument)
	ON_COMMAND(ID_PROJECT_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)

	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)

	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CLasawDoc::OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CLasawDoc::OnUpdateFileOpen)
END_MESSAGE_MAP()


// CLasawDoc 构造/析构

CLasawDoc::CLasawDoc()
{
	// TODO:  在此添加一次性构造代码

}

CLasawDoc::~CLasawDoc()
{
}

BOOL CLasawDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO:  在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CLasawDoc 序列化

void CLasawDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO:  在此添加存储代码
	}
	else
	{
		// TODO:  在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CLasawDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CLasawDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CLasawDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CLasawDoc 诊断

#ifdef _DEBUG
void CLasawDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLasawDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLasawDoc 命令
void CLasawDoc::OnFileSave()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->StartTemplateRW(FALSE);

}

void CLasawDoc::OnFileSaveAs()
{
	theApp.OpenFileDlg(FALSE);
}

void CLasawDoc::OnFileOpen()
{
	theApp.OpenFileDlg();
}

void CLasawDoc::OnFileNew()
{
	::PostMessage(theApp.m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 4, 1);

}



void CLasawDoc::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
// 	pCmdUI->Enable(theApp.m_nCurrentCameraID);
}


void CLasawDoc::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
// 	pCmdUI->Enable(theApp.m_nCurrentCameraID);

}
