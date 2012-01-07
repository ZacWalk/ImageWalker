///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Thumb.cpp: implementation of the IW::CFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Items.h"
#include "ImageStreams.h"
#include "LoadAny.h"
#include "ProgressDlg.h"
#include "ImageMetaData.h"


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



void IW::FolderItem::selectItem(IW::FolderItem *pThumb)
{
	pThumb->_uFlags |= THUMB_SELECTED;
}

void IW::FolderItem::selectInverse(IW::FolderItem *pThumb)
{
	pThumb->_uFlags ^= THUMB_SELECTED;
}

void IW::FolderItem::selectIfImage(IW::FolderItem *pThumb)
{
	if (pThumb->IsImage())
	{
		pThumb->_uFlags |= THUMB_SELECTED;
	}
	else
	{
		pThumb->_uFlags &= ~THUMB_SELECTED;
	}
}

void IW::FolderItem::selectIfImageOnFlickr(IW::FolderItem *pThumb)
{
	if (pThumb->HasFlickrUrl())
	{
		pThumb->_uFlags |= THUMB_SELECTED;
	}
	else
	{
		pThumb->_uFlags &= ~THUMB_SELECTED;
	}
}

void IW::FolderItem::clearLoadedFlag(IW::FolderItem *pThumb)
{
	pThumb->_uFlags &= ~THUMB_LOADED;
}

bool IW::FolderItem::isSelected(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_SELECTED) != 0;
}

bool IW::FolderItem::isNotSelected(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_SELECTED) == 0;
}

bool IW::FolderItem::isDeleted(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_DELETE) != 0;
}

bool IW::FolderItem::isNotDeleted(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_DELETE) == 0;
}

bool IW::FolderItem::isLoaded(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_LOADED) != 0;
}

bool IW::FolderItem::isImage(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_IMAGE) != 0 && 
			(pThumb->_ulAttribs & SFGAO_FOLDER) == 0;
}

bool IW::FolderItem::isImageIcon(const IW::FolderItem *pThumb)
{
	return (pThumb->_uFlags & THUMB_IMAGE_ICON) != 0;
}

bool IW::FolderItem::isFolder(const IW::FolderItem *pThumb)
{
	return (pThumb->_ulAttribs & SFGAO_FOLDER) != 0;
}

bool IW::FolderItem::isLink(const IW::FolderItem *pThumb)
{
	return (pThumb->_ulAttribs & SFGAO_LINK) != 0;
}

bool IW::FolderItem::isFileSystem(const IW::FolderItem *pThumb)
{
	return (pThumb->_ulAttribs & SFGAO_FILESYSTEM) != 0;
}

void IW::FolderItem::releaseItem(IW::FolderItem *pThumb)
{
	pThumb->Release();
}

int IW::FolderItem::CompareName(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b)
{
	HRESULT hr = pShellFolder->CompareIDs(0, a->_item, b->_item);
	ATLASSERT(SUCCEEDED(hr));
	return static_cast<short>(SCODE_CODE(hr));
}

int IW::FolderItem::CompareType(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b)
{
	int i = a->_uExtension - b->_uExtension;
	if ((i == 0) || (a->_ulAttribs & SFGAO_FOLDER) || (b->_ulAttribs & SFGAO_FOLDER)) i = CompareName(pShellFolder, a, b);		
	return i;
}

int IW::FolderItem::CompareSize(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b)
{
	int i = a->_sizeFile - b->_sizeFile;
	if ((i == 0) || (a->_ulAttribs & SFGAO_FOLDER) || (b->_ulAttribs & SFGAO_FOLDER)) i = CompareName(pShellFolder, a, b);		
	return i;
}

int IW::FolderItem::CompareCreationTime(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b)
{
	int i = a->_ftCreationTime.Compare(b->_ftCreationTime);		
	if ((i == 0) || (a->_ulAttribs & SFGAO_FOLDER) || (b->_ulAttribs & SFGAO_FOLDER)) i = CompareName(pShellFolder, a, b);		
	return i;
}

int IW::FolderItem::CompareDateTaken(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b)
{
	int i = a->GetTakenTime().Compare(b->GetTakenTime());		
	if ((i == 0) || (a->_ulAttribs & SFGAO_FOLDER) || (b->_ulAttribs & SFGAO_FOLDER)) i = CompareName(pShellFolder, a, b);		
	return i;
}

int IW::FolderItem::CompareLastWriteTime(IW::CShellFolder &pShellFolder, const IW::FolderItem *a, const IW::FolderItem *b)
{
	int i = a->_ftLastWriteTime.Compare(b->_ftLastWriteTime);		
	if ((i == 0) || (a->_ulAttribs & SFGAO_FOLDER) || (b->_ulAttribs & SFGAO_FOLDER)) i = CompareName(pShellFolder, a, b);		
	return i;
}

////////////////////////////////////////////////////////////
/// Thumbs

IW::FolderItem::FolderItem() :// Variable defaults.
	_ulAttribs(0),
	_uExtension(0),
	_uFlags(0),
	_nImage(-1),
	_nTimer(0),
	_nFrame(0),
	_dwFileAttributes(0),
	_nCachedThumbnailId(-1),
	_nSubItemCount(0)
{
	static DWORD dwHashCodeFolderItem = 1;
	_dwHashCode = dwHashCodeFolderItem++;
}

bool IW::FolderItem::Init(const CString &strFilePath)
{
	IW::CShellItem itemFull;
	itemFull.Open(strFilePath);

	if (itemFull.Depth() > 1)
	{
		IW::CShellItem itemFolder(itemFull);
		itemFolder.StripToParent();

		IW::CShellItem itemTail(itemFull);
		itemTail.StripToTail();

		IW::CShellFolder pShellFolder;
		pShellFolder.Open(itemFolder, true);

		return Init(pShellFolder, itemTail.Detach());
	}
	else
	{
		return Init(CShellDesktop(), itemFull.Detach());
	}
}

bool IW::FolderItem::Init(IShellFolder *pFolder, LPITEMIDLIST pItem, LPCITEMIDLIST pItemGap) 
{
	_item.Attach(pItem);
	if (pItemGap) _itemGap = pItemGap;
	_pFolder = pFolder;

	ATLASSERT(_item.Depth() == 1);
	
	return SUCCEEDED(RefreshItem());
}

void IW::FolderItem::ModifyFlags(DWORD dwRemove, DWORD dwAdd)
{
	_uFlags = (_uFlags & ~dwRemove) | dwAdd;
}

HRESULT IW::FolderItem::RefreshItem()
{
	DWORD dwRemovable = SFGAO_REMOVABLE;
	HRESULT hr = _pFolder->GetAttributesOf(1, _item, &dwRemovable);

	if( SUCCEEDED(hr) && dwRemovable & SFGAO_REMOVABLE )
	{
		_ulAttribs = SFGAO_REMOVABLE | SFGAO_FOLDER;
	}
	else
	{
		_ulAttribs = SFGAO_LINK | SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_GHOSTED;		
		hr = _pFolder->GetAttributesOf(1, _item, &_ulAttribs);
	
		if (FAILED(hr))
		{
			_ulAttribs = 0;
		}
	}
	
	if (IsFileSystem())
	{
		WIN32_FIND_DATA findFileData;
		
		
		HRESULT hr = SHGetDataFromIDList(_pFolder, _item, SHGDFIL_FINDDATA, &findFileData, sizeof(findFileData));		
		
		if (SUCCEEDED(hr))
		{
			_ftLastWriteTime = findFileData.ftLastWriteTime;
			_ftCreationTime = findFileData.ftCreationTime;
			_sizeFile = IW::FileSize(findFileData.nFileSizeHigh, findFileData.nFileSizeLow);
			_dwFileAttributes = findFileData.dwFileAttributes;
		}

		if (_ftCreationTime.IsEmpty() || _ftLastWriteTime.IsEmpty())
		{
			WIN32_FIND_DATA ffd;
			HANDLE sh = FindFirstFile(GetFilePath(), &ffd);

			if(INVALID_HANDLE_VALUE != sh)
			{
				_ftCreationTime = ffd.ftCreationTime;
				_ftLastWriteTime = ffd.ftLastWriteTime;
				FindClose(sh);
			}
		}
	}

	_uExtension = App.GetExtensionKey(GetFileName());
	
	return hr;
}

IW::FolderItem::FolderItem(const FolderItem &other) : 
	_item(other._item),
	_dwHashCode(other._dwHashCode),
	_ftCreationTime(other._ftCreationTime), 
	_ftLastWriteTime(other._ftLastWriteTime), 
	_sizeFile(other._sizeFile),
	_dwFileAttributes(other._dwFileAttributes),
	_ulAttribs(other._ulAttribs),
	_image(other._image),
	_uExtension(other._uExtension),
	_nFrame(other._nFrame),
	_nImage(other._nImage),
	_nTimer(other._nTimer),
	_uFlags(other._uFlags),
	_pFolder(other._pFolder),
	_nCachedThumbnailId(other._nCachedThumbnailId),
	_nSubItemCount(other._nSubItemCount)
{
}

IW::FolderItem::~FolderItem() 
{  
	_pFolder.Release();
}

void IW::FolderItem::operator =(const FolderItem &other)
{
	_dwHashCode = other._dwHashCode;
	_ftCreationTime = other._ftCreationTime; 
	_ftLastWriteTime = other._ftLastWriteTime; 
	_sizeFile = other._sizeFile;
	_dwFileAttributes = other._dwFileAttributes;
	_ulAttribs = other._ulAttribs;
	_image = other._image;
	_uExtension = other._uExtension;
	_uFlags = other._uFlags;
	_nImage = other._nImage;
	_nTimer = other._nTimer;
	_nFrame = other._nFrame;
	_pFolder = other._pFolder;
	_item = other._item;
	_nCachedThumbnailId = other._nCachedThumbnailId;
	_nSubItemCount = other._nSubItemCount;
}

bool IW::FolderItem::IsFolder() const { return isFolder(this); };
bool IW::FolderItem::IsImage() const { return isImage(this); };
bool IW::FolderItem::IsLink() const { return isLink(this); };
bool IW::FolderItem::IsFileSystem() const { return isFileSystem(this); };
bool IW::FolderItem::IsImageIcon() const { return isImageIcon(this); };

IW::FolderPtr IW::FolderItem::GetFolder()
{
	CShellItem item = _item;

	if (!_itemGap.IsNull())
	{
		item.Cat(_itemGap, _item);
	}	

	FolderPtr pFolder = new IW::RefObj<IW::Folder>;	

	if (!IsZipFile())
	{
		if (IsLink())
		{
			CShellItem itemLink;

			if (SUCCEEDED(_pFolder.ResolveLink(item, itemLink)))
			{
				if (pFolder->Init(itemLink))
				{
					return pFolder;
				}
			}
		}
		else
		{
			IW::CShellFolder pShellFolder;

			if (SUCCEEDED(_pFolder->BindToObject(item, NULL, IID_IShellFolder, (LPVOID*)pShellFolder.GetPtr())))
			{
				if (pFolder->Init(pShellFolder))
				{
					return pFolder;
				}
			}
		}
	}

	return 0;
}

