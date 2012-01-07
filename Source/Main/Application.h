#pragma once


class CImageMetaDataList;

#define IDC_COMMAND_BAR		 (30000)
#define IDC_ANIMATE			 (30001)
#define IDC_VIEW_ADDRESS	 (30003)
#define IDC_VIEW_TOOLBAR	 (30004)
#define IDC_LOGO			 (30005)
#define IDC_PRINT		     (30006)
#define IDC_DELAYBAR		 (30009)
#define IDC_SCALEBAR		 (30010)
#define IDC_WEB				 (30012)
#define IDC_SEARCH			 (30013)
#define IDC_SEARCHBYDATE	 (30014)
#define IDC_TAG				 (30015)
#define ID_SEARCH_BAR		 (30016)
#define ID_SEARCH_EDIT		 (30017)
#define ID_TAG_BAR			 (30018)
#define ID_TAG_COMBO		 (30019)
#define ID_SEARCHBYDATE_BAR	 (30020)
#define ID_YEAR_COMBO		 (30021)
#define ID_MONTH_COMBO		 (30022)
#define ID_FOLDER_BAR		 (30023)

struct RegistrationSettings
{
	enum 
	{
		Evaluation = 0,
		Registered = 1,
		Registered2 = 2,
		Free = 3
	};
};

class COptions
{
public:
	COptions();


public:
	// Constants
	enum { g_ePropMax = 10000 };

	// Serialisation
	void Read(const CString &strValueName, const IW::IPropertyArchive *pArchive);
	void Write(const CString &strValueName, IW::IPropertyArchive *pArchive) const;

	DWORD _nRegistrationSettings;

	// View Options
	DWORD m_nViewOptionsPage;
	DWORD m_nDescriptionPage;
	DWORD m_nWebOptionsPage;
	DWORD m_nPrintOptionsPage;
	CSize _sizeThumbImage;

	CSize _sizeRowsColumns;

	IW::CArrayDWORD m_annotations;
	IW::CArrayDWORD m_columns;

	bool ShowDescriptions;
	bool ZoomThumbnails;
	bool m_bBatchOneForAll;
	bool m_bDoubleClickShowsFullScreen;
	bool m_bShowToolTips;
	bool m_bShowMarkers;
	bool m_bShowHidden;
	bool m_bAutoSave;
	bool m_bWalkFolders;
	bool m_bSystemThumbs;
	bool m_bShortDates;
	bool m_bDontUseBackBuffer;
	bool m_bDontUseHalfTone;
	bool m_bUseEffects;
	bool _bSlideShowToolBar;
	bool m_bUseMMX;
	bool _bDontHideCursor;
	bool _bExifAutoRotate;

	bool ShowFlickrPicOfInterest;
	bool ShowFolders;
	bool ShowDescription;
	bool ShowAdvancedImageDetails;
	bool ShowAddress;
	bool ShowSearch;
	bool ShowStatusBar;
	bool BlackBackground;
	bool BlackSkin;

	CString _strCaptureSaveType;

	// Screen saver options
	bool m_bRepeat;
	bool m_bShuffle;
	bool m_bRecursSubFolders;
	bool m_bShowInformation;
	bool m_bShowButtons;
	long m_nDelay;

	long Mood;
	bool SearchByDate;
	bool AutoSelectTaggedImages;

	// Resolution
	int m_nResolutionSelection;
	DWORD m_dwXPelsPerMeter;
	DWORD m_dwYPelsPerMeter;

	struct tagFlickr
	{
		CString NSid;
		CString UserName;
		CString FullName;
		CString Token;
	} 
	Flickr;

	struct tagWeb
	{		
		CString Header;
		CString Footer;
	}
	Web;
};

// PLugin host implimentation
class Application
{
protected:


	typedef std::map<DWORD, LPCTSTR> MAPSTRINGS;
	MAPSTRINGS m_mapStrings;

	typedef std::map<UINT, int> MAPICONS;
	MAPICONS m_mapIcons;

	std::set<UINT> m_setCanBeCached;

	typedef std::map<int, CString> METADATAPROPERTYMAP;

	METADATAPROPERTYMAP m_MetaDataPropertyMap;
	METADATAPROPERTYMAP m_shortMetaDataPropertyMap;

	IW::StringPool _stringPool;
	int m_nNextId;

public:
	Application();
	virtual ~Application();

	void Init();

	// Cleanup Plugins
	void Free();

	// The options!!
	COptions Options;	

	bool CanBeCached(UINT uExtension);
	int GetIcon(DWORD dwExtension);
	void SetIcon(DWORD dwExtension, int nIcon);

	int GetNextId() { return m_nNextId++; };

	// Iterate posible meta data
	bool IterateMetaDataTypes(IW::IImageMetaDataClient *pClient);
	CString GetMetaDataTitle(DWORD dw);
	CString GetMetaDataShortTitle(DWORD dw);


	// Help and Strings
	LPCTSTR LoadString(UINT nId);
	void InvokeHelp(HWND hwnd, UINT nId);

	// Entry point for the bug report.
	void BugReport();

	// Thumbnial Cache


	// Resource handeling
	HINSTANCE GetResourceInstance();
	HINSTANCE GetBitmapResourceInstance();
	CString GetRegKey();

	// Debuging
	void RecordFileOperation(const CString &strFileName, const CString &strOperation);
	void Log(const CString &str);
	CString GetLog();

	// Resources
	HIMAGELIST GetGlobalBitmap();
	HIMAGELIST GetShellImageList(bool fSmall = false);

	CPalette m_paletteHalftone;	
	int m_nTextExtent;

	LCID GetLangId();
	void SetLangId(LCID l);

	CString GetIWSFilter();

	CCriticalSection _cs;

	bool IsTesting;
	bool CursorShown;	
	bool ControlKeyDown;

	static bool IsOnline()
	{
		DWORD dwFlags;
		return 0 != InternetGetConnectedState(&dwFlags, 0);
	}

	UINT GetExtensionKey(const CString &strFileName)
	{
		IW::CAutoLockCS lock(_cs);

		UINT uExtension = 0;
		CString strType = IW::Path::FindExtension(strFileName);
		strType.MakeUpper();

		if (!strType.IsEmpty())
		{	
			for(int i = 1; (i < 3) && strType[i]; i++)
			{
				TCHAR c = strType[i];
				uExtension = (uExtension << 8) + c;
			}
		}

		return uExtension;
	}
};

extern Application App;