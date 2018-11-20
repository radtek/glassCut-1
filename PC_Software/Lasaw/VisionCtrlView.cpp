// VisionCtrlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "VisionCtrlView.h"
#include "MainFrm.h"


// CVisionCtrlView

IMPLEMENT_DYNAMIC(CVisionCtrlView, CDialogEx)

CVisionCtrlView::CVisionCtrlView(CWnd* pParent)
: CDialogEx(CVisionCtrlView::IDD)
{
	m_bUseAngleCalib=FALSE;
	m_bUseAngleRel=FALSE;
	m_bUseMask = FALSE;
	m_bUseBinary = FALSE;
	m_pReferenceImg = NULL;

	m_bNewRefAngle = FALSE;
	m_bNewRefImg = FALSE;
	m_bNewTemp = FALSE;
}

CVisionCtrlView::~CVisionCtrlView()
{
	
}

void CVisionCtrlView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_STRENGTH, m_nSSStrength);
	DDX_Text(pDX, IDC_EDIT_GAP, m_nSSGap);
	DDX_Text(pDX, IDC_EDIT_SCORES, m_nTMScore);
	DDX_Text(pDX, IDC_EDIT_REFANGLE, m_fAngleRef);
	DDX_Check(pDX, IDC_CHECK_ANGLE_EXT, m_bUseAngleCalib);
	DDX_Check(pDX, IDC_CHECK_ANGLE_REL, m_bUseAngleRel);
	DDX_Check(pDX, IDC_CHECK_BINARY, m_bUseBinary);
	DDX_Check(pDX, IDC_CHECK_MASK, m_bUseMask);
}

BEGIN_MESSAGE_MAP(CVisionCtrlView, CDialogEx)
	ON_MESSAGE(WM_USER_UPDATEUI, &CVisionCtrlView::OnUpdateDlg)
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_STRENGTH, IDC_EDIT_SEARCH_KERNEL, OnKillFocusSSEdit)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_SEARCHRECT, IDC_BUTTON_TEMPLATERECT, OnImageEditTool)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_COMBO_POLARITY, IDC_COMBO_SEARCH_DIR, OnCbnSelchangeComboSearchDir)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_ANGLE_EXT, IDC_CHECK_ANGLE_REL, OnBnClickedCheckAngle)
	ON_BN_CLICKED(IDC_BUTTON_MATCH, &CVisionCtrlView::OnBnClickedButtonMatch)
	ON_BN_CLICKED(IDC_BUTTON_FINDTEST, &CVisionCtrlView::OnBnClickedButtonFindtest)
	ON_EN_KILLFOCUS(IDC_EDIT_SCORES, OnKillFocusTMEdit)
	ON_BN_CLICKED(IDC_CHECK_BINARY, &CVisionCtrlView::OnBnClickedCheckBinary)
	ON_BN_CLICKED(IDC_CHECK_MASK, &CVisionCtrlView::OnBnClickedCheckBinary)
	ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CVisionCtrlView::OnBnClickedButtonRegister)
	ON_BN_CLICKED(IDC_BUTTON_TOOL, &CVisionCtrlView::OnBnClickedButtonTool)
//	ON_WM_CLOSE()
ON_BN_CLICKED(ID_BTN_CANCEL, &CVisionCtrlView::OnBnClickedBtnCancel)
ON_BN_CLICKED(ID_BTN_APPLY, &CVisionCtrlView::OnBnClickedBtnApply)
END_MESSAGE_MAP()


// CVisionCtrlView 诊断

#ifdef _DEBUG
void CVisionCtrlView::AssertValid() const
{
	CDialogEx::AssertValid();
}

#ifndef _WIN32_WCE
void CVisionCtrlView::Dump(CDumpContext& dc) const
{
	CDialogEx::Dump(dc);
}
#endif
#endif //_DEBUG

