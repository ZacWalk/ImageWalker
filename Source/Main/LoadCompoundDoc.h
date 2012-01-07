// LoadCompoundDoc.h: interface for the CLoadCompoundDoc class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"

class  CLoadCompoundDoc : public CLoad<CLoadCompoundDoc>
{
public:
	CLoadCompoundDoc();
	virtual ~CLoadCompoundDoc();

	static CString _GetKey() { return _T("CompoundDocFile"); };
	static CString _GetTitle() { return App.LoadString(IDS_CDF_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_CDF_DESC); };
	static CString _GetExtensionList() { return _T("PPS,PPT,VSD"); };
	static CString _GetExtensionDefault() { return _T("PPS"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::THUMBONLY; };
	static CString _GetMimeType() { return _T("image/bmp"); };

	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);
};
