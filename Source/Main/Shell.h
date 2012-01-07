///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////

#ifndef _WINDOWS_SHELL_H_
#define _WINDOWS_SHELL_H_


#include <WinInet.h>
#include <ShlObj.h>

#include <shtypes.h>
#include <comdef.h>
#include <shlGuid.h>
#include <ShellApi.h>
#include <ShlwApi.h> // For PathIsUrl function

#include <TChar.h>
#include "Base.h"

namespace IW {

class CShellFolder;

class CShellMalloc : public IW::RefPtr<IMalloc>
{
	
public:
	CShellMalloc()
	{
		LPMALLOC pMalloc = 0;
		HRESULT hr = SHGetMalloc (&pMalloc);
		
		if (hr != NOERROR)
			throw std::exception("SHGetMalloc Failed");

		Attach(pMalloc);
	}

	virtual ~CShellMalloc()
	{
	}


};



class CShellItem
{
protected:
	
	
public:
	
	CShellItem() : m_pItem(0)
	{
	}

	CShellItem(LPCITEMIDLIST pItemSrc) : m_pItem(0)
	{
		Copy(pItemSrc);
	}

	CShellItem(LPITEMIDLIST pItemSrc) : m_pItem(pItemSrc)
	{
	}

	CShellItem(const CShellItem &item) : m_pItem(0)
	{
		Copy(item);
	}

	bool IsNull() const
	{
		return m_pItem == 0;
	}
	
	bool Open(HWND hwnd, const int nSpecialFolder)
	{
		LPITEMIDLIST pItem = 0; 
		HRESULT hr = SHGetSpecialFolderLocation(hwnd, nSpecialFolder, &pItem);
		
		if (FAILED(hr))
			return false;

		m_pItem = pItem;
		return true;
	}

	bool Open(const CString &strPath)
	{
		LPITEMIDLIST pItem = 0; 
		ULONG chEaten = 0;
		USES_CONVERSION;

		IW::RefPtr<IShellFolder> spDesktopFolder;

		HRESULT hr = ::SHGetDesktopFolder(spDesktopFolder.GetPtr());
		if (FAILED(hr))
		{
			return false;
		}

		hr = spDesktopFolder->ParseDisplayName(NULL, NULL, CT2OLE(strPath), &chEaten, &pItem, NULL);

		if (FAILED(hr))
		{
			return false;
		}

		m_pItem = pItem;
		return true;
	}

	virtual ~CShellItem()
	{
		Free();
	}

	bool GetPath(CString &str) const
	{
		bool bRet = SHGetPathFromIDList(GetItem(), str.GetBuffer(MAX_PATH)) != 0;
		str.ReleaseBuffer( );
		return bRet;
	}
	
	HRESULT Copy(const CShellItem &item)
	{
		return Copy(item.m_pItem);
	}
	
	HRESULT Copy(LPCITEMIDLIST pItemSrc)
	{
		if (pItemSrc != 0)
		{
			
			UINT cbTotal = GetSize(pItemSrc);
			LPITEMIDLIST pItem = Create(cbTotal);
			
			if (pItem == 0)
				return E_OUTOFMEMORY;
			
			IW::MemCopy(pItem, pItemSrc, cbTotal);
			
			Attach(pItem);
			
			ATLASSERT((m_pItem == 0) || IsDesktop() ||  CShellMalloc()->DidAlloc(m_pItem) );
			
		}
		else
		{
			Free();
			m_pItem = 0; 
		}
		
		return S_OK;
	}

	HRESULT StripToTail()
	{
		ATLASSERT(m_pItem != NULL);
		ATLASSERT(CShellMalloc()->DidAlloc(m_pItem));
		
		LPCITEMIDLIST p = m_pItem;
		LPCITEMIDLIST pLast;
		
		while(p->mkid.cb)
		{
			pLast = p;
			p = Next(p);
		}

		return Copy(pLast);
	}

	LPCITEMIDLIST GetTailItem() const
	{
		return GetTailItem(m_pItem);
	};

