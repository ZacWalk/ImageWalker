#pragma once

template<class T>
class CAddressBar
{

protected:
	CComboBoxEx m_wndAddress;

public:

	CAddressBar()
	{
	}

	BEGIN_MSG_MAP(CMainFrame)
		COMMAND_HANDLER(ID_ADDRESS, CBN_DROPDOWN, OnAddressDropDown)
		COMMAND_HANDLER(ID_ADDRESS, CBN_SELCHANGE, OnAddressSelectionChange)
		NOTIFY_HANDLER(ID_ADDRESS, CBEN_DELETEITEM, OnAddressDeleteItem)
	END_MSG_MAP()

	HWND CreateAddressBar()
	{
		T *pT = static_cast<T*>(this);

		// create a combo box for the address bar
		HWND hWndAddress = m_wndAddress.Create(pT->m_hWnd, 
			CRect(0,0,100,300), 
			NULL, CBS_DROPDOWN | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
			0, ID_ADDRESS);

		m_wndAddress.SetFont(IW::Style::GetFont(IW::Style::Font::Standard));
		m_wndAddress.SetImageList(App.GetShellImageList(true));
		m_wndAddress.SetExtendedStyle(CBES_EX_NOSIZELIMIT, CBES_EX_NOSIZELIMIT);

		return m_wndAddress;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			int keyCode = pMsg->wParam;

			// Special handeling for combos
			if (keyCode == VK_RETURN || 
				keyCode == VK_ESCAPE)
			{
				HWND hWndFocus = ::GetFocus();
				CEdit wndEdit = m_wndAddress.GetEditCtrl();

				if (hWndFocus == m_wndAddress || wndEdit == hWndFocus)
				{
					if (pMsg->wParam == VK_RETURN)
					{
						ParseAddress();
						return TRUE;
					}
					else if (pMsg->wParam == VK_ESCAPE)
					{
						int nSelection = m_wndAddress.GetCurSel();

						if (CB_ERR != nSelection)
						{
							m_wndAddress.SetCurSel(nSelection);
							return TRUE;
						}
					}
				}				
			}
		}

