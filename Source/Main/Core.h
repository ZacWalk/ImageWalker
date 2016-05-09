///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


#pragma once

#include "Base.h"
#include "Imaging.h"
#include "FileTime.h"

// Pre defs
class _com_error;

struct _IMAGELIST;
typedef struct _IMAGELIST* HIMAGELIST;
typedef DWORD DROPEFFECT;

class CMainFrame;
class State;

extern State *g_pState;
extern CMainFrame *g_pMainWin;


namespace IW
{

	class Image;
	class CRender;

	IWINTERFACECLASS IImageLoaderFactory;
	IWINTERFACECLASS IImageLoaderFactoryIterator;


#define ID_PREVIEW_TOOLBAR 1025

#define ID_IMAGE_SCROLL 120
#define ID_FOLDER_VIEW 122
#define ID_SEARCH_VIEW 123
#define ID_FOLDER_VIEWBAR 124
#define ID_FOLDER_SCROLL 125
#define ID_FOLDER_HEADER 126
#define ID_ADDRESS 127
#define	IDC_RENAME 128

#define WM_CONTEXTSWITCH (WM_USER + 209)
#define WM_THUMBNAILING (WM_USER + 210)
#define WM_THUMBNAILING_COMPLETE (WM_USER + 211)
#define WM_SEARCHING (WM_USER + 212)
#define WM_SEARCHING_COMPLETE (WM_USER + 213)
#define WM_SEARCHING_FOLDER (WM_USER + 214)
#define WM_ADJUST_HEADER (WM_USER + 215)

#define WM_START (WM_USER + 301)
#define WM_END (WM_USER + 302)

#define WM_GETFILENAME (WM_USER + 401)
#define WM_LOADCOMPLETE (WM_USER + 402)
#define WM_SCALECOMPLETE (WM_USER + 403)


#define IW_WS_CHILD (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

	bool SimpleMatch(const CString &strFilter, const CString &strFileNameOrg);

	// Windows Version
	bool IsWindows95();
	bool IsWindows98();
	bool IsWindowsME();
	bool IsWindowsXP(); 
	bool IsWindowsVista(); 
	bool IsWindows2000();
	bool IsWindowsNT4();
	bool IsWindowsNT();
	bool IsWindowsXPOrBetter();	
	bool IsWindowsAscii();

	// Render
	bool HasMMX();

	CWindow GetMainWindow();

	inline int Clamp(const int v, const int l, const int h)	
	{
		return v < l ? l : (v > h ? h : v);
	}

	inline CRect ClampRect(const CRect &rectIn, const CRect &rectLimit)
	{
		CSize offset(0,0);
		CRect rectOut = rectIn;

		if (rectOut.top < rectLimit.top)  
			offset.cy = rectLimit.top - rectOut.top;

		if (rectOut.left < rectLimit.left)  
			offset.cx = rectLimit.left - rectOut.left;

		if (rectOut.bottom > rectLimit.bottom)  
			offset.cy = rectLimit.bottom - rectOut.bottom;

		if (rectOut.right > rectLimit.right)  
			offset.cx = rectLimit.right - rectOut.right;

		rectOut.OffsetRect(offset);

		return rectOut;
	}

	inline CRect ClipRect(CRect &rectIn, const CRect &rectLimit)
	{
		CRect rectOut(rectIn);

		if (rectIn.top < rectLimit.top)  
			rectOut.top = rectLimit.top;

		if (rectIn.left < rectLimit.left)  
			rectOut.left = rectLimit.left;

		if (rectIn.bottom > rectLimit.bottom)  
			rectOut.bottom = rectLimit.bottom;

		if (rectIn.right > rectLimit.right)  
			rectOut.right = rectLimit.right;

		return rectOut;
	}

	inline CRect MulDivRect(const CRect &rect, const CSize &sizeNumerator, const CSize &sizeDenominator)
	{
		return CRect(
			MulDiv(rect.left, sizeNumerator.cx, sizeDenominator.cx),
			MulDiv(rect.top, sizeNumerator.cy, sizeDenominator.cy),
			MulDiv(rect.right, sizeNumerator.cx, sizeDenominator.cx),
			MulDiv(rect.bottom, sizeNumerator.cy, sizeDenominator.cy));
	}

