///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


#pragma once

class CLoadAny;
class TestSearch;
class FolderItem200;
class FolderItem105;

namespace Search
{
	class Spec;
}

namespace IW
{

IWINTERFACECLASS IPropertyArchive;

class Folder;
class CRender;
class FolderItemLoader;
class FolderItem;
class ThumbnailCache;

typedef RefPtr<Folder> FolderPtr;
typedef IW::RefPtr<FolderItem> FolderItemPtr;

typedef std::map<CString, IW::FolderItemPtr> MAPNAMETOTHUMB;
typedef std::vector<IW::FolderItemPtr> ITEMLIST;
typedef std::map<CString, int> TAGMAP;
typedef std::set<CString> TAGSET;

inline TAGSET Split(const CString &strTagsIn)
{
	TAGSET tags;
	TCHAR szSeps[]   = _T(";");
	int curPos = 0;
	CString token = strTagsIn.Tokenize(szSeps, curPos);

	while (!token.IsEmpty())
	{
		CString strEntry(token);
		strEntry.Trim();
		tags.insert(strEntry);
		token = strTagsIn.Tokenize(szSeps, curPos);
	}

	return tags;
};

inline CString Combine(const TAGSET &tags)
{
	CString strTagsOut;

	for (IW::TAGSET::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		if (!strTagsOut.IsEmpty()) strTagsOut += "; ";
		strTagsOut += *it;
	}

	return strTagsOut;
}


enum 
{
	// Flags for CanDisplayNewImage
	eAbortLoad = 0x01, 
	eAutoSave = 0x04
};

////////////////////////////////////////////////////////////////////////////////
////
//// FolderItem
////


// Metadata Constants
enum {
	ePropertyName = 0,
	ePropertyType = 1,
	ePropertySize = 2,
	ePropertyModifiedDate = 3,
	ePropertyModifiedTime = 6,
	ePropertyCreatedDate = 4,
	ePropertyCreatedTime = 7,
	ePropertyPath = 5,
	ePropertyWidth = 8,
	ePropertyHeight = 9,
	ePropertyDepth = 10,
	ePropertyTitle = 11,
	ePropertyAperture = 12,
	ePropertyIsoSpeed = 13,
	ePropertyWhiteBalance = 14,
	ePropertyExposureTime = 15,
	ePropertyFocalLength = 16,
	ePropertyDateTaken = 17,
	ePropertyDescription = 18,
	ePropertyObjectName = 19
};





#define THUMB_PADDING 2
#define THUMB_WORD_HEIGHT 25

#define THUMB_LOADED     0x0001
#define THUMB_SELECTED   0x0002
#define THUMB_IMAGE      0x0004
#define THUMB_FOCUS		 0x0008
#define THUMB_SHOWN      0x0010
#define THUMB_DELETE     0x0020
#define THUMB_LOADING    0x0040
#define THUMB_INVALIDATE 0x0100
#define THUMB_IMAGE_ICON 0x0200

#define THUMB_IS_SEARCH_RESULT	0x0800

#define THUMB_PERSIST_MASK (THUMB_LOADED | THUMB_IMAGE | THUMB_INVALIDATE)



class FolderItem ;
class Folder;

struct FolderItemAttributes
{
	bool bIsFolder;
	bool bIsImage;
	bool bIsImageIcon;
	bool bHasIPTC;
	bool bHasXmp;
	bool bHasExif;
	bool bHasICC;
	bool bHasError;	
	bool bIsLink;
	bool bIsHidden;

	int	nSubItemCount;
	int nImage;
	int nPageCount;	
	int nFrame;	
	IW::Image image;

	bool HasAttribute()
	{
		return bHasError || bIsLink  || bIsHidden || bHasIPTC || bHasExif || bHasXmp || bHasICC || (nPageCount > 1);
	}

	static void AddMarker(CString &strMarkers, const CString &strMarker)
	{
		if (strMarkers.GetLength() > 0) strMarkers += _T("\n");
		strMarkers += strMarker;
	}

