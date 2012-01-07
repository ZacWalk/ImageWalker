#pragma once



class FolderState
{
private:

	typedef FolderState ThisClass;
	Coupling *_pCoupling;
	mutable CCriticalSection _cs;		

	DWORD _dwRefreshFolder;	
	int _nLoadItem;
	IW::FolderPtr _pFolder;

public:

	Delegate::List0 ChangedDelegates;
	Delegate::List0 RefreshDelegates;
	Delegate::List0 SelectionDelegates;
	Delegate::List0 FocusDelegates;

	bool _bAssending;
	int _nSortOrder;
	volatile bool IsSearchMode;
	volatile bool IsSearching;
	volatile bool IsThumbnailing;

	FolderState(Coupling *pCoupling) :
	    _pCoupling(pCoupling),			
		_dwRefreshFolder(ULONG_MAX),
		_nLoadItem(0),
		_nSortOrder(0),
		_bAssending(true),
		IsSearchMode(false),
		IsSearching(false),
		IsThumbnailing(false)
	  {
		  IW::CShellItem itemDefault;
		  itemDefault.Open(IW::GetMainWindow(), CSIDL_PERSONAL);

		  _pFolder = new IW::RefObj<IW::Folder>;
		  _pFolder->Init(itemDefault);
	  }

	  virtual ~FolderState()
	  {
		  _pFolder = 0;
	  }

public:

	void Read(IW::IPropertyArchive *pProperties)
	{
	}

	void Write(IW::IPropertyArchive *pProperties)
	{
		IW::CShellItem item = GetFolderItem();
		item.Write(g_szDefaultFolder, pProperties);
	}

	void StopLoading()
	{
		IW::CAutoLockCS lock(_cs);
		_pCoupling->StopLoadingThumbs();
	}	

	void Search(Search::Type type, const Search::Spec &ss)
	{
		IW::CAutoLockCS lock(_cs);
		_pCoupling->StopLoadingThumbs();
		_pCoupling->StartSearching();
		_pFolder->DeleteAllThumbs();
		_pCoupling->StartSearchThread(type, ss);
	}

	void RefreshInOneSecond()
	{
		IW::CAutoLockCS lock(_cs);

		if (!IsSearchMode)
		{
			_dwRefreshFolder = GetTickCount() + 1000;
		}
	}

	IW::FolderPtr GetFolder()
	{
		IW::CAutoLockCS lock(_cs);
		IW::FolderPtr pFolder = _pFolder;
		return pFolder;
	}

	IW::TAGMAP GetTags() const
	{
		IW::CAutoLockCS lock(_cs);
		return _pFolder->GetTags();
	}

	const IW::FolderPtr GetFolder() const
	{
		IW::CAutoLockCS lock(_cs);
		const IW::FolderPtr pFolder = _pFolder;
		return pFolder;
	}

	void SetFolder(IW::Folder *pFolderIn)
	{
		IW::CAutoLockCS lock(_cs);
		_pFolder = pFolderIn;
	}
	
	DWORD GetRefreshFolder() const
	{
		IW::CAutoLockCS lock(_cs);
		return _dwRefreshFolder;
	}

	void SetRefreshFolder(DWORD dw)
	{
		IW::CAutoLockCS lock(_cs);
		_dwRefreshFolder = dw;
	}	

	int GetLastLoad() const
	{
		IW::CAutoLockCS lock(_cs);
		return _nLoadItem;
	}

	void SetLastLoad(int i)
	{
		IW::CAutoLockCS lock(_cs);
		_nLoadItem = i;
	}
	

	void ResetThreads()
	{
		IW::CAutoLockCS lock(_cs);

		_pCoupling->ResetThumbThread();
		_pCoupling->ResetWaitForFolderToChangeThread();
	}

	IW::CShellItem GetFolderItem()
	{
		IW::FolderPtr pFolder = GetFolder();
		return pFolder->GetFolderItem();
	}	

