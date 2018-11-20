//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2011 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// MultiLineHeaderGridCtrl.h : header file
//

#if !defined(AFX_MULTILINEHEADERGRIDCTRL_H__56170B45_D9F3_4EA8_A6B5_C53C2308F22C__INCLUDED_)
#define AFX_MULTILINEHEADERGRIDCTRL_H__56170B45_D9F3_4EA8_A6B5_C53C2308F22C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPGridCtrl.h"

//////////////////////////////////////////////////////////////////////
// CDoubleItem Class

class CPasswordItem : public CBCGPGridItem
{
public:
	CPasswordItem (const CString& strPassword);

	virtual void WriteToArchive(CArchive& ar);

protected:
	virtual CString FormatItem ();
	virtual CWnd* CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat);
};
/////////////////////////////////////////////////////////////////////////////
// CMultiLineHeaderGridCtrl - sample of custom header

class CMultiLineHeaderGridCtrl : public CBCGPGridCtrl
{
	DECLARE_DYNCREATE(CMultiLineHeaderGridCtrl)

// Construction
public:
	CMultiLineHeaderGridCtrl();
	virtual ~CMultiLineHeaderGridCtrl();

protected:
	virtual BOOL OnTrackHeader ()
	{
		return FALSE; // do not resize columns inside client area by mouse, only at header
	}

// Merged header items:
public:
	virtual void OnPosSizeChanged ();
	BOOL New(const LPnode& pPt, int nIndex = 0);
	virtual BOOL Delete ();
	virtual void OnItemChanged(CBCGPGridItem*pItem,int nRow,int nColumn);


protected:
	virtual CRect OnGetHeaderRect (CDC* pDC, const CRect& rectDraw);
	virtual void OnDrawHeader (CDC* pDC);
	virtual void OnPrintHeader(CDC* pDC, CPrintInfo* pInfo);

public:
	virtual CBCGPGridColumnsInfo& GetColumnsInfo ()
	{
		return m_ColumnsEx;
	}
	
	virtual const CBCGPGridColumnsInfo& GetColumnsInfo () const
	{
		return m_ColumnsEx;
	}

	// Generated message map functions
protected:
	//{{AFX_MSG(CMultiLineHeaderGridCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CBCGPGridColumnsInfoEx	m_ColumnsEx;	// other header implementation
	CImageList				m_ImagesHeader;
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_MULTILINEHEADERGRIDCTRL_H__56170B45_D9F3_4EA8_A6B5_C53C2308F22C__INCLUDED_)
