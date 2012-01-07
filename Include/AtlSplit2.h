// Windows Template Library - WTL version 7.0
// Copyright (C) 1997-2002 Microsoft Corporation
// All rights reserved.
//
// This file is a part of the Windows Template Library.
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.

#ifndef __ATLSPLIT2_H__
#define __ATLSPLIT2_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlsplit.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlsplit.h requires atlwin.h to be included first
#endif


/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CSplitter2Impl<T>
// CSplitter2WindowImpl<T, TBase, TWinTraits>
// CSplitter2WindowT


namespace WTL
{

/////////////////////////////////////////////////////////////////////////////
// CSplitter2Impl - Provides splitter support to any window

// Splitter panes constants
#define SPLIT_PANE_LEFT			 0
#define SPLIT_PANE_RIGHT		 1
#define SPLIT_PANE_TOP			 SPLIT_PANE_LEFT
#define SPLIT_PANE_BOTTOM		 SPLIT_PANE_RIGHT
#define SPLIT_PANE_NONE			-1

// Splitter extended styles
#define SPLIT_PROPORTIONAL		0x00000001
#define SPLIT_NONINTERACTIVE		0x00000002
#define SPLIT_RIGHTALIGNED		0x00000004
#define SPLIT_BOTTOMALIGNED		SPLIT_RIGHTALIGNED

// Note: SPLIT_PROPORTIONAL and SPLIT_RIGHTALIGNED/SPLIT_BOTTOMALIGNED are 
// mutually exclusive. If both are set, splitter defaults to SPLIT_PROPORTIONAL


template <class T>
class CSplitter2Impl
{
public:
	enum { m_nPanesCount = 2, m_nPropMax = 10000 };

	HWND m_hWndPane[m_nPanesCount];
	RECT m_rCSplitter2;
	int m_xySplitterPos;
	int m_nDefActivePane;
	int m_cxySplitBar;		// splitter bar width/height
	HCURSOR m_hCursor;
	int m_cxyMin;			// minimum pane size
	int m_cxyBarEdge;		// splitter bar edge
	bool m_bFullDrag;
	int m_cxyDragOffset;
	mutable int m_nProportionalPos;
	bool m_bUpdateProportionalPos;
	DWORD m_dwExtendedStyle;	// splitter specific extended styles
	int m_nSinglePane;		// single pane mode
	bool m_bVertical;
	

// Constructor
	CSplitter2Impl() :
			m_xySplitterPos(-1), m_nDefActivePane(SPLIT_PANE_NONE), 
			m_cxySplitBar(0), m_cxyMin(0), m_cxyBarEdge(0), m_bFullDrag(true), 
			m_cxyDragOffset(0), m_nProportionalPos(0), m_bUpdateProportionalPos(true),
			m_dwExtendedStyle(SPLIT_PROPORTIONAL),
			m_nSinglePane(SPLIT_PANE_NONE),
			m_bVertical(true),
			m_hCursor(0)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = NULL;
		m_hWndPane[SPLIT_PANE_RIGHT] = NULL;

		::SetRectEmpty(&m_rCSplitter2);
		m_hCursor = ::LoadCursor(NULL, m_bVertical ? IDC_SIZEWE : IDC_SIZENS);
		m_nProportionalPos = m_nPropMax / 2;
	}

	void LoadDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		int nSplitterPos = 0;
		if (pProperties->Read(g_szSplitterPos, nSplitterPos))
		{
			SetProportionalPos(nSplitterPos);		
		}
	}

	void SaveDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		pProperties->Write(g_szSplitterPos, GetProportionalPos());
	}

