// DxfDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Lasaw.h"
#include "DxfDlg.h"
#include "afxdialogex.h"


// DxfDlg 对话框

IMPLEMENT_DYNAMIC(DxfDlg, CDialogEx)

DxfDlg::DxfDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(DxfDlg::IDD, pParent)
	, m_fProportion(1)
	, m_nMin_Angle(1)
	, m_nSpline_Step(10)
	, m_bDir(FALSE)
{
	m_Img = NULL;
	m_nCurrntTool = -1;
	m_bFlashLocus = FALSE;
	m_nFlashLocus = 0;
	// 		赤 255, 0, 0
	// 		橙 255, 128, 0
	// 		黄 255, 255, 0
	// 		绿 0, 255, 0
	// 		青 0, 255, 255
	// 		蓝 0, 0, 255
	// 		紫 128, 0, 255
	DrawColor[0] = { 255, 0, 128, 0 };
	DrawColor[1] = IMAQ_RGB_BLUE;
	DrawColor[2] = { 255, 255, 0, 0 };
	DrawColor[3] = IMAQ_RGB_GREEN;
	DrawColor[4] = IMAQ_RGB_YELLOW;
	DrawColor[5] = { 0, 128, 255, 0 };
	DrawColor[6] = IMAQ_RGB_RED;
	DrawColor[7] = { 255, 255, 255, 0 };

}

DxfDlg::~DxfDlg()
{
	imaqDispose(m_Img);
}

void DxfDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PROPORTION, m_fProportion);
	DDX_Text(pDX, IDC_EDIT_MIN_ANGLE, m_nMin_Angle);
	DDX_Text(pDX, IDC_EDIT_CUT_STEP, m_nSpline_Step);
	DDX_Radio(pDX, IDC_RADIO_CW, m_bDir);
}


BEGIN_MESSAGE_MAP(DxfDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &DxfDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &DxfDlg::OnBnClickedBtnCancel)
	ON_BN_CLICKED(ID_BTN_APPLY, &DxfDlg::OnBnClickedBtnApply)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM, &DxfDlg::OnBnClickedButtonZoom)
	ON_BN_CLICKED(IDC_BUTTON_PAN, &DxfDlg::OnBnClickedButtonPan)
	ON_BN_CLICKED(IDC_BUTTON_REVERSE, &DxfDlg::OnBnClickedButtonReverse)
	ON_BN_CLICKED(IDC_BUTTON_SPLICE, &DxfDlg::OnBnClickedButtonSplice)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_EXPORT, IDC_BUTTON_EXPORT2, OnBnClickedButtonExport)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CW, IDC_RADIO_CCW, OnBnClickedButtonDir)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_COMBO_LOCUS, IDC_COMBO_LOCUS_LAST, OnCbnSelchangeComboLOCUS)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_USER_UPDATEUI, &DxfDlg::OnUpdateDlg)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// DxfDlg 消息处理程序


void DxfDlg::CreateCanvas(Image* &Img, CRect AreaRc)
{
	AreaRc.NormalizeRect();
	if (AreaRc.IsRectEmpty())
		return;
	Line semgent[2];
	theApp.m_NiVision.CreateImage(Img, AreaRc.right - AreaRc.left + 50, AreaRc.bottom - AreaRc.top + 50, IMAQ_IMAGE_RGB);
	theApp.m_NiVision.ClearOverlay(Img);

	semgent[0].start = {15,25};
	semgent[0].end = {35,25};
	semgent[1].start = {25,15};
	semgent[1].end = {25,35};
	theApp.m_NiVision.DrawLineWithArrow(Img, semgent[0], FALSE, TRUE, BASIC_INFO_LAYER, &DrawColor[7]);
	theApp.m_NiVision.DrawLineWithArrow(Img, semgent[1], FALSE, TRUE, BASIC_INFO_LAYER, &DrawColor[7]);
}

void DxfDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	if (theApp.OpenFileDlg(TRUE, TRUE))
	{
		theApp.m_DxfLocus.m_LocusChain.SetEntyDir(m_bDir);
		SetDlgItemText(IDC_STATIC_PATH,theApp.m_DxfLocus.m_strFile);
		PostMessage(WM_USER_UPDATEUI, 4, 0);
		PostMessage(WM_USER_UPDATEUI, 3, 1);
		PostMessage(WM_USER_UPDATEUI, 1,0);
	}
}


void DxfDlg::OnBnClickedButtonExport(UINT idCtl)
{
	// TODO:  在此添加控件通知处理程序代码
	BYTE nid = idCtl - IDC_BUTTON_EXPORT;
	CString str[] = { _T("确认修改正面轨迹？"), _T("确认修改背面轨迹？") };
	if (theApp.m_DxfLocus.m_bIsOpen)
	{
		if (IDYES == MessageBox(str[nid], _T("提示"), MB_YESNO))
		{
			theApp.MergeLocus(nid);
		}
	}
}


void DxfDlg::OnBnClickedBtnCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	m_fProportion=theApp.m_cParam.fProportion;
	m_nMin_Angle = theApp.m_cParam.nMin_Angle;
	m_nSpline_Step = theApp.m_cParam.nSpline_Step;

	UpdateData(FALSE);
}


void DxfDlg::OnBnClickedBtnApply()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	theApp.m_cParam.fProportion = m_fProportion;
	theApp.m_cParam.nMin_Angle = m_nMin_Angle;
	theApp.m_cParam.nSpline_Step = m_nSpline_Step;
}


BOOL DxfDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CWnd* pDisplayWnd = GetDlgItem(IDC_STATIC_DISPLAY);
	theApp.m_NiVision.SetupWindow(pDisplayWnd, USER_WINDOW); 
	CRect ClientRc; 
	pDisplayWnd->GetClientRect(ClientRc);
	theApp.m_NiVision.ResizeDisplayWnd(USER_WINDOW, ClientRc);	
	CreateCanvas(m_Img,ClientRc);
	PostMessage(WM_USER_UPDATEUI, 3, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

LRESULT DxfDlg::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CComboBox* pCombox;
	CString str;
	int nSelect[2];
	switch (wParam)
	{
	case 0:
		theApp.m_NiVision.DisplayUserImage(USER_WINDOW, m_Img);
		break;
	case 1:
		DrawLocus();
		break;
	case 2:
		DrawLocus(m_nFlashLocus);
		break;
	case 3:
		GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow(lParam);
		GetDlgItem(IDC_BUTTON_EXPORT2)->EnableWindow(lParam);
		GetDlgItem(IDC_BUTTON_ZOOM)->EnableWindow(lParam);
		GetDlgItem(IDC_BUTTON_PAN)->EnableWindow(lParam);

		pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_LOCUS);
		pCombox->EnableWindow(lParam);
		nSelect[0] = pCombox->GetCurSel();
		GetDlgItem(IDC_BUTTON_REVERSE)->EnableWindow(lParam&&nSelect[0]>=0);
		pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_LOCUS_LAST);
		pCombox->EnableWindow(lParam);
		nSelect[1] = pCombox->GetCurSel();
		GetDlgItem(IDC_BUTTON_SPLICE)->EnableWindow(lParam&&nSelect[0] >= 0 && nSelect[1] >= 0 && (nSelect[1]^ nSelect[0]));

		break;
	case 4:
		pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_LOCUS);
		pCombox->ResetContent();
		for (UINT j = 0; j < theApp.m_DxfLocus.GetLocusNum(); j++)
		{
			str.Format(_T("%d"), j + 1);
			pCombox->AddString(str);
		}

		pCombox = (CComboBox*)GetDlgItem(IDC_COMBO_LOCUS_LAST);
		pCombox->ResetContent();
		for (UINT j = 0; j < theApp.m_DxfLocus.GetLocusNum(); j++)
		{
			str.Format(_T("%d"), j + 1);
			pCombox->AddString(str);
		}

		break;
	default:
		break;
	}
	return 0;
}