	// Delete helpers
	struct delete_object
	{
		template <typename T>
		void operator()(T *ptr){ delete ptr;}
	};

	struct delete_map_object
	{
		template <typename T>
		void operator()(T &it){ delete it.second;}
	};

	// Stl Helpers
	template<typename InputIterator, typename OutputIterator, typename Predicate>
	OutputIterator copy_if(InputIterator begin, InputIterator end, OutputIterator destBegin, Predicate p)
	{
		while (begin != end){
			if(p(*begin)) *destBegin++ = *begin;
			++begin;
		}
		return destBegin;
	}

	template<typename InputIterator, typename OutputIterator, typename Predicate>
	OutputIterator copy_ref_if(InputIterator begin, InputIterator end, OutputIterator destBegin, Predicate p)
	{
		while (begin != end){
			if(p(*begin)) *destBegin++ = **begin;
			++begin;
		}
		return destBegin;
	}

	template<typename InputIterator, typename Predicate, typename UnaryFunction>
	UnaryFunction for_each_if(InputIterator first, InputIterator last, Predicate pred, UnaryFunction f)
	{
		for(;first != last; ++first)
		{
			if (pred(*first))
			{
				f(*first);
			}
		}
		return f;
	}

	struct ConstStringComparePred : std::binary_function<LPCTSTR, LPCTSTR, bool> 
	{
		bool operator()(const LPCTSTR& s1, const LPCTSTR& s2) const
		{
			return (_tcscmp(s1, s2) < 0); 
		}
	};

	struct ConstWideStringComparePred : std::binary_function<LPCWSTR, LPCWSTR, bool> 
	{
		bool operator()(const LPCWSTR& s1, const LPCWSTR& s2) const
		{
			return (wcscmp(s1, s2) < 0); 
		}
	};


	struct LessThanString
	{
		bool operator()(LPCTSTR s1, LPCTSTR s2) const
		{
			return _tcscmp(s1, s2) < 0;
		}
	};

	inline bool IsNullOrEmpty(LPCTSTR sz)
	{
		return sz == 0 || sz[0] == 0;
	}

	inline bool IsSeparator(TCHAR ch)
	{
		return ch == _T(';') ||
			ch == _T(',');
	}	

	/////////////////////////////////////////////////////////////////////////
	/// Strings

	CString TextToHtmlFriendly(const CString &strIn, int nLength = -1);
	CString HtmlFriendlyToText(const CString &strIn, int nLength = -1);
	CString MakeURLSafe(const CString &strIn);
	CString MakeURLUnSafe(const CString &strIn);

	/////////////////////////////////////////////////////////////////////////
	/// Exception

	class invalid_file : public std::exception
	{	// base of all bad allocation exceptions
	public:
		invalid_file(const char *_Message = "Invalid File") throw ()
			: exception(_Message)
		{	// construct from message string
		}

		virtual ~invalid_file() throw ()
		{	// destroy the object
		}
	};

	class startup_exception : public std::exception
	{	// base of all bad allocation exceptions
	public:
		startup_exception(const char *_Message = "Failed to start up") throw ()
			: exception(_Message)
		{	// construct from message string
		}

		virtual ~startup_exception() throw ()
		{	// destroy the object
		}
	};

	// IsEdit: a helper function to determine if a given CWnd pointer
	// points to a CEDit control.
	// Use the SDK ::GetClassName() 

	inline bool IsEdit( HWND hWnd )
	{
		if (hWnd == NULL)
			return FALSE;

		TCHAR szClassName[6];
		return ::GetClassName(hWnd, szClassName, 6) &&
			_tcsicmp(szClassName, _T("Edit")) == 0;
	}

	static bool IsActive(HWND hWndParent, HWND hWnd) 
	{
		return hWndParent == hWnd || ::IsChild(hWndParent, hWnd);
	};

