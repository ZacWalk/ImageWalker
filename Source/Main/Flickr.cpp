#include "stdafx.h"

#include "Layout.h"
#include "LoadAny.h"
#include "LoadJpg.h"
#include "ProgressDlg.h"
#include "State.h"
#include "Flickr.h"
#include "MultiImageTransform.h"
#include "md5.h"
#include "SpellEdit.h"

static CString boundary = _T("--03DBF17572FD425983C69FC6C0E47208-BOUNDARY");

void FlickrState::ShowErrorMessage(Response &r)
{
	IW::CMessageBoxIndirect mb;
	CString str(_T("Request to Flicker server failed. "));
	mb.Show(str + r._errMessage);
}

bool FlickrState::PromptUser(CString &str)
{
	IW::CMessageBoxIndirect mb;
	return mb.Show(str, MB_ICONQUESTION | MB_OKCANCEL | MB_HELP) == IDOK;
}

void FlickrThread::LoadNewRequest(InterestingImageResponse &request)
{
	if (!request.IsEmpty())
	{
		_state.Flickr.SetNewInterestingImage(request.RequestInterestingImageEntry(), request.GetImage(_state.Plugins));		
	}
}

void FlickrThread::Process()
{
	App.Log(_T("Flickr thread started"));

	if (App.IsOnline())
	{
		InterestingImageResponse request;
		HANDLE objects[2];

		objects[0] = IW::Thread::_eventExit;
		objects[1] = _eventNextImage;

		while(!_bExit)
		{
			DWORD dw = WaitForMultipleObjects(2, objects, FALSE, 60000);

			if (dw == (WAIT_OBJECT_0 + 0))
			{
				break; // Exit
			}
			else if (dw == (WAIT_OBJECT_0 + 1) || dw == WAIT_TIMEOUT)
			{
				if (App.Options.ShowFlickrPicOfInterest)
				{
					if (request.IsEmpty())
					{
						_state.Flickr.RequestInterestingImage(request);	
						request.SelectRandomEntry();
					}

					request.Next();
					LoadNewRequest(request);
				}
			}
		}
	}
}

template<class TTarget>
class XmlParser
{
private:
	XML_Parser  _prs;
	CString _strLastError;
	TTarget &_target;		

public:

	typedef XmlParser<TTarget> ThisClass;

	XmlParser(TTarget &target) : _prs(0), _target(target)
	{
		_prs = XML_ParserCreate(NULL);
	}

	virtual ~XmlParser()
	{
		XML_ParserFree(_prs);
	}

	bool Parse(IW::IStreamIn &stream)
	{
		XML_SetUserData(_prs, this);
		XML_SetElementHandler(_prs, ElementStart, ElementEnd);
		XML_SetCharacterDataHandler(_prs, ElementText);

		char buffer[IW::LoadBufferSize + 1];

		DWORD dwRead = 0;

		while (stream.Read(buffer, IW::LoadBufferSize, &dwRead) )
		{
			if ( dwRead == 0 )
				break;

			buffer[dwRead] = 0;

			if (!XML_Parse(_prs, buffer, dwRead, FALSE)) 
			{
				_strLastError.Format(_T("Parse error at line %d, column %d\n"),
					XML_GetCurrentLineNumber(_prs),
					XML_GetCurrentColumnNumber(_prs));

				return false;
			}
		}

		if (!XML_Parse(_prs, 0, 0, 1)) 
		{
			_strLastError.Format(_T("Parse error at line %d, column %d\n"),
				XML_GetCurrentLineNumber(_prs),
				XML_GetCurrentColumnNumber(_prs));

			return false;
		}

		return true;
	}

	bool ParseURL(const CString &strURL)
	{
		InternetStreamIn stream;

		if (!stream.Open(strURL, true))
			return false;		

		return Parse(stream);
	}

	static void XMLCALL ElementStart(void *data, const XML_Char *name, const XML_Char **atts)
	{
		XmlParser *pThis = reinterpret_cast<ThisClass*>(data);
		pThis->_target.ElementStart(name, atts);
	}

	static void XMLCALL ElementEnd(void *data, const XML_Char *name)
	{
		XmlParser *pThis = reinterpret_cast<ThisClass*>(data);
		pThis->_target.ElementEnd(name);
	}

	static void XMLCALL ElementText(void *data, const XML_Char *txt, int len)
	{
		XmlParser *pThis = reinterpret_cast<ThisClass*>(data);
		pThis->_target.ElementText(txt, len);
	}		
};


class InternetStream : public IW::IStreamIn
{
protected:
	CString _strURL;
	HINTERNET _hSession;
	HINTERNET _http;

public:


	InternetStream() : _hSession(0), _http(0)
	{
		_hSession = InternetOpen(_T("ImageWalker"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);	
	}

	virtual ~InternetStream()
	{
		Close();
	}

	DWORD Seek(IW::IStreamCommon::ePosition ePos, LONG nOff)
	{
		assert(0); // Not supported
		return -1;		
	}

	IW::FileSize GetFileSize()
	{
		assert(_http != NULL);

		// Get the length of the file.            
		TCHAR bufQuery[32] ;
		DWORD dwLengthBufQuery = sizeof(bufQuery);

		BOOL bQuery = ::HttpQueryInfo(_http, HTTP_QUERY_CONTENT_LENGTH, bufQuery, &dwLengthBufQuery, NULL) ;
		return (DWORD)_ttol(bufQuery);
	}

	CString GetFileName()
	{
		return _strURL;
	}

	bool Abort()
	{
		return Close();
	}

	bool Close(IW::IStatus *pStatus = IW::CNullStatus::Instance)
	{
		if (_http) InternetCloseHandle(_http);
		if (_hSession) InternetCloseHandle(_hSession);	
		return true;
	}
};

class InternetStreamIn : public InternetStream
{
public:

