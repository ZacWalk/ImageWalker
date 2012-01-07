///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadMetaFile.cpp: implementation of the CLoadMetaFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadMetaFile.h"

#define RECTWIDTH(lprc)     ((lprc)->right - (lprc)->left)
#define RECTHEIGHT(lprc)    ((lprc)->bottom - (lprc)->top)

//
// Aldus Placeable Metafile (APM) definitions
//

#define APM_SIGNATURE 0x9AC6CDD7

#ifndef RC_INVOKED       // RC can't handle #pragmas
#pragma pack(2)
typedef struct tagRECTS
{
   short   left;
   short   top;
   short   right;
   short   bottom;
} RECTS, *PRECTS;

typedef struct tagAPMFILEHEADER
{
   DWORD   key;
   WORD    hmf;
   RECTS   bbox;
   WORD    inch;
   DWORD   reserved;
   WORD    checksum;
} APMFILEHEADER, *PAPMFILEHEADER;
#pragma pack()
#endif  /* !RC_INVOKED */


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadMetaFile::CLoadMetaFile()
{

}

CLoadMetaFile::~CLoadMetaFile()
{

}


bool CLoadMetaFile::Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut,	IW::IStatus *pStatus)
{
	IW::MetaData data;
	HRESULT hr = data.LoadFromStream(pStreamIn);

	if (FAILED(hr)) 
		return false;

	return Read(data.GetData(), data.GetDataSize(), pImageOut, pStatus);
}

