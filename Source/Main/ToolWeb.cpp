// ToolWeb.cpp: implementation of the CToolWeb class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolWeb.h"
#include "Html.h"
#include "ImageStreams.h"


static LPCTSTR g_szDisableRightClick = _T("DisableRightClick");
static LPCTSTR g_szAutoRun = _T("AutoRun");
static LPCTSTR m_szAuxFolder = _T("AuxFolder");
static LPCTSTR g_szSortOrder = _T("SortOrder");
static LPCTSTR g_szSharpen = _T("Sharpen");
static LPCTSTR g_szAssending = _T("Assending");


class ScopeLockInc
{
private:
	int &_n;

public:

	ScopeLockInc(int &n) : _n(n)
	{
		_n++;
	}

	~ScopeLockInc()
	{
		_n -= 1;
	}
};

static int PopulateCombo(const CString &strTempPath, HWND hCombo, int nSel, CStringArray &ar)
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
			LPTSTR pDest = _tcschr( FindFileData.cFileName, _T('.') );
			
			if (pDest)
			{
				*pDest = 0;
			}
			
			
			IW::SetItem(hCombo, FindFileData.cFileName, i++);
		}
		while(FindNextFile(hSearch, &FindFileData));

		// Close the search handle. 
		FindClose(hSearch);
	}
	
	::SendMessage(hCombo, CB_SETCURSEL, nSel, 0);
	
	return i;
}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CToolWeb::CToolWeb(CWebSettings &options, State &state) : 
	BaseClass(state),
	_settings(options), 
	m_pageWebOutput(this),
	m_pageWebThumbnails(this, state.Plugins),
	_state(state),
	_generator(state),
	_loader(state.Plugins)
{
	m_pLoaderFactory = 0;

	m_bFrameHost = false;
	m_nFolderDepth = 0;

	_strOutputFile = _T("index.html");
	_strHome = _T("http://www.ImageWalker.com/");

	_strExt = _T(".html");

	_sizeThumbNail.cx = 80;
	_sizeThumbNail.cy = 80;

	_strType = _T("JPG");
	m_bTableOpen = false;
	m_nCurrentImage = 0;
	m_bSameSourceAndDestinationFolders = false;
	_strOutputFolder = IW::Path::PersonalFolder();

	m_bLowerCase = true;
	m_bRemoveIllegal = true;
	m_bDisableRightClick = false;
	m_bAutoRun = false;
	m_bSharpen = true;
	m_bAuxFolder = false;

	_nSortOrder = 0;
	_bAssending = true;

	_nImageNumber = 0;
	_nImagesCount = 0;
	m_nIndexNumber = 0;
	m_nTotalImageNumber = 0;
	m_nLoaderSelection = 0;
}

CToolWeb::~CToolWeb()
{
	Free();
}

void CToolWeb::Free()
{
	m_pLoader = 0;
	m_pLoaderFactory = 0;
}

void CToolWeb::Read(const IW::IPropertyArchive *pArchive, bool bFullRead)
{
	pArchive->Read(g_szRecurse, _bRecurse);

	pArchive->Read(g_szOutputFile, _strOutputFile);
	pArchive->Read(g_szFileExt, _strExt);
	pArchive->Read(g_szOutputFolder, _strOutputFolder);
	pArchive->Read(g_szHome, _strHome);	
	pArchive->Read(g_szThumbCX, _sizeThumbNail.cx);
	pArchive->Read(g_szThumbCY, _sizeThumbNail.cy);
	pArchive->Read(g_szType, _strType);
	pArchive->Read(g_szLowerCase, m_bLowerCase);
	pArchive->Read(g_szRemoveIllegal, m_bRemoveIllegal);
	pArchive->Read(g_szDisableRightClick, m_bDisableRightClick);
	pArchive->Read(m_szAuxFolder, m_bAuxFolder);	
	pArchive->Read(g_szAutoRun, m_bAutoRun);
	pArchive->Read(g_szSharpen, m_bSharpen);
	pArchive->Read(g_szSortOrder, _nSortOrder);
	pArchive->Read(g_szAssending, _bAssending);

	if (bFullRead)
	{
		if (pArchive->StartSection(g_szOptions))
		{
			_settings.Read(pArchive);
			pArchive->EndSection();
		}
	}


}

