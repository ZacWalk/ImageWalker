// TemplateParser.cpp: implementation of the CTemplateIndexParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Limits.h>
#include <AtlRX.h>
#include "LoadAny.h"
#include "Html.h"
#include "State.h"


static LPCTSTR g_szIndex = _T("Index");
static LPCTSTR g_szRequiredFiles = _T("Required Files");
static LPCTSTR g_szFrameHost = _T("FrameHost");
static LPCTSTR g_szOnlyOneIndexPage = _T("OnlyOneIndexPage");
static LPCTSTR g_szNoThumbnails = _T("NoThumbnails");
static LPCTSTR g_szCannotRecurse = _T("CannotRecurse");
static LPCTSTR g_szOverRideCols = _T("OverRideCols");
static LPCTSTR g_szTableEntry = _T("TableEntry");
static LPCTSTR g_szFolderEntry = _T("FolderEntry");
static LPCTSTR g_szBlankHtmlPage = _T("<HTML><BODY></BODY></HTML>");
static LPCTSTR g_sz1310 = _T("&#13;&#10;");
static LPCTSTR g_szBR = _T("<br>");
static LPCTSTR g_sz39 = _T("&#39;");
static LPCTSTR g_szQuot = _T("&quot;");

static LPCTSTR g_szTableEntryStart = _T("<TABLE width=100% height=100% border=0 cellspacing=0 cellpadding=4 class=Payload>\n");
static LPCTSTR g_szTableEntryDefault = _T("<td class=\"Image\" align=center valign=center width={{PercWidth}}><span class=\"Annotation\"><A  href=\"{{Image}}\"><IMG border=\"{{ImageBorderWidth}}\" src=\"{{ThumbFile}}\" width={{ThumbWidth}} height={{ThumbHeight}} Alt=\"{{ThumbAlt}}\"></A><BR>{{ThumbText}}</span></td>");

//static LPCTSTR g_szTableEntryStart = _T("<TABLE>\n");
//static LPCTSTR g_szTableEntryDefault = _T("<td class=\"Image\" align=center valign=center width={{PercWidth}}><span class=\"Annotation\"><A  href=\"{{Image}}\"><IMG border=0 src=\"{{ThumbFile}}\" width={{ThumbWidth}} height={{ThumbHeight}}></A><BR>{{ThumbText}}</span></td>");


// HTML Tags
static LPCTSTR g_szTagImageBorderWidth = _T("{{ImageBorderWidth}}");
static LPCTSTR g_szTagBack =  _T("{{Back}}");
static LPCTSTR g_szTagBackThumbnail = _T("{{BackThumbnail}}");
static LPCTSTR g_szTagBody = _T("{{Body}}");
static LPCTSTR g_szTagContext = _T("{{Context}}");
static LPCTSTR g_szTagDate = _T("{{Date}}");
static LPCTSTR g_szTagDescription = _T("{{Description}}");
static LPCTSTR g_szTagFirst = _T("{{First}}");
static LPCTSTR g_szTagFooter = _T("{{Footer}}");
static LPCTSTR g_szTagFrameHost = _T("{{FrameHost}}");
static LPCTSTR g_szTagHeader = _T("{{Header}}");
static LPCTSTR g_szTagKeyWords = _T("{{KeyWords}}");
static LPCTSTR g_szTagHome = _T("{{Home}}");
static LPCTSTR g_szTagImage = _T("{{Image}}");
static LPCTSTR g_szTagImageWidth = _T("{{ImageWidth}}");
static LPCTSTR g_szTagImageHeight = _T("{{ImageHeight}}");
static LPCTSTR g_szTagImageDepth = _T("{{ImageDepth}}");
static LPCTSTR g_szTagImageAlt = _T("{{ImageAlt}}");
static LPCTSTR g_szTagImageFile = _T("{{ImageFile}}");
static LPCTSTR g_szTagImageTitle = _T("{{ImageTitle}}");
static LPCTSTR g_szTagImageTemplate = _T("{{ImageTemplate}}");
static LPCTSTR g_szTagInformation = _T("{{Information}}");
static LPCTSTR g_szTagMainHeader = _T("{{MainHeader}}");
static LPCTSTR g_szTagModifiedDate = _T("{{ModifiedDate}}");
static LPCTSTR g_szTagModifiedTime = _T("{{ModifiedTime}}");
static LPCTSTR g_szTagNavigation = _T("{{Navigation}}");
static LPCTSTR g_szTagNext = _T("{{Next}}");
static LPCTSTR g_szTagNextThumbnail = _T("{{NextThumbnail}}");
static LPCTSTR g_szTagPercWidth = _T("{{PercWidth}}");
static LPCTSTR g_szTagSize = _T("{{Size}}");
static LPCTSTR g_szTagStyleSheet = _T("{{StyleSheet}}");
static LPCTSTR g_szTagTemplate = _T("{{Template}}");
static LPCTSTR g_szTagThumbAlt = _T("{{ThumbAlt}}");
static LPCTSTR g_szTagThumbFile = _T("{{ThumbFile}}");
static LPCTSTR g_szTagThumbFileOld = _T("{{ThumbFile}}");
static LPCTSTR g_szTagThumbHeight = _T("{{ThumbHeight}}");
static LPCTSTR g_szTagThumbText = _T("{{ThumbText}}");
static LPCTSTR g_szTagThumbTitle = _T("{{ThumbTitle}}");
static LPCTSTR g_szTagThumbWidth = _T("{{ThumbWidth}}");
static LPCTSTR g_szTagTime = _T("{{Time}}");
static LPCTSTR g_szTagUp = _T("{{Up}}");
static LPCTSTR g_szTagScripts = _T("{{Scripts}}");
static LPCTSTR g_szDelay = _T("{{Delay}}");

const int nFolderSpreadX = 10;
const int nFolderSpreadY = 4;

//////////////////////////////////////////////////////////////////////



static int PopulateCombo(CComboBox combo, int nSel, CStringArray &ar)
{
	int i = 0;
	
	while (i < ar.GetSize()) 
	{
		IW::CFilePath path(ar[i]);		
		IW::SetItem(combo, path.GetFileName(), i);		
		i++;
	}

	combo.SetCurSel(nSel);
	return i;
}

