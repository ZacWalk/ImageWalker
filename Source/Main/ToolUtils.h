///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// ITToolUtils.h: Helpers for ImageWalker tools
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Dialogs.h"
#include "LoadAny.h"
#include "State.h"

namespace IW 
{
	/*class CToolPropertyPage : public CPropertyPageImpl<CToolPropertyPage>
	{
	public:

		CToolPropertyPage() : _pDialog(0) {};
		virtual ~CToolPropertyPage() {};

		IW::IDialog *_pDialog;
		IW::CDialogScroll<IW::CFilterPropertySubDlg> _view;
		//CSize _sizePreview;

		virtual bool InsertDialog(HINSTANCE hInstance, UINT nID, IW::IDialog *pDialog)
		{		
			return _view.m_dialog.InsertDialog(hInstance, nID, pDialog);
		}

		virtual void OnInitDialog(HWND hWnd)
		{
			//CRect rc;
			//::GetWindowRect(::GetDlgItem(IDC_PREVIEW), &rc);
			//_sizePreview.cx = rc.right - rc.left;
			//_sizePreview.cy = rc.bottom - rc.top;

			_view.Create(hWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

			DoSize(hWnd);
		};


		void DoSize(HWND hWnd)
		{
			CRect r;
			::GetWindowRect(::GetDlgItem(hWnd, IDC_SETTINGS), &r);

			::ScreenToClient(hWnd, (LPPOINT)&r);
			::ScreenToClient(hWnd, ((LPPOINT)&r)+1);

			_view.MoveWindow(&r);
		}

		virtual void OnCommand(HWND hWnd, UINT nID) 
		{
		};

		BEGIN_MSG_MAP(CToolPropertyPage)
		END_MSG_MAP()

		virtual BOOL ProcessDialogMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
		{
			m_hWnd = hWnd;
			return ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);
		}

		void OnHelp(HWND hWnd) const 
		{ 
		}

	};*/

	/////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////


	
}; 
// namespace IW

