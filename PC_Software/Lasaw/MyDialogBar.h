#pragma once


// CMyDialogBar 对话框

class CMyDialogBar : public CDialogBar
{
	DECLARE_DYNAMIC(CMyDialogBar)

public:
	CMyDialogBar();   // 标准构造函数
	virtual ~CMyDialogBar();
	BOOL						m_bAutoZoom;

// 对话框数据
	enum { IDD = IDD_DIALOGBAR };
	//为导航栏可以动态改变大小，增添以下三个成员变量  
	CSize m_sizeDocked;
	CSize m_sizeFloating;
	BOOL m_bChangeDockedSize;   // Indicates whether to keep a default size for dockin  
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle,
		UINT nID, BOOL = TRUE);
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
		UINT nStyle, UINT nID, BOOL = TRUE);
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
	afx_msg BOOL OnToolTip(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnInitDialog(UINT wParam, LONG lParam); 
	afx_msg void OnBnClickedMenubuttonUser();
	CMenu m_menu;
	CButton m_btTest;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUpdateBtnUI(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedButtonZoom();
};