LRESULT CVisionCtrlView::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString str;
	CComboBox* pCombox;
	UINT i(0), j(2);
	UINT nIndex = wParam;
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);
	switch (nIndex)
	{
	case 0:
		GetDlgItem(IDC_EDIT_SCORES)->EnableWindow(bEnable);
		GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_REGISTER)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_SEARCHRECT)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_TEMPLATERECT)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_MATCH)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_TOOL)->EnableWindow(bEnable);
		GetDlgItem(IDC_CHECK_ANGLE_EXT)->EnableWindow(bEnable);
	case 1:
		bEnable = bEnable&&m_bUseAngleCalib;
		GetDlgItem(IDC_CHECK_ANGLE_REL)->EnableWindow(bEnable);
		GetDlgItem(IDC_COMBO_POLARITY)->EnableWindow(bEnable);
		GetDlgItem(IDC_COMBO_SEARCH_DIR)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_SEARCH_AREA)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_FINDTEST)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_STRENGTH)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_SEARCH_WIDTH)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_GAP)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_SEARCH_KERNEL)->EnableWindow(bEnable);

		break;
	case 2:
		m_bNewRefAngle = FALSE;
		m_bNewRefImg = FALSE;

		m_bUseAngleCalib = theApp.m_cParam.PrjCfg.uAngleCali ? 1 : 0;
		m_bUseAngleRel = theApp.m_cParam.PrjCfg.uAngleCali >>1;
		m_fAngleRef = theApp.m_cParam.PrjCfg.fAngleCali;
		m_nTMScore = theApp.m_cParam.TempData.at(0).nScore;
		m_nSSStrength = theApp.m_cParam.SlineData.at(0).nEdgeStrength;
		m_nSSGap = theApp.m_cParam.SlineData.at(0).nGap;
		m_ndir = theApp.m_cParam.SlineData.at(0).nDir;
		str.Format(_T("%d"), theApp.m_cParam.SlineData.at(0).nWidth);
		GetDlgItem(IDC_EDIT_SEARCH_WIDTH)->SetWindowText(str);
		str.Format(_T("%d"), theApp.m_cParam.SlineData.at(0).nKernelSize);
		GetDlgItem(IDC_EDIT_SEARCH_KERNEL)->SetWindowText(str);
		pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_DIR);
		pCombox->SetCurSel(m_ndir & 0x03);
		pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_POLARITY);
		pCombox->SetCurSel(m_ndir >> 2);
		UpdateData(FALSE);
		m_SearchLine = theApp.m_cParam.rorcSearch.at(0);
		m_Searchrc = theApp.m_cParam.rcSearch.at(0);
		m_Trc = theApp.m_cParam.TempData.at(0).rcTemp;
		theApp.LoadReference(m_pReferenceImg);
		break;
	case 3:
	case 4:
		UpdateData(FALSE);
		break;
	}
	return 0;
}

void CVisionCtrlView::OnKillFocusTMEdit()
{
	UINT num;
	num = m_nTMScore;
	UpdateData();
	if (m_nTMScore > 1000)
	{
		m_nTMScore = num;
		UpdateData(FALSE);
	}
}

void CVisionCtrlView::OnImageEditTool(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString str;
	BOOL bSetOffset(FALSE);
	BOOL bNewTemp(FALSE);
	Rect rc = m_Trc;
	PointFloat fPt[2];
	int offset[2] = {0,0};
	int nSel = 3;
	UINT nCount(0);
	UINT m(0);
	UINT n(0);
	UINT nTool = 12311;
	UINT nType = idCtl - IDC_BUTTON_SEARCHRECT;
	if (!m_pReferenceImg)
	{
		AfxMessageBox(_T("请先抓取基准图像！"));
		return ;
	}
	Image* TempImg = NULL;
	TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	RGBValue defaultcolor = IMAQ_RGB_GREEN;
	switch (nType)
	{
	case 0://IDC_BUTTON_SEARCHRECT
		theApp.m_NiVision.DrawRect(TempImg, imaqMakeRotatedRectFromRect(m_Searchrc), CLasawApp::SEARCH_RC_TEMPLATE, &defaultcolor);
		theApp.m_NiVision.UpdateOverlay(m_pReferenceImg, TempImg, CLasawApp::SEARCH_RC_TEMPLATE+1);
		break;
	case 1://IDC_BUTTON_SEARCH_AREA
		defaultcolor = {   0,100, 0,  0 };
		nTool = 14339;
		nSel = 12;
		theApp.m_NiVision.DrawRect(TempImg, m_SearchLine, CLasawApp::SEARCH_RC_LINE, &defaultcolor);
		theApp.m_NiVision.UpdateOverlay(m_pReferenceImg, TempImg, CLasawApp::SEARCH_RC_LINE + 1);

		break;
	case 2://IDC_BUTTON_TEMPLATERECT
		defaultcolor = { 0,170, 0,  0 };
		theApp.m_NiVision.DrawRect(TempImg, imaqMakeRotatedRectFromRect(m_Trc), CLasawApp::RC_TEMPLATE, &defaultcolor);
		theApp.m_NiVision.UpdateOverlay(m_pReferenceImg, TempImg, CLasawApp::RC_TEMPLATE + 1);

		break;
	}
	ROI* roi = imaqCreateROI();

	if (theApp.m_NiVision.CreateROI(m_pReferenceImg, roi, nSel, nTool))
	{
		nCount = imaqGetContourCount(roi);
		for (UINT i = 0; i < nCount&&nCount < 3; ++i)
		{
			ContourInfo2* pInf = imaqGetContourInfo2(roi, imaqGetContour(roi, i));
			if (pInf == NULL)
				break;
			switch (pInf->type)
			{
			case IMAQ_POINT:
				if (nType >> 1)
				{
					bSetOffset = TRUE;
					fPt[1].x = pInf->structure.point->x;
					fPt[1].y = pInf->structure.point->y;
				}
				break;
			case IMAQ_RECT:
				rc = *(pInf->structure.rect);
				if (rc.top < 0 || rc.top >= 2048)
					rc.top = 0;
				if (rc.left < 0 || rc.left >= 2048)
					rc.left = 0;
				if (rc.width + rc.left>2048)
					rc.width = 2048 - rc.left;
				if (rc.height + rc.top>2048)
					rc.height = 2048 - rc.top;
				if (nType >> 1)
					bNewTemp = TRUE;
				else if (nType)
					m_SearchLine = imaqMakeRotatedRectFromRect(rc);
				else
					m_Searchrc = rc;

				break;
			case IMAQ_ROTATED_RECT:
				if (nType)
					m_SearchLine = *(pInf->structure.rotatedRect);//找线
			case IMAQ_EMPTY_CONTOUR:
			default:

				break;
			}
			imaqDispose(pInf);
			if (bNewTemp)
			{
				fPt[0].x = rc.left + rc.width / 2;
				fPt[0].y = rc.top + rc.height / 2;
				if (bSetOffset)
				{
					offset[0] = fPt[1].x - fPt[0].x;
					offset[1] = fPt[1].y - fPt[0].y;
				}

				m_Trc = rc;
				imaqDispose(TempImg);
				TempImg = NULL;
				if (m_bUseMask)
				{
					TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
					theApp.m_NiVision.CreateMaskImage(TempImg, m_pReferenceImg, rc);
				}
				if (!theApp.StartLearning(0, TempImg,m_Trc, offset[0], offset[1]))
				{
					AfxMessageBox(_T("没有成功，可能是前次模板学习未完成，请稍后再试！\r\n"));
				}
				m_bNewTemp = TRUE;
			}

		}

	}
	imaqDispose(roi);
	imaqDispose(TempImg);
}