static int PopulateArray(const CString &strTempPath, CStringArray &ar)
{
	WIN32_FIND_DATA FindFileData;
	IW::MemZero(&FindFileData, sizeof(FindFileData));
	
	HANDLE hSearch = FindFirstFile(strTempPath, &FindFileData);
	int i = 0;
	
	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		do
		{
			ar.Add(FindFileData.cFileName);
			i++;
		}
		while(FindNextFile(hSearch, &FindFileData));

		// Close the search handle. 
		FindClose(hSearch);
	}
	
	return i;
}



int CWebPage::PopulateTemplates(HWND hCombo)
{
	IW::CAutoLockCS lock(_cs);
	return ::PopulateCombo( hCombo, _settings.m_nTemplateSelection, m_arTemplates);
}

int CWebPage::PopulateStyles(HWND hCombo)
{
	IW::CAutoLockCS lock(_cs);
	return ::PopulateCombo(hCombo, _settings.m_nStyleSheetSelection, m_arStyleSheets);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



CWebPage::CWebPage(State &state) : _state(state)
{
	m_bValid = false;

	// Folder info
	_sizeThumbNail.cx = 80;
	_sizeThumbNail.cy = 80;

	_strType = g_szJPG;

	m_bHasImageTemplate = false;
	m_bFrameHost = false;
	m_bOnlyOneIndexPage = false;
	m_bNoThumbnails = false;
	m_bCannotRecurse = false;
	m_bOverRideCols = false;
		
	IW::CFilePath pathTempPath;
	pathTempPath.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	pathTempPath += _T("*.ini");

	int nRet = ::PopulateArray(pathTempPath, m_arTemplates);

	pathTempPath.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	pathTempPath += _T("*.css");

	nRet = ::PopulateArray(pathTempPath, m_arStyleSheets);


	_pathTemplateFile.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	_pathTemplateFile += m_arTemplates[0];

	_pathStyleSheetFile.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	_pathStyleSheetFile += m_arStyleSheets[0];


	_pathTemplateFolder.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());

	m_bValid = true;
	
}

CWebPage::~CWebPage()
{
}

int Compare(IShellFolder *pShellFolder, LPITEMIDLIST a, LPITEMIDLIST b)
{
	HRESULT hr = pShellFolder->CompareIDs(0, a, b);

	ATLASSERT(SUCCEEDED(hr));
	
	short s = static_cast<short>(SCODE_CODE(hr));
	return s;
};


bool IsFullQualifiedPath(const CString &str)
{
	if (str.GetLength() >= 3)
	{
		if (str[0] == '\\') return true;
		if (str[0] == '/') return true;

		if (str[1] == ':') 
		{
			if (str[2] == '\\') return true;
			if (str[2] == '/') return true;
		}		
	}

	return false;
}

CString CWebPage::GetProfileString(const CString &strSection, const CString &strEntry, const CString &strDefault)
{
	return GetProfileString(strSection, strEntry, strDefault, _pathTemplateFile);
}

CString CWebPage::GetProfileString(const CString &strSection, const CString &strEntry, const CString &strDefault, const CString &strFileName)
{
	const int nBufferLength = 1024 * 10;
	CString str;
	DWORD dw = ::GetPrivateProfileString(strSection, strEntry, strDefault, str.GetBuffer(nBufferLength), nBufferLength, strFileName);
	str.ReleaseBuffer();
	return str;
}

bool CWebPage::SetSettings(const CWebSettings &settings)
{
	IW::CAutoLockCS lock(_cs);

	_settings = settings;
	m_bValid = true;
	m_arrayRequiredFiles.RemoveAll();

	if (_settings.m_nTemplateSelection == -1 ||
		_settings.m_nStyleSheetSelection == -1)
	{
		m_bValid = false;
		return false;
	}


	_pathTemplateFile.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	_pathTemplateFile += m_arTemplates[_settings.m_nTemplateSelection];
	
	_pathStyleSheetFile.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	_pathStyleSheetFile += m_arStyleSheets[_settings.m_nStyleSheetSelection];

	// Get the bacgound colors
	CString strCSS, str;		
	if (LoadTextFile(_pathStyleSheetFile, strCSS))
	{
		CAtlRegExp<> re;
		REParseError status = re.Parse(_T(".*image-background-color: *\\#{[0123456789abcdef]*}.*"), false);
		ATLASSERT(REPARSE_ERROR_OK == status);

		m_clrBG = RGB(255, 255, 255);

		CAtlREMatchContext<> mc;

		if (re.Match(strCSS, &mc))
		{
			const CAtlREMatchContext<>::RECHAR* szStart = 0, *szEnd = 0;
			mc.GetMatch(0, &szStart, &szEnd);

			TCHAR *szStop;
			CString str(szStart, szEnd - szStart);
			COLORREF clr = _tcstol(str, &szStop, 16);
			m_clrBG = IW::SwapRB(clr);
		}
	}

	CString strIndexFile = GetProfileString(g_szIndex, g_szTemplate, g_szEmptyString);

	if (!strIndexFile.IsEmpty())
	{
		if (IsFullQualifiedPath(strIndexFile))
		{
			_pathTemplateIndexFile = strIndexFile;
		}
		else
		{
			_pathTemplateIndexFile.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
			_pathTemplateIndexFile += strIndexFile;
		}

		_pathTemplate = strIndexFile;
		_pathTemplate.StripToFilenameAndExtension();
	}
	else
	{
		return false;
	}

	m_bFrameHost = false;
	m_bHasImageTemplate = false;

	CString strImageFile = GetProfileString(g_szImage, g_szTemplate, g_szEmptyString);

	if (IsFullQualifiedPath(strImageFile))
	{
		_pathTemplateImageFile = strImageFile;
	}
	else
	{
		_pathTemplateImageFile.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
		_pathTemplateImageFile += strImageFile;
	}

	m_bHasImageTemplate = !strImageFile.IsEmpty();		
	_pathImageTemplate = strImageFile;
	_pathImageTemplate.StripToFilenameAndExtension();


	CString strFrameHost = GetProfileString(g_szIndex, g_szFrameHost, g_szEmptyString, _pathTemplateFile);
	
	if (IsFullQualifiedPath(strFrameHost))
	{
		_pathFrameHost = strFrameHost;
	}
	else
	{
		_pathFrameHost.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
		_pathFrameHost += strFrameHost;
	}

	m_bFrameHost = !strFrameHost.IsEmpty();
	_pathFrameHostShort = strFrameHost;
	_pathFrameHostShort.StripToFilenameAndExtension();

	// Background colors
	m_bOnlyOneIndexPage = GetPrivateProfileInt(g_szIndex, g_szOnlyOneIndexPage, 0, _pathTemplateFile) > 0;
	m_bNoThumbnails = GetPrivateProfileInt(g_szIndex, g_szNoThumbnails, 0, _pathTemplateFile) > 0;	
	m_bOverRideCols = GetPrivateProfileInt(g_szIndex, g_szOverRideCols, 0, _pathTemplateFile) > 0;
	m_bCannotRecurse = GetPrivateProfileInt(g_szIndex, g_szCannotRecurse, 0, _pathTemplateFile) > 0;
	_strTableEntry = GetProfileString(g_szIndex, g_szTableEntry, g_szTableEntryDefault);
	_strFolderEntry = GetProfileString(g_szIndex, g_szFolderEntry, g_szTableEntryDefault);

	LoadRequiredFilesList();

	if (m_bOverRideCols)
	{
		_settings._sizeRowsColumns.cx = 1;
	}

	if (m_bOnlyOneIndexPage)
	{
		_settings._sizeRowsColumns.cy = INT_MAX;
	}


	return true;
}

