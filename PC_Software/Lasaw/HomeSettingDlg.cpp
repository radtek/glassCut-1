// HomeSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lasaw.h"
#include "HomeSettingDlg.h"
#include "MainFrm.h"


// CHomeSettingDlg 对话框

IMPLEMENT_DYNAMIC(CHomeSettingDlg, CDialog)

CHomeSettingDlg::CHomeSettingDlg(CWnd* pParent /*=NULL*/)
: CDialog(CHomeSettingDlg::IDD, pParent)
{
}

CHomeSettingDlg::~CHomeSettingDlg()
{
}

void CHomeSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}


BEGIN_MESSAGE_MAP(CHomeSettingDlg, CDialog)
END_MESSAGE_MAP()

BOOL CHomeSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	int i(0);
	// TODO:  在此添加额外的初始化
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
