#pragma once 

class CTagDlg : 
	public CDialogImpl<CTagDlg>
{
public:

	CString _tags;

	CTagDlg()
	{
	}

	enum { IDD = IDD_TAG};

	BEGIN_MSG_MAP(CTagDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();

		CPropertyArchiveRegistry archive(App.GetRegKey());

		if (archive.StartSection(g_szTags))
		{
			archive.Read(g_szTags, _tags);
		}

		SetDlgItemText(IDC_TAGS, _tags);
		return 1;
	}


	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_TAGS, _tags);

		CPropertyArchiveRegistry archive(App.GetRegKey(), true);

		if (archive.StartSection(g_szTags))
		{
			archive.Write(g_szTags, _tags);
		}

		EndDialog(wID);
		return 0;
	}
};
