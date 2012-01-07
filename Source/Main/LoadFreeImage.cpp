// LoadFreeImage.cpp: implementation of the CLoadFreeImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"

#define FREEIMAGE_LIB
#include "..\Libraries\FreeImage\Source\FreeImage.h"

#include "LoadFreeImage.h"
#include "ImageStreams.h"


////////////////////////////////////////////////////////////////////////////////////////
/// Loader for free image

class  CLoadFreeImage : public CLoadBase 
{
protected:
	FREE_IMAGE_FORMAT m_fif;

public:
	CLoadFreeImage(FREE_IMAGE_FORMAT fif);
	virtual ~CLoadFreeImage();

	mutable CString _strKey;
	mutable CString _strDesc;

	CString GetKey() const
	{		
		return _strKey;
	}

	CString GetTitle() const
	{		
		return _strDesc;
	}	

	CString GetTitleWithSettings() const
	{		
		return _strDesc;
	}

	bool Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus);
};

////////////////////////////////////////////////////////////////////////////////////////
/// Factory for free image

class CLoadFreeImageFactory : public IW::IImageLoaderFactory
{
protected:

	FREE_IMAGE_FORMAT m_fif;

public:
	CLoadFreeImageFactory(FREE_IMAGE_FORMAT fif) : m_fif(fif)
	{
	};

	virtual ~CLoadFreeImageFactory() 
	{  
	};

	// Create the plugin object
	IW::IImageLoader * CreatePlugin() const
	{
		return new IW::RefObj<CLoadFreeImage>(m_fif);
	};

	mutable CString _strKey;
	mutable CString _strDesc;
	mutable CString _strFormat;
	mutable CString _strExtList;
	mutable CString _strMimeType;

	CString GetKey() const
	{
		_strKey = FreeImage_GetFormatFromFIF(m_fif);
		return _strKey;
	}

	CString GetTitle() const
	{
		_strDesc = FreeImage_GetFIFDescription(m_fif);
		return _strDesc;
	}	

	CString GetDescription() const
	{
		_strDesc = FreeImage_GetFIFDescription(m_fif);
		return _strDesc;
	}

	CString GetExtensionDefault() const
	{
		_strFormat = FreeImage_GetFormatFromFIF(m_fif);
		return _strFormat;
	}

	CString GetExtensionList() const
	{
		_strExtList = FreeImage_GetFIFExtensionList(m_fif);
		return _strExtList;
	}

	CString GetMimeType() const
	{
		_strMimeType = FreeImage_GetFIFMimeType(m_fif);
		return _strMimeType;
	}

	DWORD GetFlags() const
	{
		DWORD dw = 0;

		if (FreeImage_FIFSupportsWriting(m_fif))
		{
			dw |= IW::ImageLoaderFlags::SAVE;
		}

		if (m_fif == FIF_GIF || m_fif == FIF_PNG || m_fif == FIF_JPEG)
		{
			dw |= IW::ImageLoaderFlags::HTML;
		}

		return dw;
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
		App.InvokeHelp(IW::GetMainWindow(), HELP_IMAGE_LOADER);
	}
};

//////////////////////////////////////////////////////////////////////
// FreeImage
void LoadFreeImageCodecs(PluginState &plugins)
{
	FREE_IMAGE_FORMAT listFif[] =
	{
		//FIF_BMP,
		//FIF_ICO,
		//FIF_JPEG,
		FIF_JNG,
		FIF_KOALA,
		FIF_LBM,
		//FIF_IFF,
		FIF_MNG,
		FIF_PBM,
		FIF_PBMRAW,
		FIF_PCD,
		FIF_PCX,
		FIF_PGM,
		FIF_PGMRAW,
		//FIF_PNG,
		FIF_PPM,
		FIF_PPMRAW,
		FIF_RAS,
		FIF_TARGA,
		//FIF_TIFF,
		FIF_WBMP,
		//FIF_PSD,
		FIF_CUT,
		FIF_XBM,
		FIF_XPM,
		FIF_DDS,
		FIF_GIF,
		FIF_HDR,
		FIF_FAXG3,
		FIF_SGI,
		FIF_EXR,
		FIF_J2K,
		FIF_JP2,
		FIF_PFM,
		FIF_PICT,
		FIF_RAW,
		FIF_UNKNOWN
	};

	
	for(int i = 0; listFif[i] != FIF_UNKNOWN; i++)
	{
		plugins.RegisterImageLoader(new CLoadFreeImageFactory(listFif[i]));
	}
}