void CToolWeb::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szRecurse, _bRecurse);
	pArchive->Write(g_szOutputFile, _strOutputFile);
	pArchive->Write(g_szHome, _strHome);
	pArchive->Write(g_szFileExt, _strExt);
	pArchive->Write(g_szOutputFolder, _strOutputFolder);
	pArchive->Write(g_szThumbCX, _sizeThumbNail.cx);
	pArchive->Write(g_szThumbCY, _sizeThumbNail.cy);
	pArchive->Write(g_szType, _strType);
	pArchive->Write(g_szLowerCase, m_bLowerCase);
	pArchive->Write(g_szRemoveIllegal, m_bRemoveIllegal);
	pArchive->Write(g_szDisableRightClick, m_bDisableRightClick);
	pArchive->Write(m_szAuxFolder, m_bAuxFolder);
	pArchive->Write(g_szAutoRun, m_bAutoRun);
	pArchive->Write(g_szSharpen, m_bSharpen);
	pArchive->Write(g_szSortOrder, _nSortOrder);
	pArchive->Write(g_szAssending, _bAssending);

	if (pArchive->StartSection(g_szOptions))
	{
		_settings.Write(pArchive);
		pArchive->EndSection();
	}
}

bool CToolWeb::WriteTextFile(const CString &strFileName, const CString &strText, IW::IStatus *pStatus)
{
	IW::CFile f;

	if (f.OpenForWrite(strFileName))
	{		
		CStringA strHtmlA = strText;
		f.Write(strHtmlA, strHtmlA.GetLength());
		return f.Close(pStatus);
	}	

	return false;	
}

CString CToolWeb::GetKey() const
{
	return _T("CreateWebPageWizard");
} 

CString CToolWeb::GetTitle() const
{
	return App.LoadString(IDS_TOOL_WEB_TITLE);
}

CString CToolWeb::GetSubTitle() const
{
	return App.LoadString(IDS_TOOL_WEB_SUBTITLE);
}

CString CToolWeb::GetDescription() const
{
	return App.LoadString(IDS_TOOL_WEB_DESC);
}

CString CToolWeb::GetAboutToProcessText() const
{
	return _strAboutToText;
}

CString CToolWeb::GetCompletedText() const
{
	return App.LoadString(IDS_TOOL_WEB_COMPLETE);
}

CString CToolWeb::GetCompletedShowText() const
{
	return App.LoadString(IDS_TOOL_WEB_SHOW);
}

// Control
void CToolWeb::OnAddPages()
{
	AddPage(m_pageInput);
	AddPage(m_pageWebThumbnails);
	AddPage(m_pageWebOutput);
}

void CToolWeb::OnProcess(IW::IStatus *pStatus)
{
	// Primary output folder
	IW::CFilePath pathOutputFolder(_strOutputFolder);
	pathOutputFolder.TerminateFolderPath();
	pathOutputFolder.MakeUnixPath();	

	// Check if the current and destination folders are the same
	m_bSameSourceAndDestinationFolders = pathOutputFolder == IW::CFilePath(GetFolderPath());

	// Sub folder
	IW::CFilePath pathOutputFile(_strOutputFile);
	pathOutputFile.StripToFilename();
	_strSubFolder = pathOutputFile.ToString();

	// Clean folder list
	m_arrayFolderNames.RemoveAll();

	//_generator.NewFolder(szFolderName);
	_generator.SetSettings(_settings);	

	IW::FolderPtr pFolder = _state.Folder.GetFolder();
	ScopeLockFolderStack folderStack(m_arrayFolderNames, pFolder);

	if (CreateAllDirectories())
	{
		_generator.IterateGallery(this, this, pFolder, pStatus);
	}
}

void CToolWeb::OnComplete(bool bShow)
{
	if (bShow)
	{
		CString strIndexURL;
		
		// Open the web page to have
		//  a look
		IW::CFilePath pathIndex;
		MakeIndexPath(pathIndex, -1);

		IW::CFilePath path(_strOutputFolder);
		path += pathIndex;
		
		IW::NavigateToWebPage(path);
	}

	// Just those 3 file needed for autorun
	if (m_bAutoRun)
	{
		static LPCTSTR g_szAutoRunInf = _T("[autorun]\r\nopen=start {{IndexName}}\r\nicon=iw.ico");
		static LPCTSTR g_szAutoRunInfTag = _T("{{IndexName}}");
		
		IW::CFilePath pathIndex;
		MakeIndexPath(pathIndex, -1);

		CString str(g_szAutoRunInf);
		str.Replace(g_szAutoRunInfTag, pathIndex);

		IW::CFilePath path(_strOutputFolder);
		path += _T("Autorun.inf");
		WriteTextFile(path, str, 0);

		// Required Files?
		path.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
		path += _T("iw.ico");
		OnRequiredFile(path, false);

		path.GetTemplateFolder(_AtlBaseModule.GetModuleInstance());
		path += _T("Start.exe");
		OnRequiredFile(path, false);
	}
}

