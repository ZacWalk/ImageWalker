// PropertyArchiveRegistry.cpp: implementation of the CPropertyArchiveRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Application.h"
#include "Items.h"
#include "LoadAny.h"
#include "PropertyIPTC.h"
#include "report.h"
#include "email.h"

#include <HtmlHelp.h>
#include <dbghelp.h>


#include "PropertyEXIF.h"
#include "PropertyICC.h"
#include "PropertyIPTC.h"

#define WIN_ENV
#define TXMP_STRING_TYPE std::string
#include "XMP.hpp"
#include "XMP.incl_cpp"



static LPCTSTR g_szSlideShowToolBar = _T("SlideShowToolBar");
static LPCTSTR g_szShortDates = _T("ShortDates");
static LPCTSTR g_szWalkFolders = _T("WalkFolders");
static LPCTSTR g_szDefaultFilter = _T("DefaultFilter");
static LPCTSTR g_szToolFolderList = _T("ToolFolderList");
static LPCTSTR g_szSystemThumbs = _T("SystemThumbs");
static LPCTSTR g_szBatchOneForAll = _T("BatchOneForAll");
static LPCTSTR g_szShowDescriptions = _T("ShowDescriptions");
static LPCTSTR g_szZoomThumbnails = _T("ZoomThumbnails");
static LPCTSTR g_szImageWalkerSupportEmailAddress = _T("support@imagewalker.com");
static LPCTSTR g_szShowFlickrPicOfInterest = _T("ShowFlickrPicOfInterest");
static LPCTSTR g_szMood = _T("Mood");
static LPCTSTR g_szDescriptionPage = _T("DescriptionPage");
static LPCTSTR g_szWebOptionsPage = _T("WebOptionsPage");
static LPCTSTR g_szPrintOptionsPage = _T("PrintOptionsPage");
static LPCTSTR g_szShowFolders  = _T("ShowFolders");
static LPCTSTR g_szShowDescription  = _T("ShowDescription");
static LPCTSTR g_szShowAdvancedImageDetails = _T("ShowAdvancedImageDetails");
static LPCTSTR g_szShowAddress  = _T("ShowAddress");
static LPCTSTR g_szShowSearch  = _T("ShowSearch");
static LPCTSTR g_szShowStatusBar  = _T("ShowStatusBar");
static LPCTSTR g_szBlackBackground = _T("BlackBackground");
static LPCTSTR g_szBlackSkin = _T("BlackSkin");
static LPCTSTR g_szNSid = _T("NSid");
static LPCTSTR g_szUserName = _T("UserName");
static LPCTSTR g_szFullName = _T("FullName");
static LPCTSTR g_szToken = _T("Token");
static LPCTSTR g_szSearchByDate = _T("SearchByDate");
static LPCTSTR g_szAutoSelectTaggedImages = _T("AutoSelectTaggedImages");
static LPCTSTR g_szHeader = _T("Header");
static LPCTSTR g_szFooter = _T("Footer");
static LPCTSTR g_szWeb = _T("Web");
static LPCTSTR g_szExifAutoRotate = _T("ExifAutoRotate");

class CrashHandler
{
private:
	CCriticalSection _cs;
	LPTOP_LEVEL_EXCEPTION_FILTER _pOriginalExceptionFilter;

public  :
    CrashHandler ( void )
    {
		IW::CAutoLockCS lock(_cs);
		_pOriginalExceptionFilter = SetUnhandledExceptionFilter(ExcepCallBack);
    }
    ~CrashHandler ( void )
    {
		IW::CAutoLockCS lock(_cs);

        if ( NULL != _pOriginalExceptionFilter )
        {
				SetUnhandledExceptionFilter ( _pOriginalExceptionFilter ) ;
				_pOriginalExceptionFilter = NULL ;
        }
    }

	
	
	
	static LONG WINAPI ExcepCallBack ( EXCEPTION_POINTERS * pExceptionPointers )
	{
		CReportDlg dlg;

		if (dlg.DoModal() == IDOK)
		{
			IW::CSimpleMapi email;						

			if (email.Open())
			{
				IW::CSimpleZip zip;

				IW::CFilePath pathTemp;
				pathTemp.GetTempFilePath();

				if (zip.Create(pathTemp))
				{
					GenerateDump(zip, pExceptionPointers);
					email.AddFile(pathTemp, _T("SystemParameters.zip"));
				}

				zip.Close();

				CReport report;	

				email.Send(IW::GetMainWindow(), 
					g_szImageWalkerSupportEmailAddress, 
					_T("ImageWalker Crash Report"), 
					report.GetReportText());

				return EXCEPTION_EXECUTE_HANDLER;
			}
			else
			{
				IW::CMessageBoxIndirect mb;
				mb.Show(IDS_FAILEDTOSENDEMAIL);
			}
		}

		return EXCEPTION_CONTINUE_SEARCH;		
	}

