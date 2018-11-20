#pragma once
#include "afxshelltreectrl.h"
#include "afxvslistbox.h"


// DxfDlg 对话框

class DxfDlg : public CDialogEx
{
	DECLARE_DYNAMIC(DxfDlg)
	int m_nCurrntTool;
	BOOL m_bFlashLocus;
	UINT m_nFlashLocus;
	RGBValue DrawColor[8];

public:
	DxfDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DxfDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_DXF };
	enum { BASIC_INFO_LAYER = 0, ASSISANT_INFO_LAYER, LINE_LOCUS_LAYER, ARC_LOCUS_LAYER, ALL_LOCUS_LAYER, FIRST_LOCUS_LAYER };
	Image* m_Img;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	void DrawLocus();
	void DrawLocus(UINT nIndex);
	void CreateCanvas(Image* &Img,CRect AreaRc);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonExport(UINT idCtl);
	afx_msg void OnCbnSelchangeComboLOCUS(UINT idCtl);
	afx_msg void OnBnClickedButtonDir(UINT idCtl); 
	float m_fProportion;
	BYTE m_nMin_Angle;
	DWORD m_nSpline_Step;
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnBnClickedBtnApply();
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonZoom();
	afx_msg void OnBnClickedButtonPan();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	BOOL m_bDir;
	CVSListBox m_ListLocus;
	afx_msg void OnBnClickedButtonReverse();
	afx_msg void OnBnClickedButtonSplice();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnCancel();
};