void CWebPage::LoadRequiredFilesList()
{
	// Get a list of requilred files
	// Copy over any required files
	TCHAR strFileName[MAX_PATH];
	m_arrayRequiredFiles.RemoveAll();
	const int nTextBufferSize = 1024;
	TCHAR sz[nTextBufferSize];

	if (GetPrivateProfileSection(g_szRequiredFiles, sz, nTextBufferSize, _pathTemplateFile))
	{		
		LPTSTR pIn = sz;
		
		while(*pIn != NULL)
		{
			LPTSTR pOut = strFileName;
			
			while(*pIn != NULL && *pIn != '=')
			{
				pIn++;
			}

			do
			{
				pIn++;
				*pOut = *pIn;
				pOut++;
			}
			while(*pIn != NULL);

			if (strFileName[0] != 0)
			{
				m_arrayRequiredFiles.Add(strFileName);
			}

			pIn++;
		}
	}
}

static inline int FindThumb(IW::ITEMLIST &thumbs, const CString &strFindMe)
{
	for(unsigned i = 0; i < thumbs.size(); ++i)
	{
		IW::FolderItemPtr pItem = thumbs[i];
		CString strName = pItem->GetFileName();

		if (strName.CompareNoCase(strFindMe) == 0)
		{
			return i;
		}
	}

	return -1;
}