	static void GenerateDump(IW::CSimpleZip &zip, EXCEPTION_POINTERS* pExceptionPointers)
	{
		typedef	BOOL (WINAPI * MINIDUMP_WRITE_DUMP)(
			IN HANDLE			hProcess,
			IN DWORD			ProcessId,
			IN HANDLE			hFile,
			IN MINIDUMP_TYPE	DumpType,
			IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
			IN PMINIDUMP_USER_STREAM_INFORMATION		UserStreamParam, OPTIONAL
			IN PMINIDUMP_CALLBACK_INFORMATION			CallbackParam OPTIONAL
			);

		HMODULE hDbgHelp = LoadLibrary(_T("DBGHELP.DLL"));
		MINIDUMP_WRITE_DUMP FpMiniDumpWriteDump = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

		if (FpMiniDumpWriteDump)
		{
			BOOL bMiniDumpSuccessful;
			TCHAR szPath[MAX_PATH]; 
			TCHAR szFileName[MAX_PATH]; 
			DWORD dwBufferSize = MAX_PATH;
			HANDLE hDumpFile;
			SYSTEMTIME stLocalTime;
			MINIDUMP_EXCEPTION_INFORMATION ExpParam;

			GetLocalTime( &stLocalTime );
			GetTempPath( dwBufferSize, szPath );

			_stprintf_s( szFileName, MAX_PATH, _T("%s%s"), szPath, g_szImageWalker );
			CreateDirectory( szFileName, NULL );

			_stprintf_s( szFileName, MAX_PATH, _T("%s%s\\ImageWalker-%d-%d-%d-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp"), 
				szPath, g_szImageWalker, 
				MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER,
				stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
				stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
				GetCurrentProcessId(), GetCurrentThreadId());

			hDumpFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 
				FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

			ExpParam.ThreadId = GetCurrentThreadId();
			ExpParam.ExceptionPointers = pExceptionPointers;
			ExpParam.ClientPointers = TRUE;

			bMiniDumpSuccessful = FpMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
				hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

			CloseHandle(hDumpFile);
		
			zip.AddFile(szFileName);
		}
	}


} ;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Application App;
CrashHandler Crash;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Application::Application() : 
	IsTesting(false), 
	CursorShown(true), 
	ControlKeyDown(false),
	m_nNextId(7000)
{
	
}

Application::~Application()
{
}



void Application::Init()
{
	// Log the applicatoin startup
	{
		TCHAR szBufer[100];
		_tzset();	
		_tstrtime_s(szBufer, countof(szBufer));

		CString str;
		str.Format(IDS_STARTED_AT, szBufer);
		Log(str);
	}

		
	srand(GetTickCount());		


	//jas_init(); // Jasper

	CWindowDC dc(NULL);

	IW::CRender render;
	render.Create(dc);

	const CRect rectTextIn(0, 0, 100, 100);
	const CRect rectTextOut = render.MeasureString(_T("X"), rectTextIn, IW::Style::Font::Standard, IW::Style::Text::Thumbnail); 
	m_nTextExtent = rectTextOut.Height();

	m_paletteHalftone.CreateHalftonePalette(dc);

	SXMPMeta::Initialize();
}

void Application::Free()
{
	m_setCanBeCached.clear();
	m_mapStrings.clear();
	m_mapIcons.clear();
	_stringPool.clear();

	//jas_cleanup(); // Jasper

	SXMPMeta::Terminate();

}

bool Application::CanBeCached(UINT uExtension)
{
	IW::CAutoLockCS lock(_cs);

	if (m_setCanBeCached.empty())
	{
		m_setCanBeCached.insert(GetExtensionKey(_T(".EXE")));
		m_setCanBeCached.insert(GetExtensionKey(_T(".LNK")));
		m_setCanBeCached.insert(GetExtensionKey(_T(".SCR")));
		m_setCanBeCached.insert(GetExtensionKey(_T(".DLL")));
		m_setCanBeCached.insert(GetExtensionKey(_T(".COM")));
	}

	return m_setCanBeCached.find(uExtension) == m_setCanBeCached.end();
}

int Application::GetIcon(DWORD dwExtension)
{
	IW::CAutoLockCS lock(_cs);
	MAPICONS::iterator i = m_mapIcons.find(dwExtension);

	if (i != m_mapIcons.end())
	{
		return i->second; 
	}

	return -1;
}

void Application::SetIcon(DWORD dwExtension, int nIcon)
{
	IW::CAutoLockCS lock(_cs);
	m_mapIcons[dwExtension] = nIcon;
}