	CString GetFolderPath() const
	{
		const IW::FolderPtr pFolder = GetFolder();
		return pFolder->GetFolderPath();
	}

	bool OpenFolder(const IW::CShellItem &item)
	{
		static bool bFirstTime = true;

		IW::FolderPtr pFolder = GetFolder();
		bool bIsSame = item == pFolder->GetFolderItem();
		bool bSuccess = false;

		if (!bIsSame)
		{
			IW::CFilePath path1, path2;
			item.GetPath(path1);
			pFolder->GetFolderItem().GetPath(path2);
			bIsSame = path1 == path2;
		}

		// Dont set if currently
		// is the same address
		if (bFirstTime || !bIsSame)
		{
			IW::FolderPtr pNewFolder = new IW::RefObj<IW::Folder>;

			if (pNewFolder->Init(item))
			{
				OpenFolder(pNewFolder);
				bSuccess = true;
			}
		}

		if (!bFirstTime) bFirstTime = bSuccess;
		return bSuccess;
	}

	bool OpenFolder(const CString &strPath)
	{
		USES_CONVERSION;
		IW::CShellDesktop desktop;

		LPITEMIDLIST  pidl;
		ULONG         chEaten;
		ULONG         dwAttributes;
		HRESULT       hr;

		// 
		// 
		// Convert the path to an ITEMIDLIST.
		// 
		hr = desktop->ParseDisplayName(
			NULL,
			NULL,
			(LPOLESTR)CT2OLE(strPath),
			&chEaten,
			&pidl,
			&dwAttributes);

		if (SUCCEEDED(hr))
		{
			IW::CShellItem item;
			item.Attach(pidl);

			return OpenFolder(item);
		}

		return false;
	}

	void OpenFolder(IW::FolderPtr pFolder)
	{
		StopLoading();

		pFolder->Refresh(IW::GetMainWindow());
		pFolder->Sort(_nSortOrder, _bAssending);
		pFolder->ResetCounters();
		
		SetFolder(pFolder);
		Sort();

		ResetThreads();
		ChangedDelegates.Invoke();
	}

	void OpenDefaultFolder()
	{
		IW::CShellItem item;
		if (item.Open(IW::GetMainWindow(), CSIDL_PERSONAL))
		{
			CPropertyArchiveRegistry archive(App.GetRegKey());

			if (archive.IsOpen())
				item.Read(g_szDefaultFolder, &archive);

			if (!OpenFolder(item))
				OpenFolder(IW::CShellDesktopItem());
		}
	}	


	bool OpenParent() 
	{
		IW::FolderPtr pFolder = GetFolder();
		IW::CShellItem item;
		IW::FolderPtr pNewFolder = new IW::RefObj<IW::Folder>;

		if (pFolder->GetParentItem(item) && 
			pNewFolder->Init(item))
		{
			OpenFolder(pNewFolder);
			return true;
		}

		return false;
	}

	void Sort()
	{
		IW::FolderPtr pFolder = GetFolder();

		pFolder->WriteFocus();	
		pFolder->Sort(_nSortOrder, _bAssending);
		pFolder->UpdateSelectedItems();

		int nNewFocus = pFolder->ReadFocus();

		if (nNewFocus != -1)
		{
			pFolder->SetFocusItem(nNewFocus);
		}		
	}

	void SetSortOrder(int nSortOrder, bool bAssending)
	{
		_nSortOrder = nSortOrder;
		_bAssending  = bAssending;

		Sort();
		_pCoupling->SortOrderChanged(_nSortOrder);	
	}

	void SetSortOrder(int nSortOrder)
	{
		bool bAssending = true;

		if (_nSortOrder == nSortOrder)
		{
			bAssending = !_bAssending;
		}

		SetSortOrder(nSortOrder, bAssending);
	}

	void RefreshFolder()
	{
		StopLoading();
		RefreshList(true);
	}