CString IW::FolderItem::GetToolTip() const
{
	IW::RefPtr<IQueryInfo> spInfo;
	HRESULT	hr = E_FAIL;
	CString strToolTip;

	if (!isImage(this))
	{
		hr = _pFolder->GetUIObjectOf(IW::GetMainWindow(), 1, _item, IID_IQueryInfo, 0, (LPVOID *)&spInfo);

		if (SUCCEEDED(hr))
		{
			WCHAR *wsz = 0;
			if (SUCCEEDED(spInfo->GetInfoTip(0, &wsz)))
			{
				strToolTip = wsz; 

				ATL::CComPtr<IMalloc> spMalloc;
				SHGetMalloc(&spMalloc);

				if (spMalloc)
					spMalloc->Free(wsz);
			}
		}
	}
	else
	{
		CSimpleArray<CString> arrayStrOut;
		CArrayDWORD array;

		array.Add(ePropertyTitle);
		array.Add(ePropertyObjectName);
		array.Add(ePropertyType);
		array.Add(ePropertyDateTaken);
		array.Add(ePropertyModifiedDate);
		array.Add(ePropertySize);
		array.Add(ePropertyAperture);
		array.Add(ePropertyIsoSpeed);
		array.Add(ePropertyWhiteBalance);
		array.Add(ePropertyExposureTime);
		array.Add(ePropertyFocalLength);		
		array.Add(ePropertyDescription); 

		// Get the strings
		GetFormatText(arrayStrOut, array, true);

		for(int i = 0; i < arrayStrOut.GetSize(); i++)
		{
			CString &str = arrayStrOut[i];

			if (!str.IsEmpty())
			{
				if (!strToolTip.IsEmpty()) strToolTip += g_szCRLF;

				// Dont title the description
				if (IW::ePropertyDescription != array[i])
				{
					strToolTip += App.GetMetaDataTitle(array[i]);
					strToolTip += ": ";
				}

				strToolTip += str;
			}
		}
	}

	return strToolTip;
}

IW::Image IW::FolderItem::OpenAsImage(CLoadAny &loader, IW::IStatus *pStatus)
{
	Image image;
	IW::ImageStream<IW::IImageStream> imageOut(image);

	if (!loader.LoadImage(GetFilePath(), &imageOut, pStatus))
	{
		image.Free();
	}

	return image;
}

IW::FolderItemPtr IW::FolderItem::CreateTestItem()
{
	IW::FolderItemPtr pItem = new IW::FolderItem();

	pItem->_item.Open(IW::GetMainWindow(), CSIDL_PERSONAL);
	pItem->_pFolder = IW::CShellDesktop();
	pItem->_image.CreatePage(10, 10, IW::PixelFormat::PF24);
	pItem->_uFlags |= THUMB_IMAGE;

	return pItem;
}


bool IW::FolderItem::SaveAsImage(CLoadAny &loader, IW::Image &image, const CString &strFilterName, IW::IStatus *pStatus) const
{	
	IW::RefPtr<IW::IImageLoader> pLoader = loader.GetLoader(strFilterName);
	IW::FolderStreamOut streamOut(this);
	return pLoader->Write(loader.GetExtensionDefault(strFilterName), &streamOut, image, pStatus);
}

IW::TAGSET IW::FolderItem::GetTags() const
{
	if (IsImage())
	{
		return IW::Split(_image.GetTags());
	}

	return IW::TAGSET();
}

bool IW::FolderItem::LoadJobBegin(FolderItemLoader &loader)
{
	if (_uFlags & THUMB_LOADED)
	{
		return false;
	}

	loader._item = *this;
	loader._strFilePath = GetFilePath();
	_uFlags |= THUMB_LOADING;

	return true;
}

bool IW::FolderItem::IsHTMLDisplayable(PluginState &plugins) const
{
	if (!IsFolder())
	{
		CString strKey;

		bool bHasImage = IsImage() && !_image.IsEmpty();
		if (bHasImage) strKey = _image.GetLoaderName();

		if (strKey.IsEmpty())
		{
			strKey = IW::Path::FindExtension(GetFileName());
		}

		IImageLoaderFactoryPtr pFactory = plugins.GetImageLoaderFactory(strKey);
		return pFactory && (IW::ImageLoaderFlags::HTML & pFactory->GetFlags());
	}

	return false;
}

bool IW::FolderItem::SetItemName(const CString &strNewName, DWORD uFlags)
{
	bool bRet = false;

	USES_CONVERSION;
	LPITEMIDLIST pidlOut;
	HRESULT hr = _pFolder->SetNameOf(IW::GetMainWindow(), _item, T2COLE(strNewName), uFlags, &pidlOut);

	if (SUCCEEDED(hr))
	{
		_item.Attach(pidlOut);
	}

	if (SUCCEEDED(hr))
	{		
		_uExtension = App.GetExtensionKey(GetFileName());
		bRet = true;
	}

	return bRet;
}

bool IW::FolderItem::IsReadOnly() const
{
	return 0 != (_dwFileAttributes & FILE_ATTRIBUTE_READONLY);
}

bool IW::FolderItem::CanRename() const
{
	return 0 != _pFolder.GetAttributes(_item, SFGAO_CANRENAME);
}

bool IW::FolderItem::IsSelected()const
{
	return isSelected(this);
}


void IW::FolderItem::LoadJobEnd(FolderItemLoader &loader)
{
	if (loader._bDidStartLoad)
	{
		if (loader._item._dwHashCode == _dwHashCode)
		{
			loader.SyncThumb(*this);
			_uFlags |= THUMB_INVALIDATE;
		}
	}
}

void IW::Folder::LoadJobEnd(FolderItemLoader &loader, FolderItemPtr pItem)
{
	if (loader._bDidStartLoad)
	{
		if (loader._item._dwHashCode == pItem->_dwHashCode)
		{
			bool bAlreadyImage = pItem->IsImage();
			pItem->LoadJobEnd(loader);			

			if (loader.IsImageLoaded())
			{
				if (!bAlreadyImage)
				{
					_nImageCount++;				
				}			
			}

			_nLoadedCount++;
			_timeLast = GetTickCount();
		}
	}
}

