///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// FolderTreeView.h: interface for the FolderTreeView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define TVI_NULL                ((HTREEITEM)(ULONG_PTR)0)

class FolderTreeView :
	public CWindowImpl<FolderTreeView>,
	public CComObjectRootEx<CComSingleThreadModel>,	
	public IDropTarget
{
	typedef FolderTreeView ThisClass;

protected:

	CTreeViewCtrl _tree;
	State &_state;

	CComPtr<IContextMenu2> _pContextMenu2;
	CComPtr<IDropTarget> _pDropTarget;
	CComPtr<IDataObject>  _pDataObject;

	HTREEITEM _hDragOverItem;	
	IW::CShellDesktop _pDesktop;	
	

public:

	FolderTreeView(State &state) :
		_state(state), 
		_hDragOverItem(TVI_NULL)
	{		
	}

	BEGIN_MSG_MAP(ThisClass)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		NOTIFY_CODE_HANDLER( TVN_ITEMEXPANDING, OnItemExpanding)
		NOTIFY_CODE_HANDLER( NM_RCLICK, OnRightClick)
		NOTIFY_CODE_HANDLER( TVN_SELCHANGED, OnSelChanged)
		NOTIFY_CODE_HANDLER( TVN_DELETEITEM, OnDelItem)
		NOTIFY_CODE_HANDLER( TVN_GETDISPINFO, OnGetDispInfo)

		if (ProcessContextMenuMessage(hWnd, uMsg, wParam, lParam, lResult))
			return TRUE;

	END_MSG_MAP()	

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		DWORD dwStyle = IW_WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS;
		HWND hWnd = _tree.Create(m_hWnd, rcDefault, 0, dwStyle, 0, 106);
		_tree.SetImageList(App.GetShellImageList(true), TVSIL_NORMAL);

		HTREEITEM hItem = InsertItem(new IW::CShellDesktopItem());
		_tree.Expand(hItem, TVE_EXPAND);

		//SetCurFolder(_state.Folder.GetFolderItem());

		RegisterDropTarget();	

		_state.Folder.ChangedDelegates.Bind(this, &ThisClass::OnFolderChanged);
		_state.Folder.RefreshDelegates.Bind(this, &ThisClass::OnFolderRefresh);

		return 1;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		const int cx = LOWORD(lParam);
		const int cy = HIWORD(lParam);

		_tree.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy FolderTreeView\n")); 
		_tree.SetImageList(NULL, TVSIL_NORMAL);
		RevokeDropTarget();

		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	static IW::CShellItem *CreateItem(const IW::CShellItem &itemParent, IW::CShellItem &itemChild)
	{
		IW::CShellItem *pItem = new IW::CShellItem;	

		if (itemParent.IsDesktop())
		{
			pItem->Attach(itemChild.Detach());
		}
		else
		{
			pItem->Cat(itemParent, itemChild);
		}

		return pItem;
	}

	void FillTreeView(LPSHELLFOLDER pFolderRaw, IW::CShellItem &itemParent, HTREEITEM     hParent)
	{
		IW::CShellItemEnum enumitem;
		IW::CShellFolder pFolder = pFolderRaw;

		UINT dwFlags = SHCONTF_FOLDERS;
		if (App.Options.m_bShowHidden) dwFlags |= SHCONTF_INCLUDEHIDDEN;

		HRESULT hr = enumitem.Create(m_hWnd, pFolder, dwFlags);

		if (FAILED(hr))
		{
			return;
		}

		LPITEMIDLIST pItemRaw = NULL;
		ULONG ulFetched = 0;
		IW::CShellItem item;

		while (enumitem->Next(1, &pItemRaw, &ulFetched) == S_OK)
		{
			item.Attach(pItemRaw);
			IW::CShellItem *pItem = CreateItem(itemParent, item);

			if (!InsertItem(pItem, hParent))
				delete pItem;
		}
	}

	HTREEITEM InsertItem(IW::CShellItem *pItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hPrev = TVI_LAST)
	{
		TV_INSERTSTRUCT tvins;

		tvins.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvins.item.cChildren = GetFolderItemSubFolders(*pItem);
		tvins.item.pszText    = LPSTR_TEXTCALLBACK;
		tvins.item.cchTextMax = -1;
		tvins.item.lParam = (LPARAM)pItem;
		tvins.item.iImage = GetFolderItemImageIndex(*pItem, false);
		tvins.item.iSelectedImage = GetFolderItemImageIndex(*pItem, true);

		tvins.hInsertAfter = hPrev;
		tvins.hParent      = hParent;

		return _tree.InsertItem(&tvins);
	}

	void ShowMenu(IW::CShellItem *pItemParent, IW::CShellItem *pItem, const CPoint &pt)
	{
		ATLASSERT(pItemParent && pItem);

		CComPtr<IContextMenu> spContextMenu;
		CMINVOKECOMMANDINFO cmi;
		DWORD               dwAttribs=0;
		int                 idCmd;

		IW::CShellFolder pFolder;
		HRESULT hr = pFolder.Open(*pItemParent, true);

		if (SUCCEEDED(hr))
		{
			LPCITEMIDLIST pTailItem = pItem->GetTailItem();
			hr= pFolder->GetUIObjectOf(m_hWnd, 1, &pTailItem, IID_IContextMenu, 0, (LPVOID *)&spContextMenu);

			if (SUCCEEDED(hr))  
			{
				CMenu menu;

				if (menu.CreatePopupMenu())
				{
					hr = spContextMenu->QueryContextMenu(menu, 0, 1, 0x7fff, CMF_EXPLORE);

					if (SUCCEEDED(hr))
					{
						spContextMenu->QueryInterface(IID_IContextMenu2, (void**)&_pContextMenu2);

						idCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

						if (idCmd)
						{
							cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cmi.fMask  = 0;
							cmi.hwnd   = m_hWnd;
							cmi.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmd-1);
							cmi.lpParameters = NULL;
							cmi.lpDirectory  = NULL;
							cmi.nShow        = SW_SHOWNORMAL;
							cmi.dwHotKey     = 0;
							cmi.hIcon        = NULL;
							
							spContextMenu->InvokeCommand(&cmi);
						}

						_pContextMenu2.Release();
					}

					_tree.Invalidate();
				}
			} 
		}
	}

	BOOL ProcessContextMenuMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
	{
		if (_pContextMenu2 != NULL)
		{
			if (WM_DRAWITEM == uMsg ||
				WM_MEASUREITEM == uMsg ||
				WM_INITMENUPOPUP  == uMsg ||
				WM_MENUCHAR == uMsg)
			{
				return SUCCEEDED(_pContextMenu2->HandleMenuMsg(uMsg, wParam, lParam));
			}
		}

		return FALSE;
	}

	static int CALLBACK TreeViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort)
	{
		FolderTreeView *pThis = (FolderTreeView*)lparamSort;
		IW::CShellItem* pItem1=(IW::CShellItem*)lparam1;
		IW::CShellItem* pItem2=(IW::CShellItem*)lparam2;

		return pItem1->Compare(pThis->_pDesktop, *pItem2);
	}

	void FolderExpanding(NMHDR* pNMHDR) 
	{
		IW::CShellItem*   pItem;   
		NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;

		if ((pnmtv->itemNew.state & TVIS_EXPANDEDONCE) == 0)
		{
			pItem = (IW::CShellItem*)pnmtv->itemNew.lParam;

			if (pItem)
			{
				IW::CShellFolder pNewFolder;
				HRESULT hr = pNewFolder.Open(*pItem, true);

				if (SUCCEEDED(hr))
				{
					FillTreeView(pNewFolder, *pItem, pnmtv->itemNew.hItem);
					SortChildren(pnmtv->itemNew.hItem);
				}
			}	
		}
	}


	void ShowMenu(NMHDR* pNMHDR) 
	{
		CPoint			pt;
		IW::CShellItem*	pItem;
		IW::CShellItem*	pItemParent;

		TV_HITTESTINFO	tvhti;
		TV_ITEM			tvi;

		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		tvhti.pt=pt;
		_tree.HitTest(&tvhti);
		_tree.SelectItem(tvhti.hItem);

		if (tvhti.flags & (TVHT_ONITEMLABEL|TVHT_ONITEMICON))
		{
			ClientToScreen(&pt);
			tvi.mask=TVIF_PARAM;
			tvi.hItem=tvhti.hItem;

			if (_tree.GetItem(&tvi))
			{
				pItem = (IW::CShellItem*)tvi.lParam;

				// Also need the parent item
				HTREEITEM htiParent = _tree.GetParentItem( tvhti.hItem);

				if (htiParent)
				{
					tvi.hItem=htiParent;

					if (_tree.GetItem(&tvi))
					{
						pItemParent = (IW::CShellItem*)tvi.lParam;

						ShowMenu(pItemParent, pItem, pt);
						OnFolderRefresh();
					}
				}
			}
		}	
	}

	LRESULT OnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pnmh;
		FolderExpanding(pnmh);	
		return 0;
	}

	LRESULT OnSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pnmh;

		IW::CShellItem* pItem = (IW::CShellItem*)pNMTreeView->itemNew.lParam;

		if (pNMTreeView->action == TVC_BYMOUSE ||
			pNMTreeView->action ==  TVC_BYKEYBOARD )
		{
			_state.Folder.OpenFolder(*pItem);
		}

		return 0;
	}

	LRESULT OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		ShowMenu(pnmh);
		return 0;
	}

	LRESULT OnDelItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) pnmh;
		IW::CShellItem *pItem = (IW::CShellItem*)pnmtv->itemOld.lParam;
		delete pItem;

		return 0;
	}

	LRESULT OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO)pnmh;
		IW::CShellItem *pItem = (IW::CShellItem*)lptvdi->item.lParam;

		if (lptvdi->item.mask & TVIF_TEXT)
		{
			CString str = GetFolderItemDisplayName(*pItem);
			_tcsncpy_s(lptvdi->item.pszText, lptvdi->item.cchTextMax, str, lptvdi->item.cchTextMax);
			//lptvdi->item.pszText = sfi.szDisplayName;
			//lptvdi->item.mask &= ~TVIF_TEXT;
		}

		return 0;
	}

	void SetCurFolder(const IW::CShellItem &item)
	{
		HTREEITEM hItem = FindItemData(item);

		if (hItem)
			_tree.SelectItem(hItem);
	}

	int GetFolderItemImageIndex(const IW::CShellItem &item, bool bOpen)
	{
		SHFILEINFO sfi;
		IW::MemZero(&sfi, sizeof(sfi));

		UINT uFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX  | SHGFI_SMALLICON;
		if (bOpen) uFlags |= SHGFI_OPENICON;

		SHGetFileInfo((LPCTSTR)(LPCITEMIDLIST)item.GetItem(), 0, &sfi, sizeof(SHFILEINFO), uFlags);
		return sfi.iIcon;
	}

	CString GetFolderItemDisplayName(const IW::CShellItem &item)
	{
		SHFILEINFO sfi;
		IW::MemZero(&sfi, sizeof(sfi));

		UINT uFlags = SHGFI_PIDL | SHGFI_DISPLAYNAME;
		SHGetFileInfo((LPCTSTR)(LPCITEMIDLIST)item.GetItem(), 0, &sfi, sizeof(SHFILEINFO), uFlags);
		return sfi.szDisplayName;
	}

	int GetFolderItemSubFolders(const IW::CShellItem &item)
	{
		SHFILEINFO sfi;
		IW::MemZero(&sfi, sizeof(sfi));
		sfi.dwAttributes = SFGAO_HASSUBFOLDER;

		UINT uFlags = SHGFI_PIDL | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;
		SHGetFileInfo((LPCTSTR)(LPCITEMIDLIST)item.GetItem(), 0, &sfi, sizeof(SHFILEINFO), uFlags);
		return (sfi.dwAttributes & SFGAO_HASSUBFOLDER) ? 1 : 0;
	}

	void RefreshNode(HTREEITEM htiFolder)
	{
		IW::CShellItem*	pItemFolder = (IW::CShellItem*)_tree.GetItemData( htiFolder );

		if (pItemFolder == 0)
		{
			ATLASSERT(0);
			return;
		}		

		TVITEM item;
		item.hItem = htiFolder;
		item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		item.cChildren = GetFolderItemSubFolders(*pItemFolder);
		item.pszText    = LPSTR_TEXTCALLBACK;
		item.cchTextMax = -1;
		item.lParam = (LPARAM)pItemFolder;
		item.iImage = GetFolderItemImageIndex(*pItemFolder, false);
		item.iSelectedImage = GetFolderItemImageIndex(*pItemFolder, true);

		_tree.SetItem(&item);


		if (_tree.GetItemState(htiFolder, TVIS_EXPANDED) & TVIS_EXPANDED )
		{
			IW::CShellFolder spFolder;
			HRESULT hr = spFolder.Open(*pItemFolder, true);

			if (FAILED(hr))
			{
				ATLASSERT(0);//?
				return;
			}

			typedef std::map<IW::CShellItem, HTREEITEM> MAPITEMS;
			MAPITEMS mapItems;

			// Build up a map of items?
			HTREEITEM hti = _tree.GetChildItem( htiFolder );
			IW::CShellItem*	pItemChild;

			while( hti )
			{
				pItemChild = (IW::CShellItem*)_tree.GetItemData( hti );
				mapItems[*pItemChild] = hti;

				hti = _tree.GetNextItem( hti, TVGN_NEXT);
			}

			// Loop through and look at the current folder
			// Get the IEnumIDList object for the given folder.
			IW::CShellItemEnum enumitem;

			UINT dwFlags = SHCONTF_FOLDERS;
			if (App.Options.m_bShowHidden) dwFlags |= SHCONTF_INCLUDEHIDDEN;

			hr = enumitem.Create(m_hWnd, spFolder, dwFlags);

			if (FAILED(hr))
			{
				return;
			}

			LPITEMIDLIST pItemRaw = NULL;
			ULONG ulFetched = 0;
			IW::CShellItem item;


			while (enumitem->Next(1, &pItemRaw, &ulFetched) == S_OK)
			{
				item.Attach(pItemRaw);

				//Now, make a copy of the ITEMIDLIST
				IW::CShellItem *pItem = CreateItem(*pItemFolder, item);
				MAPITEMS::iterator i = mapItems.find(*pItem); 

				if (i == mapItems.end())
				{
					// New item insert it
					if (!InsertItem(pItem, htiFolder))
						delete pItem;
				}
				else
				{
					// strip the load status so we can reload it
					mapItems.erase(i);
					delete pItem;
				}
			}

			// Remove any items that were
			// not located
			MAPITEMS::iterator i = mapItems.begin();

			while (i != mapItems.end())
			{
				_tree.DeleteItem(i->second);
				++i;
			}

			SortChildren(htiFolder);

			// Refresh any children
			hti = _tree.GetChildItem( htiFolder );

			while( hti )
			{
				RefreshNode(hti);
				hti = _tree.GetNextItem( hti, TVGN_NEXT);
			}
		}
	}

	void OnFolderChanged()
	{
		SetCurFolder(_state.Folder.GetFolderItem());
	}

	void OnFolderRefresh()
	{
		IW::CLockWindowUpdate lock(_tree);

		/*HTREEITEM hti = _tree.GetChildItem( TVI_ROOT );

		while( hti )
		{
		RefreshNode(hti);
		hti = _tree.GetNextItem( hti, TVGN_NEXT);
		}*/

		HTREEITEM hti = _tree.GetSelectedItem();
		RefreshNode(hti);
	}

	HTREEITEM FindItemData(const IW::CShellItem &item)
	{
		HTREEITEM hItem = TVI_ROOT;

		if (item.Depth() > 0)
		{
			IW::CShellItem itemParent(item);
			itemParent.StripToParent();
			hItem = FindItemData(itemParent);
		}

		if(hItem == NULL)
			return NULL;

		_tree.Expand(hItem, TVE_EXPAND);

		HTREEITEM htiCur = _tree.GetChildItem( hItem );

		while( htiCur )
		{
			IW::CShellItem*	pItemChild = (IW::CShellItem*)_tree.GetItemData( htiCur );

			if (item.Compare(_pDesktop, *pItemChild) == 0)
				return htiCur;

			htiCur = _tree.GetNextItem( htiCur, TVGN_NEXT);
		}

		return NULL;
	}

	void SortChildren(HTREEITEM hItem)
	{
		TV_SORTCB      tvscb;

		tvscb.hParent     = hItem;
		tvscb.lParam      = (LPARAM)this;
		tvscb.lpfnCompare = TreeViewCompareProc;

		_tree.SortChildrenCB(&tvscb);
	}


	void RegisterDropTarget()
	{
		::RegisterDragDrop(_tree, (LPDROPTARGET)this);
	}

	void RevokeDropTarget()
	{
		::RevokeDragDrop(_tree);
	}

	STDMETHODIMP Drop(IDataObject  *pDataObj, DWORD grfKeyState, POINTL pt, DWORD  *pdwEffect) 
	{
		HRESULT hr = _pDropTarget->Drop(pDataObj, grfKeyState, pt, pdwEffect);
		DragLeave();
		return hr;
	}


	STDMETHODIMP DragEnter(IDataObject  *pDataObj, DWORD grfKeyState, POINTL pt, DWORD  *pdwEffect) 
	{
		_hDragOverItem = TVI_NULL;
		_pDataObject = pDataObj;

		return DragOver(grfKeyState, pt, pdwEffect);
	}

	STDMETHODIMP DragLeave() 
	{
		if (_pDropTarget != NULL)
		{
			_pDropTarget->DragLeave();
			_pDropTarget.Release();
		}

		if (_pDataObject != NULL)
		{
			_pDataObject.Release();
		}	

		if (_hDragOverItem != TVI_NULL)
		{
			_hDragOverItem = TVI_NULL;
			_tree.SelectDropTarget(NULL);
		}

		return S_OK;
	}

	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD  *pdwEffect) 
	{
		HRESULT hr;

		CPoint pointScreen(pt.x, pt.y);
		::ScreenToClient(m_hWnd, &pointScreen);

		UINT				flags;
		HTREEITEM hitem = _tree.HitTest(pointScreen, &flags);

		if (_hDragOverItem != hitem)
		{
			_hDragOverItem = hitem;
			_tree.SelectDropTarget(hitem);

			if (_pDropTarget != NULL)
			{
				_pDropTarget->DragLeave();
				_pDropTarget.Release();
			}

			if (_hDragOverItem != TVI_NULL)
			{
				IW::CShellItem*	pItemChild = (IW::CShellItem*)_tree.GetItemData( hitem );
				IW::CShellFolder spFolder;
				hr = spFolder.Open(*pItemChild, true);

				if (SUCCEEDED(hr))
				{
					hr = spFolder->CreateViewObject(
						_tree,
						IID_IDropTarget, 
						(LPVOID *)&_pDropTarget);
				}
			}

			if (_pDropTarget != NULL)
			{
				hr = _pDropTarget->DragEnter(
					_pDataObject, 
					grfKeyState, pt, pdwEffect);

				if (SUCCEEDED(hr))
				{
					return hr;
				}
			}
		}

		if (_pDropTarget != NULL)
		{
			return _pDropTarget->DragOver(grfKeyState, pt, pdwEffect);
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
		}

		return S_OK;
	}

	ULONG STDMETHODCALLTYPE AddRef() { return S_OK; }
	ULONG STDMETHODCALLTYPE Release() { return S_OK; }
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) { *ppvObject = this; return S_OK; }
};

