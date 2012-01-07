// Report.h: interface for the CReport class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CReport  
{
private:
	CString _strReport;

public:

	static CString _strLastFileLoaded;
	static CString _strLastOperation;
	static CString _strLog;

	CReport()
	{
		CString str;
		str.Format(_T("--- ImageWalker v%d.%d.%d"), MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
		WriteParagraph(str);
		WriteDateTime();

		WriteParagraph( _T("To raise a bug report email this report to ImageWalker support, Support@ImageWalker.com. When raising a bug please include details of what you were doing when the problem occurred."));

		// May be caused by a file??
		if (_strLastOperation)
		{
			WriteParagraph( _T("Often a non-standard or corrupt image file causes a bug. The following file may have caused the problem and should be included with the bug report.\n"));

			WriteProperty(_T("Last File"), _strLastFileLoaded);
			WriteProperty(_T("Last Operation"), _strLastOperation);
		}

		WriteParagraph(_T("--- System Information\n"));

		WriteOperatingSystemVersion();
		WriteMemoryUsage();
		WriteLastSystemError();
		WriteShellandCommonControlsVersions();

		WriteParagraph( _T("--- ImageWalker Start-Up Log\n"));
		WriteParagraph( _strLog );
		WriteParagraph( _T("--- Report End"));
	}

	~CReport()
	{
	}
	

	void Write(const CString &str)
	{
		_strReport += str;
	}

	void WriteProperty(const CString &strName, const CString &strValue)
	{
		Write(strName);
		Write(_T(":\t"));
		Write(strValue);
		Write(_T("\n"));
	}

	void WriteParagraph(const CString &str)
	{
		Write(_T("\n"));
		Write(str);
		Write(_T("\n"));
	}

	void WriteDateTime()
	{
		IW::FileTime now = IW::FileTime::Now().ToLocalTime();

		WriteProperty(_T("Report Date"), now.GetDateFormat() + _T(" ") + now.GetTimeFormat());
		WriteProperty(_T("Build Date"), _T( __DATE__ ));
	}

	
	void WriteOperatingSystemVersion ()
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		IW::MemZero(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		CString strVersion;

		if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return;
		}

		switch (osvi.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_NT:


			if ( osvi.dwMajorVersion <= 4 )
				strVersion = _T("Microsoft Windows NT ");

			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
				strVersion = _T("Microsoft Windows 2000 ");

			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
				strVersion = _T("Microsoft Windows XP ");

			if( bOsVersionInfoEx )
			{
				/* TODO
				if ( osvi.wProductType == VER_NT_WORKSTATION )
				{
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
				Write( "Personal " );
				else
				Write( "Professional " );
				}


				else*/ if ( osvi.wProductType == VER_NT_SERVER )
				{
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						strVersion += _T(" DataCenter Server ");
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						strVersion += _T(" Advanced Server ");
					else
						strVersion += _T(" Server ");
				}
			}
			else
			{
				HKEY hKey;
				TCHAR szProductType[80];
				DWORD dwBufLen;

				RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"), 0, KEY_QUERY_VALUE, &hKey );
				RegQueryValueEx( hKey, _T("ProductType"), NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
				RegCloseKey( hKey );

				strVersion += CString(_T(" ")) + szProductType;
			}

			// Display version, service pack (if any), and build number.

			if ( osvi.dwMajorVersion <= 4 )
			{
				CString str;

				str.Format(_T("version %d.%d %s (Build %d)\n"),
					osvi.dwMajorVersion,
					osvi.dwMinorVersion,
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);

				strVersion += _T(" ") + str;
			}
			else
			{ 
				CString str;
				str.Format(_T("%s (Build %d)\n"),
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);

				strVersion += _T(" ") + str;				
			}
			break;

		case VER_PLATFORM_WIN32_WINDOWS:

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			{
				strVersion = _T("Microsoft Windows 95 ");
				if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
					strVersion += _T(" OSR2 " );
			} 
			else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			{
				strVersion = _T("Microsoft Windows 98 ");
				if ( osvi.szCSDVersion[1] == 'A' )
					strVersion += _T(" SE ");
			}
			else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			{
				strVersion = _T("Microsoft Windows Me ");
			} 
			break;

		case VER_PLATFORM_WIN32s:

			strVersion = _T("Microsoft Win32s ");
			break;
		}

		WriteProperty(_T("Operating system"), strVersion);
	}


	
	static long PACKVERSION(int major, int minor) { return MAKELONG(minor,major); };


	DWORD GetDllVersion(LPCTSTR lpszDllName)
	{
		typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

		HINSTANCE hinstDll;
		DWORD dwVersion = 0;

		hinstDll = LoadLibrary(lpszDllName);

		if(hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion;

			pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

			// Because some DLLs may not implement this function, you
			// must test for it explicitly. Depending on the particular 
			// DLL, the lack of a DllGetVersion function may
			// be a useful indicator of the version.
			if(pDllGetVersion)
			{
				DLLVERSIONINFO dvi;
				HRESULT hr;

				IW::MemZero(&dvi, sizeof(dvi));
				dvi.cbSize = sizeof(dvi);

				hr = (*pDllGetVersion)(&dvi);

				if(SUCCEEDED(hr))
				{
					dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
				}
			}

			FreeLibrary(hinstDll);
		}
		return dwVersion;
	}

	void WriteShellandCommonControlsVersions()
	{
		CString str;

		DWORD dwVersion = 0;
		LPCTSTR szDll[] = { _T("Comctl32.dll"), 
			_T("Shell32.dll"), 
			_T("Shlwapi.dll"), 
			0 };

		WriteParagraph(_T("ImageWalker is compatible with Comctl32.dll version 5.80 and later, and Shell32.dll and Shlwapi.dll version 5.0 and later. Your system has the following DLL versions installed:\n"));

		for(int i = 0; szDll[i] != 0; i++)
		{
			dwVersion = GetDllVersion(szDll[i]);
			str.Format(_T("%d.%d"), HIWORD(dwVersion), LOWORD(dwVersion) );
			WriteProperty(szDll[i], str);
		}
	}

	void WriteMemoryProperty(const CString &strName, DWORD dwTotal, DWORD dwAvail)
	{
		CString str;
		str.Format(_T("%d total (%d free) kb"), dwTotal/1024, dwAvail/1024);
		WriteProperty(strName, str);
	}

	void WriteMemoryUsage()
	{
		static MEMORYSTATUS stat;
		GlobalMemoryStatus (&stat);

		CString str;
		str.Format(_T("%ld percent."), stat.dwMemoryLoad);
		WriteProperty(_T("Memory In Use"), str);

		WriteMemoryProperty(_T("Physical Memory"), stat.dwTotalPhys, stat.dwAvailPhys);
		WriteMemoryProperty(_T("Virtual Memory"), stat.dwTotalVirtual, stat.dwAvailVirtual);
		WriteMemoryProperty(_T("Paging File"), stat.dwTotalPageFile, stat.dwAvailPageFile);
	}

	void WriteLastSystemError()
	{
		int nError = GetLastError();
		WriteProperty(_T("Last System Error Number"), IW::IToStr(nError));

		if (nError != 0)
		{
			LPVOID lpMsgBuf = 0;

			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				nError,
				App.GetLangId(), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);

			if (lpMsgBuf)
			{
				WriteProperty(_T("Last System Error Message"), (LPCTSTR)lpMsgBuf);
				LocalFree( lpMsgBuf );
			}
		}
	}


	CString GetReportText() const
	{
		return _strReport;
	}


};


class CReportDlg : public CDialogImpl<CReportDlg>
{
public:
	enum { IDD = IDD_BUG_REPORT };
	CHyperLink	m_link;

	BEGIN_MSG_MAP(CReportDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		
		// IDC_WEB_PAGE
		m_link.SetHyperLink(_T("www.ImageWalker.com"));
		m_link.SetLabel(_T("www.ImageWalker.com"));
		m_link.SubclassWindow(GetDlgItem(IDC_WEB_PAGE));		

		return (LRESULT)TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};

