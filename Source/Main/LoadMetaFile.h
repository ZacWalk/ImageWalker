///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadMetaFile.h: interface for the CLoadMetaFile class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"

class  CLoadMetaFile : public CLoad<CLoadMetaFile>
{
public:

	CLoadMetaFile();
	virtual ~CLoadMetaFile();
		
	static CString _GetKey() { return _T("WindowsMetaFile"); };
	static CString _GetTitle() { return App.LoadString(IDS_WMF_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_WMF_DESC); };
	static CString _GetExtensionList() { return _T("WMF,EMF"); };
	static CString _GetExtensionDefault() { return _T("EMF"); };
	static CString _GetMimeType() { return _T("image/emf"); };
	static DWORD _GetFlags() { return 0; };

	bool Read(LPCBYTE pByte, DWORD nSize, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);

private:

	int ReadEnhancedMetaFile(IW::IImageStream* pImageOut, LPCBYTE pByte, DWORD nSize);
	int ReadMetaFilePict(IW::IImageStream* pImageOut, LPCBYTE pByte, DWORD nSize);
	int ReadMetaFile(IW::IImageStream* pImageOut, LPCBYTE pByte, DWORD nSize);
};
