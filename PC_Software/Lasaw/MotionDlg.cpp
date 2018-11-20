// MotionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "MotionDlg.h"
#include "MainFrm.h"


// CMotionDlg 对话框

IMPLEMENT_DYNCREATE(CMotionDlg, CFormView)

CMotionDlg::CMotionDlg(CWnd* pParent /*=NULL*/)
: CFormView(CMotionDlg::IDD)
{
	m_nCurrentCheck = 0;
	bMove = 0;
	m_nCurrentAxis = 0;
	m_bInching = TRUE;
	m_fPace = 1;
}

CMotionDlg::~CMotionDlg()
{
}

void CMotionDlg::DoDataExchange(CDataExchange* pDX)
{
	int i = 0;
	CFormView::DoDataExchange(pDX);
	for (i = 0; i < 2; i++)
		DDX_Control(pDX, IDC_STATIC_ACTUAL_X + i, m_lStatus[i]);
	DDX_Control(pDX, IDC_STATIC_TARGET, m_lStatus[3]);
	DDX_Control(pDX, IDC_COMBO_POSITION_TYPE, m_cPositionChs);
	DDX_Control(pDX, IDC_COMBO_SPACING, m_cPaceChs);
	DDX_Control(pDX, IDC_COMBO_PRODUCT_CHECK, m_cVisionOrderChs);
}


BEGIN_MESSAGE_MAP(CMotionDlg, CFormView)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_MOVE_XL, IDC_BUTTON_MOVE_YD, OnBnClickedMove)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_GOPOSITION, IDC_BUTTON_SAVEPOSITION, OnBnClickedButtonGetPosition)
	ON_BN_CLICKED(IDC_CHECK_INCHING, &CMotionDlg::OnBnClickedCheckInching)
	ON_BN_CLICKED(IDC_BUTTON_MOV_STOP, &CMotionDlg::OnBnClickedButtonMovStop)
	ON_CBN_KILLFOCUS(IDC_COMBO_SPACING, &CMotionDlg::OnCbnKillfocusComboPace)
	ON_CBN_SELCHANGE(IDC_COMBO_POSITION_TYPE, &CMotionDlg::OnCbnSelchangePositionType)
	ON_CBN_SELCHANGE(IDC_COMBO_PRODUCT_CHECK, &CMotionDlg::OnCbnSelchangeCombo_VisionCheckOrder)
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_USER_UPDATEUI, &CMotionDlg::OnUpdateDlg)
	ON_BN_CLICKED(IDC_BUTTON_MAIN, &CMotionDlg::OnBnClickedButtonMain)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMotionDlg 消息处理程序

void  CMotionDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	m_cPaceChs.SetCurSel(6);
	m_cVisionOrderChs.SetCurSel(0);
	m_cPositionChs.SetCurSel(0);
	((CButton*)GetDlgItem(IDC_CHECK_INCHING))->SetCheck(m_bInching);
	m_lStatus[3].SetBkColor(CLasawApp::GreenYellow);
	return;
}

void CMotionDlg::OnBnClickedMove(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int nDir(0);
	if (!m_bInching)
		return;
	switch (idCtl)
	{
	case IDC_BUTTON_MOVE_XR:
		nDir = 1;
	case IDC_BUTTON_MOVE_XL:
		m_nCurrentAxis = 0;

		break;
	case IDC_BUTTON_MOVE_YU:
		nDir = 1;
	case IDC_BUTTON_MOVE_YD:
		m_nCurrentAxis = 1;
		break;
// 	case IDC_BUTTON_MOVE_ZD:
// 		nDir = 1;
// 	case IDC_BUTTON_MOVE_ZU:
// 		m_nCurrentAxis = 2;
// 		break;
	default:
		return;
	}
	if (!theApp.m_GgMotion.m_bAxisMotionDone[m_nCurrentAxis])
	{
		theApp.m_GgMotion.AxStop();
		return;
	}
	double dvel = theApp.m_cParam.Run_VEL[0];
	double dAcc = theApp.m_cParam.Run_ACC[0];

	if (m_bInching)
		theApp.m_GgMotion.AxMovePtp(m_nCurrentAxis + 1, dvel, dAcc, ((nDir << 1) - 1)*m_fPace, FALSE);

}