	CString ToString()
	{
		CString strMarkers;

		if (!bIsFolder)
		{
			if (bHasError) AddMarker(strMarkers, _T("Error"));
			if (bIsLink) AddMarker(strMarkers, _T("Link"));
			if (bIsHidden) AddMarker(strMarkers, _T("Hidden"));
			if (bHasIPTC) AddMarker(strMarkers, _T("IPTC"));
			if (bHasXmp) AddMarker(strMarkers, _T("XMP"));
			if (bHasExif) AddMarker(strMarkers, _T("EXIF"));
			if (bHasICC) AddMarker(strMarkers, _T("ICC"));

			if (nPageCount > 1)
			{
				CString strPages;
				strPages.Format(_T("%d Pages"), nPageCount); 
				AddMarker(strMarkers, strPages);
			}
		}

		return strMarkers;
	}
};

class FolderItem : public IW::Referenced
{
private:

	IW::FileTime _ftCreationTime; 
    IW::FileTime _ftLastWriteTime; 
	IW::Image _image;
	IW::CShellItem _item;
	IW::CShellItem _itemGap;
	IW::CShellFolder _pFolder;	
	DWORD _dwFileAttributes;
	DWORD _dwHashCode;
	IW::FileSize  _sizeFile;
	ULONG _ulAttribs;
	UINT  _uExtension;	
	int   _nImage;
	int   _nTimer;
	int   _nFrame;	
	UINT  _uFlags;	
	int	  _nSubItemCount;

	mutable __int64 _nCachedThumbnailId;

public:

	FolderItem();
	FolderItem(const FolderItem &t);
	~FolderItem();	

	void operator =(const FolderItem &t);
	
	operator LPCITEMIDLIST() const
	{ 
		return _item.GetItem(); 
	};

	bool Init(const CString &strFilePath);
	bool Init(IShellFolder *pFolder, LPITEMIDLIST pItem, LPCITEMIDLIST pItemGap = 0);

	static IW::FolderItemPtr CreateTestItem();

	HRESULT RefreshItem();
	UINT GetFlags() const { return _uFlags; };
	__int64 GetCachedThumbnailId() const { return _nCachedThumbnailId; };
	void SetCachedThumbnailId(__int64 n) const { _nCachedThumbnailId = n; };
	bool IsFolder() const;
	bool IsImage() const;
	bool IsImageIcon() const;
	bool IsLink() const;
	bool IsFileSystem() const;
	bool CanRename() const;
	bool IsReadOnly() const;

	bool IsZipFile() const
	{
		CString strZip(_T(".zip"));
		return strZip.CompareNoCase(IW::Path::FindExtension(GetFileName())) == 0;
	}

	bool LoadJobBegin(FolderItemLoader &loader);
	void LoadJobEnd(FolderItemLoader &loader);

	CString GetDisplayNameOf(DWORD flags) const
	{
		return _pFolder.GetDisplayNameOf(_item, flags);
	}

	CString GetKeyName() const
	{
		CString strKey = GetDisplayNameOf(SHGDN_FORPARSING|SHGDN_INFOLDER);
		strKey.MakeLower();
		return strKey;
	}

	CString GetCacheName() const
	{
		IW::CFilePath path = GetDisplayNameOf(SHGDN_FORPARSING);
		path.NormalizeForCompare();
		return path;
	}	

	CString GetFileName() const
	{
		return GetDisplayNameOf(SHGDN_FORPARSING|SHGDN_INFOLDER);
	}

	CString GetDisplayPath() const
	{
		return GetDisplayNameOf(SHGDN_NORMAL);
	}

	CString GetDisplayName() const
	{
		return GetDisplayNameOf(SHGDN_NORMAL|SHGDN_INFOLDER);
	}

	CString GetFilePath() const
	{
		return GetDisplayNameOf(SHGDN_FORPARSING);
	}

	bool SetItemName(const CString &strNewName, DWORD nFlags);	

	bool SetItemName(const CString &strNewName)
	{
		return SetItemName(strNewName, SHGDN_NORMAL);
	}

	bool SetFileName(const CString &strNewName)
	{
		return SetItemName(strNewName, SHGDN_INFOLDER|SHGDN_FORPARSING);
	}

	IW::FolderPtr GetFolder();
	IW::Image OpenAsImage(CLoadAny &loader, IW::IStatus *pStatus);
	
	IW::FileTime GetTakenTime() const
	{		
		IW::FileTime ft = _image.GetCameraSettings().DateTaken;
		if (!ft.IsEmpty()) return ft;
		return _ftCreationTime; 
	}

	IW::CShellItem GetFullShellItem() const
	{		
		CShellItem item(_pFolder.GetShellItem());
		item.Cat(_item);
		return item;
	}

	CRect GetItemThumbRect() const
	{
		return _image.GetBoundingRect();
	}

