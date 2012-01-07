#pragma once

#include "Layout.h"
#include "FolderWindowImpl.h"

class ItemsFrame : public Frame
{
public:
	typedef ItemsFrame ThisClass;

	State &_state;
	CSize _size;
	FolderLayoutMatrix<ThisClass> _layout;

	int _nThumbsX;
	int _nThumbsY;
	int _nDragOverItem;

	ItemsFrame(IFrameParent *pParent, State &state) : Frame(pParent), 
		_state(state),
		_layout(this),
		_nDragOverItem(-1),
		_nThumbsX(1),
		_nThumbsY(1)
	{
		_layout.Init();
	}

	int GetHoverItem() const
	{
		return -1;
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn) 
	{
		CRect rc;
		::GetClientRect(_pParent->GetHWnd(), &rc);

		_rect = rectIn;
		_rect.bottom = _rect.top + (rc.Height() - _rect.top - borderGap) / 2;

		_layout.DoSize();

		const CSize sizeThumb = _layout.GetThumbSize();
		const int nThumbCount = GetItemCount();
		const int cy = (nThumbCount == 0) ? 0 : (((nThumbCount - 1) / _nThumbsX)* sizeThumb.cy) + sizeThumb.cy;

		_rect.bottom = _rect.top + cy;
	};

	void Render(IW::CRender &render)
	{
		int count = GetItemCount();

		for(int i = 0; i < count; i++)
		{
			_layout.DrawThumb(render, i, _rect);
		}
	}

	

	HWND GetHWnd() { return _pParent->GetHWnd(); };
	void UpdateBars() {};
	CSize GetClientSize() { return _rect.Size(); }

	bool InRename() const { return false; }
	void SetScrollOffset(int x, int y) {}
	void SetScrollOffset(const CPoint &point) {}

	void SetScrollSize(int x, int y, BOOL bRedraw = TRUE, bool bResetOffset = true) { SetScrollSize(CSize(x, y), bRedraw , bResetOffset); }
	void SetScrollSize(const CSize &size, BOOL bRedraw = TRUE, bool bResetOffset = true) { _size = size; }

	CPoint GetScrollOffset() { return CPoint(0,0); }
	CPoint GetScreenOrigin() { return _rect.TopLeft(); }

	int GetItemCount() const
	{
		return _state.PlayList.GetItemCount();
	}

	const IW::FolderItem *GetItem(int nItem) 
	{
		return _state.PlayList.GetItem(nItem);
	}

	void InvalidateThumb(int nThumb) {};

	int GetFocusItem() const { return -1; }
	bool IsItemSelected(int nItem) const { return false; }

	void GetThumbText(CString &strOut, int nItem, bool bFormat)
	{
		const IW::FolderItem *pItem = _state.PlayList.GetItem(nItem);
		pItem->GetFormatText(strOut, App.Options.m_annotations, bFormat);
	}
};


class PlayListViewFrame :  public FrameGroup
{
public:

	typedef PlayListViewFrame ThisClass;

	FrameLinkBar _linkBar;
	ItemsFrame _itemList;
	FrameCommand _back;
	State &_state;

	PlayListViewFrame (IFrameParent *pParent, State &state) : 
		FrameGroup(pParent, _T("PlayList")), 		
		_linkBar(pParent),
		_itemList(pParent, state),
		_back(pParent, ImageIndex::Previous, _T("Back"), ID_VIEW_NORMAL),
		_state(state)
	{
		PopulateLinkBar();

		//AddFrame(&_back);
		AddFrame(&_linkBar);
		AddFrame(&_itemList);

		_state.Folder.SelectionDelegates.Bind(this, &ThisClass::OnSelectionChanged);
	}

	~PlayListViewFrame ()
	{
		FrameGroup::Clear();
	}


	void PopulateLinkBar()
	{
		_linkBar.RemoveAll();

		IW::FolderPtr pFolder = _state.Folder.GetFolder();

		if (pFolder->HasSelection()) _linkBar.AddLink(_T("Add Selected"),  ID_PLAYLIST_ADD);
		if (!_state.PlayList.IsEmpty()) _linkBar.AddLink(_T("Clear"), ID_PLAYLIST_CLEAR);
		if (!_state.PlayList.IsEmpty()) _linkBar.AddLink(_T("Play"), ID_PLAYLIST_PLAY);
	}

	void Add()
	{
		_state.Folder.GetFolder()->IterateSelectedThumbs(_state.PlayList);
		PopulateLinkBar();
		_pParent->ResetLayout();
	}

	void Clear()
	{
		_state.PlayList.Clear();
		PopulateLinkBar();
		_pParent->ResetLayout();
	}

	void Play()
	{
		_state.Image.Play(&_state.PlayList);
	}

	void OnSelectionChanged()
	{		
		PopulateLinkBar();
		_pParent->InvalidateRect(_rect);
	}
};