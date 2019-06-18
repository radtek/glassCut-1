// ProjectSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "ProjectSettingDlg.h"
#include "MainFrm.h"

// CProjectSettingDlg 对话框

IMPLEMENT_DYNAMIC(CProjectSettingDlg, CDialogEx)

CProjectSettingDlg::CProjectSettingDlg(CWnd* pParent)
: CDialogEx(CProjectSettingDlg::IDD)
{
	int i(0);
	m_bOnCreatNew = FALSE;
	m_nCurrentPrjSel = 0;
// 	ZeroMemory(m_dHomeSpeed, sizeof(m_dHomeSpeed));
	ZeroMemory(m_nLaserParam, sizeof(m_nLaserParam));
	ZeroMemory(m_dRunSpeed, sizeof(m_dRunSpeed));
	bFront = 0;
}

CProjectSettingDlg::~CProjectSettingDlg()
{
}

void CProjectSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	int i(0);
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NEWPROJECT, m_MaskName);
	DDX_Control(pDX, IDC_COMBO_PRJNAME, m_cbPrjName);
	DDX_Control(pDX, IDC_BUTTON_NEW, m_btnCreate);
	DDX_Control(pDX, IDC_BUTTON_RELOAD, m_btnLoad);
	DDX_Text(pDX, IDC_EDIT_PPI, m_nLaserParam[0]);
	DDX_Text(pDX, IDC_EDIT_FREQ, m_nLaserParam[1]);
	DDX_Text(pDX, IDC_EDIT_POWER, m_nLaserParam[2]);
	for (i = 0; i < 2; i++)
	{
		DDX_Text(pDX, IDC_EDIT_MU_VEL + 2 * i, m_dRunSpeed[i][0]);
		DDX_Text(pDX, IDC_EDIT_MU_VEL + 2 * i + 1, m_dRunSpeed[i][1]);
	}
	DDX_Radio(pDX, IDC_RADIO_FRONT, bFront);

}


BEGIN_MESSAGE_MAP(CProjectSettingDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_DELETE, &CProjectSettingDlg::OnPopDelete)
	ON_COMMAND(ID_EDIT_NEW, &CProjectSettingDlg::OnPopNew)
	ON_COMMAND(ID_CMD_MOVETO, &CProjectSettingDlg::OnPopMoveto)
	ON_COMMAND(ID_BTN_GETUPDATE, &CProjectSettingDlg::OnPopUpdatePos)
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_PPI, IDC_EDIT_POWER, OnKillFocusProductEdit)
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_MU_VEL, IDC_EDIT_AU_ACVEL, OnKillFocusEditVel)
	ON_BN_CLICKED(IDC_BUTTON_NEW, &CProjectSettingDlg::OnClickedBtnCreate)
	ON_CBN_SELCHANGE(IDC_COMBO_PRJNAME, &CProjectSettingDlg::OnCbnSelchangeComboPrjname)

	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_FRONT, IDC_RADIO_BACK, OnBnChangePosture)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD, &CProjectSettingDlg::OnBnClickedButtonReload)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CProjectSettingDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CProjectSettingDlg::OnBnClickedButtonApply2)
	ON_BN_CLICKED(IDC_BUTTON_LASERTEST, &CProjectSettingDlg::OnBnClickedButtonLasertest)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CProjectSettingDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CProjectSettingDlg::OnBnClickedButtonCancel)
	ON_MESSAGE(WM_USER_UPDATEUI, &CProjectSettingDlg::OnUpdateDlg)
	ON_BN_CLICKED(IDC_BUTTON_TURNOVER, &CProjectSettingDlg::OnBnClickedButtonTurnover)
	ON_COMMAND(ID_BTN_GETCENTER, &CProjectSettingDlg::OnBtnGetCenter)
	ON_UPDATE_COMMAND_UI(ID_BTN_GETCENTER, &CProjectSettingDlg::OnUpdateBtnGetCenter)
	ON_UPDATE_COMMAND_UI(ID_BTN_GETUPDATE, &CProjectSettingDlg::OnUpdateBtnGetUpdate)
	ON_UPDATE_COMMAND_UI(ID_CMD_MOVETO, &CProjectSettingDlg::OnUpdateBtnGetUpdate)
END_MESSAGE_MAP()


// CProjectSettingDlg 消息处理程序