void IW::FolderItem::GetAttributes(IW::FolderItemAttributes &attributes, bool bGetImage) const
{
	DWORD dwImageFlags = _image.GetFlags();

	attributes.bIsImage = IsImage();
	attributes.bIsImageIcon = IsImageIcon();
	attributes.bIsFolder = IsFolder();
	attributes.nImage = _nImage;
	attributes.nFrame = _nFrame;
	attributes.nSubItemCount =_nSubItemCount;
	attributes.bHasIPTC = 0 != (dwImageFlags & ImageFlags::HasIPTC);
	attributes.bHasXmp = 0 != (dwImageFlags & ImageFlags::HasXmp);
	attributes.bHasExif = 0 != (dwImageFlags & ImageFlags::HasExif);
	attributes.bHasICC = 0 != (dwImageFlags & ImageFlags::HasICC);
	attributes.bHasError = 0 != (dwImageFlags & ImageFlags::HasError);
	attributes.nPageCount = _image.GetPageCount();
	attributes.bIsLink = (_ulAttribs & SFGAO_LINK) != 0;
	attributes.bIsHidden = (_ulAttribs & SFGAO_GHOSTED) != 0;
	attributes.bHasFlickrUrl = HasFlickrUrl();

	if (bGetImage)
	{
		attributes.image = _image;
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

inline bool IsAllowedType(UINT uExtension)
{
	IW::CAutoLockCS lock(App._cs);

	// Ingnor some common known
	// non image extensions of imgae formats
	// that are to slow
	static std::set<UINT> set;

	if (set.size() == 0)
	{
		set.insert(App.GetExtensionKey(_T(".EXE")));
		set.insert(App.GetExtensionKey(_T(".SCR")));
		set.insert(App.GetExtensionKey(_T(".DLL")));
	}

	return set.find(uExtension) == set.end();
};

inline bool IsFolderWalkImageType(UINT uExtension)
{
	IW::CAutoLockCS lock(App._cs);
	static std::set<UINT> set;

	if (set.size() == 0)
	{
		set.insert(App.GetExtensionKey(_T(".JPG")));
		set.insert(App.GetExtensionKey(_T(".TIF")));
		set.insert(App.GetExtensionKey(_T(".GIF")));
		set.insert(App.GetExtensionKey(_T(".BMP")));
		set.insert(App.GetExtensionKey(_T(".PNG")));
	}

	return set.find(uExtension) != set.end();
};



CString IW::FolderItem::GetStatistics() const
{
	if (IsFolder())
	{
		CString str;
		str.Format(_T("%d Items"), _nSubItemCount);
		return str;
	}

	return _image.GetStatistics();
}



CString IW::FolderItem::GetType() const
{
	CString str = GetStatistics();

	if (str.IsEmpty())
	{
		CString strPath = GetFilePath();

		SHFILEINFO    sfi;
		IW::MemZero(&sfi, sizeof(sfi));

		SHGetFileInfo(strPath,
			0,
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_TYPENAME |
			SHGFI_DISPLAYNAME);

		str = sfi.szTypeName;
	}

	return str;
}


bool IW::FolderItem::GetFormatText(CString &strOut, CArrayDWORD &array, bool bFormat) const
{
	strOut = g_szEmptyString;
	CSimpleArray<CString> arrayStr;

	if (!GetFormatText(arrayStr, array, bFormat))
	{
		return false;
	}

	for(int i = 0; i < arrayStr.GetSize(); ++i)
	{
		CString &str = arrayStr[i];

		if (!str.IsEmpty())
		{
			if (!strOut.IsEmpty())
			{
				strOut += g_szCRLF;
			}

			strOut += str;			
		}
	}

	return true;
}



bool IW::FolderItem::GetFormatText(CSimpleArray<CString> &arrayStrOut, CArrayDWORD &array, bool bFormat) const
{
	IW::CameraSettings cameraSettings = _image.GetCameraSettings();
	CString strPath;
	int nCount = array.GetSize();

	for(int i = 0; i < nCount; ++i)
	{
		int nItem = array[i];
		int nIdent = LOWORD(nItem);
		CString str;

		switch(nIdent)
		{
		case ePropertyTitle:
			str = _image.GetTitle();
			if (str.IsEmpty()) str = GetDisplayName();
			break;
		
		case ePropertyObjectName:
			str = _image.GetObjectName();
			break;

		case ePropertyName:
			str = GetDisplayPath(); 
			if (str.IsEmpty()) str = App.LoadString(IDS_UNKNOWNFILENAME);
			break;

		case ePropertyType:
			str = GetType();
			if (str.IsEmpty()) str = App.LoadString(IDS_UNKNOWNTYPE);
			break;

		case ePropertySize:
			str = GetFileSize().ToString();
			break;

		case ePropertyModifiedDate:
			str = GetLastWriteTime().ToLocalTime().GetDateFormat(App.GetLangId(), App.Options.m_bShortDates);
			break;

		case ePropertyModifiedTime:
			str = GetLastWriteTime().ToLocalTime().GetTimeFormat(App.GetLangId());
			break;

		case ePropertyCreatedDate:
			str = GetCreatedTime().ToLocalTime().GetDateFormat(App.GetLangId(),  App.Options.m_bShortDates);
			break;

		case ePropertyCreatedTime:
			str = GetCreatedTime().ToLocalTime().GetTimeFormat(App.GetLangId());
			break;

		case ePropertyPath:
			str = GetFilePath();
			if (str.IsEmpty()) str = App.LoadString(IDS_UNKNOWNFILEPATH);
			break;			

		case ePropertyWidth:
			str = IW::IToStr(cameraSettings.OriginalImageSize.cx);
			break;

		case ePropertyHeight:
			str = IW::IToStr(cameraSettings.OriginalImageSize.cy);
			break;

		case ePropertyDepth:
			str = cameraSettings.OriginalBpp.ToString();
			break;

		case ePropertyAperture:
			str = cameraSettings.FormatAperture();
			break;

		case ePropertyIsoSpeed:
			str = cameraSettings.FormatIsoSpeed();
			break;

		case ePropertyWhiteBalance:
			str = cameraSettings.FormatWhiteBalance();
			break;

		case ePropertyExposureTime:
			str = cameraSettings.FormatExposureTime();
			break;

		case ePropertyFocalLength:
			str = cameraSettings.FormatFocalLength();
			break;
		
		case ePropertyDescription:
			str = _image.GetDescription();
			break;

		case ePropertyDateTaken:
			{
				IW::FileTime taken = GetTakenTime().ToLocalTime();
				str = taken.GetDateFormat(App.GetLangId(), App.Options.m_bShortDates);
				str += _T(" ");
				str += taken.GetTimeFormat(App.GetLangId());
			}
			break;

		default:
			ATLASSERT(0); //Unknown entry
		}

		if  (arrayStrOut.GetSize() <= i) 
		{
			arrayStrOut.Add(str);
		}
		else
		{
			arrayStrOut[i] = str;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

IW::Folder::Folder() :
	_nFocusItem(-1),
	_nImageCount(0),
	_nLoadedCount(0),
	_timeFirst(0),
	_timeLast(0),
	_timeRemaining(INT_MAX),
	_bHasSelection(false),
	_nLoadItem(0)
{
	_item = IW::CShellDesktopItem();
}



IW::Folder::~Folder()
{
	DeleteAllThumbs();
	// Sanity check FinalRelease has been called?
	assert(_thumbs.size() == 0);
}

bool IW::Folder::Init(const CString &strFilePath)
{
	IW::CFilePath path(strFilePath);
	path.Normalize(true);
	path.MakeDosPath();

	IW::CShellItem item;	
	if (!item.Open(path))
	{
		return false;
	}

	return Init(item);
}

bool IW::Folder::Init(IW::CShellFolder &folder)
{
	return Init(folder.GetShellItem(), folder);
}

bool IW::Folder::Init(const IW::CShellItem &itemFull)
{
	if (itemFull.Depth() > 1)
	{
		IW::CShellItem itemFolder(itemFull);
		itemFolder.StripToParent();

		IW::CShellItem itemTail(itemFull);
		itemTail.StripToTail();

		IW::CShellFolder pShellFolder;
		HRESULT hr = pShellFolder.Open(itemFolder, true);
		if (FAILED(hr)) return false;

		return Init(itemFull, pShellFolder, itemTail);
	}

	return Init(itemFull, CShellDesktop(), itemFull);
}

bool IW::Folder::Init(const IW::CShellItem &itemFull, IW::CShellFolder &pFolder, const IW::CShellItem &item)
{
	HRESULT hr;

	// Connect to folder
	// Get the IEnumIDList object for the given folder.
	IW::CShellFolder pNewFolder;

	if (itemFull.IsDesktop())
	{
		pNewFolder = IW::CShellDesktop();
	}
	else
	{
		hr = pNewFolder.Open(pFolder, item, true);

		if (FAILED(hr)) 
		{
			return false;
		}
	}

	return Init(itemFull, pNewFolder);
}

bool IW::Folder::Init(const IW::CShellItem &itemFull, IW::CShellFolder &pNewFolder)
{
	{
		IW::CAutoLockCS lock(_cs);

		// Get the IEnumIDList object for the given folder.
		IW::CShellItemEnum enumitem;

		UINT dwFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
		if (App.Options.m_bShowHidden) dwFlags |= SHCONTF_INCLUDEHIDDEN;

		HRESULT hr = enumitem.Create(IW::GetMainWindow(), pNewFolder, dwFlags);

		if (FAILED(hr) || enumitem == NULL)
		{
			return false;
		}

		// Build up item map
		// Enumerate throught the list of items.	
		ITEMLIST thumbsNew;
		LPITEMIDLIST pItem = NULL;
		ULONG ulFetched = 0;

		while (enumitem->Next(1, &pItem, &ulFetched) == S_OK)
		{
			IW::FolderItemPtr pThumb = new IW::RefObj<IW::FolderItem>;

			if (!pThumb->Init(pNewFolder, pItem, 0))
				return false;

			int nIcon = App.GetIcon(pThumb->_uExtension);
			if (nIcon != -1) pThumb->_nImage = nIcon;
			pThumb->AddRef();
			thumbsNew.push_back(pThumb);
		}

		// Swap over
		_bHasSelection = false;
		_item = itemFull;

		SetShellFolder(pNewFolder);

		// Delete old thumbs and insert the new
		//std::for_each(_thumbs.begin(), _thumbs.end(), releaseItem);
		_thumbs = thumbsNew;
	}

	// update selected items
	UpdateSelectedItems();	

	return true;
}

class TagMapper
{
public:
	IW::TAGMAP &_tags;

	TagMapper(IW::TAGMAP &tags) : _tags(tags)
	{
	}

	TagMapper(const TagMapper &other) : _tags(other._tags)
	{
	}

	void operator()(const IW::FolderItem *pItem)
	{
		IW::TAGSET tagset = pItem->GetTags();

		for (IW::TAGSET::const_iterator it = tagset.begin(); it != tagset.end(); ++it)
		{
			_tags[*it]++;
		}
	}
};

IW::TAGMAP IW::Folder::GetTags() const
{
	IW::CAutoLockCS lock(_cs);

	IW::TAGMAP tags;
	TagMapper mapper(tags);
	
	std::for_each(_thumbs.begin(), _thumbs.end(), mapper);

	return tags;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


int IW::Folder::WriteFocus()
{
	IW::CAutoLockCS lock(_cs);

	int nFocusItem = GetFocusItem();

	if (nFocusItem != -1)
	{
		int nSize = GetSize();

		for(int i = 0; i < nSize; i++)
		{
			if (GetItemFlags(i) & THUMB_FOCUS)
			{
				ModifyItemFlags(i, THUMB_FOCUS, 0);
			}	
		}

		if(nFocusItem < nSize && nFocusItem >= 0)
		{
			ModifyItemFlags(nFocusItem, 0, THUMB_FOCUS);
		}
	}

	return nFocusItem;
}

int IW::Folder::ReadFocus()
{
	IW::CAutoLockCS lock(_cs);

	int nSize = GetSize();
	int nFocusItem = -1;

	for(int i = 0; i < nSize; i++)
	{
		if (GetItemFlags(i) & THUMB_FOCUS)
		{
			ModifyItemFlags(i, THUMB_FOCUS, 0);
			nFocusItem = i;
			break;
		}	
	}

	return nFocusItem;
}




HRESULT IW::Folder::GetSelectedUIObjectOf(HWND hwnd, REFIID riid, UINT * prgfInOut, void **ppv)
{
	IW::CAutoLockCS lock(_cs);

	int nCount = std::count_if(_thumbs.begin(), _thumbs.end(), FolderItem::isSelected);
	if (nCount == 0) return E_FAIL;
	LPCITEMIDLIST *items = static_cast<LPCITEMIDLIST*>(_alloca(sizeof(LPCITEMIDLIST) * (nCount + 1)));
	IW::copy_ref_if(_thumbs.begin(), _thumbs.end(), items, FolderItem::isSelected);

	return  GetShellFolder()->GetUIObjectOf(hwnd, nCount, items, riid, prgfInOut, ppv);
}


CString IW::Folder::GetToolTip(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->GetToolTip();
}

HRESULT IW::Folder::BindToStorage(int nItem, REFIID riid, void **ppv) const
{
	IW::CAutoLockCS lock(_cs);
	return _thumbs[nItem]->BindToStorage(nItem, riid, ppv);
}

HRESULT IW::FolderItem::BindToStorage(int nItem, REFIID riid, void **ppv) const
{
	if (_itemGap.IsNull())
	{
		return _pFolder->BindToObject(GetShellItem(), 0, riid, ppv);
	}

	IW::CShellItem item;
	item.Cat(_itemGap, _item);
	return _pFolder->BindToStorage(item, 0, riid, ppv);
}

HRESULT IW::Folder::BindToObject(int nItem, REFIID riid, void **ppv)
{
	IW::CAutoLockCS lock(_cs);
	return _thumbs[nItem]->BindToObject(nItem, riid, ppv);

}


HRESULT IW::FolderItem::BindToObject(int nItem, REFIID riid, void **ppv)
{
	if (_itemGap.IsNull())
	{
		return _pFolder->BindToObject(_item, 0, riid, ppv);
	}

	IW::CShellItem item;
	item.Cat(_itemGap, _item);
	return _pFolder->BindToObject(item, 0, riid, ppv);
}

HRESULT IW::Folder::GetUIObjectOf(int nItem, REFIID riid, void **ppv)
{
	IW::CAutoLockCS lock(_cs);
	return _thumbs[nItem]->GetUIObjectOf(nItem, riid, ppv);
}

HRESULT IW::FolderItem::GetUIObjectOf(int nItem, REFIID riid, void **ppv)
{
	if (_itemGap.IsNull())
	{
		return _pFolder->GetUIObjectOf(IW::GetMainWindow(), 1, _item,	riid, 0, ppv);
	}

	IW::CShellItem item;
	item.Cat(_itemGap, _item);
	return _pFolder->GetUIObjectOf(IW::GetMainWindow(), 1, item, riid, 0, ppv);
}



HRESULT IW::Folder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
	IW::CAutoLockCS lock(_cs);
	return GetShellFolder()->CreateViewObject(hwnd, riid, ppv);
}

class CShellItemCompare
{
protected:
	IShellFolder *_pFolder;
	const IW::CShellItem &_item;

public:

	CShellItemCompare(IShellFolder *pFolder, IW::CShellItem &item) : _pFolder(pFolder), _item(item)
	{
	}

	CShellItemCompare(IW::FolderItem *pThumb) : _pFolder(pThumb->GetShellFolder()), _item(pThumb->GetShellItem())
	{
	}

	CShellItemCompare(IW::FolderItemPtr pThumb) : _pFolder(pThumb->GetShellFolder()), _item(pThumb->GetShellItem())
	{
	}

	CShellItemCompare(const CShellItemCompare &sc) : _pFolder(sc._pFolder), _item(sc._item) 
	{
	};

	inline bool operator<(const CShellItemCompare &sc) const 
	{
		return _item.Compare(_pFolder, sc._item) < 0;
	}

private:

	void operator=(IW::FolderItem *pThumb) {};
	void operator=(const CShellItemCompare &sc) {};


};

HRESULT IW::Folder::Refresh(HWND hwnd) 
{	
	{
		IW::CAutoLockCS lock(_cs);

		SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, (LPCITEMIDLIST) _item, NULL);

		typedef std::map<CShellItemCompare, FolderItemPtr> MAPTHUMBS;
		MAPTHUMBS mapThumbs;

		for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
		{
			IW::FolderItem *pThumb = *i;
			mapThumbs[pThumb] = pThumb;
			pThumb->_uFlags |= THUMB_DELETE;
		}

		// Get the IEnumIDList object for the given folder.
		IW::CShellItemEnum enumitem;

		UINT dwFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
		if (App.Options.m_bShowHidden) dwFlags |= SHCONTF_INCLUDEHIDDEN;

		HRESULT hr = enumitem.Create(hwnd, GetShellFolder(), dwFlags);

		if (FAILED(hr))
		{
			// In this case we will probably
			// Want to rever to the parent item
			return hr;
		}

		// Build up item map
		// Enumerate throught the list of items.


		LPITEMIDLIST pItem = NULL;
		ULONG ulFetched = 0;

		while (enumitem->Next(1, &pItem, &ulFetched) == S_OK)
		{
			IW::FolderItemPtr pThumb = new IW::RefObj<IW::FolderItem>;

			if (!pThumb->Init(GetShellFolder(), pItem, 0))
				return false;		

			MAPTHUMBS::iterator thumbOld = mapThumbs.find(pThumb);

			if (thumbOld != mapThumbs.end())
			{
				IW::FolderItem *pFoundThumb = thumbOld->second;

				// If its not an image strip the
				// load status so we can reload it
				// OR if the image is been updated
				if ((pFoundThumb->_ftLastWriteTime != pThumb->_ftLastWriteTime) ||
					(pFoundThumb->_sizeFile != pThumb->_sizeFile))
				{
					pFoundThumb->_uFlags &= ~THUMB_LOADED;
				}

				// Copy over posibly refreshed values
				pFoundThumb->_ulAttribs = pThumb->_ulAttribs;
				pFoundThumb->_ftCreationTime = pThumb->_ftCreationTime;
				pFoundThumb->_ftLastWriteTime = pThumb->_ftLastWriteTime;
				pFoundThumb->_sizeFile = pThumb->_sizeFile;
				pFoundThumb->_dwFileAttributes = pThumb->_dwFileAttributes;

				// Mark for do not delete
				pFoundThumb->_uFlags &= ~THUMB_DELETE;
			}
			else
			{
				// New file!
				pThumb->AddRef();
				_thumbs.push_back(pThumb);
			}		
		}

		IW::ITEMLIST notDeleted;
		IW::copy_if(_thumbs.begin(), _thumbs.end(), std::inserter(notDeleted, notDeleted.end()), IW::FolderItem::isNotDeleted);
		_thumbs = notDeleted;

		mapThumbs.clear();

		// Reset timer
		_timeRemaining = INT_MAX;	

	}

	UpdateSelectedItems();

	return S_OK;
}



UINT IW::Folder::GetSelectionAttributes() const 
{
	IW::CAutoLockCS lock(_cs);

	int nCount = std::count_if(_thumbs.begin(), _thumbs.end(), FolderItem::isSelected);
	if (nCount == 0) return 0;

	LPCITEMIDLIST *items = static_cast<LPCITEMIDLIST*>(_alloca(sizeof(LPCITEMIDLIST) * (nCount + 1)));
	IW::copy_ref_if(_thumbs.begin(), _thumbs.end(), items, FolderItem::isSelected);

	ULONG ulInOut = SFGAO_CANRENAME | SFGAO_CANMOVE | SFGAO_CANDELETE | SFGAO_CANCOPY | SFGAO_HASPROPSHEET | SFGAO_READONLY;
	ULONG nAttribs = 0;

	HRESULT hr = GetShellFolder()->GetAttributesOf(nCount, items, &ulInOut);

	if (SUCCEEDED(hr))
		nAttribs = ulInOut;

	return nAttribs; 
};

void IW::Folder::UpdateSelectedItems()
{
	IW::CAutoLockCS lock(_cs);
	_bHasSelection = _thumbs.end() != std::find_if(_thumbs.begin(), _thumbs.end(), FolderItem::isSelected);
	return;
}





CString IW::Folder::GetSelectedFileList() const
{
	IW::CAutoLockCS lock(_cs);
	CString str;
	TCHAR szDelim = _T('\n');

	for(ITEMLIST::const_iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
	{
		const FolderItemPtr pThumb = *i;

		if (pThumb->IsSelected())
		{
			str += pThumb->GetFilePath();
			str += szDelim;
		}
	}

	str += szDelim;	
	return str;
}




void IW::Folder::DeleteAllThumbs()
{
	IW::CAutoLockCS lock(_cs);

	std::for_each(_thumbs.begin(), _thumbs.end(), FolderItem::releaseItem);
	_thumbs.clear();

	// Reset from delete
	_nFocusItem = -1;
	_timeRemaining = INT_MAX;	
}

void IW::Folder::InsertThumb(IW::FolderItem *pThumbIn)
{
	IW::CAutoLockCS lock(_cs);

	pThumbIn->AddRef();
	_thumbs.push_back(pThumbIn);

	if (FolderItem::isImage(pThumbIn))
	{
		_nImageCount++;
	}

	if (pThumbIn->_uFlags & THUMB_LOADED)
	{
		_nLoadedCount++;
	}
}





bool IW::Folder::GetParentItem(IW::CShellItem &item) const
{
	IW::CAutoLockCS lock(_cs);

	// item
	item = _item;
	return item.StripToParent();
}

int IW::Folder::Find(const IW::CShellItem &item) const
{
	IW::CAutoLockCS lock(_cs);
	int n = 0;

	for(ITEMLIST::const_iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
	{
		const IW::FolderItem *pThumb = *i;

		HRESULT hr = GetShellFolder()->CompareIDs(0, pThumb->_item, item);

		ATLASSERT(SUCCEEDED(hr));

		if (SCODE_CODE(hr) == 0)
			return n;

		n++;
	}

	return -1;
}


int IW::Folder::Find(const CString &strFileName) const
{
	IW::CAutoLockCS lock(_cs);

	for(int i = 0; i < GetSize(); ++i)
	{
		if (GetItemName(i).CompareNoCase(strFileName) == 0)
		{
			return i;
		}
	}

	return -1;
}

CString IW::Folder::GetFolderName() const
{
	IW::CAutoLockCS lock(_cs);
	IW::CShellFolder pParentFolder;

	if (_item.Depth() > 1)
	{
		IW::CShellItem itemParent(_item);
		itemParent.StripToParent();
		pParentFolder.Open(itemParent, true);	
	}
	else
	{
		pParentFolder = IW::CShellDesktop();
	}

	return pParentFolder.GetDisplayNameOf(_item.GetTailItem(), SHGDN_FORPARSING|SHGDN_INFOLDER);
}

CString IW::Folder::GetFolderPath() const
{
	IW::CAutoLockCS lock(_cs);

	IW::CShellDesktop desktop;
	return desktop.GetDisplayNameOf(_item, SHGDN_FORPARSING);
}

bool IW::Folder::GetParentFolder(IW::Folder ** ppFolderOut)
{
	IW::CAutoLockCS lock(_cs);

	if (!_item.IsDesktop())
	{
		IW::CShellItem item(_item);
		item.StripToParent();

		IW::FolderPtr pFolder = new IW::RefObj<IW::Folder>;
		if (!pFolder->Init(item))
			return false;

		return (IW::ReferencePtrAssign(ppFolderOut, static_cast<IW::Folder*>(pFolder)) != 0);
	}

	IW::ReferencePtrAssign(ppFolderOut, static_cast<IW::Folder*>(0));
	return false;
}

// Methods to handel selectability
long IW::Folder::GetSelectedItemCount() const
{
	IW::CAutoLockCS lock(_cs);
	int nCount = std::count_if(_thumbs.begin(), _thumbs.end(), FolderItem::isSelected);
	return nCount;
}

long IW::Folder::GetItemCount() const
{
	IW::CAutoLockCS lock(_cs);
	return _thumbs.size();
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
//// FolderItemLoader
////

IW::FolderItemLoader::FolderItemLoader(ThumbnailCache &cache) : 
	_bLoadedImage(false), 
	_bLoadedIcon(false), 
	_bDidStartLoad(false),
	_nSubItemCount(0),
	_sizeThumbnail(App.Options._sizeThumbImage),
	_cache(cache)
{
}

IW::FolderItemLoader::FolderItemLoader(ThumbnailCache &cache, const CSize &sizeThumbnail) : 
	_bLoadedImage(false), 
	_bLoadedIcon(false), 
	_bDidStartLoad(false),
	_nSubItemCount(0),
	_sizeThumbnail(sizeThumbnail),
	_cache(cache)
{
}

IW::FolderItemLoader::FolderItemLoader(ThumbnailCache &cache, IW::FolderItem &item): 
	_bLoadedImage(false), 
	_bLoadedIcon(false), 
	_bDidStartLoad(false),
	_item(item),
	_sizeThumbnail(App.Options._sizeThumbImage),
	_cache(cache)
{
	item._uFlags |= THUMB_LOADING;
	_strFilePath = item.GetFilePath();
}

IW::FolderItemLoader::~FolderItemLoader()
{
}

bool IW::FolderItemLoader::SyncThumb(IW::FolderItem &item)
{
	if (_bLoadedImage)
	{	
		// Set the thumbnail dib
		item._uFlags |= THUMB_IMAGE;
		item._image = _item._image;
	}

	if (_bLoadedIcon)
	{
		item._uFlags |= THUMB_IMAGE_ICON;
		item._image = _item._image;
	}

	item._nSubItemCount = _nSubItemCount;
	item._nImage = _item._nImage;
	item._uFlags |= THUMB_LOADED;
	item._uFlags &= ~THUMB_LOADING;

	return true;
}


bool IW::FolderItemLoader::LoadImage(CLoadAny *pLoader, const Search::Spec &spec, bool bForceLoad)
{
	if (bForceLoad || (_item._uFlags & THUMB_LOADED) == 0)
	{
		_bDidStartLoad = true;
		DWORD timeStart = GetTickCount();

		// Figure out extension
		DWORD uExtension = App.GetExtensionKey(GetFilePath());

		// Load an icon
		if (_item._nImage == -1)
		{
			bool bMayBeCachedIcon = App.CanBeCached(uExtension);

			if (bMayBeCachedIcon)
			{
				_item._nImage = App.GetIcon(uExtension);
			}

			if (_item._nImage == -1)
			{				
				/* IW::RefPtr<IShellIcon> pShellIcon;
				hr = _item._pFolder->QueryInterface(IID_IShellIcon, (void**)&pShellIcon);

				if (FAILED(hr))
				{
					hr = _item._pFolder->GetUIObjectOf(NULL, 1, _item._item, IID_IShellIcon, NULL, (void**)&pShellIcon);
				}

				if (SUCCEEDED(hr) && pShellIcon) // early shell version, thumbs not supported
				{
					hr = pShellIcon->GetIconOf(_item._item, GIL_FORSHELL, &_item._nImage);
				}

				if (hr != S_OK)
				{*/
					SHFILEINFO    sfi;
					IW::MemZero(&sfi, sizeof(sfi));

					if (SHGetFileInfo(GetFilePath(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX ) != 0)
					{				
						_item._nImage = sfi.iIcon;
					}
				//}

				if (bMayBeCachedIcon)
				{
					App.SetIcon(uExtension, _item._nImage);
				}
			}
		}		

		if (pLoader && _item.IsFolder())
		{
			if (App.Options.m_bWalkFolders)
			{				
				LoadFolderImage(pLoader, bForceLoad);
			}
		}
		else if (!bForceLoad && _cache.FindAndLoadThumbFromCache(_item))
		{
			// Loaded from cache
			_bLoadedImage = _item.IsImage();
		}	
		else
		{
			IW::CShellSource source(GetFilePath());

			if (pLoader)
			{
				ATLTRACE(_T("Thumbnailing Image %s\n"), GetFilePath());
				App.RecordFileOperation(GetFilePath(), _T("Thumbnailing Image"));

				if (IsAllowedType(uExtension))
				{
					IW::Image &image = _item._image;
					image.Free();
					IW::ImageStreamThumbnail<IW::IImageStream> imageOut(image, spec, _sizeThumbnail);
					_bLoadedImage = pLoader->LoadImage(GetFilePath(), &imageOut, IW::CNullStatus::Instance);
					_bLoadedImage = _bLoadedImage && !image.IsEmpty();
				}
			}


			/*
			USES_CONVERSION;

			Bitmap image( T2W(path) );
			if( image.GetFlags() != ImageFlagsNone )
			{			
			int sourceWidth  = image.GetWidth();
			int sourceHeight = image.GetHeight();

			int destX = 0,
			destY = 0; 

			float nPercent  = 0;
			float nPercentW = ((float)App.Options._sizeThumbImage.cx/(float)sourceWidth);
			float nPercentH = ((float)App.Options._sizeThumbImage.cy/(float)sourceHeight);

			if(nPercentH < nPercentW)
			{
			nPercent = nPercentH;
			destX    = (int)((App.Options._sizeThumbImage.cx - (sourceWidth * nPercent))/2);
			}
			else
			{
			nPercent = nPercentW;
			destY    = (int)((App.Options._sizeThumbImage.cy - (sourceHeight * nPercent))/2);
			}

			int destWidth  = (int)(sourceWidth * nPercent);
			int destHeight = (int)(sourceHeight * nPercent);


			Image* pThumbnail = image.GetThumbnailImage(destWidth, destHeight, NULL, NULL);

			Bitmap bitmap( destWidth, destHeight, PixelFormat24bppRGB );
			Graphics *g = Graphics::FromImage( &bitmap );
			g->DrawImage( pThumbnail, 0, 0, destWidth, destHeight );

			HBITMAP hBitmap = NULL;
			Color colorW(255, 255, 255, 255);
			bitmap.GetHBITMAP( colorW, &hBitmap );


			CWindowDC dcDesktop(GetDesktopWindow());
			_image.Copy(dcDesktop, hBitmap);
			DeleteObject( hBitmap );

			delete g;
			delete pThumbnail;				

			_bImageState = true;
			}

			*/


			/* if (!_bLoadedImage && App.Options._bSystemThumbs)
			{
				ULONG u = _item._pFolder.GetAttributes(_item._item, SFGAO_ISSLOW);

				if ((SFGAO_ISSLOW & u) == 0)
				{
					HRESULT hr;
					IW::RefPtr<IExtractImage> pIExtract;
					hr = _item._pFolder->GetUIObjectOf(NULL, 1, _item._item, IID_IExtractImage, NULL, (void**)&pIExtract);

					if (SUCCEEDED(hr) && pIExtract) // early shell version, thumbs not supported
					{
						OLECHAR wszPathBuffer[MAX_PATH];
						DWORD dwPriority = IEIT_PRIORITY_NORMAL | IEIFLAG_OFFLINE | IEIFLAG_QUALITY;
						DWORD dwFlags = IEIFLAG_QUALITY;
						DWORD dwRecClrDepth = 24;
						HBITMAP hBmpImage = NULL;
						CSize sizeScale = App.Options._sizeThumbImage;

						hr = pIExtract->GetLocation(wszPathBuffer, MAX_PATH, &dwPriority, &sizeScale, dwRecClrDepth, &dwFlags);

						// even if we've got shell v4.70+, not all files support thumbnails 
						if(NOERROR == hr) 
						{
							hr = pIExtract->Extract(&hBmpImage);

							if (SUCCEEDED(hr))
							{
								CWindowDC dc(GetDesktopWindow());
								if (_item._image.Copy(dc, hBmpImage))
								{
									_bLoadedIcon = true;
								}

								DeleteObject(hBmpImage);
							}
						}						
					}
				}
			} */


			if (_bLoadedImage && _sizeThumbnail == App.Options._sizeThumbImage)
			{
				_cache.QueueThumbCacheUpdate(_item);
			}
		}

		// Did we take longer than 5 seconds?
		//assert(timeStart + 5000 > GetTickCount());
	}

	return true;
}

void IW::FolderItemLoader::LoadFolderImage(CLoadAny *pLoader, bool bForceLoad)
{
	WIN32_FIND_DATA findData;
	CString strSearch = GetFilePath() + _T("\\*.*");
	HANDLE hSearch = FindFirstFile(strSearch, &findData);

	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		do
		{
			if (IsFolderWalkImageType(App.GetExtensionKey(findData.cFileName)))
			{
				if (!_bLoadedIcon)
				{
					CString strFilePath = IW::Path::Combine(GetFilePath(), findData.cFileName);

					IW::FolderItem subItem;	
					subItem.Init(strFilePath);

					IW::FolderItemLoader job(_cache, _sizeThumbnail);

					if (subItem.LoadJobBegin(job))
					{
						job.LoadImage(pLoader, Search::Any, bForceLoad);
						job.RenderAndScale();
					}

					subItem.LoadJobEnd(job);

					if (subItem.IsImage())
					{
						CSize size(_sizeThumbnail.cx / 2,  _sizeThumbnail.cy / 2);

						IW::ImageStreamThumbnail<IW::IImageStream> imageOut(_item._image, Search::Any, size);
						IW::IterateImage(subItem.GetImage(), imageOut, IW::CNullStatus::Instance);

						_bLoadedIcon = !_item._image.IsEmpty();
					}
				}

				_nSubItemCount++;
			}
		} 
		while(FindNextFile(hSearch, &findData)); 

		FindClose(hSearch);
	}
}

void IW::FolderItemLoader::RenderAndScale()
{	

	if (_bLoadedImage || _bLoadedIcon)
	{
		bool bIsFolder = (_item._ulAttribs & SFGAO_FOLDER) != 0;

		CSize sizeScale = _sizeThumbnail;
		const CRect rectSize = _item._image.GetBoundingRect();

		if (bIsFolder)
		{
			sizeScale.cx /= 2;
			sizeScale.cy /= 2;
		}

		if (_item._image.NeedRenderForDisplay())
		{
			IW::Image imageTemp;
			_item._image.Render(imageTemp);
			_item._image = imageTemp;
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString IW::Folder::GetItemName(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	return _thumbs[nItem]->GetFileName();
}

CString IW::Folder::GetItemPath(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	return _thumbs[nItem]->GetFilePath();
}

///////////////////////////////////////////////////////////////////////////////////////
// Selection Helpers

void IW::Folder::Select(int nFocusNew, UINT nFlags)
{
	IW::CAutoLockCS lock(_cs);

	int nSize = GetSize();

	if (nFocusNew < 0 || nFocusNew > (nSize -1))
		return;

	int nFocusOld = _nFocusItem;

	if (nFocusNew != nFocusOld)
	{
		if (-1 != nFocusOld) 
		{
			IW::FolderItem *pThumb = _thumbs[nFocusOld];		
			pThumb->_uFlags |= THUMB_INVALIDATE;
		}

		_nFocusItem = nFocusNew;
		IW::FolderItem *pThumb = _thumbs[nFocusNew];		
		pThumb->_uFlags |= THUMB_INVALIDATE;
	}

	if (!(nFlags & MK_CONTROL))
	{
		for(int j = 0; j < nSize; j++)
		{
			IW::FolderItem *pThumb = _thumbs[j];

			if (pThumb->IsSelected())
			{
				pThumb->_uFlags &= ~THUMB_SELECTED;
				pThumb->_uFlags |= THUMB_INVALIDATE;
			}
		}
	}

	if (nFlags & MK_SHIFT && nFocusOld != -1)
	{
		if (nFocusNew < nFocusOld)
		{
			for(int j = nFocusNew; j <= nFocusOld; j++)
			{
				IW::FolderItem *pThumb = _thumbs[j];

				if (nFlags & MK_CONTROL)
					pThumb->_uFlags ^= THUMB_SELECTED;
				else
					pThumb->_uFlags |= THUMB_SELECTED;

				pThumb->_uFlags |= THUMB_INVALIDATE;
			}
		}
		else
		{
			for(int j = nFocusOld; j <= nFocusNew; j++)
			{
				IW::FolderItem *pThumb = _thumbs[j];

				if (nFlags & MK_CONTROL)
					pThumb->_uFlags ^= THUMB_SELECTED;
				else
					pThumb->_uFlags |= THUMB_SELECTED;

				pThumb->_uFlags |= THUMB_INVALIDATE;
			}
		}
	}
	else
	{
		IW::FolderItem *pThumb = _thumbs[nFocusNew];

		if (nFlags & MK_CONTROL)
			pThumb->_uFlags ^= THUMB_SELECTED;
		else
			pThumb->_uFlags |= THUMB_SELECTED;

		pThumb->_uFlags |= THUMB_INVALIDATE;
	}

	UpdateSelectedItems();
}

void IW::Folder::SelectAll()
{
	IW::CAutoLockCS lock(_cs);
	std::for_each(_thumbs.begin(), _thumbs.end(), FolderItem::selectItem);
	UpdateSelectedItems();
}

void IW::Folder::SelectInverse()
{
	IW::CAutoLockCS lock(_cs);
	std::for_each(_thumbs.begin(), _thumbs.end(), FolderItem::selectInverse);
	UpdateSelectedItems();
}

class SelectTagAdapter
{
public:

	CString _tag;

	SelectTagAdapter(CString tag) : _tag(tag)
	{
	}

	SelectTagAdapter(const SelectTagAdapter &other) : _tag(other._tag)
	{
	}

	void operator()(IW::FolderItem *pThumb)
	{
		IW::TAGSET tags = pThumb->GetTags();
		bool bSelect = false;

		for (IW::TAGSET::const_iterator it = tags.begin(); it != tags.end(); ++it)
		{
			if (_tag.CompareNoCase(*it) == 0)
			{
				bSelect = true;
				break;
			}
		}

		if (bSelect)
		{
			pThumb->ModifyFlags(0, THUMB_SELECTED);
		}
		else
		{
			pThumb->ModifyFlags(THUMB_SELECTED, 0);
		}
	}
};


void IW::Folder::SelectImages()
{
	IW::CAutoLockCS lock(_cs);
	std::for_each(_thumbs.begin(), _thumbs.end(), FolderItem::selectIfImage);
	UpdateSelectedItems();
}

void IW::Folder::SelectImagesOnFlickr()
{
	IW::CAutoLockCS lock(_cs);
	std::for_each(_thumbs.begin(), _thumbs.end(), FolderItem::selectIfImageOnFlickr);
	UpdateSelectedItems();
}

void IW::Folder::SelectTag(const CString &strTag)
{
	IW::CAutoLockCS lock(_cs);
	SelectTagAdapter selector(strTag);

	std::for_each(_thumbs.begin(), _thumbs.end(), selector);
	UpdateSelectedItems();
}

int IW::Folder::GetSelectCount() const
{
	IW::CAutoLockCS lock(_cs);
	return std::count_if(_thumbs.begin(), _thumbs.end(), FolderItem::isSelected);
}

void IW::Folder::GetSelectStatus(int &nCount, int &nImages, IW::FileSize &size) const
{
	IW::CAutoLockCS lock(_cs);

	for(ITEMLIST::const_iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
	{
		const IW::FolderItem *pThumb = *i;

		if (pThumb->IsSelected())
		{
			nCount++;
			size += pThumb->_sizeFile;

			if (pThumb->IsImage())
			{
				nImages++;
			}
		}
	}
}


void IW::Folder::DragSelection(int nItem)
{
	IW::CAutoLockCS lock(_cs);

	int nItemCount = _thumbs.size();
	int nInsertLocation = IW::Clamp(nItem, 0, nItemCount);	

	IW::ITEMLIST newOrderThumbs;
	newOrderThumbs.reserve(nItemCount);

	int nIndex = 0;

	for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
	{
		if (nIndex == nInsertLocation)
		{
			IW::copy_if(_thumbs.begin(), _thumbs.end(), 
				std::inserter(newOrderThumbs, newOrderThumbs.end()), 
				FolderItem::isSelected);
		}

		if (!FolderItem::isSelected(*i))
		{
			newOrderThumbs.push_back(*i);
		}		

		nIndex++;
	}
	
	//IW::copy_if(_thumbs.begin(), _thumbs.end(), newOrderThumbs.end(), isNotSelected);
	//IW::copy_if(_thumbs.begin(), _thumbs.end(), newOrderThumbs.end(), isSelected);
	//IW::copy_if(_thumbs.begin() + nInsertLocation, _thumbs.end(), newOrderThumbs.end(), isNotSelected);

	_thumbs = newOrderThumbs;
}

bool IW::Folder::IsItemDropTarget(int nItem) const
{
	IW::CAutoLockCS lock(_cs);

	const IW::FolderItem *pThumb = _thumbs[nItem];

	IW::CShellItem item = pThumb->_item; 
	IW::CShellFolder pShellFolder = pThumb->GetShellFolder(); 

	ULONG u = pShellFolder.GetAttributes(item, SFGAO_LINK | SFGAO_DROPTARGET);			

	if (u & SFGAO_LINK)
	{
		if (FAILED(pShellFolder.ResolveLink(item, item)))
		{
			u = 0;
		}
		else
		{
			IW::CShellDesktop desktop;	
			u = desktop.GetAttributes(item, SFGAO_DROPTARGET);
		}
	}

	return (u & SFGAO_DROPTARGET) == 0;
}

bool IW::Folder::IsItemBrowsable(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->GetShellFolder().IsBrowsable(pThumb->_item);
}



bool IW::Folder::IsItemImage(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	return  FolderItem::isImage(_thumbs[nItem]);
}


void IW::Folder::GetItemImage(int nItem, IW::Image &imageOut) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];
	imageOut = pThumb->_image;
}

int IW::Folder::GetItemImageNum(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->_nImage;
}

bool IW::Folder::IsItemMultiPageImage(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];

	return pThumb->IsImage() &&
		pThumb->IsFolder() &&
		pThumb->_image.GetPageCount() > 1;
}

bool IW::Folder::IsItemAnimatedImage(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];

	return pThumb->IsImage() &&
		pThumb->IsFolder() &&
		pThumb->_image.CanAnimate();
}	

bool IW::Folder::AnimationStep(int nItem)
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];

	bool bInvalidate = false;
	pThumb->_nTimer -= 100;

	if (pThumb->_nTimer <= 0)
	{
		// Work out page
		int nPage = (pThumb->_nFrame + 1) % pThumb->_image.GetPageCount();
		IW::Page page = pThumb->_image.GetPage(nPage); 

		pThumb->_nTimer = page.GetTimeDelay();
		pThumb->_nFrame = nPage;

		bInvalidate = true;
	}

	return bInvalidate;
}

bool IW::Folder::IsItemSelected(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	const IW::FolderItem *pThumb = _thumbs[nItem];
	return ((pThumb->IsSelected()) != 0);
}

bool IW::Folder::CanLoad(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	const IW::FolderItem *pThumb = _thumbs[nItem];
	return (pThumb->_uFlags & (THUMB_LOADED | THUMB_LOADING)) == 0;
}

UINT IW::Folder::GetItemFlags(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	const IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->_uFlags;
}

int IW::Folder::GetItemFrame(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	const IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->_nFrame;
}

UINT IW::Folder::GetItemAttribs(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	const IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->_ulAttribs;
}

CRect IW::Folder::GetItemThumbRect(int nItem) const
{
	IW::CAutoLockCS lock(_cs);
	const IW::FolderItem *pThumb = _thumbs[nItem];
	return pThumb->_image.GetBoundingRect();
}

void IW::Folder::GetCameraSettings(int nItem, IW::CameraSettings &settings) const
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];
	settings = pThumb->_image.GetCameraSettings();
}

void IW::Folder::SetItemFlags(int nItem, UINT uFlags)
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];
	pThumb->_uFlags = uFlags;
}

void IW::Folder::ModifyItemFlags(int nItem, DWORD dwRemove, DWORD dwAdd)
{
	IW::CAutoLockCS lock(_cs);
	IW::FolderItem *pThumb = _thumbs[nItem];

	DWORD dwFlags = pThumb->_uFlags;
	DWORD dwNewFlags = (dwFlags & ~dwRemove) | dwAdd;

	if(dwFlags != dwNewFlags)
	{
		pThumb->_uFlags = dwNewFlags;
	}
}

void IW::Folder::ResetLoadedFlag()
{
	IW::CAutoLockCS lock(_cs);
	std::for_each(_thumbs.begin(), _thumbs.end(), FolderItem::clearLoadedFlag);
}

int IW::Folder::CalcLoadedCount()
{
	IW::CAutoLockCS lock(_cs);
	return std::count_if(_thumbs.begin(), _thumbs.end(), FolderItem::isLoaded);
}

int IW::Folder::CalcImageCount()
{
	IW::CAutoLockCS lock(_cs);
	return std::count_if(_thumbs.begin(), _thumbs.end(), FolderItem::isImage);
}

int IW::Folder::GetFocusItem() const
{
	IW::CAutoLockCS lock(_cs);
	return _nFocusItem;
}

void IW::Folder::SetFocusItem(int nFocusItem)
{
	IW::CAutoLockCS lock(_cs);
	_nFocusItem = nFocusItem;
}

int IW::Folder::GetImageCount() const
{
	IW::CAutoLockCS lock(_cs);
	return _nImageCount;
}

void IW::Folder::InvalidateThumb(int nItem)
{
	ModifyItemFlags(nItem, 0, THUMB_INVALIDATE);
}

int IW::Folder::GetPercentComplete() const 
{
	IW::CAutoLockCS lock(_cs);
	long nFilesOrFolders = GetSize();
	long nPercentComplete = MulDiv(_nLoadedCount, 100, nFilesOrFolders);
	return nPercentComplete;
}

int IW::Folder::GetTimeRemaining() const
{
	IW::CAutoLockCS lock(_cs);
	int timeTaken = (_timeLast - _timeFirst) / 100;

	long nFilesOrFolders = GetSize();
	long nImages = _nImageCount;

	// If wParam is set then all
	// thumbs are loaded
	long nPercentComplete = MulDiv(_nLoadedCount, 100, nFilesOrFolders);
	long nSecondsTimesTen = timeTaken;

	int nPercentRemaining = 100 - nPercentComplete;
	int timeRemaining = MulDiv(nSecondsTimesTen, nPercentRemaining, nPercentComplete);

	if (timeRemaining < 1)
		timeRemaining = 1;

	return _timeRemaining = (timeRemaining + _timeRemaining) / 2;
}

int IW::Folder::GetTimeTaken() const
{
	IW::CAutoLockCS lock(_cs);
	int timeTaken = (_timeLast - _timeFirst) / 100;
	return timeTaken;
}

void IW::Folder::ResetCounters()
{
	IW::CAutoLockCS lock(_cs);

	_nLoadedCount = CalcLoadedCount();
	_nImageCount = CalcImageCount();

	// Rest the timer
	_timeLast = _timeFirst = GetTickCount();
	_timeRemaining = INT_MAX;
	_nLoadItem = 0;
}




/////////////////////////////////////////////////////////////////////////////

// Compare
template<bool bReverse>
class CCompareName
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareName(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder) {}	

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = IW::FolderItem::CompareName(_pShellFolder, a, b);
		return (bReverse ? -i : i) < 0;
	}	
};

