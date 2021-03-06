// CommandIndex.h: interface for the WebIndex class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class WebIndex : public WebHTML
{

public:
	State &_state;

	WebIndex(State &state, WebBuffer* pBuffer) : _state(state), WebHTML(pBuffer)
	{
	}

	~WebIndex()
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

