#pragma once

#include "Layout.h"
#include "ToolTipWindow.h"

template<class T>
class CView : 
	public FrameWindowImpl<T>,
	public ToolTipWindowImpl<T>
{
public:

	
	CView()
	{
	}

	BEGIN_MSG_MAP(CView<T>)

		MESSAGE_HANDLER(WM_COMMAND, OnCommand)

		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)

		CHAIN_MSG_MAP(ToolTipWindowImpl<T>)
		CHAIN_MSG_MAP(FrameWindowImpl<T>)

	END_MSG_MAP()

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);
		if (wParam != 0)
		{
			pT->OnPaint((HDC)wParam);		
		}
		else
		{
			CPaintDC dc(pT->m_hWnd);
			pT->OnPaint((HDC)dc);
		}

		return 0;
	}

	void OnPaint(CDCHandle dc)
	{
	}	

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);
		WORD id = LOWORD(wParam);
		pT->OnCommand(id);
		bHandled = FALSE;
		return 0;
	}
};