//////////////////////////////////////////////////////////////////////
// Loader



CLoadFreeImage::CLoadFreeImage(FREE_IMAGE_FORMAT fif) : m_fif(fif)
{
	_strKey = FreeImage_GetFormatFromFIF(m_fif);
	_strDesc = FreeImage_GetFIFDescription(m_fif);
	_strDesc = FreeImage_GetFIFDescription(m_fif);
}

CLoadFreeImage::~CLoadFreeImage()
{

}

class CFreeImageIO
{
protected:
	IW::IStreamIn *m_pStreamIn;
	IW::IStreamOut *m_pStreamOut;
	FreeImageIO m_io;
	DWORD m_nCurPos;
	
public:
	CFreeImageIO( IW::IStreamIn *pStreamIn ) : m_pStreamIn(pStreamIn), m_pStreamOut(0), m_nCurPos(0)
	{	
		m_io.read_proc  = _ReadProc;
		m_io.seek_proc  = _SeekProc;
		m_io.tell_proc  = _TellProc;
		m_io.write_proc = _WriteProc;
	}

	CFreeImageIO( IW::IStreamOut *pStreamOut ) : m_pStreamOut(pStreamOut), m_pStreamIn(0), m_nCurPos(0)
	{	
		m_io.read_proc  = _ReadProc;
		m_io.seek_proc  = _SeekProc;
		m_io.tell_proc  = _TellProc;
		m_io.write_proc = _WriteProc;
	}
	
	FreeImageIO* GetFreeImageIO()
	{
		return &m_io;
	}
	
	// ----- Default IO functions -----------------------
	
	static unsigned DLL_CALLCONV _ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) 
	{
		CFreeImageIO *pThis = reinterpret_cast<CFreeImageIO*>(handle);
		DWORD dw = 0;

		if (pThis->m_pStreamIn)
		{		
			pThis->m_pStreamIn->Read(buffer, size * count, &dw);
			pThis->m_nCurPos += dw;
		}

		return dw/size;
	}
	
	static unsigned DLL_CALLCONV _WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) 
	{
		CFreeImageIO *pThis = reinterpret_cast<CFreeImageIO*>(handle);

		DWORD dw = 0;
		pThis->m_pStreamOut->Write(buffer, size * count, &dw);
		pThis->m_nCurPos += dw;
		return dw/size;
	}
	
	static int DLL_CALLCONV _SeekProc(fi_handle handle, long offset, int origin) 
	{
		CFreeImageIO *pThis = reinterpret_cast<CFreeImageIO*>(handle);
		IW::IStreamCommon::ePosition pos;
		
		switch(origin)
		{
		case SEEK_CUR:
			pos = IW::IStreamCommon::eCurrent;
			break;
		case SEEK_END:
			pos = IW::IStreamCommon::eEnd;
			break;
		case SEEK_SET:
		default:
			pos = IW::IStreamCommon::eBegin;
			break;
		}
		
		if (pThis->m_pStreamIn != 0)
		{
			pThis->m_nCurPos = pThis->m_pStreamIn->Seek(pos, offset);
		}
		else
		{
			pThis->m_nCurPos = pThis->m_pStreamOut->Seek(pos, offset);
		}

		return pThis->m_nCurPos;
	}
	
	static long DLL_CALLCONV _TellProc(fi_handle handle) 
	{
		CFreeImageIO *pThis = static_cast<CFreeImageIO*>(handle);		
		return pThis->m_nCurPos;
	}
	
};


