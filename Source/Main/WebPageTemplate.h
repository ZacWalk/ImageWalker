// CommandTemplate.h: interface for the WebTemplate class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class WebTemplate : public WebHTML  
{
public:
	State &_state;

	WebTemplate(State &state, WebBuffer* pBuffer) : _state(state), WebHTML(pBuffer)
	{

	}

	~WebTemplate()
	{

	}

	HRESULT Generate()
	{
		CString strOutput;
		IW::CFile f;

		IW::CFilePath path, extension;
		path.GetTemplateFolder(App.GetResourceInstance());	

		if (f.OpenForRead(path))
		{
			extension = path;
			extension.StripToExtension();

			DWORD dwLength = f.GetFileSize();
			LPTSTR sz = strOutput.GetBufferSetLength(dwLength);
			f.Read(sz, dwLength);
			strOutput.ReleaseBuffer();

			if (_tcsicmp(extension, _T(".html")) == 0 ||
				_tcsicmp(extension, _T(".htm")) == 0)
			{
				CString strLocation = _T("c:\\");
				_properties.Read(_T("loc"), strLocation);
				path += strLocation;

				CWebSettings settings;
				settings.Read(&_properties);

				CAddressPolicyPreview address(_state, settings);

				CWebPage page(_state);
				page.SetSettings(settings);
				page.ReplaceConstants(strOutput, &address, strLocation, true, true);

				_pDataBuffer->Write(strOutput);
			}
			else
			{
				_pDataBuffer->Write(sz, dwLength);
			}
		}

		_pDataBuffer->Done();

		return S_OK;
	}

};