	static LPCITEMIDLIST GetTailItem(LPCITEMIDLIST p)
	{
		ATLASSERT(p != NULL);
		ATLASSERT(CShellMalloc()->DidAlloc((LPVOID)p));
		
		LPCITEMIDLIST pLast = p;
		
		while(p->mkid.cb)
		{
			pLast = p;
			p = Next(p);
		}

		return pLast;
	};
	

	HRESULT CopyTo(long **ppItemLongOut) const
	{
		// Probably a gug if its not 0?
		ATLASSERT(*ppItemLongOut == 0);

		CShellItem item;
		
		HRESULT hr = item.Copy(m_pItem);
		
		if (SUCCEEDED(hr))
			*ppItemLongOut = reinterpret_cast<long*>(item.Detach());
		
		return hr;
	}
	
	HRESULT CopyTo(LPITEMIDLIST *ppidl) const
	{
		// Probably a gug if its not 0?
		ATLASSERT(*ppidl == 0);

		CShellItem item;
		
		HRESULT hr = item.Copy(m_pItem);
		
		if (SUCCEEDED(hr))
			*ppidl = item.Detach();
		
		return hr;
	}
	
	// The assert on operator& usually indicates a bug.  
	// If this is really
	// what is needed, however, take the address of the 
	// m_pItem member explicitly.
	LPITEMIDLIST* GetPtr()
	{
		ATLASSERT(m_pItem == 0);
		return &m_pItem;
	}

	long** GetLongPtr()
	{
		ATLASSERT(m_pItem == 0);
		return reinterpret_cast<long**>(&m_pItem);
	}

	long *GetLong()
	{
		return reinterpret_cast<long*>(m_pItem);
	}
	
	void Free()
	{
		if (m_pItem)
		{
			ATLASSERT(CShellMalloc()->DidAlloc(m_pItem));
			CShellMalloc()->Free(m_pItem);
		}
		
		m_pItem = NULL;   
	}
	
	
	
	void Attach(LPITEMIDLIST pItem)
	{
		ATLASSERT((m_pItem == 0) || CShellMalloc()->DidAlloc(m_pItem) );
		ATLASSERT((pItem != 0) || CShellMalloc()->DidAlloc(pItem));
		
		Free();
		
		m_pItem = pItem; 
	}
	
	LPITEMIDLIST Detach()
	{
		ATLASSERT((m_pItem == 0) || IsDesktop() || CShellMalloc()->DidAlloc(m_pItem) );
		
		LPITEMIDLIST p = m_pItem;
		m_pItem = NULL; 
		
		return p;
	}
	
	inline const CShellItem& operator=(const CShellItem &item) 
	{
		ATLASSERT(item.m_pItem != NULL);
		Copy(item);
		return *this;
	}

	inline const CShellItem& operator=(LPCITEMIDLIST pidl)
	{
		ATLASSERT(pidl != NULL);
		Copy(pidl);
		return *this;
	}

	
	short Compare(const CShellItem &item) const;
	short Compare(IShellFolder *pFolder, const CShellItem &item) const;
	
	inline bool operator>(const CShellItem &item) const 
	{
		ATLASSERT(m_pItem != NULL);
		return Compare(item) > 0;
	}
	
	inline bool operator<(const CShellItem &item) const 
	{
		ATLASSERT(m_pItem != NULL);
		return Compare(item) < 0;
	}
	
	inline bool operator==(const CShellItem &item) const
	{
		ATLASSERT(m_pItem != NULL);
		return Compare(item) == 0;
	}

	
	inline bool operator!=(const CShellItem &item) const
	{
		return Compare(item) != 0;
	}
	
	operator LPCITEMIDLIST*() const
	{ 
		ATLASSERT(m_pItem != NULL);
		return (LPCITEMIDLIST*)&m_pItem; 
	};
	
	operator LPCITEMIDLIST() const
	{ 
		ATLASSERT(m_pItem != NULL);
		return m_pItem; 
	};
	