BOOL CProjectSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int i(0);
	CString szNew = theApp.m_cParam.DefaultProjectName.Left(2);
	m_MaskName.EnableMask(_T("DD-AAAAAAAA"), _T("__-________"), _T('0'));
	m_MaskName.SetValidChars(_T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
	m_MaskName.EnableGetMaskedCharsOnly(FALSE);
	m_MaskName.EnableSelectByGroup();
	i = STR_INT(szNew);
	szNew.Format(_T("%02d-ADDEDNEW"), i + 1);
	m_MaskName.SetWindowText(szNew);

	POSITION pos = theApp.m_cParam.m_cListPrjName.GetHeadPosition();
	for (i = 0; (pos != NULL) && (i < theApp.m_cParam.m_cListPrjName.GetCount()); i++)
	{
		szNew = theApp.m_cParam.m_cListPrjName.GetNext(pos);
		int nItem = m_cbPrjName.AddString(szNew);
		if ((nItem != CB_ERR) && (nItem != CB_ERRSPACE) && (szNew == theApp.m_cParam.DefaultProjectName))
		{
			m_cbPrjName.SetCurSel(nItem);
			m_nCurrentPrjSel = nItem;
		}
	}
	PostMessage(WM_USER_UPDATEUI, 1, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void CProjectSettingDlg::OnCbnSelchangeComboPrjname()
{
	// TODO:  在此添加控件通知处理程序代码
	m_nCurrentPrjSel = m_cbPrjName.GetCurSel();
}

void CProjectSettingDlg::OnClickedBtnCreate()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString szNew;
	m_btnCreate.GetWindowText(szNew);
	if (szNew == _T("添加"))
	{
		m_MaskName.GetWindowText(szNew);
		if (szNew.TrimRight().IsEmpty())
		{
			AfxMessageBox(_T("项目名称不能为空!\r\n"));
			return;
		}
		szNew.TrimLeft();
		if (CB_ERR != m_cbPrjName.FindString(-1, szNew))
			AfxMessageBox(_T("项目名称已经存在!\r\n"));
		else
		{
			int nItem = m_cbPrjName.AddString(szNew);
			if ((nItem != CB_ERR) && (nItem != CB_ERRSPACE))
			{
				m_cbPrjName.SetCurSel(nItem);
				m_nCurrentPrjSel = nItem;
				if (!theApp.m_cParam.CreateNewPrj(szNew))
				{
					AfxMessageBox(_T("此项目文件已经存在!\r\n"));
				}else
					AfxMessageBox(_T("添加成功!\r\n"));

				PostMessage(WM_USER_UPDATEUI, 6, 0);
				::PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_USER_UPDATEUI, 0, 1);

			}
		}
	}
	else //modify
	{
		CString szold;
		m_cbPrjName.GetLBText(m_nCurrentPrjSel, szold);
		if (m_cbPrjName.IsWindowVisible())
		{
			m_cbPrjName.ShowWindow(SW_HIDE);
			m_MaskName.SetWindowText(szold);
			m_MaskName.ShowWindow(SW_SHOW);
		}
		else
		{
			m_MaskName.GetWindowText(szNew);
			int nIndex = m_cbPrjName.FindString(-1, szNew);
			if (CB_ERR != nIndex)
				AfxMessageBox(_T("项目名称已经存在!\r\n"));
			else
			{
				if (!theApp.m_cParam.PrjRename(szold, szNew))
				{
					AfxMessageBox(_T("存在同名文件，或未找到旧项目文件 !\r\n"));
					return;
				}
				m_cbPrjName.DeleteString(m_nCurrentPrjSel);
				m_cbPrjName.InsertString(m_nCurrentPrjSel, szNew);
				::PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_USER_UPDATEUI, 0, 0);

			}
		}
	}
}

void CProjectSettingDlg::OnKillFocusProductEdit(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int i(0);
	UINT	nNum(0);
	CString Tip;
	UpdateData();
	switch (idCtl)
	{
	case IDC_EDIT_PPI:break;

	case IDC_EDIT_FREQ:
		if (m_nLaserParam[1] > 90)
		{
			m_nLaserParam[1] = 96;
			UpdateData(FALSE);
		}
		break;
	case IDC_EDIT_POWER:
		if (m_nLaserParam[2] > 99.99)
		{
			m_nLaserParam[2] = 100;
			UpdateData(FALSE);
		}
		break;
	}

}

