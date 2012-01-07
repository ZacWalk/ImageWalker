///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


class ACCCOMMAND
{
public:
	ACCCOMMAND()
	{
	}

	ACCCOMMAND(CString strCommand) : _strCommand(strCommand)
	{
	}

	ACCCOMMAND(const ACCCOMMAND &other) : _strCommand(other._strCommand), _nImage(other._nImage), _id(other._id)
	{
	}   

	void operator=(const ACCCOMMAND &other)
	{
		_strCommand = other._strCommand;
		_nImage = other._nImage; 
		_id = other._id;
	}

	bool operator<(const ACCCOMMAND &other) const
	{
		return _strCommand.CompareNoCase(other._strCommand) < 0;
	}

	CString _strCommand;
	int _nImage;
	int _id;
};

typedef std::map<int, ACCCOMMAND> MAPIDTOACCCOMMAND;

class CAcceleratorEditDlg : public CDialogImpl<CAcceleratorEditDlg>
{
public:

	typedef std::vector<ACCCOMMAND> COMMANDLIST;  
	COMMANDLIST _commands;

	ACCEL m_acc;

	CAcceleratorEditDlg()
	{
		m_acc.fVirt = 0;
		m_acc.key = 0;
		m_acc.cmd = 0;
	}


	enum { IDD = IDD_ACCELERATOR_EDIT };

	BEGIN_MSG_MAP(CAcceleratorEditDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());

		// Sort
		std::sort(_commands.begin(), _commands.end());    

		CComboBoxEx combo = GetDlgItem(IDC_COMMAND);
		combo.SetImageList(App.GetGlobalBitmap());

		for(COMMANDLIST::iterator it = _commands.begin(); it != _commands.end(); ++it)
		{
			ACCCOMMAND &a = *it;
			combo.InsertItem(-1, a._strCommand, a._nImage, a._nImage, 0, a._id);
		}

		// Default selection

		int i = 0;

		if (m_acc.cmd != 0)
		{
			int nCount = combo.GetCount(); 

			for(i = 0; i < nCount; i++)
			{
				int nId = combo.GetItemData(i);

				if (m_acc.cmd == nId)
				{
					break;
				}
			}

		}

		combo.SetCurSel(i);

		// Hot key
		HWND hwndHotKey = GetDlgItem(IDC_HOTKEY);
		SendMessage(hwndHotKey, HKM_SETHOTKEY, GetHotKey(), 0);

		return (LRESULT)TRUE;
	}

	DWORD GetHotKey()
	{
		DWORD dw = 0;
		DWORD dwVert = 0;

		if (m_acc.fVirt & FALT)
		{
			dw |= HOTKEYF_ALT;
		}
		if (m_acc.fVirt & FCONTROL)
		{
			dw |= HOTKEYF_CONTROL;
		}
		if (m_acc.fVirt & FSHIFT)
		{
			dw |= HOTKEYF_SHIFT;
		}

		return MAKEWORD(m_acc.key, dw);

	}

	void SetHotKey(DWORD dw)
	{
		DWORD dwLow = HIBYTE(dw);

		m_acc.fVirt = FVIRTKEY;

		if (dwLow & HOTKEYF_ALT)
		{
			m_acc.fVirt |= FALT;
		}
		if (dwLow & HOTKEYF_CONTROL)
		{
			m_acc.fVirt |= FCONTROL;
		}
		if (dwLow & HOTKEYF_SHIFT)
		{
			m_acc.fVirt |= FSHIFT;
		}


		m_acc.key = LOBYTE(dw);

	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (wID == IDOK)
		{
			HWND hwndCombo = GetDlgItem(IDC_COMMAND);

			int nSelection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
			m_acc.cmd = static_cast<WORD>(SendMessage(hwndCombo, CB_GETITEMDATA, nSelection, 0L));

			// Set the key
			HWND hwndHotKey = GetDlgItem(IDC_HOTKEY);
			SetHotKey(SendMessage(hwndHotKey, HKM_GETHOTKEY, 0, 0));
		}

		EndDialog(wID);
		return 0;
	}
};


class CAcceleratorListDlg : public CDialogImpl<CAcceleratorListDlg>
{
public:
	enum { IDD = IDD_ACCELERATOR_LIST };

	HACCEL m_hAccel;
	CSimpleArray<ACCEL> m_arrAccel;
	MAPIDTOACCCOMMAND _mapCommands;


	CAcceleratorListDlg()
	{
	}


