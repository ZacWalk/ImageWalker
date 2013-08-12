///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadAny.cpp: implementation of the IW::CLoadAny class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "LoadJpg.h"
#include "LoadBmp.h"
#include "LoadPng.h"
#include "LoadTiff.h"
#include "LoadPSD.h"
#include "LoadWpg.h"
#include "LoadMetaFile.h"
#include "LoadCompoundDoc.h"
#include "LoadAny.h"

//////////////////////////////////////////////////////////////
// Helper to add all the Loader Factories

template<class T>
class ImageLoaderFactory : public IW::IImageLoaderFactory
{
protected:
	CString _strKey;
	CString _strTitle;
	CString _strDescription;
	CString _strExtensionList;
	CString _strExtensionDefault;
	CString _strMimeType;
	DWORD _dwFlags;

public:
	ImageLoaderFactory()
	{
		_strKey = T::_GetKey();
		_strTitle = T::_GetTitle();
		_strDescription = T::_GetDescription();
		_strExtensionList = T::_GetExtensionList();
		_strExtensionDefault = T::_GetExtensionDefault();
		_strMimeType = T::_GetMimeType();
		_dwFlags = T::_GetFlags();
	};

	virtual ~ImageLoaderFactory() 
	{
	};

	// Create the plugin object
	IW::IImageLoader * CreatePlugin() const
	{
		return new IW::RefObj<T>;
	};

	// Settings
	CString GetKey() const
	{
		return _strKey;
	}

	CString GetTitle() const
	{
		return _strTitle;
	}

	CString GetDescription() const
	{
		return _strDescription;
	}

	CString GetExtensionDefault() const
	{
		return _strExtensionDefault;
	}

	CString GetExtensionList() const
	{
		return _strExtensionList;
	}

	CString GetMimeType() const
	{
		return _strMimeType;
	}

	DWORD GetFlags() const
	{
		return _dwFlags;
	}

	DWORD GetIcon() const
	{
		return -1;
	}

	CString GetSection() const
	{
		return _T("Image Loader");
	}


	void OnHelp(HWND hWnd) const
	{
		App.InvokeHelp(hWnd, HELP_IMAGE_LOADER);
	}

};

void InitLoaderFactories(PluginState &plugins)
{
	// Jpeg 
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadJpg>);		
	plugins.RegisterImageLoaderHeaderWord(0xD8FF, _T("JPG"));

	// BMP
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadBmp>);		
	plugins.RegisterImageLoaderHeaderWord(0x4D42, _T("BMP"));


#ifdef COMPILE_PNG

	// PNG
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadPng>);
	plugins.RegisterImageLoaderHeaderWord(0x5089, _T("PNG"));

#endif // COMPILE_PNG

#ifdef COMPILE_TIFF

	// Tif
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadTiff>);
	plugins.RegisterImageLoaderHeaderWord(0x4949, _T("TIF"));
	plugins.RegisterImageLoaderHeaderWord(0x4d4d, _T("TIF"));

#endif // COMPILE_TIFF

#ifdef COMPILE_PSD

	// Psd
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadPsd>);

#endif // COMPILE_PSD

	// Wpg
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadWpg>);

	// WMF
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadMetaFile>);

	// Compound Doc
	plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadCompoundDoc>);

	// Compound Media
	//plugins.RegisterImageLoader( new ImageLoaderFactory<CLoadMedia>);

	
};

bool CLoadAny::LoadImage(const CString &strFileName, IW::IImageStream *pImageOut,	IW::IStatus *pStatus)
{
	bool bRet = false;
	const CString strExt = IW::Path::FindExtension(strFileName);

	IW::CFile file;
	if (file.OpenForRead(strFileName))
	{
		bRet = Read(strExt, &file, pImageOut, pStatus);
		pImageOut->Flush();
	}

	return bRet;
}

bool CLoadAny::SaveImage(const CString &strFileName, IW::Image &image, IW::IStatus *pStatus)
{
	IW::CFileTemp f;
	if (!f.OpenForWrite(strFileName))
	{
		return false;
	}

	if (!Write(IW::Path::FindExtension(strFileName), &f, image, pStatus))
	{
		return false;
	}

	return f.Close(pStatus);
}