void CProjectSettingDlg::OnKillFocusOthersEdit(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	UINT	nNum(0);
	CString Tip;
}

LRESULT CProjectSettingDlg::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int i(0);
	UINT nIndex = wParam;
	CStatic* pStatic = NULL;
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);
	switch (nIndex)
	{
	case 0:
		m_wndLaserTrack.EnableWindow(bEnable);
		m_cbPrjName.EnableWindow(bEnable);
		m_btnCreate.EnableWindow(bEnable);
		m_btnLoad.EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_NEWPROJECT)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_MU_VEL)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_MU_ACVEL)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_AU_VEL)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_AU_ACVEL)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_TURNOVER)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_POWER)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_PPI)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_LASERTEST)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_APPLY)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_CANCEL)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_UPDATE)->EnableWindow(bEnable);
		break;
	case 1:
	case 2:
		///******更新界面数据******///
		m_nLaserParam[0] = theApp.m_cParam.PrjCfg.fLaser2VelRatio;
		m_nLaserParam[1] = theApp.m_cParam.PrjCfg.fPWMfrequency;
		m_nLaserParam[2] = theApp.m_cParam.PrjCfg.fPower;
		for (i = 0; i < 2; i++)
		{
			m_dRunSpeed[i][0] = theApp.m_cParam.Run_VEL[i];
			m_dRunSpeed[i][1] = theApp.m_cParam.Run_ACC[i];
		}
		UpdateData(FALSE);

	case 3:
		if (lParam)
			break;
	case 4:
		UpdateTaskList(lParam);
		break;
	case 5:

		break;
	case 6:
		if (lParam)
		{
			m_btnCreate.SetWindowText(_T("添加"));
			m_cbPrjName.ShowWindow(SW_HIDE);
			m_btnLoad.EnableWindow(FALSE);
			m_MaskName.ShowWindow(SW_SHOW);
		}
		else
		{
			m_btnCreate.SetWindowText(_T("修改名称"));
			m_cbPrjName.ShowWindow(SW_SHOW);
			m_MaskName.ShowWindow(SW_HIDE);
			m_btnLoad.EnableWindow(TRUE);

		}
		break;
	default:
		break;
	}

	return 0;
}

void CProjectSettingDlg::OnBnClickedButtonReload()
{
		// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int i(0);
	CString prjname;
	m_cbPrjName.GetLBText(m_nCurrentPrjSel, prjname);
	if (theApp.m_cParam.LoadPrjData(prjname))
	{
		pFrame->StartTemplateRW(TRUE,1);
		::PostMessage(pFrame->GetSafeHwnd(), WM_USER_UPDATEUI, 0, 1);
	}
}

void CProjectSettingDlg::OnBnClickedButtonApply2()
{
	// TODO:  在此添加控件通知处理程序代码
	CString Tip = _T("确认修改参数吗？\n");
	if (IDYES == MessageBox(Tip, _T("提示"), MB_YESNO))
	{
		UpdateData();
		theApp.m_cParam.PrjCfg.fLaser2VelRatio= m_nLaserParam[0] ;
		theApp.m_cParam.PrjCfg.fPWMfrequency = m_nLaserParam[1];
		theApp.m_cParam.PrjCfg.fPower = m_nLaserParam[2];
		OnBnClickedButtonApply();
	}
	else
	{
		m_nLaserParam[0] = theApp.m_cParam.PrjCfg.fLaser2VelRatio;
		m_nLaserParam[1] = theApp.m_cParam.PrjCfg.fPWMfrequency;
		m_nLaserParam[2] = theApp.m_cParam.PrjCfg.fPower;
		UpdateData(FALSE);
	}
}