// Attributes
	void SetSplitterRect(LPRECT lpRect = NULL, bool bUpdate = true)
	{
		T* pT = static_cast<T*>(this);

		if(lpRect == NULL)
		{
			pT->GetClientRect(&m_rCSplitter2);
		}
		else
		{
			m_rCSplitter2 = *lpRect;
		}

		if(IsProportional())
			UpdateProportionalPos();
		else if(IsRightAligned())
			UpdateRightAlignPos();

		if(bUpdate)
			pT->UpdateSplitterLayout();
	}

	void GetSplitterRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		*lpRect = m_rCSplitter2;
	}

	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		T* pT = static_cast<T*>(this);

		if(xyPos == -1)		// -1 == middle
		{
			if(m_bVertical)
				xyPos = (m_rCSplitter2.right - m_rCSplitter2.left - m_cxySplitBar - m_cxyBarEdge) / 2;
			else
				xyPos = (m_rCSplitter2.bottom - m_rCSplitter2.top - m_cxySplitBar - m_cxyBarEdge) / 2;
		}

		// Adjust if out of valid range
		int cxyMax = 0;
		if(m_bVertical)
			cxyMax = m_rCSplitter2.right - m_rCSplitter2.left;
		else
			cxyMax = m_rCSplitter2.bottom - m_rCSplitter2.top;

		if(xyPos < m_cxyMin + m_cxyBarEdge)
			xyPos = m_cxyMin;
		else if(xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
			xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;

		// Set new position and update if requested
		bool bRet = (m_xySplitterPos != xyPos);
		m_xySplitterPos = xyPos;

		if(m_bUpdateProportionalPos)
		{
			if(IsProportional())
				StoreProportionalPos();
			else if(IsRightAligned())
				StoreRightAlignPos();
		}
		else
		{
			m_bUpdateProportionalPos = true;
		}

		if(bUpdate && bRet)
			pT->UpdateSplitterLayout();

		return bRet;
	}

	int GetSplitterPos() const
	{
		return m_xySplitterPos;
	}

	bool SetSinglePaneMode(int nPane = SPLIT_PANE_NONE)
	{
		T* pT = static_cast<T*>(this);

		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT || nPane == SPLIT_PANE_NONE);
		if(!(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT || nPane == SPLIT_PANE_NONE))
			return false;

		if(nPane != SPLIT_PANE_NONE)
		{
			pT->ShowPane(nPane, SW_SHOW);
			pT->OnShowPane(nPane);
			int nOtherPane = (nPane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			pT->ShowPane(nOtherPane, SW_HIDE);
			
			if(m_nDefActivePane != nPane)
				m_nDefActivePane = nPane;
		}
		else if(m_nSinglePane != SPLIT_PANE_NONE)
		{
			int nOtherPane = (m_nSinglePane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			pT->ShowPane(nOtherPane, SW_SHOW);
			pT->OnShowPane(nOtherPane);
		}

		m_nSinglePane = nPane;
		pT->UpdateSplitterLayout();
		return true;
	}

	void OnShowPane(int)
	{
	}

	void ShowPane(int nPane, int nCmdShow)
	{
		if (m_hWndPane[nPane]) ::ShowWindow(m_hWndPane[nPane], nCmdShow);
	}


	int GetSinglePaneMode() const
	{
		return m_nSinglePane;
	}

	void SetAspectAspect(bool bVertical)
	{
		T* pT = static_cast<T*>(this);

		StoreProportionalPos();
		m_bVertical = bVertical;
		UpdateProportionalPos();
		

		pT->UpdateSplitterLayout();
		m_hCursor = ::LoadCursor(NULL, m_bVertical ? IDC_SIZEWE : IDC_SIZENS);
	}

	DWORD GetSplitterExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetSplitterExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);

		return dwPrevStyle;
	}

