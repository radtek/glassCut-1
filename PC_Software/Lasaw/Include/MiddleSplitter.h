#if !defined(AFX_MIDDLESPLITTER_H__C19B7D40_91ED_49A1_8E64_8EF6F40CD864__INCLUDED_)
#define AFX_MIDDLESPLITTER_H__C19B7D40_91ED_49A1_8E64_8EF6F40CD864__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MiddleSplitter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMiddleSplitter view

class CMiddleSplitter : public CSplitterWnd
{
protected:
	       // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMiddleSplitter)

// Attributes
public:
	CMiddleSplitter();    
	virtual ~CMiddleSplitter();
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiddleSplitter)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMiddleSplitter)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDDLESPLITTER_H__C19B7D40_91ED_49A1_8E64_8EF6F40CD864__INCLUDED_)