bool CToolWeb::OnFolder(const CString &strLocation, const CString &strPageOut, IW::IStatus *pStatus)
{
	ScopeLockInc inc(m_nFolderDepth);
	ScopeLockFolderStack folderStack(m_arrayFolderNames, strPageOut);	

	if (!CreateAllDirectories())
	{
		return false;
	}

	CString strFolderMessage;
	strFolderMessage.Format(IDS_PROCESSING_FOLDER_FMT, strPageOut);
	pStatus->SetHighLevelStatusMessage(strFolderMessage);

	IW::CFilePath pathSubFolder(strLocation);
	pathSubFolder += strPageOut;

	IW::FolderPtr pFolderIn = new IW::RefObj<IW::Folder>;
	if (!pFolderIn->Init(pathSubFolder))
	{
		return false;
	}

	_generator.IterateGallery(this, this, pFolderIn, pStatus);
	

	return true;
}


bool CToolWeb::OnIndex(const CString &strLocation, int nPage, const CString &strPageOut, IW::IStatus *pStatus)
{
	IW::CFilePath pathIndex;
	MakeIndexPath(pathIndex, nPage);

	IW::CFilePath path;
	MakeFilePath(path, pathIndex, g_szEmptyString);
	
	IW::CFileTemp f;
	if (!f.OpenForWrite(path))
	{
		CString str;
		str.Format(IDS_FAILEDTOWRITEINDEXFILE, path);
		pStatus->SetError(str);
		return false;
	}

	CStringA strPage = strPageOut;
  	f.Write(strPage, strPage.GetLength());

	return f.Close(pStatus);
}

bool CToolWeb::OnImage(const CString &strLocation, const CString &strImage, bool bIsFolder, const CString &strPageOut, IW::IStatus *pStatus)
{
	IW::CFilePath path;
	MakeFilePath(path, strImage, _strExt);
	if (m_bAuxFolder) path.PrefixAsPath(_strSubFolder, true);

	IW::CFileTemp f;
	if (!f.OpenForWrite(path))
	{
		pStatus->SetError(App.LoadString(IDS_FAILEDTOWRITEHTML));
	}
	
	CStringA strPage = strPageOut;
  	f.Write(strPage, strPage.GetLength());

	if (!f.Close(pStatus))
	{
		return false;
	}

	// Copy over image file?
	if (!bIsFolder)
	{
		IW::CFilePath pathDst, pathSrc(strLocation);
		pathSrc += strImage;
		MakeFilePath(pathDst, strImage, g_szEmptyString);

		if (m_bSameSourceAndDestinationFolders)
		{
			MoveFile(pathSrc, pathDst);
		}
		else
		{
			if (!CopyFile(pathSrc, pathDst, FALSE))
			{
				pStatus->SetError(App.LoadString(IDS_FAILEDTOWRITEFILE));
			}
		}
	}
	

	return true;
}
 
