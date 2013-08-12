///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadTiff.h: interface for the CLoadTiff class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"

#ifdef COMPILE_TIFF

#ifdef unix
#undef unix
#endif
#ifdef __unix
#undef __unix
#endif

#undef int16

#include "..\Libraries\LibTIFF\tiffiop.h"

class  CLoadTiff :  public CLoad<CLoadTiff>
{
private:
	TIFF *m_hTiffOut;

public:

	CLoadTiff();
	virtual ~CLoadTiff();

	static CString _GetKey() { return _T("TaggedImageFile"); };
	static CString _GetTitle() { return App.LoadString(IDS_TIFF_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_TIFF_DESC); };
	static CString _GetExtensionList() { return _T("TIF,TIFF"); };
	static CString _GetExtensionDefault() { return _T("TIF"); };
	static CString _GetMimeType() { return _T("image/tiff"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::ALPHA | IW::ImageLoaderFlags::METADATA | IW::ImageLoaderFlags::SAVE; };

	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);

	bool AddMetaDataBlob(const IW::MetaData &data);
};


#endif // COMPILE_TIFF