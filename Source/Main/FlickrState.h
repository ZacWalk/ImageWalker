#pragma once

typedef char XML_Char;

class Response
{
public:
	bool _success;
	int _errCode;
	CString _errMessage;

	typedef enum { InUnknown, 
		InDescription, InTag, InFrob, 
		InToken, InPerms, InUser, 
		InUsername, InBandwidth, 
		InFilesize, InPhotoId, 
		InTitle, InTicketId,
		InImageUrl } State;

	State _state;

	Response() : _success(false), _state(InUnknown)
	{
	}

	static bool IsPropertyName(LPCSTR sz1, LPCSTR sz2)
	{
		return _stricmp(sz1, sz2) == 0;
	}

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("rsp", name))
		{
			while (*atts) 
			{
				SetRspProperty(atts[0], atts[1]);
				atts += 2;
			}
		}
		else if (IsPropertyName("err", name))
		{
			while (*atts) 
			{
				SetErrProperty(atts[0], atts[1]);
				atts += 2;
			}
		}
	}

	void ElementText(const XML_Char *txt, int len)
	{
	}

	void ElementEnd(const XML_Char *name)
	{
		_state = InUnknown;
	}

	void SetRspProperty(const char *name, const char *value)
	{
		if (IsPropertyName("stat", name))
		{
			_success = IsPropertyName("ok", value);
		}
	}

	void SetErrProperty(const char *name, const char *value)
	{
		if (IsPropertyName("code", name))
		{
			_errCode = atoi(value);
		}
		else if (IsPropertyName("msg", name))
		{
			_errMessage = value;
		}
	}
};

struct InterestingImageEntry
{
	InterestingImageEntry()
	{
	}		

	CString GetThumbnailURL() const
	{
		CString url;
		url.Format(_T("http://farm%s.static.flickr.com/%s/%s_%s_t.jpg"),  farm, server, id, secret);
		return url;
	}

	CString GetImageURL() const
	{
		CString url;
		url.Format(_T("http://www.flickr.com/photos/%s/%s"),  owner, id);
		return url;
	}

	CString GetIptcEntry() const
	{
		return id;
	}

	InterestingImageEntry(const InterestingImageEntry &other)
	{
		Copy(other);
	}
	void operator=(const InterestingImageEntry &other)
	{
		Copy(other);
	}

	void Copy(const InterestingImageEntry &other)
	{
		id = other.id;
		owner = other.owner;
		secret = other.secret;
		server = other.server;
		title = other.title;
		ispublic = other.ispublic;
		originalFormat = other.originalFormat;
		isfriend = other.isfriend;
		isfamily = other.isfamily;
		farm = other.farm;
	}

	bool CanDownload() const
	{
		return originalFormat.CompareNoCase(_T("jpg")) == 0;
	}

	CString id;
	CString owner;
	CString secret;
	CString server;
	CString title;
	CString farm;
	CString originalFormat;

	bool ispublic;
	bool isfriend;
	bool isfamily;
};

class InterestingImageResponse : public Response
{
public:

	std::vector<InterestingImageEntry> _entries;
	int _nCurrentEntry;	

	enum { MaxEntryCount = 64 };	

	InterestingImageResponse() : _nCurrentEntry(0)
	{
	}

	void SelectRandomEntry()
	{
		srand(GetTickCount());
		_nCurrentEntry = (rand() * _entries.size()) / RAND_MAX;
	}

	void Next()
	{
		if (!IsEmpty())
		{
			_nCurrentEntry = (_nCurrentEntry + 1) % _entries.size();
		}
	}

	bool IsEmpty() const
	{
		return _entries.size() == 0;
	}

	IW::Image GetImage(PluginState &plugins) const;

	InterestingImageEntry RequestInterestingImageEntry() const
	{
		if (_entries.size() > 0)
		{
			return _entries[_nCurrentEntry];
		}

		return InterestingImageEntry();
	}

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("photo", name))
		{
			InterestingImageEntry photo;

			while (*atts) 
			{
				SetInterestingImageEntryProperty(photo, atts[0], atts[1]);
				atts += 2;
			}

			_entries.push_back(photo);
		}

		Response::ElementStart(name, atts);
	}

	void SetInterestingImageEntryProperty(InterestingImageEntry &photo, LPCSTR szProperty, LPCSTR szValue)
	{
		if (IsPropertyName("id", szProperty))
		{
			photo.id = szValue;
		}
		else if (IsPropertyName("owner", szProperty))
		{
			photo.owner = szValue;
		}
		else if (IsPropertyName("secret", szProperty))
		{
			photo.secret = szValue;
		}
		else if (IsPropertyName("server", szProperty))
		{
			photo.server = szValue;
		}
		else if (IsPropertyName("title", szProperty))
		{
			photo.title = szValue;
		}
		else if (IsPropertyName("ispublic", szProperty))
		{
			photo.ispublic = atoi(szValue) > 0;
		}
		else if (IsPropertyName("isfriend", szProperty))
		{
			photo.isfriend = atoi(szValue) > 0;
		}
		else if (IsPropertyName("isfamily", szProperty))
		{
			photo.isfamily = atoi(szValue) > 0;
		}
		else if (IsPropertyName("farm", szProperty))
		{
			photo.farm = szValue;
		}
		else if (IsPropertyName("originalformat", szProperty))
		{
			photo.originalFormat = szValue;
		}
	}
};