void DxfDlg::DrawLocus()
{
	RGBValue DrawLineColor = { 0, 100, 0, 0 };
	RGBValue DrawArcColor = { 0, 0, 100, 0 };

	CString str;
	double pt[9] = { 0.0 };
	Line semgent;
	ArcInfo arc;
	CreateCanvas(m_Img,theApp.m_DxfLocus.GetTargetRect());
	LONG offset[2] = { theApp.m_DxfLocus.AreaRc.left-25, theApp.m_DxfLocus.AreaRc.top-25 };
	for (UINT chain_num = 0; chain_num < theApp.m_DxfLocus.m_LocusChain.GetEntySize(); chain_num++)
	{
		for (UINT node_num = 0; node_num < theApp.m_DxfLocus.m_LocusChain.GetChainSize(chain_num); node_num++)
		{
			const MyNode* pNode = theApp.m_DxfLocus.m_LocusChain.GetNode(chain_num, node_num);
			UINT i = pNode->nIndex;
			BYTE nDir = pNode->nDir;
			DWORD Sample_Num = theApp.m_DxfLocus.GetSampleNum(i);//当前轨迹图元数
			for (UINT j = 0; j < Sample_Num; j++)
			{
				DWORD MoveDirection(0);
				BYTE bType = theApp.m_DxfLocus.GetType(nDir, i, j);
				str.Format(_T("轨迹：%d；段落：%d；类型：%d\n"), i + 1, j + 1, bType);
				OutputDebugString(str);
				switch (bType)
				{
				case 0:
					if (theApp.m_DxfLocus.GetLine(&pt[0], &pt[1], &pt[2], &pt[3], nDir, i, j))
					{
						BOOL bDrawArrow = j == 0 || j == Sample_Num - 1 || j == Sample_Num / 2;
						str.Format(_T("线段起点X:%.1f Y:%.1f    终点X:%.1f Y:%.1f\n\r\n"), pt[0], pt[1], pt[2], pt[3]);
						OutputDebugString(str);
						semgent.start.x = pt[0] - offset[0];
						semgent.start.y = pt[1] - offset[1];
						semgent.end.x = pt[2] - offset[0];
						semgent.end.y = pt[3] - offset[1];
						theApp.m_NiVision.DrawLineWithArrow(m_Img, semgent, FALSE, bDrawArrow, FIRST_LOCUS_LAYER + i, &DrawColor[chain_num % 7]);
					}
					break;
				case 2:
					if (theApp.m_DxfLocus.GetArc(&pt[0], &pt[1], &pt[2],
						&pt[3], &pt[4], &pt[5], &pt[6],
						&MoveDirection, &pt[7], &pt[8], nDir, i, j))
					{
						str.Format(_T("圆弧起点X:%.1f Y:%.1f    终点X:%.1f Y:%.1f方向:%d\n\r\n"), pt[3], pt[4], pt[5], pt[6], MoveDirection);
						OutputDebugString(str);
						if (MoveDirection)
						{
							arc.startAngle = 360 - pt[8] + 0.001;
							arc.endAngle = 360 - pt[7] + 0.001;
						}
						else
						{
							arc.startAngle = 360 - pt[7] + 0.001;
							arc.endAngle = 360 - pt[8] + 0.001;
						}
						arc.boundingBox.top = pt[1] - pt[2] - offset[1];
						arc.boundingBox.left = pt[0] - pt[2] - offset[0];
						arc.boundingBox.height = 2 * pt[2];
						arc.boundingBox.width = 2 * pt[2];
						theApp.m_NiVision.DrawArcWithArrow(m_Img, arc, MoveDirection, TRUE, FIRST_LOCUS_LAYER + i, &DrawColor[chain_num % 7]);
					}
					break;
				}
			}
		}
	}
	theApp.m_NiVision.Flip(m_Img, m_Img, 0);
	PostMessage(WM_USER_UPDATEUI);
}