	bool Open(const CString &strURL, bool bText = true)
	{
		_strURL = strURL;

		DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;
		flags |= bText ? INTERNET_FLAG_TRANSFER_ASCII : INTERNET_FLAG_TRANSFER_BINARY;

		_http = ::InternetOpenUrl(_hSession, strURL, NULL, 0, flags, 0);

		return _http != NULL;
	}	

	bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0)
	{
		if (nCount == 0) return true;   // avoid Win32 "null-read"
		assert(lpBuf != NULL);

		DWORD dw;
		if (pdwRead == 0) pdwRead = &dw;
		return ::InternetReadFile(_http, lpBuf, nCount, pdwRead) != 0;
	}
};

class UploadResponse : public Response
{
public:
	CString _photoid;		
	CString _ticketid;

	UploadResponse()
	{
	}

	void ElementText(const XML_Char *txt, int len)
	{
		CString str(txt, len);

		if (!str.IsEmpty())
		{
			switch(_state)
			{
			case InPhotoId:
				_photoid += str;
				break;
			case InTicketId:
				_ticketid += str;
				break;
			}
		}
	}	

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("photoid", name))
		{
			_state = InPhotoId;
		}
		else if (IsPropertyName("ticketid", name))
		{
			_state = InTicketId;
		}

		Response::ElementStart(name, atts);
	}
};

class InternetUpload : public InternetStream
{
public:

	typedef InternetStream BaseClass;
	HINTERNET _hRequest;	

	InternetUpload() : _hRequest(0)
	{
	}

	bool Close(IW::IStatus *pStatus = IW::CNullStatus::Instance)
	{
		if (_hRequest) InternetCloseHandle(_hRequest);
		return BaseClass::Close(pStatus);
	}


	bool Open(const CString &strServer)
	{	
		_strURL = strServer;
		_http = ::InternetConnect(_hSession, strServer, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		return _http != NULL;
	}

	bool Post(const CString &strDestination, UploadResponse &response, IW::IStreamIn &streamRequest, CString header, IW::IStatus *pStatus)
	{
		_hRequest = HttpOpenRequest(_http, _T("POST"), strDestination, NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0);

		if (_hRequest)
		{
			WriteRequest(streamRequest, header, pStatus);

			XmlParser<UploadResponse> parser(response);
			parser.Parse(*this);			

			return InternetCloseHandle(_hRequest) != 0;
		}

		return false;
	}

	bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0)
	{
		if (nCount == 0) return true;   // avoid Win32 "null-read"
		assert(lpBuf != NULL);

		DWORD dw;
		if (pdwRead == 0) pdwRead = &dw;
		return ::InternetReadFile(_hRequest, lpBuf, nCount, pdwRead) != 0;
	}

	BOOL WriteRequest(IW::IStreamIn &streamRequest, CString header, IW::IStatus *pStatus)
	{
		DWORD dwSize = streamRequest.GetFileSize();
		DWORD dwTotal = 0;
		DWORD dwRead = 0;

		INTERNET_BUFFERS BufferIn;

		const int nBufferSize = 1024;
		char buffer[nBufferSize + 1];

		BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
		BufferIn.Next = NULL; 
		BufferIn.lpcszHeader = header;
		BufferIn.dwHeadersLength = header.GetLength();
		BufferIn.dwHeadersTotal = header.GetLength();
		BufferIn.lpvBuffer = NULL;                
		BufferIn.dwBufferLength = 0;
		BufferIn.dwBufferTotal = dwSize; // This is the only member used other than dwStructSize
		BufferIn.dwOffsetLow = 0;
		BufferIn.dwOffsetHigh = 0;

		if(HttpSendRequestEx(_hRequest, &BufferIn, NULL, 0, 0))
		{
			while (streamRequest.Read(buffer, nBufferSize, &dwRead))
			{
				if ( dwRead == 0 ) break;
				if ( pStatus->QueryCancel()) break;

				if (!Write(buffer, dwRead)) break;

				dwTotal += dwRead;
				pStatus->Progress(dwTotal, dwSize);					
			}

			return HttpEndRequest(_hRequest, NULL, 0, 0) != 0;
		}

		return false;
	}

	bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0)
	{
		if (nCount == 0) return true;   // avoid Win32 "null-read"
		assert(lpBuf != NULL);

		DWORD dw;
		if (pdwWritten == 0) pdwWritten = &dw;
		return ::InternetWriteFile(_hRequest, lpBuf, nCount, pdwWritten) != 0;
	}
};

/*

class InternetStreamOut : public InternetStream<IW::IStreamOut>
{
public:

bool Open(const CString &strURL)
{
_strURL = szURL;
_http = ::HttpOpenRequest(_hSession, _T("POST"), _strURL, NULL, NULL, NULL,  0, 0);

return _http != NULL;
}

bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0)
{
if (nCount == 0) return true;   // avoid Win32 "null-read"
assert(lpBuf != NULL);

DWORD dw;
if (pdwWritten == 0) pdwWritten = &dw;
return ::InternetWriteFile(_http, lpBuf, nCount, pdwWritten) != 0;
}


bool Flush()
{
return true;
}	
};*/

