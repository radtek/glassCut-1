
// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "Lasaw.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_MESSAGE(WM_USER_SHOWERRORINFO, &CMainFrame::OnShowErrorMsg)
	ON_MESSAGE(WM_USER_UPDATEUI, &CMainFrame::OnUpdateMainFrm)
	ON_MESSAGE(WM_USER_IOCHANGE, &CMainFrame::OnIochange)
	ON_MESSAGE(WM_USER_MOTIONPOS, &CMainFrame::OnPositionChange)
	ON_MESSAGE(WM_USER_HOMED, &CMainFrame::OnHomed)
	ON_MESSAGE(WM_USER_SHOWIMAGE, &CMainFrame::OnUpdateDisplay)

	ON_COMMAND(ID_VIEW_TOOLBAR, &CMainFrame::OnViewTool)
	ON_COMMAND(ID_VIEW_IO, &CMainFrame::OnViewIODlg)
	ON_COMMAND(ID_VIEW_DXF, &CMainFrame::OnViewDxfDlg)
	ON_COMMAND(ID_RESET_STATUS, &CMainFrame::ResetSts)
	ON_COMMAND(ID_RESET_PROCESS, &CMainFrame::ResetProcess)
	ON_COMMAND(ID_BUTTON_STOP, &CMainFrame::OnCommandStop)
	ON_COMMAND(ID_BUTTON_EMG, &CMainFrame::OnButtonEmg)
// 	ON_COMMAND(ID_BUTTON_ZOOM, &CMainFrame::OnButtonZoom)
ON_MESSAGE(WM_COMM_RXSTR, OnCommRecvString)
ON_MESSAGE(WM_COMM_RXCHAR, OnCommRecvString)

	ON_COMMAND_RANGE(ID_BUTTON_HOME, ID_BUTTON_LATEST, &CMainFrame::OnOperationMode)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI_RANGE(ID_PROJECT_CONFIGURATION, ID_USER_LOGOUT, &CMainFrame::OnUpdateCommand)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CMainFrame::OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CMainFrame::OnUpdateFileSave)

END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_LABEL_A,
	ID_INDICATOR_LABEL_B,
	ID_INDICATOR_LABEL_C,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO:  在此添加成员初始化代码
	m_pOptionViewDlg = NULL;
	m_bFileThreadAlive = FALSE;
	m_bCreateSpl=FALSE;
	m_lQuantityStart = 0;
	m_pMainDlg=NULL;
	m_pMotionDlg = NULL;
	m_pWaiteProcessDlg = NULL;
	m_pDxfDlg = NULL;
}