/*

class FoldersFrame : public FrameGroup
{
public:

	class FrameTree : public FrameWindow
	{
	public:

		typedef FrameTree ThisClass;
		State &_state;
		IW::RefPtr<FolderTreeView> _pTree;

		FrameTree(IFrameParent *pParent, State &state) : 
			FrameWindow(pParent),
			_state(state)
		{			
		}

		~FrameTree()
		{
		}

		void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn) 
		{
			CRect rc;
			::GetClientRect(_pParent->GetHWnd(), &rc);

			_rect = rectIn;
			_rect.bottom = rc.bottom - borderGap;
			_pParent->SetChildWindowPos(positions, _wnd, _rect);
		};

		void Activate(HWND hWndParent, bool bShow)
		{
			if (bShow && _pTree == 0)
			{
				_pTree = IW::CreateComObject<FolderTreeView>();
				_pTree->SetState(&_state);
				_pTree->Create(hWndParent, CWindow::rcDefault, _T("Folders"));
				_pTree->SetCurFolder(_state.Folder.GetFolderItem());
				_wnd = _pTree->m_hWnd;
			}

			if (_wnd.m_hWnd)
			{
				_wnd.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			}
		};
	};

	FrameTree _tree;

	FoldersFrame(IFrameParent *pParent, State &state) : 
		FrameGroup(pParent, _T("Folders")),
		_tree(pParent, state)
	{
		_linkBar.AddLink(_T("Close"), ID_VIEW_FOLDERS);
		AddFrame(&_tree);
	}	

	~FoldersFrame()
	{
		Clear();
	}
};

*/