// Splitter operations
	void SetSplitterPanes(HWND hWndLeftTop, HWND hWndRightBottom, bool bUpdate = true)
	{
		T* pT = static_cast<T*>(this);

		m_hWndPane[SPLIT_PANE_LEFT] = hWndLeftTop;
		m_hWndPane[SPLIT_PANE_RIGHT] = hWndRightBottom;
		ATLASSERT(m_hWndPane[SPLIT_PANE_LEFT] == NULL || m_hWndPane[SPLIT_PANE_RIGHT] == NULL || m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]);
		if(bUpdate)
			pT->UpdateSplitterLayout();
	}

	bool SetSplitterPane(int nPane, HWND hWnd, bool bUpdate = true)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);
		T* pT = static_cast<T*>(this);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		m_hWndPane[nPane] = hWnd;
		ATLASSERT(m_hWndPane[SPLIT_PANE_LEFT] == NULL || m_hWndPane[SPLIT_PANE_RIGHT] == NULL || m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]);
		if(bUpdate)
			pT->UpdateSplitterLayout();
		return true;
	}

	HWND GetSplitterPane(int nPane) const
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		return m_hWndPane[nPane];
	}

	bool SetActivePane(int nPane)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		if(m_nSinglePane != SPLIT_PANE_NONE && nPane != m_nSinglePane)
			return false;
		::SetFocus(m_hWndPane[nPane]);
		m_nDefActivePane = nPane;
		return true;
	}

	int GetActivePane() const
	{
		int nRet = SPLIT_PANE_NONE;
		HWND hWndFocus = ::GetFocus();
		if(hWndFocus != NULL)
		{
			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if(hWndFocus == m_hWndPane[nPane] || ::IsChild(m_hWndPane[nPane], hWndFocus))
				{
					nRet = nPane;
					break;
				}
			}
		}
		return nRet;
	}

	bool ActivateNextPane(bool bNext = true)
	{
		int nPane = m_nSinglePane;
		if(nPane == SPLIT_PANE_NONE)
		{
			switch(GetActivePane())
			{
			case SPLIT_PANE_LEFT:
				nPane = SPLIT_PANE_RIGHT;
				break;
			case SPLIT_PANE_RIGHT:
				nPane = SPLIT_PANE_LEFT;
				break;
			default:
				nPane = bNext ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
				break;
			}
		}
		return SetActivePane(nPane);
	}

	bool SetDefaultActivePane(int nPane)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		m_nDefActivePane = nPane;
		return true;
	}

	bool SetDefaultActivePane(HWND hWnd)
	{
		for(int nPane = 0; nPane < m_nPanesCount; nPane++)
		{
			if(hWnd == m_hWndPane[nPane])
			{
				m_nDefActivePane = nPane;
				return true;
			}
		}
		return false;	// not found
	}

	int GetDefaultActivePane() const
	{
		return m_nDefActivePane;
	}

	void DrawSplitter(CDCHandle dc)
	{
		ATLASSERT(dc.m_hDC != NULL);
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			return;

		T* pT = static_cast<T*>(this);
		if(m_nSinglePane == SPLIT_PANE_NONE)
		{
			pT->DrawSplitterBar(dc);

			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				pT->DrawSplitterPane(dc, nPane);
			}
		}
		else
		{
			pT->DrawSplitterPane(dc, m_nSinglePane);
		}
	}

// Overrideables
	void DrawSplitterBar(CDCHandle dc)
	{
		T* pT = static_cast<T*>(this);

		RECT rect;
		if(GetSplitterBarRect(&rect))
		{
			if (::GetCapture() == pT->m_hWnd)
			{
				dc.FillSolidRect(&rect, IW::Style::Color::Highlight);
			}
			else
			{
				const int nDarken = 8;
				COLORREF c = IW::Style::Color::Window;
				dc.FillSolidRect(&rect, IW::Emphasize(c, nDarken));
			}
		}
	}

	// called only if pane is empty
	void DrawSplitterPane(CDCHandle dc, int nPane)
	{
		if(m_hWndPane[nPane] == NULL)
		{
			RECT rect;
			if(GetSplitterPaneRect(nPane, &rect))
			{
				dc.FillRect(&rect, COLOR_APPWORKSPACE);
			}
		}
	}