class RequestUrl
{
public:	

	typedef std::map<CString, CString> ARGMAP;
	std::map<CString, CString> _args;

	RequestUrl(const CString &strMethod)
	{
		_args[_T("api_key")] = GetApiKey();
		_args[_T("method")] = strMethod;
	}

	RequestUrl()
	{
		_args[_T("api_key")] = GetApiKey();
	}

	static CString GetURL() { return _T("http://api.flickr.com/services/rest"); };		
	static CString GetApiKey() { return  _T("8d575fb83a915c6ea1ded6c612067b29"); };	
	static CString GetSecret() { return  _T("cfd22c469d44493f"); };	

	void Add(const CString &strName, const CString &strValue)
	{
		_args[strName] = strValue;
	}

	void Add(const CString &strName, int value)
	{
		Add(strName, IW::IToStr(value));
	}

	CString GetAuthUrl() const
	{
		bool bFirst = true;
		CString str = _T("http://flickr.com/services/auth/?");

		for(ARGMAP::const_iterator it = _args.begin(); it != _args.end(); ++it)
		{
			if (!bFirst)
			{
				str += _T("&");				
			}

			str += it->first;
			str += _T("=");
			str += it->second;

			bFirst = false;
		}

		str += _T("&api_sig=");
		str += GenerateMD5();

		return str;
	}

	CString GetUrl() const
	{
		bool bFirst = true;
		CString str(GetURL());
		str += _T("/?");

		for(ARGMAP::const_iterator it = _args.begin(); it != _args.end(); ++it)
		{
			if (!bFirst)
			{
				str += _T("&");				
			}

			str += it->first;
			str += _T("=");
			str += it->second;

			bFirst = false;
		}

		str += _T("&api_sig=");
		str += GenerateMD5();

		return str;
	}

	CString GenerateMD5() const
	{
		CStringA str(GetSecret());

		for(ARGMAP::const_iterator it = _args.begin(); it != _args.end(); ++it)
		{
			str += it->first;
			str += it->second;
		}

		MD5 md5(str);
		return CString(md5.hex_digest());
	}
};

void FlickrState::OpenImage(const CString &strId)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.photos.getInfo"));
	url.Add(_T("photo_id"), strId);

	PhotoInfoResponse request;
	XmlParser<PhotoInfoResponse> parser(request);
	parser.ParseURL(url.GetUrl());

	IW::NavigateToWebPage(request._url);
}


void FlickrState::DownloadById(const CString &strId)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.photos.getInfo"));
	url.Add(_T("photo_id"), strId);
	url.Add(_T("auth_token"), App.Options.Flickr.Token);

	PhotoInfoResponse request;
	XmlParser<PhotoInfoResponse> parser(request);
	parser.ParseURL(url.GetUrl());

	DownloadByUrl(request.GetDownloadUrl(), strId, request._title, request._tags, request._description);
}

void FlickrState::DownloadById(const CString &strId, const CString &strSecret)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.photos.getInfo"));
	url.Add(_T("photo_id"), strId);
	url.Add(_T("secret"), strSecret);
	url.Add(_T("auth_token"), App.Options.Flickr.Token);

	PhotoInfoResponse request;
	XmlParser<PhotoInfoResponse> parser(request);
	parser.ParseURL(url.GetUrl());

	DownloadByUrl(request.GetDownloadUrl(), strId, request._title, request._tags, request._description);
}

void FlickrState::DownloadByUrl(const CString &strUrl, const CString &strId, const CString &strTitle, const CString &strKeywords, const CString &strDescription)
{
	CProgressDlg pd;
	pd.Create(IW::GetMainWindow(), IDS_DOWNLOADING);
	pd.SetStatusMessage(IDS_DOWNLOADING);	

	InternetStreamIn streamIn;
	if (streamIn.Open(strUrl, false))
	{
		DWORD dwSize = IW::LowerLimit<1024>(streamIn.GetFileSize());

		int nCurrentStep = 1;
		DWORD dwTotal = 0;
		DWORD dwRead = 0;

		const int nBufferSize = 1024;
		char buffer[nBufferSize + 1];

		IW::SimpleBlob data;
		IW::StreamBlob<IW::SimpleBlob> memoryStream(data);			

		while (streamIn.Read(buffer, nBufferSize, &dwRead))
		{
			if ( dwRead == 0 ) break;
			if ( pd.QueryCancel()) break;

			memoryStream.Write(buffer, dwRead);

			dwTotal += dwRead;
			pd.Progress(dwTotal, dwSize);					
		}

		memoryStream.Seek(IW::IStreamCommon::eBegin, 0);

		IW::Image image;
		CLoadAny loader(_state.Plugins);
		IW::ImageStream<IW::IImageStream, false> imageStream(image); 

		if (loader.Read(_T("JPG"), &memoryStream, &imageStream, &pd))
		{
			ImageMetaData properties(image);
			CString strTitleExisting = properties.GetTitle();
			CString strKeywordsExisting = properties.GetTags();
			CString strDescriptionExisting = properties.GetDescription();

			if (strTitleExisting.GetLength() <= strTitle.GetLength()) properties.SetTitle(strTitle);			
			if (strKeywordsExisting.GetLength() <= strKeywords.GetLength()) properties.SetTags(strKeywords);
			if (strDescriptionExisting.GetLength() <= strDescription.GetLength()) properties.SetDescription(strDescription);

			properties.SetFlickrId(strId);
			properties.Apply(image);

			IW::CFileTemp file;
			IW::CFilePath path;
			path.CombinePathAndFilename(_state.Folder.GetFolderPath(), IW::Path::FindFileName(strUrl));

			if (file.OpenForWrite(path))
			{
				loader.Write(_T("JPG"), &file, image, &pd);
			}
		}
	}
}

