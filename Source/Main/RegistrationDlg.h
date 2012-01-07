///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


// RegistrationDlg.h : Declaration of the CRegistrationDlg

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDlg
class CRegistrationDlg : 
	public CDialogImpl<CRegistrationDlg>
{
public:
	CRegistrationDlg()
	{
		_nRegistrationSettings = 0;
	}

	~CRegistrationDlg()
	{
	}

	int _nRegistrationSettings;
	CHyperLink	m_link;

	enum { IDD = IDD_REGISTRATION };

BEGIN_MSG_MAP(CRegistrationDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDHELP, OnHelp)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();

		// IDC_WEB_PAGE
		m_link.SetHyperLink(_T("www.ImageWalker.com\\register.html"));
		m_link.SetLabel(_T("www.ImageWalker.com\\register.html"));
		m_link.SubclassWindow(GetDlgItem(IDC_REGISTER));

		CheckRadioButton(IDC_REG1, IDC_REG4, IDC_REG1 + _nRegistrationSettings);

		return 1;  // Let the system set the focus
	}

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (IsDlgButtonChecked(IDC_REG4) == BST_CHECKED)
		{
			_nRegistrationSettings = RegistrationSettings::Free;
		}
		else if (IsDlgButtonChecked(IDC_REG3) == BST_CHECKED ||
			IsDlgButtonChecked(IDC_REG2) == BST_CHECKED)
		{
			_nRegistrationSettings = RegistrationSettings::Registered;
		}
		else
		{
			_nRegistrationSettings = RegistrationSettings::Evaluation;
		}
			

		EndDialog(wID);
		return 0;
	}



	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_REGISTER);
		return 0;
	}
};

