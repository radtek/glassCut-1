// CustomStatusBar.cpp : implementation file
//

/////////////////////////////////////////////////////////////////////////////
//
// Author: Dan Salvano
// Copyright ?2005, Dan Salvano
//
// This code provided "AS IS," without warranty of any kind.
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 2005/06	Initial Release.
//
/////////////////////////////////////////////////////////////////////////////
//
// Replacement class for CStatusBar that can fade the text using AlphaBlend.
//
// To use, substitute CCustomStatusBar for CStatusBar in your declaration and
// make all the panes Owner Draw.  Call ManageTimer(true) to have the class manage
// the fading for you or ManageTimer(false) to manage the fading yourself.  Set the
// text color and background using the class API's.  See the header file for a
// full description of the functions.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "Lasaw.h"
#include "CustomStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int FADE_TIMER_ID			= 1000;		// Identifier for our Fade Timer
const int FADE_TIME_INTERVAL	= 100;		// Time out interval for the timer
const int FADE_STEP				= 25;		// Amount to step in the fade
const BYTE MAX_ALPHA			= 0xFF;		// Max Alpha value		

/////////////////////////////////////////////////////////////////////////////
// CCustomStatusBar

CCustomStatusBar::CCustomStatusBar()
{
	int nI = 0;

	// Initialize everything
	for (nI=0; nI<MAX_NUM_OF_PANES; nI++)
	{
		m_byAlpha[nI]					= MAX_ALPHA;
		m_bBounce[nI]					= true;
		m_bFadeOut[nI]					= false;
		m_bFlashOn[nI]					= false;
		m_clrPaneBackgroundColor[nI]	= ::GetSysColor(COLOR_3DFACE);
		m_clrPaneTextSourceColor[nI]	= ::GetSysColor(COLOR_BTNTEXT);
		m_clrPaneTextDestColor[nI]		= ::GetSysColor(COLOR_3DFACE);
	}

	m_hInst = NULL;
	m_pProcAlphaBlend = NULL;

	// Verify Alpha Blend is present
	m_hInst = LoadLibrary(_T("Msimg32.dll")); 
	m_pProcAlphaBlend = (FARPROC)GetProcAddress(m_hInst, "AlphaBlend"); 
}

CCustomStatusBar::~CCustomStatusBar()
{
	// Free up the library
	if (m_hInst)
	{
		FreeLibrary(m_hInst);
		m_hInst = NULL;
	}
}

int CCustomStatusBar::CalculateWidthForText(CString strText)
{
	// Adjust the width of the status bar pane to ensure the text is visible
	int			nI = 0;
	CString		str;
	CSize		sz;
	TEXTMETRIC	tm;
	int			dx=0;
	CDC*		pDC = GetDC();
	CFont*		pFont = GetFont();

	// Select the listbox font, save the old font
	CFont* pOldFont = pDC->SelectObject(pFont);

	// Get the text metrics for avg char width
	pDC->GetTextMetrics(&tm);

	sz = pDC->GetTextExtent(strText);

	// Add the avg width to prevent clipping
	sz.cx += tm.tmAveCharWidth;

	dx = max(sz.cx, dx);

	// Select the old font back into the DC
	pDC->SelectObject(pOldFont);

	ReleaseDC(pDC);

	return dx;
}