bool CWebPage::GetImage(IW::ITEMLIST &thumbs, CAddressPolicy *pAddress, const CString &strLocation, const CString &strFileName, int nIndex, CString &strPageOut)
{
	IW::CAutoLockCS lock(_cs);

	IW::FolderPtr pFolderIn = new IW::RefObj<IW::Folder>;
	if (!pFolderIn->Init(strLocation))
	{
		return false;
	}	

	if (thumbs.empty())
	{
		strPageOut.Format(IDS_HTML_PAGENOTFOUND, strLocation);
		return false;
	}

	if (!m_bValid)
	{
		strPageOut = g_szBlankHtmlPage;
	}
	
	LoadTextFile(_pathTemplateImageFile, strPageOut);

	// Get path and load the image
	int nItem = FindThumb(thumbs, strFileName);
	if (nItem == -1) return false;
	IW::FolderItemPtr pItem = thumbs[nItem];

	const IW::Image &image = pItem->GetImage();

	CString strTitle = IW::TextToHtmlFriendly(image.GetTitle());
	if (strTitle.IsEmpty()) strTitle = pItem->GetDisplayName();
	strPageOut.Replace(g_szTagHeader, strTitle);	

	CString strKeyWords = IW::TextToHtmlFriendly(image.GetTags());
	strPageOut.Replace(g_szTagKeyWords, strKeyWords);	

	IW::FileTime modified = pItem->GetLastWriteTime().ToLocalTime();
	CString strModifiedTime = modified.GetTimeFormat( App.GetLangId());
	CString strModifiedDate = modified.GetDateFormat( App.GetLangId(), App.Options.m_bShortDates);
	CString strFileSize = pItem->GetFileSize().ToString();

	// The image description
	CString strStatistics = pItem->GetStatistics();
	CString strDescription = IW::TextToHtmlFriendly(image.GetDescription());
	strDescription.Replace(g_sz1310, g_szBR);
	//strText.Replace(g_sz39, "");
	strDescription.Replace(g_szQuot, g_sz39);

	strPageOut.Replace(g_szTagInformation, strStatistics);
	strPageOut.Replace(g_szTagDescription, strDescription);
	strPageOut.Replace(g_szTagSize, strFileSize);
	strPageOut.Replace(g_szTagModifiedDate, strModifiedDate);
	strPageOut.Replace(g_szTagModifiedTime, strModifiedTime);	

	// Image Width and Height
	const IW::CameraSettings &settings = pItem->GetCameraSettings();


	strPageOut.Replace(g_szTagImageWidth, IW::IToStr(settings.OriginalImageSize.cx));
	strPageOut.Replace(g_szTagImageHeight, IW::IToStr(settings.OriginalImageSize.cy));
	strPageOut.Replace(g_szTagImageDepth, settings.OriginalBpp.ToString());

	// Image Alt
	CString strAlt = IW::TextToHtmlFriendly(image.GetDescription());		
	if (strAlt.IsEmpty()) IW::TextToHtmlFriendly(strTitle);		
	strPageOut.Replace(g_szTagThumbAlt, strAlt);
	
	// Navigation context
	CString strContext;

	if (_settings.m_bBreadCrumbs)
	{
		strContext = pAddress->GetBreadCrumbs(strLocation, false);
		strContext += _T("&nbsp;&gt;&nbsp;");
		strContext += strFileName;
	}

	strPageOut.Replace(g_szTagContext, strContext);

	CString strImageURL;

	if (pItem->IsHTMLDisplayable(_state.Plugins) && m_bHasImageTemplate)
	{		
		strImageURL = pAddress->GetImageFileURL(strLocation, strFileName, false);
	}
	else
	{
		strImageURL = pAddress->GetThumbnailURL(strLocation, strFileName, m_clrBG, false);
	}

	CString strImage;

	// Do the replacments
	/*if (_settings.m_bFitImages)
	{
		//strImage = _T("<span style=\"background-image: url(");
		//strImage += strImageURL;
		//strImage += _T("); background-repeat:no-repeat;background-repeat:no-repeat; background-position=center center; width:100%; height:100%;\"></span>");

		strImage = _T("<img");
		strImage += _T(" style=\"display:block;width:100%;\"");
		strImage += _T(" src=\"");
		strImage += strImageURL;		
		strImage += _T("\">");
	}
	else*/
	{
		strImage = _T("<img border=0 src=\"");
		strImage += strImageURL;
		strImage += _T("\">");
	}

	strPageOut.Replace(g_szTagImage, strImage);
	strPageOut.Replace(g_szTagImageFile, strImageURL);
	strPageOut.Replace(g_szTagImageAlt, strAlt);

	// Title
	IW::CFilePath pathTitle(strImageURL); pathTitle.StripToFilename();
	strPageOut.Replace(g_szTagImageTitle, pathTitle);

	CString strNavigation, str;
	
	// Find the nex and back
	int nNext = nItem;
	int nLast = nItem;
	int nItemCount = thumbs.size();

	if (nItemCount > 0)
	{
	
		do
		{
			nNext = (nNext + 1) % nItemCount;
			if (nNext == nItem) break; // Stop enless loop
		}
		while(!thumbs[nNext]->IsHTMLDisplayable(_state.Plugins));

		do
		{
			nLast = nLast-1;
			if (nLast < 0) nLast += nItemCount;	
			if (nLast == nItem) break; // Stop enless loop
		}
		while(!thumbs[nLast]->IsHTMLDisplayable(_state.Plugins));

	}
	


	CString strRef;

	/////////////////////////////////////////////////////////////
	// Do back
	CString strThumbLastFileName = thumbs[nLast]->GetFileName();
	strRef = pAddress->GetImageURL(strLocation, strThumbLastFileName, false);
	strPageOut.Replace(g_szTagBack, strRef);
	strRef = pAddress->GetThumbnailURL(strLocation, strThumbLastFileName, m_clrBG, false);
	str.Format(_T("<IMG border=%d src=\"%s\" ALT=\"Back\">"), _settings.m_nImageBorderWidth, strRef);
	strPageOut.Replace(g_szTagBackThumbnail, str);

	/////////////////////////////////////////////////////////////
	// Do Next
	CString strThumbNextFileName = thumbs[nNext]->GetFileName();
	strRef = pAddress->GetImageURL(strLocation, strThumbNextFileName, false);
	strPageOut.Replace(g_szTagNext, strRef);
	strRef = pAddress->GetThumbnailURL(strLocation, strThumbNextFileName, m_clrBG, false);
	str.Format(_T("<IMG border=%d src=\"%s\" ALT=\"Next\">"), _settings.m_nImageBorderWidth, strRef);
	strPageOut.Replace(g_szTagNextThumbnail, str);

	/////////////////////////////////////////////////////////////
	// Do up
	strRef = pAddress->GetIndexURL(strLocation, nIndex, false);
	strPageOut.Replace(g_szTagUp, strRef);
	
	// Defaults
	ReplaceConstants(thumbs, strPageOut, pAddress, strLocation, false, false);

	return true;
}

