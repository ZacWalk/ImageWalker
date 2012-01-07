///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadTiff.cpp: implementation of the CLoadPsd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadPSD.h"
#include "ImageStreams.h"

#ifdef COMPILE_PSD

CLoadPsd::CLoadPsd()
{
}

CLoadPsd::~CLoadPsd()
{
}

typedef enum
{
  BitmapMode = 0,
  GrayscaleMode = 1,
  IndexedMode = 2,
  RGBMode = 3,
  CMYKMode = 4,
  MultichannelMode = 7,
  DuotoneMode = 8,
  LabMode = 9
} PSDImageType;


static LPCTSTR g_szModes[] = { 
	_T("Bitmap"),
	_T("Grayscale"),
	_T("Indexed"),
	_T("RGB"),
	_T("CMYK"),
	_T("??"),
	_T("??"),
	_T("Multichannel"),
	_T("Duotone"),
	_T("Lab")
};




class CSetRGBPixel
{
public:

	inline DWORD operator()(DWORD c, int pixel, const int channel)
	{
		// case -1  transparency mask
		// case 0	first component (Red, Cyan, Gray or Index)
		// case 1:  second component (Green, Magenta, or opacity)
		// case 2:  third component (Blue or Yellow)
		// case 3:  fourth component (Opacity or Black)
		// case 4:  fifth component (opacity)

		DWORD bb = IW::GetR(c);
		DWORD gg = IW::GetG(c); 
		DWORD rr = IW::GetB(c);
		DWORD aa = IW::GetA(c);

		switch (channel)
		{
		case -1:
			{
				aa=ScaleCharToQuantum(pixel);
			}
		case 0:
			{
				rr=ScaleCharToQuantum(pixel);
				break;
			}
		case 1:
			{
				gg=ScaleCharToQuantum(pixel);
			}
		case 2:
			{
				bb=ScaleCharToQuantum(pixel);
				break;
			}
		case 3:
			{
				aa=ScaleCharToQuantum(pixel);
				break;
			}
		case 4:
			{
				aa=ScaleCharToQuantum(pixel);
				break;
			}
		default:
			break;
		}

		return IW::RGBA(bb, gg, rr, aa);	
	}

};

class CSetGrayScalePixel
{
public:

	inline DWORD operator()(DWORD c, int pixel, const int channel)
	{
		return pixel;
	}
};




template<class TSetPixelPred>
bool DecodeRLEImage(IW::Page &page, IW::IStreamIn *pStreamIn, const int channel)
{
	int cx = page.GetWidth();
	int cy = page.GetHeight();

	TSetPixelPred SetPixel;

	int	number_pixels = cy * cx;
	DWORD pixel;
	bool bReading = false;
	int count = 0;
	int x = 0, y = 0;

	IW::IImageSurfaceLockPtr pLock = page.GetSurfaceLock();

	IW::CAutoFree<COLORREF> pBuffer(cx);	
	pLock->GetLine(pBuffer, y, 0, cx);

	while (number_pixels > 0)
	{
		if (count <= 0)
		{
			count = ReadBlobByte(pStreamIn);

			if (count >= 128)
				count-=256;

			if (count == -128)
			{
				continue;
			}
			else if (count < 0)
			{
				pixel = ReadBlobByte(pStreamIn);
				count=(-count+1);
				bReading = false;
			}
			else
			{
				count++;
				bReading = true;
			}
		}

		if (bReading)
		{
			pixel = ReadBlobByte(pStreamIn);
		}

		//SetPixel(pLock, (x % cx), (x/cx), pixel, channel);	
		int xx = (x % cx);
		pBuffer[xx] = SetPixel(pBuffer[xx], pixel, channel);

		x++;
		number_pixels--;
		count--;

		if ((x % cx) == 0)
		{
			pLock->SetLine(pBuffer, y, 0, cx);
			y++;
			if (cy > y) pLock->GetLine(pBuffer, y, 0, cx);
		}	
	}

	// Guarentee the correct number of pixel packets.
	if (number_pixels != 0)
	{
		throw IW::invalid_file();
	}

	return true;
}