	LPCITEMIDLIST GetItem() const
	{
		ATLASSERT(m_pItem != NULL);
		return m_pItem; 
	};
	
	bool IsDesktop() const 
	{ 
		ATLASSERT(m_pItem != NULL);
		return (m_pItem->mkid.cb == 0); 
	}

	static bool IsDesktop(LPCITEMIDLIST pItem) 
	{ 
		ATLASSERT(pItem != NULL);
		return (pItem->mkid.cb == 0); 
	}
	
	int Depth() const
	{
		return Depth(m_pItem);		
	}

	static int Depth(LPCITEMIDLIST p)
	{
		ATLASSERT(p != NULL);
		ATLASSERT(CShellMalloc()->DidAlloc((LPVOID)p));
		
		int i = 0;
		
		while(p->mkid.cb)
		{
			p = Next(p);
			i+=1;
		}
		
		return i;
	}
	
	
	void Cat(LPCITEMIDLIST pItem1, LPCITEMIDLIST pItem2)
	{
		ATLASSERT((m_pItem == 0) || IsDesktop() || CShellMalloc()->DidAlloc(m_pItem) );

		if (pItem1 == 0 || IsDesktop(pItem1))
		{
			Copy(pItem2);
		}
		else if (pItem2 == 0 || IsDesktop(pItem2))
		{
			Copy(pItem1);
		}
		else
		{
			LPITEMIDLIST pidlNew;
			
			UINT cb1 = GetSize(pItem1) - sizeof(m_pItem->mkid.cb);
			UINT cb2 = GetSize(pItem2);
			
			pidlNew = Create(cb1 + cb2);
			
			if (pidlNew)
			{
				IW::MemCopy(((LPBYTE)pidlNew), pItem1, cb1);
				IW::MemCopy(((LPBYTE)pidlNew) + cb1, pItem2, cb2);
				
				Attach(pidlNew);
			}
		}
	}
	
	void Cat(LPCITEMIDLIST pItem)
	{
		ATLASSERT((m_pItem == 0) || IsDesktop() || CShellMalloc()->DidAlloc(m_pItem) );

		if (m_pItem == 0 || IsDesktop())
		{
			Copy(pItem);
		}
		else
		{
			LPITEMIDLIST pidlNew;
			
			UINT cb1 = GetSize(m_pItem) - sizeof(m_pItem->mkid.cb);
			UINT cb2 = GetSize(pItem);
			
			pidlNew = Create(cb1 + cb2);
			
			if (pidlNew)
			{
				IW::MemCopy(((LPBYTE)pidlNew), m_pItem, cb1);
				IW::MemCopy(((LPBYTE)pidlNew) + cb1, pItem, cb2);
				
				Free();
				m_pItem = pidlNew;
			}
		}
	}
	
	bool Read(const CString &strValueName, const IPropertyArchive *pArchive)
	{
		CString str;

		if (pArchive->Read(strValueName, str) && !str.IsEmpty())
		{
			return Open(str);
		}
		
		return false;
	}
	
	bool Write(const CString &strValueName, IPropertyArchive *pArchive) const;
	
	// Strip final item to make this the parent
	bool StripToParent()
	{
		ATLASSERT(m_pItem != 0);
		ATLASSERT(CShellMalloc()->DidAlloc(m_pItem) );	
		
		int n;
		int nItemSize2 = 0;
		SHITEMID    *pmkid;
		SHITEMID    *pmkid2;
		
		if (m_pItem && m_pItem->mkid.cb)
		{
			pmkid = (SHITEMID*)m_pItem;
			
			while(pmkid->cb != 0)
			{
				n = pmkid->cb;
				pmkid2 = pmkid;
				pmkid = (SHITEMID*)((LPBYTE)pmkid + n);
				nItemSize2 += n;
			}
			
			pmkid2->cb = 0;
		}
		else
		{
			return false;
		}
		
		return true;
	}

