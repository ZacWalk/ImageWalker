///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
#pragma once

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUT };
	CHyperLink	_link;

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		
		CString str;
		str.Format(_T("ImageWalker v%d.%d.%d\n© Zac Walker 2006"), MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
		SetDlgItemText(IDC_TITLE, str);

		_link.SetHyperLink(_T("www.ImageWalker.com"));
		_link.SetLabel(_T("www.ImageWalker.com"));
		_link.SubclassWindow(GetDlgItem(IDC_WEB_PAGE));	

		return (LRESULT)TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};
