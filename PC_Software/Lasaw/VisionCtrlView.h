#pragma once



// CVisionCtrlView 窗体视图

class CVisionCtrlView : public CDialogEx
{
	DECLARE_DYNAMIC(CVisionCtrlView)

protected:
	Image*				m_pReferenceImg;
public:
	CVisionCtrlView(CWnd* pParent = NULL);           // 
	virtual ~CVisionCtrlView();
	enum { IDD = IDD_DIALOG_VISIONCTRL };

	BOOL	m_bNewRefImg;
	BOOL	m_bNewRefAngle;
	BOOL	m_bNewTemp;

	BYTE	m_nSSStrength;
	BYTE	m_ndir;
	UINT	m_nTMScore;
	UINT	m_nSSGap;
	BOOL	m_bUseAngleCalib;
	BOOL	m_bUseAngleRel;
	BOOL	m_bUseBinary;
	BOOL	m_bUseMask;
	float   m_fAngleRef;

	Rect	m_Searchrc;
	RotatedRect m_SearchLine;
	Rect	m_Trc;

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
public:
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKillFocusTMEdit();
	afx_msg void OnKillFocusSSEdit(UINT idCtl);
	afx_msg void OnImageEditTool(UINT idCtl);
	afx_msg void OnCbnSelchangeComboSearchDir(UINT idCtl);
	afx_msg void OnBnClickedButtonMatch();
	afx_msg void OnBnClickedButtonFindtest();
	afx_msg void OnBnClickedCheckBinary();
	afx_msg void OnBnClickedCheckAngle(UINT idCtl);
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonRegister();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonTool();
//	afx_msg void OnClose();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnBnClickedBtnApply();
};


