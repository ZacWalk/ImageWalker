#pragma once

class WebFile : public Web  
{
public:
	State &_state;

	WebFile(State &state, WebBuffer* pBuffer) : _state(state), Web(pBuffer)
	{
	}

	~WebFile()
	{

	}

	LPCWSTR GetMimeType() { return L"text"; };
	LPCWSTR GetExtension() { return L"txt"; };

	HRESULT Generate()
	{
		// Read the location variable
		CString strName = _T("text.txt");
		_properties.Read(_T("name"), strName);

		IW::CFile f;
		IW::CFilePath path;
		path.GetTemplateFolder(App.GetResourceInstance());	
		path += strName;

		if (f.OpenForRead(path))
		{
			CStringA str;
			DWORD dwLength = f.GetFileSize();
			LPSTR sz = str.GetBufferSetLength(dwLength);
			f.Read(sz, dwLength);
			_pDataBuffer->Write(sz, dwLength);
			str.ReleaseBuffer();			
		}

		_pDataBuffer->Done();

		return S_OK;
	}

};