void CMotionDlg::OnBnDownMove(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int nDir(0);
	if (m_bInching || bMove || theApp.m_bAutoThreadAlive || !theApp.m_nIsHomeEnd)
		return;
	static long c(0);
	CString str;
// 	bMove = 1;
// 	return;
// 
	switch (idCtl)
	{
	case IDC_BUTTON_MOVE_XR:
		nDir = 1;
	case IDC_BUTTON_MOVE_XL:
		m_nCurrentAxis = 0;

		break;
	case IDC_BUTTON_MOVE_YD:
		nDir = 1;
	case IDC_BUTTON_MOVE_YU:
		m_nCurrentAxis = 1;
		break;
// 	case IDC_BUTTON_MOVE_ZD:
// 		nDir = 1;
// 	case IDC_BUTTON_MOVE_ZU:
// 		m_nCurrentAxis = 2;
// 		break;
	default:
		return;
	}
	if (m_nCurrentAxis == 2 || !theApp.m_GgMotion.m_bAxisMotionDone[m_nCurrentAxis])
	{
		theApp.m_GgMotion.AxStop();
		return;
	}
	GetDlgItem(IDC_CHECK_INCHING)->EnableWindow(FALSE);

	double dvel = theApp.m_cParam.Run_VEL[0];
	double dAcc = theApp.m_cParam.Run_ACC[0];
	bMove = TRUE;
	m_lStatus[3].SetBkColor(CLasawApp::DarkBlue);
	if (!theApp.m_nIsHomeEnd)
	{
		dvel = 0.5;
		dAcc = 1;
	}
	theApp.m_GgMotion.AxMovePtp(m_nCurrentAxis + 1, dvel, dAcc, ((nDir << 1) - 1) * 1000);

}

void CMotionDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CFormView::OnLButtonUp(nFlags, point);
}

void CMotionDlg::OnBnClickedCheckInching()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CEdit* pEdit(NULL);
	m_bInching = !m_bInching;
	pEdit = (CEdit*)GetDlgItem(IDC_COMBO_SPACING);
	pEdit->EnableWindow(m_bInching);

}

void CMotionDlg::OnBnClickedButtonMovStop()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	SetEvent(theApp.m_hStopRun);
	theApp.m_GgMotion.AxStop();
	bMove = FALSE;
}

void CMotionDlg::OnCbnKillfocusComboPace()
{
	CString str;
	float fvalue = m_fPace;
	UpdateData();
	m_cPaceChs.GetWindowText(str);
	m_fPace = _tstof(str);
	if (m_fPace > 500.0 || m_fPace < 0.001)
	{
		m_fPace = fvalue;
		str.Format(_T("%f"), m_fPace);
		m_cPaceChs.SetWindowText(str);
		UpdateData(FALSE);
	}
}

LRESULT CMotionDlg::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString str;
	BOOL	bEnable(FALSE);
	UINT nOrder(0), nTotal(0);
	double* pdPos;

	UINT i(0), j(3);
	UINT nIndex = wParam;

	switch (nIndex)
	{
	case 0:
		bEnable =theApp.GetUserType() && !(theApp.m_nCurrentRunMode>>1)  ;
		for (i = 0; i < 4; i++)
			GetDlgItem(IDC_BUTTON_MOVE_XL+i)->EnableWindow(lParam&&bEnable);
		m_cPositionChs.EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_GOPOSITION)->EnableWindow(lParam&&bEnable&& theApp.m_nIsHomeEnd);
		j = m_cPositionChs.GetCurSel();
		m_cVisionOrderChs.EnableWindow(j&0x02 ? bEnable : FALSE);

		GetDlgItem(IDC_BUTTON_SAVEPOSITION)->EnableWindow(lParam&&bEnable&& theApp.m_nIsHomeEnd);

		break;
	case 1:
		m_nCurrentCheck = theApp.m_nCurrentPane[0];
		m_cVisionOrderChs.SetCurSel(m_nCurrentCheck);
		break;
	case 2:
	case 3://更新坐标
		for ( i = 0; i < 2; i++)
		{
			str = DOUBLE_STR07(theApp.m_GgMotion.m_dAxisCurPos[i]);
			m_lStatus[i].SetText(str);
		}

		break;
	case 4://项目参数更新
	case 5:
		j = m_cPositionChs.GetCurSel();
		switch (j)
		{
		case 0://装载位
			str.Format(_T("目标位置---->面板上下料位: \n X %8.3f, Y %8.3f"), theApp.m_cParam.PrjCfg.dLoadPosition[0], theApp.m_cParam.PrjCfg.dLoadPosition[1]);
			m_lStatus[3].SetText(str);
			break;
		case 1://校正出光
			str.Format(_T("目标位置---->激光校正试射位:\n X %8.3f, Y %8.3f"), theApp.m_cParam.PrjCfg.dCaliPosition[0],
				theApp.m_cParam.PrjCfg.dCaliPosition[1]);
			m_lStatus[3].SetText(str);
			break;
		case 2://A拍照采集
		case 3://B拍照采集
			bEnable = TRUE;
			if (j&0x01)
			{
				pdPos = theApp.m_cParam.ProductSnapShotB.at(m_nCurrentCheck).EndPoint;
				str.Format(_T("目标位置---->反面定位采集位：\n X %8.3f, Y %8.3f"), pdPos[0], pdPos[1]);
			}
			else
			{
				pdPos = theApp.m_cParam.ProductSnapShotF.at(m_nCurrentCheck).EndPoint;
				str.Format(_T("目标位置---->正面定位采集位：\n X %8.3f, Y %8.3f"), pdPos[0], pdPos[1]);
			}
			m_lStatus[3].SetText(str);
			break;
		default:
			break;
		}
		m_cVisionOrderChs.EnableWindow(bEnable);
	case 6:
	default:
		break;
	}
	return 0;
}
//Z Safe;Loading Area;Nozzle Check;Nozzle Clean;Cali Check;Cali Shot;Product Position;
void CMotionDlg::OnCbnSelchangePositionType()
{
	PostMessage(WM_USER_UPDATEUI, 5, 0);
}