	const IW::CShellItem &GetShellItem() const { return _item; };
	IW::CShellFolder& GetShellFolder() { return _pFolder; } 
	const IW::CShellFolder& GetShellFolder() const { return _pFolder; } 
	void SetShellFolder(IShellFolder *pFolder) {  _pFolder = pFolder; };


	// Get text attributes
	bool GetFormatText(CString &strOut, CArrayDWORD &array, bool bFormat) const;
	bool GetFormatText(CSimpleArray<CString> &arrayStrOut, CArrayDWORD &array, bool bFormat) const;
	CString GetStatistics() const;
	CString GetType() const;
	const IW::FileSize &GetFileSize() const { return _sizeFile; };
	const IW::FileTime &GetLastWriteTime() const { return _ftLastWriteTime; };
	const IW::FileTime &GetCreatedTime() const { return _ftCreationTime; };
	CString GetToolTip() const;
	bool IsHTMLDisplayable(PluginState &plugins) const;
	bool IsSelected() const;
	int GetImageNum() const { return _nImage; };

	HRESULT BindToObject(int nItem, REFIID riid, void **ppv); 
	HRESULT BindToStorage(int nItem, REFIID riid, void **ppv) const;
	HRESULT GetUIObjectOf(int nItem, REFIID riid, void **ppv);

	void GetAttributes(FolderItemAttributes &attributes, bool bGetImage = false) const;
	const Image &GetImage() const { return _image; };
	void SetImage(const Image &image) { _image = image; if (!image.IsEmpty()) _uFlags |= THUMB_IMAGE; };
	const IW::CameraSettings &GetCameraSettings() const { return _image.GetCameraSettings(); };

	bool SaveAsImage(CLoadAny &loader, IW::Image&, const CString &strFilterName, IW::IStatus *pStatus) const;
	TAGSET GetTags() const;	

	bool Write(IW::CFile &f) const;
	bool Read(IW::CFile &f, unsigned nFileVersion);

	void ModifyFlags(DWORD dwRemove, DWORD dwAdd);

	static void selectItem(IW::FolderItem *pThumb);
	static void selectInverse(IW::FolderItem *pThumb);
	static void selectIfImage(IW::FolderItem *pThumb);
	static void clearLoadedFlag(IW::FolderItem *pThumb);
	static bool isSelected(const IW::FolderItem *pThumb);
	static bool isNotSelected(const IW::FolderItem *pThumb);
	static bool isDeleted(const IW::FolderItem *pThumb);
	static bool isNotDeleted(const IW::FolderItem *pThumb);
	static bool isLoaded(const IW::FolderItem *pThumb);
	static bool isImage(const IW::FolderItem *pThumb);
	static bool isImageIcon(const IW::FolderItem *pThumb);
	static bool isFolder(const IW::FolderItem *pThumb);
	static bool isLink(const IW::FolderItem *pThumb);
	static bool isFileSystem(const IW::FolderItem *pThumb);
	static void releaseItem(IW::FolderItem *pThumb);

	static int CompareName(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b);
	static int CompareType(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b);
	static int CompareSize(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b);
	static int CompareCreationTime(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b);
	static int CompareDateTaken(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b);
	static int CompareLastWriteTime(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b);

	friend class Folder;
	friend class FolderItemLoader;
	friend class ::TestSearch;
	friend class ::FolderItem200;
	friend class ::FolderItem105;
};

////////////////////////////////////////////////////////////////////////////////
////
//// FolderItemLoader
////

class FolderItemLoader
{
private:
	FolderItemLoader(const IW::FolderItemLoader &other);
	void operator=(const IW::FolderItemLoader &other);

protected:

	ThumbnailCache &_cache;
	FolderItem _item;
	CString _strFilePath;

	bool _bLoadedImage;
	bool _bLoadedIcon;
	bool _bDidStartLoad;
	int _nSubItemCount;
	const CSize _sizeThumbnail;

public:

	FolderItemLoader(ThumbnailCache &cache);
	FolderItemLoader(ThumbnailCache &cache, const CSize &sizeThumbnail);
	FolderItemLoader(ThumbnailCache &cache, IW::FolderItem &item);	
	~FolderItemLoader();

	bool IsImageLoaded() const { return _bLoadedImage; };
	bool SyncThumb(IW::FolderItem &item);

	CString GetFilePath() { return _strFilePath; };