CMainFrame::~CMainFrame()
{
	SAFE_DELETEDLG(m_pDxfDlg);

// 	for (int i = 0; i < 4;i++)
// 		theApp.m_NiVision.StartImageMonitor(i, FALSE);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMenu m_menu;
	m_menu.LoadMenu(IDR_MENU_MAIN);
	SetMenu(&m_menu);
	m_menu.Detach();   //该函数是用来将菜单句柄与菜单对象分离  
	if (!m_wndDlgBar.Create(this, IDD_DIALOGBAR, CBRS_ALIGN_TOP /*| CBRS_SIZE_DYNAMIC*/, IDD_DIALOGBAR))
	{
		TRACE0("未能创建对话栏\n");
		return -1;		// 未能创建
	}

	if (!m_wndReBar.Create(this, RBS_AUTOSIZE | RBS_BANDBORDERS) /*|| !m_wndReBar.AddBar(&m_wndToolBar)*/ || !m_wndReBar.AddBar(&m_wndDlgBar))
	{
		TRACE0("未能创建 Rebar\n");
		return -1;      // 未能创建
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("未能创建状态栏\r\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetPaneStyle(0,
		m_wndStatusBar.GetPaneStyle(0) | SBPS_OWNERDRAW);
	m_wndStatusBar.SetPaneStyle(1,
		m_wndStatusBar.GetPaneStyle(1) | SBPS_OWNERDRAW);
	m_wndStatusBar.SetPaneStyle(2,
		m_wndStatusBar.GetPaneStyle(2) | SBPS_OWNERDRAW);
	m_wndStatusBar.SetPaneStyle(3,
		m_wndStatusBar.GetPaneStyle(3) | SBPS_OWNERDRAW);
	m_wndStatusBar.SetPaneBackgroundColor(1, CLasawApp::DarkRed);
	// 	m_wndStatusBar.SetPaneTextColor(1, CBCGPColor::DarkRed);
// 	m_wndStatusBar.SetPaneWidth(1, 130);

	// TODO:  如果不需要工具提示，则将此移除
	m_wndDlgBar.SetBarStyle((m_wndDlgBar.GetBarStyle() | CBRS_TOOLTIPS) ^CBRS_FLYBY);
	SetTitle(_T("面板切割机"));
	CenterWindow();
	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO:  在此添加专用代码和/或调用基类
	CRect rect1;    GetWindowRect(rect1);
	CRect ClientRc; GetClientRect(ClientRc);
	m_frmSplitterWnd.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER);
	m_frmSplitterWnd.SetColumnInfo(0, 340, 10);
	m_frmSplitterWnd.SetColumnInfo(1, 940, 10);
	// 
	if (m_leftSplitterWnd.CreateStatic(&m_frmSplitterWnd, 2, 1, WS_CHILD | WS_VISIBLE, m_frmSplitterWnd.IdFromRowCol(0, 0)) == NULL)
		return FALSE;
	m_leftSplitterWnd.CreateView(0, 0, RUNTIME_CLASS(CMainCtrlDlg), CSize(0, 0), pContext);
	m_leftSplitterWnd.CreateView(1, 0, RUNTIME_CLASS(CMotionDlg), CSize(0, 0), pContext);
	m_leftSplitterWnd.SetRowInfo(0, ClientRc.Height(), 10);
	if (m_rightSplitterWnd.CreateStatic(&m_frmSplitterWnd, 2, 1, WS_CHILD | WS_VISIBLE, m_frmSplitterWnd.IdFromRowCol(0, 1)) == NULL)
		return FALSE;
	m_rightSplitterWnd.CreateView(0, 0, RUNTIME_CLASS(CImageView), CSize(940, 512), pContext);
	m_rightSplitterWnd.CreateView(1, 0, RUNTIME_CLASS(COptionsViewDlg), CSize(940, 300), pContext);
	m_rightSplitterWnd.SetRowInfo(0, 480, 10);

	m_pMainDlg = (CMainCtrlDlg*)m_leftSplitterWnd.GetPane(0, 0);
	m_pMotionDlg = (CMotionDlg*)m_leftSplitterWnd.GetPane(1, 0);
	m_pCCDView = (CImageView*)m_rightSplitterWnd.GetPane(0, 0);
	m_pOptionViewDlg = (COptionsViewDlg*)m_rightSplitterWnd.GetPane(1, 0);
	m_bCreateSpl = TRUE;
	RecalcLayout(FALSE);
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		/*| WS_MAXIMIZE */ | WS_MINIMIZEBOX | WS_SYSMENU ;
	cs.cx = 1920;
	cs.cy = 1080;

	return TRUE;
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO:  在此添加专用代码和/或调用基类
	if (m_wndDlgBar.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

LRESULT CMainFrame::OnCommRecvString(WPARAM wString, LPARAM lLength)
{
	DWORD nLength = (DWORD)lLength;
	CString strTemp;

	// 	LPCTSTR lpBuffer = (LPCTSTR)wString;
	// 	for (DWORD i = 0; i < nLength; i++)
	// 		strTemp += (TCHAR)lpBuffer[0];
	PulseEvent(theApp.m_hRs485Event);
	strTemp = (LPCTSTR)wString;
	TRACE1("%S\r\n", strTemp);
	return 0;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::UpdateTitle()
{
	CString sz;
	sz = theApp.m_cParam.DefaultProjectName + _T("<---------") + theApp.m_szUserName;
	GetActiveDocument()->SetTitle(sz);

}

BOOL CMainFrame::SetHardWareRun()
{
	CRect ClientRc;
	if (!(m_pCCDView &&m_pCCDView->GetSafeHwnd()))
		return FALSE;
	CString str;
	LightControllerOpen(TRUE);
	theApp.m_NiVision.SetHandleWnd(m_pCCDView);
	for (int i = 0; i < theApp.m_NiVision.m_nCameraNum; i++)
	{
		BYTE nWnd = theApp.m_NiVision.m_nDisplayBinder[theApp.m_nCameraID[i]];
		theApp.m_NiVision.SetWndParent(nWnd, m_pCCDView->GetDlgItem(IDC_STATIC_DISPLAY));
		theApp.m_NiVision.DrawCross(theApp.m_NiVision.m_pOverlay[theApp.m_nCameraID[i]], imaqMakePoint(1024, 1024), CLasawApp::BASIC_INFO_LAYER, &IMAQ_RGB_GREEN);
		str.Format(_T("相机%d"),i+1);
		theApp.m_NiVision.DrawTextInfo(theApp.m_NiVision.m_pOverlay[theApp.m_nCameraID[i]], str, imaqMakePoint(20, 2000), CLasawApp::BASIC_INFO_LAYER, 150, &IMAQ_RGB_GREEN);
	}
	// 	GetDlgItem(IDC_STATIC_CONTAIN)->GetClientRect(ClientRc);
	theApp.m_GgMotion.StartMotionCardMonitor(this);
	return TRUE;
}



// CMainFrame 消息处理程序

LRESULT CMainFrame::OnUpdateMainFrm(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	int i(0), j(0);
	CString str;
	CRect rect1;    GetWindowRect(&rect1);
	switch (wParam)
	{
	case 0:
		UpdateTitle();
		if (lParam)//数据刷新
		{
			::PostMessage(m_pOptionViewDlg->m_hWnd, WM_USER_UPDATEUI, 1, 0);
			::PostMessage(m_pMainDlg->m_hWnd, WM_USER_UPDATEUI, 2, 0);
			::PostMessage(m_pMotionDlg->m_hWnd, WM_USER_UPDATEUI, 4, 0);
		}
		else//界面状态更新
		{
			::PostMessage(m_pOptionViewDlg->m_hWnd, WM_USER_UPDATEUI, 0, 0);
			::PostMessage(m_pMainDlg->m_hWnd, WM_USER_UPDATEUI, 0, 0);
			::PostMessage(m_pMotionDlg->m_hWnd, WM_USER_UPDATEUI, 0, 1);

		}
		break;
	case 1://回零
		if (lParam != 0)//结束
		{
			theApp.m_nCurrentRunMode &= 0x01;
			AddedProcessMsg(_T("回零复位结束\r\n"));
		}
		else
		{
			theApp.m_nCurrentRunMode &= 0x03;
			PostMessage(WM_USER_UPDATEUI, 5, 0);
		}
		PostMessage(WM_USER_UPDATEUI, 0, 0);

		break;
	case 2://自动
		if (lParam != 0)//结束
		{
			theApp.m_nCurrentRunMode &= 0x01;
		}
		else{
			theApp.m_nCurrentRunMode &= 0x0D;//
			PostMessage(WM_USER_UPDATEUI, 5, 0);
		}
		PostMessage(WM_USER_UPDATEUI, 0, 0);

		break;
	case 3://等待对话框
		ShowWaiteProcessDlg(lParam);
		break;
	case 4://新项目视图
		m_pOptionViewDlg->ShowTabWindow(2);
		::PostMessage(m_pOptionViewDlg->m_hWnd, WM_USER_UPDATEUI, 0x30, 1);
		break;
	case 5://左视图切换
		if (lParam)
		{
			m_leftSplitterWnd.SetRowInfo(0, 0, 0);//main
			m_leftSplitterWnd.SetRowInfo(1, rect1.Height(), 10);
		}
		else
		{
			m_leftSplitterWnd.SetRowInfo(1, 0, 0);//motion
			m_leftSplitterWnd.SetRowInfo(0, rect1.Height(), 10);
		}
		m_leftSplitterWnd.RecalcLayout();
		RecalcLayout(FALSE);
		break;
	case 6:
		str.Format(_T("当前：%d"), lParam);
		m_wndStatusBar.SetPaneText(1, str);
		break;
	case 7:
		SetHardWareRun();
		StartTemplateRW(TRUE,1);
		if (!lParam)
			break;
	case 8:
		m_pCCDView->ModifyCamView(lParam);
		break;
	case 9:
		::PostMessage(m_pMainDlg->m_hWnd, WM_USER_UPDATEUI, 4, 1);
		break;
	case 10:
		::PostMessage(m_pOptionViewDlg->m_hWnd, WM_USER_UPDATEUI, 0x20, 0);

		break;
	}
	return 0;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	if (m_pCCDView&&SIZE_MINIMIZED != nType)
	{
	}

	// TODO:  在此处添加消息处理程序代码
}

void CMainFrame::ModifyCamView(BYTE nCameraID)
{
	CRect ClientRc;
	BOOL bLive;
	BYTE nCount = _mm_popcnt_u32(nCameraID);
	if (!nCount||(nCameraID>>4))
		return;
	m_pCCDView->GetClientRect(ClientRc);
// 	theApp.m_NiVision.SetHandleWnd(m_pCCDView);
	ClientRc.left = ClientRc.right - (ClientRc.right - ClientRc.left) / nCount;
	//先关闭释放显示区域
	if (nCount < 4)
	{
		for (int i = 0; i < 4; i++)
		{
			bLive = (nCameraID >> i) & 0x01;
			if (!bLive)
			{
				theApp.m_NiVision.EnableUpdateDisplay(theApp.m_nCameraID[i], FALSE);
// 				theApp.m_NiVision.StartImageMonitor(theApp.m_nCameraID[i], FALSE);
				theApp.m_NiVision.Camera_Display(theApp.m_nCameraID[i], FALSE);
			}
		}
	}
	//再打开
	if (nCount)
	{
		for (int i = 0; i < 4; i++)
		{
			bLive = (nCameraID >> i) & 0x01;
			if (bLive)
			{
				BYTE nWnd = theApp.m_NiVision.m_nDisplayBinder[theApp.m_nCameraID[i]];
				theApp.m_NiVision.ResizeDisplayWnd(nWnd, ClientRc);
				theApp.m_NiVision.EnableUpdateDisplay(theApp.m_nCameraID[i], TRUE);
				ClientRc.OffsetRect(-ClientRc.Width(), 0);
				theApp.m_NiVision.StartImageMonitor(theApp.m_nCameraID[i], TRUE);
			}
		}
	}
}

void CMainFrame::OnViewTool()
{
	// TODO:  在此添加命令处理程序代码
	static BOOL b(1);
	b = !b;
	ShowControlBar(&m_wndDlgBar, b, 0);
}

void CMainFrame::OnOperationMode(UINT idCtl)
{
	switch (idCtl)
	{
	case ID_BUTTON_HOME:
		if (!(theApp.m_nCurrentRunMode >> 1))
		{
			theApp.ShowHomeProcessDlg();
			if (!theApp.StartReturnHome())
				PostMessage(WM_USER_HOMED, (WPARAM)0x03, -1);
		}
		break;
	case ID_BUTTON_AUTO:
		if (theApp.m_nIsHomeEnd&&!(theApp.m_nCurrentRunMode >> 1))
		{
			if (theApp.StartAuto(TRUE))
			{
				theApp.m_bOnline = TRUE;
				theApp.m_nCurrentRunMode = 0x04 + (theApp.m_nCurrentRunMode & 0x01);
			}
		}
		break;
	case ID_BUTTON_LATEST:
		if (!(theApp.m_nCurrentRunMode >> 1))
		{
			theApp.LaserTestOut();
			TRACE0("test\n");
		}
		break;
	default:break;
	}
}

void CMainFrame::OnCommandStop()
{
	theApp.StartAuto(FALSE);
}

void CMainFrame::OnButtonEmg()
{
	// TODO:  在此添加命令处理程序代码
	theApp.SetStop(TRUE, TRUE);
}

void CMainFrame::ResetSts()
{
	theApp.ResetSts();
}


void CMainFrame::StopCommand(BOOL bEmg /*= FALSE*/)
{
	if (theApp.m_bHomeThreadAlive || bEmg)
	{
		theApp.m_GgMotion.SetEStop(TRUE);
	}
	theApp.StartAuto(FALSE);
}

LRESULT CMainFrame::OnIochange(WPARAM wParam, LPARAM lParam)
{
	ULONG nCount(0);
	UINT nChange = wParam;
	UINT nID = lParam;
	BOOL bDriverAlarm = theApp.m_GgMotion.m_nAlarm & 0x03;
	theApp.m_GgMotion.GetInputCounter(theApp.m_cParam.In_EmgStop + 1, nCount);
	if (nCount)
	{
		theApp.m_GgMotion.EnableInputCounter(theApp.m_cParam.In_EmgStop + 1);
		if (theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_EmgStop])
		{
// 			theApp.SetStop(TRUE,TRUE);
			theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
			AddedProcessMsg(_T("触发急停，输出报警\n"));
		}
		else
		{
			theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, FALSE);
			AddedProcessMsg(_T("急停复位，清除报警\n"));
		}
	}
	if (bDriverAlarm || (theApp.m_bAutoThreadAlive&& theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_ExtStop]))
	{
// 		theApp.SetStop(TRUE, bDriverAlarm);
		if (bDriverAlarm)
		{
			theApp.AddedErrorMsg(_T("驱动器警报\n(报警解除后请回零复位，如断电请重启程序！\r\n)"));
		}

	}
	else if (theApp.m_bIsNeedStop==0)
	{
		if (theApp.m_bOnline)//外控有效
		{
			if (theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_ExtStop])
			{
				theApp.m_GgMotion.GetInputCounter(theApp.m_cParam.In_ExtStop + 1, nCount);
				if (nCount)
				{
					theApp.m_GgMotion.EnableInputCounter(theApp.m_cParam.In_ExtStop + 1);
					theApp.SetStop(TRUE);
					AddedProcessMsg(_T("外部触发停止\n"));
				}
			}
			else
			{
				theApp.StartAuto(TRUE);
				if (theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_ExtStart])
				{
					theApp.m_GgMotion.GetInputCounter(theApp.m_cParam.In_ExtStart + 1, nCount);
					if (nCount)
					{
						if (theApp.m_bWaitforTrigger)
						{
							theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtCircleDone + 1, FALSE);
							PulseEvent(theApp.m_hOneCycleStart);
						}
						theApp.m_GgMotion.EnableInputCounter(theApp.m_cParam.In_ExtStart + 1);
					}
				}
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnPositionChange(WPARAM wParam, LPARAM lParam)
{
	if (m_pMotionDlg && m_pMotionDlg->GetSafeHwnd())
		::PostMessage(m_pMotionDlg->GetSafeHwnd(), WM_USER_UPDATEUI, 2, 0);
	BOOL bDriverAlarm = theApp.m_GgMotion.m_nAlarm & 0x03;
	if (bDriverAlarm)
	{
		theApp.SetStop(TRUE, TRUE);
		theApp.AddedErrorMsg(_T("驱动器警报\n(报警解除后请回零复位，如断电请重启程序！\r\n)"));
	}
	return 0;
}

LRESULT CMainFrame::OnHomed(WPARAM wParam, LPARAM lParam)
{
	return theApp.OnHomed(wParam, lParam);
}

void CMainFrame::ResetProcess()
{
	theApp.ResetProcess();
}

LRESULT CMainFrame::OnShowErrorMsg(WPARAM wParam, LPARAM lParam)
{
	return theApp.CreateErrorDlg();
}

BOOL CMainFrame::LightControllerOpen(BOOL bOpen /*= TRUE*/)
{
	BOOL bSuccess(!bOpen);
	int i(0);
	if (bOpen)
	{
		if (!m_LightControl.IsOpen())
		{
			if (m_LightControl.InitPort(m_hWnd, theApp.m_cParam.nComPort[0], 19200, 'N', 8, 1, EV_RXFLAG | EV_RXCHAR))
			{
				bSuccess = m_LightControl.StartMonitoring();
			}
			// 			else
			// 				ShowPopup(_T("光源控制端口打开失败！"));
		}
		else
			bSuccess = TRUE;
	}
	else if (m_LightControl.IsOpen())
		m_LightControl.ClosePort();
	return bSuccess;
}

BOOL CMainFrame::LightControllerSet(BYTE nID, BYTE nValue, BYTE nMode /*= 0*/)
{
	int i(0);
	int hexlenth(0);
	BYTE sum(0);
	CHAR str[2];
	CStringA szCmdHead;
	CStringA szCmdCode;
	CStringA szMode;
	CStringA result = "";
	switch (nMode)
	{
	case 0:
		if (nValue > 1)
			nValue = 0xff;
		szCmdCode.Format("%1d", nValue);
		szMode = "L";

		break;
	case 1:
		if (nValue > 10)
			nValue = 0;
		szMode = "S";

		szCmdCode.Format("%02d", nValue);
		break;
	case 2:
		if (nValue > 255)
			nValue = 255;

		szCmdCode.Format("%03d", nValue);
		szMode = "F";
		break;
	default:
		return 0;
	}
	szCmdHead.Format("@%02d%s%s00", nID, szMode, szCmdCode);
	hexlenth = szCmdHead.GetLength();
	for (i = 0; i < hexlenth; i++)
	{
		int converted = 0;
		str[0] = szCmdHead.GetAt(i);
		long lhex = str[0];
		sum += lhex;
	}
	result.Format("%02X", sum);
	szCmdHead += result + '\r' + '\n';
	if (m_LightControl.IsOpen())
		m_LightControl.WriteToPort(szCmdHead);
	return 1;
}

BOOL CMainFrame::ShowWaiteProcessDlg(BOOL bShow /*= TRUE*/)
{
	static BYTE nCount(0);
	if (bShow){
		if (m_pWaiteProcessDlg == NULL)
			m_pWaiteProcessDlg = new CWaiteProcessDlg;
		if (m_pWaiteProcessDlg &&  m_pWaiteProcessDlg->GetSafeHwnd() == NULL)
			m_pWaiteProcessDlg->Create(CWaiteProcessDlg::IDD, theApp.m_pMainWnd);
		m_pWaiteProcessDlg->ShowWindow(SW_RESTORE);
		nCount++;
	}
	else if (m_pWaiteProcessDlg != NULL)
	{
		if (nCount)
			nCount--;
		if (!nCount)
		{
			SAFE_DELETEDLG(m_pWaiteProcessDlg);
		}
	}
	return nCount;
}

BOOL CMainFrame::StartTemplateRW(BOOL bLoad /*= TRUE*/, BYTE nMode /*= 0*/)
{
	BOOL bSuccess(TRUE);
	CWinThread* pThread(NULL);
	pLuckyBag pOperateOrder = new LuckyBag;
	pOperateOrder->pOwner = this;
	pOperateOrder->nIndex = new int[2];
	pOperateOrder->nIndex[0] = bLoad;
	pOperateOrder->nIndex[1] = nMode;
	if (!m_bFileThreadAlive)
		ShowWaiteProcessDlg(TRUE);

	if (m_bFileThreadAlive || !(pThread = AfxBeginThread(theApp.FileRWProcess, (LPVOID)pOperateOrder)))
	{
		bSuccess = FALSE;
		SAFE_DELETE(pOperateOrder->nIndex);
		SAFE_DELETE(pOperateOrder);
	}
	return bSuccess;
}

void CMainFrame::UpdateWaiteProcessDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	if (m_pWaiteProcessDlg&&m_pWaiteProcessDlg->GetSafeHwnd())
		::PostMessage(m_pWaiteProcessDlg->m_hWnd, WM_USER_SHOWPROCESS, wParam, lParam);
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:
		KillTimer(nIDEvent);
		break;
	}
	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnViewIODlg()
{
	theApp.OnViewIODlg();
}

