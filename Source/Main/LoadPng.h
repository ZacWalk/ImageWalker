///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadPng.h: interface for the CLoadPng class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"

#ifdef COMPILE_PNG

//extern "C" {
#define PNG_INTERNAL
#include "..\Libraries\LibPNG\png.h"
#include "..\Libraries\LibPNG\pngpriv.h"
//}

class  CLoadPng : public CLoad<CLoadPng>
{
private:
	png_structp m_png_ptr;
	png_infop m_info_ptr;
	
	static void __cdecl read_data(png_struct *png_ptr, png_byte *data, png_size_t length);
	static void __cdecl user_error_fn(png_structp png_ptr, png_const_charp error_msg);
	static void __cdecl user_warning_fn(png_structp png_ptr, png_const_charp warning_msg);
	static void __cdecl write_data(png_struct *png_ptr, png_byte *data, png_size_t length);
	
public:
	static bool IsPng(LPCBYTE pByte);
	
	CLoadPng();
	virtual ~CLoadPng();	
	
	static CString _GetKey() { return _T("PortableNetworkGraphics"); };
	static CString _GetTitle() { return App.LoadString(IDS_PNG_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_PNG_DESC); };
	static CString _GetExtensionList() { return _T("PNG"); };
	static CString _GetExtensionDefault() { return _T("PNG"); };
	static CString _GetMimeType() { return _T("image/png"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::ALPHA | IW::ImageLoaderFlags::METADATA | IW::ImageLoaderFlags::SAVE | IW::ImageLoaderFlags::HTML; };
	
	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);
	bool Write(IW::IStreamOut *pStreamOut, IW::IStreamIn *pStreamIn, const IW::Image &imageIn, IW::IStatus *pStatus);
	
};


#endif // COMPILE_PNG