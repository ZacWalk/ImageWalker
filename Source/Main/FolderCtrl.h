#pragma once

#include "FolderWindowImpl.h"
#include "View.h"
#include "ThumbnailCache.h"
#include "FolderFrames.h"

namespace IW
{
	class IW::Folder;
}

class CFolderCtrl : 
	public CWindowImpl<CFolderCtrl>,
	public CView<CFolderCtrl>,
	public CScrollImpl<CFolderCtrl>,
	public CPaletteImpl<CFolderCtrl>,
	public FolderWindowImpl<CFolderCtrl>
{
public:

	typedef CFolderCtrl ThisClass;
	typedef CWindowImpl<ThisClass> WindowType;
	typedef ToolTipWindowImpl<ThisClass> ToolTipType;
	typedef FolderLayout<ThisClass> LayoutType;
	typedef CScrollImpl<ThisClass> ScrollType;

	Coupling *_pCoupling;
	State &_state;	
	CPoint _pointScreenOrigin;

	FolderLayoutNormal<ThisClass> _layoutNormal;
	FolderLayoutDetail<ThisClass> _layoutDetail;
	FolderLayoutMatrix<ThisClass> _layoutMatrix;

	LayoutType *_pLayout;	
	FolderDisplayMode _displayMode;

	
	CFolderCtrl(Coupling *pCoupling, State &state) : 
		FolderWindowImpl<CFolderCtrl>(state),
		_state(state),
		_pCoupling(pCoupling),
		_displayMode(eViewMatrix),
		_pLayout(&_layoutMatrix),
		_layoutNormal(this),
		_layoutDetail(this),
		_layoutMatrix(this),
		_pointScreenOrigin(0,0)
	{	
		GetLayout()->Init();
		SetScrollExtendedStyle(0);
	}

	~CFolderCtrl()
	{
	}	

	LayoutType *GetLayout()
	{
		return _pLayout;
	}

	const LayoutType *GetLayout() const
	{
		return _pLayout;
	}

	Coupling *GetCoupling()
	{
		return _pCoupling;
	}

	const Coupling *GetCoupling() const
	{
		return _pCoupling;
	}

	void OnFolderChanged()
	{
		ScrollTop();
		SetScrollSizeList(true);
		UpdateBars();
		ResetCounters();
		UpdateScrollSize();
		Invalidate();
	}

	void OnFolderRefresh() 
	{
		SetScrollSizeList(false);

		IW::FolderPtr pFolder = GetFolder();
		int nNewFocus = pFolder->GetFocusItem();

		if (nNewFocus != -1)
		{
			MakeItemVisible(nNewFocus);
		}

		if (!_strNewCreation.IsEmpty())
		{	
			if (_state.Folder.Select(_strNewCreation))
			{
				_strNewCreation.Empty();
			}
		}

		/*
		if (bDoSelect)
		{
			if (nNewFocus != -1)
			{
				pFolder->SetFocusItem(nNewFocus);
			}
			else if (nOldFocus != -1)
			{
				pFolder->SetFocusItem(-1);
				Select(pFolder, IW::Min(nOldFocus, nSize - 1), 0, true);
			}
		}
		*/

		ResetCounters();
		Invalidate();
		UpdateBars();
	}

	void OnResetFrames()
	{
		ShowFrames();
		SizeClients();
		GetLayout()->DoSize();
		UpdateBars();
		Invalidate();
	}

	void ShowFrames()
	{
		Invalidate();
	}

	void OnActivate()
	{
	}


	void OnCommand(WORD id)
	{
		_pCoupling->Command(id);
	}	

	void SetViewMode(FolderDisplayMode eMode)
	{
		switch(eMode)
		{
		case eViewNormal:
		case eViewDetail:
		case eViewMatrix:
			break;
		default:
			eMode = eViewMatrix;
		}

		_displayMode = eMode;

		_pLayout = ModeEnumToObject(eMode);
		
		Fade fade(m_hWnd);
		m_ptOffset = CPoint(0,0);
		GetLayout()->Init();

		SetScrollSizeList(true);

		IW::FolderPtr pFolder = GetFolder();
		int nFocusItem = pFolder->GetFocusItem();

		if (nFocusItem != -1)
		{
			MakeItemVisible(nFocusItem);
		}

		ResetCounters();
		fade.FadeIn();
		Invalidate();
	}

	LayoutType *ModeEnumToObject(FolderDisplayMode eMode)
	{
		switch(eMode)
		{
		case eViewNormal:
			return &_layoutNormal;
		case eViewDetail:
			return &_layoutDetail;
		case eViewMatrix:
			return &_layoutMatrix;
		}

		return &_layoutNormal; // Default
	}

	void UpdateBars()
	{
		// Update the scroll bar
		SCROLLINFO si;
		IW::MemZero(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;

		si.nMax = m_sizeAll.cx - 1;
		si.nPage = m_sizeClient.cx;
		si.nPos = m_ptOffset.x;
		SetScrollInfo(SB_HORZ, &si, true);

		si.nMax = m_sizeAll.cy - 1;
		si.nPage = m_sizeClient.cy;
		si.nPos = m_ptOffset.y;
		SetScrollInfo(SB_VERT, &si, true);
	}

	bool AdjustScrollOffset(int& x, int& y)
	{
		const CSize sizeThumb = GetLayout()->_sizeThumb;

		int xOld = x;
		int yOld = y;

		int cxMax = m_sizeAll.cx - m_sizeClient.cx;
		int cyMax = m_sizeAll.cy - m_sizeClient.cy;
		int cxMin = ((_nThumbsX * sizeThumb.cx) - m_sizeClient.cx) / 2;
		int cyMin = 0;

		if(x > cxMax)
			x = (cxMax >= cxMin) ? cxMax : cxMin;
		else if(x < cxMin)
			x = cxMin;

		if(y > cyMax)
			y = (cyMax >= cyMin) ? cyMax : cyMin;
		else if(y < cyMin)
			y = cyMin;

		return (x != xOld || y != yOld);
	}

	void UpdateScrollSize()
	{
		IW::FolderPtr pFolder = GetFolder();
		const CSize sizeThumb = GetLayout()->_sizeThumb;

		int y = ((pFolder->GetSize() - 1) / _nThumbsX)*sizeThumb.cy + sizeThumb.cy;

		y = IW::LowerLimit<1>(y);

		if (y != m_sizeAll.cy)
		{
			CRect rectClient(m_ptOffset, m_sizeClient);
			CRect rectAdded(0, m_sizeAll.cy, m_sizeAll.cx, y);

			if (rectClient & rectAdded)
				Invalidate();

			m_sizeAll.cy = y;

			UpdateBars();
		}
	}

	void InvalidateThumb(int nThumb)
	{
		IW::FolderPtr pFolder = GetFolder();
		InvalidateThumb(pFolder, nThumb);
	}


	void InvalidateThumb(IW::Folder *pFolder, int nThumb)
	{
		if (IsThumbVisible(pFolder, nThumb))
		{			
			CRect rectThumb = GetLayout()->GetThumbRect(nThumb);
			rectThumb.OffsetRect(-m_ptOffset.x, -m_ptOffset.y);
			rectThumb.InflateRect(3,3);
			InvalidateRect(rectThumb);
		}
	}

	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE)
	{
		return WindowType::SetScrollPos(nBar, nPos, bRedraw);
	}

	CString GetToolTipText(int idCtrl)
	{
		if(App.Options.m_bShowToolTips &&  
			(_nHoverItem != -1) &&
			(m_hWnd == (HWND)idCtrl))
		{
			CString str;
			IW::FolderPtr pFolder = GetFolder();
			return pFolder->GetToolTip(_nHoverItem);
		}

		return ToolTipType::GetToolTipText(idCtrl);
	}

	CPoint GetScreenOrigin() const
	{
		return _pointScreenOrigin;
	}

	CPoint GetScrollOffset() const
	{
		return m_ptOffset;
	}

	HWND GetHWnd()
	{
		return m_hWnd;
	}

	const CSize GetClientSize() const 
	{ 
		return m_sizeClient; 
	}	

	const CSize GetSizeAll() const 
	{ 
		return m_sizeAll; 
	}

	BEGIN_MSG_MAP(CFolderCtrl)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		CHAIN_MSG_MAP(CView<ThisClass>)
		CHAIN_MSG_MAP(FolderWindowImpl<ThisClass>)
		CHAIN_MSG_MAP(ScrollType)
		CHAIN_MSG_MAP(CPaletteImpl<ThisClass>)

	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SetViewMode(_displayMode);
		ShowFrames();

		_state.Folder.ChangedDelegates.Bind(this, &ThisClass::OnFolderChanged);
		_state.Folder.RefreshDelegates.Bind(this, &ThisClass::OnFolderRefresh);		
		_state.Folder.SelectionDelegates.Bind(this, &ThisClass::OnSelectionChanged);
		_state.Folder.FocusDelegates.Bind(this, &ThisClass::OnFocusChanged);
		_state.ResetFrames.Bind(this, &ThisClass::OnResetFrames);

		bHandled = false;
		return 0;
	}	

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		ATLTRACE(_T("Destroy CFolderCtrl\n")); 		
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			m_sizeClient.cx = GET_X_LPARAM(lParam);
			m_sizeClient.cy = GET_Y_LPARAM(lParam);

			SizeClients();
			GetLayout()->DoSize();
			UpdateBars();

			CRect rect;
			GetClientRect(rect);
			m_sizeClient = rect.Size();
			GetLayout()->DoSize();
		}

		return 1;
	}

	void LayoutFrames(int cx, int cy)
	{
		SizeClients();
		GetLayout()->DoSize();
		UpdateBars();
		Invalidate();
	}

	void SizeClients()
	{	
	}	

	void Accept(FrameVisitor &visitor) 
	{ 
	}

	void OnPaint(CDCHandle dc)
	{
		IW::CRender render;

		if (render.Create(dc))
		{
			render.Fill(IW::Style::Color::Window);

			CRect rectClip;
			dc.GetClipBox(rectClip);
			DrawFolder(render, rectClip);			

			Accept(FrameVisitorErase(render));
			Accept(FrameVisitorRender(render));

			render.Flip();
		}
	}	

	void  PageUp()
	{
		Up();
	}

	void  PageDown()
	{
		Down();
	}	

	void  Cut()
	{
		//Invoke("cut", false);
		SetClipboard(true);	
	}

	void  Copy()
	{
		//Invoke("copy", false);  
		SetClipboard(false);
	}

	void  Paste()
	{
		if (!::IsClipboardFormatAvailable(CF_HDROP) &&
			::IsClipboardFormatAvailable(CF_DIB))
		{
			OpenClipboard();

			HGLOBAL hglb = ::GetClipboardData(CF_DIB); 

			if (hglb)
			{			  
				IW::Image dib;
				dib.Copy(hglb);

				SaveNewImage(dib);
			}

			::CloseClipboard();
		}
		else
		{
			Invoke("paste", TRUE);
		}
	}

	void ShowProperties()
	{
		Invoke("Properties", false);
	}

	void OnAfterDelete()
	{
		const IW::FolderPtr pFolder = GetFolder();
		const int nFocusItem = pFolder->GetFocusItem();
		const int nSize = pFolder->GetSize();

		// Next thumb after selection
		for(int i = nFocusItem + 1; i < nSize; i++)
		{
			if (!pFolder->IsItemSelected(i))
			{
				_state.Folder.Select(i, 0, true);
				return;
			}
		}

		_state.Folder.Select(-1, 0, true);
	}

	void LookForFile(LPCTSTR szName)
	{
		_strNewCreation = szName;
	}	

	IW::FolderPtr GetFolder()
	{
		return _state.Folder.GetFolder();
	}

	int GetItemCount() const
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		return pFolder->GetSize();
	}

	bool IsItemSelected(long nItem) const
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		return pFolder->IsItemSelected(nItem);
	}

	bool IsItemImage(long nItem) const
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		return pFolder->IsItemImage(nItem);
	}

	int GetSelectedItemCount() const
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		return pFolder->GetSelectedItemCount();
	}	

	bool IsItemFolder(long nItem) const
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		return pFolder->IsItemFolder(nItem);
	}

	int GetImageCount() const
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		return pFolder->GetImageCount();
	}

	void OnSelectionChanged()
	{
		Invalidate();
	}

	void OnFocusChanged() 
	{
		MakeFocusItemVisible();
	}

	void OnStartSearching()
	{
		Invalidate();
		ScrollTop();
		SetScrollSizeList(true);
		ResetCounters();	
	}

	void  MakeFocusItemVisible()
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		int nFocusItem = pFolder->GetFocusItem();

		if (nFocusItem != -1)
		{
			MakeItemVisible(nFocusItem);
		}
	}	

	CString GetDefaultName()
	{
		CString str;
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		int nCountSelected = pFolder->GetSelectedItemCount();

		if (1 == nCountSelected)
		{
			int nCount = pFolder->GetItemCount();

			for(int i = 0; i < nCount; i++)
			{
				if (pFolder->IsItemSelected(i))
				{
					str = pFolder->GetItemName(i);
					break;
				}
			}
		}
		else
		{
			str = pFolder->GetFolderName();
		}


		int nExtension = str.ReverseFind(_T('.'));

		if (nExtension != -1)
		{
			str = str.Left(nExtension);
		}

		return str;
	}

	void SetSortOrder(int nSortOrder)
	{
		Fade fade(m_hWnd);
		_state.Folder.SetSortOrder(nSortOrder);
		fade.FadeIn();
	}

	void SetSortOrder(int nSortOrder, bool bAssending)
	{
		Fade fade(m_hWnd);
		_state.Folder.SetSortOrder(nSortOrder, bAssending);
		fade.FadeIn();
	}

	void OnOptionsChanged()
	{
		ResetThumbnailSize();
		SetViewMode(_displayMode);
		Invalidate();
	}
};