void CProjectSettingDlg::UpdateTaskList(BOOL bSaveAndValidate /*= FALSE*/)
{
	BYTE nIndex(0);
	int nflag(0);
	UINT i(0), j(0);
	UINT nNum(0);
	CString szNo;
	if (!bSaveAndValidate)
	{
		m_wndLaserTrack.RemoveAll();
		if (theApp.m_cParam.PtArray.Ctline.size())
		{
			CTrackLine &tl = theApp.m_cParam.PtArray.Ctline.at(bFront);
			for (i = 0; i < tl.Tlline.size(); i++)
				m_wndLaserTrack.New(tl.Tlline.at(i));
		}
		m_wndLaserTrack.AdjustLayout();
	}
	else
	{
		int nCount = m_wndLaserTrack.GetRowCount();
		if (!theApp.m_cParam.PtArray.nCount)
		{
			theApp.m_cParam.PtArray.Ctline.resize(2);
			theApp.m_cParam.PtArray.nCount = 2;
		}
		if (!nCount)
			return;
		CTrackLine &tl = theApp.m_cParam.PtArray.Ctline.at(bFront);
		tl.Tlline.resize(nCount);
		tl.nCount = nCount;
		for (; nCount != 0; nCount--)
		{
			LPnode& lpn = tl.Tlline.at(nCount - 1);
			CBCGPGridRow* pRow = m_wndLaserTrack.GetRow(nCount - 1);
			lpn.bLaserOn = pRow->GetItem(0)->GetValue() ? TRUE : FALSE;
			szNo = pRow->GetItem(1)->GetValue();
			lpn.uType = szNo == _T("弧线") ? 2 :  1;

			for (j = 0; j < 2; j++)
				lpn.EndPoint[j] = (double)pRow->GetItem(j + 2)->GetValue();

			szNo = pRow->GetItem(4)->GetValue();
			lpn.nDir = szNo == _T("顺时针") ? 0 : 1;

			for (j = 0; j < 2; j++)
				lpn.CenterPt[j] = pRow->GetItem(j + 5)->GetValue();
			lpn.Vel = theApp.m_cParam.Run_VEL[lpn.bLaserOn];
			lpn.ACC = theApp.m_cParam.Run_ACC[lpn.bLaserOn];
		}

	}
}

void CProjectSettingDlg::OnButtonClear()
{

}

void CProjectSettingDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO:  在此处添加消息处理程序代码
	CPoint pt = point;
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);
	if (!bEnable)
		return;
	if (pWnd->GetSafeHwnd() == m_wndLaserTrack.GetSafeHwnd())
	{
		m_wndLaserTrack.ScreenToClient(&pt);
		CMenu menu;
		menu.LoadMenuW(IDR_CONTEXT_POPMUNE);

		CMenu *pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
	}

}

void CProjectSettingDlg::OnPopUpdateOffset()
{
	// TODO:  在此添加命令处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	UINT nTotal = 0;
	CString Tip = _T("确认更新Pin补偿位置 ？务必在设定定位模板后的状态下使用此命令");
}


void CProjectSettingDlg::OnPopDelete()
{
	// TODO:  在此添加命令处理程序代码
	m_wndLaserTrack.Delete();
}


void CProjectSettingDlg::OnPopNew()
{
	// TODO:  在此添加命令处理程序代码
	LPnode lpn;
	int i(0);
	ZeroMemory(&lpn, sizeof(LPnode));
	for (i = 0; i < 2; i++)
		lpn.EndPoint[i] = theApp.m_GgMotion.m_dAxisCurPos[i];
	lpn.uType = 1;
	lpn.Vel = theApp.m_cParam.Run_VEL[0];
	lpn.ACC = theApp.m_cParam.Run_ACC[0];

	m_wndLaserTrack.New(lpn);
// 	::PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_USER_UPDATEUI, 0, 1);
}


void CProjectSettingDlg::OnPopMoveto()
{
	// TODO:  在此添加命令处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CBCGPGridItem* pSelcItem = m_wndLaserTrack.GetCurSelItem();
	CBCGPGridRow* pRow = m_wndLaserTrack.GetCurSel();
	int nIndex = pSelcItem->GetColumnId();
	TrackPoint tempPoint;
	ZeroMemory(&tempPoint, sizeof(TrackPoint));
	for (BYTE i = 0; i < 2; i++)
		tempPoint.EndPoint[i] = pRow->GetItem(2 + i)->GetValue();
	if (!theApp.m_GgMotion.m_bIsRuninMode)
		theApp.m_GgMotion.RuninConfig(1);
	theApp.AddedPointMove(tempPoint, TRUE, FALSE);
}

void CProjectSettingDlg::OnPopUpdatePos()
{
	// TODO:  在此添加命令处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString Tip;
	CBCGPGridRow* pRow = m_wndLaserTrack.GetCurSel();
	int nRow = pRow->GetRowId();
	Tip.Format(_T("确认更新第%d点坐标 ？"), nRow);
	for (int i = 0; i < 2; i++)
		pRow->GetItem(2 + i)->SetValue(theApp.m_GgMotion.m_dAxisCurPos[i]);
}