bool CToolWeb::OnThumbnail(CLoadAny *pLoader, const CString &strLocation, const CString &strFileName, COLORREF clrBackGround, IW::IStatus *pStatus)
{
	// Find our folder
	IW::FolderPtr pFolder = new IW::RefObj<IW::Folder>;
	if (!pFolder->Init(strLocation))
		return false;

	int nItem = pFolder->Find(strFileName);
	if (nItem == -1) return false;

	IW::FolderItemLock pItem(pFolder, nItem);
		
	IW::FolderItemLoader job(_state.Cache, _sizeThumbNail);
	if (pItem->LoadJobBegin(job))
	{
		job.LoadImage(pLoader, Search::Any, true);
		job.RenderAndScale();
	}
	pItem->LoadJobEnd(job);	

	bool bIsImage = pItem->IsImage();
	bool bIsImageIcon = pItem->IsImageIcon();
	CString strFile = pItem->GetFilePath();
	IW::Image imageOut, imageIcon;

	if (bIsImage)
	{
		imageOut = pItem->GetImage();
	}
	else
	{
		CWindowDC dcDesktop(GetDesktopWindow());
		CBitmap bitmap;
		CDC dcCompatible;
		CRect rcImageIcon;

		int nWidth = 32;
		int nHeight = 32;
		const int nFolderSpreadX = 10;
		const int nFolderSpreadY = 4;

		if (bIsImageIcon)
		{
			imageIcon = pItem->GetImage();
			rcImageIcon = imageIcon.GetBoundingRect();

			nWidth = IW::Max(32, rcImageIcon.Width()) + (nFolderSpreadX * 2);
			nHeight = IW::Max(32, rcImageIcon.Height()) + (nFolderSpreadY * 2);
		}

		m_mapThumbToSize[strFile] = CSize(nWidth, nHeight);
		
		
		if (bitmap.CreateCompatibleBitmap(dcDesktop, nWidth, nHeight) && 
			dcCompatible.CreateCompatibleDC(dcDesktop))
		{
			HBITMAP hBitmapOld = dcCompatible.SelectBitmap(bitmap);

			if (hBitmapOld)
			{
				CRect rc(0,0,nWidth,nHeight);
				dcCompatible.FillSolidRect(&rc, clrBackGround);

				// May need to draw image icon
				if (bIsImageIcon)
				{
					rcImageIcon += CPoint(nWidth - rcImageIcon.Width(), nHeight - rcImageIcon.Height());
					IW::CRender::DrawToDC(dcCompatible, imageIcon.GetFirstPage(), rcImageIcon);
				}
				
				UINT uStyle = ILD_TRANSPARENT;			
				ImageList_Draw(App.GetShellImageList(false), 
					pItem->GetImageNum(), dcCompatible, 
					0, 0, uStyle);
				
				dcCompatible.SelectBitmap(hBitmapOld);		
				imageOut.Copy(dcDesktop, bitmap);
			}
		}
	}
	

	// We have an image?
	ATLASSERT(imageOut.GetPageCount() > 0);
	imageOut.ClearMetaData();

	if (m_bSharpen)
	{
		IW::Image image;
		long kernel[]={-1,-1,-1,-1,15,-1,-1,-1,-1};
		Filter(imageOut, image, kernel,3,7,0, pStatus);
		imageOut = image;
	}

	for(IW::Image::PageList::iterator page = imageOut.Pages.begin(); page != imageOut.Pages.end(); ++page)
	{
		page->SetBackGround(clrBackGround);
	}

	IW::CFilePath path, pathName(strFileName);
	pathName.PrefixToFileName(_T("_"));
	MakeFilePath(path, pathName, m_pLoaderFactory->GetExtensionDefault());

	if (m_bAuxFolder) path.PrefixAsPath(_strSubFolder, true);	
	
	IW::CFileTemp f;
	if (!f.OpenForWrite(path))
	{
		CString str;
		str.Format(IDS_FAILEDTOWRITETHUMBNAIL, path);
		pStatus->SetError(str);
		return false;
	}
	
	m_pLoader->Write(g_szEmptyString, &f, imageOut, pStatus);

	const CRect rcBounding = imageOut.GetBoundingRect();
	m_mapThumbToSize[strFile] = rcBounding.Size();

	return f.Close(pStatus);
}

void CToolWeb::MakeIndexPath(IW::CFilePath &path, int i)
{
	TCHAR strFileName[MAX_PATH + 1];
	_tsplitpath_s(_strOutputFile, NULL, 0, NULL, 0, strFileName, countof(strFileName), NULL, 0);

	if ((i >= 0 && m_bFrameHost) || 
		(i > 0 && !m_bFrameHost))
	{
		CString str;
		str.Format(_T("%s%d"), strFileName, i + 1);
		path += str;
	}
	else
	{
		path += strFileName;
	}	
	
	path.SetExtension(_strExt);
	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();
	path.MakeUnixPath();
}

////////////////////////////////////////////////////////////
/// CAddressPolicy Enry Points

CString CToolWeb::GetIndexURL(const CString &strLocation, int i, bool bFromIndex)
{
	IW::CFilePath path;
	MakeIndexPath(path, i);

	if (m_bAuxFolder && !bFromIndex) path.PrefixAsPath(_T(".."), false);

	return IW::MakeURLSafe(path);
}

CString CToolWeb::GetImageURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	IW::CFilePath path(strName);
	path.SetExtension(_strExt);	
	
	path.StripToFilenameAndExtension();

	if (m_bAuxFolder && bFromIndex) path.PrefixAsPath(_strSubFolder, false);
	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();

	return IW::MakeURLSafe(path);
}

