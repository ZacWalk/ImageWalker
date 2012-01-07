///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadBmp.h: interface for the LoadBmp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"

class  CLoadBmp : public CLoad<CLoadBmp>
{
protected:
	void CopyPalette(LPCOLORREF pDst, IW::LPCCOLORREF pSrc, UINT nColorEntries) const;

public:
	CLoadBmp();
	virtual ~CLoadBmp();

	static CString _GetKey() { return _T("MicrosoftWindowsBitmap"); };
	static CString _GetTitle() { return App.LoadString(IDS_BMP_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_BMP_DESC); };
	static CString _GetExtensionList() { return _T("BMP,DIB"); };
	static CString _GetExtensionDefault() { return _T("BMP"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::SAVE; };
	static CString _GetMimeType() { return _T("image/bmp"); };

	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);
};
