// ImageView.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "ImageView.h"
#include "MainFrm.h"
// CImageView

IMPLEMENT_DYNCREATE(CImageView, CFormView )

CImageView::CImageView()
: CFormView(CImageView::IDD)
{
	m_fRate         =1;
	m_fZoom			= 1;
	m_bShowLines    = FALSE;
	m_bCameraLive   = FALSE;
	m_nID			= m_nOrder;
	szImgInfo.Format(_T("这是%d号图像界面\n"),m_nID+1);
	m_nOrder++;
}

UINT CImageView::m_nOrder = 0;

CImageView::~CImageView()
{
}

void CImageView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_PROCESS, m_ListBoxMSG);
}

BEGIN_MESSAGE_MAP(CImageView, CFormView )
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_MESSAGE(WM_USER_SHOWIMAGE, &CImageView::OnUpdateDisplay)
	ON_MESSAGE(WM_USER_UPDATEUI, &CImageView::OnUpdateDlg)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CImageView 诊断

#ifdef _DEBUG
void CImageView::AssertValid() const
{
	CFormView ::AssertValid();
}

#ifndef _WIN32_WCE
void CImageView::Dump(CDumpContext& dc) const
{
	CFormView ::Dump(dc);
}
#endif
#endif //_DEBUG


// CImageView 消息处理程序

void CImageView::OnInitialUpdate()
{
	CFormView ::OnInitialUpdate();
	CRect ClientRc; GetClientRect(ClientRc);
	m_ListBoxMSG.SetSel(-1, -1);
	// TODO: 在此添加专用代码和/或调用基类
}

void CImageView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMainFrame* pFrame=(CMainFrame*)AfxGetMainWnd();
	switch (nIDEvent)
	{
	case 1:
		break;
	case 2:
		break;
	}
	CFormView::OnTimer(nIDEvent);
}


void CImageView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if (!IsWindowVisible())
		return ;
	CRect rectClient;
	CWnd* pWnd = GetDlgItem(IDC_STATIC_DISPLAY);
	if (pWnd)
	{
		CRect rc;
// 		GetWindowRect(rectClient);
// 		ScreenToClient(rectClient);
		GetClientRect(rectClient);
		rc=rectClient;
		rc.InflateRect(0, 0, rectClient.Height()*4/3 - rectClient.Width()+2, 0);
		pWnd->MoveWindow(rc);

		rectClient.InflateRect(-rc.right,0,0,0);
		GetDlgItem(IDC_STATIC_SUMMARY)->MoveWindow(rectClient);
		rectClient.InflateRect(-10, -30, -10, -1);
		m_ListBoxMSG.MoveWindow(rectClient);
	}

	// TODO: 在此处添加消息处理程序代码
}

LRESULT CImageView::OnGetImageData(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CImageView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFormView::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
	}
	// TODO: 在此处添加消息处理程序代码
}

LRESULT CImageView::OnUpdateDisplay(WPARAM wParam, LPARAM lParam)
{
	UINT nIndex = wParam;
// 	if (!theApp.m_nCurrentCameraID||nIndex==(theApp.m_nCurrentCameraID-1))
		theApp.m_NiVision.Camera_Display(nIndex);
	return 0;
}

void CImageView::ModifyCamView(BYTE nCameraID)
{
	CRect ClientRc;
	CRect disRc[4];
	BOOL bLive(1);
	GetDlgItem(IDC_STATIC_DISPLAY)->GetClientRect(ClientRc);
	int nLenth = ClientRc.Width();
	int nWidth = ClientRc.Height();
	disRc[0] = ClientRc;
	disRc[0].InflateRect(0, 0, nWidth - nLenth, 0);

	disRc[1] = ClientRc;
	disRc[1].InflateRect(-disRc[0].Width()-2, 0,0, -(nWidth-4)*2 / 3-4);

	disRc[2] = disRc[1];
	disRc[2].MoveToY(disRc[1].bottom+2);

	disRc[3] = disRc[2];
	disRc[3].MoveToY(disRc[2].bottom+2);
	if (nCameraID > 3)
		nCameraID = 0;
	for (int i = 0; i < theApp.m_NiVision.m_nCameraNum; i++)
	{
		BYTE nWnd = theApp.m_NiVision.m_nDisplayBinder[theApp.m_nCameraID[i]];
		if (nCameraID==i)
			theApp.m_NiVision.ResizeDisplayWnd(nWnd, disRc[0]);
		else
			theApp.m_NiVision.ResizeDisplayWnd(nWnd, disRc[bLive++]);
		theApp.m_NiVision.StartImageMonitor(theApp.m_nCameraID[i], TRUE);
	}
}

void CImageView::AddedStatusMsg(CString Info, BOOL bClear)
{
	if (bClear)
	{
		PostMessage(WM_USER_UPDATEUI, 1, 1);
		return;
	}
	if (!Info.IsEmpty())
	{
		m_cListMsg.AddTail(Info);
		PostMessage(WM_USER_UPDATEUI, 1, 0);
	}
	return;
}

LRESULT CImageView::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	switch (wParam)
	{
	case 0:
		break;
	case 1:
		if (lParam)
		{
			m_cListMsg.RemoveAll();
			m_ListBoxMSG.SetSel(0, -1);
			m_ListBoxMSG.Clear();
		}
		else
		{
			if (m_cListMsg.GetCount())
			{
				CTime time = CTime::GetCurrentTime();
				CString strMsg = time.Format("[%H:%M:%S]:");
				strMsg = strMsg + m_cListMsg.GetHead();
				m_ListBoxMSG.SetSel(-1, -1);
				m_ListBoxMSG.ReplaceSel(strMsg);
				m_cListMsg.RemoveHead();
			}
		}
		break;
	default:
		break;
	}
	return 0;
}
