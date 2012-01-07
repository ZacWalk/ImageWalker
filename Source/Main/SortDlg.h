///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////

class CSortDlg : 
	public CDialogImpl<CSortDlg>
{
public:
	CSortDlg()
	{
		_nSortOrder = 0;
		_bAssending = true;
	}

	enum { IDD = IDD_SORT };

	long _nSortOrder;
	bool _bAssending;

	BEGIN_MSG_MAP(CSortDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());

		AddMetaDataType(IW::ePropertyName, App.LoadString(IDS_FILENAME));
		AddMetaDataType(IW::ePropertyType, App.LoadString(IDS_FILETYPE));
		AddMetaDataType(IW::ePropertySize, App.LoadString(IDS_FILESIZE));
		AddMetaDataType(IW::ePropertyModifiedDate, App.LoadString(IDS_FILEMODIFIED));
		AddMetaDataType(IW::ePropertyCreatedDate, App.LoadString(IDS_FILECREATED));
		AddMetaDataType(IW::ePropertyPath, App.LoadString(IDS_FILEPATH));
		AddMetaDataType(IW::ePropertyModifiedTime, App.LoadString(IDS_FILEMODIFIED_TIME));
		AddMetaDataType(IW::ePropertyCreatedTime, App.LoadString(IDS_FILECREATED_TIME));
		AddMetaDataType(IW::ePropertyWidth, App.LoadString(IDS_WIDTH));
		AddMetaDataType(IW::ePropertyHeight, App.LoadString(IDS_HEIGHT));
		AddMetaDataType(IW::ePropertyDepth, App.LoadString(IDS_DEPTH));
		AddMetaDataType(IW::ePropertyDateTaken, _T("Date Taken"));

		HWND hwndCombo = GetDlgItem(IDC_SORT);

		int nCount = SendMessage(hwndCombo , CB_GETCOUNT, 0, 0); 
		int i = 0;

		for(; i < nCount; i++)
		{
			int nId = SendMessage(hwndCombo, CB_GETITEMDATA, i, 0L);

			if (_nSortOrder == nId)
			{
				break;
			}
		}

		SendMessage(hwndCombo , CB_SETCURSEL, i, 0);
		CheckDlgButton(IDC_ASSENDING, _bAssending ? BST_CHECKED : BST_UNCHECKED);


		return (LRESULT)TRUE;
	}

	bool AddMetaDataType(DWORD dwId, LPCTSTR szTitle)
	{
		if (HIWORD(dwId) == 0)
		{
			HWND hCombo = GetDlgItem(IDC_SORT);		
			DWORD dwIndex = ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) szTitle); 		
			::SendMessage(hCombo, CB_SETITEMDATA, dwIndex, dwId);
		}

		return true;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		HWND hCombo = GetDlgItem(IDC_SORT);

		int n = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
		_nSortOrder = ::SendMessage(hCombo, CB_GETITEMDATA, n, 0);
		_bAssending = BST_CHECKED == IsDlgButtonChecked(IDC_ASSENDING);

		EndDialog(wID);
		return 0;
	}
};