CString CToolWeb::GetFolderURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	IW::CFilePath pathIndex;
	MakeIndexPath(pathIndex, -1);

	IW::CFilePath path(strName);
	path += pathIndex;
	
	if (m_bAuxFolder && !bFromIndex) path.PrefixAsPath(_T(".."), false);
	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegal();

	path.MakeUnixPath();
	return IW::MakeURLSafe(path);
}

CString CToolWeb::GetThumbnailURL(const CString &strLocation, const CString &strName, COLORREF clrBG, bool bFromIndex)
{
	IW::CFilePath path(strName);
	path.StripToFilenameAndExtension();
	path.PrefixToFileName(_T("_"));
	path.SetExtension(m_pLoaderFactory->GetExtensionDefault());	

	if (m_bAuxFolder && bFromIndex) path.PrefixAsPath(_strSubFolder, false);
	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();

	path.MakeUnixPath();
	return IW::MakeURLSafe(path);
}


CString CToolWeb::GetFileURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	IW::CFilePath path(strName);
	path.Prefix(GetTemplatePathPrefix(bFromIndex));

	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();

	path.MakeUnixPath();
	return IW::MakeURLSafe(path);
}



CString CToolWeb::GetImageFileURL(const CString &strLocation, const CString &strName, bool bFromIndex)
{
	IW::CFilePath path(strName);	

	if (m_bAuxFolder && !bFromIndex)
	{
		path.PrefixAsPath(_T(".."), false);
	}

	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();

	path.MakeUnixPath();
	return IW::MakeURLSafe(path);
}

CString CToolWeb::GetTemplatePathPrefix(bool bFromIndex) const
{
	CString strPrefix;

	for(int i = 1; i < m_arrayFolderNames.GetSize(); ++i)
	{
		strPrefix += _T("../");
	}

	if (m_bAuxFolder)
	{
		if (!bFromIndex) strPrefix += _T("../");
		strPrefix += _strSubFolder + _T("/");
	}

	return strPrefix;
}

