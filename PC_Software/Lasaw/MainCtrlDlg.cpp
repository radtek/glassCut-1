// MainCtrlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "MainCtrlDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"


// CMainCtrlDlg 对话框

IMPLEMENT_DYNCREATE(CMainCtrlDlg, CFormView)

CMainCtrlDlg::CMainCtrlDlg(CWnd* pParent /*=NULL*/)
: CFormView(CMainCtrlDlg::IDD)
, bIdling(TRUE)
, bAppointByMu(FALSE)
, bFront(0)
{
}

CMainCtrlDlg::~CMainCtrlDlg()
{
	CreateLabel(0);
}

void CMainCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	for (int i = 0; i < 2; ++i)
		DDX_Control(pDX, IDC_STATIC_CIRCLETIME + i, m_lStatus[i]);
	DDX_Check(pDX, IDC_CHECK_IDLING, bIdling);
	DDX_Radio(pDX, IDC_RADIO_FRONT, bFront);
}


BEGIN_MESSAGE_MAP(CMainCtrlDlg, CFormView)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_FRONT, IDC_RADIO_BACK, OnBnChangePosture)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_PAD_CHECK, IDC_BUTTON_PROCESS, OnBnClickedButtonGetposition)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_AUTO, IDC_BUTTON_STOP, OnBnClickedButtonMain)
	ON_BN_CLICKED(IDC_CHECK_IDLING, &CMainCtrlDlg::OnBnClickedCheckIdling)
	ON_MESSAGE(WM_USER_UPDATEUI, &CMainCtrlDlg::OnUpdateDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_RESET_COUNTER, &CMainCtrlDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_MOTION, &CMainCtrlDlg::OnBnClickedButtonMotion)
END_MESSAGE_MAP()


// CMainCtrlDlg 消息处理程序
void  CMainCtrlDlg::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	return;
}

LRESULT CMainCtrlDlg::OnUpdateDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	// TODO:  在此添加额外的
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CString str;
	UINT i(0), j(0);
	UINT num;
	UINT nIndex = wParam;
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);

	switch (nIndex)
	{
	case 0://
		GetDlgItem(IDC_BUTTON_AUTO)->EnableWindow(bEnable&&theApp.m_nIsHomeEnd);
		GetDlgItem(IDC_BUTTON_MOTION)->EnableWindow(bEnable);
		GetDlgItem(IDC_CHECK_IDLING)->EnableWindow(bEnable);
		GetDlgItem(IDC_BUTTON_PAD_CHECK)->EnableWindow(bEnable&&theApp.m_nIsHomeEnd);
		GetDlgItem(IDC_BUTTON_GETPOSITION)->EnableWindow(bEnable&&theApp.m_nIsHomeEnd);
		GetDlgItem(IDC_RADIO_FRONT)->EnableWindow(bEnable);
		GetDlgItem(IDC_RADIO_BACK)->EnableWindow(bEnable);
		break;
	case 1://
		break;
	case 2:
		m_pStatusdata.resize(8);
		Typesetting(2, 4);
		PostMessage(WM_USER_UPDATEUI, 4, 0);

		break;
	case 3:
		str.Format(_T("%.1f s"), (double)lParam / 1000.0);
		m_lStatus[0].SetText(str);
		break;
	case 4:
		if (lParam)
			theApp.m_cParam.nlQuantity++;
		str.Format(_T("%d"), theApp.m_cParam.nlQuantity);
		m_lStatus[1].SetText(str);
		break;
	case 5:

		break;
	case 6:
		break;
	case 7:////更新指示器状态
		num = m_plStatus.size();
		if (lParam < MAX_ProductNumber && lParam < num)
			i = lParam;
		do
		{
			BYTE bsts;
			COLORREF crbk(0xF0FAFF);
			bsts = m_pStatusdata.at(i);
			m_plStatus.at(i)->FlashBackground(bsts & 0x01);
			switch (bsts >> 1)
			{
			case 0://初始
				break;
			case 1://定位失败
			case 3://部分失败
				crbk = 0x0000FF;
				break;
			case 2://定位成功
				crbk = 0xffff00;
				break;
			case 4://加工完成
				crbk = 0x00ff00;
				break;
			case 5://加工失败
			default:
				crbk = 0x0000FF;
				break;
			}
			m_plStatus.at(i)->SetBkColor(crbk);
			i++;
		} while ((i < num) && (lParam >= MAX_ProductNumber || lParam >= num));
		break;
	}
	return 0;
}