void DxfDlg::DrawLocus(UINT nIndex)
{
	double pt[9] = { 0.0 };
	BYTE nDir(0);
	Line semgent;
	ArcInfo arc;
	Image* TempImg=NULL;
	CreateCanvas(TempImg, theApp.m_DxfLocus.GetTargetRect());
	LONG offset[2] = { theApp.m_DxfLocus.AreaRc.left - 25, theApp.m_DxfLocus.AreaRc.bottom - 25 };
	UINT color_Index(0);
	theApp.m_DxfLocus.m_LocusChain.GetNodeMark(nIndex, color_Index);
	const RGBValue* pLastColor = &DrawColor[m_bFlashLocus ? 7 : (color_Index % 7)];
	if(nIndex < theApp.m_DxfLocus.GetLocusNum())
	{
		theApp.m_DxfLocus.m_LocusChain.GetNodeDir(nIndex, nDir);
		DWORD Sample_Num = theApp.m_DxfLocus.GetSampleNum(nIndex);//当前轨迹图元数
		for (UINT j = 0; j < Sample_Num; j++)
		{
			DWORD MoveDirection(0);
			BYTE bType = theApp.m_DxfLocus.GetType(nDir, nIndex, j);
			switch (bType)
			{
			case 0:
				if (theApp.m_DxfLocus.GetLine(&pt[0], &pt[1], &pt[2], &pt[3], nDir, nIndex, j))
				{
					BOOL bDrawArrow = j == 0 || j == Sample_Num - 1 || j == Sample_Num / 2;
					semgent.start.x = pt[0] - offset[0];
					semgent.start.y = pt[1] - offset[1];
					semgent.end.x = pt[2] - offset[0];
					semgent.end.y = pt[3] - offset[1];
					theApp.m_NiVision.DrawLineWithArrow(TempImg, semgent, FALSE, bDrawArrow, FIRST_LOCUS_LAYER + nIndex, pLastColor);
				}
				break;
			case 2:
				if (theApp.m_DxfLocus.GetArc(&pt[0], &pt[1], &pt[2],
					&pt[3], &pt[4], &pt[5], &pt[6],
					&MoveDirection, &pt[7], &pt[8], nDir, nIndex, j))
				{
					if (MoveDirection)
					{
						arc.startAngle = 360 - pt[8] + 0.001;
						arc.endAngle = 360 - pt[7] + 0.001;
					}
					else
					{
						arc.startAngle = 360 - pt[7] + 0.001;
						arc.endAngle = 360 - pt[8] + 0.001;
					}
					arc.boundingBox.top = pt[1] - pt[2] - offset[1];
					arc.boundingBox.left = pt[0] - pt[2] - offset[0];
					arc.boundingBox.height = 2 * pt[2];
					arc.boundingBox.width = 2 * pt[2];
					theApp.m_NiVision.DrawArcWithArrow(TempImg, arc, MoveDirection, TRUE, FIRST_LOCUS_LAYER + nIndex, pLastColor);
				}
				break;
			}

		}
	}
	theApp.m_NiVision.Flip(TempImg, TempImg, 0);

	theApp.m_NiVision.UpdateOverlay(m_Img, TempImg, FIRST_LOCUS_LAYER + nIndex+1,TRUE);
	theApp.m_NiVision.ReleaseNiObj(TempImg);
	PostMessage(WM_USER_UPDATEUI);
}


void DxfDlg::OnBnClickedButtonZoom()
{
	// TODO:  在此添加控件通知处理程序代码
	BOOL bZoom(m_nCurrntTool!=8);

	theApp.m_NiVision.EnableZoomDisplay(0xff, bZoom);
	if (!bZoom)
		PostMessage(WM_USER_UPDATEUI);
}


void DxfDlg::OnBnClickedButtonPan()
{
	// TODO:  在此添加控件通知处理程序代码
	BOOL bPan(m_nCurrntTool != 9);

	theApp.m_NiVision.EnablePanDisplay(USER_WINDOW, bPan);
	if (!bPan)
		PostMessage(WM_USER_UPDATEUI);

}


void DxfDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	BOOL bShow(0);
	switch (nIDEvent)
	{
	case 1:
		bShow=IsWindowVisible();
		m_nCurrntTool=theApp.m_NiVision.GetCurrentTool();
		if (m_nCurrntTool == 8)
			((CMFCButton*)GetDlgItem(IDC_BUTTON_ZOOM))->SetFaceColor(CLasawApp::DarkOrange);
		else
			((CMFCButton*)GetDlgItem(IDC_BUTTON_ZOOM))->SetFaceColor(CLasawApp::DarkGray);
		if(m_nCurrntTool == 9)
			((CMFCButton*)GetDlgItem(IDC_BUTTON_PAN))->SetFaceColor(CLasawApp::DarkOrange);
		else
			((CMFCButton*)GetDlgItem(IDC_BUTTON_PAN))->SetFaceColor(CLasawApp::DarkGray);

		break;
	case 2:
		m_bFlashLocus = !m_bFlashLocus;
		PostMessage(WM_USER_UPDATEUI, 2, 0);
		KillTimer(nIDEvent);
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void DxfDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	theApp.m_NiVision.SetCurrentTool(IMAQ_SELECTION_TOOL);
	KillTimer(1);
	CDialogEx::OnClose();
}

void DxfDlg::OnBnClickedButtonDir(UINT idCtl)
{
	BOOL bsts = m_bDir;
	UpdateData(TRUE);
	if (theApp.m_DxfLocus.m_bIsOpen&&(bsts^m_bDir))
	{
		theApp.m_DxfLocus.m_LocusChain.ReverseEntyDir();
		PostMessage(WM_USER_UPDATEUI, 1, 0);
	}
}

void DxfDlg::OnBnClickedButtonReverse()
{
	// TODO:  在此添加控件通知处理程序代码
	int nSelect = ((CComboBox*)GetDlgItem(IDC_COMBO_LOCUS))->GetCurSel();
	BYTE nDir(0);
	theApp.m_DxfLocus.m_LocusChain.GetNodeDir(nSelect, nDir);
	theApp.m_DxfLocus.m_LocusChain.SetNodeDir(nSelect, !nDir);
	m_nFlashLocus = nSelect;
	m_bFlashLocus = TRUE;
	PostMessage(WM_USER_UPDATEUI, 2, 0);
	SetTimer(2, 500, NULL);
}


void DxfDlg::OnBnClickedButtonSplice()
{
	// TODO:  在此添加控件通知处理程序代码
	int nSelect[2];
	nSelect[0] = ((CComboBox*)GetDlgItem(IDC_COMBO_LOCUS))->GetCurSel();
	nSelect[1] = ((CComboBox*)GetDlgItem(IDC_COMBO_LOCUS_LAST))->GetCurSel();
	theApp.m_DxfLocus.m_LocusChain.Combine(nSelect[1], nSelect[0]);
	PostMessage(WM_USER_UPDATEUI, 1, 0);

}

void DxfDlg::OnCbnSelchangeComboLOCUS(UINT idCtl)
{
	BYTE nCtrl = idCtl - IDC_COMBO_LOCUS;
	int nSelect[2];
	nSelect[0]= ((CComboBox*)GetDlgItem(IDC_COMBO_LOCUS))->GetCurSel();
	nSelect[1] = ((CComboBox*)GetDlgItem(IDC_COMBO_LOCUS_LAST))->GetCurSel();
	m_nFlashLocus = nSelect[nCtrl];
	m_bFlashLocus = TRUE;
	PostMessage(WM_USER_UPDATEUI, 2, 0);
	SetTimer(2, 500, NULL);

	PostMessage(WM_USER_UPDATEUI, 3, 1);
}


void DxfDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (!bShow)
		KillTimer(1);
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO:  在此处添加消息处理程序代码
}


void DxfDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类
	KillTimer(1);
	CDialogEx::OnCancel();
}