void FlickrState::SetNewInterestingImage(InterestingImageEntry &entry, IW::Image &image)
{
	_entry = entry;
	_image = image;
	_state.ContextSwitch(NewInterestingImage);
}

void FlickrState::RequestPhotoInfo(PhotoInfoResponse &request, const CString &strPhotoId, const CString &strSecret)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.photos.getInfo"));
	url.Add(_T("photo_id"), strPhotoId);
	url.Add(_T("secret"), strSecret);

	XmlParser<PhotoInfoResponse> parser(request);
	parser.ParseURL(url.GetUrl());
}

void FlickrState::RequestPhotoSetList(PhotoSetResponse &request)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.photosets.getList"));
	url.Add(_T("user_id"), App.Options.Flickr.NSid);

	XmlParser<PhotoSetResponse> parser(request);
	parser.ParseURL(url.GetUrl());
}

void FlickrState::RequestInterestingImage(InterestingImageResponse &request)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.interestingness.getList"));
	url.Add(_T("per_page"), InterestingImageResponse::MaxEntryCount);
	url.Add(_T("extras"), "original_format");
	if (!App.Options.Flickr.Token.IsEmpty()) url.Add(_T("auth_token"), App.Options.Flickr.Token);

	XmlParser<InterestingImageResponse> parser(request);
	parser.ParseURL(url.GetUrl());
}

void FlickrState::RequestFrob(FrobResponse &request)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.auth.getFrob"));
	XmlParser<FrobResponse> parser(request);
	parser.ParseURL(url.GetUrl());
}

void FlickrState::RequestToken(TokenResponse &request, const CString &strFrob)
{
	CWaitCursor wait;		

	RequestUrl url(_T("flickr.auth.getToken"));
	url.Add(_T("frob"), strFrob);

	XmlParser<TokenResponse> parser(request);
	parser.ParseURL(url.GetUrl());
}


IW::Image InterestingImageResponse::GetImage(PluginState &plugins) const
{
	IW::Image image;

	if (_entries.size() > 0)
	{
		InternetStreamIn stream;

		if (stream.Open(_entries[_nCurrentEntry].GetThumbnailURL(), false))
		{					
			CLoadAny loader(plugins);
			IW::ImageStream<IW::IImageStream, false> imageStream(image); 
			loader.Read(_T("JPG"), &stream, &imageStream, IW::CNullStatus::Instance);
		}
	}

	return image;
}

static CString CreateGuid()
{
	GUID g = GUID_NULL;
	::CoCreateGuid(&g);

	const int nBufferSize = 60;
	OLECHAR sz[nBufferSize];
	StringFromGUID2(g, sz, nBufferSize);
	return sz;
}



LPCTSTR g_szFlickrUpload = _T("FlickrUpload");
LPCTSTR g_szResize = _T("Resize");
LPCTSTR g_szResizeSize = _T("ResizeSize");
LPCTSTR g_szPrivate = _T("Private");
LPCTSTR g_szFriends = _T("Friends");
LPCTSTR g_szFamily = _T("Family");
LPCTSTR g_szPublic = _T("Public");
LPCTSTR g_szPhotoSetId = _T("PhotoSetId");

class FlickrUploadSettings
{
public:

	PHOTOSETLIST entries;
	CString selectedPhotoSetId;

	CString strToken;
	CString strTitle;
	CString strDescription;
	CString strTags;
	CString strPath;	

	bool bResize;
	int nResizeSize;
	bool bIsPrivate;
	bool bIsFriend;
	bool bIsFamily;
	bool bIsPublic;

	FlickrUploadSettings() : 
	bResize(false),
		nResizeSize(1024),
		bIsPrivate(false),
		bIsFriend(false),
		bIsFamily(false),
		bIsPublic(true),
		strToken(App.Options.Flickr.Token)
	{
	}

	void Read(const CString &strPathIn, const IW::Image &image)
	{
		strPath = strPathIn;

		ImageMetaData properties(image);
		strTitle = properties.GetTitle();	
		strTags = properties.GetTags();
		strDescription = properties.GetDescription();

		strDescription.Replace(g_szCRLF, g_szCR);
		strDescription.Replace(g_szLF, g_szCR);
		strDescription.Replace(g_szCR, g_szCRLF);

		if (strTitle.IsEmpty()) 
		{
			IW::CFilePath path = IW::Path::FindFileName(strPath);
			path.StripToFilename();
			strTitle = path.ToString();
		}
	}

	void Read(const IW::IPropertyArchive *pArchive)
	{
		if (pArchive && pArchive->StartSection(g_szFlickrUpload))
		{
			pArchive->Read(g_szResize, bResize);
			pArchive->Read(g_szResizeSize, nResizeSize);
			pArchive->Read(g_szPrivate, bIsPrivate);
			pArchive->Read(g_szFriends, bIsFriend);
			pArchive->Read(g_szFamily, bIsFamily);
			pArchive->Read(g_szPublic, bIsPublic);
			pArchive->Read(g_szPhotoSetId, selectedPhotoSetId);
		}
	}