struct PhotoSetEntry
{
	PhotoSetEntry()
	{
	}				

	PhotoSetEntry(const PhotoSetEntry &other)
	{
		Copy(other);
	}
	void operator=(const PhotoSetEntry &other)
	{
		Copy(other);
	}

	void Copy(const PhotoSetEntry &other)
	{
		id = other.id;
		primary = other.primary;
		secret = other.secret;
		server = other.server;
		photos = other.photos;
		title = other.title;
		description = other.description;
	}

	CString id;
	CString primary;
	CString secret;
	CString server;
	CString photos;
	CString title;
	CString description;
};

typedef std::vector<PhotoSetEntry> PHOTOSETLIST;

class PhotoSetResponse : public Response
{
public:

	PHOTOSETLIST _entries;
	PhotoSetEntry _set;

	PhotoSetResponse()
	{
	}

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("photoset", name))
		{
			_set.description = g_szEmptyString;
			_set.title = g_szEmptyString;

			while (*atts) 
			{
				SetPhotoSetEntryProperty(atts[0], atts[1]);
				atts += 2;
			}
		}
		else if (IsPropertyName("description", name))
		{
			_state = InDescription;
		}
		else if (IsPropertyName("title", name))
		{
			_state = InTitle;
		}

		Response::ElementStart(name, atts);
	}

	void ElementText(const XML_Char *txt, int len)
	{
		CString str(txt, len);

		if (!str.IsEmpty())
		{
			switch(_state)
			{
			case InDescription:
				_set.description += str;
				break;
			case InTitle:
				_set.title += str;
				break;
			}
		}
	}

	void ElementEnd(const XML_Char *name)
	{
		if (IsPropertyName("photoset", name))
		{
			_entries.push_back(_set);
		}

		_state = InUnknown;
	}

	void SetPhotoSetEntryProperty(LPCSTR szProperty, LPCSTR szValue)
	{
		if (IsPropertyName("id", szProperty))
		{
			_set.id = szValue;
		}
		else if (IsPropertyName("primary", szProperty))
		{
			_set.primary = szValue;
		}
		else if (IsPropertyName("secret", szProperty))
		{
			_set.secret = szValue;
		}
		else if (IsPropertyName("server", szProperty))
		{
			_set.server = szValue;
		}
		else if (IsPropertyName("photos", szProperty))
		{
			_set.photos = szValue;
		}
	}
};

class PhotoInfoResponse : public Response
{
public:
	CString _description;
	CString _tags;
	CString _title;
	CString _url;

	CString _id;
	CString _secret;
	CString _server;
	CString _farm;
	CString _originalsecret;

	PhotoInfoResponse() 
	{
	}

	CString GetDownloadUrl() const
	{
		CString url;

		if (!_originalsecret.IsEmpty())
		{
			url.Format(_T("http://farm%s.static.flickr.com/%s/%s_%s_o.jpg"),  _farm, _server, _id, _originalsecret);
		}
		else
		{
			url.Format(_T("http://farm%s.static.flickr.com/%s/%s_%s.jpg"),  _farm, _server, _id, _secret);
		}

		return url;
	}

	void ElementText(const XML_Char *txt, int len)
	{
		CString str(txt, len);

		if (!str.IsEmpty())
		{
			switch(_state)
			{
			case InDescription:
				_description += str;
				break;
			case InTag:
				if (!_tags.IsEmpty()) _tags += _T(";");
				_tags += str;
				break;
			case InImageUrl:
				_url += str;
				break;
			case InTitle:
				_title += str;
				break;
			}
		}
	}

	static bool HasAttribute(const XML_Char **atts, const XML_Char *attribute, const XML_Char *value)
	{
		while (*atts) 
		{
			if (IsPropertyName(attribute, atts[0]) && 
				IsPropertyName(value, atts[1]))
			{
				return true;
			}

			atts += 2;
		}

		return false;
	}

	void SetPhotoProperty(LPCSTR szProperty, LPCSTR szValue)
	{
		if (IsPropertyName("id", szProperty))
		{
			_id = szValue;
		}
		else if (IsPropertyName("secret", szProperty))
		{
			_secret = szValue;
		}
		else if (IsPropertyName("server", szProperty))
		{
			_server = szValue;
		}
		else if (IsPropertyName("farm", szProperty))
		{
			_farm = szValue;
		}
		else if (IsPropertyName("originalsecret", szProperty))
		{
			_originalsecret = szValue;
		}			
	}


	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("photo", name))
		{
			while (*atts) 
			{
				SetPhotoProperty(atts[0], atts[1]);
				atts += 2;
			}
		}
		else if (IsPropertyName("description", name))
		{
			_state = InDescription;
		}
		else if (IsPropertyName("tag", name))
		{
			_state = InTag;
		}
		else if (IsPropertyName("url", name) && HasAttribute(atts, "type", "photopage"))
		{
			_state = InImageUrl;
		}
		else if (IsPropertyName("title", name))
		{
			_state = InTitle;
		}

		Response::ElementStart(name, atts);
	}
};