void CVisionCtrlView::OnBnClickedButtonMatch()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CComboBox* pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_CAMERA);
	int j = pCombox->GetCurSel();
	theApp.ClearOverlayInfo(theApp.m_nCameraID[theApp.m_nCurrentCameraID]);
	theApp.MatchProcessing(NULL, 0, m_Searchrc,	m_nTMScore, 0, NULL);

}

void CVisionCtrlView::OnKillFocusSSEdit(UINT idCtl)
{
	CString str;
	UINT num;
	switch (idCtl)
	{
	case IDC_EDIT_STRENGTH:
		num = m_nSSStrength;
		UpdateData();
		if (m_nSSStrength > 255)
		{
			m_nSSStrength = num;
			UpdateData(FALSE);
		}

		break;
	case IDC_EDIT_GAP:
		num = m_nSSGap;
		UpdateData();
		if (m_nSSGap > 255)
		{
			m_nSSGap = num;
			UpdateData(FALSE);
		}
		break;
	case IDC_EDIT_SEARCH_WIDTH:
		break;
	case IDC_EDIT_SEARCH_KERNEL:
		break;
	}

}

void CVisionCtrlView::OnBnClickedButtonFindtest()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	BOOL bIsReferenceImg(FALSE);
	IVA_StepResults LineResults;
	IVA_Result angledata[] = { 0.0 };
	LineResults.results = angledata;
	LineResults.numResults = 1;

	theApp.ClearOverlayInfo(theApp.m_nCameraID[theApp.m_nCurrentCameraID], 3);
	if (m_bUseAngleRel)
	{
		if (IDYES == MessageBox(_T("已勾选相对角度：当前图像是基准图像吗？\r\n\r\n欲将找线结果保存为基准角度，请点击是\r\n否则,请点击否，仅做搜寻测试"), _T("请确认"), MB_YESNO))
			bIsReferenceImg = TRUE;
	}
	if (theApp.FindLine(NULL, m_SearchLine, FALSE, &LineResults) && bIsReferenceImg)
	{
		m_fAngleRef = (int)(angledata[0].resultVal.numVal*100)/100.0;
		UpdateData(FALSE);
	}

}

void CVisionCtrlView::OnCbnSelchangeComboSearchDir(UINT idCtl)
{
	CComboBox* pCombox;
	
	pCombox = (CComboBox*)GetDlgItem(idCtl);
	switch (idCtl)
	{
	case IDC_COMBO_POLARITY:
		m_ndir = (m_ndir & 0x03) | (pCombox->GetCurSel() << 2);
		break;
	case IDC_COMBO_SEARCH_DIR:
		m_ndir = (m_ndir & 0x0C) | pCombox->GetCurSel();
	default:break;
	}
}

