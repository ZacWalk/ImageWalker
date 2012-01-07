///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// ViewOptions.h : Declaration of the CViewOptions

#pragma once

struct CFolderCtrlOptions;

class CViewOptions: public CPropertySheetImpl<CViewOptions>
{
public:

	State &_state;
	
	CViewOptions(State &state) : _state(state), CPropertySheetImpl<CViewOptions>(IDS_VIEW_OPTIONS, 0, NULL)
	{
	}

	BEGIN_MSG_MAP(CViewOptions)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam) CenterWindow();
		return 1;
	}

	INT_PTR DoModal(HWND hWndParent = IW::GetMainWindow());
	void OnHelp();

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
};