void CMainCtrlDlg::OnBnClickedButtonGetposition(UINT idCtl)
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (!theApp.m_nIsHomeEnd || theApp.m_nCurrentRunMode >> 1)
		return;
	BOOL bSuccess(FALSE);
	theApp.m_nPanePosture = bFront;
	switch (idCtl)
	{
	case IDC_BUTTON_PAD_CHECK: 
		theApp.WorkProcess(CLasawApp::SNAPSHOT_MATCH);
		break;
	case IDC_BUTTON_GETPOSITION:
		theApp.WorkProcess(CLasawApp::PROCESSING_START);
		break;
	}
	GetDlgItem(IDC_BUTTON_PAD_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_GETPOSITION)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_AUTO)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MOTION)->EnableWindow(FALSE);

	SetTimer(3,1500,NULL);
}

void CMainCtrlDlg::OnBnClickedButtonReset()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	theApp.m_cParam.nlQuantity = 0;
	PostMessage(WM_USER_UPDATEUI, 4, 0);
}

void CMainCtrlDlg::OnBnClickedButtonMain(UINT idCtl)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	switch (idCtl)
	{
	case IDC_BUTTON_AUTO:
		if (!theApp.m_nIsHomeEnd||theApp.m_nCurrentRunMode>>1)
			return;
		if (theApp.StartAuto(TRUE))
		{
			theApp.m_nCurrentRunMode|=0x08;
			SetTimer(2, 5500, NULL);
		}
		break;
	case IDC_BUTTON_STOP:
		theApp.SetStop(TRUE);
		break;
	}
}

void CMainCtrlDlg::OnBnClickedCheckIdling()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	UpdateData();
	theApp.m_bIdling = bIdling;
}

void CMainCtrlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	switch (nIDEvent)
	{
	case 2:
		PulseEvent(theApp.m_hOneCycleStart);
		KillTimer(nIDEvent);
		break;
	case 3:
		if (!theApp.m_bSalveThreadAlive)
		{
			GetDlgItem(IDC_BUTTON_PAD_CHECK)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_GETPOSITION)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_AUTO)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_MOTION)->EnableWindow(TRUE);
			KillTimer(nIDEvent);
		}
	}
	CFormView::OnTimer(nIDEvent);
}

void CMainCtrlDlg::OnBnClickedButtonMotion()
{
	// TODO:  在此添加控件通知处理程序代码
	::PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_USER_UPDATEUI, 5, 1);
}

void CMainCtrlDlg::OnBnClickedButtonReview()
{
	// TODO:  在此添加控件通知处理程序代码
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	
}

void CMainCtrlDlg::OnBnClickedCheckQr()
{
	// TODO:  在此添加控件通知处理程序代码
}

BOOL CMainCtrlDlg::PreTranslateMessage(MSG* pMsg)
{
	BOOL bEnable = theApp.GetUserType() && !(theApp.m_nCurrentRunMode >> 1);
	if (bEnable&&!theApp.m_bSalveThreadAlive)
	{
		int buID;
		if (pMsg->message == WM_LBUTTONDBLCLK)//
		{
			buID = GetWindowLong(pMsg->hwnd, GWL_ID);
			SelchangeProduct(buID, FALSE);
		}
		else if (pMsg->message == WM_RBUTTONDBLCLK)
		{
			buID = GetWindowLong(pMsg->hwnd, GWL_ID);
			SelchangeProduct(buID, TRUE);

		}
	}
	return CFormView::PreTranslateMessage(pMsg);
}