CString CToolWeb::GetTemplateURL(const CString &strLocation, bool bFromIndex)
{	
	IW::CFilePath path(strLocation);
	path.StripToFilenameAndExtension();
	path.Prefix(GetTemplatePathPrefix(bFromIndex));

	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();

	path.MakeUnixPath();
	return IW::MakeURLSafe(path);
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

bool CToolWeb::OnRequiredFile(const CString &strLocation, bool bCanBeInAuxFolder)
{
	IW::CFilePath path(_strOutputFolder);
	path += IW::Path::FindFileName(strLocation);

	if (m_bAuxFolder) path.PrefixAsPath(_strSubFolder, true);
	if (m_bLowerCase) path.MakeLower();
	if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();
	path.MakeUnixPath();

	CopyFile(strLocation, path, FALSE);

	return true;
}


bool CToolWeb::Init(bool bHasFrameHost)
{
	m_bFrameHost = bHasFrameHost;

	return true;
}

bool CToolWeb::OnFrameHost(const CString &strLocation, const CString &strPageOut, IW::IStatus *pStatus)
{
	m_bFrameHost = true;

	IW::CFilePath pathIndex(_strOutputFile);
	pathIndex.StripToFilename();	
	pathIndex.SetExtension(_strExt);
	if (m_bLowerCase) pathIndex.MakeLower();
	if (m_bRemoveIllegal) pathIndex.RemoveIllegalFromFileName();
	pathIndex.MakeUnixPath();

	IW::CFilePath path;
	MakeFilePath(path, pathIndex, g_szEmptyString);	

	IW::CFileTemp f;
	if (!f.OpenForWrite(path))
	{
		CString str;
		str.Format(IDS_FAILEDTOWRITEINDEX, path);
		pStatus->SetError(str);
		return false;
	}
	
	CStringA strPage = strPageOut;
	f.Write(strPage, strPage.GetLength());

	return f.Close(pStatus);
}

CString CToolWeb::GetBreadCrumbs(const CString &strLocation, bool bFromIndex)
{
	CString strOut;
	// Now do the Context	
	IW::CFilePath path;
	MakeIndexPath(path, -1);

	strOut = _T("<a Target='_top' href='");
	strOut += GetHome();
	strOut += _T("'>");
	strOut += App.LoadString(IDS_HOME);
	strOut += _T("</a>");

	for(int i = 0; i < m_arrayFolderNames.GetSize(); i++)
	{		
		CString strDots;

		int nDotCount = m_arrayFolderNames.GetSize();
		for(int j = nDotCount - 1; j > i; j--)
		{
			strDots += _T("../");
		}

		if (m_bAuxFolder && !bFromIndex) 
		{
			strDots += _T("../");
		}

		strOut += _T("&nbsp;&gt;&nbsp;");
		strOut += _T("<a Target='_top' href='");
		strOut += strDots;
		strOut += path;
		strOut += _T("'>");
		strOut += m_arrayFolderNames[i];
		strOut += _T("</a>");
	}

	return strOut;
}

CSize CToolWeb::ThumbnailSize(IW::FolderItemPtr pItem, CLoadAny *pLoader)
{
	CString strFile = pItem->GetFilePath();

	MAPTHUMBTOSIZE::iterator itSize = m_mapThumbToSize.find(strFile);

	if (itSize == m_mapThumbToSize.end())
	{
		IW::FolderItemLoader job(_state.Cache, _sizeThumbNail);
		if (pItem->LoadJobBegin(job))
		{
			job.LoadImage(pLoader, Search::Any, true);
			job.RenderAndScale();			
		}
		pItem->LoadJobEnd(job);	

		const CRect rcBounding = pItem->GetItemThumbRect();
		return rcBounding.Size();
	}

	return itSize->second;
}

void CToolWeb::ReplaceConstants(CString &strOut)
{
	// Disable the right click
	if (m_bDisableRightClick)
	{
		static LPCTSTR g_szDisableRightClick = 

			_T("<script language=\"JavaScript\">\n")
			_T("function disable_right_click(e)\n")
			_T("{\n")
			_T("    var browser = navigator.appName.substring ( 0, 9 );\n")
			_T("    var event_number = 0;\n")
			_T("    if (browser==\"Microsoft\")\n")
			_T("        event_number = event.button;\n")
			_T("    else if (browser==\"Netscape\")\n")
			_T("        event_number = e.which;\n")
			_T("\n")
			_T("    if ( event_number==2 || event_number==3 )\n")
			_T("        {\n")
			_T("        alert (\"{{RightMouseClickIsDisabled}}\");\n")
			_T("        return (false);\n")
			_T("        }\n")
			_T("\n")
			_T("    return (true);\n")
			_T("}\n")
			_T("\n")
			_T("function trap_images_mouse_events ()\n")
			_T("{\n")
			_T("    if ( document.images )\n")
			_T("        {\n")
			_T("        for (var pic=0; pic<document.images.length; pic++)\n")
			_T("            document.images[pic].onmousedown = disable_right_click;\n")
			_T("        }\n")
			_T("}\n")
			_T("\n")
			_T("window.onload = trap_images_mouse_events;\n")
			_T("\n")
			_T("</script>\n")
			_T("</head>\n");


		static LPCTSTR g_szDisableRightClickTag = _T("</head>");
		static LPCTSTR g_szRightMouseClickIsDisabledTag = _T("{{RightMouseClickIsDisabled}}");

		strOut.Replace(g_szDisableRightClickTag, g_szDisableRightClick);
		strOut.Replace(g_szRightMouseClickIsDisabledTag, App.LoadString(IDS_RIGHTMOUSEBUTTONISDISABLED));		
	}
}

IW::ITEMLIST CToolWeb::GetItemList(IW::Folder *pFolder, bool bCannotRecurse)
{
	IW::ITEMLIST thumbs;	

	int nSize = pFolder->GetItemCount();
	thumbs.reserve( nSize );

	for(int nItem = 0; nItem < nSize; ++nItem)
	{
		IW::FolderItemLock pItem(pFolder, nItem);
		bool bShowThisOne = true;
		bool bIsFolder = pItem->IsFolder();

		if (_settings.m_bShowImagesOnly && !pItem->IsHTMLDisplayable(_state.Plugins))
		{
			bShowThisOne = bIsFolder;
		}

		if ((bCannotRecurse || !_bRecurse) && bIsFolder) 
		{
			bShowThisOne = false;
		}

		if (_bSelected && m_nFolderDepth == 0)
		{
			bShowThisOne = bShowThisOne && pItem->IsSelected();
		}		

		if (bShowThisOne)
		{
			thumbs.push_back(pItem);
		}
	}

	pFolder->Sort(thumbs, _nSortOrder, _bAssending != 0);

	return thumbs;
}