	bool LoadImage(CLoadAny *pLoader, const Search::Spec &spec, bool bForceLoad = false);
	void LoadFolderImage(CLoadAny *pLoader, bool bForceLoad = false);
	void RenderAndScale();
    
	friend class Folder;
	friend class FolderItem;
};

////////////////////////////////////////////////////////////////////////////////
////
//// Folder
////



class Folder : public IW::Referenced
{
public:
	Folder();
	~Folder();

	bool Init(const IW::CShellItem &itemFull, IW::CShellFolder &pFolder);
	bool Init(const IW::CShellItem &itemFull, IW::CShellFolder &pFolder, const IW::CShellItem &item);
	bool Init(const CString &strFilePath);
	bool Init(const IW::CShellItem &itemFull);
	bool Init(IW::CShellFolder &pShellFolder);

	int Find(const IW::CShellItem &item) const;
	int Find(const CString &strFileName) const;

	void InsertThumb(IW::FolderItem *pThumb);

	//bool GetPath(LPTSTR szPathOut, const CString &strFile, const CString &strFileExt) const;
	void DeleteAllThumbs();

	// Focus
	int WriteFocus();
	int ReadFocus();
	
	// Item sorting
	void Sort(int nSortOrder, bool bAssending);
	void Sort(ITEMLIST &thumbs, int nSortOrder, bool bAssending);

	int LoadCacheFile(CSize &sizeThumbImageCurrent, volatile bool &bAbortDecoding);
	int LoadCacheFile(MAPNAMETOTHUMB &mapThumbs, const CString &strFileName, CSize &sizeThumbImageCurrent, volatile bool &bAbortDecoding);
	int LoadCacheFile105(MAPNAMETOTHUMB &mapThumbs, const CString &strFileName, CSize &sizeThumbImageCurrent, volatile bool &bAbortDecoding);
	bool SaveCacheFile(const CString &strCacheFileName, const CSize &sizeThumbImageCurrent);

	TAGMAP GetTags() const;
	
	// Item refreshing
	HRESULT Refresh(HWND hwnd);
		
	// Item Access
	int GetSize() const 
	{ 
		IW::CAutoLockCS lock(_cs);
		return _thumbs.size(); 
	};	

	const IW::CShellItem &GetFolderItem() const { return _item; };	
	bool GetParentItem(IW::CShellItem &item) const;
	IW::CShellFolder& GetShellFolder() { return _pFolder; } 
	const IW::CShellFolder& GetShellFolder() const { return _pFolder; } 
	void SetShellFolder(IShellFolder *pFolder) {  _pFolder = pFolder; };


	// Selection of items
	bool HasSelection() const { return _bHasSelection; };
	void UpdateSelectedItems();
	CString GetSelectedFileList() const;

	bool GetAttributesOfSelectedItems(ULONG ulAttribs) const { return (ulAttribs & GetSelectionAttributes()) != 0; };
	UINT GetSelectionAttributes() const;
	
	// Shell Folder Functionality
	HRESULT GetSelectedUIObjectOf(HWND hwnd, REFIID riid, UINT * prgfInOut, void **ppv);
	HRESULT CreateViewObject(HWND hwnd, REFIID riid, void **ppv);

	// For Items
	HRESULT BindToObject(int nItem, REFIID riid, void **ppv); 
	HRESULT BindToStorage(int nItem, REFIID riid, void **ppv) const;
	HRESULT GetUIObjectOf(int nItem, REFIID riid, void **ppv);
	CString GetToolTip(int nItem) const;

	void LoadJobEnd(FolderItemLoader &loader, FolderItemPtr pItem);

	ITEMLIST GetItemList() const
	{
		IW::CAutoLockCS lock(_cs);
		return _thumbs;
	}

	// Properties
	//bool IterateProperties(int nItem, IPropertyStream *pStreamOut) const;
 
	// Item Iteration
	template<class THandeler>
	bool IterateItems(THandeler *pItemHandeler, IW::IStatus *pStatus)
	{
		ITEMLIST items;

		{
			IW::CAutoLockCS lock(_cs);
			CString strFolderMessage;
			strFolderMessage.Format(IDS_PROCESSING_FOLDER_FMT, GetFolderName());
			pStatus->SetHighLevelStatusMessage(strFolderMessage);		
			items = _thumbs;
		}

		return IterateItemList(items, pItemHandeler, pStatus);
	}