void CCustomStatusBar::AlphaDraw(CDC* pDC, CString strText, int nPaneNumber, CRect* pRect)
{
	CDC dcMemBackground;					// Memory DC for the background
	CDC dcMemText;							// Memory DC for the text
	CBitmap bmpBackground;					// Bitmap for dcMemBackground
	CBitmap bmpText;						// Bitmap for dcMemText
	CRect rect;								// Drawing area
	CBitmap* pbmpOldBackground	= NULL;		// Bitmap originally found in dcMemBackground
	CBitmap* pbmpOldText		= NULL;		// Bitmap originally found in dcMemText

	// Get the pane dimensions
	if (pRect)
	{
		rect.CopyRect(pRect);
	}
	else
	{
		pDC->GetClipBox(&rect);
	}

	// We need two separate DC's.  One is for the background and the other is
	// for the text.  We will do an AlphaBlend of these two and then blt
	// the result back to the screen.
	
	// Create a compatible memory DC for the background.  
	dcMemBackground.CreateCompatibleDC(pDC);
	bmpBackground.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	pbmpOldBackground = dcMemBackground.SelectObject(&bmpBackground);
	dcMemBackground.SetWindowOrg(rect.left, rect.top);
	
	// Create a compatible memory DC for the text
	dcMemText.CreateCompatibleDC(pDC);
	bmpText.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	pbmpOldText = dcMemText.SelectObject(&bmpText);
	dcMemText.SetWindowOrg(rect.left, rect.top);
	
	// First, paint the background of the dcMemBackground and the dcMemText.  This
	// is so the AlphaBlend only affects the text
	dcMemBackground.FillSolidRect(rect, m_clrPaneBackgroundColor[nPaneNumber]);
	dcMemText.FillSolidRect(rect, m_clrPaneBackgroundColor[nPaneNumber]);
	
	// This is so the text does not hide the background
	dcMemBackground.SetBkMode(TRANSPARENT);
	dcMemText.SetBkMode(TRANSPARENT);

	// Set the Font
	dcMemBackground.SelectObject(GetFont());
	dcMemText.SelectObject(GetFont());

	// Set the source text color.  This is the color we are blending from.
	dcMemText.SetTextColor(m_clrPaneTextSourceColor[nPaneNumber]);

	// Set the destination text color.  This is the color we are blending to.
	dcMemBackground.SetTextColor(m_clrPaneTextDestColor[nPaneNumber]);

	// Write the text.  The +1 is to get the text off the border
	dcMemBackground.TextOut(rect.left+1, rect.top, strText);
	dcMemText.TextOut(rect.left+1, rect.top, strText);

	// At this point we have 2 DC's that are identical except the color of the
	// text in them may be different.  This is what we will be AlphaBlending together.

	// Set the blend parameters.
	BLENDFUNCTION pixelBlend = { AC_SRC_OVER, 0, m_byAlpha[nPaneNumber], 0 };
	int nWidth = rect.Width();
	int nHeight = rect.Height();
	if (nWidth > 0 && nHeight > 0)
	{
		// Is AlphaBlend supported?
		if (m_pProcAlphaBlend)
		{
			// Go ahead and blend
			if (!AlphaBlend(dcMemBackground.GetSafeHdc(),
				rect.left,
				rect.top,
				nWidth,
				nHeight,
				dcMemText.GetSafeHdc(),
				rect.left,
				rect.top,
				nWidth,
				nHeight,
				pixelBlend))
			{
				CString strErr;
				strErr.Format(_T("AlphaBlend Failed with error #%X."), GetLastError());
				AfxMessageBox(strErr);
			}

			// Blt the blended dc back so the caller can display it.
			pDC->BitBlt(rect.left,
				rect.top,
				nWidth,
				nHeight,
				&dcMemBackground,
				rect.left,
				rect.top,
				SRCCOPY);
		}
		else
		{
			// Alpha blend not supported, give the user the text with no blending.
			pDC->BitBlt(rect.left,
				rect.top,
				rect.Width(),
				rect.Height(),
				&dcMemText,
				rect.left,
				rect.top,
				SRCCOPY);
		}
	}
	// Clean up
	dcMemBackground.SelectObject(pbmpOldBackground);
	dcMemText.SelectObject(pbmpOldText);
}

void CCustomStatusBar::ManageTimer(bool bEnable)
{
	// Turn the timer on or off
	if (bEnable)
	{
		SetTimer(FADE_TIMER_ID, FADE_TIME_INTERVAL, NULL);
	}
	else
	{
		KillTimer(FADE_TIMER_ID);
	}
}

bool CCustomStatusBar::IsValidPaneNumber(int nPane)
{
	CRect rect;

	return GetStatusBarCtrl().GetRect(nPane, &rect) ? true : false;
}

bool CCustomStatusBar::InvalidatePane(int nPane)
{
	// Force a redraw of a pane

	bool bReturn = true;
	CRect rect;
	CString strText;

	if (!GetStatusBarCtrl().GetRect(nPane, &rect))
	{
		return false;
	}

	strText = GetPaneText(nPane);
	rect.DeflateRect(1,1);

	CDC* pDC = GetDC();
	AlphaDraw(pDC, strText, nPane, &rect);
	ReleaseDC(pDC);

	return true;
}

