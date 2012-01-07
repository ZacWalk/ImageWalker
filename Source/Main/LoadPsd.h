///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadTiff.h: interface for the CLoadPsd class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"

#ifdef COMPILE_PSD

class  CLoadPsd : public CLoad<CLoadPsd>
{
public:

	CLoadPsd();
	virtual ~CLoadPsd();

	static CString _GetKey() { return _T("PhotoShop"); };
	static CString _GetTitle() { return App.LoadString(IDS_PSD_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_PSD_DESC); };
	static CString _GetExtensionList() { return _T("PSD"); };
	static CString _GetExtensionDefault() { return _T("PSD"); };
	static CString _GetMimeType() { return _T("image/psd"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::ALPHA | IW::ImageLoaderFlags::METADATA; };

	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);

	bool AddMetaDataBlob(IW::MetaData &data);
};

#endif // COMPILE_PSD