	void Write(IW::IPropertyArchive *pArchive) const
	{
		if (pArchive && pArchive->StartSection(g_szFlickrUpload))
		{
			pArchive->Write(g_szResize, bResize);
			pArchive->Write(g_szResizeSize, nResizeSize);
			pArchive->Write(g_szPrivate, bIsPrivate);
			pArchive->Write(g_szFriends, bIsFriend);
			pArchive->Write(g_szFamily, bIsFamily);
			pArchive->Write(g_szPublic, bIsPublic);
			pArchive->Write(g_szPhotoSetId, selectedPhotoSetId);
		}
	}	
};

CString FormatMultipartEntry(CString strName, CString strValue, CString boundary)
{
	CString dash = CString("--");
	CString crlf = _T("\r\n");
	CString str;

	str += dash + boundary + crlf;
	str += _T("Content-Type: text/plain; charset=\"utf-8\"") + crlf;
	str += _T("Content-Disposition: form-data; name=\"" + strName + "\"") + crlf + crlf;
	str += strValue + crlf;

	return str;
}

CString FormatTagsForFlickr(CString strTagsIn)
{
	TCHAR szSeps[]   = _T(";");
	CString strTagsOut;

	int curPos = 0;
	CString token = strTagsIn.Tokenize(szSeps, curPos);

	while (!token.IsEmpty())
	{
		CString strEntry(token);
		strEntry.Trim();

		if (!strTagsOut.IsEmpty()) strTagsOut += _T(" ");

		if (_tcschr(strEntry, ' ') != NULL)
		{
			strTagsOut += '"';
			strTagsOut += strEntry;
			strTagsOut += '"';
		}
		else
		{
			strTagsOut += strEntry;
		}		

		token = strTagsIn.Tokenize(szSeps, curPos);
	}

	return strTagsOut;
}

void GetUploadBlob(CLoadAny &loader, IW::IStreamOut &stream, const IW::Image &imageIn, FlickrUploadSettings &params, IW::IStatus *pStatus)
{
	CWaitCursor wait;
	RequestUrl args;

	CString crlf = _T("\r\n");    
	CString dash = CString("--");

	CString str;    
	str += FormatMultipartEntry(_T("api_key"), args.GetApiKey(), boundary);

	//args.Add(_T("async"), _T("1"));
	//str += FormatMultipartEntry(_T("async"), _T("1"), boundary);

	args.Add(_T("auth_token"), params.strToken);
	str += FormatMultipartEntry(_T("auth_token"), params.strToken, boundary);	

	if(!params.strTitle.IsEmpty())
	{
		args.Add(_T("title"), params.strTitle);
		str += FormatMultipartEntry(_T("title"), params.strTitle, boundary);
	}

	if(!params.strDescription.IsEmpty())
	{
		args.Add(_T("description"), params.strDescription);
		str += FormatMultipartEntry(_T("description"), params.strDescription, boundary);
	}

	if(!params.strTags.IsEmpty())
	{
		CString strTags = FormatTagsForFlickr(params.strTags);
		args.Add(_T("tags"), strTags);
		str += FormatMultipartEntry(_T("tags"), strTags, boundary);
	}

	args.Add(_T("is_public"), params.bIsPublic? _T("1") : _T("0"));
	str += FormatMultipartEntry(_T("is_public"), (params.bIsPublic ? _T("1") : _T("0")), boundary);

	args.Add(_T("is_family"), params.bIsFamily ? _T("1") : _T("0"));
	str += FormatMultipartEntry(_T("is_family"), (params.bIsFamily ? _T("1") : _T("0")), boundary);

	args.Add(_T("is_friend"), params.bIsFriend ? _T("1") : _T("0"));
	str += FormatMultipartEntry(_T("is_friend"), (params.bIsFriend ? _T("1") : _T("0")), boundary);

	CString md5 = args.GenerateMD5(); 
	str += FormatMultipartEntry(_T("api_sig"), md5, boundary);

	str += dash + boundary + crlf;
	str += _T("Content-Disposition: form-data; name=\"photo\"; filename=\""); + params.strPath + crlf;
	str += _T("Content-Type: image/jpeg") + crlf + crlf;	

	stream.Write((LPCSTR)CStringA(str), str.GetLength());

	IW::Image imageToSave = imageIn;

	if (params.bResize)
	{
		const CRect r = imageToSave.GetBoundingRect();

		if (r.Width() > params.nResizeSize || r.Height() > params.nResizeSize)
		{
			CSize size(params.nResizeSize, params.nResizeSize);

			IW::Image imageOut;
			IW::ImageStreamScale<IW::CNull> thumbnailStream(imageOut, Search::Any, size); 
			IW::IterateImage(imageToSave, thumbnailStream, pStatus);

			imageToSave = imageOut;
		}
	}

	loader.Write(_T("JPG"), &stream, imageToSave, pStatus);

	str = crlf + dash + boundary + _T("--");

	stream.Write((LPCSTR)CStringA(str), str.GetLength());
	stream.Seek(IW::IStreamCommon::eBegin, 0);	
}

void AddPhotoToPhotoSet(CString& photoSetId, CString& photoId)
{
	RequestUrl url(_T("flickr.photosets.addPhoto"));
	url.Add(_T("auth_token"), App.Options.Flickr.Token);
	url.Add(_T("photoset_id"), photoSetId);
	url.Add(_T("photo_id"), photoId);

	Response response;
	XmlParser<Response> parser(response);
	parser.ParseURL(url.GetUrl());
}

