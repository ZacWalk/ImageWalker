#pragma once

#include "Load.h"
interface IMediaDet;

class  CLoadMedia : public CLoad<CLoadMedia>
{
protected:

	CComPtr<IMediaDet> _pDet;

public:
	CLoadMedia();
	virtual ~CLoadMedia();
	
	static CString _GetKey() { return _T("WindowsMediaFile"); };
	static CString _GetTitle() { return App.LoadString(IDS_MEDIA_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_MEDIA_DESC); };
	static CString _GetExtensionList() { return _T("AVI,ASF,MPE,MPEG,MPG,ASF,WMA,WMV"); };
	static CString _GetExtensionDefault() { return _T("AVI"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::THUMBONLY | IW::ImageLoaderFlags::MEDIA; };
	static CString _GetMimeType() { return _T("image/bmp"); };

	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);
};