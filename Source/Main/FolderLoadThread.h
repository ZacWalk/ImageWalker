#pragma once

#include "Threads.h"

class WaitForFolderToChangeThread : public IW::Thread
{
private:
	typedef WaitForFolderToChangeThread ThisClass;
	CEvent _eventFolderChange;	

public:

	Coupling *_pCoupling;
	State &_state;

	WaitForFolderToChangeThread(Coupling *pCoupling, State &state) : 
		_pCoupling(pCoupling),
		_state(state),
		_eventFolderChange(FALSE, FALSE)
	{
	}	

	void ResetThread()
	{
		_eventFolderChange.Set();
	}

	void Process()
	{
		App.Log(_T("Wait For Directory To Change thread started"));		

		HANDLE objects[3];

		objects[0] = IW::Thread::_eventExit;
		objects[1] = _eventFolderChange;
		objects[2] = INVALID_HANDLE_VALUE;

		while(!_bExit)
		{
			DWORD dw;
			if (objects[2] == INVALID_HANDLE_VALUE)
			{
				dw = WaitForMultipleObjects(2, objects, FALSE, INFINITE);
			}
			else
			{
				dw = WaitForMultipleObjects(3, objects, FALSE, INFINITE);
			}

			switch(dw)
			{
			case WAIT_OBJECT_0 + 1:
				{
					if(objects[2] != INVALID_HANDLE_VALUE)
						::FindCloseChangeNotification (objects[2]);

					objects[2] = INVALID_HANDLE_VALUE;

					IW::FolderPtr pFolder = _state.Folder.GetFolder();

					CString strPath = pFolder->GetFolderPath();

					if (!strPath.IsEmpty())
					{
						objects[2] = ::FindFirstChangeNotification (strPath,
							FALSE, FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
					}

				}
				break;

			case WAIT_OBJECT_0 + 2:

				_state.Folder.RefreshInOneSecond();
				::FindNextChangeNotification (objects[2]);

				break;

			case WAIT_FAILED:
			case WAIT_OBJECT_0:
				_bExit = true;
				break;
			}
		}

		if(objects[2] != INVALID_HANDLE_VALUE)
		{
			::FindCloseChangeNotification (objects[2]);
		}
	}	

};



class DecodeThumbsThread : public IW::Thread
{
private:
	typedef DecodeThumbsThread ThisClass;

	CEvent _eventFolderChange;
	Search::Spec _searchSpec;
	Search::Type _searchType;

	volatile bool _bPause;	

	int _timeLastStatus;
	volatile bool _bAbort;		

public:

	Coupling *_pCoupling;
	State &_state;

	enum { statusUpdateTicks = 500 };

	DecodeThumbsThread(Coupling *pCoupling, State &state) : 
		_pCoupling(pCoupling),
		_state(state),
		_eventFolderChange(FALSE, FALSE),
		_bAbort(false),
		_timeLastStatus(0),
		_bPause(false)
	{
	}	

	bool IsPaused() const
	{
		return _bPause;
	}

	void SetPause(bool bPause)
	{
		_bPause = bPause;
	}	

	void ResetThread()
	{
		_eventFolderChange.Set();
	}

	void Abort()
	{
		_bAbort = true;
		_state.Folder.IsSearchMode = false;
	}	

	void StartSearch(Search::Type type, const Search::Spec &ss)
	{			
		_state.Folder.IsSearchMode = true;
		_state.Folder.IsSearching = true;
		_searchType = type;
		_searchSpec = ss;
		_eventFolderChange.Set();
	}