void CMotionDlg::OnCbnSelchangeCombo_VisionCheckOrder()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
// 	CComboBox* pCombox;
// 	pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_PRODUCT_CHECK);
	m_nCurrentCheck =m_cVisionOrderChs.GetCurSel();
	PostMessage(WM_USER_UPDATEUI, 5, 0);
}

//Z Safe;Loading Area;Cali Check;Cali Shot;Product Position;
void CMotionDlg::OnBnClickedButtonGetPosition(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (!theApp.m_nIsHomeEnd || theApp.m_bAutoThreadAlive)
		return;
	BOOL	bSave(TRUE);
	UINT	nOrder(0), nTotal(0);
	int i(0);
	double* pdPos;
	bSave = idCtl - IDC_BUTTON_GOPOSITION;
	UINT nIndex = m_cPositionChs.GetCurSel();
	if (bSave)
	{
		if (IDYES == MessageBox(_T("确定更新当前位置坐标吗?（即刻生效）"), _T("重要提示"), MB_YESNO))
		{
			switch (nIndex)
			{
			case 0://loading
				theApp.m_cParam.PrjCfg.dLoadPosition[0] = theApp.m_GgMotion.m_dAxisCurPos[0];
				theApp.m_cParam.PrjCfg.dLoadPosition[1] = theApp.m_GgMotion.m_dAxisCurPos[1];
				break;
			case 1://Try Check
				theApp.m_cParam.PrjCfg.dCaliPosition[0] = theApp.m_GgMotion.m_dAxisCurPos[0];
				theApp.m_cParam.PrjCfg.dCaliPosition[1] = theApp.m_GgMotion.m_dAxisCurPos[1];
				break;
			case 2://A Product Position
			case 3://B Product Position
				if (nIndex&0x01)
					pdPos = theApp.m_cParam.ProductSnapShotB.at(m_nCurrentCheck).EndPoint;
				else
					pdPos = theApp.m_cParam.ProductSnapShotF.at(m_nCurrentCheck).EndPoint;
				pdPos[0] = theApp.m_GgMotion.m_dAxisCurPos[0];
				pdPos[1] = theApp.m_GgMotion.m_dAxisCurPos[1];
				break;
			default:
				break;
			}
			PostMessage(WM_USER_UPDATEUI, 5, 0);
		}
	}
	else if (theApp.m_GgMotion.m_bCrdDone[0]){
		if (IDYES == MessageBox(_T("确定移动至该位置吗?"), _T("重要提示"), MB_YESNO))
		{
			PostMessage(WM_USER_UPDATEUI, 0, 0);
			switch (nIndex)
			{
			case 2://A Product Position
			case 3://B Product Position
				theApp.m_nPanePosture = nIndex & 0x01;
				theApp.m_nCurrentPane[0] = m_nCurrentCheck;
				theApp.GoPosition(CLasawApp::SNAPSHOT_POSITION, FALSE);
				break;
			case 0://loading area
				theApp.GoPosition(CLasawApp::RELOAD_POSITION, FALSE);
				break;
			case 1://Try Shot
				theApp.GoPosition(CLasawApp::LASER_CALI_POSITION, FALSE);
			default:break;
			}
			SetTimer(1, 1000, NULL);
		}
	}
}

BOOL CMotionDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMsg->message == WM_LBUTTONDOWN)
	{
		int buID;
		buID = GetWindowLong(pMsg->hwnd, GWL_ID);
		if (buID&&!m_bInching)
			OnBnDownMove(buID);
	}
	else if (pMsg->message == WM_LBUTTONUP&&bMove)
	{
		theApp.m_GgMotion.AxStop();
		GetDlgItem(IDC_CHECK_INCHING)->EnableWindow(TRUE);
		bMove = FALSE;
		m_lStatus[3].SetBkColor(CLasawApp::GreenYellow);

	}

	return CFormView::PreTranslateMessage(pMsg);
}



void CMotionDlg::OnBnClickedButtonMain()
{
	// TODO:  在此添加控件通知处理程序代码
	::PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_USER_UPDATEUI, 5, 0);
// 	PostMessage(WM_USER_UPDATEUI, 0, 1);

}

void CMotionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:
		if (theApp.m_GgMotion.m_bCrdDone[0])
		{
			PostMessage(WM_USER_UPDATEUI, 0, 1);

			KillTimer(nIDEvent);
		}
		break;
	default:
		break;
	}
	CFormView::OnTimer(nIDEvent);
}