template<class TSetPixelPred>
bool DecodeImage(IW::Page &page, IW::IStreamIn *pStreamIn, const int channel)
{
	int cx = page.GetWidth();
	int cy = page.GetHeight();

	TSetPixelPred SetPixel;
	
	IW::CAutoFree<COLORREF> pBuffer(cx);
	IW::CAutoFree<BYTE> pBufferIn(cx);

	IW::IImageSurfaceLockPtr pLock = page.GetSurfaceLock();

	// Read uncompressed pixel data as separate planes.
	for (int y=0; y < cy; y++)
	{
		pLock->GetLine(pBuffer, y, 0, cx);

		if (!pStreamIn->Read(pBufferIn, cx, NULL))
		{
			return false;
		}

		for (int x=0; x < (long) cx; x++)
		{
			pBuffer[x] = SetPixel(pBuffer[x], pBufferIn[x], channel);
		}

		pLock->SetLine(pBuffer, y, 0, cx);
	}

	return true;
}


static bool lab2rgb(IW::Image &imageIn)
{
	int bb, gg, rr, aa; 
	int l, a, b;

	for(IW::Image::PageList::iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		if (pageIn->GetPixelFormat().HasPalette()) return false;

		IW::IImageSurfaceLockPtr pLock = pageIn->GetSurfaceLock();

		const unsigned nHeight = pageIn->GetHeight();
		const unsigned nWidth = pageIn->GetWidth();

		unsigned y, x;
		unsigned c;

		IW::CAutoFree<COLORREF> pBuffer(nWidth);
		
		for(y = 0; y < nHeight; y++)
		{
			pLock->GetLine(pBuffer, y, 0, nWidth);
		
			for(x = 0; x < nWidth; x++)
			{
				c = pBuffer[x];

				b = IW::GetR(c);
				a = IW::GetG(c); 
				l = IW::GetB(c);

				IW::LABtoRGB(l, a, b, rr, gg, bb);
				aa = 255;
				pBuffer[x] = IW::RGBA(bb, gg, rr, aa);
			}			

			pLock->SetLine(pBuffer, y, 0, nWidth);
		}
	}

	return true;
}

static bool cmyk2rgb(IW::Image &imageIn)
{
	DWORD bb, gg, rr, aa;

	for(IW::Image::PageList::iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		IW::IImageSurfaceLockPtr pLock = pageIn->GetSurfaceLock();
		if (pageIn->GetPixelFormat().HasPalette()) return false;

		const unsigned nHeight = pageIn->GetHeight();
		const unsigned nWidth = pageIn->GetWidth();

		unsigned y, x;
		unsigned c;

		IW::CAutoFree<COLORREF> pBuffer(nWidth);
		
		for(y = 0; y < nHeight; y++)
		{
			pLock->GetLine(pBuffer, y, 0, nWidth);
		
			for(x = 0; x < nWidth; x++)
			{
				c = pBuffer[x];

				bb = IW::GetR(c);
				gg = IW::GetG(c); 
				rr = IW::GetB(c);
				aa = IW::GetA(c);

				rr = IW::ByteClamp(255 - (rr + aa));
				gg = IW::ByteClamp(255 - (gg + aa));
				bb = IW::ByteClamp(255 - (bb + aa));
				aa = 255;

				pBuffer[x] = IW::RGBA(bb, gg, rr, aa);
			}			

			pLock->SetLine(pBuffer, y, 0, nWidth);
		}
	}

	return true;
}


typedef struct _ChannelInfo
{
	short int type;
	unsigned long size;
} ChannelInfo;

typedef struct _RectangleInfo
{
	unsigned int  width,  height;
	int x, y;
} RectangleInfo;

typedef struct _LayerInfo
{
	RectangleInfo page, mask;
	unsigned short channels;
	ChannelInfo channel_info[24];
	char blendkey[4];
	Quantum opacity;
	unsigned char clipping, visible, flags;
	unsigned long offset_x, offset_y;
	unsigned char name[256];
} LayerInfo;