	template<class THandeler>
	bool IterateSelectedItems(THandeler *pItemHandeler, IW::IStatus *pStatus)
	{		
		ITEMLIST items;

		{
			IW::CAutoLockCS lock(_cs);
			CString strFolderMessage;
			strFolderMessage.Format(IDS_PROCESSING_FOLDER_FMT, GetFolderName());
			pStatus->SetHighLevelStatusMessage(strFolderMessage);		

			for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)      
			{
				IW::FolderItemPtr pThumb = *i;

				if (pThumb->IsSelected())
				{
					items.push_back(pThumb);
				}
			}
		}

		return IterateItemList(items, pItemHandeler, pStatus);
	}

	template<class THandeler>
	static bool IterateItemList(ITEMLIST &items, THandeler *pItemHandeler, IW::IStatus *pStatus)
	{
		HRESULT hr = S_OK;
		int nCount = 0;		

		for(ITEMLIST::iterator i = items.begin(); i != items.end(); ++i)       
		{
			IW::FolderItemPtr pThumb = *i;

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

			pStatus->SetHighLevelProgress(nCount, items.size());

			if (pStatus->QueryCancel())
			{
				pStatus->SetError(App.LoadString(IDS_CANCELED));
				return false;
			}

			nCount++;
		}

		return true;
	}

	template<class THandeler>
	void IterateSelectedThumbs(THandeler &handeler)
	{
		IW::CAutoLockCS lock(_cs);

		for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)      
		{
			IW::FolderItemPtr pThumb = (*i);

			if (pThumb->IsSelected())
			{
				handeler.AddItem(pThumb);
			}
		}
	}


	CString GetFolderName() const;
	CString GetFolderPath() const;

	// Methods to handel selectability
	long GetSelectedItemCount() const;
	long GetItemCount() const;
	bool GetParentFolder(Folder **);

	// The name
	CString GetItemName(int nItem) const;
	CString GetItemPath(int nItem) const;

	bool IsItemFolder(int nItem) const
	{
		IW::CAutoLockCS lock(_cs);
		return _thumbs[nItem]->IsFolder();
	}

	void SetLoadItem(int n)
	{
		_nLoadItem = n;
	}

	IW::FolderItemPtr GetNextThumbToLoad(IW::Folder *pFolder)
	{
		IW::CAutoLockCS lock(_cs);
		int nCount = pFolder->GetSize();

		// Try focus item first?
		int nFocusItem = GetFocusItem();

		if (nFocusItem != -1 &&
			nFocusItem < nCount &&
			CanLoad(nFocusItem))
		{
			return _thumbs[_nLoadItem = nFocusItem];
		}

		for(int i = 0; i < nCount; i++)   
		{
			int nThumb = (_nLoadItem + i) % nCount;

			if (CanLoad(nThumb))
			{
				return _thumbs[_nLoadItem = nThumb];
			}
		}

		return 0;
	}

	bool CanLoad(int nItem) const;
	bool IsItemBrowsable(int nItem) const;
	bool IsItemDropTarget(int nItem) const;
	bool IsItemImage(int nItem) const;
	void GetItemImage(int nItem, IW::Image &imageOut) const;
	int GetItemImageNum(int nItem) const;
	bool IsItemMultiPageImage(int nItem) const;
	bool IsItemAnimatedImage(int nItem) const;
	bool AnimationStep(int nItem);
	bool IsItemSelected(int nItem) const;
	UINT GetItemFlags(int nItem) const;	
	int GetItemFrame(int nItem) const;
	UINT GetItemAttribs(int nItem) const;
	CRect GetItemThumbRect(int nItem) const;
	void GetCameraSettings(int nItem, IW::CameraSettings &settings) const;
	void SetItemFlags(int nItem, UINT uFlags);
	void ModifyItemFlags(int nItem, DWORD dwRemove, DWORD dwAdd);
	void ResetLoadedFlag();
	int CalcLoadedCount();
	int CalcImageCount();
	int GetFocusItem() const;
	void SetFocusItem(int nFocusItem);
	int GetImageCount() const;
	void InvalidateThumb(int nItem);
	int GetPercentComplete() const;
	int GetTimeRemaining() const;
	int GetTimeTaken() const;
	void ResetCounters();

	// Selection Helpers
	void Select(int n, UINT nFlags);
	void SelectAll();
	void SelectInverse();
	void SelectImages();
	void SelectTag(const CString &strTag);
	void GetSelectStatus(int &nCount, int &nImages, IW::FileSize &size) const;
	int GetSelectCount() const;
	void DragSelection(int nItem);