bool UploadImage(CLoadAny &loader, const IW::Image &imageIn, FlickrUploadSettings& settings, CString& photoId, IW::IStatus *pStatus)
{
	IW::SimpleBlob data;
	IW::StreamBlob<IW::SimpleBlob> memoryStream(data);

	GetUploadBlob(loader, memoryStream, imageIn, settings, pStatus);

	UploadResponse response;
	InternetUpload uploader;

	bool success = false;

	if (uploader.Open(_T("api.flickr.com")))
	{
		CString header;
		header.Format(_T("Content-Type: multipart/form-data; boundary=%s\n"), boundary);

		pStatus->SetStatusMessage(App.LoadString(IDS_UPLOADING));

		if (uploader.Post(_T("/services/upload/?"), response, memoryStream, header, pStatus))
		{
			if (!response._success)
			{
				FlickrState::ShowErrorMessage(response);
			}
			else 
			{
				CString photoSetId = settings.selectedPhotoSetId;
				photoId = response._photoid;

				if (!photoSetId.IsEmpty())
				{
					AddPhotoToPhotoSet(photoSetId, photoId);
				}

				success = true;
			}
		}
	}
	return success;
}

template<class T>
class CFlickrDlgBase : public CDialogImpl<T>
{
public:
	CString _strAccountMessage;
	FlickrUploadSettings &_settings;

	CFlickrDlgBase(FlickrUploadSettings &settings, const CString &strAccountStatus) : _settings(settings), _strAccountMessage(strAccountStatus)
	{
	}	

	void PopulateSettings()
	{
		T *pT = static_cast<T*>(this);

		pT->CheckDlgButton(IDC_RESIZE, _settings.bResize ? BST_CHECKED : BST_UNCHECKED);

		CComboBox comboResize = pT->GetDlgItem(IDC_RESIZE_LENGTH);
		int resizeValues[] = { 800, 1024, 1280, 1600, 2048, -1 };
		IW::SetItems(comboResize, resizeValues, _settings.nResizeSize);

		CheckDlgButton(IDC_PRIVATE, _settings.bIsPrivate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_FRIENDS, _settings.bIsFriend ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_FAMILY, _settings.bIsFamily ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_PUBLIC, _settings.bIsPublic ? BST_CHECKED : BST_UNCHECKED);


		CComboBox comboPhotoSets = pT->GetDlgItem(IDC_PHOTOSET);

		comboPhotoSets.AddString(_T(" None"));
		comboPhotoSets.SetItemData(0, -1);
		comboPhotoSets.SetCurSel(0);		

		int n = 0;
		for(PHOTOSETLIST::const_iterator i = _settings.entries.begin(); i != _settings.entries.end(); i++)
		{
			CString id = i->id;
			int index = comboPhotoSets.AddString(i->title);
			comboPhotoSets.SetItemData(index, n);

			if (_settings.selectedPhotoSetId == id)
			{
				comboPhotoSets.SetCurSel(index);
			}

			n += 1;
		}
	}

	void DoEnable()
	{
		T *pT = static_cast<T*>(this);

		bool bEnableResize  = BST_CHECKED == pT->IsDlgButtonChecked(IDC_RESIZE);
		bool bEnablePrivate = BST_CHECKED == pT->IsDlgButtonChecked(IDC_PRIVATE);

		pT->GetDlgItem(IDC_RESIZE_LENGTH).EnableWindow(bEnableResize);
		pT->GetDlgItem(IDC_FRIENDS).EnableWindow(bEnablePrivate);
		pT->GetDlgItem(IDC_FAMILY).EnableWindow(bEnablePrivate);
	}	

	void ApplySettings()
	{
		T *pT = static_cast<T*>(this);

		_settings.bResize = BST_CHECKED == pT->IsDlgButtonChecked(IDC_RESIZE);
		_settings.nResizeSize = pT->GetDlgItemInt(IDC_RESIZE_LENGTH);

		_settings.bIsPrivate = BST_CHECKED == pT->IsDlgButtonChecked(IDC_PRIVATE);
		_settings.bIsFriend = BST_CHECKED == pT->IsDlgButtonChecked(IDC_FRIENDS);
		_settings.bIsFamily = BST_CHECKED == pT->IsDlgButtonChecked(IDC_FAMILY);
		_settings.bIsPublic = BST_CHECKED == pT->IsDlgButtonChecked(IDC_PUBLIC);

		CComboBox comboPhotoSets = pT->GetDlgItem(IDC_PHOTOSET);
		int index = comboPhotoSets.GetItemData(comboPhotoSets.GetCurSel());
		_settings.selectedPhotoSetId = (index >= 0) ? _settings.entries[index].id : g_szEmptyString;
	}
};

class CFlickrUploadDlg : public CFlickrDlgBase<CFlickrUploadDlg>
{
public:
	enum { IDD = IDD_FLICKRUPLOAD };

	ImageState &_imageState;
	CSpellEdit _spellEdit;

	CFlickrUploadDlg(ImageState &imageState, FlickrUploadSettings &settings, const CString &strAccountStatus) : 
	_imageState(imageState), CFlickrDlgBase<CFlickrUploadDlg>(settings, strAccountStatus)
	{
	}

	BEGIN_MSG_MAP(CFlickrUploadDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

		COMMAND_ID_HANDLER(IDC_RESIZE, OnEnable)
		COMMAND_ID_HANDLER(IDC_PRIVATE, OnEnable)
		COMMAND_ID_HANDLER(IDC_PUBLIC, OnEnable)

	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());		

