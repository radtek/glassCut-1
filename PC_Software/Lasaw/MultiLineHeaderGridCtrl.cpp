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
// MultiLineHeaderGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MultiLineHeaderGridCtrl.h"

#include "lasaw.h"
#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Custom Header
//////////////////////////////////////////////////////////////////////
// CPasswordItem Class

const TCHAR cPassword = _T('*');

CPasswordItem::CPasswordItem (const CString& strPassword) :
	CBCGPGridItem ((LPCTSTR) strPassword)
{
}
//******************************************************************************
CWnd* CPasswordItem::CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat)
{
	CEdit* pWndEdit = new CEdit;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_PASSWORD;

	if (!m_bEnabled || !m_bAllowEdit)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->Create (dwStyle, rectEdit, GetOwnerList(), BCGPROPLIST_ID_INPLACE);
	pWndEdit->SetPasswordChar (cPassword);

	bDefaultFormat = TRUE;
	return pWndEdit;
}
//******************************************************************************
CString CPasswordItem::FormatItem ()
{
	CString strVal = (LPCTSTR)(_bstr_t)m_varValue;
	
	for (int i = 0; i < strVal.GetLength (); i++)
	{
		strVal.SetAt (i, cPassword);
	}

	return strVal;
}
//******************************************************************************
void CPasswordItem::WriteToArchive(CArchive& ar)
{
	if (m_varValue.vt != VT_BSTR)
	{
		CBCGPGridItem::WriteToArchive (ar);
		return;
	}

	// Copying real item's value is not allowed.
	// Serialize all visible characters using the password character
	_variant_t varOld = m_varValue;
	m_varValue = (LPCTSTR) FormatItem ();
	
	CBCGPGridItem::WriteToArchive (ar);
	
	m_varValue = varOld;
}
//******************************************************************************

CRect CMultiLineHeaderGridCtrl::OnGetHeaderRect (CDC* pDC, const CRect& rectDraw)
{
	CRect rect = CBCGPGridCtrl::OnGetHeaderRect (pDC, rectDraw);
	rect.bottom = rect.top + rect.Height () * m_ColumnsEx.GetHeaderLineCount ();

	return rect;
}

void CMultiLineHeaderGridCtrl::OnDrawHeader (CDC* pDC)
{
	m_ColumnsEx.PrepareDrawHeader ();

	CBCGPGridCtrl::OnDrawHeader (pDC);
}

void CMultiLineHeaderGridCtrl::OnPrintHeader(CDC* pDC, CPrintInfo* pInfo)
{
	m_ColumnsEx.PreparePrintHeader ();

	CBCGPGridCtrl::OnPrintHeader(pDC, pInfo);
}

void CMultiLineHeaderGridCtrl::OnPosSizeChanged ()
{
	CBCGPGridCtrl::OnPosSizeChanged ();

	m_ColumnsEx.ReposHeaderItems ();
}

/////////////////////////////////////////////////////////////////////////////
// CMultiLineHeaderGridCtrl

IMPLEMENT_DYNCREATE(CMultiLineHeaderGridCtrl, CBCGPGridCtrl)

CMultiLineHeaderGridCtrl::CMultiLineHeaderGridCtrl()
{
}

CMultiLineHeaderGridCtrl::~CMultiLineHeaderGridCtrl()
{
}

BEGIN_MESSAGE_MAP(CMultiLineHeaderGridCtrl, CBCGPGridCtrl)
	//{{AFX_MSG_MAP(CMultiLineHeaderGridCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
// 	ON_WM_CONTEXTMENU()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Methods:

int CMultiLineHeaderGridCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CBCGPGridRow::m_strFormatDouble = _T("%.3lf");
	CBCGPGridRow::m_strFormatFloat = _T("%.1f");
	if (CBCGPGridCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetScrollBarsStyle (CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER);
	EnableLineNumbers ();
	SetClearInplaceEditOnEnter (FALSE);

	EnableHeader (TRUE/*, BCGP_GRID_HEADER_SELECT*/);
	EnableRowHeader (TRUE, /*BCGP_GRID_HEADER_SELECT|*/BCGP_GRID_HEADER_MOVE_ITEMS);
	
	SetEditFirstClick (FALSE);
	SetRowHeaderWidth(33, FALSE);

	#pragma warning (disable : 4245)
	BCGP_GRID_COLOR_DATA theme = 
	{
		RGB (190, 250, 190),
		RGB (70, 22, 194),
		-1,
		{-1, -1, -1, 0, -1},
		{-1, -1, -1, 0, -1},
		{RGB (210, 230, 249), RGB (16, 37, 127), RGB (228, 241, 251), 90, -1},
		{RGB (250, 199, 97), RGB (0, 0, 0), -1, 0, RGB (192, 192, 192)},
		{RGB (100, 100, 255), RGB (70, 22, 194), -1, 0, RGB (192, 192, 192)},
		{-1, -1, -1, 0, -1},
		{-1, -1, -1, 0, -1},
		{RGB (207, 221, 240), -1, RGB (227, 234, 244), 90, RGB (136, 176, 228)},
		RGB (190, 142, 224),
		RGB (190, 142, 224),
		-1,
	};
	#pragma warning (default : 4245)
	SetColorTheme(theme);


	m_ColumnsEx.SetHeaderLineCount (3);

	CArray<int, int> arrMergeColumns;
	arrMergeColumns.Add (1);//类型：线、弧
	arrMergeColumns.Add (2);//点坐标x
	arrMergeColumns.Add (3);//点坐标y
	CArray<int, int> arrMergeLines;
	arrMergeLines.Add (0);
	m_ColumnsEx.AddHeaderItem (&arrMergeColumns, &arrMergeLines, -1/*1*/, _T("轨迹点"), HDF_CENTER);

	CArray<int, int> arrMergeColumns1;
	arrMergeColumns1.Add (4);//方向
	arrMergeColumns1.Add (5);//辅助x
	arrMergeColumns1.Add (6);//辅助y

	CArray<int, int> arrMergeLines1;
	arrMergeLines1.Add (0);
	m_ColumnsEx.AddHeaderItem (&arrMergeColumns1, &arrMergeLines1, -1/*13*/, _T("圆弧"), HDF_CENTER);

	CArray<int, int> arrMergeColumns2;
	arrMergeColumns2.Add (5);
	arrMergeColumns2.Add (6);
	CArray<int, int> arrMergeLines2;
	arrMergeLines2.Add (1);

	m_ColumnsEx.AddHeaderItem (&arrMergeColumns2, &arrMergeLines2, -1, _T("辅助点"), HDF_CENTER);

	//-------------
	// Add columns:
	//-------------
	LPCTSTR arrColumns [] = {_T("出光"),	_T("类型"),_T("X"), _T("Y"),
		_T("方向"), _T("偏移量X"), _T("偏移量Y")};
	const int nColumns = sizeof (arrColumns) / sizeof (LPCTSTR);
	int nWith[] = {35,55,70,70,65,70,70};
	for (int nColumn = 0; nColumn < nColumns; nColumn++)
	{		
		InsertColumn (nColumn, arrColumns[nColumn], nWith[nColumn]);
		SetHeaderAlign (nColumn, HDF_CENTER);
		SetColumnAlign (nColumn, HDF_CENTER);
		SetColumnLocked(nColumn);
	}	
	AdjustLayout ();
	return 0;
}