bool CLoadFreeImage::Read(
		const CString &str,
		IW::IStreamIn *pStreamIn,
		IW::IImageStream *pImageOut,		
		IW::IStatus *pStatus)
{
	USES_CONVERSION;

	FIBITMAP *dib = NULL;
	
	try
	{
		CFreeImageIO io(pStreamIn);
		fi_handle handle = &io;
		int flags = _strKey == "RAW" && pImageOut->WantThumbnail() ? RAW_PREVIEW : 0;

		dib = FreeImage_LoadFromHandle(m_fif, io.GetFreeImageIO(), handle, flags);
	}
	catch(const char *sz)
	{
		// Catch error
		FreeImage_Unload(dib); 
		dib = NULL;
		ATLTRACE(_T("Exception in CLoadFreeImage::Read %s"), sz);
	}
		
	if (dib)
	{
		// Image Header
		const BITMAPINFO *pInfo = FreeImage_GetInfo(dib);

		const int nWidth = pInfo->bmiHeader.biWidth;
		const int nHeight = pInfo->bmiHeader.biHeight;

		if (pInfo->bmiHeader.biBitCount > 32)
		{
			FIBITMAP *converted = FreeImage_ConvertTo32Bits(dib);
			FreeImage_Unload(dib);
			dib = converted;
			pInfo = FreeImage_GetInfo(dib);
		}
			
		const IW::PixelFormat pf = IW::PixelFormat::FromBpp(pInfo->bmiHeader.biBitCount);			

		CRect rc(0, 0, nWidth, nHeight);
		pImageOut->CreatePage(rc, pf, true);

		if (pf.HasPalette())
		{
			pImageOut->SetPalette((IW::LPCCOLORREF)FreeImage_GetPalette(dib));
		}

		for(int y = 0; y < nHeight; ++y) 
		{
			pImageOut->SetBitmap(y, FreeImage_GetScanLine(dib, (nHeight-1) - y));
		}

		IW::CameraSettings settings;
		settings.XPelsPerMeter = pInfo->bmiHeader.biXPelsPerMeter;
		settings.YPelsPerMeter = pInfo->bmiHeader.biYPelsPerMeter;
		settings.OriginalImageSize.cx = nWidth;
		settings.OriginalImageSize.cy = nHeight;
		settings.OriginalBpp = pf;

		pImageOut->SetCameraSettings(settings);

		CString str;
		str.Format(_T("%dx%dx%d %s"), nWidth, nHeight, pf.ToBpp(), CA2T(FreeImage_GetFIFDescription(m_fif)));
		
		pImageOut->SetStatistics(str);
		pImageOut->SetLoaderName(GetKey());
		pImageOut->Flush();
		
		// Free free image
		FreeImage_Unload(dib);
	}
	else
	{
		return false;
	}

	return true;
}



bool CLoadFreeImage::Write(const CString &strType, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus)
{
	CString str;
	str.Format(IDS_ENCODING_FMT, GetTitle());
	pStatus->SetStatusMessage(str);

	IW::Page page = imageIn.GetFirstPage();


	bool bSuccess = false;
    int nPaletteEntries = 0;
	IW::PixelFormat pf = page.GetPixelFormat();
	int cx = page.GetWidth();
	int cy = page.GetHeight();
	int nStorageWidth = IW::CalcStorageWidth(cx, pf);

	FIBITMAP *dib = FreeImage_ConvertFromRawBits(
		(BYTE*)page.GetBitmap(), 
		cx, cy, 
		nStorageWidth, pf.ToBpp(), 
		0xff0000, 0x00ff00, 0x0000ff, 
		FALSE);

	if (dib)
	{
		try
		{
			CFreeImageIO io(pStreamOut);
			fi_handle handle = &io;

			if (FreeImage_FIFSupportsExportBPP(m_fif, pf.ToBpp()))
			{
				bSuccess = FreeImage_SaveToHandle(m_fif, dib, io.GetFreeImageIO(), handle, 0) != 0;
			}
			else
			{
				if (FreeImage_FIFSupportsExportBPP(m_fif, 8))
				{
					FIBITMAP *fib8 = FreeImage_ColorQuantize(dib, FIQ_WUQUANT);
					bSuccess = FreeImage_SaveToHandle(m_fif, fib8, io.GetFreeImageIO(), handle, 0) != 0;
					FreeImage_Unload(fib8);
				}
				else if (FreeImage_FIFSupportsExportBPP(m_fif, 1))
				{
					FIBITMAP *fib1 = FreeImage_Dither(dib, FID_FS);
					bSuccess = FreeImage_SaveToHandle(m_fif, fib1, io.GetFreeImageIO(), handle, 0) != 0;
					FreeImage_Unload(fib1);
				}
				else
				{
					bSuccess = false;
				}
			}
		}
		catch(const char *sz)
		{
			ATLTRACE(_T("Exception in CLoadFreeImage::Write %s"), sz);
		}

		// Free free image
		FreeImage_Unload(dib);
	}

    return bSuccess;
}