		SetDlgItemText(IDC_ACCOUNT, _strAccountMessage);
		SetDlgItemText(IDC_HEADLINE, _imageState.GetTitle());
		SetDlgItemText(IDC_TAGS, _imageState.GetTags());
		SetDlgItemText(IDC_CAPTION, _imageState.GetDescription());

		_spellEdit.SubclassWindow(GetDlgItem(IDC_CAPTION));	
		_settings.strPath = _imageState.GetImageFileName();

		PopulateSettings();
		DoEnable();

		return (LRESULT)TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CString strTitle, strKeywords, strDescription;

		GetDlgItemText(IDC_HEADLINE, strTitle);
		GetDlgItemText(IDC_TAGS, strKeywords);
		GetDlgItemText(IDC_CAPTION, strDescription);

		_settings.strTitle = strTitle;
		_settings.strDescription = strDescription;
		_settings.strTags = strKeywords;
		_settings.strPath = _imageState.GetImageFileName();

		ApplySettings();

		EndDialog(wID);
		return 0;
	}

	LRESULT OnEnable(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DoEnable();
		return 0;
	}
};



class CFlickrMultiUploadDlg : public CFlickrDlgBase<CFlickrMultiUploadDlg>
{
public:
	enum { IDD = IDD_FLICKRUPLOAD_MULTI };

	CFlickrMultiUploadDlg(FlickrUploadSettings &settings, const CString &strAccountStatus) :  
	CFlickrDlgBase<CFlickrMultiUploadDlg>(settings, strAccountStatus)
	{
	}

	BEGIN_MSG_MAP(CFlickrMultiUploadDlg)

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

		COMMAND_ID_HANDLER(IDC_RESIZE, OnEnable)
		COMMAND_ID_HANDLER(IDC_PRIVATE, OnEnable)
		COMMAND_ID_HANDLER(IDC_PUBLIC, OnEnable)

	END_MSG_MAP()



	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());		
		SetDlgItemText(IDC_ACCOUNT, _strAccountMessage);

		PopulateSettings();
		DoEnable();

		return (LRESULT)TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ApplySettings();
		EndDialog(wID);
		return 0;
	}

	LRESULT OnEnable(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DoEnable();
		return 0;
	}
};



CString FlickrState::GetNewToken()
{
	FrobResponse frob;
	RequestFrob(frob);	

	if (!frob._success)
	{
		ShowErrorMessage(frob);
	}
	else
	{
		CString strAsk, strWait;

		strAsk += _T("ImageWalker requires your authorisation before it can read or modify your photos and data on Flickr.\n\n");
		strAsk += _T("Authorizing is a simple process which takes place in your web browser. When you’re finished, return to this window to complete authorization and begin using ImageWalker.\n\n");
		strAsk += _T("You must be connected to the internet in order to authorize this program.");

		RequestUrl url;
		url.Add(_T("perms"), _T("write"));
		url.Add(_T("frob"), frob._frob);

		strWait += _T("Return to this window after you have finished the authorization process on Flickr.com\n\n");
		strWait += _T("Once you’re done, click OK below and you can begin using ImageWalker to upload images to Flickr.\n\n");
		strWait += _T("You can revoke this program’s authorisation at any time in your account page on Flickr.com");

		if (PromptUser(strAsk) && 
			IW::NavigateToWebPage(url.GetAuthUrl()) &&
			PromptUser(strWait))
		{
			TokenResponse token;
			RequestToken(token, frob._frob);

			if (!token._success)
			{
				ShowErrorMessage(token);
			}
			else
			{
				App.Options.Flickr.Token = token._token;
				App.Options.Flickr.NSid = token._nsid;
				App.Options.Flickr.UserName = token._username;
				App.Options.Flickr.FullName = token._fullname;

				return token._token;
			}
		}
	}

	return g_szEmptyString;
}

class ItemFlickrUploader : 
	public ImageTransformer<ItemFlickrUploader>
{	
private:

	FlickrUploadSettings _settings;
	CLoadAny _loader;

public:

	CString photoIds;

	ItemFlickrUploader(PluginState &plugins, FlickrUploadSettings &settings) : 
	ImageTransformer<ItemFlickrUploader>(plugins),
		_loader(plugins),
		_settings(settings)
	{
	};	

	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
	{
		_pItem = pItem;

		CString str;
		IW::Image imageIn = pItem->OpenAsImage(_loader, pStatus);

		if (!imageIn.IsEmpty())
		{			
			const CString &strKey = imageIn.GetLoaderName();

			_settings.Read(pItem->GetFilePath(), imageIn);

			CString newPhotoId;	

			if (UploadImage(_loader, imageIn, _settings, newPhotoId, pStatus))
			{
				if (!photoIds.IsEmpty()) photoIds += ",";
				photoIds += newPhotoId;

				if (IW::ImageLoaderFlags::SAVE & _loader.GetFlags(strKey))
				{
					SetPhotoId(imageIn, newPhotoId);

					if (pItem->SaveAsImage(_loader, imageIn, imageIn.GetLoaderName(), pStatus))
					{
						return true;
					}
					else
					{
						str.Format(_T("Could not save '%s' as an image."), pItem->GetFileName());
						pStatus->SetError(str);
					}
				}
			}
		}
		else
		{
			str.Format(_T("Could not open '%s' as an image."), pItem->GetFileName());
			pStatus->SetError(str);
		}

		return false;
	}

	void SetPhotoId(IW::Image &image, const CString &photoId)
	{
		ImageMetaData properties(image);
		properties.SetFlickrId(photoId);
		properties.Apply(image);
	}
};