	// Get the fist item only
	bool StripToFirst()
	{
		ATLASSERT(m_pItem != 0);
		ATLASSERT(CShellMalloc()->DidAlloc(m_pItem) );	
		
		int nItemSize2 = 0;
		SHITEMID    *pmkid;
		
		if (m_pItem && m_pItem->mkid.cb)
		{
			pmkid = (SHITEMID*)m_pItem;
			pmkid = (SHITEMID*)((LPBYTE)pmkid + pmkid->cb);			
			pmkid->cb = 0;
		}
		else
		{
			return false;
		}
		
		return true;
	}

	
private:
	LPITEMIDLIST m_pItem;	
	
protected:			
	static LPITEMIDLIST Create(UINT cbSize)
	{
		CShellMalloc pMalloc;
		LPITEMIDLIST pidl = (LPITEMIDLIST)pMalloc->Alloc(cbSize);		
		if (pidl == NULL) throw std::bad_alloc();		
		return pidl;
	}
	
public:
	static LPCITEMIDLIST Next(LPCITEMIDLIST pidl)
	{
		LPBYTE lpMem=(LPBYTE)pidl;
		
		lpMem+=pidl->mkid.cb;
		
		return (LPCITEMIDLIST)lpMem;
	}

protected:
	
	static UINT GetSize(LPCITEMIDLIST pidl)
	{
		UINT cbTotal = 0;
		
		if (pidl)
		{
			cbTotal += sizeof(pidl->mkid.cb);       // Null terminator
			
			while (pidl->mkid.cb)
			{
				cbTotal += pidl->mkid.cb;
				pidl = Next(pidl);
			}
		}
		
		return cbTotal;
	}
	
};


class CShellDesktopItem : public CShellItem
{
public:
	CShellDesktopItem()
	{
		static ITEMIDLIST itemDesktop = {0};
		Copy(&itemDesktop);
	}

	virtual ~CShellDesktopItem()
	{
	}

	
};



class CShellFolder : public IW::RefPtr<IShellFolder>
{

public:	

	CShellFolder() throw()
	{
	}

	CShellFolder(IShellFolder* pIn) throw() :  IW::RefPtr<IShellFolder>(pIn)
	{
	}

	virtual ~CShellFolder() throw()
	{
	}

	IShellFolder* operator=(IShellFolder* pIn) 
	{
		IW::ReferencePtrAssign(&p, pIn);
		return p;
	}

public:

	HRESULT Open(const CShellItem &item, bool bBuildUpFolder = false);
	HRESULT Open(CShellFolder &pFolder, const CShellItem &item, bool bBuildUpFolder = false);
	
	DWORD GetAttributes(const IW::CShellFolder &pFolder, LPCITEMIDLIST pItem, DWORD dwAttributes) const;
	DWORD GetAttributes(LPCITEMIDLIST pItem, DWORD dwAttributes) const
	{
		return GetAttributes(*this, pItem, dwAttributes);
	}

	HRESULT BindToStorage(const IW::CShellFolder &pFolder, const IW::CShellItem &item, REFIID riid, void **ppv) const;
	HRESULT BindToStorage(const IW::CShellItem &item, REFIID riid, void **ppv) const
	{
		return BindToStorage(*this, item, riid, ppv);
	}

	bool IsFolder(LPCITEMIDLIST pItem) const
	{
		if (GetAttributes(pItem, SFGAO_REMOVABLE) & SFGAO_REMOVABLE )
		{
			return true;
		}

		return (GetAttributes(pItem, SFGAO_FOLDER) & SFGAO_FOLDER) != 0;
	}

	bool IsBrowsable(LPCITEMIDLIST pItem) const
	{
		if (GetAttributes(pItem, SFGAO_REMOVABLE) & SFGAO_REMOVABLE )
		{
			return true;
		}

		DWORD u = GetAttributes(pItem, SFGAO_FOLDER);
		bool bIsFolder = ((u & SFGAO_FOLDER) != 0);

		//bool bIsStream = ((u & SFGAO_STREAM) != 0);
		//bool bIsBrowsable = ((u & SFGAO_BROWSABLE) != 0);
		//bool bIsFileSysAncestor = ((u & SFGAO_FILESYSANCESTOR) != 0);

		// Nasty hack to remove .zip files
		if (bIsFolder)
		{
			CString str = GetDisplayNameOf(pItem, SHGDN_INFOLDER|SHGDN_FORPARSING);
			LPCTSTR szExt = IW::Path::FindExtension(str);
			bIsFolder = (0 != _tcsicmp(szExt, _T(".zip")));
		}

		return bIsFolder;
	}