void CMainCtrlDlg::SelchangeProduct(UINT idCtl, BOOL breset)
{
	if (idCtl > (IDC_STATIC_STATE_P1 - 1) && idCtl < (IDC_STATIC_STATE_P1 + m_plStatus.size()))
	{
		if (!breset)
		{
			theApp.m_nCurrentPane[0] = (idCtl - IDC_STATIC_STATE_P1) / 4;
			theApp.m_nCurrentPane[1] = (idCtl - IDC_STATIC_STATE_P1) - theApp.m_nCurrentPane[0]*4;
			OnUpdateSts(MAX_ProductNumber, 0x01, 0x02);
			OnUpdateSts(idCtl - IDC_STATIC_STATE_P1, 0x01, 0x01);
		}
		else
			OnUpdateSts();
	}
}

BOOL CMainCtrlDlg::CreateLabel(BYTE nCount)
{
	CString info;
	BOOL bSuccess(TRUE);
	BYTE nSize = m_plStatus.size();
	if (nSize < nCount)
	{
		for (BYTE i = m_plStatus.size(); i < nCount; i++)
		{
			info.Format(_T("\r\n\r\n产\r\n品\r\n%d"), i + 1);
			CLabel* pLabel = new CLabel;
			DWORD dwStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | SS_CENTER | SS_NOTIFY | SS_CENTERIMAGE;
			if (!pLabel || !pLabel->Create(info, dwStyle, CRect(10, 10, 100, 80), this, IDC_STATIC_STATE_P1 + i))
			{
				bSuccess = FALSE;
				break;
			}
			m_plStatus.push_back(pLabel);
			pLabel->SetFontSize(10, FALSE);
			pLabel->SetFontName(_T("微软雅黑"));
			pLabel->SetBkColor(0xF0FAFF);
			pLabel->ShowWindow(TRUE);
		}
	}
	else
	{
		for (BYTE i = nCount; i < nSize; i++)
		{
			SAFE_DELETEDLG(m_plStatus.back());
			m_plStatus.pop_back();
		}
	}
	return bSuccess;

}