HIMAGELIST Application::GetShellImageList(bool fSmall)
{
	static HIMAGELIST hilSmall = 0;
	static HIMAGELIST hilLarge = 0;

	HIMAGELIST &hilOut = fSmall ? hilSmall : hilLarge;

	if (hilOut == 0)
	{
		HWND hWnd = IW::GetMainWindow();
		USES_CONVERSION;
		SHFILEINFO  sfi;
		ZeroMemory(&sfi, sizeof(sfi));

		hilOut = (HIMAGELIST)SHGetFileInfo(_T("C:\\"), 0, &sfi,
			sizeof(SHFILEINFO), SHGFI_SYSICONINDEX |
			(fSmall ? SHGFI_SMALLICON : SHGFI_LARGEICON));

		// Do a version check first because you only need to use this code on
		// Windows NT version 4.0.
		if(IW::IsWindowsNT4())
		{
			return hilOut;
		}

		// You need to create a temporary, empty .lnk file that you can use to
		// pass to IShellIconOverlay::GetOverlayIndex. You could just enumerate
		// down from the Start Menu folder to find an existing .lnk file, but
		// there is a very remote chance that you will not find one. By creating
		// your own, you know this code will always work.
		IW::CShellFolder spsfTempDir;
		CComPtr<IShellIconOverlay> spsio;

		IW::CShellItem itemTempDir;
		IW::CShellItem itemTempFile;

		HRESULT           hr;
		TCHAR             szTempDir[MAX_PATH];
		TCHAR             szTempFile[MAX_PATH] = _T("");
		TCHAR             szFile[MAX_PATH];
		HANDLE            hFile;
		int               i;
		DWORD             dwAttributes;
		DWORD             dwEaten;
		int               nIndex;

		// Get the desktop folder.
		IW::CShellDesktop pDesktop;

		// Get the TEMP directory.
		if(!GetTempPath(MAX_PATH, szTempDir))
		{			
			//There might not be a TEMP directory. If this is the case, use the
			//Windows directory. 
			if(!GetWindowsDirectory(szTempDir, MAX_PATH))
			{
				return hilOut;
			}
		}

		// Create a temporary .lnk file.
		if(szTempDir[_tcsclen(szTempDir) - 1] != '\\')
			_tcscat_s(szTempDir, MAX_PATH, _T("\\"));

		for(i = 0, hFile = INVALID_HANDLE_VALUE;
			INVALID_HANDLE_VALUE == hFile;
			i++)
		{
			_tcscpy_s(szTempFile, MAX_PATH, szTempDir);
			_stprintf_s(szFile, MAX_PATH, _T("temp%d.lnk"), i);
			_tcscat_s(szTempFile, MAX_PATH, szFile);

			hFile = CreateFile(  szTempFile,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			// Do not try this more than 100 times.
			if(i > 100)
			{
				return hilOut;
			}
		}

		// Close the file you just created.
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		// Get the PIDL for the directory.
		hr = pDesktop->ParseDisplayName(
			hWnd,
			NULL,
			T2OLE(szTempDir),
			&dwEaten,
			itemTempDir.GetPtr(),
			&dwAttributes);

		if(SUCCEEDED(hr))
		{
			// Get the IShellFolder for the TEMP directory.
			hr = pDesktop->BindToObject(   itemTempDir,
				NULL,
				IID_IShellFolder,
				(LPVOID*)spsfTempDir.GetPtr());

			if(SUCCEEDED(hr))
			{
				// Get the IShellIconOverlay interface for this folder. If this fails,
				// it could indicate that you are running on a pre-Internet Explorer 4.0
				// shell, which doesn't support this interface. If this is the case, the
				// overlay icons are already in the system image list.
				hr = spsfTempDir->QueryInterface(IID_IShellIconOverlay, (LPVOID*)&spsio);

				if(SUCCEEDED(hr))
				{

					// Get the PIDL for the temporary .lnk file.
					hr = spsfTempDir->ParseDisplayName(  
						hWnd,
						NULL,
						T2OLE(szFile),
						&dwEaten,
						itemTempFile.GetPtr(),
						&dwAttributes);

					if(SUCCEEDED(hr))
					{
						// Get the overlay icon for the .lnk file. This causes the shell
						// to put all of the standard overlay icons into your copy of the system
						// image list.
						hr = spsio->GetOverlayIndex(itemTempFile, &nIndex);
					}
				}
			}
		}

		// Delete the temporary file.
		DeleteFile(szTempFile);
	}

	return hilOut;
}

HIMAGELIST Application::GetGlobalBitmap()
{
	static CImageList images;

	if (images == 0)
	{
		

		DWORD dwMajor = 0;
		DWORD dwMinor = 0;
		//HRESULT hRet = AtlGetCommCtrlVersion(&dwMajor, &dwMinor);
		bool bhasAlpha = true; //(SUCCEEDED(hRet) && dwMajor >= 6);

		if (bhasAlpha)
		{	
			images.Create(16, 16, ILC_COLOR32|ILC_MASK, 0, 1);

			CBitmap bmImage;
			bmImage.Attach((HBITMAP)LoadImage(GetBitmapResourceInstance(), MAKEINTRESOURCE(IDB_TB_IMAGEALPHA), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION));
			images.Add(bmImage);
		} 
		else
		{
			images.Create(16, 16, ILC_COLOR32|ILC_MASK, 0, 1);

			CBitmap bmImage;
			bmImage.LoadBitmap(IDB_TB_IMAGE);
			images.Add(bmImage, RGB(0x81, 0x81, 0x81));
		}

		// Load the global image list
		// Load the image list
		//hImageList = ImageList_LoadImage(GetBitmapResourceInstance(), 
		//	MAKEINTRESOURCE(IDB_BITMAPS), 20, 1, RGB(0xff, 0x00, 0xff), 
		//	IMAGE_BITMAP, LR_CREATEDIBSECTION);
	}

	return images;
}

HINSTANCE Application::GetBitmapResourceInstance()
{
	return _AtlBaseModule.GetModuleInstance();
}

HINSTANCE Application::GetResourceInstance()
{
	return _AtlBaseModule.GetModuleInstance();
}

CString CReport::_strLastFileLoaded;
CString CReport::_strLastOperation;
CString CReport::_strLog;

void Application::RecordFileOperation(const CString &strFileName, const CString &strOperation)
{
	IW::CAutoLockCS lock(_cs);
	
	CReport::_strLastOperation = strOperation;
	CReport::_strLastFileLoaded = strFileName;
}

void Application::Log(const CString &str)
{
	IW::CAutoLockCS lock(_cs);

	CReport::_strLog += str;
	CReport::_strLog += g_szCRLF;

	ATLTRACE(str);
	ATLTRACE(_T("\n"));
}

void Application::BugReport()
{
	IW::CSimpleMapi email;

	if (email.Open())
	{
		CReport report;		

		email.Send(IW::GetMainWindow(), 
			g_szImageWalkerSupportEmailAddress, 
			_T("ImageWalker Bug Report"), 
			report.GetReportText());
	}
	else
	{
		IW::CMessageBoxIndirect mb;
		mb.Show(IDS_FAILEDTOSENDEMAIL);
	}
}

///////////////////////////////////////////////////////////////////////
/// Languages 

static LCID g_lLangId = MAKELCID(LANG_NEUTRAL, SORT_DEFAULT);

LCID Application::GetLangId()
{
	return g_lLangId;	
}

void Application::SetLangId(LCID l)
{
	g_lLangId = l; 
}



// Helper to invoke help
void Application::InvokeHelp(HWND hwnd, UINT nId)
{
	IW::CFilePath path;
	path.GetModuleFileName(NULL);
	path.SetFileNameAndExtension(_T("ImageWalker"), _T("chm"));

	if (nId == 0)
	{		
		HtmlHelp(hwnd, path, HH_DISPLAY_TOPIC, 0);
	}
	else
	{		
		HtmlHelp(hwnd, path, HH_HELP_CONTEXT, nId);
	}
	
	return;
}




COptions::COptions()
{
	m_nViewOptionsPage = 0;
	m_nDescriptionPage = 0;
	m_nWebOptionsPage = 0;
	m_nPrintOptionsPage = 0;
	_nRegistrationSettings = 0;
	_sizeRowsColumns.cx = 3;
	_sizeRowsColumns.cy = 3;
	_strCaptureSaveType = g_szJPG;
	m_annotations.Add(IW::ePropertyName);
	m_columns.Add(IW::ePropertyName);
	m_columns.Add(IW::ePropertyType);
	m_columns.Add(IW::ePropertySize);
	m_columns.Add(IW::ePropertyModifiedDate);
	_sizeThumbImage.cx = IW::IMAGE_X;
	_sizeThumbImage.cy = IW::IMAGE_Y;
	m_bUseEffects = true;
	m_bDontUseBackBuffer = false;
	m_bDontUseHalfTone = false;
	m_bUseMMX = true;
	m_bBatchOneForAll = true;
	_bExifAutoRotate = true;
	m_bDoubleClickShowsFullScreen = true;
	m_bAutoSave = false;
	m_bShowToolTips = true;
	m_bShowMarkers = true;
	m_bShortDates = true;
	m_bWalkFolders = true;
	m_bSystemThumbs = false;
	m_bRepeat = true;
	m_bShuffle = false;
	m_bRecursSubFolders = true;
	m_bShowInformation = true;
	_bSlideShowToolBar = true;
	_bDontHideCursor = false;
	ShowDescriptions = true;
	ZoomThumbnails = true;	
	m_nDelay = 5;
	Mood = 0;	
	m_nResolutionSelection = 0;
	m_dwXPelsPerMeter = 2834;
	m_dwYPelsPerMeter = 2834;
	ShowFlickrPicOfInterest = true;
	ShowFolders = false;
	ShowDescription = true;
	ShowAdvancedImageDetails = true;
	ShowAddress = true;
	ShowSearch = false;
	BlackBackground = true;
	BlackSkin = !IW::IsWindowsVista();
	SearchByDate = false;
	AutoSelectTaggedImages = true;
	ShowStatusBar = true;	
	Web.Header.LoadString(IDS_HTML_DEFAULT_HEADER);
	Web.Footer.LoadString(IDS_HTML_DEFAULT_FOOTER);	
};



CString Application::GetRegKey()
{
	return g_szImageWalkerRegKey231;
}



void COptions::Read(const CString &strValueName, const IW::IPropertyArchive *pArchive)
{
	if (pArchive->StartSection(strValueName))
	{
		pArchive->Read(g_szPage, m_nViewOptionsPage);
		pArchive->Read(g_szDescriptionPage, m_nDescriptionPage);
		pArchive->Read(g_szWebOptionsPage, m_nWebOptionsPage);
		pArchive->Read(g_szPrintOptionsPage, m_nPrintOptionsPage);
		pArchive->Read(g_szColumns, _sizeRowsColumns.cx);
		pArchive->Read(g_szRows, _sizeRowsColumns.cy);
		pArchive->Read(g_szThumbCX, _sizeThumbImage.cx);
		pArchive->Read(g_szThumbCY, _sizeThumbImage.cy);
		pArchive->Read(g_szUseEffects, m_bUseEffects);		
		pArchive->Read(g_szSlideShowToolBar, _bSlideShowToolBar);				
		pArchive->Read(g_szDontUseBackBuffer, m_bDontUseBackBuffer);
		pArchive->Read(g_szDontUseHalfTone, m_bDontUseHalfTone);
		pArchive->Read(g_szMMX, m_bUseMMX);		
		pArchive->Read(g_szBatchOneForAll, m_bBatchOneForAll);
		pArchive->Read(g_szShowDescriptions, ShowDescriptions);
		pArchive->Read(g_szZoomThumbnails, ZoomThumbnails);
		pArchive->Read(g_szExifAutoRotate, _bExifAutoRotate);		
		pArchive->Read(g_szDoubleClickShowsFullScreen, m_bDoubleClickShowsFullScreen);
		pArchive->Read(g_szToolTips, m_bShowToolTips);
		pArchive->Read(g_szHidden, m_bShowHidden);
		pArchive->Read(g_szMarkers, m_bShowMarkers);
		pArchive->Read(g_szAutoSave, m_bAutoSave);
		pArchive->Read(g_szShortDates, m_bShortDates);
		pArchive->Read(g_szWalkFolders, m_bWalkFolders);
		pArchive->Read(g_szSystemThumbs, m_bSystemThumbs);			
		pArchive->Read(g_szCaptureSaveType, _strCaptureSaveType);
		pArchive->Read(g_szRegistrationSettings, _nRegistrationSettings);

		CString str;
		pArchive->Read(g_szViewAnnotations, str);
		m_annotations.ParseFromString(str);
		pArchive->Read(g_szViewColumns, str);
		m_columns.ParseFromString(str);

		pArchive->Read(g_szRepeat, m_bRepeat);
		pArchive->Read(g_szShuffle, m_bShuffle);
		pArchive->Read(g_szRecursSubFolders, m_bRecursSubFolders);
		pArchive->Read(g_szShowInformation, m_bShowInformation);
		pArchive->Read(g_szDelay, m_nDelay);
		pArchive->Read(g_szMood, Mood);
		pArchive->Read(g_szResolution, m_nResolutionSelection);
		pArchive->Read(g_szXPelsPerMeter, m_dwXPelsPerMeter);
		pArchive->Read(g_szYPelsPerMeter, m_dwYPelsPerMeter);
		pArchive->Read(g_szShowFlickrPicOfInterest, ShowFlickrPicOfInterest);
		pArchive->Read(g_szShowFolders, ShowFolders);
		pArchive->Read(g_szShowDescription, ShowDescription);
		pArchive->Read(g_szShowAdvancedImageDetails, ShowAdvancedImageDetails);
		pArchive->Read(g_szShowAddress, ShowAddress);
		pArchive->Read(g_szShowSearch, ShowSearch);
		pArchive->Read(g_szShowStatusBar, ShowStatusBar);		
		pArchive->Read(g_szBlackBackground, BlackBackground);		
		pArchive->Read(g_szBlackSkin, BlackSkin);	
		pArchive->Read(g_szSearchByDate, SearchByDate);		
		pArchive->Read(g_szAutoSelectTaggedImages, AutoSelectTaggedImages);	

		if (pArchive->StartSection(g_szFlickr))
		{
			pArchive->Read(g_szNSid, Flickr.NSid);
			pArchive->Read(g_szUserName, Flickr.UserName);
			pArchive->Read(g_szFullName, Flickr.FullName);
			pArchive->Read(g_szToken, Flickr.Token);

			pArchive->EndSection();
		}

		if (pArchive->StartSection(g_szWeb))
		{
			pArchive->Read(g_szHeader, Web.Header);
			pArchive->Read(g_szFooter, Web.Footer);

			pArchive->EndSection();
		}

		pArchive->EndSection();
	}
}

void COptions::Write(const CString &strValueName, IW::IPropertyArchive *pArchive) const
{
	if (pArchive->StartSection(strValueName))
	{
		
		pArchive->Write(g_szPage, m_nViewOptionsPage);
		pArchive->Write(g_szDescriptionPage, m_nDescriptionPage);
		pArchive->Write(g_szWebOptionsPage, m_nWebOptionsPage);
		pArchive->Write(g_szPrintOptionsPage, m_nPrintOptionsPage);
		pArchive->Write(g_szColumns, _sizeRowsColumns.cx);
		pArchive->Write(g_szRows, _sizeRowsColumns.cy);		
		pArchive->Write(g_szThumbCX, _sizeThumbImage.cx);
		pArchive->Write(g_szThumbCY, _sizeThumbImage.cy);
		pArchive->Write(g_szUseEffects, m_bUseEffects);
		pArchive->Write(g_szSlideShowToolBar, _bSlideShowToolBar);		
		pArchive->Write(g_szDontUseBackBuffer, m_bDontUseBackBuffer);
		pArchive->Write(g_szDontUseHalfTone, m_bDontUseHalfTone);
		pArchive->Write(g_szMMX, m_bUseMMX);
		pArchive->Write(g_szBatchOneForAll, m_bBatchOneForAll);
		pArchive->Write(g_szShowDescriptions, ShowDescriptions);
		pArchive->Write(g_szZoomThumbnails, ZoomThumbnails);
		pArchive->Write(g_szExifAutoRotate, _bExifAutoRotate);
		pArchive->Write(g_szDoubleClickShowsFullScreen, m_bDoubleClickShowsFullScreen);
		pArchive->Write(g_szToolTips, m_bShowToolTips);
		pArchive->Write(g_szHidden, m_bShowHidden);
		pArchive->Write(g_szMarkers, m_bShowMarkers);
		pArchive->Write(g_szAutoSave, m_bAutoSave);
		pArchive->Write(g_szShortDates, m_bShortDates);
		pArchive->Write(g_szWalkFolders, m_bWalkFolders);
		pArchive->Write(g_szSystemThumbs, m_bSystemThumbs);		
		pArchive->Write(g_szCaptureSaveType, _strCaptureSaveType);
		pArchive->Write(g_szViewAnnotations, m_annotations.GetAsString());
		pArchive->Write(g_szViewColumns, m_columns.GetAsString());
		pArchive->Write(g_szRepeat, m_bRepeat);
		pArchive->Write(g_szShuffle, m_bShuffle);
		pArchive->Write(g_szRecursSubFolders, m_bRecursSubFolders);
		pArchive->Write(g_szShowInformation, m_bShowInformation);
		pArchive->Write(g_szDelay, m_nDelay);
		pArchive->Write(g_szMood, Mood);
		pArchive->Write(g_szResolution, m_nResolutionSelection);
		pArchive->Write(g_szXPelsPerMeter, m_dwXPelsPerMeter);
		pArchive->Write(g_szYPelsPerMeter, m_dwYPelsPerMeter);
		pArchive->Write(g_szShowFlickrPicOfInterest, ShowFlickrPicOfInterest);
		pArchive->Write(g_szShowFolders, ShowFolders);
		pArchive->Write(g_szShowDescription, ShowDescription);
		pArchive->Write(g_szShowAdvancedImageDetails, ShowAdvancedImageDetails);
		pArchive->Write(g_szShowAddress, ShowAddress);
		pArchive->Write(g_szShowSearch, ShowSearch);
		pArchive->Write(g_szShowStatusBar, ShowStatusBar);
		pArchive->Write(g_szBlackBackground, BlackBackground);
		pArchive->Write(g_szBlackSkin, BlackSkin);	
		pArchive->Write(g_szSearchByDate, SearchByDate);		
		pArchive->Write(g_szAutoSelectTaggedImages, AutoSelectTaggedImages);
		pArchive->Write(g_szRegistrationSettings, _nRegistrationSettings);
		
		if (pArchive->StartSection(g_szFlickr))
		{
			pArchive->Write(g_szNSid, Flickr.NSid);
			pArchive->Write(g_szUserName, Flickr.UserName);
			pArchive->Write(g_szFullName, Flickr.FullName);
			pArchive->Write(g_szToken, Flickr.Token);

			pArchive->EndSection();
		}

		if (pArchive->StartSection(g_szWeb))
		{
			pArchive->Write(g_szHeader, Web.Header);
			pArchive->Write(g_szFooter, Web.Footer);

			pArchive->EndSection();
		}

		pArchive->EndSection();
	}
}

bool PluginState::RegisterImageLoaderHeaderWord(WORD w, const CString &strTitle)
{
	IW::IImageLoaderFactoryPtr pFactory = GetImageLoaderFactory(strTitle);
	m_LoaderFactoryHeaderMap[w] = pFactory;

	return true;
}

bool PluginState::RegisterImageLoader(IW::IImageLoaderFactoryPtr pFactory)
{
	CString strKey = pFactory->GetKey();
	strKey.MakeLower();

	// Is it already registered?
	assert(m_LoaderFactoryNameMap.find(strKey) == m_LoaderFactoryNameMap.end());

	m_LoaderFactoryNameMap[strKey] = pFactory;


	CString str(pFactory->GetExtensionList());
	int curPos = 0;

	CString token = str.Tokenize(_T(","), curPos);
	while (token != "")
	{
		token.MakeLower();
		m_LoaderFactoryExtensionMap[token] = pFactory;
		token= str.Tokenize(_T(","), curPos);
	};

	return true;
}

IW::IImageLoaderFactoryPtr PluginState::GetImageLoaderFactory(CString strKey) const
{
	strKey.MakeLower();

	LOADERFACTORYSTRINGMAP::const_iterator itTitle = m_LoaderFactoryNameMap.find(strKey);
	
	if (m_LoaderFactoryNameMap.end() != itTitle)
	{
		return itTitle->second;
	}


	// If is has a dot try without
	if (strKey[0] == '.')
	{
		strKey.Delete(0);
	}
	
	LOADERFACTORYSTRINGMAP::const_iterator itExt =  m_LoaderFactoryExtensionMap.find(strKey);
	
	if (m_LoaderFactoryExtensionMap.end() != itExt)
	{
		return itExt->second;
	}
	
	return 0;
}

IW::IImageLoaderFactoryPtr PluginState::GetImageLoaderFactory(WORD w) const
{
	LOADERFACTORYHEADERMAP::const_iterator it = m_LoaderFactoryHeaderMap.find(w);
	
	if (m_LoaderFactoryHeaderMap.end() != it)
	{
		return it->second;
	}

	return 0;
}

LPCTSTR Application::LoadString(UINT dwId)
{
	MAPSTRINGS::iterator it = m_mapStrings.find(dwId);

	if (it != m_mapStrings.end())
	{
		return it->second;
	}

	// Load it new
	LPCTSTR sz = _stringPool.LoadString(GetResourceInstance(), dwId);
	return m_mapStrings[dwId] = sz;
}

template<class T>
void IterateMetaDataTypesT(T *pFunctor)
{
	pFunctor->AddMetaDataType(IW::ePropertyName, App.LoadString(IDS_FILENAME));
	pFunctor->AddMetaDataType(IW::ePropertyType, App.LoadString(IDS_FILETYPE));
	pFunctor->AddMetaDataType(IW::ePropertySize, App.LoadString(IDS_FILESIZE));
	pFunctor->AddMetaDataType(IW::ePropertyModifiedDate, App.LoadString(IDS_FILEMODIFIED));
	pFunctor->AddMetaDataType(IW::ePropertyCreatedDate, App.LoadString(IDS_FILECREATED));
	pFunctor->AddMetaDataType(IW::ePropertyPath, App.LoadString(IDS_FILEPATH));
	pFunctor->AddMetaDataType(IW::ePropertyModifiedTime, App.LoadString(IDS_FILEMODIFIED_TIME));
	pFunctor->AddMetaDataType(IW::ePropertyCreatedTime, App.LoadString(IDS_FILECREATED_TIME));
	pFunctor->AddMetaDataType(IW::ePropertyWidth, App.LoadString(IDS_WIDTH));
	pFunctor->AddMetaDataType(IW::ePropertyHeight, App.LoadString(IDS_HEIGHT));
	pFunctor->AddMetaDataType(IW::ePropertyDepth, App.LoadString(IDS_DEPTH));
	pFunctor->AddMetaDataType(IW::ePropertyTitle, App.LoadString(IDS_TITLE));
	pFunctor->AddMetaDataType(IW::ePropertyObjectName, "Object Name");
	pFunctor->AddMetaDataType(IW::ePropertyAperture, _T("Aperture"));
	pFunctor->AddMetaDataType(IW::ePropertyIsoSpeed, _T("Iso Speed"));
	pFunctor->AddMetaDataType(IW::ePropertyWhiteBalance, _T("White Balance"));
	pFunctor->AddMetaDataType(IW::ePropertyExposureTime, _T("Exposure Time"));
	pFunctor->AddMetaDataType(IW::ePropertyFocalLength, _T("Focal Length"));
	pFunctor->AddMetaDataType(IW::ePropertyDateTaken, _T("Date Taken"));
	pFunctor->AddMetaDataType(IW::ePropertyDescription, _T("Description"));
}

template<class TMap>
class Functor
{
public:

	TMap &m_map;

	Functor(TMap &map) : m_map(map)
	{
	}

	void AddMetaDataType(DWORD dwId, const CString &strTitle)
	{
		m_map[dwId] = strTitle;
	}
};


bool Application::IterateMetaDataTypes(IW::IImageMetaDataClient *pFunctor)
{
	IterateMetaDataTypesT(pFunctor);
	return true;
}

CString Application::GetMetaDataTitle(DWORD dw)
{
	if (m_MetaDataPropertyMap.size() == 0)
	{
		Functor<METADATAPROPERTYMAP> f(m_MetaDataPropertyMap);
		IterateMetaDataTypesT(&f);
	}
	
	return m_MetaDataPropertyMap[dw];
}

CString Application::GetMetaDataShortTitle(DWORD dw)
{
	if (m_shortMetaDataPropertyMap.size() == 0)
	{
		m_shortMetaDataPropertyMap[IW::ePropertyName] = _T("Name");
		m_shortMetaDataPropertyMap[IW::ePropertyType] = _T("Type");
		m_shortMetaDataPropertyMap[IW::ePropertySize] = _T("Size");
		m_shortMetaDataPropertyMap[IW::ePropertyModifiedDate] = _T("Modified");
		m_shortMetaDataPropertyMap[IW::ePropertyCreatedDate] = _T("Created");
		m_shortMetaDataPropertyMap[IW::ePropertyPath] = _T("Path");
		m_shortMetaDataPropertyMap[IW::ePropertyModifiedTime] = _T("Modified");
		m_shortMetaDataPropertyMap[IW::ePropertyCreatedTime] = _T("Created");
		m_shortMetaDataPropertyMap[IW::ePropertyWidth] = _T("Width");
		m_shortMetaDataPropertyMap[IW::ePropertyHeight] = _T("Height");
		m_shortMetaDataPropertyMap[IW::ePropertyDepth] = _T("Depth");
		m_shortMetaDataPropertyMap[IW::ePropertyTitle] = _T("Title");
		m_shortMetaDataPropertyMap[IW::ePropertyObjectName] = _T("Object Name");
		m_shortMetaDataPropertyMap[IW::ePropertyAperture] = _T("Aperture");
		m_shortMetaDataPropertyMap[IW::ePropertyIsoSpeed] = _T("Iso");
		m_shortMetaDataPropertyMap[IW::ePropertyWhiteBalance] = _T("White Bal");
		m_shortMetaDataPropertyMap[IW::ePropertyExposureTime] = _T("Exposure");
		m_shortMetaDataPropertyMap[IW::ePropertyFocalLength] = _T("Focal Len");
		m_shortMetaDataPropertyMap[IW::ePropertyDateTaken] = _T("Taken");
		m_shortMetaDataPropertyMap[IW::ePropertyDescription] = _T("Description");
	}
	
	return m_shortMetaDataPropertyMap[dw];
}

CString Application::GetIWSFilter()
{
	static CString str;

	if (str.IsEmpty())
	{
		CString strAllFiles, strSettingFiles;

		strAllFiles.LoadString(IDS_ALL_FILES); 
		strSettingFiles.LoadString(IDS_SETTING_FILES);

		str.Format(_T("%s (*.iws)|*.iws|%s (*.*)|*.*||"), strSettingFiles, strAllFiles);
		str.Replace('|', 0);
	}

	return str;
}




//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// Memory and properties









