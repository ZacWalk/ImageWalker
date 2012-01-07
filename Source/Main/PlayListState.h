#pragma once

class PlayListState : public Coupling
{
public:

	ITEMLIST _listItems;

	void AddItem(IW::FolderItem *pThumb)
	{
		_listItems.push_back(pThumb);
	}

	const IW::FolderItem *GetItem(int nItem) const
	{
		return _listItems[nItem];
	}

	int GetItemCount() const
	{
		return _listItems.size();
	}

	bool IsEmpty() const
	{
		return _listItems.empty();
	}

	void Clear()
	{
		_listItems.clear();
	}

	CString ToString() const
	{
		CString str;

		for(ITEMLIST::const_iterator it = _listItems.begin(); it != _listItems.end(); ++it)
		{
			str += (*it)->GetFileName();
			str += g_szCRLF;
		}

		return str;
	}

	ITEMLIST GetItemList() const
	{
		return _listItems;
	}

	int GetFocusItem() const
	{
		return -1;
	}

	void SetFocusItem(int nFocusItem, bool bSignalEvent)
	{
	}

	bool IsItemFolder(long nItem) const
	{
		return _listItems[nItem]->IsFolder();
	}

	CString GetItemPath(int nItem) const
	{
		return _listItems[nItem]->GetFilePath();
	}

	template<class THandeler>
	bool IterateItems(THandeler *pItemHandeler, IW::IStatus *pStatus)
	{
		CString strFolderMessage;
		strFolderMessage.Format(IDS_PROCESSING_FOLDER_FMT, _T("Play list"));
		pStatus->SetHighLevelStatusMessage(strFolderMessage);

		HRESULT hr = S_OK;
		int nCount = 0;

		for(ITEMLIST::const_iterator it = _listItems.begin(); it != _listItems.end(); ++it)      
		{
			IW::FolderItemPtr pThumb = *it;

			if (pThumb->IsFolder())
			{
				IW::FolderPtr pFolder = pThumb->GetFolder();

				if (!pItemHandeler->StartFolder(pFolder, pStatus))
				{
					return false;
				}

				pItemHandeler->EndFolder();
			}
			else
			{
				pStatus->SetContext(pThumb->GetFileName());

				pItemHandeler->StartItem(pThumb, pStatus);
				pItemHandeler->EndItem();
			}

			pStatus->SetHighLevelProgress(nCount, _listItems.size());

			if (pStatus->QueryCancel())
			{
				pStatus->SetError(App.LoadString(IDS_CANCELED));
				return false;
			}

			nCount++;
		}


		return true;
	}

};