	static bool HasVisibleStyle(HWND hWnd)
	{
		return hWnd != NULL &&
			((DWORD)::GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE);
	}

	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable( const NonCopyable & );
		const NonCopyable & operator = ( const NonCopyable & ); 
	};

	class Focus
	{
	protected:
		HWND _hFocus;
		HWND _hCapture;
	public:
		Focus()
		{
			_hFocus = ::GetFocus();
			_hCapture = ::GetCapture();
			if (_hCapture) ::ReleaseCapture();
		}

		~Focus()
		{
			if (_hCapture) ::SetCapture(_hCapture);
			::SetFocus(_hFocus);			
		}
	};

	class ListViewItem : public LVITEM
	{
	public:
		ListViewItem()
		{
			IW::MemZero(static_cast<LVITEM*>(this), sizeof(LVITEM));
		}

		ListViewItem(int n)
		{
			IW::MemZero(static_cast<LVITEM*>(this), sizeof(LVITEM));
			mask = LVIF_PARAM;
			iItem = n;
		}

		ListViewItem(int n, void *lParamIn)
		{
			IW::MemZero(static_cast<LVITEM*>(this), sizeof(LVITEM));

			mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			iItem = n;
			pszText = (LPTSTR)LPSTR_TEXTCALLBACK;
			lParam = (LPARAM)lParamIn;
		}

	};

	// CArrayDWORD Helper
	class CArrayDWORD : public CSimpleValArray<DWORD>
	{
	public:
		CArrayDWORD() {};
		virtual ~CArrayDWORD() {};

		CArrayDWORD(const CArrayDWORD &aIn)
		{ 
			Copy(aIn);
		}

		CArrayDWORD &operator=(const CArrayDWORD &aIn)
		{
			Copy(aIn);
			return *this;
		}

		void Copy(const CArrayDWORD &aIn)
		{
			RemoveAll();

			for(int i = 0; i < aIn.GetSize(); i++)
			{
				Add(aIn[i]);
			}
		}


		// Serialisation
		CString GetAsString() const
		{
			if (GetSize() == 0)
			{
				return g_szEmptyString;
			}

			CString str;

			for(int i = 0; i < GetSize(); i++)
			{
				str += IToStr(operator[](i));
				str += _T("-");
			}

			return str;
		}

		bool ParseFromString(const CString &str)
		{
			if (str.IsEmpty())
			{
				RemoveAll();
				return true;
			}

			bool bFirst = true;

			static const TCHAR szSeps[] = _T(" -");
			CString token;
			int curPos = 0;

			token = str.Tokenize(szSeps, curPos);
			while (!token.IsEmpty())
			{
				if (bFirst)
				{
					RemoveAll();
					bFirst = false;
				}

				DWORD dw = _ttol(token);
				Add(dw);

				token = str.Tokenize(szSeps, curPos);
			};


			return true;
		}
	};

	// CoInitialize Helper
	class CCoInit
	{
	public:

		CCoInit(bool bIsWorkerThead)
		{
			HRESULT hRes = ::CoInitializeEx(NULL, bIsWorkerThead ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);
			assert(SUCCEEDED(hRes));
		}

		~CCoInit()
		{
			::CoUninitialize();
		};
	};

	class  CMessageBoxIndirect : public MSGBOXPARAMS
	{
	public:

		CMessageBoxIndirect();
		static VOID CALLBACK ErrorBoxCallBack(LPHELPINFO lpHelpInfo);

		int Show(unsigned nId, unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);
		int Show(const CString &str = _T("A serious error has occurred."), unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);
		int Show(unsigned nIdFormat, const CString &strMessage, unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);

		int ShowOsError(unsigned nId, int nError = GetLastError(), unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);
		int ShowOsErrorWithFile(const CString &strFile, unsigned nId, int nError = GetLastError(), unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);

		int ShowException(unsigned nId, _com_error &e, unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);
		int ShowException(unsigned nId, std::exception &e, unsigned nStyle = MB_ICONHAND | MB_OK | MB_HELP);

	};

	///////////////////////////////////////////////////////////////
	//
	// CSearch
	//
	typedef enum tagSEARCH_EXPRESSION
	{
		seAND = 0,
		seOR = 1,
		seNOT = 2
	}
	SEARCH_EXPRESSION;

	class CSearchNode
	{
	public:

		DWORD _se;
		CString _str;

		CSearchNode();
		CSearchNode(DWORD se, const CString &str, bool bNot = false);
		CSearchNode(const CSearchNode &sn);

		bool ParseFromString(const CString &strPrefix, const CString &strText);
	}; 

	class CSearchNodeList
	{
	protected:
		CSimpleArray<CSearchNode> _children;

	public:
		CSearchNodeList();
		virtual ~CSearchNodeList();

		CSearchNodeList(const CSearchNodeList &ss);
		void operator=(const CSearchNodeList &ss);
		void Copy(const CSearchNodeList &ss);

		void operator=(const CString &strSource) { ParseFromString(strSource); };

		bool ParseFromString(const CString &strSource);
		bool ParseFromString(DWORD se, const CString &str);
		bool ParseFromString(const CString &strNOT, const CString &strAND, const CString &strOR);

		bool Match(const CString &strSource, bool bMatchSubString = false) const;

		bool Format(CString &strOut);
		bool Format(CString &strNOT, CString &strAND, CString &strOR);	

		bool IsEmpty() const { return (_children.GetSize() == 0); };
	};

	class CFilePath
	{
	protected:
		CString _strPath;

	public:
		CFilePath()
		{
		}

		CFilePath(const CString &str) : _strPath(str)
		{
		}

		CFilePath(const CFilePath &other) : _strPath(other._strPath)
		{
		}		

		void operator+=(const CString &str)
		{
			if (!_strPath.IsEmpty()) TerminateFolderPath();
			_strPath += str;
		}		

		void operator=(const CString &str)
		{
			_strPath = str;
		}

		void operator=(const CFilePath &other)
		{
			_strPath = other._strPath;
		}

		bool operator==(const CFilePath &other) const
		{
			CFilePath path1(*this);
			CFilePath path2(other);

			path1.NormalizeForCompare();
			path2.NormalizeForCompare();

			return path1._strPath == path2._strPath;
		}

		const CString &ToString() const
		{
			return _strPath;
		}

		void CopyTo(LPTSTR sz, int nMax)
		{
			_tcsncpy_s(sz, nMax, _strPath, nMax);
		}

		bool Exists() const
		{
			return PathFileExists(_strPath) != 0;
		}

		void GetModuleFileName(HMODULE hModule)
		{
			::GetModuleFileName(hModule, _strPath.GetBuffer(MAX_PATH), MAX_PATH);
			_strPath.ReleaseBuffer();
		}

		void GetTempFilePath()
		{
			static UINT uUnique = (UINT)time(NULL);

			TCHAR szTempPath[MAX_PATH];
			TCHAR szTempFileName[MAX_PATH];

			::GetTempPath(_MAX_PATH, szTempPath);
			::GetTempFileName(szTempPath, _T("IW"), ++uUnique, szTempFileName);

			_strPath = szTempFileName;
		}

		void GetTemplateFolder(HMODULE hModule)
		{
			GetModuleFileName(hModule);
			RemoveFileName();

			TerminateFolderPath();
			_strPath += g_szTemplateFolder;
			TerminateFolderPath();
		}

		void GetExeFolder(HMODULE hModule)
		{
			GetModuleFileName(hModule);
			RemoveFileName();
			TerminateFolderPath();
		}

		bool IsTerminated() const
		{
			int nLast = _strPath.GetLength() - 1;

			return  (nLast >= 0) && 
				(_strPath[nLast] == _T('\\') ||
				_strPath[nLast] == _T('/'));
		}

		void TerminateFolderPath()
		{
			if (!IsTerminated())
			{
				_strPath += _T('\\');
			}
		}

		void RemoveTermination()
		{
			if (IsTerminated())
			{
				int nLast = _strPath.GetLength() - 1;
				_strPath.SetAt(nLast, 0);
			}
		}

		void MakeUnixPath()
		{
			_strPath.Replace(_T('\\'), _T('/'));
		}

		void MakeDosPath()
		{
			_strPath.Replace(_T('/'), _T('\\'));
		}

		void GetCurrentDirectory()
		{
			::GetCurrentDirectory(MAX_PATH, _strPath.GetBuffer(MAX_PATH));
			_strPath.ReleaseBuffer();
		}

		void CombinePathAndFilename(const CString &strPath, const CString &strFilename)
		{
			::PathCombine(_strPath.GetBuffer(MAX_PATH), strPath, strFilename);
			_strPath.ReleaseBuffer();
		}

		void RemoveIllegalFromFileName()
		{
			TCHAR szFileName[_MAX_FNAME];
			TCHAR szExt[_MAX_EXT];
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];

			_tsplitpath_s( _strPath, szDrive, countof(szDrive), szDir, countof(szDir), szFileName, countof(szFileName), szExt, countof(szExt));

			RemoveIllegalFromFileName(szFileName);
			RemoveIllegalFromFileName(szExt);

			MakePath(szDrive, szDir, szFileName, szExt);
		}

		void RemoveIllegal()
		{
			TCHAR szFileName[_MAX_FNAME];
			TCHAR szExt[_MAX_EXT];
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];

			_tsplitpath_s( _strPath, szDrive, countof(szDrive), szDir, countof(szDir), szFileName, countof(szFileName), szExt, countof(szExt));

			RemoveIllegalFromFileName(szFileName);
			RemoveIllegalFromFileName(szExt);
			RemoveIllegalFromFileName(szDir);

			MakePath(szDrive, szDir, szFileName, szExt);
		}

		void MakePath(LPCTSTR szDrive, LPCTSTR szDir, LPCTSTR szFileName, LPCTSTR szExt)
		{
			_tmakepath_s(_strPath.GetBuffer(MAX_PATH), MAX_PATH, szDrive, szDir, szFileName, szExt);
			_strPath.ReleaseBuffer();
		}

		void RemoveIllegalFromFileName(LPTSTR sz)
		{
			// remove chars we dont like
			while(*sz != 0)
			{
				switch(*sz)
				{
				case '\'':
				case '\"':
				case ' ':
				case '?':
				case '&':
					*sz = '_';
					break;
				default:
					break;
				}

				sz++;
			}	
		}

		CString GetFileName() const
		{
			TCHAR szFileName[_MAX_FNAME];
			_tsplitpath_s(_strPath, NULL, 0, NULL, 0, szFileName, _MAX_FNAME, NULL, 0);
			return szFileName;
		}

		void SetExtension(LPCTSTR szExt)
		{
			TCHAR szFileName[_MAX_FNAME];
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];

			_tsplitpath_s(_strPath, szDrive, countof(szDrive), szDir, countof(szDir), szFileName, countof(szFileName), NULL, 0);
			MakePath(szDrive, szDir, szFileName, szExt);
		}

		void SetFileNameAndExtension(LPCTSTR szFileName, LPCTSTR szExt)
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];

			_tsplitpath_s(_strPath, szDrive, countof(szDrive), szDir, countof(szDir), NULL, 0, NULL, 0);
			MakePath(szDrive, szDir, szFileName, szExt);
		}


		void NormalizeForCompare()
		{
			RemoveTermination();
			MakeDosPath();
			MakeUpper();
		}

		void Normalize(bool bIsFolder)
		{
			if (bIsFolder)
			{
				TerminateFolderPath();
			}

			MakeUnixPath();
		}

		void RemoveFileName()
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];

			_tsplitpath_s(_strPath, szDrive, countof(szDrive), szDir, countof(szDir), NULL, 0, NULL, 0);
			MakePath(szDrive, szDir, NULL, NULL);
		}

		void StripToFilename()
		{
			TCHAR szFileName[_MAX_FNAME];
			_tsplitpath_s(_strPath, NULL, 0, NULL, 0, szFileName, countof(szFileName), NULL, 0);
			_strPath = szFileName;
		}

		void StripToExtension()
		{
			CString str = PathFindExtension(_strPath);
			_strPath = str;
		}

		void StripToFilenameAndExtension()
		{
			CString str = PathFindFileName(_strPath);
			_strPath = str;
		}

		void StripToPath()
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR]; 

			_tsplitpath_s(_strPath, szDrive, countof(szDrive), szDir, countof(szDir), NULL, 0, NULL, 0);
			MakePath(szDrive, szDir, NULL, NULL );
		}

		void StripToFolderName()
		{
			RemoveTermination();
			StripToFilenameAndExtension();
		}

		void Prefix(const CString &strPre)
		{
			_strPath = strPre + _strPath;
		}

		void PrefixToFileName(const CString &strPre)
		{
			TCHAR szFileName[_MAX_FNAME];
			TCHAR szExt[_MAX_EXT];
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];

			_tsplitpath_s(_strPath, szDrive, countof(szDrive), szDir, countof(szDir), szFileName, countof(szFileName), szExt, countof(szExt));

			TCHAR sz[MAX_PATH];
			_tcscpy_s(sz, MAX_PATH, strPre);
			_tcscat_s(sz, MAX_PATH, szFileName);

			MakePath(szDrive, szDir, sz, szExt );	
		}

		void PrefixAsPath(const CString &strPre, bool bFullyQualifiedPath)
		{
			if (bFullyQualifiedPath)
			{
				TCHAR szFileName[_MAX_FNAME];
				TCHAR szExt[_MAX_EXT];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];

				// Get image target
				_tsplitpath_s(_strPath, szDrive, countof(szDrive), szDir, countof(szDir), szFileName, countof(szFileName), szExt, countof(szExt));

				int nLen = _tcsclen(szDir);

				if (nLen && (szDir[nLen - 1] != '\\' &&
					szDir[nLen - 1] != '/'))
				{
					szDir[nLen] = '/';
					szDir[nLen + 1] = 0;
				}

				_tcscat_s(szDir, _MAX_DIR, strPre);

				MakePath(szDrive, szDir, szFileName, szExt );
			}
			else
			{
				CString str = _strPath;
				_strPath.Format(_T("%s/%s"), strPre, str);
			}
		}

		void MakeLower()
		{
			_strPath.MakeLower();
		}

		void MakeUpper()
		{
			_strPath.MakeUpper();
		}

		static bool CreateAllDirectories(const CString &strFolder)
		{
			TCHAR sz[MAX_PATH], szCurrent[MAX_PATH]; 
			_tcscpy_s(sz, MAX_PATH, strFolder);

			int n = _tcsclen(sz) - 1;

			if (n < 3)
				return true;

			// remove ending / if exists
			while(n > 0 && (sz[n] == '\\' || sz[n] == '/'))
				sz[n--] = 0; 

			_tcscpy_s(szCurrent, MAX_PATH, sz);

			// base case . . .if directory exists
			if(GetFileAttributes(sz)!=-1) 
				return true;

			// remove ending folder
			// recursive call, one less directory
			while(n > 0 && sz[n] != '\\' && sz[n] != '/')
				sz[n--] = 0; 

			if (!CreateAllDirectories(sz))
				return false;

			// actual work
			return CreateDirectory(szCurrent,NULL) != 0; 
		}

		static bool CheckFileName(const CString &strFileNameIn)
		{
			TCHAR szName[_MAX_FNAME];
			_tsplitpath_s(strFileNameIn, NULL, 0, NULL, 0, szName, countof(szName), NULL, 0);

			if (_tcsclen(szName) > 215)
			{
				return false;
			}

			static TCHAR szIllegal[] = _T("\\/:*?\"<>|");

			for (int i = 0; szIllegal[i] != 0; i++)
			{
				if (_tcschr(szName, szIllegal[i]) != NULL)
					return false;
			}

			return true;
		}

		bool CreateAllDirectories() { return CreateAllDirectories(_strPath); };

		operator LPCTSTR() const 
		{ 
			return _strPath; 
		};

		operator const CString&() const 
		{ 
			return _strPath; 
		};

		operator CString&()
		{ 
			return _strPath; 
		};
	};








	//////////////////////////////////////////////////////////////////////////////
	// Template Helpers

	template<class T> CComPtr<T> CreateComObject()
	{	
		CComObject<T> *p;
		p = new CComObject<T>;
		if (p == NULL) throw std::bad_alloc();

		p->SetVoid(NULL);
		p->InternalFinalConstructAddRef();

		HRESULT hRes = p->FinalConstruct();

		if (SUCCEEDED(hRes))
		{
			hRes = p->_AtlFinalConstruct();
		}

		p->InternalFinalConstructRelease();

		if (FAILED(hRes))
		{
			assert("CreateComObject Failed");

			delete p;
			p = NULL;
			throw std::bad_alloc();
		}

		return CComPtr<T>(p);
	}


	///////////////////////////////////////////////////////////////////////
	//
	// CAutoLock
	// Normally used to lock a CCriticalSection
	//  

	class CAutoLockCS
	{

	public:

		CAutoLockCS(CCriticalSection &lock) : _lock(lock)
		{
			_lock.Enter();
		}

		~CAutoLockCS()
		{
			_lock.Leave();
		}

		// Implementation
	private:
		CCriticalSection &_lock;

		// Private to prevent accidental use
		CAutoLockCS( const CAutoLockCS& ) throw();
		CAutoLockCS& operator=( const CAutoLockCS& ) throw();
	};


	class CNullStatus : public IStatus
	{
	public:
		// Iterate Properties
		void SetMessage(const CString &strMessage) { };
		void SetWarning(const CString &strWarning) { };
		void SetError(const CString &strError) { };
		void SetContext(const CString &strContext) { };
		void Progress(int nCurrentStep, int TotalSteps) { };
		bool QueryCancel() { return false; };
		void SetStatusMessage(const CString &strMessage) { };
		void SetHighLevelProgress(int nCurrentStep, int TotalSteps) { };
		void SetHighLevelStatusMessage(const CString &strMessage) { };

		static CNullStatus *Instance;
	};

	class Path
	{
	public:
		static CString Combine(const CString strFolder, const CString strFile)
		{
			CString strFolderCopy = strFolder; 
			TCHAR chFolderLastChar = strFolderCopy[strFolderCopy.GetLength() - 1];

			if (chFolderLastChar != _T('\\') && chFolderLastChar != _T('/'))
			{
				strFolderCopy += _T('\\') ;
			}
			return strFolderCopy + strFile;
		}

		static bool Compare(CFilePath path1, CFilePath path2)
		{
			return path1 == path2;
		}

		static LPCTSTR FindExtension(const CString &strPath)
		{
			return PathFindExtension(strPath);
		}

		static LPCTSTR FindFileName(const CString &strPath)
		{
			return PathFindFileName(strPath);
		}

		static bool FileExists(const CString &strPath)
		{
			return PathFileExists(strPath) != 0;
		}

		static bool IsDirectory(const CString &strPath)
		{
			return PathIsDirectory(strPath) != 0; 
		}

		static CString PersonalFolder()
		{
			TCHAR sz[MAX_PATH];
			if (!SHGetSpecialFolderPath(IW::GetMainWindow(), sz, CSIDL_PERSONAL, TRUE))
			{
				GetCurrentDirectory(MAX_PATH, sz);
			}
			return sz;
		}
	};


	class FileSize
	{
	private:
		__int64 _size; 

	public:

		FileSize(): _size(0)
		{
		}

		FileSize(DWORD nHigh, DWORD nLow) : _size((((__int64)nHigh) << 32 ) + nLow)
		{
		}

		FileSize(const FileSize &other): _size(other._size)
		{
		}

		FileSize(unsigned nSize): _size(nSize)
		{
		}

		void operator=(const FileSize &other)
		{
			_size = other._size;
		}

		void operator=(unsigned nSize)
		{
			_size = nSize;
		}

		void operator+=(const FileSize &other)
		{
			_size += other._size;
		}

		operator int() const
		{
			return (int)_size;
		}

		static FileSize FromFile(const CString &strFileName)
		{
			WIN32_FIND_DATA   ff32; MemZero(&ff32, sizeof(ff32));

			if (!IW::IsNullOrEmpty(strFileName))
			{
				HANDLE hFind = FindFirstFile(strFileName, &ff32);

				if (INVALID_HANDLE_VALUE != hFind)
				{
					FindClose(hFind);        
				}
			}

			return FileSize(ff32.nFileSizeHigh, ff32.nFileSizeLow);
		}

		static FileSize FromHandle(HANDLE hFile)
		{
			assert(hFile != INVALID_HANDLE_VALUE);

			DWORD nHigh = 0;
			DWORD nLow = ::GetFileSize(hFile, &nHigh);

			return FileSize(nHigh, nLow);
		}

		CString ToString(LCID locale = LOCALE_USER_DEFAULT) const
		{
			static const int nMB = 1024 * 1024;
			static const int nKB = 1024;		

			LPCTSTR szUnitType = _T(" kb");
			int divisor = nKB;
			int nUnitsFraction = 0;

			// KB or MB
			if (_size > nMB)
			{
				divisor = nMB;
				szUnitType = _T(" mb");
			}	

			static const int nBufferSize = 128;
			TCHAR szFileSize[nBufferSize];

			CString str;
			str.Format(_T("%d.%02d"), _size / divisor, (_size % divisor) / 10);
			GetNumberFormat(locale, 0, str, NULL, szFileSize, nBufferSize);

			str = szFileSize;
			str += szUnitType;
			return str;
		}
	};


	class FileOperation
	{
	private:
		SHFILEOPSTRUCT _struct;
		IW::CAutoFree<TCHAR> szFiles; 

	public:
		FileOperation(const CString &strFiles)
		{			
			IW::MemZero(&_struct, sizeof(_struct));
			_struct.hwnd = IW::GetMainWindow();

			TCHAR szDelim = _T('\n');
			LPTSTR sz = IW::StrDup(strFiles);

			for(int n = 0; sz[n] != 0; n++)
			{
				if (sz[n] == szDelim)
				{
					sz[n] = 0;
				}
			}

			szFiles = sz;
			_struct.pFrom = sz;
		}

		bool Delete(bool bAllowUndo)
		{
			_struct.wFunc = FO_DELETE;
			_struct.fFlags = bAllowUndo ? FOF_ALLOWUNDO : 0;

			return 0 == SHFileOperation(&_struct);
		}

		bool Copy(const CString &strDestFolder, bool bMove)
		{
			_struct.wFunc = bMove ? FO_MOVE : FO_COPY;
			_struct.pTo = strDestFolder;
			_struct.fFlags = FOF_ALLOWUNDO;

			return 0 == SHFileOperation(&_struct);
		}
	};

	class ScopeLockedBool
	{
	private:
		volatile bool &_b;
	public:
		ScopeLockedBool(volatile bool &b) : _b(b)
		{
			_b = true;
		}

		~ScopeLockedBool()
		{
			_b = false;
		}

	};

	class CLockWindowUpdate
	{
	public:
		CWindow _wnd;

		CLockWindowUpdate(CWindow wnd) : _wnd(wnd)
		{
			_wnd.LockWindowUpdate(true);
		}

		~CLockWindowUpdate()
		{
			_wnd.LockWindowUpdate(false);
		}
	};


	inline bool NavigateToWebPage(const CString url)
	{
		return ShellExecute(IW::GetMainWindow(),
			_T("open"),
			url,
			g_szEmptyString,
			g_szEmptyString,
			SW_SHOW) > 0;
	}

	inline int SetItem(CComboBox ctrl, int nNumber)
	{
		CString str;
		str.Format(_T("%4d"), nNumber); 

		int nSel = ctrl.FindString(-1, str);

		if (CB_ERR  == nSel)
		{
			return ctrl.AddString(str); 
		}

		return nSel;
	}

	inline void SetItem(CComboBox combo, const CString &str, UINT nFlags)
	{
		combo.SetItemData(combo.AddString(str), nFlags);
	}

	inline void SetItems(CComboBox ctrl, int values[], int nDefault)
	{
		for(int i = 0; values[i] != -1; i++)
		{
			IW::SetItem(ctrl, values[i]);
		}

		int nSel = IW::SetItem(ctrl, nDefault);
		ctrl.SetCurSel(nSel);
	}

	inline CSize GetToolbarSize(CToolBarCtrl tb)
	{
		CRect r;
		tb.GetItemRect(tb.GetButtonCount() - 1, r);
		return CSize(r.right, r.bottom);
	}

	inline CString Format(UINT nFormatID, ... )
	{
		CString str;
		CString strFormat;
		ATLVERIFY( strFormat.LoadString( nFormatID ) );

		va_list argList;
		va_start( argList, nFormatID );
		str.FormatV( strFormat, argList );
		va_end( argList );

		return str;
	}

	inline CString Format(const CString strFormat, ... )
	{
		CString str;
		va_list argList;
		va_start( argList, strFormat );
		str.FormatV( strFormat, argList );
		va_end( argList );
		return str;
	}

	inline void AddToList(CString &list, const CString &item)
	{
		if (!list.IsEmpty()) list += _T(", ");
		list += item;
	}

	inline CSize Half(const CSize &size)
	{
		return CSize(size.cx / 2, size.cy / 2);
	}
	
	inline int Half(const int n)
	{
		return n / 2;
	}

} // namespace IW
