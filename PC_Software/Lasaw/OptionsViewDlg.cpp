// OptionsViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Lasaw.h"
#include "OptionsViewDlg.h"
#include "MainFrm.h"


// COptionsViewDlg 对话框

IMPLEMENT_DYNCREATE(COptionsViewDlg, CFormView)

COptionsViewDlg::COptionsViewDlg(CWnd* pParent /*=NULL*/)
: CFormView(COptionsViewDlg::IDD)
{
	m_tab_selected = 0;
}

COptionsViewDlg::~COptionsViewDlg()
{
}

void COptionsViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SETTING, m_tab);
}


BEGIN_MESSAGE_MAP(COptionsViewDlg, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE(WM_USER_UPDATEUI, &COptionsViewDlg::OnUpdateDlg)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SETTING, &COptionsViewDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// COptionsViewDlg 消息处理程序
void COptionsViewDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO:  在此添加额外的初始化
	//插入标签
	m_tab.InsertItem(0, _T("产品设置"));
	m_tab.InsertItem(1, _T("切割设置"));
	m_tab.InsertItem(2, _T("视觉设置"));
	m_tab.InsertItem(3, _T("相机设置"));
	//创建子对话框
	m_PaneSettingDlg.Create(CPaneSettingsDlg::IDD, &m_tab);
	m_PrjSettingDlg.Create(CProjectSettingDlg::IDD, &m_tab);
	m_VisionCtrlDlg.Create(CVisionCtrlView::IDD, &m_tab);
	m_CaliDlg.Create(CalibrationDlg::IDD, &m_tab);
	m_tab.SetItemSize(CSize(150,30));
	m_PaneSettingDlg.ShowWindow(SW_SHOW);
	PostMessage(WM_USER_UPDATEUI,2,0);
	// TODO:  在此添加专用代码和/或调用基类
}

BOOL COptionsViewDlg::ShowTabWindow(int nIndex/*=0*/)
{
	BOOL bSuccess(TRUE);
	switch (nIndex)
	{
	case 0:
		m_CaliDlg.ShowWindow(SW_HIDE);
		m_PrjSettingDlg.ShowWindow(SW_HIDE);
		m_VisionCtrlDlg.ShowWindow(SW_HIDE);

		m_PaneSettingDlg.ShowWindow(SW_SHOW);
		m_PaneSettingDlg.SetFocus();
		::PostMessage(m_PaneSettingDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
		::PostMessage(m_PaneSettingDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);
		m_tab_selected = nIndex;
		break;
	case 1:
		m_CaliDlg.ShowWindow(SW_HIDE);
		m_PaneSettingDlg.ShowWindow(SW_HIDE);
		m_VisionCtrlDlg.ShowWindow(SW_HIDE);

		m_PrjSettingDlg.ShowWindow(SW_SHOW);
		m_PrjSettingDlg.SetFocus();
		::PostMessage(m_PrjSettingDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);
		::PostMessage(m_PrjSettingDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
		m_tab_selected = nIndex;
		break;
	case 2:
		m_CaliDlg.ShowWindow(SW_HIDE);
		m_PaneSettingDlg.ShowWindow(SW_HIDE);
		m_PrjSettingDlg.ShowWindow(SW_HIDE);

		m_VisionCtrlDlg.ShowWindow(SW_SHOW);
		m_VisionCtrlDlg.SetFocus();
		::PostMessage(m_VisionCtrlDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
		::PostMessage(m_VisionCtrlDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);
		m_tab_selected = nIndex;
		break;
	case 3:
		m_PaneSettingDlg.ShowWindow(SW_HIDE);
		m_PrjSettingDlg.ShowWindow(SW_HIDE);
		m_VisionCtrlDlg.ShowWindow(SW_HIDE);

		m_CaliDlg.ShowWindow(SW_SHOW);
		m_CaliDlg.SetFocus();
		::PostMessage(m_CaliDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
		::PostMessage(m_CaliDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);
		m_tab_selected = nIndex;
		break;
	default: bSuccess = FALSE; break;
	}
	return bSuccess;
}

void COptionsViewDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShowTabWindow(m_tab.GetCurSel());
	*pResult = 0;
}

LRESULT COptionsViewDlg::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);
	switch (wParam)
	{
	case 0://状态更新
		m_tab.EnableWindow(bEnable);
		switch (m_tab_selected)
		{
		case 0:
			::PostMessage(m_PaneSettingDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);
			break;
		case 1:
			::PostMessage(m_PrjSettingDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);
			break;
		case 2:
			::PostMessage(m_VisionCtrlDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);

			break;
		case 3:
			::PostMessage(m_CaliDlg.m_hWnd, WM_USER_UPDATEUI, 0, 0);

		}
		break;
	case 1://数据更新
		switch (m_tab_selected)
		{
		case 0:
			::PostMessage(m_PaneSettingDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
			break;

		case 1:
			::PostMessage(m_PrjSettingDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
			break;

		case 2:
			::PostMessage(m_VisionCtrlDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);
			break;
		case 3:
			::PostMessage(m_CaliDlg.m_hWnd, WM_USER_UPDATEUI, 2, 0);

		}
		break;
	case 2://切换页面
		if (lParam >= 0)
			m_tab.SetCurSel(lParam);
		ShowTabWindow(lParam);
		break;
	case 3:

		break;
	case 0x10:
		break;
	case 0x20:
		::PostMessage(m_PrjSettingDlg.m_hWnd, WM_USER_UPDATEUI, 4, lParam);
		break;
	case 0x30:
		::PostMessage(m_PrjSettingDlg.m_hWnd, WM_USER_UPDATEUI, 6, lParam);
		break;
	default:
		break;
	}
	return 0;
}




void COptionsViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if (!::IsWindow(m_hWnd))
		return;
	if (m_tab.m_hWnd == NULL)
		return;      // Return if window is not created yet.

	CRect rect;

	// Get size of dialog window.
	GetClientRect(rect);

// 	// Adjust the rectangle to fit the tab control into the 
// 	// dialog's client rectangle.
//  	m_tab.AdjustRect(FALSE, rect);
// 
	// Move the tab control to the new position and size.
	m_tab.MoveWindow(rect, TRUE);
	if (m_tab&&m_tab.GetItemCount()>0)
	{
		//调整子对话框的大小和位置
		m_tab.GetClientRect(rect);
		m_tab.SetItemSize(CSize((rect.Width()/4)-10, 30));

		CRect rcTabItem;
		m_tab.GetItemRect(0, rcTabItem);
		rect.top += rcTabItem.Height() + 4;
		rect.left += 4;
		rect.bottom -= 4;
		rect.right -= 4;
		m_PaneSettingDlg.MoveWindow(&rect);
		m_VisionCtrlDlg.MoveWindow(&rect);
		m_CaliDlg.MoveWindow(&rect);
		m_PrjSettingDlg.MoveWindow(&rect);

	}
	// TODO:  在此处添加消息处理程序代码
}


BOOL COptionsViewDlg::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	// TODO:  在此添加专用代码和/或调用基类
	Invalidate(FALSE);
	return CFormView::OnScroll(nScrollCode, nPos, bDoScroll);
}


BOOL COptionsViewDlg::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	// TODO:  在此添加专用代码和/或调用基类

	return CFormView::OnScrollBy(sizeScroll, bDoScroll);
}