void CProjectSettingDlg::OnBtnGetCenter()
{
	// TODO:  在此添加命令处理程序代码
	// TODO: 在此添加命令处理程序代码
	int j;
	double pt[3][2];
	double cp[2];
	ZeroMemory(&cp, 2 * sizeof(double));

	CBCGPGridRow* pRow = m_wndLaserTrack.GetCurSel();
	int nRow = pRow->GetRowId();
	if (nRow){
		//获取选择的平面方向
		CBCGPGridItem* pItem ;
		CBCGPGridRow* pLastRow = m_wndLaserTrack.GetRow(nRow - 1);
		//曲线尾点
		pItem = pRow->GetItem(2);
		pt[2][0] = pItem->GetValue();
		pItem = pRow->GetItem(3);
		pt[2][1] = pItem->GetValue();
		//曲线起点
		pItem = pLastRow->GetItem(2);
		pt[0][0] = pItem->GetValue();
		pItem = pLastRow->GetItem(3);
		pt[0][1] = pItem->GetValue();
		//曲线中间某点
		pt[1][0] = theApp.m_GgMotion.m_dAxisCurPos[0];
		pt[1][1] = theApp.m_GgMotion.m_dAxisCurPos[1];

	}
	else{
		BCGPMessageBox(_T("此地不宜为曲线，因为没起点啊"));
		return;
	}
	switch (GetCenter(pt, cp))
	{
	case 0:
	case 1:
		pRow->GetItem(5)->SetValue(cp[0]);
		pRow->GetItem(6)->SetValue(cp[1]);
		break;
	case 2:
		BCGPMessageBox(_T("换个地方再试试？"));
		break;
	}

}

void CProjectSettingDlg::OnUpdatePopOffset(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
}


int CProjectSettingDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	RECT rc1 = { 400, 90, 900, 390 };
	m_wndLaserTrack.Create(WS_CHILD | WS_VISIBLE, rc1, this, IDC_LIST_LASERTRACK);

	// TODO:  在此添加您专用的创建代码
	return 0;
}


void CProjectSettingDlg::OnBnClickedButtonUpdate()
{
	// TODO:  在此添加控件通知处理程序代码
	BeginWaitCursor();
	UpdateTaskList(TRUE);
	EndWaitCursor();
}


void CProjectSettingDlg::OnBnClickedButtonLasertest()
{
	// TODO:  在此添加控件通知处理程序代码
// 	theApp.LaserTestOut();
	theApp.LaserOut(TRUE,FALSE,m_nLaserParam[1]);
}


void CProjectSettingDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (m_wndLaserTrack.m_hWnd == NULL)
		return;      // Return if window is not created yet.

	CRect rectClient;
	CWnd* pWnd = GetDlgItem(IDC_STATIC_CONTAIN);
	if (pWnd)
	{
		pWnd->GetWindowRect(rectClient);
		ScreenToClient(rectClient);
		m_wndLaserTrack.MoveWindow(rectClient);
	}

	// TODO:  在此处添加消息处理程序代码
}


void CProjectSettingDlg::PreInitDialog()
{
	// TODO:  在此添加专用代码和/或调用基类

	CDialogEx::PreInitDialog();
}


void CProjectSettingDlg::OnKillFocusEditVel(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	UINT nIndex = idCtl - IDC_EDIT_MU_VEL;
	double dvalue = m_dRunSpeed[nIndex>>1][nIndex&0x01];
	UpdateData();
	float& NewValue = m_dRunSpeed[nIndex >> 1][nIndex & 0x01];
	switch (nIndex)
	{
	case 0://IDC_EDIT_MU_VEL
	case 2://IDC_EDIT_AU_VEL
		if (NewValue < 1 || NewValue>399){
			NewValue = dvalue;
			UpdateData(FALSE);
		}
		break;
	case 1://IDC_EDIT_MU_ACVEL
	case 3://IDC_EDIT_AU_ACVEL
		if (NewValue < 1||NewValue>1500){
			NewValue = dvalue;
			UpdateData(FALSE);
		}
		break;
	}

}

// CProjectSettingDlg 消息处理程序

