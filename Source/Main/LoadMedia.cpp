// LoadMedia.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "LoadMedia.h"
#include "ImageStreams.h"
#include "VideoGraphBuilder.h"

#include <dShow.h>
#include <Qedit.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadMedia::CLoadMedia()
{
}

CLoadMedia::~CLoadMedia()
{
}


bool CLoadMedia::Read(const CString &str,
		IW::IStreamIn *pStreamIn,
		IW::IImageStream *pImageOut,		
		IW::IStatus *pStatus)
{
	USES_CONVERSION;

	/*

	CComPtr<IGraphBuilder> pGB;
	if (FAILED(pGB.CoCreateInstance(CLSID_FilterGraph)))
		return false;

	ATLASSERT(pGB != NULL);

	// Create the "Grabber filter"
	CComPtr<IBaseFilter>    pGrabberBaseFilter;
	CComQIPtr<ISampleGrabber> pSampleGrabber;
	
	if (FAILED(pGrabberBaseFilter.CoCreateInstance(CLSID_SampleGrabber)))
		return false;

	pSampleGrabber = pGrabberBaseFilter;

	if (pSampleGrabber == NULL)
		return false;

	if (FAILED(pGB->AddFilter(pGrabberBaseFilter, L"Grabber")))
		return false;

	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	mt.formattype = FORMAT_VideoInfo; 

	if (FAILED(pSampleGrabber->SetMediaType(&mt)))
		return false;

	if (FAILED(pSampleGrabber->SetOneShot(TRUE)))
		return false;

	if (!pStreamIn->Close(pStatus))
		return false;

	LPCTSTR szFileName = pStreamIn->GetFileName();
	if (szFileName == 0)
		return false;

	if (FAILED(pGB->RenderFile(CT2OLE(szFileName),NULL)))
		return false;
		
	CComQIPtr<IMediaControl> pMediaControl;
	CComQIPtr<IMediaEvent> pMediaEventEx;

	// QueryInterface for some basic interfaces
    pGB->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
    pGB->QueryInterface(IID_IMediaEvent, (void **)&pMediaEventEx);

	if (pMediaControl == NULL || pMediaEventEx == NULL)
		return false;

	// Set up one-shot mode.
	if (FAILED(pSampleGrabber->SetBufferSamples(TRUE)))
		return false;

	if (FAILED(pSampleGrabber->SetOneShot(TRUE)))
		return false;

	CComQIPtr<IMediaSeeking> pSeek = pMediaControl;

	if (pSeek == NULL)
		return false;

	LONGLONG Duration;
	if (FAILED(pSeek->GetDuration(&Duration)))
		return false;

	REFERENCE_TIME rtStart = IW::Min(10000000, Duration);
	REFERENCE_TIME rtStop = IW::Min(10000000, Duration);
			
	if (FAILED(pSeek->SetPositions(&rtStart, AM_SEEKING_SeekToKeyFrame, &rtStop, AM_SEEKING_SeekToKeyFrame )))
		return false;

	CComQIPtr<IVideoWindow> pVideoWindow = pGB;
	if (FAILED(pVideoWindow->put_AutoShow(OAFALSE)))
		return false;

	// Run the graph and wait for completion.
	if (FAILED(pMediaControl->Run()))
		return false;

	long evCode;
	if (FAILED(pMediaEventEx->WaitForCompletion(5000, &evCode)))
	{
		return false;
	}

	pMediaControl->StopWhenReady();
				
	AM_MEDIA_TYPE MediaType;
	ZeroMemory(&MediaType,sizeof(MediaType));
	if (FAILED(pSampleGrabber->GetConnectedMediaType(&MediaType)))
		return false;

	// Get a pointer to the video header.
	VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)MediaType.pbFormat;
	if (pVideoHeader == NULL)
		return false;
	
	long nWidth = pVideoHeader->bmiHeader.biWidth;
	long nHeight = pVideoHeader->bmiHeader.biHeight;
	long nSize = pVideoHeader->bmiHeader.biSizeImage;

	if (FAILED(pSampleGrabber->GetCurrentBuffer(&nSize, (LPLONG)0)))
	{
		return false;
	}

	LPBYTE pBuffer = (LPBYTE)malloc(nSize);

	if (SUCCEEDED(pSampleGrabber->GetCurrentBuffer(&nSize, (LPLONG)(LPBYTE)pBuffer)))
	{
		pImageOut->SetLoaderName(GetKey());

		IW::Image image;	
		image.Copy((const BITMAPINFO *)&(pVideoHeader->bmiHeader), pBuffer, nSize);
		IW::IterateImage(image, *pImageOut, pStatus);
	}

	free(pBuffer);

	*/
	
	CString strFileName = pStreamIn->GetFileName(); 

	if (IW::IsNullOrEmpty(strFileName))
		return false; 

	_pDet = 0;
	
	// See http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmform/htm/sampleapplications.asp
	if (_pDet == NULL)
		if (FAILED(_pDet.CoCreateInstance(__uuidof(MediaDet))))
			return false;
	
	if (FAILED(_pDet->put_Filename(CComBSTR(strFileName))))
		return false;

    long lStreams;
    bool bFound = false;
    _pDet->get_OutputStreams(&lStreams);

    for (long i = 0; i < lStreams; i++)
    {
        GUID major_type;
        _pDet->put_CurrentStream(i);
        _pDet->get_StreamType(&major_type);

        if (major_type == MEDIATYPE_Video)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound) 
		return false; //VFW_E_INVALIDMEDIATYPE;

    long width = 0, height = 0; 

    AM_MEDIA_TYPE mt;
	ZeroMemory(&mt,sizeof(mt));

	double duration = 0.0;
	CComBSTR type;

	_pDet->get_StreamLength(&duration);
	_pDet->get_StreamTypeB(&type);

	if (FAILED(_pDet->get_StreamMediaType(&mt)))
	{
		return false; //?????
	}
    else if (mt.formattype == FORMAT_VideoInfo) 
    {
        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)(mt.pbFormat);
        width = pVih->bmiHeader.biWidth;
        height = pVih->bmiHeader.biHeight;
        
        // We want the absolute height, don't care about orientation.
        if (height < 0) height *= -1;
    }
    else 
	{
        return false; //VFW_E_INVALIDMEDIATYPE; // Should not happen, in theory.
    }

	CString strInformation;
	strInformation.Format(_T("%dx%d video - %.0f seconds"), width, height, duration);

	try 
	{
		long size;
		if (SUCCEEDED(_pDet->GetBitmapBits(0.0, &size, NULL, width, height))) 
		{
			IW::CAutoFree<BYTE> buffer(size + 1024);
			LPBYTE p = (LPBYTE)buffer;

			if (SUCCEEDED(_pDet->GetBitmapBits(0.0, NULL, (char*)p, width, height)))
			{
				LPBITMAPINFOHEADER pBitmap = (LPBITMAPINFOHEADER)p;
				pImageOut->SetLoaderName(GetKey());
				pImageOut->SetStatistics(strInformation);

				IW::CameraSettings settings;
				settings.OriginalImageSize.cx = width;
				settings.OriginalImageSize.cy = height;
				settings.OriginalBpp = IW::PixelFormat::FromBpp(pBitmap->biPlanes * pBitmap->biBitCount);
				pImageOut->SetCameraSettings(settings);

				IW::Image image;	
				image.Copy(pBitmap, size);
				IW::IterateImage(image, *pImageOut, pStatus);
			}
		}	
	}
	catch (...) 
	{
		return false; //E_OUTOFMEMORY;
	}	

	return true;
}