template<bool bReverse>
class CCompareSize 
{
protected:
	IW::CShellFolder &_pShellFolder;
public:
	CCompareSize(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder) {}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = IW::FolderItem::CompareSize(_pShellFolder, a, b);
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
class CCompareType 
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareType(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder)
	{
	}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = IW::FolderItem::CompareType(_pShellFolder, a, b);
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
class CCompareCreationTime 
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareCreationTime(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder)
	{
	}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = IW::FolderItem::CompareCreationTime(_pShellFolder, a, b);
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
class CCompareDateTaken 
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareDateTaken(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder)
	{
	}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = IW::FolderItem::CompareDateTaken(_pShellFolder, a, b);
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
class CCompareLastWriteTime 
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareLastWriteTime(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder)
	{
	}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = IW::FolderItem::CompareLastWriteTime(_pShellFolder, a, b);
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
class CCompareWidth 
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareWidth(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder) {}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = a->GetImage().GetCameraSettings().OriginalImageSize.cx - 
			b->GetImage().GetCameraSettings().OriginalImageSize.cx;
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
class CCompareHeight
{
protected:
	IW::CShellFolder &_pShellFolder;

public:
	CCompareHeight(IW::CShellFolder &pShellFolder) : _pShellFolder(pShellFolder) {}

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		int i = a->GetImage().GetCameraSettings().OriginalImageSize.cy - 
			b->GetImage().GetCameraSettings().OriginalImageSize.cy;
		return (bReverse ? -i : i) < 0;
	}
};