bool CWebPage::GetIndex(IW::ITEMLIST &thumbs, CAddressPolicy *pAddress, const CString &strLocation, int nPage, CString &strPageOut)
{
	CString strIndexTable;
	
	IW::FolderPtr pFolderIn = new IW::RefObj<IW::Folder>;
	if (!pFolderIn->Init(strLocation))
	{
		return false;
	}

	CLoadAny loader(_state.Plugins);
	bool bValid = false;
	bool bFrameHost = false;
	bool bHasImageTemplate = false;
	IW::CFilePath pathFrameHost, pathTemplateIndexFile;
	CWebSettings settings;
	CString strFolderEntry, strTableEntry;
	DWORD clrBG;

	{
		IW::CAutoLockCS lock(_cs);
		bValid = m_bValid;
		bFrameHost = m_bFrameHost;
		bHasImageTemplate = m_bHasImageTemplate;
		pathFrameHost = _pathFrameHost;
		settings = _settings;
		strFolderEntry = _strFolderEntry;		
		strTableEntry = _strTableEntry;
		clrBG = m_clrBG;
		pathTemplateIndexFile = _pathTemplateIndexFile;
	}	

	if (!bValid)
	{
		strPageOut = g_szBlankHtmlPage;
	}

	CString strRef;
	IW::CFile f;

	if (nPage == -1)
	{
		if (bFrameHost && LoadTextFile(pathFrameHost, strPageOut))
		{
			// Do the replacments
			strPageOut.Replace(g_szTagHeader, App.Options.Web.Header);
			ReplaceConstants(thumbs, strPageOut, pAddress, strLocation, true, true);
			return true;
		}
		else
		{
			nPage = 0;
		}
	}

	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
	/// Write out the HTML
	int nImagePerPage = settings._sizeRowsColumns.cx * settings._sizeRowsColumns.cy;	
	int nCount = nImagePerPage;

	bool bTableOpen = false;

	CString strPerc;
	strPerc.Format(_T("%d%%"), 100 / settings._sizeRowsColumns.cx);

	int nItemCount = thumbs.size(); //nFileCount + nFolderCount;

	if (nItemCount < ((nImagePerPage * nPage) + nImagePerPage))
	{		
		nCount = nItemCount - (nImagePerPage * nPage);
	}

	int nPageCount = nItemCount / nImagePerPage;

	if ((nPageCount * nImagePerPage) < nItemCount)
	{
		nPageCount++;
	}

	strIndexTable += g_szTableEntryStart;
	//strIndexTable += bAltRow ? "PayloadAlt" : "Payload";

	int i;

	for(i = 0; i < nCount; i++)
	{
		int nImage = (nImagePerPage * nPage) + i;
		IW::FolderItemPtr pItem = thumbs[nImage];

		IW::FolderItemLoader job(_state.Cache);
		if (pItem->LoadJobBegin(job))
		{
			job.LoadImage(&loader, Search::Any);
			job.RenderAndScale();			
		}
		pItem->LoadJobEnd(job);

		bool bIsFolder = pItem->IsFolder();		
		
		// Add information to index table
		int nInRow = nImage % settings._sizeRowsColumns.cx;
		int nInCol = nImage / settings._sizeRowsColumns.cx;

		bool bAltRow = (nInCol & 0x01);
				
		// If it is the start of a row?
		if (nInRow == 0)
		{
			
			strIndexTable += _T("<TR>\n");
			bTableOpen = true;
		}
		
		CString strEntry;

		if (bIsFolder)
		{
			strEntry = strFolderEntry;		
		}
		else
		{
			strEntry = strTableEntry;		
		}		
		
		// Thumb text
		CString str;
		pItem->GetFormatText(str, settings.m_annotations, true);
		str = IW::TextToHtmlFriendly(str);
		str.Replace(g_sz1310, g_szBR);
		str.Replace(g_szQuot, g_sz39);
		strEntry.Replace(g_szTagThumbText, str);		

		// Image File
		CString strFileName = pItem->GetFileName();
		
		if (bIsFolder)
		{
			strRef = pAddress->GetFolderURL(strLocation, strFileName, true);
		}
		else if (pItem->IsHTMLDisplayable(_state.Plugins) && bHasImageTemplate)
		{
			strRef = pAddress->GetImageURL(strLocation, strFileName, true);
		}
		else
		{
			strRef = pAddress->GetImageFileURL(strLocation, strFileName, true);
		}

		strEntry.Replace(g_szTagImage, strRef);


		strRef = pAddress->GetThumbnailURL(strLocation, strFileName, clrBG, true);

		strEntry.Replace(g_szTagThumbFile, strRef);
		strEntry.Replace(g_szTagThumbFileOld, strRef);
		
		// Figure out the width and height
		bool bIsImage = pItem->IsImage();
		bool bIsImageIcon = pItem->IsImageIcon();
		CSize sizeRaw = pAddress->ThumbnailSize(pItem, &loader);
				
		strEntry.Replace(g_szTagThumbWidth, IW::IToStr(sizeRaw.cx));
		strEntry.Replace(g_szTagThumbHeight, IW::IToStr(sizeRaw.cy));
		strEntry.Replace(g_szTagPercWidth, strPerc);

		const IW::Image &image = pItem->GetImage();
		CString strTitle = IW::TextToHtmlFriendly(image.GetTitle());
		if (strTitle.IsEmpty()) strTitle = pItem->GetDisplayName();
		CString strAlt = IW::TextToHtmlFriendly(image.GetDescription());		
		if (strAlt.IsEmpty()) IW::TextToHtmlFriendly(strTitle);	

		strEntry.Replace(g_szTagThumbAlt, strAlt);
		strEntry.Replace(g_szTagThumbTitle, strFileName);

		// Image Width and Height
		const IW::CameraSettings &cameraSettings = pItem->GetCameraSettings();

		strEntry.Replace(g_szTagImageWidth, IW::IToStr(cameraSettings.OriginalImageSize.cx));
		strEntry.Replace(g_szTagImageHeight, IW::IToStr(cameraSettings.OriginalImageSize.cy));
		strEntry.Replace(g_szTagImageDepth, cameraSettings.OriginalBpp.ToString());
		
		// The entry is done
		strIndexTable += strEntry;
		strIndexTable += _T("\n");
		
		// End of a row
		if (nInRow == (settings._sizeRowsColumns.cx - 1))
		{
			strIndexTable += _T("</TR>");
			bTableOpen = false;
		}		
	}	

	if (bTableOpen)
	{
		strIndexTable += _T("</TR>");
	}

	strIndexTable += _T("</TABLE>\n");
	
	if (!LoadTextFile(pathTemplateIndexFile, strPageOut))
	{
		return false;
	}

	// We want to build up the 
	// navigation section
	CString strNavigation;
	CString strIndexName;

	//do we have a prev
	if (nPage > 0)
	{
		strIndexName = pAddress->GetIndexURL(strLocation, nPage-1, true);

		strNavigation += _T("<a href=\"");
		strNavigation += strIndexName;
		strNavigation += _T("\" target=\"_self\">[&lt;&lt; Prev]</a>&nbsp;\n");

		strPageOut.Replace(g_szTagBack, strIndexName);
	}
	else
	{
		strIndexName = pAddress->GetIndexURL(strLocation, nPageCount-1, true);
		strPageOut.Replace(g_szTagBack, strIndexName);
	}

	for(i = 0; i < nPageCount; i++)
	{
		if (nPage == i)
		{
			strNavigation += _T("<b>");
			strNavigation += IW::IToStr(i + 1);
			strNavigation += _T("</b>&nbsp;");
		}
		else
		{
			strIndexName = pAddress->GetIndexURL(strLocation, i, true);

			strNavigation += _T("<a href=\"");
			strNavigation += strIndexName;
			strNavigation += _T("\" target=\"_self\">");
			strNavigation += IW::IToStr(i + 1);
			strNavigation += _T("</a>\n");
			
		}
	}

	//do we have a next
	if (nPage < (nPageCount-1))
	{
		strIndexName = pAddress->GetIndexURL(strLocation, nPage+1, true);

		strNavigation += _T("<a href=\"");
		strNavigation += strIndexName;
		strNavigation += _T("\" target=\"_self\">[Next &gt;&gt;]</a>\n");

		strPageOut.Replace(g_szTagNext, strIndexName);
	}
	else
	{
		strIndexName = pAddress->GetIndexURL(strLocation, 0, true);
		strPageOut.Replace(g_szTagNext, strIndexName);
	}
	
	
	// Do the replacments
	strPageOut.Replace(g_szTagHeader, App.Options.Web.Header);
	strPageOut.Replace(g_szTagBody, strIndexTable);	

	if (nPageCount > 1)
	{
		strPageOut.Replace(g_szTagNavigation, strNavigation);
	}
	else
	{
		strPageOut.Replace(g_szTagNavigation, g_szEmptyString);
	}


	CString strContext;
	if (settings.m_bBreadCrumbs) strContext = pAddress->GetBreadCrumbs(strLocation, true);
	strPageOut.Replace(g_szTagContext, strContext);	

	// Keywords & Desription
	strPageOut.Replace(g_szTagKeyWords, App.Options.Web.Header);	
	strPageOut.Replace(g_szTagDescription, App.Options.Web.Header);	

	ReplaceConstants(thumbs, strPageOut, pAddress, strLocation, false, true);
	
	return true;
}

