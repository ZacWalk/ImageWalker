class CRenameDlg : public CDialogImpl<CRenameDlg>
{
protected:
public:
	CRenameDlg(const CString &strName, const CString &strExtension) : _strName(strName), _strExtension(strExtension)
	{		
	}

	enum { IDD = IDD_RENAME_SINGLE };

	CString _strName;
	CString _strExtension;

	BEGIN_MSG_MAP(CRenameDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()	

	CRenameDlg(const CString &strFileName)
	{
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT] = _T("");

		_tsplitpath_s( strFileName, NULL, 0, NULL, 0, szFileName, _MAX_FNAME, szExt, _MAX_EXT);

		_strName = szFileName;
		_strExtension = szExt;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		SetDlgItemText(IDC_NAME, _strName);
		SetDlgItemText(IDC_EXTENSION, _strExtension);
		return (LRESULT)TRUE;
	}

	CString GetFileName() const
	{
		TCHAR sz[MAX_PATH];
		_tmakepath_s( sz, MAX_PATH, 0, 0, _strName, _strExtension);
		return sz;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_NAME, _strName);
		GetDlgItemText(IDC_EXTENSION, _strExtension);

		EndDialog(wID);
		return 0;
	}
};

class CRenameSelectedDlg : public CDialogImpl<CRenameSelectedDlg>
{
protected:
public:
	CRenameSelectedDlg(const CString &strTemplate, int nStart) : _strTemplate(strTemplate), _nStart(nStart)
	{	
	}

	enum { IDD = IDD_RENAME };

	CString _strTemplate;
	int _nStart;

	BEGIN_MSG_MAP(CRenameSelectedDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)
	END_MSG_MAP()	

	CRenameSelectedDlg(const CString &strFileName)
	{

	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		SetDlgItemText(IDC_TEMPLATE, _strTemplate);
		SetDlgItemInt(IDC_START_AT, _nStart, FALSE);
		return (LRESULT)TRUE;
	}

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(m_hWnd, HELP_TOOL_RENAME);   
		return 0;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_TEMPLATE, _strTemplate);
		_nStart = GetDlgItemInt(IDC_START_AT, NULL, FALSE);
		EndDialog(wID);
		return 0;
	}
};

class CRenameSelected
{
public:

	CString _strTemplate;
	DWORD _nStart;
	int _nPosition;

	CRenameSelected()
	{
		_strTemplate = _T("File####");
		_nStart = 1;

		CPropertyArchiveRegistry archive(App.GetRegKey());
		if (archive.StartSection(g_szRename))
		{
			archive.Read(g_szPosition, _nPosition);
			archive.Read(g_szTemplate, _strTemplate);
			archive.EndSection();
		}
	}

	~CRenameSelected()
	{
		CPropertyArchiveRegistry archive(App.GetRegKey(), true);
		if (archive.StartSection(g_szRename))
		{
			archive.Write(g_szPosition, _nPosition);
			archive.Write(g_szTemplate, _strTemplate);
			archive.EndSection();
		}		
	}

	void Rename(IW::FolderPtr pFolder)
	{
		CRenameSelectedDlg dlg(_strTemplate, _nStart);

		if (IDOK == dlg.DoModal())
		{
			_strTemplate = dlg._strTemplate;
			_nPosition = _nStart = dlg._nStart;

			CProgressDlg pd(IDD_PROGRESS_ADVANCED);
			pd.Create(IW::GetMainWindow(), _T("Renaming Selected Files"));

			if (!pFolder->IterateSelectedItems(this, &pd))
			{
				IW::CMessageBoxIndirect mb;
				mb.Show(_T("Failed to rename all files."));
			}
		}
	}

	bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus)
	{
		return true;
	}

	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
	{
		static TCHAR *szNumbers = _T("0123456789");

		if (!pItem->CanRename())
		{
			pStatus->SetError(App.LoadString(IDS_FAILEDTORENAME));
			return true;
		}

		if (pItem->IsReadOnly())
		{
			pStatus->SetError(App.LoadString(IDS_READONLY));
			return true;
		}

		// Rename
		TCHAR szNewName[_MAX_FNAME];
		TCHAR szOrgName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR]; 

		CString strPath = pItem->GetFilePath();
		_tsplitpath_s(strPath, szDrive, countof(szDrive), szDir, countof(szDir), szOrgName, countof(szOrgName), szExt, countof(szExt));

		// Work out new file name.
		int i = _strTemplate.GetLength();
		int nOrgLen = _tcsclen(szOrgName);
		int iDiv = _nPosition;

		while(i >= 0)
		{
			switch(_strTemplate[i])
			{
			case _T('#'):
				szNewName[i] = szNumbers[iDiv % 10];
				iDiv /= 10;
				break;

			case _T('?'):
				if (nOrgLen > i)
				{
					szNewName[i] = szOrgName[i];
				}
				else
				{
					szNewName[i] = ' ';
				}
				break;

			default:
				szNewName[i] = _strTemplate[i];
				break;
			}

			i-= 1;
		}

		// do the rename
		TCHAR sz[MAX_PATH];
		_tmakepath_s(sz, MAX_PATH, szDrive, szDir, szNewName, szExt );

		if (::MoveFile(strPath, sz) == 0)
		{
			pStatus->SetError(App.LoadString(IDS_FAILEDTORENAME));
			return false;
		}

		// Incriment the position
		_nPosition++;

		return true;
	}

	bool EndItem()
	{
		return true;
	}

	bool EndFolder()
	{
		return true;
	}
};