private:

	mutable CCriticalSection _cs;

	ITEMLIST _thumbs;
	bool _bHasSelection;
	int	_nFocusItem;
	int _nImageCount;	
	int _nLoadedCount;
	int _timeLast;	
	mutable int _timeRemaining;
	int _timeFirst;	
	int   _nLoadItem;

	IW::CShellDesktop _desktop;
	IW::CShellItem _item;
	//CComGITPtr<IShellFolder> _pFolder;
	IW::CShellFolder _pFolder;

public:


	friend class FolderItemLock;
};


class FolderItemLock : public FolderItemPtr
{
public:
	FolderItemLock(Folder *pFolder, int nItem) : 
		FolderItemPtr(pFolder->_thumbs[nItem])
	{
		
	}

	~FolderItemLock()
	{
	}
};



class FolderStreamOut : public CFileTemp
{
public:
	FolderStreamOut(const FolderItem *pItem)
	{
		CString strPath = pItem->GetFilePath();

		if (!CFileTemp::OpenForWrite(strPath))
		{
			throw std::exception();
		}
	}
};

class FolderStreamIn : public CFile
{
public:
	FolderStreamIn(const FolderItem *pItem)
	{
		CString strPath = pItem->GetFilePath();

		if (!CFile::OpenForRead(strPath))
		{
			throw std::exception();
		}
	}
};



////////////////////////////////////////////////////////////////////////////////
////
//// CShellSource
////



class CShellSource : public IW::IStreamIn
{
private:
	CString _path;
	HANDLE _hFile;	

	bool Open()
	{
		_hFile = ::CreateFile(_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		return _hFile != INVALID_HANDLE_VALUE;
	}

public:

	CShellSource(const CString &strNameIn) : _path(strNameIn), _hFile(INVALID_HANDLE_VALUE)
	{
		Open();
	}

	CShellSource(IW::CShellFolder &folder, const IW::CShellItem &item) : _hFile(INVALID_HANDLE_VALUE)
	{
		_path = folder.GetDisplayNameOf(item, SHGDN_FORPARSING);
		Open();
	}

	CShellSource(IW::Folder *pFolder, int nItem) : _hFile(INVALID_HANDLE_VALUE)
	{
		_path = pFolder->GetItemPath(nItem);
		Open();
	}

	CString GetFileName() { return _path; };

	DWORD Seek(ePosition ePos, LONG offset)
	{
		DWORD dwMoveMethod = FILE_BEGIN;

		/* we use this as a special code, so avoid accepting it */
		if( offset == 0xFFFFFFFF )
			return 0xFFFFFFFF;

		switch(ePos)
		{
		case IStreamIn::eCurrent:
			dwMoveMethod = FILE_CURRENT;
			break;

		case IStreamIn::eEnd:
			dwMoveMethod = FILE_END;
			break;

		default:
			dwMoveMethod = FILE_BEGIN;
			break;
		}

		LONG lowerBits = (LONG)(offset & 0xffffffff);

		return ::SetFilePointer(_hFile, lowerBits, NULL, dwMoveMethod);
	};


	bool Flush()
	{
		if (_hFile == INVALID_HANDLE_VALUE)
			return false;

		return ::FlushFileBuffers(_hFile) != 0;
	}

	bool Close(IStatus *pStatus = IW::CNullStatus::Instance)
	{
		BOOL bError = FALSE;

		if (_hFile != INVALID_HANDLE_VALUE)
		{
			bError = !::CloseHandle(_hFile);
			_hFile =  INVALID_HANDLE_VALUE;
		}

		if (bError)
			std::exception("Failed to clode file");

		return true;
	}

	bool Abort()
	{
		if (_hFile != INVALID_HANDLE_VALUE)
		{
			// close but ignore errors
			::CloseHandle(_hFile);
			_hFile = INVALID_HANDLE_VALUE;
		}

		return true;
	}

	IW::FileSize GetFileSize()
	{
		return IW::FileSize::FromHandle(_hFile);
	}
	
	bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0)
	{
		if (nCount == 0) return true;   // avoid Win32 "null-read"
		assert(lpBuf != NULL);

		DWORD dw;
		if (pdwRead == 0) pdwRead = &dw;
		return ::ReadFile(_hFile, lpBuf, nCount, pdwRead, NULL) != 0;
	}

	~CShellSource()
	{
		Close();
	}
};

	


}; // namespace IW
