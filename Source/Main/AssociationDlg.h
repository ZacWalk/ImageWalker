// CAssociationDlg.h : Declaration of the CAssociationDlg

#pragma once


/////////////////////////////////////////////////////////////////////////////
// CAssociationDlg
class CAssociationDlg : 
	public CAxDialogImpl<CAssociationDlg>
{
private:

	State &_state;

public:
	CAssociationDlg(State &state) : _state(state)
	{
	}

	~CAssociationDlg()
	{
	}

	/////////////////////////////////////////////////////////////////////////////
// CAssociationDlg

	class CAssociationDlgInserter
	{
	private:
		HWND m_hWndReport;
		State &_state;

	public:
		CAssociationDlgInserter(State &state, HWND hWndReport) : _state(state), m_hWndReport(hWndReport)
		{
			_state.Plugins.IterateImageExtensions(this);
		}

		bool AddExtension(IW::IImageLoaderFactoryPtr pFactory, LPCTSTR szExtension)
		{
			// Screen if not want thumbnail
			if (IW::ImageLoaderFlags::THUMBONLY & pFactory->GetFlags())
			{
				return true;
			}

			int nState = 0;

			HKEY hOpen;
			DWORD cbData;
			DWORD dwType;
			DWORD rc;
			BOOL bSuccess = TRUE;

			CString strKey;
			strKey.Format(_T(".%s"), szExtension);
			
			if ( (rc = RegOpenKeyEx(HKEY_CLASSES_ROOT, strKey, 0, KEY_READ, &hOpen)) == ERROR_SUCCESS)
			{
				// query to get data size
				if ( (rc = RegQueryValueEx(hOpen,NULL,NULL,&dwType, NULL, &cbData )) == ERROR_SUCCESS )
				{
					LPBYTE pByte = (LPBYTE)LocalAlloc(LPTR, cbData);
					
					rc = RegQueryValueEx(hOpen,NULL,NULL,&dwType, pByte, &cbData );

					if (_tcsicmp(g_szClassName, (LPCTSTR)pByte) == 0)
					{
						nState = 1;
					}

					LocalFree(pByte);
				}
				
				RegCloseKey(hOpen);
			}

			// Add items
			LVITEM lvItem;
			IW::MemZero(&lvItem,sizeof(LVITEM));

			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = ListView_GetItemCount (m_hWndReport);
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPTSTR)szExtension;
			
			int dIndex = ListView_InsertItem(m_hWndReport,&lvItem);
			
			// Add subitems
			lvItem.mask = TVIF_TEXT;
			lvItem.iItem = dIndex;
			lvItem.iSubItem = 1;
			lvItem.pszText = (LPTSTR)(LPCTSTR)pFactory->GetTitle();
			
			ListView_SetItem(m_hWndReport,&lvItem);
			ListView_SetItemState (m_hWndReport, dIndex, (UINT(nState + 1) << 12), LVIS_STATEIMAGEMASK);

			return true;
		}
	};

	enum { IDD = IDD_ASSOCIATION };

BEGIN_MSG_MAP(CAssociationDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();
		
		HWND hwndReport = GetDlgItem(IDC_LIST1);
		ListView_SetExtendedListViewStyle(hwndReport, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		
		int index;                      
		LV_COLUMN lvColumn;        
		LPCTSTR szString[] = {
			App.LoadString(IDS_EXTENSION), 
			App.LoadString(IDS_FILETYPE),
			0};
		
		// Empty the list in list view.
		ListView_DeleteAllItems (hwndReport);
		
		// Initialize the columns in the list view.
		lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.cx = 120;
		
		// Insert the five columns in the list view.
		for (index = 0; szString[index] != 0; index++)
		{
			lvColumn.pszText = (LPTSTR)szString[index];
			ListView_InsertColumn (hwndReport, index, &lvColumn);
			lvColumn.cx = 500;
		}
		
		// Insert the extensions
		CAssociationDlgInserter inserter(_state, hwndReport);
		
		return 1;  // Let the system set the focus
	}

	LRESULT CAssociationDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		HWND hwndReport = GetDlgItem(IDC_LIST1);
		int nCount = ListView_GetItemCount (hwndReport);

		for(int i = 0; i < nCount; i++)
		{
			UINT nState = ListView_GetItemState (hwndReport, i, LVIS_STATEIMAGEMASK);
			bool bChecked = (((nState & LVIS_STATEIMAGEMASK)>>12)-1) != 0;  
			
			if (bChecked)
			{
				TCHAR szText[100];

				LV_ITEM lvi;
				IW::MemZero(&lvi,sizeof(LVITEM));

				lvi.mask = LVIF_TEXT;
				lvi.iItem = i;
				lvi.iSubItem = 0;
				lvi.pszText = szText;
				lvi.cchTextMax = 100;
				
				if ( ListView_GetItem( hwndReport, &lvi ))
				{
					CString strKey;
					strKey.Format(_T(".%s"), lvi.pszText);
					
					HKEY hKey;
					DWORD err, disposition;

					// Open the registry key (or create it if the first time being used)
					err = RegCreateKeyEx( HKEY_CLASSES_ROOT, strKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, &disposition );
					
					if ( ERROR_SUCCESS != err )
					{
						err = RegOpenKeyEx(HKEY_CLASSES_ROOT, strKey, 0, KEY_WRITE, &hKey);
					}
					
					
					if ( ERROR_SUCCESS == err )
					{
						RegSetValueEx ( hKey, NULL, 0, REG_SZ, ( LPBYTE) g_szClassName, (_tcsclen (g_szClassName) + 1) * sizeof(TCHAR));
						RegCloseKey( hKey );
					}
				}
			}
		}

		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};