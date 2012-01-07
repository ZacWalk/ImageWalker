// CommandImage.h: interface for the WebImage class.
//
//////////////////////////////////////////////////////////////////////

#pragma once



class WebImage : public WebHTML  
{
public:

	State &_state;

	WebImage(State &state, WebBuffer* pBuffer) : _state(state), WebHTML(pBuffer)
	{
	}

	~WebImage()
	{

	}


	HRESULT Generate()
	{
		try
		{
			CString strOut;
			int nPage = -1;
			CString strName = g_szEmptyString;
			CString strLocation = _T("c:\\");	

			_properties.Read(g_szName, strName);
			_properties.Read(_T("idx"), nPage);
			_properties.Read(_T("loc"), strLocation);

			CWebSettings settings;
			settings.Read(&_properties);

			CAddressPolicyPreview address(_state, settings);

			CWebPage page(_state);
			page.SetSettings(settings);
			page.GetImage(&address, strLocation, strName, nPage, strOut);

			_pDataBuffer->Write(strOut);

		}
		catch(std::exception &)
		{
			ATLTRACE(_T("Exception in WebImage::Generate"));
		}


		_pDataBuffer->Done();

		return S_OK;
	}

};