class FrobResponse : public Response
{
public:
	CString _frob;


	FrobResponse()
	{
	}

	void ElementText(const XML_Char *txt, int len)
	{
		CString str(txt, len);

		if (!str.IsEmpty())
		{
			switch(_state)
			{
			case InFrob:
				_frob = str;
				break;
			}
		}
	}

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("frob", name))
		{
			_state = InFrob;
		}

		Response::ElementStart(name, atts);
	}
};

class TokenResponse : public Response
{
public:
	CString _token;
	CString _perms;

	CString _nsid;
	CString _username;
	CString _fullname;

	TokenResponse()
	{
	}

	void ElementText(const XML_Char *txt, int len)
	{
		CString str(txt, len);

		if (!str.IsEmpty())
		{
			switch(_state)
			{
			case InToken:
				_token = str;
				break;
			case InPerms:
				_perms = str;
				break;
			}
		}
	}

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("token", name))
		{
			_state = InToken;
		}
		else if (IsPropertyName("perms", name))
		{
			_state = InPerms;
		}
		else if (IsPropertyName("user", name))
		{
			_state = InUser;

			while (*atts) 
			{
				SetUserProperty(atts[0], atts[1]);
				atts += 2;
			}
		}

		Response::ElementStart(name, atts);
	}

	void SetUserProperty(const char *name, const char *value)
	{
		if (IsPropertyName("nsid", name))
		{
			_nsid = value;
		}
		else if (IsPropertyName("username", name))
		{
			_username = value;
		}
		else if (IsPropertyName("fullname", name))
		{
			_fullname = value;
		}
	}
};

class UploadStatusResponse : public Response
{
public:

	CString _username;

	int _bandwidthMax;
	int _bandwidthUsed;
	int _filesizeMax;

	UploadStatusResponse()
	{
	}

	void ElementText(const XML_Char *txt, int len)
	{
		CString str(txt, len);

		if (!str.IsEmpty())
		{
			switch(_state)
			{
			case InUsername:
				_username = str;
				break;
			}
		}
	}

	void ElementStart(const XML_Char *name, const XML_Char **atts)
	{
		if (IsPropertyName("username", name))
		{
			_state = InUsername;
		}
		else if (IsPropertyName("bandwidth", name))
		{
			_state = InBandwidth;

			while (*atts) 
			{
				SetBandwidthProperty(atts[0], atts[1]);
				atts += 2;
			}
		}
		else if (IsPropertyName("filesize", name))
		{
			_state = InFilesize;

			while (*atts) 
			{
				SetFilesizeProperty(atts[0], atts[1]);
				atts += 2;
			}
		}

		Response::ElementStart(name, atts);
	}

	void SetBandwidthProperty(const char *name, const char *value)
	{
		if (IsPropertyName("used", name))
		{
			_bandwidthUsed = atoi(value);
		}
		else if (IsPropertyName("max", name))
		{
			_bandwidthMax = atoi(value);
		}
	}

	void SetFilesizeProperty(const char *name, const char *value)
	{
		if (IsPropertyName("max", name))
		{
			_filesizeMax = atoi(value);
		}
	}
};

class FlickrThread : public IW::Thread
{
private:
	typedef FlickrThread ThisClass;
	CEvent _eventNextImage;	

public:

	State &_state;

	FlickrThread(State &state) : 
		_eventNextImage(FALSE, TRUE),
		_state(state)
	{
	}

	void Next()
	{
		_eventNextImage.Set();
	}

	void Process();
	void LoadNewRequest(InterestingImageResponse &request);
};

class FlickrState
{
public:

	FlickrThread _thread;
	InterestingImageEntry _entry;	
	IW::Image _image;
	State &_state;

	FlickrState(State &state) : 
		_thread(state), _state(state)
	{
		_thread.StartThread();
	}

	Delegate::List0 NewInterestingImage;

	void Next()
	{
		_thread.Next();
	}

	void Download()
	{
		DownloadById(_entry.id, _entry.secret);
	}		

	void SetNewInterestingImage(InterestingImageEntry &entry, IW::Image &image);

	void RequestPhotoInfo(PhotoInfoResponse &request, const CString &strPhotoId, const CString &strSecret);
	void RequestInterestingImage(InterestingImageResponse &request);
	void RequestPhotoSetList(PhotoSetResponse &request);	
	void RequestFrob(FrobResponse &request);	
	void RequestToken(TokenResponse &request, const CString &strFrob);

	void OpenImage(const CString &strId);
	bool GetAccountStatus(CString& strAccountStatus);
	bool ConfirmAuthToken();

	CString GetNewToken();

	void UploadImage();
	void UploadSelectedImages();

	void DownloadById(const CString &strId);
	void DownloadById(const CString &strId, const CString &strSecret);
	void DownloadByUrl(const CString &strUrl, const CString &strId, const CString &strTitle, const CString &strKeywords, const CString &strDescription);

	static void ShowErrorMessage(Response &r);
	static bool PromptUser(CString &str);
};