void CProjectSettingDlg::OnKillFocusEditPos(UINT idCtl)
{
}

void CProjectSettingDlg::OnBnClickedButtonApply()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	int i(0);
	if (IDYES == MessageBox(_T("更新加工运转设置及轨迹?"), _T("重要提示"), MB_YESNO))
	{
		UpdateData();
		for (i = 0; i < 2; i++)
		{
			theApp.m_cParam.Run_VEL[i] = m_dRunSpeed[i][0];
			theApp.m_cParam.Run_ACC[i] = m_dRunSpeed[i][1];
			theApp.m_GgMotion.m_dCrdVel[i] = theApp.m_cParam.Run_VEL[i];
			theApp.m_GgMotion.m_dCrdAcc[i] = theApp.m_cParam.Run_ACC[i];
		}
		CTrackLine &tl = theApp.m_cParam.PtArray.Ctline.at(bFront);
		i = tl.Tlline.size();
		for (; i != 0; i--)
		{
			LPnode& lpn = tl.Tlline.at(i - 1);
			lpn.Vel = theApp.m_cParam.Run_VEL[lpn.bLaserOn & 0x01];
			lpn.ACC = theApp.m_cParam.Run_ACC[lpn.bLaserOn & 0x01];
		}
		return;
	}
}



void CProjectSettingDlg::OnBnClickedButtonTest()
{
	// TODO:  在此添加控件通知处理程序代码
	theApp.WorkProcess(CLasawApp::CUTTING_TEST+bFront);
}


void CProjectSettingDlg::OnBnClickedButtonCancel()
{
	// TODO:  在此添加控件通知处理程序代码

	PostMessage(WM_USER_UPDATEUI, 6, 0);
	PostMessage(WM_USER_UPDATEUI, 2, 0);
	theApp.LaserOut(FALSE,FALSE,0);
}


void CProjectSettingDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnCancel();
}


void CProjectSettingDlg::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnOK();
}

void CProjectSettingDlg::OnBnChangePosture(UINT idCtl)
{
	CString str[] = { _T("F面"), _T("B面")};
	UpdateData();
	PostMessage(WM_USER_UPDATEUI, 4, 0);
	SetDlgItemText(IDC_STATIC_LEFT, str[1>>bFront]);
	SetDlgItemText(IDC_STATIC_RIGHT, str[bFront]);
}


void CProjectSettingDlg::OnBnClickedButtonTurnover()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str[] = { _T("F面"), _T("B面") };
	CString msg;
	msg.Format(_T("执行后，%s轨迹描述由%s轨迹沿X轴翻转计算出来，直接生效，确认执行计算？"),str[bFront],str[1>>bFront]);
	if (IDYES == MessageBox(msg, _T("翻转提示"), MB_YESNO))
	{
		if (theApp.TurnOverPane(theApp.m_cParam.PtArray.Ctline.at(bFront).Tlline, theApp.m_cParam.PtArray.Ctline.at(1 >> bFront).Tlline))
			PostMessage(WM_USER_UPDATEUI, 4, 0);
		else
			AfxMessageBox(_T("没有轨迹供翻转！"));
	}
}



void CProjectSettingDlg::OnUpdateBtnGetCenter(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
	BOOL bShow(FALSE);
	CBCGPGridItem* pSelcItem = m_wndLaserTrack.GetCurSelItem();
	if (pSelcItem)
	{
		CBCGPGridItemID nid = pSelcItem->GetGridItemID();
		switch (nid.m_nColumn)
		{
		case 5:
		case 6:
			if (pSelcItem->IsEnabled())
				bShow = TRUE;
			break;
		}
	}
	pCmdUI->Enable(bShow);
}


void CProjectSettingDlg::OnUpdateBtnGetUpdate(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
	BOOL bShow(FALSE);
	CBCGPGridItem* pSelcItem = m_wndLaserTrack.GetCurSelItem();
	if (pSelcItem)
	{
		CBCGPGridItemID nid = pSelcItem->GetGridItemID();
		switch (nid.m_nColumn)
		{
		case 2:
		case 3:
			if (pSelcItem->IsEnabled())
				bShow = TRUE;
			break;
		}
	}
	pCmdUI->Enable(bShow && (theApp.m_nIsHomeEnd==0x03));
}


void CProjectSettingDlg::OnUpdateCmdMoveto(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
}