	CShellItem GetShellItem() const
	{
		CComQIPtr<IPersistFolder2> pExtInit = *this;
		CShellItem item;

		if (pExtInit != 0)
		{
			pExtInit->GetCurFolder(item.GetPtr());
		}

		return item;
	}

	inline CString GetDisplayNameOf (LPCITEMIDLIST pItem, unsigned long uFlags) const
	{
		STRRET strReturn;		
		HRESULT hr = p->GetDisplayNameOf(pItem, uFlags, &strReturn);

		if (SUCCEEDED(hr))
			return ConvertStrRetToString(strReturn, pItem);
		
		return g_szEmptyString;
	}
	
	static CString ConvertStrRetToString(STRRET &strReturn, LPCITEMIDLIST pidl)
	{
		HRESULT hr = S_OK;
		CString str;
		
		switch (strReturn.uType)
		{
		case STRRET_WSTR:

			str = strReturn.pOleStr;
			
			// Release the ole string
			{
				
				CShellMalloc spMalloc;
				ATLASSERT(spMalloc->DidAlloc(strReturn.pOleStr));
				
				if (spMalloc)
					spMalloc->Free(strReturn.pOleStr);
				
			}
			break;
			
		case STRRET_OFFSET:			
			str = ((LPCSTR)pidl) + strReturn.uOffset;
			break;
			
		case STRRET_CSTR:			
			str = (LPCSTR)strReturn.cStr;
			break;
			
		default:
			assert(0);
			break;
		}
		
		return str;
	}

	HRESULT ResolveLink(LPCITEMIDLIST pItemIn, IW::CShellItem &itemOut) const
	{
		IW::RefPtr<IStream> pStream;
		HRESULT hr = BindToStorage(pItemIn, IID_IStream, (LPVOID*)&pStream);
		if (FAILED(hr)) 
			return hr;
				
		// Get a pointer to the IShellLink interface.
		CComPtr<IShellLink> psl;		
		hr = psl.CoCreateInstance(CLSID_ShellLink);
		
		if (SUCCEEDED (hr))
		{
			// Get a pointer to the IPersistFile interface.
			CComQIPtr<IPersistStream> pps(psl);

			if (pps != 0)
			{
				hr = pps->Load(pStream);
			}
			else
			{
				hr = E_FAIL;
			}				

			if (FAILED(hr))
			{
				CString strFileName = GetDisplayNameOf(pItemIn, SHGDN_FORPARSING);

				if (!strFileName.IsEmpty())
				{
					USES_CONVERSION;
					CComQIPtr<IPersistFile> ppf(psl); 
					hr = ppf->Load(CT2OLE(strFileName), STGM_WRITE);
				}
			}
			
			if (SUCCEEDED (hr))
			{
				LPITEMIDLIST pidl;
				hr = psl->GetIDList(&pidl);
				
				if (SUCCEEDED (hr))
				{
					itemOut.Attach(pidl);
				}
			}
		}
		
		return hr;
	}
};

class CShellDesktop : public CShellFolder
{
public:
	CShellDesktop()
	{
		LPSHELLFOLDER  pshf;
		HRESULT hr = SHGetDesktopFolder (&pshf);
		
		if (hr != NOERROR)
			throw std::exception("SHGetDesktopFolder Failed");
		
		Attach(pshf);
	}