// Message map and handlers
	typedef CSplitter2Impl< T >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
		if(IsInteractive())
		{
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		}
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		GetSystemSettings(false);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		// try setting position if not set
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			pT->SetSplitterPos();


		if (wParam != 0)
		{
			pT->DrawSplitter((HDC)wParam);
		}
		else
		{
			CPaintDC dc(pT->m_hWnd);
			pT->DrawSplitter(dc.m_hDC);
		}

		return 0;
	}

	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if((HWND)wParam == pT->m_hWnd && LOWORD(lParam) == HTCLIENT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			pT->ScreenToClient(&ptPos);
			if(IsOverSplitterBar(ptPos.x, ptPos.y))
				return 1;
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if((wParam & MK_LBUTTON) && ::GetCapture() == pT->m_hWnd)
		{
			int xyNewSplitPos = 0;
			if(m_bVertical)
				xyNewSplitPos = xPos - m_rCSplitter2.left - m_cxyDragOffset;
			else
				xyNewSplitPos = yPos - m_rCSplitter2.top - m_cxyDragOffset;

			if(xyNewSplitPos == -1)	// avoid -1, that means middle
				xyNewSplitPos = -2;

			if(m_xySplitterPos != xyNewSplitPos)
			{
				if(m_bFullDrag)
				{
					if(pT->SetSplitterPos(xyNewSplitPos, true))
						pT->UpdateWindow();
				}
				else
				{
					DrawGhostBar();
					pT->SetSplitterPos(xyNewSplitPos, false);
					DrawGhostBar();
				}
			}
		}
		else		// not dragging, just set cursor
		{
			if(IsOverSplitterBar(xPos, yPos))
			{
				::SetCursor(m_hCursor);

				if (::GetCapture() != pT->m_hWnd)
				{
					pT->SetCapture();
					pT->UpdateSplitterLayout();
				}
			}
			else if (::GetCapture() == pT->m_hWnd)
			{
				::ReleaseCapture();
				pT->UpdateSplitterLayout();
			}

			bHandled = FALSE;
		}

		return 0;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if(IsOverSplitterBar(xPos, yPos))
		{
			T* pT = static_cast<T*>(this);
			pT->SetCapture();
			::SetCursor(m_hCursor);
			if(!m_bFullDrag)
				DrawGhostBar();
			if(m_bVertical)
				m_cxyDragOffset = xPos - m_rCSplitter2.left - m_xySplitterPos;
			else
				m_cxyDragOffset = yPos - m_rCSplitter2.top - m_xySplitterPos;
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);

		if(!m_bFullDrag)
		{
			DrawGhostBar();
			pT->UpdateSplitterLayout();
			pT->UpdateWindow();
		}
		::ReleaseCapture();
		pT->UpdateSplitterLayout();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->SetSplitterPos();	// middle
		return 0;
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM, BOOL& bHandled)
	{
		if(m_nSinglePane == SPLIT_PANE_NONE)
		{
			if(m_nDefActivePane == SPLIT_PANE_LEFT || m_nDefActivePane == SPLIT_PANE_RIGHT)
				::SetFocus(m_hWndPane[m_nDefActivePane]);
		}
		else
		{
			::SetFocus(m_hWndPane[m_nSinglePane]);
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		LRESULT lRet = pT->DefWindowProc(uMsg, wParam, lParam);
		if(lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT pt = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			pT->ScreenToClient(&pt);
			RECT rcPane;
			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if(GetSplitterPaneRect(nPane, &rcPane) && ::PtInRect(&rcPane, pt))
				{
					m_nDefActivePane = nPane;
					break;
				}
			}
		}
		return lRet;
	}

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		GetSystemSettings(true);
		return 0;
	}

