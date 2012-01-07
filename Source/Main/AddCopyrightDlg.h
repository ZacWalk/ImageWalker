#pragma once 

class CAddCopyrightDlg : 
	public CDialogImpl<CAddCopyrightDlg>
{
public:

	CString _credit;
	CString _source;
	CString _copyright;	

	CAddCopyrightDlg()
	{
	}

	enum { IDD = IDD_ADDCOPYRIGHT};

	BEGIN_MSG_MAP(CAddCopyrightDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();

		CPropertyArchiveRegistry archive(App.GetRegKey());

		if (archive.StartSection(g_szAddCopyright))
		{
			archive.Read(g_szCredit, _credit);
			archive.Read(g_szSource, _source);
			archive.Read(g_szCopyright, _copyright);
		}

		SetDlgItemText(IDC_CREDIT, _credit);
		SetDlgItemText(IDC_SOURCE, _source);
		SetDlgItemText(IDC_COPYRIGHT, _copyright);

		return 1;
	}


	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_CREDIT, _credit);
		GetDlgItemText(IDC_SOURCE, _source);
		GetDlgItemText(IDC_COPYRIGHT, _copyright);

		CPropertyArchiveRegistry archive(App.GetRegKey(), true);

		if (archive.StartSection(g_szAddCopyright))
		{
			archive.Write(g_szCredit, _credit);
			archive.Write(g_szSource, _source);
			archive.Write(g_szCopyright, _copyright);
		}

		EndDialog(wID);
		return 0;
	}

};