void CMultiLineHeaderGridCtrl::OnItemChanged(CBCGPGridItem*pItem,int nRow,int nColumn)
{
	BOOL bEnable(FALSE);
	CString sType;
	USHORT i(0);
	switch (nColumn)
	{
	case 0:
		bEnable = pItem->GetValue() ? TRUE : FALSE;
		pItem->SetBackgroundColor(bEnable ? -1 : CBCGPColor::Gray);
		pItem->SetTextColor(bEnable ? -1 : CBCGPColor::LightGray);
		break;
	case 1:
		sType = pItem->GetValue();
		bEnable = sType==_T("弧线")?TRUE:FALSE;
		for (i=4;i!=7;i++)
		{
			pItem = pItem->GetParentRow()->GetItem(i);
			pItem->AllowEdit(bEnable);
			pItem->SetBackgroundColor(bEnable?-1:CBCGPColor::Gray);
			pItem->SetTextColor(bEnable?-1:CBCGPColor::LightGray);
			pItem->Enable(bEnable);
		}
		break;
	}
}

void CMultiLineHeaderGridCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here
	if (point.x == -1 && point.y == -1 && this != NULL)
	{
		CBCGPGridItem* pItem = GetCurSelItem ();
		if (pItem != NULL)
		{
			CRect rectItem = pItem->GetRect ();
			CPoint ptItem (rectItem.left, rectItem.bottom + 1);

			if (GetListRect ().PtInRect (ptItem))
			{
				point = ptItem;
				ClientToScreen (&point);
			}
		}
	}
	CMenu menu;
	menu.LoadMenuW(IDR_CONTEXT_POPMUNE);
	CMenu *pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
}

BOOL CMultiLineHeaderGridCtrl::Delete()
{
	m_bWholeRowSel = TRUE;
	m_pSerializeManager->SetClipboardFormatType (CBCGPGridSerializeManager::CF_Rows);
	CBCGPGridCtrl::Delete();
	m_bWholeRowSel = FALSE;
	m_pSerializeManager->SetClipboardFormatType (CBCGPGridSerializeManager::CF_Items);
	return TRUE;
}

BOOL CMultiLineHeaderGridCtrl::New(const LPnode& pPt, int nIndex)
{
	CBCGPGridRow* pRow = CreateRow (GetColumnCount ());
	ASSERT_VALID (pRow);

	int i;
	BOOL bEnable = pPt.bLaserOn;
	CBCGPGridItem* pItem = new CBCGPGridCheckItem ( bEnable? true : false);
	pRow->ReplaceItem (0, pItem);

	LPCTSTR arrDroplist[] = { _T("直线"), _T("弧线"),_T("顺时针"), _T("逆时针") };

	for (i = 0; i < 2; i++)
		pRow->GetItem (2+i)->SetValue ((_variant_t)pPt.EndPoint[i]);

	pItem = new CBCGPGridItem(arrDroplist[2 + pPt.nDir]);
	pItem->SetReadOnly(0x02>>pPt.uType);
	for (i = 2; i < 4; i++)
		pItem->AddOption(arrDroplist[i]);
	pRow->ReplaceItem(4, pItem);

	pItem = new CBCGPGridItem(arrDroplist[pPt.uType >> 1]);
	pItem->SetReadOnly(TRUE);
	for (i = 0; i < 2; i++)
		pItem->AddOption(arrDroplist[i]);
	pRow->ReplaceItem(1, pItem);
	OnItemChanged(pItem, 1, 1);


	for (i = 0; i < 2; i++)
		pRow->GetItem (5+i)->SetValue ((_variant_t)pPt.CenterPt[i]);
	if (!nIndex)
	{
		CBCGPGridRow* pSelcRow = GetCurSel ();
		if (pSelcRow)
			nIndex = pSelcRow->GetRowId()+1;
	}
	if (!nIndex)
		AddRow (pRow, TRUE);
	else
		InsertRowBefore(nIndex-1,pRow);
	return TRUE;
}

void CMultiLineHeaderGridCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
// 	CBCGPSliderCtrl* pItemSliderCtrl = DYNAMIC_DOWNCAST
// 		(CBCGPSliderCtrl, pScrollBar);
// 
// 	if (pItemSliderCtrl != NULL)
// 	{
// 		CWnd::OnHScroll (nSBCode, nPos, pScrollBar);
// 		pItemSliderCtrl->Redraw ();
// 	}
// 	else
// 	{

		CBCGPGridCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
	AdjustLayout ();
//	}
}