	virtual ~CShellDesktop()
	{
	}	
	
	
	
	
	static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
	{
		
		switch(uMsg) 
		{
		case BFFM_INITIALIZED: 
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			SendMessage(hwnd, BFFM_SETSELECTION, FALSE, pData);
			return 1;
			
		default:
			break;
		}
		return 0;
	}
	
	
	static bool GetDirectory(HWND hWndParent, CString &strDir)
	{
		USES_CONVERSION;
		
		LPITEMIDLIST pidl; 
		
		CShellItem item;
		CShellDesktop desktop;
		
		ULONG         chEaten;
		ULONG         dwAttributes;
		HRESULT       hr;
		

		CString strDirCorrected = strDir;
		strDirCorrected.Replace(_T('/'), _T('\\'));
		
		hr = desktop->ParseDisplayName(
			NULL,
			NULL,
			(LPOLESTR)CT2OLE(strDirCorrected),
			&chEaten,
			&pidl,
			&dwAttributes);
		
		if (SUCCEEDED(hr))
		{
			CShellItem item;
			item.Attach(pidl);
			pidl = 0;			
			
			return GetDirectory(hWndParent, item) &&
				item.GetPath(strDir); 
		}
		else
		{
			CShellItem item;
			
			return GetDirectory(hWndParent, item) &&
				item.GetPath(strDir); 
		}
		
		return false;
	}
	
	
	static bool GetDirectory(HWND hWndParent, CShellItem &item)
	{
		LPITEMIDLIST pidl; 
		TCHAR szPath[MAX_PATH];
		
		BROWSEINFO bi;
		bi.hwndOwner =	hWndParent;
		bi.pidlRoot	= NULL;
		bi.pszDisplayName	= szPath;
		bi.lpszTitle =	_T("Select Folder..."); 
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE; 
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)(LPCITEMIDLIST)item;
		bi.iImage = 0;
		
		pidl = SHBrowseForFolder(&bi); 
		item.Attach(pidl);
		
		return  (NULL != pidl);
	}
	
	
};




class CShellItemEnum : public CComPtr<IEnumIDList>
{
public:
	CShellItemEnum()
	{
	}

	virtual ~CShellItemEnum()
	{
	}
	
	HRESULT Create(HWND hwnd, IShellFolder *pFolder, UINT nFlags)
	{
		LPENUMIDLIST    lpe;
		
		// Populate first Level
		// Get the IEnumIDList object for the given folder.
		HRESULT hr = pFolder->EnumObjects(hwnd, nFlags, &lpe);
		
		// Cant bind?
		if (FAILED(hr)) return hr;
		
		Attach(lpe);
		
		return S_OK;
	}
};

inline short CShellItem::Compare(IShellFolder *pFolder, const CShellItem &item) const
{
	if (IsDesktop())
	{
		return item.IsDesktop() ? 0 : -1;
	}
	
	if (item.IsDesktop())
	{
		return IsDesktop() ? 0 : 1;
	}
	
	HRESULT hr = pFolder->CompareIDs(0, *this, item);

	if (SUCCEEDED(hr))
	{	
		short n = static_cast<short>(SUCCEEDED(hr) ? SCODE_CODE(hr) : 1);	
		return n;
	}

	return -1;
}


inline short CShellItem::Compare(const CShellItem &item) const
{
	return Compare(CShellDesktop(), item);
}

inline HRESULT CShellFolder::Open(const CShellItem &item, bool bBuildUpFolder)
{
	HRESULT hr = S_OK;
	
	CShellDesktop pDesktop;
	
	if (item.IsDesktop())
	{
		Copy(pDesktop);
	}
	else
	{
		hr = Open(pDesktop, item, bBuildUpFolder);
	}

	return hr;
}

inline HRESULT CShellFolder::Open(CShellFolder &pFolder, const CShellItem &item, bool bBuildUpFolder)
{
	HRESULT hr = S_OK;

	if (bBuildUpFolder && item.Depth() > 1)
	{
		CShellItem itemFirst(item);

		if (!itemFirst.StripToFirst())
			return E_FAIL;

		CShellFolder pFolderLocal;
		hr = pFolder->BindToObject(itemFirst, 
				NULL, IID_IShellFolder, 
				(LPVOID*)pFolderLocal.GetPtr());

		if (SUCCEEDED(hr))
		{
			CShellItem itemRest(CShellItem::Next(item));
			hr = Open(pFolderLocal, itemRest, bBuildUpFolder);
		}
	}
	else
	{
		hr = pFolder->BindToObject(item, 
				NULL, IID_IShellFolder, 
				(LPVOID*)GetPtr());
	}

	return hr;
}

