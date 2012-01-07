#pragma once

#include <ShlObj.h>
#include <vector>

template<class T>
class ATL_NO_VTABLE CShellExtensionMenuImpl :
	public IShellExtInit,
	public IContextMenu
{
protected:

	CShellExtensionMenuImpl()
	{
		HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	}

	~CShellExtensionMenuImpl()
	{
		ClearMenuEntries();
		CoUninitialize();
	}

public:

	void ClearMenuEntries()
	{
		for(UINT i = 0;  i < m_vecMenuEntries.size(); i++)
		{
			if (m_vecMenuEntries[i])
				delete m_vecMenuEntries[i];
		}

		m_vecMenuEntries.clear();
	}

	class CMenuEntry
	{
	public:
		CMenuEntry(int nOperation, LPCTSTR szName, LPCTSTR szVerb, LPCTSTR szHelp, LPCTSTR szFileName) 
		{
			_imageName = szName;
			_imageVerb = szVerb;
			_imageHelp = szHelp;
			_imageFileName = szFileName;
			m_nOperation = nOperation;
		};

		virtual ~CMenuEntry() {};

		CString _imageName;
		CString _imageVerb;
		CString _imageHelp;
		CString _imageFileName;
		int m_nOperation;


	public:
		virtual void Invoke(HWND hwndParent)
		{
			// {D7E8384A-752B-447d-8D30-14A1A79FB210}
			static const GUID CLSID_ImageWalker = 
			{ 0xd7e8384a, 0x752b, 0x447d, { 0x8d, 0x30, 0x14, 0xa1, 0xa7, 0x9f, 0xb2, 0x10 } };

			CComPtr <IDispatch> spDispatch;

			HRESULT hr = ::CoCreateInstance(CLSID_ImageWalker, NULL, 
				CLSCTX_SERVER, IID_IDispatch, (void**)&spDispatch);

			if (SUCCEEDED(hr))
			{
				CComVariant varResult;
				CComVariant* pvars = new CComVariant[1];

				if (pvars == NULL)
					return;


				if (spDispatch != NULL)
				{
					VariantClear(&varResult);
					pvars[0] = T2BSTR(_imageFileName);
					DISPPARAMS disp = { pvars, NULL, 1, 0 };

					spDispatch->Invoke(m_nOperation, IID_NULL, LOCALE_USER_DEFAULT, 
						DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
				}

				delete[] pvars;
				//return varResult.scode;
			}
			else
			{
				MessageBeep(MB_OK);
			}
		}

	};

	std::vector<CMenuEntry*> m_vecMenuEntries;
	std::vector<CString> m_vecFileNames;


	STDMETHODIMP QueryContextMenu(HMENU hMenu,
		UINT indexMenu,
		UINT idCmdFirst,
		UINT idCmdLast,
		UINT uFlags)
	{
		T* pT = static_cast<T*>(this);


		UINT idCmd = idCmdFirst;
		BOOL bAppendItems=TRUE;

		if ((uFlags & 0x000F) == CMF_NORMAL)  //Check == here, since CMF_NORMAL=0
		{
		}
		else if (uFlags & CMF_VERBSONLY)
		{
		}
		else if (uFlags & CMF_EXPLORE)
		{
		}
		else if (uFlags & CMF_DEFAULTONLY)
		{
			bAppendItems = FALSE;
		}
		else
		{
			bAppendItems = FALSE;
		}


		if (bAppendItems)
		{
			HMENU hParentMenu = ::CreateMenu();


			// Add the menu entries
			pT->GetMenuEntries();


			if (hParentMenu && m_vecMenuEntries.size()) 
			{
				// pop-up title
				::InsertMenu(hMenu, indexMenu, MF_POPUP|MF_BYPOSITION, 	(UINT_PTR)hParentMenu, _T("ImageWalker"));

				HBITMAP hBitmap = ::LoadBitmap(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDB_IMAGEWALKER));
				SetMenuItemBitmaps(hMenu, indexMenu, MF_BYPOSITION, hBitmap, hBitmap);
				//::DeleteObject(hBitmap);

				for(UINT i = 0;  i < m_vecMenuEntries.size(); i++)
				{
					if (m_vecMenuEntries[i])
					{
						::InsertMenu(hParentMenu, i, MF_STRING|MF_BYPOSITION, 
							idCmd++, m_vecMenuEntries[i]->_imageName);
					}
					else
					{
						::InsertMenu(hParentMenu, i, MF_SEPARATOR|MF_BYPOSITION, idCmd++, NULL);
					}

				}
			}

			//Must return number of menu
			return MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(idCmd-idCmdFirst));
			//items we added.
		}
		return NOERROR;

	}

	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
	{
		HRESULT hr = NOERROR;
		//If HIWORD(lpcmi->lpVerb) then we have been called programmatically
		//and lpVerb is a command that should be invoked.  Otherwise, the shell
		//has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
		//selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
		if (!HIWORD(lpcmi->lpVerb)) 
		{
			UINT idCmd = LOWORD(lpcmi->lpVerb);

			if (m_vecMenuEntries.size() > idCmd)
			{
				m_vecMenuEntries[idCmd]->Invoke(lpcmi->hwnd);
			}
		}

		return hr;
	}

	STDMETHODIMP GetCommandString(UINT idCmd,
		UINT uFlags,
		UINT FAR *reserved,
		LPSTR pszName,
		UINT cchMax)
	{
		USES_CONVERSION;

		switch (uFlags) 
		{
		case GCS_HELPTEXT:		// fetch help text for display at the bottom of the 
			// explorer window
			if (m_vecMenuEntries.size() > idCmd)
			{
				strncpy_s(pszName, cchMax, CT2A(m_vecMenuEntries[idCmd]->_imageHelp), cchMax);
			}
			else
			{
				pszName[0] = 0;
			}
			break;

		case GCS_VALIDATE:
			break;

		case GCS_VERB:			// language-independent command name for the menu item 
			if (m_vecMenuEntries.size() > idCmd)
			{
				strncpy_s(pszName, cchMax, CT2A(m_vecMenuEntries[idCmd]->_imageVerb), cchMax);
			}
			else
			{
				pszName[0] = 0;
			}
			break;
		}
		return NOERROR;
	}
	

	STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder,
		LPDATAOBJECT pDataObj,
		HKEY hRegKey)
	{
		T* pT = static_cast<T*>(this);

		// Fail if no data object
		if (!pDataObj)
			return E_FAIL;


		// get these paths into a vector of strings
		m_vecFileNames.clear();

		// fetch all of the file names we're supposed to operate on
		if (pDataObj) 
		{
			pDataObj->AddRef();

			STGMEDIUM medium;
			FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};


			HRESULT hr = pDataObj->GetData (&fe, &medium);
			if (FAILED (hr))
			{
				return E_FAIL;
			}

			// buffer to receive filenames
			TCHAR path[MAX_PATH];

			// how many are there?
			UINT fileCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, path, MAX_PATH);

			if (fileCount>0)
			{

				// stash the paths in our vector of strings
				for (UINT i=0;i<fileCount;i++) 
				{
					// clear old path
					ZeroMemory(path, MAX_PATH);
					// fetch new path
					if (DragQueryFile((HDROP)medium.hGlobal, i, path, MAX_PATH)) 
					{
						m_vecFileNames.push_back(path);
					}
				}

			}

			// free our path memory - we have the info in our vector of strings
			ReleaseStgMedium(&medium);
		}

		return S_OK;
	}

};