bool CWebPage::GetIndex(CAddressPolicy *pAddress, const CString &strLocation, int nPage, CString &strPageOut)
{
	IW::ITEMLIST thumbs;

	{
		IW::CAutoLockCS lock(_cs);

		IW::FolderPtr pFolderIn = new IW::RefObj<IW::Folder>;
		if (!pFolderIn->Init(strLocation))
		{
			strPageOut = _T("<HTML><BODY>Page ");
			strPageOut += strLocation;
			strPageOut += _T(" not found.</BODY></HTML>");
			return false; 
		}

		thumbs = pAddress->GetItemList(pFolderIn, m_bCannotRecurse);
	}

	return GetIndex(thumbs, pAddress, strLocation, nPage, strPageOut);
}

bool CWebPage::GetImage(CAddressPolicy *pAddress, const CString &strLocation, const CString &strImage, int nPageReturn, CString &strPageOut)
{
	IW::ITEMLIST thumbs;

	{
		IW::CAutoLockCS lock(_cs);

		IW::FolderPtr pFolderIn = new IW::RefObj<IW::Folder>;
		if (!pFolderIn->Init(strLocation))
		{
			strPageOut = _T("<HTML><BODY>Page ");
			strPageOut += strLocation;
			strPageOut += _T(" not found.</BODY></HTML>");
			return false; 
		}

		thumbs = pAddress->GetItemList(pFolderIn, m_bCannotRecurse);
	}

	return GetImage(thumbs, pAddress, strLocation, strImage, nPageReturn, strPageOut);
}

bool CWebPage::ReplaceConstants(CString &str, CAddressPolicy *pAddress, const CString &strLocation, bool bIsFrameHost, bool bFromIndex)
{
	IW::CAutoLockCS lock(_cs);

	IW::FolderPtr pFolderIn = new IW::RefObj<IW::Folder>;
	if (!pFolderIn->Init(strLocation))
	{
		return false; 
	}

	IW::ITEMLIST thumbs = pAddress->GetItemList(pFolderIn, m_bCannotRecurse);

	return ReplaceConstants(thumbs, str, pAddress, strLocation, bIsFrameHost, bFromIndex);
}

bool CWebPage::ReplaceConstants(IW::ITEMLIST &thumbs, CString &str, CAddressPolicy *pAddress, const CString &strLocation, bool bIsFrameHost, bool bFromIndex)
{
	IW::CAutoLockCS lock(_cs);

	str.Replace(g_szTagFooter, App.Options.Web.Footer);
	str.Replace(g_szTagMainHeader, App.Options.Web.Header);
	str.Replace(g_szTagHome, pAddress->GetHome());	
	str.Replace(g_szTagTemplate, pAddress->GetIndexURL(strLocation, 0, bFromIndex));
	str.Replace(g_szTagImageTemplate, _pathImageTemplate);
	str.Replace(g_szTagFrameHost, _pathFrameHostShort);
	str.Replace(g_szTagStyleSheet, pAddress->GetTemplateURL(IW::Path::FindFileName(_pathStyleSheetFile), bFromIndex));
	str.Replace(g_szTagScripts, pAddress->GetTemplateURL(_T("ImageWalker.js"), bFromIndex));
	str.Replace(g_szTagImageBorderWidth, IW::IToStr(_settings.m_nImageBorderWidth));
	str.Replace(g_szDelay, IW::IToStr(_settings.m_nDelay));

	IW::FileTime now = IW::FileTime::Now().ToLocalTime();
	CString strTime = now.GetTimeFormat( App.GetLangId());
	CString strDate = now.GetDateFormat( App.GetLangId(), App.Options.m_bShortDates);

	str.Replace(g_szTagDate, strDate);
	str.Replace(g_szTagTime, strTime);
	

	_sizeThumbNail.cx = 80; // TODO ???
	str.Replace(_T("{{Thumb Width}}"), IW::IToStr(_sizeThumbNail.cx + 70));


	IW::CFilePath path, extension;

	for(int i = 0; i < m_arrayRequiredFiles.GetSize(); i++)
	{
		path = m_arrayRequiredFiles[i];
		extension = path;
		extension.StripToExtension();

		if (_tcsicmp(extension, _T(".gif")) == 0 ||
			_tcsicmp(extension, _T(".png")) == 0 ||
			_tcsicmp(extension, _T(".jpg")) == 0 ||
			_tcsicmp(extension, _T(".jpeg")) == 0)
		{
			str.Replace(m_arrayRequiredFiles[i], pAddress->GetTemplateURL(path, bFromIndex));
		}
		else
		{
			str.Replace(m_arrayRequiredFiles[i], pAddress->GetTemplateURL(path, bFromIndex));
		}
	}

	IW::FolderPtr pFolder = new IW::RefObj<IW::Folder>;
	if (pFolder->Init(strLocation))
	{	
		if (thumbs.size() > 0)
		{
			CString strFileName = thumbs[0]->GetFileName();
			str.Replace(g_szTagFirst, pAddress->GetImageURL(strLocation, strFileName, bFromIndex));
		}
	}


	// Any contextual constants to replace?
	pAddress->ReplaceConstants(str);

	return true;
}