inline DWORD CShellFolder::GetAttributes(const IW::CShellFolder &pFolder, LPCITEMIDLIST pItem, DWORD dwAttributes) const
{
	HRESULT hr = S_OK;
	ULONG ulAttrs = dwAttributes;

	if (IW::CShellItem::Depth(pItem) > 1)
	{
		IW::CShellItem itemFirst(pItem);

		if (!itemFirst.StripToFirst())
			return E_FAIL;

		IW::CShellFolder pFolderLocal;
		hr = pFolder->BindToObject(itemFirst, 
				NULL, IID_IShellFolder, 
				(LPVOID*)pFolderLocal.GetPtr());

		if (SUCCEEDED(hr))
		{
			IW::CShellItem itemRest(IW::CShellItem::Next(pItem));
			hr = pFolder->GetAttributesOf(1, itemRest, &ulAttrs);
		}
	}
	else
	{
		hr = pFolder->GetAttributesOf(1, &pItem, &ulAttrs);
	}
	
	assert(SUCCEEDED(hr));
	
	return ulAttrs;
}

inline HRESULT CShellFolder::BindToStorage(const IW::CShellFolder &pFolder, const IW::CShellItem &item, REFIID riid, void **ppv) const
{
	HRESULT hr = S_OK;

	if (item.Depth() > 1)
	{
		IW::CShellItem itemFirst(item);

		if (!itemFirst.StripToFirst())
			return E_FAIL;

		IW::CShellFolder pFolderLocal;
		hr = pFolder->BindToObject(itemFirst, 
				NULL, IID_IShellFolder, 
				(LPVOID*)pFolderLocal.GetPtr());

		if (SUCCEEDED(hr))
		{
			IW::CShellItem itemRest(IW::CShellItem::Next(item));
			hr = BindToStorage(pFolderLocal, itemRest, riid, ppv);
		}
	}
	else
	{
		hr = pFolder->BindToStorage(item, 0, riid, ppv);
	}

	return hr;
}


typedef CSimpleValArray<CShellItem> ShellItemList;

template<class TProperties>
void LoadShellItemList(IW::ShellItemList &list, const CString &strValueName, TProperties *pArchive)
{
	if (pArchive->StartSection(strValueName))
	{
		list.RemoveAll();

		// Load the copt to list
		CString str;
		IW::CShellItem item;
		unsigned int i = 0;
		DWORD nMax = 0;

		if (!pArchive->Read(_T("Count"), nMax))
			nMax = 0;

		while(i < nMax)
		{
			str.Format(_T("Item %d"), i);

			if (!item.Read(str, pArchive))
				break;

			list.Add(item);
			i++;
		}

		pArchive->EndSection();
	};
}

template<class TProperties>
void SaveShellItemList(const IW::ShellItemList &list, const CString &strValueName, TProperties *pArchive)
{
	if (pArchive->StartSection(strValueName))
	{
		CString str;
		pArchive->Write(_T("Count"), list.GetSize());

		for(int i = 0; i < list.GetSize(); i++)
		{
			str.Format(_T("Item %d"), i);
			list[i].Write(str, pArchive);
		}

		pArchive->EndSection();
	}
}

inline bool CShellItem::Write(const CString &strValueName, IPropertyArchive *pArchive) const
{
	ATLASSERT(m_pItem != 0);
	ATLASSERT(CShellMalloc()->DidAlloc(m_pItem) );

	CString strPath = CShellDesktop().GetDisplayNameOf(*this, SHGDN_FORPARSING);
	return pArchive->Write(strValueName, strPath);
};

}; // namespace IW


#endif //_WINDOWS_SHELL_H_