	BEGIN_MSG_MAP(CAcceleratorListDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_EDIT, OnEdit)
		COMMAND_ID_HANDLER(IDC_REMOVE, OnRemove)
		COMMAND_ID_HANDLER(IDC_ADD, OnAdd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()


	LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DWORD dw = GetSelected();

		if (dw)
		{

			CAcceleratorEditDlg dlg;

			for(MAPIDTOACCCOMMAND::iterator it = _mapCommands.begin(); it != _mapCommands.end(); ++it)
			{
				dlg._commands.push_back(it->second);
			}

			for(int i = 0; i < m_arrAccel.GetSize(); ++i)
			{
				ACCEL &a = m_arrAccel[i];

				if (MAKELONG(a.fVirt, a.key) == dw)
				{
					dlg.m_acc = a;
				}
			}

			if (IDOK == dlg.DoModal() && dlg.m_acc.key)
			{
				Delete(dw);
				m_arrAccel.Add(dlg.m_acc);
				AddAccel(dlg.m_acc);
				Find(MAKELONG(dlg.m_acc.fVirt, dlg.m_acc.key));
			}
		}

		return 0;
	}

	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DWORD dw = GetSelected();
		if (dw)
		{
			Delete(dw);
		}

		return 0;
	}

	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CAcceleratorEditDlg dlg;

		for(MAPIDTOACCCOMMAND::iterator it = _mapCommands.begin(); it != _mapCommands.end(); ++it)
		{
			dlg._commands.push_back(it->second);
		}

		if (IDOK == dlg.DoModal() && dlg.m_acc.key)
		{
			m_arrAccel.Add(dlg.m_acc);
			AddAccel(dlg.m_acc);
			Find(MAKELONG(dlg.m_acc.fVirt, dlg.m_acc.key));
		}

		return 0;
	}


	static void StripMenuString(LPTSTR szOut, LPTSTR szIn)
	{
		// Rest shortcut
		TCHAR *pch = szIn, *pch2 = szOut;
		for (; *pch != '\0'; ) 
		{ 
			if (*pch != '&') 
			{ 
				if (*pch == '\t') 
				{ 
					*pch = '\0'; 
					*pch2 = '\0'; 
				} 
				else 
				{
					TCHAR ch = *pch++;
					*pch2++ = ch;
				}
			} 
			else pch++; 
		}

		*pch2++ = 0;
	}


	void LoadAccel(HACCEL hAccel)
	{
		m_arrAccel.RemoveAll();

		int nAccel;
		if (hAccel && (nAccel = CopyAcceleratorTable(hAccel, NULL, 0)) > 0) 
		{
			IW::CAutoFree<ACCEL> pAccel(nAccel);
			CopyAcceleratorTable(hAccel, pAccel, nAccel);

			for (int i=0; i<nAccel; i++) 
			{
				m_arrAccel.Add(pAccel[i]);
			}
		}
	}

	void ConstructKeyString(LPTSTR szOut, int nStringBufferSize, ACCEL &a)
	{
		szOut[0] = 0;

		if (a.fVirt & FALT)
		{
			_tcscat_s(szOut, nStringBufferSize, App.LoadString(IDS_ALT_PLUS));
		}
		if (a.fVirt & FCONTROL)
		{
			_tcscat_s(szOut, nStringBufferSize, App.LoadString(IDS_CTRL_PLUS));
		}
		if (a.fVirt & FSHIFT)
		{
			_tcscat_s(szOut, nStringBufferSize, App.LoadString(IDS_SHIFT_PLUS));
		}

		if (a.fVirt & FVIRTKEY) 
		{
			TCHAR keyname[64];
			UINT vkey = MapVirtualKey(a.key, 0)<<16;
			GetKeyNameText(vkey, keyname, countof(keyname));
			_tcscat_s(szOut, nStringBufferSize, keyname);
		} 
		else
		{
			TCHAR szTemp[2] = { (TCHAR)a.key, 0 };
			_tcscat_s(szOut, nStringBufferSize, szTemp);
		}


		return;
	}


	template<class TImageMap>
	void BuildIdToStringMap(HMENU hMenu, const TImageMap &mapImages)
	{
		TCHAR szIn[MAX_PATH + 1];        // temporary buffer 
		TCHAR szOut[MAX_PATH + 1]; // buffer for menu-item text 

		DWORD cItems = GetMenuItemCount(hMenu); 
		for (DWORD i = 0; i < cItems; i++) 
		{
			MENUITEMINFO mii; 
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_SUBMENU | MIIM_ID | MIIM_TYPE;
			mii.dwTypeData = szIn; 
			mii.cch = MAX_PATH; 

			if (GetMenuItemInfo(hMenu, i, TRUE, &mii)
				&& !(mii.fType & MFT_SEPARATOR) &&
				GetMenuString(hMenu, i, szIn, MAX_PATH, MF_BYPOSITION))
			{
				StripMenuString(szOut, szIn);

				ACCCOMMAND cmd(szOut);
				cmd._id = mii.wID;

				TImageMap::const_iterator itImage = mapImages.find(mii.wID);
				cmd._nImage = (itImage != mapImages.end()) ? itImage->second : ImageIndex::None;

				_mapCommands[mii.wID] = cmd;

				if (mii.hSubMenu)
				{
					BuildIdToStringMap(mii.hSubMenu, mapImages);
				}
			}
		} 
	}

	DWORD GetSelected()
	{
		HWND hLV = GetDlgItem(IDC_LIST);

		int iSel = ListView_GetNextItem( hLV, -1, LVNI_SELECTED );

		if ( iSel >= 0 )
		{
			IW::ListViewItem lvi(iSel);

			if ( ListView_GetItem( hLV, &lvi ))
			{
				return lvi.lParam;
			}

		}

		return 0;
	}

	void Find(DWORD dw)
	{
		HWND hLV = GetDlgItem(IDC_LIST);

		LVFINDINFO f;
		f.flags = LVFI_PARAM;
		f.lParam = dw;

		int nItem = ListView_FindItem(hLV, -1, &f);
		ListView_SetItemState (hLV, nItem, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
	}

	void Delete(DWORD dw)
	{
		HWND hLV = GetDlgItem(IDC_LIST);

		LVFINDINFO f;
		f.flags = LVFI_PARAM;
		f.lParam = dw;

		int nItem = ListView_FindItem(hLV, -1, &f);

		ListView_DeleteItem (hLV, nItem);

		for(int i = 0; i < m_arrAccel.GetSize(); ++i)
		{
			ACCEL &a = m_arrAccel[i];

			if (MAKELONG(a.fVirt, a.key) == dw)
			{
				m_arrAccel.RemoveAt(i);
			}
		}
	}

	void AddAccel(ACCEL &a)
	{
		HWND hLV = GetDlgItem(IDC_LIST);

		LVITEM lvItem;
		IW::MemZero(&lvItem,sizeof(LVITEM));

		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iItem = 0;
		lvItem.iSubItem = 0;
		lvItem.lParam = MAKELONG(a.fVirt, a.key);

		MAPIDTOACCCOMMAND::iterator itCommand = _mapCommands.find(a.cmd);

		if (_mapCommands.end() != itCommand)
		{
			lvItem.mask |= LVIF_IMAGE;
			lvItem.iImage = itCommand->second._nImage;
			lvItem.pszText = (LPTSTR)(LPCTSTR)itCommand->second._strCommand;
		}

		const int nStringBufferSize = 100;
		TCHAR sz[nStringBufferSize + 1];
		ConstructKeyString(sz, nStringBufferSize, a);

		int dIndex = ListView_InsertItem(hLV,&lvItem);

		// Add subitems
		lvItem.mask = TVIF_TEXT;
		lvItem.iItem = dIndex;
		lvItem.iSubItem = 1;
		lvItem.pszText = sz;

		ListView_SetItem(hLV,&lvItem);
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());

		LoadAccel(m_hAccel);

		HWND hLV = GetDlgItem(IDC_LIST);

		// Assign image list to control
		ListView_SetImageList(hLV,App.GetGlobalBitmap(),LVSIL_SMALL);

		// Populate the list
		// Add columns
		LVCOLUMN lvColumn;

		lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.cx = 120;
		lvColumn.pszText = (LPTSTR)(LPCTSTR)App.LoadString(IDS_COMMAND);
		ListView_InsertColumn(hLV,0,&lvColumn);

		lvColumn.pszText = (LPTSTR)(LPCTSTR)App.LoadString(IDS_HOT_KEY);
		ListView_InsertColumn(hLV,1,&lvColumn);

		// Set the number of items in the list view to ITEM_COUNT.
		ListView_SetItemCount (hLV, 2);

		for(int i = 0; i < m_arrAccel.GetSize(); ++i)
		{
			AddAccel(m_arrAccel[i]);
		}

		return (LRESULT)TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (wID == IDOK)
		{
			// Create new accelerator list
			DestroyAcceleratorTable(m_hAccel);
			m_hAccel = CreateAcceleratorTable(m_arrAccel.GetData(), m_arrAccel.GetSize());
		}

		EndDialog(wID);
		return 0;
	}
};