template<bool bReverse>
void SortThumbs(int nSortOrder, IW::CShellFolder &pShellFolder, IW::ITEMLIST &thumbs)
{	
	switch(nSortOrder)
	{
	default:
	case IW::ePropertyName:
	case IW::ePropertyPath:
		{
			CCompareName<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;

	case IW::ePropertyType:
		{
			CCompareType<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;
		
	case IW::ePropertySize:
		{
			CCompareSize<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;
		
	case IW::ePropertyModifiedDate:
	case IW::ePropertyModifiedTime:
		{
			CCompareCreationTime<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;

	case IW::ePropertyDateTaken:
		{
			CCompareDateTaken<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;
		
	case IW::ePropertyCreatedDate:
	case IW::ePropertyCreatedTime:
		{
			CCompareLastWriteTime<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;
	case IW::ePropertyWidth:
		{
			CCompareWidth<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;
	case IW::ePropertyHeight:
		{
			CCompareHeight<bReverse> t(pShellFolder);
			std::sort(thumbs.begin(), thumbs.end(), t);
		}
		break;
	}
};



void IW::Folder::Sort(int nSortOrder, bool bAssending)
{
	CWaitCursor wait;
	IW::CAutoLockCS lock(_cs);

	if (bAssending)
	{
		SortThumbs<false>(nSortOrder, _pFolder, _thumbs);
	}
	else
	{
		SortThumbs<true>(nSortOrder, _pFolder, _thumbs);
	}
}

class FolderItemCompareName
{

public:
	

	bool operator()(const IW::FolderItem *a, const IW::FolderItem *b) const
	{
		CString strNameA = a->GetFileName();
		CString strNameB = b->GetFileName();
		return strNameA.CompareNoCase(strNameB) < 0;
	}
};

void IW::Folder::Sort(ITEMLIST &thumbs, int nSortOrder, bool bAssending)
{
	CWaitCursor wait;
	IW::CAutoLockCS lock(_cs);

	if (bAssending)
	{
		SortThumbs<false>(nSortOrder, _pFolder, thumbs);
	}
	else
	{
		SortThumbs<true>(nSortOrder, _pFolder, thumbs);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class FolderItem200
{
public:
	FILETIME _timeFile;
	long  _sizeFile;
	ULONG _ulAttribs;
	UINT  _uExtension;
	UINT  _uFlags;
	int  _nImage;
	
	
	FolderItem200() :
	// Variable defaults.
		_ulAttribs(0),
		_sizeFile(0),
		_uExtension(0),
		_uFlags(0),
		_nImage(-1)
	{
		_timeFile.dwLowDateTime = 0;
		_timeFile.dwHighDateTime = 0;
	}

	FolderItem200(const IW::FolderItem &thumb)
	{
		Copy(thumb);
	}

	void Copy(const IW::FolderItem &thumb)
	{
		_timeFile.dwLowDateTime = thumb._ftLastWriteTime.GetLowDateTime();
		_timeFile.dwHighDateTime = thumb._ftLastWriteTime.GetHighDateTime();

		_sizeFile = thumb._sizeFile;
		_ulAttribs = thumb._ulAttribs;
		_uExtension = thumb._uExtension;
		_uFlags = thumb._uFlags;
		_nImage = thumb._nImage;
	}

	void CopyTo(IW::FolderItem &thumb) const
	{
		thumb._ftLastWriteTime = _timeFile;
		thumb._sizeFile = _sizeFile;
		thumb._ulAttribs = _ulAttribs;
		thumb._uExtension = _uExtension;
		thumb._uFlags = _uFlags;
		thumb._nImage = _nImage;
	}
};

bool IW::FolderItem::Write(IW::CFile &f) const
{
	unsigned nFlags = 0;

	// Save Thumb Information
	FolderItem200 tp(*this);
	f.Write(&tp, sizeof(FolderItem200));

	const IW::Page &page = _image.Pages[0];
	IW::CImageBitmapInfoHeader bmi(page);

	int nPageCount = 1;
	int nTimeDelay = 0;
	int clrBackGround = 0;
	IW::PixelFormat pf = page.GetPixelFormat();
	int nPaletteEntries = page.GetPixelFormat().NumberOfPaletteEntries();
	int cx = page.GetWidth();
	int cy = page.GetHeight();
	int nStorageWidth = page.GetStorageWidth();
	int nSizeImage = nStorageWidth * cy;
	int nPageSize = sizeof(BITMAPINFOHEADER) + (nPaletteEntries * sizeof(RGBQUAD)) + nSizeImage;

	// Write Image details
	f.Write(&nFlags, sizeof(nFlags));
	f.Write(&nStorageWidth, sizeof(nStorageWidth));
	f.Write(&nPageSize, sizeof(nPageSize));
	f.Write(&nPageCount, sizeof(nPageCount));
	f.Write(&nTimeDelay, sizeof(nTimeDelay));
	f.Write(&clrBackGround, sizeof(clrBackGround));
	
	// Write the info string
	CStringA str = _image.GetStatistics();
	f.WriteString(str);

	str.Empty();
	f.WriteString(str);
	
	// IPTC Profile
	const IW::MetaData iptc = _image.GetMetaData(IW::MetaDataTypes::PROFILE_IPTC);

	if (!iptc.IsEmpty())
	{
		int n = iptc.GetSize();
		f.Write(&n, sizeof(n));
		f.Write(iptc.GetData(), n);
	}
	else
	{
		int n = 0;
		f.Write(&n, sizeof(n));
	}	
	
	// Write the image memory	
	f.Write((LPBITMAPINFO)bmi, sizeof(BITMAPINFOHEADER));

	if (nPaletteEntries != 0)
	{
		f.Write(page.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
	}
	
	f.Write(page.GetBitmap(), nSizeImage);
	
	return true;
}

bool IW::FolderItem::Read(IW::CFile &f, unsigned nFileVersion)
{
	DWORD dwRead;

	FolderItem200 persist;
		
	// Read the thumb information
	if (!f.Read(&persist, sizeof(persist), &dwRead) || dwRead != sizeof(persist))
	{
		return false;
	}	

	persist._uFlags &= THUMB_PERSIST_MASK;
	
	// Read the image info
	unsigned nFlags;
	unsigned nStorageWidth;
	unsigned nPageSize;
	unsigned nPageCount;
	int nTimeDelay;
	COLORREF clrBackGround;	
	CStringA strComments;
	CStringA strInformation;	
	int nIPTCLength = 0;
	
	if (!f.Read(&nFlags, sizeof(nFlags), &dwRead) || (dwRead != sizeof(nFlags)) ||
		!f.Read(&nStorageWidth, sizeof(nStorageWidth), &dwRead) || (dwRead != sizeof(nStorageWidth)) ||
		!f.Read(&nPageSize, sizeof(nPageSize), &dwRead) || (dwRead != sizeof(nPageSize)) ||
		!f.Read(&nPageCount, sizeof(nPageCount), &dwRead) || (dwRead != sizeof(nPageCount)) ||
		!f.Read(&nTimeDelay, sizeof(nTimeDelay), &dwRead) || (dwRead != sizeof(nTimeDelay)) ||
		!f.Read(&clrBackGround, sizeof(clrBackGround), &dwRead) || (dwRead != sizeof(clrBackGround)) ||
		!f.ReadString(strInformation) ||
		!f.ReadString(strComments) ||
		!f.Read(&nIPTCLength, sizeof(nIPTCLength), &dwRead) || (dwRead != sizeof(nIPTCLength)))
	{
		// Failied
		return false;
	}
	
	unsigned nSizeImage = nPageSize * nPageCount;	
	
	// only update if existing is not upddated
	// after the thumb was saved
	if (_ftLastWriteTime == persist._timeFile)
	{
		//*((FolderItem200*)this) = persist;

		// Strip away extra flags?		
		//_image._nFlags = nFlags;
		//_image._nTimeDelay = nTimeDelay;
		//_image._clrBackGround = clrBackGround;		
		
		// Set the text
		_image.SetStatistics(CString(strInformation));

		// Try to get the original width height etc
		SIZE sizeOrg = { 0, 0 };
		int nBppOrg = 0;

		if (3 == _snscanf_s(strInformation, strInformation.GetLength(), "%dx%dx%d", &sizeOrg.cx, &sizeOrg.cy, &nBppOrg))
		{
			CameraSettings settings;
			settings.OriginalImageSize = sizeOrg;
			settings.OriginalBpp = PixelFormat::FromBpp(nBppOrg);
			_image.SetCameraSettings(settings);

			// Exif::Parse(src->pSettings, data.GetData(), nLength);
		}


		//_image._strComments = strComments;
		_image.SetFlags(0);
		
		// IPTC Profile		
		if (nIPTCLength > 0)
		{
			IW::MetaData iptc(IW::MetaDataTypes::PROFILE_IPTC);
			iptc.Alloc(nIPTCLength);

			if (!f.Read(iptc.GetData(), nIPTCLength, &dwRead) || dwRead != nIPTCLength)
			{
				return false;
			}

			_image.SetMetaData(iptc);
		}
		
		// Image Data
		int nImageSize = nPageSize * nPageCount;
		IW::SimpleBlob blob;
		blob.ReAlloc(nImageSize);

		LPBYTE p = blob.GetData();
		
		if (!p || !f.Read(p, nImageSize, &dwRead) || dwRead != nImageSize)
		{
			return false;
		}

		BITMAPINFO *pInfo = (BITMAPINFO*)p;
		PixelFormat pf = PixelFormat::FromBpp(pInfo->bmiHeader.biBitCount);		
		DWORD nSizeHeader = sizeof(BITMAPINFOHEADER) + pf.NumberOfPaletteEntries() * sizeof(RGBQUAD);
		LPBYTE pBytes = p + nSizeHeader;

		_image.Copy(pInfo, pBytes, nImageSize);
		_uFlags |= THUMB_IMAGE | THUMB_LOADED;
	}
	else
	{
		nSizeImage += nIPTCLength;

		// Skip Image
		BYTE p[255];

		while(nSizeImage > 0)
		{
			unsigned nSizeRead = IW::Min(255, nSizeImage);
			if (!f.Read(p, nSizeRead)) 	
				return false;
			nSizeImage -= nSizeRead;
		}
	}	
	
	return true;
}

int IW::Folder::LoadCacheFile(CSize &sizeThumbImageCurrent, volatile bool &bAbortDecoding)
{
	IW::CAutoLockCS lock(_cs);

	int nCount = 0;
	
	WIN32_FIND_DATA FileData; 
	CString pathSearch = IW::Path::Combine(GetFolderPath(), _T("*.iw"));
	
	// Start searching for Cache files in the current directory. 
	HANDLE hSearch = FindFirstFile(pathSearch, &FileData);
	
	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		MAPNAMETOTHUMB mapThumbs;

		for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
		{
			IW::FolderItem *pThumb = *i;
			mapThumbs[pThumb->GetKeyName()] = pThumb;
		}
		
		do
		{
			CString path = IW::Path::Combine(GetFolderPath(), FileData.cFileName);
			nCount += LoadCacheFile(mapThumbs, path, sizeThumbImageCurrent, bAbortDecoding);
			
			if (nCount == 0)
			{
				nCount += LoadCacheFile105(mapThumbs, path, sizeThumbImageCurrent, bAbortDecoding);
			}
		} 
		while(FindNextFile(hSearch, &FileData) && !bAbortDecoding); 

		FindClose(hSearch);	
	} 
	
	return nCount;
}

static const int g_headerSize = 100;
static CHAR g_szCacheHeader[g_headerSize] = "ImageWalker V2.0 Cache File V1.3\nThis File Cannot be edited!\0\0";

typedef struct 
{
	unsigned nHeaderSize;
	unsigned nFileVersion;
	unsigned nImageCount;
	SIZE sizeThumbnails;
	
} CacheFileHeader;


typedef struct 
{
	unsigned nHeaderSize;
	unsigned nFileVersion;
	unsigned nImageCount;
	SIZE sizeThumbnails;
	
} CacheThumbHeader;



int IW::Folder::LoadCacheFile(MAPNAMETOTHUMB &mapThumbs, const CString &strFileName, CSize &sizeThumbImageCurrent, volatile bool &bAbortDecoding)
{
	CStringA strItemName;
	IW::CCompressFile f;
	int nCount = 0;
	bool bFail = false;
	
	try
	{		
		if (f.OpenForRead(strFileName))  
		{
			MAPNAMETOTHUMB::iterator iExistingThumb;
			CacheFileHeader header;
			
			CHAR sz[g_headerSize];
			f.ReadRaw(&sz, g_headerSize);
			f.ReadRaw(&header.nImageCount, sizeof(header.nImageCount));
			
			if (strcmp(g_szCacheHeader, sz) == 0)
			{
				bool bMarkForRerender = false;
				
				
				// If image count is zero this means there is some
				// extended header information
				if (header.nImageCount == 0)
				{
					// Version 
					f.ReadRaw(&header, sizeof(header));
					
					// Skip remining header
					int nRemainingHeader = header.nHeaderSize - sizeof(header);
					
					if (nRemainingHeader > 0)
					{
						LPVOID p = IW_ALLOCA(LPVOID, header.nHeaderSize - sizeof(header));
						f.ReadRaw(p,  header.nHeaderSize - nRemainingHeader);
					}
					
					bMarkForRerender = (sizeThumbImageCurrent.cx != header.sizeThumbnails.cx ||
						sizeThumbImageCurrent.cy != header.sizeThumbnails.cy);
				}
				else
				{
					bMarkForRerender = (IMAGE_X != sizeThumbImageCurrent.cx ||
						IMAGE_Y != sizeThumbImageCurrent.cy);
				}
				
				// Read the thumb list
				for(unsigned i = 0; i < header.nImageCount && !bFail && !bAbortDecoding; i++)
				{
					if (!f.ReadString(strItemName))
					{
						bFail = true;
					}
					else
					{
						FolderItem dummyThumb;
						strItemName.MakeLower();
						iExistingThumb = mapThumbs.find(CString(strItemName));
						
						if (iExistingThumb == mapThumbs.end())
						{							
							bFail = !dummyThumb.Read(f, header.nFileVersion);
						}
						else
						{
							FolderItemPtr pThumb = iExistingThumb->second;

							if (pThumb->IsImage())
							{
								bFail = !dummyThumb.Read(f, header.nFileVersion);
							}
							else
							{
								bFail = !pThumb->Read(f, header.nFileVersion);
								if (bMarkForRerender) pThumb->ModifyFlags(THUMB_LOADED, 0);
								pThumb->ModifyFlags(0, THUMB_INVALIDATE);
								nCount += 1;
							}
						}
					}
				}
			}
			else
			{
				// Old or unknown version
				// Just skip it
			}
			
			f.Close(0);			
			
			if (bFail)
			{
				IW::CMessageBoxIndirect mb;
				mb.ShowOsError(IDS_INVALID_CACHE_FILE_WITH_OS);
			}
	   }
   } 
   catch(std::exception &e)
   {
	   IW::CMessageBoxIndirect mb;
	   mb.ShowException(IDS_INVALID_CACHE_FILE_WITH_OS, e);
   }  
   
   return nCount;
}

#define OLD_THUMB_LOADED    0x0001
#define OLD_THUMB_SELECTED  0x0002
#define OLD_THUMB_DELETE    0x0004
#define OLD_THUMB_IMAGE     0x0008
#define OLD_THUMB_SHOWN     0x0010


class FolderItem105
{
public:
	
	FILETIME _timeFile;
	long  _sizeFile;
	ULONG _ulAttribs;
	UINT  _uExtension;
	UINT  _uFlags;
	int  _nImage;
	BITMAPINFO* _pBMI;         // Pointer to BITMAPINFO struct
	IW::CShellItem _item;
	
	FolderItem105()
	{
		_pBMI = 0;
	}
	
	~FolderItem105()
	{
		if (_pBMI)
		{
			IW::Free(_pBMI);
		}
	}
	
	void LoadFromCacheFile(IW::CCompressFile& ar)
	{
		int nItemSize;
		ar >> nItemSize;
		
		LPITEMIDLIST pItem = IW_ALLOCA(LPITEMIDLIST, nItemSize);
		ar.Read(pItem, nItemSize);
		_item.Copy(pItem);
		
		ar >> _timeFile.dwLowDateTime;
		ar >> _timeFile.dwHighDateTime;
		ar >> _sizeFile;
		ar >> _ulAttribs;
		ar >> _uExtension;
		ar >> _uFlags;
		
		if (_uFlags & OLD_THUMB_IMAGE)
		{
			UINT nSize = 0;
			UINT nOffset = 0;
			int nVersion = 2;
			int nStorageWidth;
			int nOriginalType;
			UINT uFlags;
			
			ar >> nVersion;
			ar >> nStorageWidth;
			ar >> uFlags;
			ar >> nOriginalType;
			
			ar >> nSize;
			ar >> nOffset;
			
			ATLASSERT(_pBMI == 0);
			_pBMI = (BITMAPINFO*)IW::Alloc(nSize);
			ar.Read(_pBMI,  nSize);
			
		}
	}
	
	CString GetKeyName(IW::CShellFolder pFolder) const
	{
		assert(_item.Depth() == 1); // GetDisplayNameOf only allows a single level
		return pFolder.GetDisplayNameOf(_item, SHGDN_FORPARSING|SHGDN_INFOLDER);
	}
};

int IW::Folder::LoadCacheFile105(MAPNAMETOTHUMB &mapThumbs, const CString &strFileName, CSize &sizeThumbImageCurrent, volatile bool &bAbortDecoding)
{
	int nCount = 0; 
	bool bMarkForRerender = (80 != sizeThumbImageCurrent.cx || 80 != sizeThumbImageCurrent.cy);
	IW::CCompressFile ar;
	CString strItemName;
	
	try
	{
		if (ar.OpenForRead(strFileName)) 
		{
			UINT nVersion;
			ar >> nVersion;
			
			if (nVersion == 0x00000001)
			{
				UINT nThumbCount;
				ar >> nThumbCount;
				
				MAPNAMETOTHUMB::iterator iExistingThumb;
				
				for(UINT i = 0; i < nThumbCount; i++)    
				{
					FolderItem105 thumb;
					thumb.LoadFromCacheFile(ar);
					strItemName = thumb.GetKeyName(GetShellFolder());
					strItemName.MakeLower();
					iExistingThumb = mapThumbs.find(strItemName);
					
					if (iExistingThumb != mapThumbs.end())
					{
						FolderItem *pExistingThumb = iExistingThumb->second;
						IW::Image image;
						image.Copy(thumb._pBMI);
						pExistingThumb->SetImage(image);						
						pExistingThumb->ModifyFlags(0, THUMB_IMAGE | THUMB_LOADED | THUMB_INVALIDATE);	
						if (bMarkForRerender) pExistingThumb->ModifyFlags(THUMB_LOADED, 0);						
						nCount += 1;
					}
				}
			}
			
			ar.Close(0);
		}
	} 
	catch(std::exception &)
	{
	}  
	
	
	return nCount;
}

bool IW::Folder::SaveCacheFile(const CString &strCacheFileName, const CSize &sizeThumbImageCurrent)
{
	IW::CAutoLockCS lock(_cs);

	// Header
	CacheFileHeader header;
	header.nHeaderSize = sizeof(header);
	header.nFileVersion = 1;
	header.nImageCount = 0;
	header.sizeThumbnails = sizeThumbImageCurrent;
	
	
	// Get the file output name
	CString strFileName;
	
	if (_tcsclen(strCacheFileName) != 0)
	{
		strFileName = strCacheFileName;
	}
	else
	{
		strFileName = IW::Path::Combine(GetFolderPath(), _T("ImageWalkerC.iw"));
	}
	
	
	CCompressFile f;
	
	UINT nImageSoFar = 0;
	UINT nImageStatus = 10;

	for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
	{
		IW::FolderItem *pThumb = *i;
		if (pThumb->IsImage())
			header.nImageCount++;
	}
	
	bool bFail = false;
	
	try
	{
		CFilePath path;
		path.GetTempFilePath();		
		
		if (header.nImageCount && f.OpenForWrite(path)) 
		{			
			CProgressDlg pd;
			pd.Create(IW::GetMainWindow(), IDS_SAVING_CACHE);
					
			
			// Header
			f.WriteRaw(g_szCacheHeader, g_headerSize);
			
			UINT nZero = 0;
			f.WriteRaw(&nZero, sizeof(nZero));
			
			// Not the file header
			f.WriteRaw(&header, sizeof(header));
			
			
			// Write the thumb list
			for(ITEMLIST::iterator i = _thumbs.begin(); i != _thumbs.end(); ++i)   
			{
				const FolderItem &thumb = **i;
				
				if (thumb.IsImage())
				{
					// Write the key first
					CStringA strFileName = thumb.GetKeyName();
					int nLen = strFileName.GetLength();

					f.Write(&nLen, sizeof(nLen));
					f.Write((LPCSTR)strFileName, nLen);
					
					bFail = !thumb.Write(f);					
					
					if (nImageStatus == nImageSoFar++)
					{
						CString str;						
						str.Format(IDS_SAVECACHE, nImageSoFar, header.nImageCount);						
						nImageStatus += 10;						
						pd.Progress(nImageSoFar, _thumbs.size());	
						pd.SetStatusMessage(str);
						
					}
				}
				
				if (pd.QueryCancel())
				{
					f.Abort();					
					DeleteFile(path);					
					return false;
				}
			}
			
			f.Close(0);
			
			if (!bFail)
			{
				pd.SetStatusMessage(IDS_MOVING_FILE);				
				DeleteFile(path);
				bFail = !MoveFile(path, strFileName);
			}
			
			if (bFail)
			{
				IW::CMessageBoxIndirect mb;
				mb.ShowOsError(IDS_INVALID_CACHE_FILE_SAVE);				
				return false;
			}
		}
		
	} 
	catch(std::exception &e) 
	{
		IW::CMessageBoxIndirect mb;
		mb.ShowException(IDS_INVALID_CACHE_FILE_SAVE, e);		
		return false;
	}  
	
	
	return true;
}
