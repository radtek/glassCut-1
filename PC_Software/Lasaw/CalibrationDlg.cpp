// CalibrationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "CalibrationDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"


// CalibrationDlg 对话框

IMPLEMENT_DYNAMIC(CalibrationDlg, CDialogEx)

CalibrationDlg::CalibrationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CalibrationDlg::IDD, pParent)
{
	UINT i(0);
	for (i = 0; i < 4; i++)
	{
		nExposure[i] =5000;
		nIntensity[i] = 10;
	}
}

CalibrationDlg::~CalibrationDlg()
{
}

void CalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	for (int i = 0; i < 4; i++)
	{
		DDX_Text(pDX, IDC_EDIT_CCD1_OFFSETX + 2 * i, dOffset[i][0]);
		DDX_Text(pDX, IDC_EDIT_CCD1_OFFSETY + 2 * i, dOffset[i][1]);
		DDX_Text(pDX, IDC_EDIT_EXPOSURE1 + 2 * i, nExposure[i]);
		DDX_Text(pDX, IDC_EDIT_LIGHT1 + 2 * i, nIntensity[i]);
		DDX_Control(pDX, IDC_SPIN1 + 2*i, m_ctlExposureSpin[i]);
		DDX_Control(pDX, IDC_SPIN2 + 2*i, m_ctlIntensitySpin[i]);
	}
}


BEGIN_MESSAGE_MAP(CalibrationDlg, CDialogEx)
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_CCD1_OFFSETX, IDC_EDIT_CCD4_OFFSETY, OnKillFocusCCD_NozzleEdit)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_CACULATE1, IDC_BUTTON_CACULATE4, OnBnClickedButtonCaculate)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_EXPOSURE1, IDC_EDIT_LIGHT4, OnEnChangeEdit)
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER_UPDATEUI, &CalibrationDlg::OnUpdateDlg)
	ON_BN_CLICKED(ID_BTN_APPLY, &CalibrationDlg::OnBnClickedOk)
	ON_BN_CLICKED(ID_BTN_CANCEL, &CalibrationDlg::OnBnClickedBtnCancel)
END_MESSAGE_MAP()


// CalibrationDlg 消息处理程序


BOOL CalibrationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	UINT i(0);
	for (i = 0; i < 4; i++)
	{
// 		m_ctlExposureSpin[i].SetBuddy(GetDlgItem(IDC_EDIT_TOP_EXPOSURE + i));
		m_ctlExposureSpin[i].SetBase(1);
		m_ctlExposureSpin[i].SetRange32(1, 60000);

// 		m_ctlIntensitySpin[i].SetBuddy(GetDlgItem(IDC_EDIT_TOP_LIGHT + i));
		m_ctlIntensitySpin[i].SetBase(1);
		m_ctlIntensitySpin[i].SetRange(0, 255);
	}

	PostMessage(WM_USER_UPDATEUI, 2, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void CalibrationDlg::OnKillFocusCCD_NozzleEdit(UINT idCtl)
{
}

void CalibrationDlg::OnBnClickedButtonCaculate(UINT idCtl)
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	BOOL bSuccess(FALSE);
	ROI* roi = imaqCreateROI();
	UINT nCount;
	PointFloat fPt;
	BYTE nCamId = idCtl - IDC_BUTTON_CACULATE1;
	Image* CurrentImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	theApp.m_NiVision.GetImage(theApp.m_nCameraID[nCamId], CurrentImg);
	UINT nTool = 12311;
	if (theApp.m_NiVision.CreateROI(CurrentImg, roi,1, nTool))
	{
		nCount = imaqGetContourCount(roi);
		if (nCount)
		{
			ContourInfo2* pInf = imaqGetContourInfo2(roi, imaqGetContour(roi, 0));
			switch (pInf->type)
			{
			case IMAQ_EMPTY_CONTOUR:
				break;
			case IMAQ_POINT:
				fPt.x = pInf->structure.point->x;
				fPt.y = pInf->structure.point->y;
				break;
			case IMAQ_ROTATED_RECT:
				fPt.x = pInf->structure.rotatedRect->left + pInf->structure.rotatedRect->width / 2;
				fPt.y = pInf->structure.rotatedRect->top + pInf->structure.rotatedRect->height / 2;
			default:
				break;
			}
			if (pInf->type == IMAQ_POINT || pInf->type == IMAQ_ROTATED_RECT)
			{
				float temp[2];
				if (theApp.m_NiVision.m_pCalibration[theApp.m_nCameraID[nCamId]])
				{
					TransformReport* pReport = imaqTransformPixelToRealWorld(theApp.m_NiVision.m_pCalibration[theApp.m_nCameraID[nCamId]], &fPt, 1);
					fPt.x = pReport->points[0].x * theApp.m_NiVision.m_dCalibrationUnit[theApp.m_nCameraID[nCamId]];
					fPt.y = pReport->points[0].y * theApp.m_NiVision.m_dCalibrationUnit[theApp.m_nCameraID[nCamId]];
					imaqDispose(pReport);
				}
				temp[0] = theApp.m_cParam.PrjCfg.dCaliPosition[0] - theApp.m_GgMotion.m_dAxisCurPos[0];
				temp[1] = theApp.m_cParam.PrjCfg.dCaliPosition[1] - theApp.m_GgMotion.m_dAxisCurPos[1];

				dOffset[nCamId][0] = (int(1000 * (temp[0] - fPt.x))) / 1000.0;
				dOffset[nCamId][1] = (int(1000 * (temp[1] - fPt.y))) / 1000.0;
				UpdateData(FALSE);
			}
			imaqDispose(pInf);
		}
	}
	imaqDispose(CurrentImg);
	imaqDispose(roi);
}


void CalibrationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	switch (nIDEvent)
	{
	case 1:
		break;
	case 2:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CalibrationDlg::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);
	switch (wParam)
	{
	case 0://
		for (int i = 0; i < 8; i++)
		{
			GetDlgItem(IDC_EDIT_CCD1_OFFSETX+i)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_EXPOSURE1+i)->EnableWindow(bEnable);
			GetDlgItem(IDC_SPIN1+i)->EnableWindow(bEnable);
		}
		GetDlgItem(IDC_BUTTON_CACULATE1)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_CACULATE2)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_CACULATE3)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_CACULATE4)->EnableWindow(bEnable);
		GetDlgItem(ID_BTN_APPLY)->EnableWindow(bEnable);
		GetDlgItem(ID_BTN_CANCEL)->EnableWindow(bEnable);
		break;
	case 1:
		break;
	case 2:
		for (int i = 0; i < 4; i++)
		{
			dOffset[i][0] = theApp.m_cParam.dCCD_Nozzle[i][0];
			dOffset[i][1] = theApp.m_cParam.dCCD_Nozzle[i][1];
			nExposure[i] = theApp.m_cParam.PrjCfg.nExposure[i];
			nIntensity[i] = theApp.m_cParam.PrjCfg.bIntensity[i];
		}
		UpdateData(FALSE);
		break;
	case 3:
		break;
	case 4:
		break;
	}
	return 0;

}


void CalibrationDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	CString Tip = _T("确认修改参数吗？\n");
	if (IDYES == MessageBox(Tip, _T("提示"), MB_YESNO))
	{
		UpdateData();
		for (int i = 0; i < 4; i++)
		{
			theApp.m_cParam.dCCD_Nozzle[i][0] = dOffset[i][0];
			theApp.m_cParam.dCCD_Nozzle[i][1] = dOffset[i][1];
			theApp.m_cParam.PrjCfg.nExposure[i] = nExposure[i];
			theApp.m_cParam.PrjCfg.bIntensity[i] = nIntensity[i];
		}

	}
}


void CalibrationDlg::OnBnClickedBtnCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	PostMessage(WM_USER_UPDATEUI, 2, 0);
}

void CalibrationDlg::OnEnChangeEdit(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (GetDlgItem(idCtl) != this->GetFocus())
		return;
	UpdateData();
	BYTE nIndex;
	switch (idCtl)
	{
	case IDC_EDIT_EXPOSURE1:
	case IDC_EDIT_EXPOSURE2:
	case IDC_EDIT_EXPOSURE3:
	case IDC_EDIT_EXPOSURE4:
		nIndex = (idCtl - IDC_EDIT_EXPOSURE1)>>1;
		if (nExposure[nIndex] < 60000)
			theApp.m_NiVision.Camera_SetExposure(nIndex, nExposure[nIndex]);

		break;
	case IDC_EDIT_LIGHT1:
	case IDC_EDIT_LIGHT2:
	case IDC_EDIT_LIGHT3:
	case IDC_EDIT_LIGHT4:
		nIndex = (idCtl - IDC_EDIT_LIGHT1)>>1;
		if (nIntensity[nIndex] < 255)
			pFrame->LightControllerSet(nIndex, nIntensity[nIndex], 2);
		break;
	default:
		return;
		break;
	}

}


void CalibrationDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnCancel();
}


void CalibrationDlg::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnOK();
}