template<class TParent>
class CommandViewAccelerators : public CommandBase
{
public:
	TParent *_pParent;

	CommandViewAccelerators(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CAcceleratorListDlg dlg;

		dlg.m_hAccel = _pParent->m_hAccel;
		dlg.BuildIdToStringMap(_pParent->m_CmdBar.m_hMenu, _pParent->m_CmdBar.m_mapCommand);


		if (dlg.DoModal() == IDOK)
		{
			_pParent->m_hAccel = dlg.m_hAccel;
			_pParent->UpdateMenuStrings(_pParent->m_hAccel, _pParent->m_CmdBar.m_hMenu);

			// Save the hot keys
			CPropertyArchiveRegistry archive(App.GetRegKey(), true);

			if (archive.StartSection(g_szKeys))
			{
				int nCount = dlg.m_arrAccel.GetSize();
				if (archive.Write(g_szCount, nCount))
				{
					for(int i = 0; i < nCount; ++i)
					{
						ACCEL &a = dlg.m_arrAccel[i];

						MAPIDTOACCCOMMAND::iterator itCommand = dlg._mapCommands.find(a.cmd);

						if (dlg._mapCommands.end() != itCommand)
						{
							CString str;
							str.Format(g_szIdItemTag, i);
							archive.Write(str, itCommand->second._strCommand);

							str.Format(g_szKeyItemTag, i);
							archive.Write(str, MAKELONG(a.fVirt, a.key));							
						}
					}
				}

				archive.EndSection();
			}
		}

		return;
	}
};
