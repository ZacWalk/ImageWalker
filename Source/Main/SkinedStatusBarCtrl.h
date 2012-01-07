#pragma once

class CSkinedStatusBarCtrl :
	public CWindowImpl<CSkinedStatusBarCtrl, CStatusBarCtrl>
{
public:
	HWND _hWndOwner;
	State &_state;
	CString _strFolderStatus;
	int _nSortOrder;

	CSkinedStatusBarCtrl(State &state) : 
		_hWndOwner(0),		 
		_nSortOrder(0),
		_state(state)
	{
	}

	void SetOwner(HWND hWndOwner)
	{
		_hWndOwner = hWndOwner;
	}

	BEGIN_MSG_MAP(CSkinedStatusBarCtrl)
		MESSAGE_HANDLER(WM_COMMAND, OnForward)
		MESSAGE_HANDLER(WM_NOTIFY, OnForward)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	END_MSG_MAP()

	LRESULT OnForward(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return ::SendMessage(_hWndOwner, uMsg, wParam, lParam);
	}


	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CDCHandle dc((HDC)wParam);
		CRect r; GetClientRect(r);
		//dc.FillSolidRect(r, IW::Style::Color::Window);
		DWORD c1 = IW::Style::Color::Window;			
		DWORD c2 = IW::Emphasize(IW::Style::Color::Window, 64);	
		IW::Skin::DrawGradient(dc, r, c1, c2);
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		CRect r; GetClientRect(r);
		CString str;

		if (IsSimple())
		{
			r.left += 4;
			GetWindowText(str);
			HFONT hOldFont = dc.SelectFont((HFONT)GetStockObject (DEFAULT_GUI_FONT));
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(IW::Style::Color::WindowText);
			dc.DrawText(str, -1, r, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
			dc.SelectFont(hOldFont);	
		}
		else
		{
			int nCount = (int) GetParts(0, NULL);
			for( int i = 0; i < nCount; i++ ) 
			{
				GetRect(i, r);
				GetText(i, str);

				if (i == 0)
				{
					r.left += 4;
					HFONT hOldFont = dc.SelectFont((HFONT)GetStockObject (DEFAULT_GUI_FONT));
					dc.SetBkMode(TRANSPARENT);
					dc.SetTextColor(IW::Style::Color::WindowText);
					dc.DrawText(str, -1, r, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
					dc.SelectFont(hOldFont);			
				}
				else if (i == 2)
				{
					if (_state.Folder.IsThumbnailing && !_state.Folder.IsSearching)
					{
						IW::FolderPtr pFolder = _state.Folder.GetFolder();
						int nPercentComplete = pFolder->GetPercentComplete();
						int nWidth = r.Width();
						int nSplit = MulDiv(nWidth, nPercentComplete, 100);

						CRect rBar(r);
						rBar.right = rBar.left + nSplit;
						dc.FillSolidRect(rBar, IW::Style::Color::Highlight);
						dc.FrameRect(rBar, IW::Style::Brush::EmphasizedHighlight);
					}

					HFONT hOldFont = dc.SelectFont((HFONT)GetStockObject (DEFAULT_GUI_FONT));
					dc.SetBkMode(TRANSPARENT);
					dc.SetTextColor(IW::Style::Color::WindowText);
					dc.DrawText(_strFolderStatus, -1, r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
					dc.SelectFont(hOldFont);
				}
			}	
		}

		return 0;
	}

	void OnSortOrderChanged(int order)
	{
		_nSortOrder = order;
	}	

	void UpdateMessage()
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();

		if (_state.Folder.IsSearching)
		{			
			_strFolderStatus.Format(IDS_STATUS_FOUND, pFolder->GetImageCount(), pFolder->GetSize());
		}
		else if (pFolder->GetSelectedItemCount() > 1)
		{
			int nCount = 0, nImages = 0;
			IW::FileSize size;
			pFolder->GetSelectStatus(nCount, nImages, size);

			_strFolderStatus.Format(IDS_IMAGES_FILES_FOLDERS, nImages, nCount, size.ToString());
		}
		else
		{
			long nFilesOrFolders = pFolder->GetSize();
			long nImageCount = pFolder->GetImageCount();
			long nPercentComplete = pFolder->GetPercentComplete();
			long nSecondsRemaining = pFolder->GetTimeRemaining();
			long nSecondsTaken = pFolder->GetTimeTaken();		

			LPCTSTR szOrder = App.GetMetaDataTitle(_nSortOrder);
			int nSecondsMin = 50;

#ifdef _DEBUG

			nSecondsMin = 0;

#endif

			if (_state.Folder.IsThumbnailing && nSecondsRemaining > 0)
			{
				_strFolderStatus.Format(IDS_STATUS_REMAINING, nImageCount, nFilesOrFolders, nSecondsRemaining / 10);
			}
			else if (nSecondsTaken >= nSecondsMin)
			{
				_strFolderStatus.Format(IDS_STATUS_SECONDS, nImageCount, nFilesOrFolders, szOrder, nSecondsTaken / 10);
			}
			else
			{
				_strFolderStatus.Format(IDS_STATUS_NORMAL, nImageCount, nFilesOrFolders, szOrder);
			}
		}
	}
};