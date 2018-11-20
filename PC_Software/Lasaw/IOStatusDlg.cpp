// IOStatusDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "IOStatusDlg.h"
#include "MainFrm.h"

// CIOStatusDlg 对话框

IMPLEMENT_DYNAMIC(CIOStatusDlg, CDialogEx)

CIOStatusDlg::CIOStatusDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CIOStatusDlg::IDD, pParent)
{
	m_hRedBitmap = NULL;
	m_hGreenBitmap = NULL;
	nOuptSts = 0;
}

CIOStatusDlg::~CIOStatusDlg()
{
	TRACE0("done!\n");
}

void CIOStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	for (int i = 0; i < 12;++i)
		DDX_Control(pDX, IDC_STATIC_XPLIMIT + i, m_lStatus[i]);

}


BEGIN_MESSAGE_MAP(CIOStatusDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK00, IDC_CHECK17, OnOutPut)
END_MESSAGE_MAP()


// CIOStatusDlg 消息处理程序


BOOL CIOStatusDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CMainFrame* pFrame=(CMainFrame*)AfxGetMainWnd();

	int i;
	CString sz;
	for (i=0;i<16;i++)
	{
		sz.Format(_T("%-2d"), i);
		SetDlgItemText(IDC_CHECK00 + i, sz);
	}

	CStatic* pStatic=NULL;
	pStatic = (CStatic*)GetDlgItem(IDC_PICT01);
	m_hRedBitmap = pStatic->GetBitmap();
	pStatic = (CStatic*)GetDlgItem(IDC_PICT00);
	m_hGreenBitmap = pStatic->GetBitmap();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CIOStatusDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	BOOL bBitState = FALSE;
	CStatic* pState = NULL;
	CButton* pBtn = NULL;
	LONG uOutValue(0);
	static LONG uoldValue(0x0ff);
	int i = 0;
	UINT nAlarm;
	switch (nIDEvent)
	{
	case 3:
		theApp.m_GgMotion.GetInput(ALL_CH, uOutValue, 8);
		uOutValue =0x0ffff& ~uOutValue;
		nOuptSts = uOutValue;
		for (i = 0; i < 16; i++)
		{
			pState = (CStatic*)GetDlgItem(IDC_PICT00 + i);
			bBitState = theApp.m_GgMotion.m_bInput[i];
			pState->SetBitmap(bBitState ? m_hGreenBitmap : m_hRedBitmap);
			CRect rc;
			pState->GetClientRect(&rc);
			InvalidateRect(&rc, false);
			if (uoldValue^uOutValue)
			{
				pBtn = (CButton*)GetDlgItem(IDC_CHECK00 + i);
				pBtn->SetCheck(uOutValue >> i & 0x01);
			}
		}
		nAlarm = 0x0f & theApp.m_GgMotion.m_nAlarm;
		for (i = 0; i < 3; i++)
		{
			UINT nStatus(0);
			theApp.m_GgMotion.GetSensorState(ALL_AXES, i, nStatus);
			for (int j = 0; j < 2; j++)//XY
				m_lStatus[i + 3 * j].SetBkColor((nStatus >> j & 0x01) ? CLasawApp::DarkRed : CLasawApp::DarkGreen);
		}
		m_lStatus[9].SetBkColor((nAlarm & 0x01) ? CLasawApp::DarkRed : CLasawApp::DarkGreen);
		m_lStatus[10].SetBkColor((nAlarm >> 1 & 0x01) ? CLasawApp::DarkRed : CLasawApp::DarkGreen);

		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CIOStatusDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	KillTimer(3);
	CDialogEx::OnClose();
}

void CIOStatusDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (bShow)
	{
		if (theApp.m_nHwReady)
			SetTimer(3, 400, NULL);
	}
	else
		KillTimer(3);

}

void CIOStatusDlg::OnOutPut(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	UINT nPort = idCtl - IDC_CHECK00;
	theApp.m_GgMotion.SetOutput(nPort + 1, 0x01 & (~nOuptSts >> nPort));
}



void CIOStatusDlg::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类

// 	CDialogEx::OnOK();
}
