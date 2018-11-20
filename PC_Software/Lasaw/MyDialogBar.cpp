// MyDialogBar.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "MyDialogBar.h"


// CMyDialogBar 对话框

IMPLEMENT_DYNAMIC(CMyDialogBar, CDialogBar)

CMyDialogBar::CMyDialogBar()
: CDialogBar()
{
	m_bAutoZoom = TRUE;
}

CMyDialogBar::~CMyDialogBar()
{
}

void CMyDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyDialogBar, CDialogBar)
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTip)
	ON_BN_CLICKED(ID_MENUBUTTON_USER, &CMyDialogBar::OnBnClickedMenubuttonUser)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(ID_BUTTON_ZOOM, &CMyDialogBar::OnBnClickedButtonZoom)
END_MESSAGE_MAP()

LRESULT CMyDialogBar::OnInitDialog(UINT wParam, LONG lParam)
{
	BOOL bRet = HandleInitDialog(wParam, lParam);
	// Add Code here
	if (!UpdateData(FALSE)){
		TRACE0("Warning, Unalbe to init update./n");
	}
	m_menu.LoadMenu(IDR_MENU_USER);
	EnableToolTips();
	return bRet;
}

BOOL CMyDialogBar::OnToolTip(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT*)pNMHDR;
	UINT uID = pNMHDR->idFrom;
	if (pTTT->uFlags & TTF_IDISHWND)
		uID = ::GetDlgCtrlID((HWND)uID);
	if (uID == NULL)   return FALSE;
	switch (uID)
	{
	case  ID_BUTTON_AUTO:
		pTTT->lpszText = _T("进入联机自动运行模式");
		break;
	case  ID_BUTTON_HOME:
		pTTT->lpszText = _T("进行回零复位状态");
		break;
	case  ID_BUTTON_STOP:
		pTTT->lpszText = _T("停止当前运行状态");
		break;
	case  ID_BUTTON_EMG:
		pTTT->lpszText = _T("异常停止后，需回零复位");
		break;
	case  ID_RESET_STATUS:
		pTTT->lpszText = _T("重置异常状态");
		break;
	case  ID_RESET_PROCESS:
		pTTT->lpszText = _T("重置运转进度");
		break;
	case  ID_VIEW_IO:
		pTTT->lpszText = _T("输入输出信号及轴状态");
		break;
	case  ID_BUTTON_ZOOM:
		pTTT->lpszText = _T("图像缩放或自适应");
		break;
	case  ID_MENUBUTTON_USER:
		pTTT->lpszText = _T("账户登录");
		break;
	}
	return   TRUE;
}


// CMyDialogBar 消息处理程序


void CMyDialogBar::OnBnClickedMenubuttonUser()
{
	// TODO:  在此添加控件通知处理程序代码
	CMFCMenuButton* pBtn = (CMFCMenuButton*)GetDlgItem(ID_MENUBUTTON_USER);
	switch (pBtn->m_nMenuResult)
	{
	case ID_USER_LOGIN:
		theApp.UserLogin();
		break;
	case ID_USER_LOGOUT:
		theApp.UserLogOut();
		break;
	default:

		break;
	}
}

void CMyDialogBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogBar::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
// 		CRect ClientRc; GetWindowRect(ClientRc);
// 		GetDockingFrame()->GetClientRect(&ClientRc);
// 		m_sizeDefault.cx = ClientRc.Width();

		CMFCMenuButton* pBtn = (CMFCMenuButton*)GetDlgItem(ID_MENUBUTTON_USER);
		pBtn->m_hMenu = m_menu.GetSubMenu(0)->GetSafeHmenu();
// 		pBtn->m_bRightArrow = TRUE;
// 		pBtn->RedrawWindow();
	}
	// TODO:  在此处添加消息处理程序代码
}


void CMyDialogBar::OnUpdateBtnUI(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable();
}

BOOL CMyDialogBar::Create(CWnd* pParentWnd, UINT nIDTemplate,
	UINT nStyle, UINT nID, BOOL bChange)
{
	if (!CDialogBar::Create(pParentWnd, nIDTemplate, nStyle, nID))
		return FALSE;
	m_sizeDefault.cx = 1280;
	m_bChangeDockedSize = bChange;
	m_sizeFloating = m_sizeDocked = m_sizeDefault;
	return TRUE;
}

BOOL CMyDialogBar::Create(CWnd* pParentWnd,
	LPCTSTR lpszTemplateName,
	UINT nStyle, UINT nID, BOOL bChange)
{
	if (!CDialogBar::Create(pParentWnd, lpszTemplateName,
		nStyle, nID))
		return FALSE;

	m_bChangeDockedSize = bChange;
	m_sizeFloating = m_sizeDocked = m_sizeDefault;
	return TRUE;
}

CSize CMyDialogBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	// Return default if it is being docked or floated
	if ((dwMode & LM_VERTDOCK) || (dwMode & LM_HORZDOCK))
	{
		if (dwMode & LM_STRETCH) // if not docked stretch to fit
			return CSize((dwMode & LM_HORZ) ? 32767 : m_sizeDocked.cx,
			(dwMode & LM_HORZ) ? m_sizeDocked.cy : 32767);
		else
			return m_sizeDocked;
	}
	if (dwMode & LM_MRUWIDTH)
		return m_sizeFloating;
	// In all other cases, accept the dynamic length
	if (dwMode & LM_LENGTHY)
		return CSize(m_sizeFloating.cx, (m_bChangeDockedSize) ?
		m_sizeFloating.cy = m_sizeDocked.cy = nLength :
		m_sizeFloating.cy = nLength);
	else
		return CSize((m_bChangeDockedSize) ?
		m_sizeFloating.cx = m_sizeDocked.cx = nLength :
		m_sizeFloating.cx = nLength, m_sizeFloating.cy);
}


void CMyDialogBar::OnBnClickedButtonZoom()
{
	// TODO:  在此添加控件通知处理程序代码
	CString btntext[2] = { _T("自适应"), _T("图像缩放") };
	theApp.m_NiVision.EnableZoomDisplay(0xff, m_bAutoZoom);
	m_bAutoZoom = !m_bAutoZoom;
	SetDlgItemText(ID_BUTTON_ZOOM, btntext[m_bAutoZoom]);
}
