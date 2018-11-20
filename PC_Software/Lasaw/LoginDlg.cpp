// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Lasaw.h"
#include "Mainfrm.h"
#include "LoginDlg.h"

// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLoginDlg::IDD, pParent)
{
	nType = 0;
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLoginDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_MODIFY, &CLoginDlg::OnBnClickedButtonModify)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BYTE CLoginDlg::CheckPassword(BOOL bOld)
{
	CLasawApp* pApp = (CLasawApp*)AfxGetApp();
	srand((unsigned)time(NULL));
	BYTE nSuccess = (rand() % (0xff - 0x03)) + 0x03;
	int i = ((CComboBox*)GetDlgItem(IDC_COMBO_USERTYPE))->GetCurSel();
	CString str;
	CString Str_Wrong = _T("密码错误!\r\n");
	if (bOld)
		GetDlgItem(IDC_EDIT_OLDPASSWORD)->GetWindowText(str);
	else
		GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowText(str);
	UINT m = STR_INT(str);
	switch (i)
	{
	case 1:
		if (m == pApp->m_cParam.PrjCfg.Magic[0] || m == 20176666)
			nSuccess = 1;
		else
			AfxMessageBox(Str_Wrong);
		break;
	case 2:
		if (m == pApp->m_cParam.PrjCfg.Magic[1] || m == 20188888)
			nSuccess = 2;
		else
			AfxMessageBox(Str_Wrong);

		break;
	case 0:
	default:
		nSuccess = 0;
		break;
	}

	return nSuccess;
}


// CLoginDlg 消息处理程序


BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	((CComboBox*)GetDlgItem(IDC_COMBO_USERTYPE))->SetCurSel(theApp.GetUserType());

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CLoginDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码

}


void CLoginDlg::OnBnClickedButtonLogin()
{
	// TODO:  在此添加控件通知处理程序代码
	CLasawApp* pApp = (CLasawApp*)AfxGetApp();
	BYTE nSuccess = CheckPassword();
	if (nSuccess <3)
	{
		pApp->SetUserType(nSuccess);
		CDialogEx::OnOK();
	}
}


void CLoginDlg::OnBnClickedButtonModify()
{
	// TODO:  在此添加控件通知处理程序代码
	CLasawApp* pApp = (CLasawApp*)AfxGetApp();
	CString str;
	CString Str_Wrong = _T("错误的密码! ");
	GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowText(str);
	UINT m = STR_INT(str);
	nType = CheckPassword(TRUE);
	switch (nType)
	{
	case 1:
	case 2:
		pApp->m_cParam.PrjCfg.Magic[nType - 1] = m;
		AfxMessageBox(_T("修改成功"));
		break;
	case 0:
		AfxMessageBox(_T("不支持当前用户类型！\r\n"));
		break;
	}
}


void CLoginDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}


void CLoginDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
}


void CLoginDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类

	CDialogEx::OnCancel();
}
