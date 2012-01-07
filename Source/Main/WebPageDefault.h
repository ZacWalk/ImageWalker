// CommandDefault.h: interface for the WebDefault class.
//
//////////////////////////////////////////////////////////////////////

#pragma once




class WebDefault : public WebHTML
{

public:
	State &_state;

	WebDefault(State &state, WebBuffer* pBuffer) : _state(state), WebHTML(pBuffer)
	{
	}

	~WebDefault()
	{

	}

	HRESULT Generate()
	{
		CString strOut;
		CString strLocation = _T("c:\\");
		int nPage = -1;

		_properties.Read(_T("idx"), nPage);
		_properties.Read(_T("loc"), strLocation);

		CWebSettings settings;
		settings.Read(&_properties);

		CAddressPolicyPreview address(_state, settings);

		CWebPage page(_state);
		page.SetSettings(settings);
		page.GetIndex(&address, strLocation, nPage, strOut);

		_pDataBuffer->Write(strOut);
		_pDataBuffer->Done();

		return S_OK;
	}
};