void CMainFrame::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1));

}

void CMainFrame::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.GetUserType() && !(theApp.m_nCurrentRunMode>>1)&&!m_bFileThreadAlive);
}

void CMainFrame::OnUpdateCommand(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
	UINT nSts(FALSE);
	BYTE nUser = theApp.GetUserType();
	switch (pCmdUI->m_nID)
	{
	case ID_PROJECT_CONFIGURATION:
	case ID_PROJECT_NEW:
		pCmdUI->Enable(nUser&&!(theApp.m_nCurrentRunMode >> 1) && !theApp.m_bSalveThreadAlive);
		break;
	case ID_BUTTON_HOME:
		pCmdUI->SetCheck(theApp.m_bHomeThreadAlive);
		pCmdUI->Enable(!(theApp.m_nCurrentRunMode >> 1) && !theApp.m_bSalveThreadAlive);
		break;
	case ID_BUTTON_AUTO:
		pCmdUI->SetCheck(theApp.m_bAutoThreadAlive);
		pCmdUI->Enable(theApp.m_nIsHomeEnd&&!(theApp.m_nCurrentRunMode >> 1) && !theApp.m_bSalveThreadAlive);
		break;
	case ID_BUTTON_LATEST:
		pCmdUI->SetCheck(theApp.m_nCurrentRunMode &0x010);
		pCmdUI->Enable(!theApp.m_bIdling&&nUser && !(theApp.m_nCurrentRunMode >> 1) && !theApp.m_bSalveThreadAlive);
		break;
	case ID_VIEW_IO:
		pCmdUI->Enable(nUser);
		break;
	case ID_RESET_STATUS:
	case ID_RESET_PROCESS:
		pCmdUI->Enable(!(theApp.m_nCurrentRunMode >> 1));
		break;
	case ID_MENUBUTTON_USER:
	case ID_USER_LOGIN:
		break;
	case ID_USER_LOGOUT:
		pCmdUI->Enable(nUser);
		break;
	default:break;
	}
}

void CMainFrame::AddedProcessMsg(CString Info, BOOL bClear /*= FALSE*/)
{
	m_pCCDView->AddedStatusMsg(Info, bClear);
}

LRESULT CMainFrame::OnUpdateDisplay(WPARAM wParam, LPARAM lParam)
{
	m_pCCDView->PostMessageW(WM_USER_SHOWIMAGE,0,0);
	return 0;
}

void CMainFrame::OnViewDxfDlg()
{
	if (m_pDxfDlg == NULL)
	{
		m_pDxfDlg = new DxfDlg;
	}
	if (m_pDxfDlg->GetSafeHwnd() == NULL)
	{
		m_pDxfDlg->Create(DxfDlg::IDD, this);
	}
	m_pDxfDlg->ShowWindow(SW_RESTORE);
	m_pDxfDlg->SetTimer(1, 500, NULL);

	::PostMessage(m_pDxfDlg->m_hWnd, WM_USER_UPDATEUI,0,0);
}