void FlickrState::UploadSelectedImages()
{
	CWaitCursor wait;

	if (ConfirmAuthToken())
	{
		CString strAccountStatus;

		if (GetAccountStatus(strAccountStatus))
		{
			FlickrUploadSettings settings;

			{
				CPropertyArchiveRegistry archive(App.GetRegKey());
				settings.Read(&archive);
			}

			PhotoSetResponse photoSet;
			RequestPhotoSetList(photoSet);
			settings.entries = photoSet._entries;

			CFlickrMultiUploadDlg dlg(settings, strAccountStatus);					

			if (dlg.DoModal() == IDOK)
			{
				{
					CPropertyArchiveRegistry archive(App.GetRegKey(), true);
					settings.Write(&archive);
				}

				CProgressDlg pd;
				pd.Create(IW::GetMainWindow(), IDS_UPLOADING);
				pd.SetStatusMessage(IDS_UPLOADING);

				IW::FolderPtr pFolder = _state.Folder.GetFolder();	
				ItemFlickrUploader uploader(_state.Plugins, settings);

				if (!pFolder->IterateSelectedItems(&uploader, &pd) &&
					!pd.QueryCancel())
				{
					IW::CMessageBoxIndirect mb;
					mb.Show(pd.GetError());
				}
				else
				{
					CString url;
					url.Format(_T("http://www.flickr.com/tools/uploader_edit.gne?ids=%s"), uploader.photoIds);
					IW::NavigateToWebPage(url);	
				}
			}
		}
	}
}

void FlickrState::UploadImage()
{
	CWaitCursor wait;	

	if (ConfirmAuthToken())
	{
		CString strAccountStatus;

		if (GetAccountStatus(strAccountStatus))
		{
			FlickrUploadSettings settings;

			{
				CPropertyArchiveRegistry archive(App.GetRegKey());
				settings.Read(&archive);
			}

			PhotoSetResponse photoSet;
			RequestPhotoSetList(photoSet);
			settings.entries = photoSet._entries;

			CFlickrUploadDlg dlg(_state.Image, settings, strAccountStatus);					

			if (dlg.DoModal() == IDOK)
			{
				{
					CPropertyArchiveRegistry archive(App.GetRegKey(), true);
					settings.Write(&archive);
				}

				CProgressDlg pd;
				pd.Create(IW::GetMainWindow(), IDS_UPLOADING);
				pd.SetStatusMessage(IDS_UPLOADING);

				IW::Image image = _state.Image.GetImage().Clone();
				ImageMetaData properties(image);
				properties.SetTitle(settings.strTitle);
				properties.SetTags(settings.strTags);
				properties.SetDescription(settings.strDescription);
				properties.Apply(image);

				CString newPhotoId;	
				CLoadAny loader(_state.Plugins);

				if (::UploadImage(loader, image, settings, newPhotoId, &pd))
				{
					properties.SetFlickrId(newPhotoId);
					properties.Apply(image);

					_state.Image.SetImageWithHistory(image, _T("Flicker PhotoId"));
					IW::NavigateToWebPage(IW::Format(_T("http://www.flickr.com/tools/uploader_edit.gne?ids=%s"), newPhotoId));	
				}
			}
		}
	}
}




bool FlickrState::GetAccountStatus(CString& strAccountStatus)
{
	RequestUrl urlUploadStatus(_T("flickr.people.getUploadStatus"));
	urlUploadStatus.Add(_T("auth_token"), App.Options.Flickr.Token);

	UploadStatusResponse uploadStatusResponse;
	XmlParser<UploadStatusResponse> uploadStatusParser(uploadStatusResponse);
	uploadStatusParser.ParseURL(urlUploadStatus.GetUrl());

	bool success = uploadStatusResponse._success;

	if (!success)
	{
		ShowErrorMessage(uploadStatusResponse);
	}
	else
	{
		IW::FileSize filesizeMax(uploadStatusResponse._filesizeMax);
		int remainingLimit = uploadStatusResponse._bandwidthMax - uploadStatusResponse._bandwidthUsed;
		IW::FileSize limitRemaining(remainingLimit);
		int percentageRemaining = MulDiv(remainingLimit, 100, uploadStatusResponse._bandwidthMax);

		strAccountStatus.Format(_T("Uploading for user: %s (%s)\nMonthly uploaded limit remaining: %d%% (%s)\nMaximum upload file size: %s"), 
			App.Options.Flickr.UserName, App.Options.Flickr.FullName,
			percentageRemaining, limitRemaining.ToString(),
			filesizeMax.ToString());
	}
	return success;
}

bool FlickrState::ConfirmAuthToken()
{
	if (App.Options.Flickr.Token.IsEmpty())
	{
		GetNewToken();

		if (App.Options.Flickr.Token.IsEmpty())
		{
			return false;
		}
	}

	RequestUrl url(_T("flickr.auth.checkToken"));
	url.Add(_T("auth_token"), App.Options.Flickr.Token);

	TokenResponse tokenCheckResponse;
	XmlParser<TokenResponse> parser(tokenCheckResponse);
	parser.ParseURL(url.GetUrl());

	if (!tokenCheckResponse._success)
	{
		ShowErrorMessage(tokenCheckResponse);
	}

	return tokenCheckResponse._success;
}