void CMainCtrlDlg::Typesetting(BYTE nRow, BYTE nCol)
{
	CRect rc[2];
	UINT nWidth, nHeight;
	UINT nRowstep, nColstep;
	UINT nStart;
	if (nRow*nCol > MAX_ProductNumber || nRow*nCol == 0)
		return;
	if (CreateLabel(nRow*nCol))
	{
		GetDlgItem(IDC_STATIC_PRODUCT)->GetWindowRect(&rc[0]);
		m_plStatus.at(0)->GetWindowRect(&rc[1]);
		rc[0].DeflateRect(5, 35, 0, 10);
		nWidth = rc[0].Width();
		nHeight = rc[0].Height();
		ScreenToClient(rc[0]);
		if (theApp.m_bSortingDir & 0x01)
		{
			nRowstep = nHeight / nRow;
			nColstep = nWidth / nCol;
			rc[1].DeflateRect(0, 0, rc[1].Width() - nColstep * 9.0 / 10, rc[1].Height() - nRowstep*9.0 / 10);
			switch (theApp.m_bSortingDir >> 1)
			{
			case 0:
				/********/
				/* 123  */
				/* 654  */
				/* 789  */
				/********/
				for (int i = 0; i < nRow; i++)
				{
					nStart = rc[0].top + i*nRowstep;
					for (int j = 0; j < nCol; j++)
					{
						if (i & 0x01)
							rc[1].MoveToXY(rc[0].left + nColstep*(nCol - j - 1), nStart);
						else
							rc[1].MoveToXY(rc[0].left + nColstep*j, nStart);
						m_plStatus.at(i*nCol + j)->MoveWindow(rc[1]);
					}
				}
				break;
			case 1:
				/********/
				/* 123  */
				/* 456  */
				/* 789  */
				/********/
				for (int i = 0; i < nRow; i++)
				{
					nStart = rc[0].top + i*nRowstep;
					for (int j = 0; j < nCol; j++)
					{
						rc[1].MoveToXY(rc[0].left + nColstep*j, nStart);
						m_plStatus.at(i*nCol + j)->MoveWindow(rc[1]);
					}
				}
				break;
			case 2:
				/********/
				/* 321  */
				/* 456  */
				/* 987  */
				/********/
				for (int i = 0; i < nRow; i++)
				{
					nStart = rc[0].top + i*nRowstep;
					for (int j = 0; j < nCol; j++)
					{
						if (i & 0x01)
							rc[1].MoveToXY(rc[0].left + nColstep*j, nStart);
						else
							rc[1].MoveToXY(rc[0].left + nColstep*(nCol - j - 1), nStart);
						m_plStatus.at(i*nCol + j)->MoveWindow(rc[1]);
					}
				}
				break;
			case 3:
				/********/
				/* 321  */
				/* 654  */
				/* 987  */
				/********/
				for (int i = 0; i < nRow; i++)
				{
					nStart = rc[0].top + i*nRowstep;
					for (int j = 0; j < nCol; j++)
					{
						rc[1].MoveToXY(rc[0].left + nColstep*(nCol - j - 1), nStart);
						m_plStatus.at(i*nCol + j)->MoveWindow(rc[1]);
					}
				}
				break;
			}
		}
		else
		{
			nRowstep = nWidth / nRow;
			nColstep = nHeight / nCol;
			rc[1].DeflateRect(0, 0, rc[1].Width() - nRowstep + 20, rc[1].Height() - nColstep + 10);
			switch (theApp.m_bSortingDir >> 1)
			{
			case 0:
				/********/
				/* 963  */
				/* 852  */
				/* 741  */
				/********/
				for (int i = 0; i < nRow; i++)
				{
					UINT nStart = rc[0].right - (i + 1)*nRowstep;
					for (int j = 0; j < nCol; j++)
					{
						rc[1].MoveToXY(nStart, rc[0].bottom - nColstep*(j + 1));
						m_plStatus.at(i*nCol + j)->MoveWindow(rc[1]);
					}
				}

				break;
			case 1:
				/********/
				/* 369  */
				/* 258  */
				/* 147  */
				/********/
				for (int i = 0; i < nRow; i++)
				{
					UINT nStart = rc[0].left + i*nRowstep;
					for (int j = 0; j < nCol; j++)
					{
						rc[1].MoveToXY(nStart, rc[0].bottom - nColstep*(j + 1));
						m_plStatus.at(i*nCol + j)->MoveWindow(rc[1]);
					}
				}
				break;
			case 2:
				/********/
				/* 741  */
				/* 852  */
				/* 963  */
				/********/
				break;
			case 3:
				/********/
				/* 147  */
				/* 258  */
				/* 369  */
				/********/
				break;
			}

		}
		InvalidateRect(rc[0]);
	}

}

void CMainCtrlDlg::OnUpdateSts(UINT nNum /*= 100*/, BYTE nSts /*= 0*/, BYTE bMode /*= 0*/)
{
	UINT i(0);
	BYTE bChange(0x01);
	if (nNum >= MAX_ProductNumber || nNum >= m_pStatusdata.size())
	{
		for (i = 0; i < m_pStatusdata.size(); i++)
		{
			switch (bMode)
			{
			case 0://设置
				m_pStatusdata.at(i) = nSts;
				break;
			case 1://加
				m_pStatusdata.at(i) |= nSts;
				break;
			case 2://减
				m_pStatusdata.at(i) ^= (m_pStatusdata.at(i)&nSts);
				break;
			}
		}

	}
	else
	{
		BYTE oldValue = m_pStatusdata.at(nNum);
		switch (bMode)
		{
		case 0://设置
			m_pStatusdata.at(nNum) = nSts;
			break;
		case 1://加
			m_pStatusdata.at(nNum) |= nSts;
			break;
		case 2://减
			m_pStatusdata.at(nNum) ^= (oldValue&nSts);
			break;
		}
		bChange = m_pStatusdata.at(nNum) ^ oldValue;
	}
	if (bChange)
		PostMessage(WM_USER_UPDATEUI, 7, nNum);
}

void CMainCtrlDlg::OnBnChangePosture(UINT idCtl)
{
	UpdateData();
}