		return FALSE;
	}

	void AddressInsertItem(DWORD csidl)
	{
		T *pT = static_cast<T*>(this);

		IW::CShellItem item;
		if (item.Open(pT->m_hWnd, csidl))
		{
			AddressInsertItem(item, item.Depth());
		}
	}

	int AddressInsertItem(const IW::CShellItem &item, int nIndent, bool bFocusItem = false)
	{
		IW::CShellDesktop desktop;
		int nPos = -1, i;

		// Remove the elements with even key values.
		IW::CShellItem *pItem;

		int count = m_wndAddress.GetCount();

		for (i = 0; i < count; i++)
		{
			pItem = reinterpret_cast<IW::CShellItem*>(m_wndAddress.GetItemData(i));

			short n = item.Compare(desktop, *pItem);

			if (n == 0)
			{
				return i; // Already Have!!
			}

			if (n < 0)
			{
				nPos = i;
				break;
			}
		}

		// Insert the item
		SHFILEINFO sfi;
		IW::MemZero(&sfi, sizeof(sfi));

		SHGetFileInfo((LPCTSTR)(LPCITEMIDLIST)item,
			0,
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_PIDL |
			SHGFI_SYSICONINDEX |
			SHGFI_SMALLICON |
			SHGFI_DISPLAYNAME);

		if (bFocusItem)
		{
			SHGetPathFromIDList(item, sfi.szDisplayName);
		}

		m_wndAddress.InsertItem(nPos, sfi.szDisplayName, sfi.iIcon, sfi.iIcon, nIndent, (LPARAM)new IW::CShellItem(item));

		return i;
	}

	void AddressInsertFolder(const IW::CShellItem &item, int nIndent)
	{
		T *pT = static_cast<T*>(this);

		IW::CShellFolder spFolder;
		HRESULT hr = spFolder.Open(item, true);

		// Not a folder?
		ATLASSERT(SUCCEEDED(hr));

		if (FAILED(hr))
			return;

		IW::CShellItemEnum shellenum;

		hr = shellenum.Create(pT->m_hWnd, spFolder, SHCONTF_FOLDERS);

		if (FAILED(hr))
			return;

		LPITEMIDLIST pItem = NULL;
		ULONG ulFetched = 0;
		IW::CShellItem itemCat, itemEnum;

		while (shellenum->Next(1, &pItem, &ulFetched) == S_OK)
		{
			itemEnum.Attach(pItem);
			itemCat.Cat(item, itemEnum);

			AddressInsertItem(itemCat, nIndent);
		}
	}

	// Handel new address selection
	LRESULT OnAddressSelectionChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)	
	{
		AddressSelectionChanged();
		return 0;
	}

	LRESULT OnAddressDropDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T *pT = static_cast<T*>(this);

		CWaitCursor wait;
		// Populate the box

		m_wndAddress.ResetContent();

		IW::CShellDesktopItem itemDesktop;


		int nSelection = AddressInsertItem(itemDesktop, 0);
		AddressInsertFolder(itemDesktop, 1);

		AddressInsertItem(CSIDL_MYPICTURES);
		AddressInsertItem(CSIDL_MYVIDEO);
		AddressInsertItem(CSIDL_MYMUSIC);

		IW::CShellItem itemDrives;
		if (itemDrives.Open(pT->m_hWnd, CSIDL_DRIVES))
		{
			AddressInsertFolder(itemDrives, 2);
		}

		IW::CShellItem item = pT->_state.Folder.GetFolderItem();
		bool bInDesktopFolder = item.IsDesktop();

		// select new address
		if (!bInDesktopFolder)
		{
			IW::CShellItem itemCopy(item);
			itemCopy.StripToParent();

			while(!itemCopy.IsDesktop())
			{
				AddressInsertItem(itemCopy, itemCopy.Depth());
				itemCopy.StripToParent();
			}

			nSelection = AddressInsertItem(item, item.Depth(), false);
		}

		m_wndAddress.SetCurSel(nSelection);

		return 0;
	}

	


	// Handel Address Items being deleted
	LRESULT OnAddressDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)	
	{
		PNMCOMBOBOXEX pCBEx = (PNMCOMBOBOXEX) pnmh;

		int nSelection = m_wndAddress.GetCurSel();

		if (pCBEx->ceItem.lParam)
		{
			IW::CShellItem *pItem = reinterpret_cast<IW::CShellItem*>(pCBEx->ceItem.lParam);
			delete pItem;
		}

		return 0;
	}


	void AddressSelectionChanged()
	{
		int nSelection = m_wndAddress.GetCurSel();

		if (CB_ERR != nSelection)
		{
			IW::CShellItem *pItem= reinterpret_cast<IW::CShellItem*>(m_wndAddress.GetItemData(nSelection));

			AddressSelectionChanged(*pItem);
		}
	}

	void AddressSelectionChanged(const IW::CShellItem &item)
	{
		T *pT = static_cast<T*>(this);
		pT->_state.Folder.OpenFolder(item);
	}


	void ParseAddress()
	{
		T *pT = static_cast<T*>(this);

		USES_CONVERSION;

		IW::CShellDesktop desktop;
		LPITEMIDLIST  pidl;
		ULONG         chEaten;
		ULONG         dwAttributes;
		HRESULT       hr;

		CString str, strEntered;

		m_wndAddress.GetWindowText(strEntered);

		if (strEntered == _T(".."))
		{
			pT->_state.Folder.OpenParent();
			return;
		}

		str = IW::Path::Combine(pT->GetFolderPath(), strEntered);

		hr = desktop->ParseDisplayName(
			NULL,
			NULL,
			CT2OLE(strEntered),
			&chEaten,
			&pidl,
			&dwAttributes);

		if (SUCCEEDED(hr))
		{
			IW::CShellItem item;
			item.Attach(pidl);
			AddressSelectionChanged(item);
			return;
		}

		hr = desktop->ParseDisplayName(
			NULL,
			NULL,
			CT2OLE(strEntered),
			&chEaten,
			&pidl,
			&dwAttributes);

		if (SUCCEEDED(hr))
		{
			IW::CShellItem item;
			item.Attach(pidl);
			AddressSelectionChanged(item);
			return;
		}

		MessageBeep(MB_OK);
	}


	void SetAddress(LPCTSTR szName, int nIcon)
	{
		COMBOBOXEXITEM     cbi;
		cbi.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT;		
		cbi.iItem = -1;
		cbi.pszText = (LPTSTR)szName;
		cbi.cchTextMax = _tcsclen(szName);
		cbi.iImage = nIcon;
		cbi.iSelectedImage = nIcon;

		m_wndAddress.SetItem(&cbi);
	}
};