typedef struct _PSDInfo
{
	char signature[4];
	unsigned short channels, version;
	unsigned char reserved[6];
	unsigned long rows, columns;
	unsigned short depth, mode;
} PSDInfo;


bool CLoadPsd::Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
{
	IW::Image imageTemp;

	try
	{
		int number_layers = 0;
		PSDInfo psd_info;
		int i;
		unsigned int skip_first_alpha = 0;
		unsigned short compression;

		//  Open image file.
		bool bSuccess = SUCCEEDED(pStreamIn->Read(psd_info.signature, 4, NULL));
		psd_info.version=ReadMSBShort(pStreamIn);

		if ((memcmp(psd_info.signature,"8BPS",4) != 0) || (psd_info.version != 1)) 
		{
			throw IW::invalid_file();
		}

		bSuccess = SUCCEEDED(pStreamIn->Read(psd_info.reserved, 6, 0));
		if (!bSuccess) 
		{
			throw IW::invalid_file();
		}

		psd_info.channels=ReadMSBShort(pStreamIn);
		psd_info.rows=ReadMSBLong(pStreamIn);
		psd_info.columns=ReadMSBLong(pStreamIn);
		psd_info.depth=ReadMSBShort(pStreamIn);
		psd_info.mode=ReadMSBShort(pStreamIn);

		IW::PixelFormat pf = IW::PixelFormat::FromBpp(psd_info.depth);
		int cx = psd_info.columns;
		int cy = psd_info.rows;
		bool bHasAlpha = false;
		bool bSingleChannel = false;

		// cannot yet handel all modes
		switch(psd_info.mode) 
		{
		case BitmapMode:
			pf = IW::PixelFormat::PF1;
			break;

		case RGBMode:
		case LabMode:
			bHasAlpha = (psd_info.channels >= 4);
			pf =  bHasAlpha ? IW::PixelFormat::PF32Alpha : IW::PixelFormat::PF24;
			break;

		case CMYKMode:
			bHasAlpha = (psd_info.channels >= 5);
			pf =  bHasAlpha ? IW::PixelFormat::PF32Alpha : IW::PixelFormat::PF24;
			break;
		
		case GrayscaleMode:
			pf = IW::PixelFormat::PF8GrayScale;
			bSingleChannel = true;
			break;
		case IndexedMode:
		case MultichannelMode:
		case DuotoneMode:
			pf = IW::PixelFormat::PF8;
			bSingleChannel = true;
			break;
		}
		

		// Read PSD raster colormap only present for indexed and duotone images.
		int nLength = ReadMSBLong(pStreamIn);
		IW::CAutoFree<BYTE> pBuffer(nLength + 1);
		unsigned nColors = 0;

		if (nLength != 0)
		{			
			pStreamIn->Read(pBuffer, nLength, 0);

			if (psd_info.mode == DuotoneMode)
			{
				// Duotone image data;  the format of this data is undocumented.			
			}
			else
			{
				// Read PSD raster colormap.
				nColors = nLength/3;
			}			
		}

		IW::MetaData iptc(IW::MetaDataTypes::PROFILE_IPTC);
		IW::MetaData xmp(IW::MetaDataTypes::PROFILE_XMP);

		// IPTC
		int nIptcLength = ReadMSBLong(pStreamIn);

		if (nIptcLength > 6)
		{
			IW::CAutoFree<BYTE> pBuffer(nIptcLength);
			bSuccess = SUCCEEDED(pStreamIn->Read(pBuffer, nIptcLength, 0));
			
			if (!bSuccess || (memcmp(pBuffer, "8BIM",4) != 0)) 
			{
				throw IW::invalid_file();
			}

			LPCBYTE p = (LPCBYTE)pBuffer;

			// Find the beginning of the IPTC portion of the binary data.
			for (i = 0;  i < (nIptcLength - 1); ++i)
			{
				if ((p[i] == 0x1c) && (p[i + 1] == 0x02))
					break;
			}

			int nSize = nIptcLength - i;

			if (nSize > 0)
			{
				iptc.CopyData(((LPCBYTE)pBuffer) + i, nSize);				
			}
		}

		// Layer and mask block.		
		number_layers=0;
		nLength=ReadMSBLong(pStreamIn);
		if (nLength == 8)
		{
			nLength=ReadMSBLong(pStreamIn);
			nLength=ReadMSBLong(pStreamIn);
		}
		if (nLength != 0)
		{
			pStreamIn->Seek(IW::IStreamCommon::eCurrent, nLength);
		} 
		else 
		{
			//  image has no layers?
		}

		
		if (number_layers == 0)
		{	
			IW::Page &page = imageTemp.CreatePage(cx, cy, pf);

			if (nColors)
			{
				COLORREF *palette = page.GetPalette(); 
					
				for (unsigned i=0; i < (unsigned)IW::Min(nColors, 256u); i++)
				{
					palette[i] = RGB(
						pBuffer[i + (2 * nColors)], 
						pBuffer[i + nColors], 
						pBuffer[i]);
				}
			}

			// Read the precombined image, present for PSD < 4 compatibility
			compression=ReadMSBShort(pStreamIn);
			if (compression == 1)
			{
				// Read Packbit encoded pixel data as separate planes.
				for (i=0; i < (long) (cy*psd_info.channels); i++)
					ReadMSBShort(pStreamIn);

				
				if (bSingleChannel)
				{
					DecodeRLEImage<CSetGrayScalePixel>(page, pStreamIn, i);
				}
				else
				{
					for (i=0; i < psd_info.channels; i++)
					{
						DecodeRLEImage<CSetRGBPixel>(page, pStreamIn, i);
					}
				}
			}
			else
			{
				// Read uncompressed pixel data as separate planes.
				IW::CAutoFree<COLORREF> pBuffer(cx);	

				if (bSingleChannel)
				{
					DecodeImage<CSetGrayScalePixel>(page, pStreamIn, i);
				}
				else
				{
					for (i=0; i < psd_info.channels; i++)
					{
						DecodeImage<CSetRGBPixel>(page, pStreamIn, i);
					}
				}
			}

			
		}

		if (psd_info.mode == CMYKMode)
		{
			// Convert to rgb
			cmyk2rgb(imageTemp);
		}
		else if (psd_info.mode == LabMode)
		{
			lab2rgb(imageTemp);
		}		

		IW::IterateImage(imageTemp, *pImageOut, IW::CNullStatus::Instance);

		if (!imageTemp.IsEmpty())
		{
			CString strInformation;
			strInformation.Format(IDS_PSD_FMT, cx, cy, pf.ToBpp(), 
				(compression == 1) ? _T(" RLE") : _T(" "),
				g_szModes[psd_info.mode]);

			// Information
			pImageOut->SetStatistics(strInformation);
			pImageOut->SetLoaderName(GetKey());	

			IW::CameraSettings settings;
			settings.OriginalImageSize.cx = cx;
			settings.OriginalImageSize.cy = cy;
			settings.OriginalBpp = pf;
			pImageOut->SetCameraSettings(settings);
			pImageOut->AddMetaDataBlob(iptc, xmp);
		}
	} 
	catch(std::exception &)
	{
		//pStatus->SetError(e.what());
		//ATLTRACE(_T("Exception in CLoadPsd::Read %s"), e.what());

		return false;
	}
	
	return true;
}




bool CLoadPsd::Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus)
{
	return false;
}


bool CLoadPsd::AddMetaDataBlob(IW::MetaData &data)
{
	DWORD dwType = data.GetType();
	LPCBYTE p = data.GetData();
	DWORD dwSize = data.GetDataSize();

	/*
	if (dwType == IW::MetaDataTypes::PROFILE_IPTC)
	{
	WriteNewsProfile(m_hTiffOut, p, dwSize);
	}
	else if (dwType == IW::MetaDataTypes::PROFILE_ICC)
	{
	TIFFSetField(m_hTiffOut, TIFFTAG_ICCPROFILE, dwSize, p);
	}
	*/

	return true;
}


#endif // COMPILE_PSD