bool CWebPage::GetThumbnail(CLoadAny *pLoader, const CString &strLocation, const CString &strFileName, IW::Image &imageOut, COLORREF clrBG)
{
	IW::CAutoLockCS lock(_cs);

	// Find our folder
	IW::FolderPtr pFolder = new IW::RefObj<IW::Folder>;
	if (!pFolder->Init(strLocation)) return false;

	int nItem = pFolder->Find(strFileName);
	if (nItem == -1) return false;	

	IW::FolderItemLock pItem(pFolder, nItem);

	IW::FolderItemLoader job(_state.Cache);
	if (pItem->LoadJobBegin(job))
	{
		job.LoadImage(pLoader, Search::Any);
		job.RenderAndScale();
	}
	pItem->LoadJobEnd(job);

	bool bIsImage = pItem->IsImage();
	bool bIsImageIcon = pItem->IsImageIcon();

	if (bIsImage)
	{
		imageOut = pItem->GetImage();
	}
	else
	{		
		IW::Image imageIcon;
		CRect rcImageIcon;

		int nWidth = 32;
		int nHeight = 32;		

		if (bIsImageIcon)
		{
			imageIcon = pItem->GetImage();
			rcImageIcon = imageIcon.GetBoundingRect();

			nWidth = IW::Max(32, rcImageIcon.Width()) + (nFolderSpreadX * 2);
			nHeight = IW::Max(32, rcImageIcon.Height()) + (nFolderSpreadY * 2);
		}	

		IW::CRender render;
		CRect rcCreate(0, 0, nWidth, nHeight);
		
		if (render.Create(NULL, rcCreate))
		{	
			render.Fill(clrBG);

			// May need to draw image icon
			if (bIsImageIcon)
			{
				rcImageIcon += CPoint(nWidth - rcImageIcon.Width(), nHeight - rcImageIcon.Height());
				render.DrawImage(imageIcon.GetFirstPage(), rcImageIcon);
			}

			render.DrawImageList(App.GetShellImageList(false), pItem->GetImageNum(), 0, 0, 32, 32);
			render.RenderToSurface(imageOut);
		}
	}

	for(IW::Image::PageList::iterator page = imageOut.Pages.begin(); page != imageOut.Pages.end(); ++page)
	{
		page->SetBackGround(clrBG);
	}

	return true; 
}



bool CWebPage::IterateGallery(CGalleryClient *pClient, CAddressPolicy *pAddress, IW::Folder *pFolder, IW::IStatus *pStatus)
{
	CString strLocation = pFolder->GetFolderPath();
	CLoadAny loader(_state.Plugins);

	CString strFolderMessage;
	strFolderMessage.Format(IDS_PROCESSING_FOLDER_FMT, strLocation);
	pStatus->SetHighLevelStatusMessage(strFolderMessage);	

	IW::ITEMLIST thumbs = pAddress->GetItemList(pFolder, m_bCannotRecurse);
	
	int nImagePerPage = _settings._sizeRowsColumns.cx * _settings._sizeRowsColumns.cy;	
	int nCount = nImagePerPage;
	int nItemCount = thumbs.size(); //nFileCount + nFolderCount;
	int nPageCount = nItemCount / nImagePerPage;

	
	if ((nPageCount * nImagePerPage) < nItemCount)
	{
		nPageCount++;
	}

	pClient->Init(m_bFrameHost);

	if (m_bFrameHost)
	{
		CString strPage;
		if (!GetIndex(thumbs, pAddress, strLocation, -1, strPage))
		{
			return false;
		}

		pClient->OnFrameHost(strLocation, strPage, pStatus);
	}

	// Iterate through the index pages
	for(int nPage = 0; nPage < nPageCount; ++nPage)
	{
		if (nItemCount < ((nImagePerPage * nPage) + nImagePerPage))
		{		
			nCount = nItemCount - (nImagePerPage * nPage);
		}		

		for(int i = 0; i < nCount; i++)
		{
			int nImage = (nImagePerPage * nPage) + i;
			IW::FolderItemPtr pItem = thumbs[nImage];

			pStatus->SetHighLevelProgress(nImage, thumbs.size());

			IW::FolderItemLoader job(_state.Cache);
			if (pItem->LoadJobBegin(job))
			{
				job.LoadImage(&loader, Search::Any);
				job.RenderAndScale();
			}
			pItem->LoadJobEnd(job);	
			
			// Add information to index table
			int nInRow = nImage % _settings._sizeRowsColumns.cx;
			int nInCol = nImage / _settings._sizeRowsColumns.cx;

			bool bAltRow = (nInCol & 0x01);
			
			CString strFileName = pItem->GetFileName();
			pStatus->SetContext(strFileName);			

			if (!m_bNoThumbnails)
			{			
				// Image
				pClient->OnThumbnail(
					&loader, strLocation, 
					strFileName, m_clrBG,
					pStatus); 
			}

			bool bIsFolder = pItem->IsFolder();

			CString strImagePage;
			GetImage(thumbs, pAddress, strLocation, strFileName, nPage, strImagePage);
			pClient->OnImage(strLocation, strFileName, bIsFolder, strImagePage, pStatus);
			
			if (bIsFolder)
			{
				// On Folder
				pClient->OnFolder(strLocation, strFileName, pStatus);
			}

			if (pStatus->QueryCancel())
			{
				pStatus->SetError(App.LoadString(IDS_CANCELED));
				return false;
			}		
		}

		CString strPage;

		if (!GetIndex(thumbs, pAddress, strLocation, nPage, strPage))
		{
			return false;
		}

		pClient->OnIndex(strLocation, nPage, strPage, pStatus);
	}	

	

	ProcessRequiredFiles(pClient);
	ProcessStyleSheet(pClient);

	return true;
}

void CWebPage::ProcessRequiredFiles(CGalleryClient* pClient)
{
	IW::CFilePath path, extension;

	for(int i = 0; i < m_arrayRequiredFiles.GetSize(); i++)
	{
		IW::CFilePath path(_pathTemplateFolder);
		path += m_arrayRequiredFiles[i];

		pClient->OnRequiredFile(path, true);
	}

	path.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
	path += _T("ImageWalker.js");
	pClient->OnRequiredFile(path, true);
}

