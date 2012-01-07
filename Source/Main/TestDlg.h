///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
#pragma once

#include "Dialogs.h"
#include "ProgressDlg.h"
#include "LoadAny.h"

class CTestDlg :  public CDialogImpl<CTestDlg>
{
public:
	CTestDlg(State &state);

	enum { IDD = IDD_TEST };

	CLoadAny _loader;
	bool m_bCancel;
	CString m_strMessage;
	State &_state;
	CString m_strHtml;

	IW::CFilePath m_pathOutputFolder;
	IW::CFilePath m_pathHome;

	CProgressDlg m_pd;

	// Properties
	LPCTSTR GetOutputFolder() const { return m_pathOutputFolder; };

	void TestGalleries();

	//////////////////////////////////////////////////////////
	BEGIN_MSG_MAP(CTestDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnTest)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnSelectFolder)
		COMMAND_ID_HANDLER(ID_SELECT_BROWSE, OnBrowseFolder)
		COMMAND_ID_HANDLER(ID_SELECT_CURRENTFOLDER, OnCurrentFolder)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);		
	LRESULT OnTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelectFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCurrentFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};
