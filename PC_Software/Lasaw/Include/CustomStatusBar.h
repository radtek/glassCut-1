#if !defined(AFX_CUSTOMSTATUSBAR_H__E17ADCCD_3958_47E2_A76E_838F9ADDC10C__INCLUDED_)
#define AFX_CUSTOMSTATUSBAR_H__E17ADCCD_3958_47E2_A76E_838F9ADDC10C__INCLUDED_

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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomStatusBar.h : header file
//

#define MAX_NUM_OF_PANES	16

/////////////////////////////////////////////////////////////////////////////
// CCustomStatusBar window

class CCustomStatusBar : public CStatusBar
{
// Construction
public:
	CCustomStatusBar();
	virtual ~CCustomStatusBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomStatusBar)
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	//}}AFX_VIRTUAL

// Implementation
public:

	// Name:		CalculateWidthForText()
	// Arguments:	CString
	// Returns:		int
	// Notes:		Returns the size of the string passed in.
	int CalculateWidthForText(CString strText);

	// Name:		InvalidatePane()
	// Arguments:	int
	// Returns:		bool
	// Notes:		Forces a redraw of the pane number passed in.
	bool InvalidatePane(int nPane);

	// Name:		SetPaneTextSourceColor()
	// Arguments:	int, COLORREF
	// Returns:		void
	// Notes:		Sets the source text color for a pane.  This is the start color
	//				of the text before it is faded.
	void SetPaneTextSourceColor(int nPane, COLORREF clrText);

	// Name:		SetPaneTextDestinationColor()
	// Arguments:	int, COLORREF
	// Returns:		void
	// Notes:		Sets the destination text color for a pane.  This is the final color
	//				of the text after it is faded.
	void SetPaneTextDestinationColor(int nPane, COLORREF clrText);

	// Name:		SetPaneBackgroundColor()
	// Arguments:	int, COLORREF
	// Returns:		void
	// Notes:		Sets the background color of the pane.
	void SetPaneBackgroundColor(int nPane, COLORREF clrBackground);

	// Name:		EnablePaneFlashBounce()
	// Arguments:	int, bool
	// Returns:		void
	// Notes:		If fading is enabled, the bool will determine if the text should
	//				fade from the start color to the destination color and then repeat
	//				(bBounce = false) or if the text should fade from the start color to
	//				the destination color and then back to the start color (bBounce = true).
	void EnablePaneFlashBounce(int nPane, bool bBounce);

	// Name:		EnablePaneFlashing()
	// Arguments:	int, bool
	// Returns:		void
	// Notes:		Turns the flash/fade for the pane on or off.
	void EnablePaneFlashing(int nPane, bool bEnable);

	// Name:		ManageTimer()
	// Arguments:	bool
	// Returns:		void
	// Notes:		Turns internal management of the timer on or off.  If bEnable is true,
	//				the class will start a timer to manage the fading.  If bEnable is false,
	//				it is assumed that the user of the class will use some other mechanism to
	//				manage the fading.
	void ManageTimer(bool bEnable);

	// Name:		FadeStep()
	// Arguments:	none
	// Returns:		void
	// Notes:		This is the function that is called everytime you need to step the fading.
	//				If you called ManageTimer(true), this is called internally for you.  If 
	//				you called ManageTimer(false), then you should call this function whenever
	//				you want to step the fading.
	void FadeStep();

	// Generated message map functions
protected:
	HINSTANCE m_hInst;									// Handle to DLL.  Use to test for AlphaBlend support
	FARPROC m_pProcAlphaBlend;							// Handle to AlphaBlend proc.  Use to test for AlphaBlend support

	bool	m_bManageTimer;								// Should the control spawn a timer to manage fading?  true=yes, false=no
	BYTE	m_byAlpha[MAX_NUM_OF_PANES];				// Alpha value to use in the blend.
	bool	m_bBounce[MAX_NUM_OF_PANES];				// Determines if the display will bounce when it hits a boundry.
	bool	m_bFadeOut[MAX_NUM_OF_PANES];				// Flag for fading in or fading out.  true=fade out, false=fade in.
	COLORREF m_clrPaneBackgroundColor[MAX_NUM_OF_PANES];// Color for the Pane background
	COLORREF m_clrPaneTextSourceColor[MAX_NUM_OF_PANES];// Source color for the Pane text
	COLORREF m_clrPaneTextDestColor[MAX_NUM_OF_PANES];	// Destination color for the Pane text
	bool	m_bFlashOn[MAX_NUM_OF_PANES];				// Flag to turn the fade on or off for a pane

	bool IsValidPaneNumber(int nPane);
	void AlphaDraw(CDC* pDC, CString strText, int nPaneNumber, CRect* pRect = NULL);

	//{{AFX_MSG(CCustomStatusBar)
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMSTATUSBAR_H__E17ADCCD_3958_47E2_A76E_838F9ADDC10C__INCLUDED_)