	void RefreshList(bool bFullRefresh)
	{
		IW::FolderPtr pFolder = GetFolder();

		StopLoading();
		SetRefreshFolder(ULONG_MAX);

		int nOldFocus = pFolder->WriteFocus();
		HRESULT hr =  pFolder->Refresh(IW::GetMainWindow());

		if (FAILED(hr))
		{
			OpenParent();
			return;
		}

		if (SUCCEEDED(hr))
		{
			pFolder->SetFocusItem(pFolder->ReadFocus());

			if (bFullRefresh)
			{
				pFolder->Sort(_nSortOrder, _bAssending);
			}			

			if (bFullRefresh)
			{
				pFolder->ResetLoadedFlag();			
			}			

			pFolder->UpdateSelectedItems();			

			SelectionDelegates.Invoke();
			RefreshDelegates.Invoke();			
			ResetThreads();
		}
	}

	void SelectAll()
	{
		GetFolder()->SelectAll();
		SelectionDelegates.Invoke();
	}
	
	void SelectInverse()
	{
		GetFolder()->SelectInverse();
		SelectionDelegates.Invoke();
	}

	void SelectImages()
	{
		GetFolder()->SelectImages();
		SelectionDelegates.Invoke();
	}
	void SelectImagesOnFlickr()
	{
		GetFolder()->SelectImagesOnFlickr();
		SelectionDelegates.Invoke();
	}
	void SelectTag(const CString &strTag)
	{
		GetFolder()->SelectTag(strTag);
		SelectionDelegates.Invoke();
	}

	inline bool HasImages() const throw()
	{
		return GetFolder()->GetImageCount() > 0;
	}

	inline int GetItemCount() const throw()
	{
		return GetFolder()->GetItemCount();
	}

	bool Select(const CString &strFile)
	{
		IW::FolderPtr pFolder = GetFolder();
		const int nItem = pFolder->Find(strFile);

		if (nItem != -1)
		{
			Select(nItem, 0, true);
			return true;
		}

		return false;
	}

	void Select(int nSelect, UINT nFlags = 0, bool bSignalEvent = true)
	{
		IW::FolderPtr pFolder = GetFolder();
		const int nFocusItemOld = pFolder->GetFocusItem();
		pFolder->Select(nSelect, nFlags);		
		SelectionDelegates.Invoke();

		const bool bFocusChanged = pFolder->GetFocusItem() != nFocusItemOld;
		if (bFocusChanged && bSignalEvent)
		{
			FocusDelegates.Invoke();
		}
	}	

	void OnFileDefault() 
	{
		IW::FolderPtr pFolder = GetFolder();

		int nSelected = pFolder->GetFocusItem();
		if (nSelected == -1)
			return;

		IW::FolderItemLock pItem(pFolder, nSelected);
		IW::FolderPtr pNewFolder = pItem->GetFolder();

		if (pNewFolder != 0)
		{
			OpenFolder(pNewFolder);
		}
		else
		{
			Open(nSelected);
		}
	}

	void Open(const int nSelected)
	{
		CComPtr<IContextMenu> spContextMenu;
		IW::FolderPtr pFolder = GetFolder();

		// May need to adjust the selection
		if (!pFolder->IsItemSelected(nSelected))
		{
			Select(nSelected, 0, true);
		}

		int nSize = pFolder->GetSize();

		// loop throught and open selected thumbs
		for(int i = 0; i < nSize; i++)
		{
			if (pFolder->IsItemSelected(i))
			{
				ShellExecute(IW::GetMainWindow(), 
					_T("Open"), 
					pFolder->GetItemPath(i),
					g_szEmptyString, 
					g_szEmptyString, 
					SW_SHOWNORMAL);
			}
		}
	}

	inline bool HasSelection() const throw()
	{
		IW::FolderPtr pFolder = GetFolder();
		return pFolder->HasSelection();
	}
};