void CWebPage::ProcessStyleSheet(CGalleryClient* pClient)
{
	CString strCSS, str;
	LoadTextFile(_pathStyleSheetFile, strCSS);

	CAtlRegExp<> re;
	REParseError status = re.Parse(_T("{.*}background-image: *url\\(\"{.*}\"\\);.*"), false);
	ATLASSERT(REPARSE_ERROR_OK == status);

	CAtlREMatchContext<> mc;

	while (re.Match(strCSS, &mc))
	{
		const CAtlREMatchContext<>::RECHAR *szStart = 0, *szEnd = 0;

		mc.GetMatch(1, &szStart, &szEnd);
		str.SetString(szStart, szEnd - szStart);

		IW::CFilePath path(_pathTemplateFolder);
		path += str;
		pClient->OnRequiredFile(path, true);

		mc.GetMatch(0, &szStart, &szEnd);
		str.SetString(szStart, szEnd - szStart);
		strCSS = str;
	}

	pClient->OnRequiredFile(_pathStyleSheetFile, true);
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
///
///  CAddressPolicyPreview
///

CAddressPolicyPreview::CAddressPolicyPreview(State &state, const CWebSettings &settings) : _state(state), _settings(settings)
{
}

CString CAddressPolicyPreview::GetIndexURL(const CString &strLocation, int i, bool bFromIndex)
{
	CURLProperties properties;
	_settings.Write(&properties);
	properties.Write(_T("idx"), i);
	properties.Write(_T("loc"), strLocation);

	return _T("IW231:///Index?") + properties.ToString();	
}

CString CAddressPolicyPreview::GetImageURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	CURLProperties properties;
	_settings.Write(&properties);
	//properties.Write(_T("idx"), i);
	properties.Write(_T("loc"), strLocation);
	properties.Write(g_szName, strName);

	return _T("IW231:///Image?") + properties.ToString();	
}

CString CAddressPolicyPreview::GetFolderURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	CURLProperties properties;
	_settings.Write(&properties);
	properties.Write(_T("idx"), -1);
	properties.Write(_T("loc"), IW::Path::Combine(strLocation, strName));

	return _T("IW231:///Index?") +  properties.ToString();
}

CString CAddressPolicyPreview::GetThumbnailURL(const CString &strLocation, const CString &strName, COLORREF clrBG, bool bFromIndex)
{
	CURLProperties properties;
	_settings.Write(&properties);
	properties.Write(_T("loc"), strLocation);
	properties.Write(g_szName, strName);
	properties.Write(_T("bg"), (clrBG & 0x00ffffff));

	return _T("IW231:///Thumbnail?") + properties.ToString();	
}

CString CAddressPolicyPreview::GetFileURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	IW::CFilePath path(strLocation);
	path += strName;

	return _T("FILE:///") + path.ToString();
}

CString CAddressPolicyPreview::GetImageFileURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	IW::CFilePath path(strLocation);
	path += strName;

	return _T("FILE:///") + path.ToString();
}

CString CAddressPolicyPreview::GetTemplateURL(const CString &strLocation, bool bFromIndex)
{
	CURLProperties properties;
	properties.Write(_T("name"), strLocation);

	return _T("IW231:///File?") + properties.ToString();	
}

CString CAddressPolicyPreview::GetBreadCrumbs(const CString &strLocation, bool bFromIndex)
{
	// Now do the Context
	CString strRef, strFolders;
	IW::CFilePath path;

	// Loop through and get all folders
	LPCTSTR szSeps = _T("\\/");
	CString str(strLocation);
	CString token;
	int curPos = 0;

	token = str.Tokenize(szSeps, curPos);
	while (!token.IsEmpty())
	{
		path += token;
		path.Normalize(true);
		strRef = GetIndexURL(path, -1, bFromIndex);

		strFolders += _T("&nbsp;&gt;&nbsp;");
		strFolders += _T("<a Target='_top' href='");
		strFolders += strRef;
		strFolders += _T("'>");
		strFolders += token;
		strFolders += _T("</a>");	

		token = str.Tokenize(szSeps, curPos);
	}

	return strFolders;
}

CString CAddressPolicyPreview::GetHome() 
{ 
	return _T("http://www.imagewalker.com/"); 
};


IW::ITEMLIST CAddressPolicyPreview::GetItemList(IW::Folder *pFolder, bool bCannotRecurse)
{
	IW::ITEMLIST thumbs;	

	// Remove folders?
	int nSize = pFolder->GetItemCount();
	thumbs.reserve( nSize );

	for(int nItem = 0; nItem < nSize; ++nItem)
	{
		IW::FolderItemLock pItem(pFolder, nItem);
		if (pItem->IsHTMLDisplayable(_state.Plugins) || pItem->IsFolder())
		{
			thumbs.push_back(pItem);				
		}
	}

	pFolder->Sort(thumbs, 0, true);

	return thumbs;
}

void CAddressPolicyPreview::ReplaceConstants(CString &strOut)
{
}

CSize CAddressPolicyPreview::ThumbnailSize(IW::FolderItemPtr pItem, CLoadAny *pLoader)
{
	IW::FolderItemLoader job(_state.Cache);

	if (pItem->LoadJobBegin(job))
	{
		job.LoadImage(pLoader, Search::Any);
		job.RenderAndScale();
	}
	pItem->LoadJobEnd(job);

	bool bIsImage = pItem->IsImage();
	bool bIsImageIcon = pItem->IsImageIcon();

	int cx = 0, cy = 0;

	// Calc the actual icon size
	if (bIsImage)
	{
		const CRect rcBounding = pItem->GetItemThumbRect();

		cx = rcBounding.Width();
		cy = rcBounding.Height();
	}
	else
	{
		cx = 32;
		cy = 32;

		if (bIsImageIcon)
		{
			const CRect rcImageIcon = pItem->GetItemThumbRect();

			cx = IW::Max(32, rcImageIcon.Width()) + (nFolderSpreadX * 2);
			cy = IW::Max(32, rcImageIcon.Height()) + (nFolderSpreadY * 2);
		}
	}	

	return CSize(cx, cy);
}
