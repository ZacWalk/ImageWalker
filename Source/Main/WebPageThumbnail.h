// CommandThumbnail.h: interface for the WebThumbnail class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "LoadAny.h"

class StreamBuffer : public IW::IStreamOut
{

	IW::RefPtr<WebBuffer> _pDataBuffer;

public:

	StreamBuffer(WebBuffer* pBuffer) : _pDataBuffer(pBuffer)
	{
	} 

	virtual ~StreamBuffer() 
	{  
	}

	DWORD Seek(ePosition ePos, LONG nOff)
	{
		return 0;
	}

	IW::FileSize GetFileSize()
	{
		return IW::FileSize();
	}

	CString GetFileName()
	{
		return g_szEmptyString;
	}

	bool Abort()
	{
		return true;
	}

	bool Close(IW::IStatus *pStatus)
	{
		return true;
	}

	bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0)
	{
		_pDataBuffer->Write(lpBuf, nCount);
		if (pdwWritten) *pdwWritten = nCount;
		return true;
	}

	bool Flush()
	{
		return true;
	}

};

class WebThumbnail : public Web  
{
public:
	State &_state;

	WebThumbnail(State &state, WebBuffer* pBuffer) : _state(state), Web(pBuffer)
	{
	}

	~WebThumbnail()
	{

	}

	LPCWSTR GetMimeType() { return L"image/jpeg"; };
	LPCWSTR GetExtension() { return L"jpg"; };

	HRESULT Generate()
	{
		CLoadAny loader(_state.Plugins);

		// Load settings from URL
		CWebSettings settings;
		settings.Read(&_properties);

		CAddressPolicyPreview address(_state, settings);

		// Read the location variable
		CString strLocation = _T("c:\\");
		_properties.Read(_T("loc"), strLocation);

		// File name
		CString strName = g_szEmptyString;
		_properties.Read(g_szName, strName);

		// Background color
		COLORREF clrBG = RGB(0,0,0);
		_properties.Read(_T("bg"), clrBG);

		IW::Image image;
		CWebPage page(_state);
		page.SetSettings(settings);	

		if (page.GetThumbnail(&loader, strLocation, strName, image, clrBG))
		{
			StreamBuffer stream(_pDataBuffer);
			loader.Write(_T("JPG"), &stream, image, IW::CNullStatus::Instance);
		}	

		_pDataBuffer->Done();

		return S_OK;
	}

};