// Implementation - internal helpers
	void UpdateSplitterLayout()
	{
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			return;

		T* pT = static_cast<T*>(this);
		RECT rect = { 0, 0, 0, 0 };
		if(m_nSinglePane == SPLIT_PANE_NONE)
		{
			if(GetSplitterBarRect(&rect))
				pT->InvalidateRect(&rect);

			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{				
				if(GetSplitterPaneRect(nPane, &rect))
				{
					pT->ResizePane(nPane, rect);					
				}
			}
		}
		else
		{
			if(GetSplitterPaneRect(m_nSinglePane, &rect))
			{
				pT->ResizePane(m_nSinglePane, rect);
			}
		}
	}

	void ResizePane(int nPane, RECT &rect)
	{
		T* pT = static_cast<T*>(this);

		if(m_hWndPane[nPane] != NULL)
			::SetWindowPos(m_hWndPane[nPane], NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		else
			pT->InvalidateRect(&rect);
	}

	bool GetSplitterBarRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		if(m_nSinglePane != SPLIT_PANE_NONE || m_xySplitterPos == -1)
			return false;

		if(m_bVertical)
		{
			lpRect->left = m_rCSplitter2.left + m_xySplitterPos;
			lpRect->top = m_rCSplitter2.top;
			lpRect->right = m_rCSplitter2.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
			lpRect->bottom = m_rCSplitter2.bottom;
		}
		else
		{
			lpRect->left = m_rCSplitter2.left;
			lpRect->top = m_rCSplitter2.top + m_xySplitterPos;
			lpRect->right = m_rCSplitter2.right;
			lpRect->bottom = m_rCSplitter2.top + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
		}

		return true;
	}

	bool GetSplitterPaneRect(int nPane, LPRECT lpRect) const
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);
		ATLASSERT(lpRect != NULL);
		bool bRet = true;
		if(m_nSinglePane != SPLIT_PANE_NONE)
		{
			if(nPane == m_nSinglePane)
				*lpRect = m_rCSplitter2;
			else
				bRet = false;
		}
		else if(nPane == SPLIT_PANE_LEFT)
		{
			if(m_bVertical)
			{
				lpRect->left = m_rCSplitter2.left;
				lpRect->top = m_rCSplitter2.top;
				lpRect->right = m_rCSplitter2.left + m_xySplitterPos;
				lpRect->bottom = m_rCSplitter2.bottom;
			}
			else
			{
				lpRect->left = m_rCSplitter2.left;
				lpRect->top = m_rCSplitter2.top;
				lpRect->right = m_rCSplitter2.right;
				lpRect->bottom = m_rCSplitter2.top + m_xySplitterPos;
			}
		}	
		else if(nPane == SPLIT_PANE_RIGHT)
		{
			if(m_bVertical)
			{
				lpRect->left = m_rCSplitter2.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
				lpRect->top = m_rCSplitter2.top;
				lpRect->right = m_rCSplitter2.right;
				lpRect->bottom = m_rCSplitter2.bottom;
			}
			else
			{
				lpRect->left = m_rCSplitter2.left;
				lpRect->top = m_rCSplitter2.top + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
				lpRect->right = m_rCSplitter2.right;
				lpRect->bottom = m_rCSplitter2.bottom;
			}
		}
		else
		{
			bRet = false;
		}
		return bRet;
	}

	bool IsOverSplitterRect(int x, int y) const
	{
		// -1 == don't check
		return ((x == -1 || (x >= m_rCSplitter2.left && x <= m_rCSplitter2.right)) &&
			(y == -1 || (y >= m_rCSplitter2.top && y <= m_rCSplitter2.bottom)));
	}

	bool IsOverSplitterBar(int x, int y) const
	{
		if(m_nSinglePane != SPLIT_PANE_NONE)
			return false;
		if(m_xySplitterPos == -1 || !IsOverSplitterRect(x, y))
			return false;
		int xy = m_bVertical ? x : y;
		int xyOff = m_bVertical ? m_rCSplitter2.left : m_rCSplitter2.top;
		return ((xy >= (xyOff + m_xySplitterPos)) && (xy < xyOff + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge));
	}

	void DrawGhostBar()
	{
		RECT rect = { 0, 0, 0, 0 };
		if(GetSplitterBarRect(&rect))
		{
			// invert the brush pattern (looks just like frame window sizing)
			T* pT = static_cast<T*>(this);
			CWindowDC dc(pT->m_hWnd);
			CBrush brush = CDCHandle::GetHalftoneBrush();
			if(brush.m_hBrush != NULL)
			{
				CBrushHandle brushOld = dc.SelectBrush(brush);
				dc.PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
				dc.SelectBrush(brushOld);
			}
		}
	}

	void GetSystemSettings(bool bUpdate)
	{
		T* pT = static_cast<T*>(this);
		m_cxySplitBar = ::GetSystemMetrics(m_bVertical ? SM_CXSIZEFRAME : SM_CYSIZEFRAME);

		::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bFullDrag, 0);

		if(bUpdate)
			pT->UpdateSplitterLayout();
	}

	bool IsProportional() const
	{
		return ((m_dwExtendedStyle & SPLIT_PROPORTIONAL) != 0);
	}

	void StoreProportionalPos() const
	{
		int cxyTotal = m_bVertical ? (m_rCSplitter2.right - m_rCSplitter2.left - m_cxySplitBar - m_cxyBarEdge) : (m_rCSplitter2.bottom - m_rCSplitter2.top - m_cxySplitBar - m_cxyBarEdge);
		if(cxyTotal > 0)
			m_nProportionalPos = ::MulDiv(m_xySplitterPos, m_nPropMax, cxyTotal);
		else
			m_nProportionalPos = m_nPropMax / 2;
	}

	void SetProportionalPos(int nPos)
	{
		m_nProportionalPos = nPos;
		UpdateProportionalPos();

		T* pT = static_cast<T*>(this);
		if (pT->m_hWnd) pT->UpdateSplitterLayout();
	}

	int GetProportionalPos() const
	{
		StoreProportionalPos();

		if (m_nProportionalPos != 0) 
			return m_nProportionalPos;

		return m_nPropMax / 2;
	}

	void UpdateProportionalPos()
	{
		int cxyTotal = m_bVertical ? (m_rCSplitter2.right - m_rCSplitter2.left - m_cxySplitBar - m_cxyBarEdge) : (m_rCSplitter2.bottom - m_rCSplitter2.top - m_cxySplitBar - m_cxyBarEdge);
		if(cxyTotal > 0)
		{
			int xyNewPos = ::MulDiv(m_nProportionalPos, cxyTotal, m_nPropMax);
			m_bUpdateProportionalPos = false;
			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos(xyNewPos, false);
		}
	}

	bool IsRightAligned() const
	{
		return ((m_dwExtendedStyle & SPLIT_RIGHTALIGNED) != 0);
	}

	void StoreRightAlignPos()
	{
		int cxyTotal = m_bVertical ? (m_rCSplitter2.right - m_rCSplitter2.left - m_cxySplitBar - m_cxyBarEdge) : (m_rCSplitter2.bottom - m_rCSplitter2.top - m_cxySplitBar - m_cxyBarEdge);
		if(cxyTotal > 0)
			m_nProportionalPos = cxyTotal - m_xySplitterPos;
		else
			m_nProportionalPos = m_nPropMax / 2;
	}

	void UpdateRightAlignPos()
	{
		int cxyTotal = m_bVertical ? (m_rCSplitter2.right - m_rCSplitter2.left - m_cxySplitBar - m_cxyBarEdge) : (m_rCSplitter2.bottom - m_rCSplitter2.top - m_cxySplitBar - m_cxyBarEdge);
		if(cxyTotal > 0)
		{
			m_bUpdateProportionalPos = false;
			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos(cxyTotal - m_nProportionalPos, false);
		}
	}

	bool IsInteractive() const
	{
		return ((m_dwExtendedStyle & SPLIT_NONINTERACTIVE) == 0);
	}
};



/////////////////////////////////////////////////////////////////////////////
// CSplitter2WindowImpl - Implements a splitter window

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CSplitter2WindowImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >, public CSplitter2Impl< T >
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, COLOR_WINDOW)

	typedef CSplitter2WindowImpl< T , TBase, TWinTraits >	thisClass;
	typedef CSplitter2Impl< T >				baseClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseClass)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();

		bHandled = FALSE;
		return 1;
	}
};


/////////////////////////////////////////////////////////////////////////////
// CSplitter2Window - Implements a splitter window to be used as is

class CSplitter2Window : public CSplitter2WindowImpl<CSplitter2Window>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_SplitterWindow"), CS_DBLCLKS, COLOR_WINDOW)
};


}; //namespace WTL

#endif // __ATLSPLIT2_H__