void CVisionCtrlView::OnBnClickedCheckBinary()
{
	UpdateData();
}



// CVisionCtrlView 消息处理程序

void CVisionCtrlView::OnBnClickedCheckAngle(UINT idCtl)
{
	UpdateData();
	switch (idCtl)
	{
	case IDC_CHECK_ANGLE_EXT:
		PostMessage(WM_USER_UPDATEUI, 1, 0);
		break;
	case IDC_CHECK_ANGLE_REL:
		break;
	default:break;
	}
}

void CVisionCtrlView::OnBnClickedButtonRegister()
{
	// TODO:  在此添加控件通知处理程序代码
	UINT nIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_CAMERA))->GetCurSel();
	if (nIndex < 4)
	{
		CString Tip; Tip.Format(_T("确认从%d号相机获取新的基准图像吗？（将覆盖旧图像文件）\n"), nIndex+1);
		if (IDYES == MessageBox(Tip, _T("提示"), MB_YESNO))
		{
			if (!m_pReferenceImg)
				m_pReferenceImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
			m_bNewRefImg = theApp.m_NiVision.GetImage(nIndex, m_pReferenceImg,TRUE);
			if (m_bNewRefImg)
			{
				if (m_bUseBinary)
					theApp.m_NiVision.CreateAutoBinary(m_pReferenceImg, m_pReferenceImg, 2);
				if (!theApp.LoadReference(m_pReferenceImg, TRUE))
					AfxMessageBox(_T("新基准图像保存失败！"));
				else
					theApp.AddedProcessMsg(_T("创建了新的基准图像\r\n"));
			}
			else
			{
				imaqDispose(m_pReferenceImg);
				m_pReferenceImg = NULL;
			}
		}
	}
}


BOOL CVisionCtrlView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CComboBox* pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_CAMERA);
	pCombox->SetCurSel(0);

	PostMessage(WM_USER_UPDATEUI, 2, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CVisionCtrlView::OnBnClickedButtonTool()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str = _T("C:\\Program Files (x86)\\National Instruments\\Vision\\Utility\\NI Vision Template Editor\\NI Vision Template Editor.exe");
	AfxMessageBox(_T("此工具为独立软件，仅可编辑已存在的模板文件 \r\n请确定你欲编辑的模板对象已保存至文件!"));

	if ((int)ShellExecute(NULL, _T("open"), str, NULL, NULL, SW_SHOWNORMAL) < 32)
	{
		AfxMessageBox(_T("工具未安装或者路径不对!"));
		return;
	}
	AfxMessageBox(_T("工具已打开，使用完毕后请重新加载模板文件"));
}


//void CVisionCtrlView::OnClose()
//{
//	// TODO:  在此添加消息处理程序代码和/或调用默认值
//
//	CDialogEx::OnClose();
//}


void CVisionCtrlView::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnCancel();
}


void CVisionCtrlView::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnOK();
}


void CVisionCtrlView::OnBnClickedBtnCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (m_bNewTemp)
		pFrame->StartTemplateRW(TRUE, TRUE);
	PostMessage(WM_USER_UPDATEUI, 2, 0);
}


void CVisionCtrlView::OnBnClickedBtnApply()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString Tip = _T("确认应用修改吗？\n");
	CString str;
	if (IDYES == MessageBox(Tip, _T("提示"), MB_YESNO))
	{
		UpdateData();
		theApp.m_cParam.TempData.at(0).nScore = m_nTMScore;

		theApp.m_cParam.SlineData.at(0).nDir = m_ndir;
		theApp.m_cParam.SlineData.at(0).nEdgeStrength = m_nSSStrength;
		theApp.m_cParam.SlineData.at(0).nGap = m_nSSGap;
		GetDlgItemText(IDC_EDIT_SEARCH_WIDTH, str);
		theApp.m_cParam.SlineData.at(0).nWidth = _ttoi(str);
		GetDlgItemText(IDC_EDIT_SEARCH_KERNEL, str);
		theApp.m_cParam.SlineData.at(0).nKernelSize = _ttoi(str);

		theApp.m_cParam.PrjCfg.uAngleCali = m_bUseAngleCalib|(m_bUseAngleRel << 1);

		theApp.m_cParam.rorcSearch.at(0) = m_SearchLine;
		theApp.m_cParam.rcSearch.at(0) = m_Searchrc;
		theApp.m_cParam.TempData.at(0).rcTemp = m_Trc;
		if (m_bNewRefAngle)
		{
			theApp.m_cParam.PrjCfg.fAngleCali = m_fAngleRef;
			m_bNewRefAngle = FALSE;
		}
		if (m_bNewTemp)
		{
			m_bNewTemp = FALSE;
			pFrame->StartTemplateRW(FALSE,TRUE);
		}
	}

}
