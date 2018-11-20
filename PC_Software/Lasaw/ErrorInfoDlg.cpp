// ErrorInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Lasaw.h"
#include "ErrorInfoDlg.h"


// CErrorInfoDlg 对话框

IMPLEMENT_DYNAMIC(CErrorInfoDlg, CBCGPDialog)

CErrorInfoDlg::CErrorInfoDlg(CWnd* pParent /*=NULL*/)
: CBCGPDialog(CErrorInfoDlg::IDD, pParent)
{

}
CStringList  CErrorInfoDlg::m_cListErr;
CCriticalSection	CErrorInfoDlg::m_Critical;

CErrorInfoDlg::~CErrorInfoDlg()
{
}

void CErrorInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ERRORLIST, m_lErrorList);
	DDX_Control(pDX, IDC_EDIT_ERRORLIST, m_lErrorListBox);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_lTitle);
}


BEGIN_MESSAGE_MAP(CErrorInfoDlg, CBCGPDialog)
	ON_BN_CLICKED(IDOK, &CErrorInfoDlg::OnBnClickedOk)
	ON_MESSAGE(WM_USER_SHOWERRORINFO, OnErrorInfo)
END_MESSAGE_MAP()


// CErrorInfoDlg 消息处理程序


void CErrorInfoDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	if (!m_cListErr.IsEmpty())
		m_cListErr.RemoveAll();
// 	m_lErrorList.SetText(_T(""));
	m_lErrorListBox.SetWindowText(_T(""));
	ShowWindow(SW_HIDE);
	m_lTitle.FlashText(FALSE);
	DestroyWindow();
// 	CDialogEx::OnOK();
}

LRESULT CErrorInfoDlg::OnErrorInfo(WPARAM wparam, LPARAM lparam)
{
	ShowErrorInfo();
	ShowWindow(SW_SHOW);
	SetActiveWindow();
// 	m_lTitle.FlashText(TRUE);
	m_lTitle.FlashBackground(TRUE);
	return 0;
}

UINT CErrorInfoDlg::AddedErrorMsg(CString Info)
{
	ScopedLocker<CCriticalSection> ll(m_Critical);
	if (m_cListErr.Find(Info) == NULL)
		m_cListErr.AddTail(CString(Info));
	return 0;
}

void CErrorInfoDlg::ShowErrorInfo()
{
	CString ErrDetail("");
	POSITION pos = m_cListErr.GetHeadPosition();
	for (int i = 0; (pos != NULL) && (i < m_cListErr.GetCount()); i++)
	{
		ErrDetail += m_cListErr.GetNext(pos);
	}
// 	ErrDetail += _T("\r\n");
// 	m_lErrorList.SetText(ErrDetail);
	m_lErrorListBox.SetWindowText(ErrDetail);
}


BOOL CErrorInfoDlg::OnInitDialog()
{
	CBCGPDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lTitle.SetFontSize(44);
	m_lTitle.SetFontBold(TRUE);
	m_lTitle.SetFont3D(TRUE);
	m_lTitle.SetText3DHiliteColor(0x3ffdff);
	m_lTitle.SetTextColor(0x90EE90);
	// 	m_lTitle.FlashText(TRUE);
	m_lErrorList.SetTextColor(0x0000ff);
	m_lTitle.SetBkColor(0x0000ff);
	m_lErrorList.SetBkColor(0xffffff);
	m_lErrorList.ShowWindow(SW_HIDE);
	SetBackgroundColor(0xffffff);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
