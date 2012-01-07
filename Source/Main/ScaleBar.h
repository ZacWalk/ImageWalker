#pragma once

template<class T>
class CScaleBar
{

protected:
	CComboBoxEx m_wndScale;

public:

	CScaleBar()
	{
	}

	BEGIN_MSG_MAP(CMainFrame)

		COMMAND_HANDLER(IDC_SCALEBAR, CBN_EDITCHANGE, OnChangeScale)
		COMMAND_HANDLER(IDC_SCALEBAR, CBN_SELCHANGE, OnChangeScaleSelect)

	END_MSG_MAP()

	CComboBoxEx &CreateScaleBar()
	{
		T *pT = static_cast<T*>(this);

		// create a combo box for the Scale bar
		m_wndScale.Create(pT->m_hWnd, CRect(0,0,60,400), NULL, 
			CBS_DROPDOWN | WS_CHILD | WS_VISIBLE  | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
			0, IDC_SCALEBAR);

		m_wndScale.SetFont(IW::Style::GetFont(IW::Style::Font::Standard));

		// Update the delay
		COMBOBOXEXITEM     cbi;	
		cbi.mask = CBEIF_TEXT;
		cbi.iItem = -1;
		cbi.cchTextMax = -1;

		// Scale
		cbi.pszText = (LPTSTR)App.LoadString(IDS_FIT); m_wndScale.InsertItem(&cbi);
		cbi.pszText = (LPTSTR)App.LoadString(IDS_FILL); m_wndScale.InsertItem(&cbi);
		cbi.pszText = (LPTSTR)App.LoadString(IDS_UP); m_wndScale.InsertItem(&cbi);
		cbi.pszText = (LPTSTR)App.LoadString(IDS_DOWN); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T(" 25%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T(" 50%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T(" 75%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T("100%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T("150%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T("200%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T("300%"); m_wndScale.InsertItem(&cbi);
		cbi.pszText = _T("400%"); m_wndScale.InsertItem(&cbi);	

		return m_wndScale;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		T *pT = static_cast<T*>(this);

		bool bIsEdit = false;
		HWND hWndFocus = ::GetFocus();

		if (pMsg->message == WM_KEYDOWN)
		{
			hWndFocus = ::GetFocus();

			// Special handeling for combos
			if (pMsg->wParam == VK_RETURN || 
				pMsg->wParam == VK_ESCAPE)
			{
				// Scale Combo
				HWND hScaleEdit = (HWND)::SendMessage(m_wndScale, CBEM_GETEDITCONTROL, 0, 0L);

				if (hWndFocus == hScaleEdit)
				{
					if (pMsg->wParam == VK_RETURN)
					{
						CString str;
						m_wndScale.GetWindowText(str);
						pT->SetScale(str);
						return TRUE;
					}
					else if (pMsg->wParam == VK_ESCAPE)
					{
						pT->UpdateScaleEditBox();
						return TRUE;
					}

					return FALSE;
				}

			}
		}

		return FALSE;
	}

	void SetScaleBarText(LPCTSTR szNew)
	{
		if (szNew)
		{
			CString strCurrent;
			m_wndScale.GetWindowText(strCurrent);

			if (strCurrent != szNew)
			{
				m_wndScale.SetWindowText(szNew);
			}
		}
	}

	LRESULT OnChangeScale(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);

		CString str;
		m_wndScale.GetWindowText(str);
		pT->SetScale(str);		

		return 0;
	}


	LRESULT OnChangeScaleSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);

		int nIndex = m_wndScale.GetCurSel();
		if(nIndex != -1)
		{
			TCHAR sz[100];
			m_wndScale.GetLBText(nIndex, sz);
			pT->SetScale(sz);
		}

		return 0;
	}

};