bool CLoadMetaFile::Read(LPCBYTE pByte,	DWORD nSize, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
{
	// First check to see if it is an enhanced metafile
	//
	if ((nSize > sizeof(ENHMETAHEADER)) 
		&& (((ENHMETAHEADER*)pByte)->dSignature == ENHMETA_SIGNATURE))
	{
		return ReadEnhancedMetaFile(pImageOut, pByte, nSize) != 0;
	}
	//
	// Not an enhanced metafile... check for Aldus Placeable Metafile (APM)
	//
	else  if ((nSize > (sizeof(APMFILEHEADER) + sizeof(METAHEADER))) &&
		*((LPDWORD)pByte) == APM_SIGNATURE)
	{
		return ReadMetaFilePict(pImageOut, pByte, nSize) != 0;
	}
	//
	// Not enhanced or placeable... must be a Windows 3x metafile (we hope)
	//
	else if (nSize > sizeof(METAHEADER))// && (*((LPWORD)pByte)) == 2)
	{
		return ReadMetaFile(pImageOut, pByte, nSize) != 0;
	}
		
	return FALSE;
}

int CLoadMetaFile::ReadMetaFile(IW::IImageStream* pImageOut, LPCBYTE pByte, DWORD nSize)
{
	CRect r(0,0,640, 480);
	//HMETAFILE hemf = SetWinMetaFileBits(nSize,
	//	(LPBYTE)pByte, adc, NULL);

	HMETAFILE hmf = SetMetaFileBitsEx(nSize, (PBYTE)pByte);

	// We dont know the extent of this meta file?
	//dt._imageInformation = _T("Standard Windows Metafile");

	if (hmf)
	{
		const int cx = 640;
		const int cy = 480;

		IW::CRender render;
		CWindowDC dc(IW::GetMainWindow());

		if (render.Create(NULL, r))
		{
			render.Play(hmf, cx, cy);
			render.RenderToSurface(pImageOut);
		}

		DeleteMetaFile(hmf);

		// Information
		pImageOut->SetStatistics(GetTitle());
		pImageOut->SetLoaderName(GetKey());

		return TRUE;
	}

	return FALSE;
}

int CLoadMetaFile::ReadMetaFilePict(IW::IImageStream* pImageOut, LPCBYTE pByte, DWORD nSize)
{
	CRect r(0,0,640, 480);
	CWindowDC dc(NULL);

	PAPMFILEHEADER papm = (PAPMFILEHEADER)pByte;
	PMETAHEADER  pmf = (PMETAHEADER)(pByte + sizeof(APMFILEHEADER));     // Windows MetaFile


	//
	// Set up a METAFILEPICT with MM_ANISOTROPIC (requires xExt and
	// yExt to be in MM_HIMETRIC units).
	//
	// Note: papm->bbox   = bounding rect in metafile units
	//       papm->inch   = # of metafile units per inch
	//       HIMETRICINCH = # of hi-metric units (.01mm) per inch
	//

	int xExt = MulDiv(RECTWIDTH(&papm->bbox), 2540, papm->inch);
	int yExt = MulDiv(RECTHEIGHT(&papm->bbox), 2540, papm->inch);

	r.top = 0;
	r.left = 0;
	int cx = r.right = MulDiv(xExt, dc.GetDeviceCaps(HORZRES), dc.GetDeviceCaps(HORZSIZE)  * 100);
	int cy = r.bottom = MulDiv(yExt, dc.GetDeviceCaps(VERTRES), dc.GetDeviceCaps(VERTSIZE)  * 100);

	HMETAFILE hmf = SetMetaFileBitsEx(nSize - sizeof(APMFILEHEADER), (PBYTE)pmf);

	if (r.right > 2000) r.right = 2000;
	if (r.bottom > 2000) r.bottom = 2000;

	if (hmf)
	{

		IW::CRender render;
		CWindowDC dc(IW::GetMainWindow());

		if (render.Create(dc, r))
		{
			render.Play(hmf, xExt, yExt);
			render.RenderToSurface(pImageOut);
		}

		DeleteMetaFile(hmf);


		// Information
		CString str;
		str.Format(IDS_ALDUS_FMT, cx, cy);

		pImageOut->SetStatistics(str);
		pImageOut->SetLoaderName(GetKey());

		IW::CameraSettings settings;
		settings.OriginalImageSize.cx = cx;
		settings.OriginalImageSize.cy = cy;
		pImageOut->SetCameraSettings(settings);
		pImageOut->Flush();

		return TRUE;
	}

	return FALSE;
}

int CLoadMetaFile::ReadEnhancedMetaFile(IW::IImageStream* pImageOut, LPCBYTE pByte, DWORD nSize)
{
	CRect r(0,0,640, 480);

	ENHMETAHEADER emh;
	HENHMETAFILE    hemf = NULL;

	hemf = SetEnhMetaFileBits(nSize, (LPBYTE)pByte);

	if ((hemf) && GetEnhMetaFileHeader(hemf, sizeof(emh), &emh))
	{
		r.left = 0;
		r.top = 0;
		r.right = MulDiv(RECTWIDTH(&emh.rclFrame), emh.szlDevice.cx, emh.szlMillimeters.cx * 100);
		r.bottom = MulDiv(RECTHEIGHT(&emh.rclFrame), emh.szlDevice.cy, emh.szlMillimeters.cy * 100);
	}

	if (r.right > 2000) r.right = 2000;
	if (r.bottom > 2000) r.bottom = 2000;

	CSize size(r.Size());

	// Information
	CString str;
	str.Format(IDS_EMF_FMT, size.cx, size.cy);

	pImageOut->SetStatistics(str);
	pImageOut->SetLoaderName(GetKey());

	IW::CameraSettings settings;
	settings.OriginalImageSize.cx = size.cx;
	settings.OriginalImageSize.cy = size.cy;
	pImageOut->SetCameraSettings(settings);

	if (hemf)
	{
		IW::CRender render;
		CWindowDC dc(IW::GetMainWindow());

		if (render.Create(dc, r))
		{
			render.Play(hemf);
			render.RenderToSurface(pImageOut);
		}

		DeleteEnhMetaFile(hemf);

		return TRUE;
	}

	return FALSE;
}

bool CLoadMetaFile::Write(
		const CString &str,
		IW::IStreamOut *pStreamOut,
		const IW::Image &imageIn,
		
		IW::IStatus *pStatus)
{
	return FALSE;
}