void CCustomStatusBar::SetPaneTextSourceColor(int nPane, COLORREF clrText)
{
	if (!IsValidPaneNumber(nPane))
	{
		return;
	}

	m_clrPaneTextSourceColor[nPane] = clrText;
	InvalidatePane(nPane);
}

void CCustomStatusBar::SetPaneTextDestinationColor(int nPane, COLORREF clrText)
{
	if (!IsValidPaneNumber(nPane))
	{
		return;
	}

	m_clrPaneTextDestColor[nPane] = clrText;
	InvalidatePane(nPane);
}

void CCustomStatusBar::SetPaneBackgroundColor(int nPane, COLORREF clrBackground)
{
	if (!IsValidPaneNumber(nPane))
	{
		return;
	}

	m_clrPaneBackgroundColor[nPane] = clrBackground;
	InvalidatePane(nPane);
}

void CCustomStatusBar::EnablePaneFlashBounce(int nPane, bool bBounce)
{
	if (!IsValidPaneNumber(nPane))
	{
		return;
	}

	m_bBounce[nPane] = bBounce;
}

void CCustomStatusBar::EnablePaneFlashing(int nPane, bool bEnable)
{
	if (!IsValidPaneNumber(nPane))
	{
		return;
	}

	m_bFlashOn[nPane] = bEnable;

	if (bEnable)
	{
	}
	else
	{
		// Reset the Alpha for this pane
		m_byAlpha[nPane] = MAX_ALPHA;
		InvalidatePane(nPane);
	}
}

void CCustomStatusBar::FadeStep()
{
	// Cycle through the panes in the status bar and fades the
	// text as appropriate.

	BOOL bReturn = TRUE;
	CRect rect;
	int nPane = 0;

	while (GetStatusBarCtrl().GetRect(nPane, &rect))
	{
		if (m_bFlashOn[nPane] == false)
		{
			// Fading Disabled for this pane
			nPane++;
			continue;
		}

		// Don't get the borders of the pane
		rect.DeflateRect(1,1);

		CString strText;
		strText = GetPaneText(nPane);

		// Do the AlphaBlend thing
		CDC* pDC = GetDC();
		AlphaDraw(pDC, strText, nPane, &rect);
		ReleaseDC(pDC);

		// Adjust the Fade state parameters for 
		// the next time through the loop.
		if (!m_bBounce[nPane])
		{
			if (m_bFadeOut)
			{
				m_byAlpha[nPane] -= FADE_STEP;
			}
			else
			{
				m_byAlpha[nPane] += FADE_STEP;
			}
		}
		else
		{
			if (m_bFadeOut[nPane])
			{
				if (FADE_STEP > m_byAlpha[nPane])
				{
					m_bFadeOut[nPane] = false;
				}
				else
				{
					m_byAlpha[nPane] -= FADE_STEP;
				}
			}
			else
			{
				if (m_byAlpha[nPane] > (MAX_ALPHA - FADE_STEP))
				{
					m_bFadeOut[nPane] = true;
				}
				else
				{
					m_byAlpha[nPane] += FADE_STEP;
				}
			}
		}

		nPane++;
	}
}

BEGIN_MESSAGE_MAP(CCustomStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(CCustomStatusBar)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCustomStatusBar message handlers

void CCustomStatusBar::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC* pOrigDC	= CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect		= lpDrawItemStruct->rcItem;
	CString strText	= GetPaneText(lpDrawItemStruct->itemID);

	GetStatusBarCtrl().GetRect(lpDrawItemStruct->itemID, &rect);

	// Don't get the pane borders
	rect.DeflateRect(1,1);

	// Do the AlphaBlend thing
	AlphaDraw(pOrigDC, strText, lpDrawItemStruct->itemID, &rect);
}

void CCustomStatusBar::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == FADE_TIMER_ID)
	{
		FadeStep();
	}
	
	CStatusBar::OnTimer(nIDEvent);
}