	void Process()
	{		
		App.Log(_T("Decode Thumbs thread started"));	

		CLoadAny loader(_state.Plugins);
		int nFolderNumber = -1;	

		DWORD dw;
		HANDLE objectsRefresh[2];

		objectsRefresh[0] = _eventFolderChange;
		objectsRefresh[1] = IW::Thread::_eventExit;

		while(!_bExit)
		{
			IW::FolderPtr pFolder = _state.Folder.GetFolder();
			_bAbort = false;

			// What shall we load?
			if (_state.Folder.IsSearchMode)
			{
				IW::ScopeLockedBool isSearching(_state.Folder.IsSearching);
				_pCoupling->SignalSearching();					

				if (_searchType == Search::Current)
				{
					DecodeFolder(pFolder, &loader, pFolder->GetFolderItem(), pFolder->GetShellFolder());
				}
				else if (_searchType == Search::MyPictures)
				{
					try
					{
						IW::CShellItem item;
						if (item.Open(IW::GetMainWindow(), CSIDL_MYPICTURES))
						{
							IW::CShellFolder pMyPicsFolder;
							if (SUCCEEDED(pMyPicsFolder.Open(item)))
							{
								DecodeFolder(pFolder, &loader, item, pMyPicsFolder);
							}
						}
					}
					catch (std::exception &)
					{
					}
				}

				_pCoupling->SignalSearchingComplete();
			}
			else
			{
				IW::ScopeLockedBool isThumbnailing(_state.Folder.IsThumbnailing);
				_pCoupling->SignalThumbnailing();
				pFolder->LoadCacheFile(App.Options._sizeThumbImage, _bAbort);

				while(!_bAbort && !_bExit)
				{
					if (IsPaused())
					{
						Sleep(100);
					}
					else
					{						
						IW::FolderItemPtr pItem = pFolder->GetNextThumbToLoad(pFolder);

						if (pItem == 0)
							break;

						// Really load
						IW::FolderItemLoader job(_state.Cache);
						if (pItem->LoadJobBegin(job))
						{
							job.LoadImage(&loader, Search::Any);
							job.RenderAndScale();						
						}

						pFolder->LoadJobEnd(job, pItem);

						int t = GetTickCount();

						// May fire status update if it is time
						// 2 times a second?
						if (_timeLastStatus < t)
						{
							_timeLastStatus = t + statusUpdateTicks;
							_pCoupling->SignalThumbnailing();
						}
					}
				}

				_pCoupling->SignalThumbnailingComplete();				
			}

			_state.Cache.SyncCache();

			// No more images?
			dw = WaitForMultipleObjects(2, objectsRefresh, FALSE, INFINITE);

			if (dw != WAIT_OBJECT_0)
			{
				return;
			}
		}
	}	

	bool DecodeFolder(IW::Folder *pFolder, CLoadAny *pLoader, const IW::CShellItem &itemFolder, IW::CShellFolder &pShellFolder, LPCITEMIDLIST pItemGap = 0)
	{
		CString strPath = IW::CShellDesktop().GetDisplayNameOf(itemFolder, SHGDN_FORPARSING);
		_pCoupling->SignalSearchingFolder(strPath);

		// Get the IEnumIDList object for the given folder.
		IW::CShellItemEnum enumItems;

		UINT dwFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
		if (App.Options.m_bShowHidden) dwFlags |= SHCONTF_INCLUDEHIDDEN;

		HRESULT hr = enumItems.Create(IW::GetMainWindow(), pShellFolder, dwFlags);

		if (SUCCEEDED(hr))
		{
			// Build up item map
			// Enumerate throught the list of items.
			LPITEMIDLIST pItem = NULL;
			ULONG ulFetched = 0;

			while (enumItems->Next(1, &pItem, &ulFetched) == S_OK && _state.Folder.IsSearchMode && !_bExit && !_bAbort)
			{
				IW::FolderItemPtr pThumb = new IW::RefObj<IW::FolderItem>;

				if (!pThumb->Init(pShellFolder, pItem, pItemGap))
					return false;

				if (_searchSpec.DoMatch(pThumb, _state.Cache, pLoader))
				{
					pThumb->ModifyFlags(0, THUMB_IS_SEARCH_RESULT | THUMB_INVALIDATE);
					pFolder->InsertThumb(pThumb);

					int t = GetTickCount();

					// May fire status update if it is time
					// 2 times a second?
					if (_timeLastStatus < t)
					{
						_timeLastStatus = t + statusUpdateTicks;
						_pCoupling->SignalSearching();
					}
				}			
			}
		}


		// Get the IEnumIDList object for the given folder.
		IW::CShellItemEnum enumFolders;

		dwFlags = SHCONTF_FOLDERS;
		if (App.Options.m_bShowHidden) dwFlags |= SHCONTF_INCLUDEHIDDEN;

		hr = enumFolders.Create(IW::GetMainWindow(), pShellFolder, dwFlags);

		if (SUCCEEDED(hr))
		{
			// Build up item map
			// Enumerate throught the list of items.
			LPITEMIDLIST pItem = NULL;
			ULONG ulFetched = 0;

			while (enumFolders->Next(1, &pItem, &ulFetched) == S_OK && _state.Folder.IsSearchMode && !_bExit && !_bAbort)
			{
				IW::CShellItem item;
				item.Attach(pItem);

				IW::CShellFolder pSubFolder;
				hr = pShellFolder->BindToObject(pItem, NULL, IID_IShellFolder, (LPVOID*)pSubFolder.GetPtr());

				if (SUCCEEDED(hr))
				{
					IW::CShellItem itemSubFolder;
					itemSubFolder.Cat(itemFolder, pItem);

					
					if (pItemGap)
					{
						IW::CShellItem itemGap;
						itemGap.Cat(pItemGap, item);
						DecodeFolder(pFolder, pLoader, itemSubFolder, pSubFolder, itemGap);
					}
					else
					{
						DecodeFolder(pFolder, pLoader, itemSubFolder, pSubFolder, item);
					}
				}
			}
		}

		return true;	
	}

};


