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

namespace IW
{
	//////////////////////////////////////////////////////////////////////////////////
	/// StreamBlob : Abstract base class to allow loading from a BLOB
	class StreamConstBlob : public IStreamIn
	{
	protected:
		LPCBYTE m_pBytes;
		FileSize m_nSize;
		DWORD _nPosition;

		bool m_bDelete;

	public:

		StreamConstBlob(LPCBYTE pBytes, const FileSize &size)
		{
			m_pBytes = pBytes;
			_nPosition = 0;
			m_nSize = size;
			m_bDelete = false;
		}

		template<class TBlob>
		StreamConstBlob(const TBlob &blob)
		{
			m_pBytes = blob.GetData();
			_nPosition = 0;
			m_nSize = blob.GetDataSize();
			m_bDelete = false;
		}

		~StreamConstBlob()
		{
			if (m_bDelete && m_pBytes)
			{
				IW::Free((LPVOID)m_pBytes);
			}
		}

		bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0)
		{
			UINT nAbsoluteSize = min(nCount, m_nSize - _nPosition);
			IW::MemCopy(lpBuf, m_pBytes + _nPosition, nAbsoluteSize);
			_nPosition += nAbsoluteSize;

			if (pdwRead) *pdwRead = nAbsoluteSize;

			return nAbsoluteSize > 0;
		}




		DWORD Seek(ePosition ePos, LONG lOff)
		{
			UINT nPosNew = _nPosition;

			switch(ePos)
			{
			case IW::IStreamCommon::eBegin:
				nPosNew = lOff;
				break;
			case IW::IStreamCommon::eEnd:
				nPosNew = m_nSize - lOff;
				break;
			default:
				// Inknown seek?
				assert(0);
			case IW::IStreamCommon::eCurrent:
				nPosNew += lOff;
				break;
			}

			return _nPosition = Clamp(nPosNew,0,m_nSize-1);
		}

		bool Flush()
		{
			return true;
		}

		FileSize GetFileSize()
		{
			return m_nSize;
		}


		CString GetFileName() { return g_szEmptyString; };
		bool Abort() { return true; };
		bool Close(IW::IStatus *) { return true; };

	};


	template<class TBlob>
	class StreamBlob :
		public IStreamIn,
		public IStreamOut
	{
	protected:
		TBlob &m_blob;
		DWORD _nPosition;
		FileSize m_nSize;

	public:

		StreamBlob(TBlob &blob) : m_blob(blob)
		{
			_nPosition = 0;
			m_nSize = m_blob.GetDataSize();
		}

		~StreamBlob()
		{
		}

		bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0)
		{
			UINT nAbsoluteSize = min(nCount, GetFileSize() - _nPosition);

			if (nAbsoluteSize)
			{
				IW::MemCopy(lpBuf, GetData() + _nPosition, nAbsoluteSize);
				_nPosition += nAbsoluteSize;
			}

			if (pdwRead) *pdwRead = nAbsoluteSize;

			return nAbsoluteSize > 0;
		}

		bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0)
		{
			UINT nNewSize = IW::Max(_nPosition + nCount, GetFileSize());
			ReAlloc(nNewSize);

			IW::MemCopy((LPBYTE)GetData() + _nPosition, lpBuf, nCount);
			_nPosition += nCount;
			m_nSize = nNewSize;

			if (pdwWritten) *pdwWritten = nCount;

			return true;
		}

		void ReAlloc(int nRequired)
		{
			if( m_blob.GetDataSize() < (unsigned)nRequired )
			{
				int nAllocate = m_blob.GetDataSize();
				const int nStep = 1024 * 1024;

				if( nAllocate > nStep )
				{
					nAllocate += nStep;
				}
				else
				{
					nAllocate *= 2;
				}

				if( nAllocate < nRequired )
				{
					nAllocate = nRequired;
				}

				m_blob.ReAlloc(nAllocate);
			}
		}

		DWORD Seek(ePosition ePos, LONG lOff)
		{
			UINT nPosNew = _nPosition;

			switch(ePos)
			{
			case IW::IStreamCommon::eBegin:
				nPosNew = lOff;
				break;
			case IW::IStreamCommon::eEnd:
				nPosNew = GetFileSize() - lOff;
				break;
			default:
				// Inknown seek?
				assert(0);
			case IW::IStreamCommon::eCurrent:
				nPosNew += lOff;
				break;
			}

			_nPosition = nPosNew;

			UINT nNewSize = IW::Max(_nPosition, GetFileSize());
			ReAlloc(nNewSize);

			return _nPosition;
		}

		bool Flush()
		{
			return true;
		}

		
		CString GetFileName() { return g_szEmptyString; };

		bool Abort() { return true; };

		bool Close(IW::IStatus *) 
		{
			m_blob.ReAlloc(m_nSize);			
			return true; 
		};

		inline FileSize GetFileSize() { return m_nSize; };
		inline LPCBYTE GetData() const { return m_blob.GetData(); };
	};


	//////////////////////////////////////////////////////////////////////////////////
	/// Stream : Abstract base class to allow loading from resource files
	class StreamResource : public StreamConstBlob
	{
	protected:
		HRSRC m_hrsrc;
		HGLOBAL m_hg;

	public:

		StreamResource(HINSTANCE hInstance, const int nID) : StreamConstBlob(0, 0)
		{

			m_hrsrc = ::FindResource(hInstance, MAKEINTRESOURCE(nID), _T("IMAGE"));

			if (!m_hrsrc) 
			{
				throw std::exception("Failed to load bitmap resource"); 
			}

			m_hg = ::LoadResource(hInstance, m_hrsrc);

			if (!m_hg) 
			{
				throw std::exception("Failed to load bitmap resource");
			}

			m_pBytes = (LPCBYTE)::LockResource(m_hg);
			_nPosition = 0;
			m_nSize = SizeofResource(hInstance, m_hrsrc);
		}

		~StreamResource()
		{
			//::UnlockResource(hg);
			::FreeResource(m_hg);
		}
	};




	///////////////////////////////////////////////////////////////
	//
	// CMemMapFile
	//


	class CMemMapFile : public StreamConstBlob
	{
	protected:
		HANDLE _hFile;
		HANDLE _hFileMap;
		CString _strFileName;

	public:
		CMemMapFile() : StreamConstBlob(0, -1)
		{
			_hFileMap = 0;
			_hFile = INVALID_HANDLE_VALUE;
		}

		virtual ~CMemMapFile()
		{
			Close();
		}

		void Close()
		{
			if (_hFileMap != 0) 
			{
				CloseHandle(_hFileMap);
				_hFileMap = 0;
			}

			if (_hFile != INVALID_HANDLE_VALUE) 
			{
				CloseHandle(_hFile);
				_hFile = INVALID_HANDLE_VALUE;
			}
		}

		bool MapFile(const CString &strFileName)
		{
			_strFileName = strFileName;
			_hFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

			if (_hFile != INVALID_HANDLE_VALUE) 
			{					         
				m_nSize = ::GetFileSize(_hFile, NULL);
				_hFileMap = CreateFileMapping(_hFile, NULL, PAGE_READONLY, 0, 0, NULL);

				if (_hFileMap != 0) 
				{
					m_pBytes = (LPCBYTE)MapViewOfFile(_hFileMap, FILE_MAP_READ, 0, 0, 0);
					_nPosition = 0;
					return true;
				}
			}			

			return false;
		}

		CString GetFileName() { return _strFileName; };

		IW::FileTime GetCreationTime();
		IW::FileTime GetModifiedTime();

		LPCVOID GetMem() const { return m_pBytes; };
		const UINT GetSize() const { return m_nSize; };
	};


	///////////////////////////////////////////////////////////////
	//
	// Class to wrapper file API
	//
	class CFile :
		public IStreamIn,
		public IStreamOut

	{
	protected:
		HANDLE _hFile;
		bool m_bCloseOnDelete;
		CString _strFileName;

	public:

		operator HFILE() const { return (HFILE)_hFile; };


		CFile()
		{
			_hFile = INVALID_HANDLE_VALUE;
			m_bCloseOnDelete = FALSE;
		}


		~CFile()
		{
			if (_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete)
				Close(0);
		}

		bool OpenForWrite(const CString &strFileName)
		{
			return Open(strFileName, GENERIC_WRITE, 0, CREATE_ALWAYS);
		}

		bool OpenForRead(const CString &strFileName)
		{
			return Open(strFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
		}

	protected:
		bool Open(const CString &strFileName, DWORD dwAccess, DWORD dwShareMode, DWORD dwCreateFlag)
		{

			m_bCloseOnDelete = FALSE;
			_hFile = INVALID_HANDLE_VALUE;		

			// attempt file creation
			HANDLE hFile = ::CreateFile(strFileName, dwAccess, dwShareMode, NULL, dwCreateFlag, 0, NULL);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				return FALSE;
			}

			_hFile = hFile;
			m_bCloseOnDelete = TRUE;
			_strFileName = strFileName;

			return TRUE;
		}

	public:

		// String helpers
		void WriteString(const CStringA &str)
		{
			int nSize = str.GetLength();

			Write(&nSize, sizeof(nSize));

			if (nSize == 0)
				return;

			Write(str, nSize);
		}

		bool ReadString(CStringA &str)
		{
			UINT nSize = str.GetLength();

			if (!Read(&nSize, sizeof(nSize)))
				return false;

			if (nSize == 0)
			{
				str.Empty();
			}
			else
			{
				LPSTR p = str.GetBufferSetLength(nSize + 1);

				if (!Read(p, nSize))
				{
					return false;
				}

				p[nSize] = 0;
			}

			return true;
		}

		bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0)
		{
			assert(_hFile != INVALID_HANDLE_VALUE);

			if (nCount == 0)
				return true;   // avoid Win32 "null-read"

			assert(lpBuf != NULL);			

			DWORD dw;
			if (pdwRead == 0) pdwRead = &dw;

			return ::ReadFile(_hFile, lpBuf, nCount, pdwRead, NULL) != 0;
		}

		bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0)
		{
			assert(_hFile != INVALID_HANDLE_VALUE);

			if (nCount == 0)
				return true;     // avoid Win32 "null-write" option

			assert(lpBuf != NULL);

			DWORD dw;
			if (pdwWritten == 0) pdwWritten = &dw;

			return ::WriteFile(_hFile, lpBuf, nCount, pdwWritten, NULL) != 0;
		}

		DWORD Seek(ePosition ePos, LONG offset)
		{
			DWORD dwMoveMethod = FILE_BEGIN;

			/* we use this as a special code, so avoid accepting it */
			if( offset == 0xFFFFFFFF )
				return 0xFFFFFFFF;

			switch(ePos)
			{
			case IStreamIn::eCurrent:
				dwMoveMethod = FILE_CURRENT;
				break;

			case IStreamIn::eEnd:
				dwMoveMethod = FILE_END;
				break;

			default:
				dwMoveMethod = FILE_BEGIN;
				break;
			}

			LONG lowerBits = (LONG)(offset & 0xffffffff);

			return ::SetFilePointer(_hFile, lowerBits, NULL, dwMoveMethod);
		};

		bool Flush()
		{
			if (_hFile == INVALID_HANDLE_VALUE)
				return false;

			return ::FlushFileBuffers(_hFile) != 0;
		}

		bool Close(IStatus *pStatus)
		{

			assert(_hFile != INVALID_HANDLE_VALUE);

			BOOL bError = FALSE;
			if (_hFile != INVALID_HANDLE_VALUE)
				bError = !::CloseHandle(_hFile);

			_hFile = INVALID_HANDLE_VALUE;
			m_bCloseOnDelete = FALSE;

			if (bError)
				std::exception("Failed to clode file");

			return true;
		}

		bool Abort()
		{
			if (_hFile != INVALID_HANDLE_VALUE)
			{
				// close but ignore errors
				::CloseHandle(_hFile);
				_hFile = INVALID_HANDLE_VALUE;
			}

			return true;
		}

		IW::FileSize GetFileSize()
		{
			return IW::FileSize::FromHandle(_hFile);
		}

		bool GetFileInformationByHandle(LPBY_HANDLE_FILE_INFORMATION lpFileInformation)
		{
			return ::GetFileInformationByHandle(_hFile, lpFileInformation) != 0;
		}

		virtual CString GetFileName() { return _strFileName; };

		// insertion operations
		CFile& operator<<(BYTE by);
		CFile& operator<<(WORD w);
		CFile& operator<<(LONG l);
		CFile& operator<<(DWORD dw);
		CFile& operator<<(float f);
		CFile& operator<<(double d);

		CFile& operator<<(int i);
		CFile& operator<<(short w);
		CFile& operator<<(char ch);
		CFile& operator<<(unsigned u);

		// extraction operations
		CFile& operator>>(BYTE& by);
		CFile& operator>>(WORD& w);
		CFile& operator>>(DWORD& dw);
		CFile& operator>>(LONG& l);
		CFile& operator>>(float& f);
		CFile& operator>>(double& d);

		CFile& operator>>(int& i);
		CFile& operator>>(short& w);
		CFile& operator>>(char& ch);
		CFile& operator>>(unsigned& u);
	};

	inline CFile& CFile::operator<<(int i)
	{ return CFile::operator<<((LONG)i); }
	inline CFile& CFile::operator<<(unsigned u)
	{ return CFile::operator<<((LONG)u); }
	inline CFile& CFile::operator<<(short w)
	{ return CFile::operator<<((WORD)w); }
	inline CFile& CFile::operator<<(char ch)
	{ return CFile::operator<<((BYTE)ch); }
	inline CFile& CFile::operator<<(BYTE by)
	{ Write(&by, sizeof(by)); return *this; }
	inline CFile& CFile::operator<<(WORD w)
	{ Write(&w, sizeof(w)); return *this; }
	inline CFile& CFile::operator<<(LONG l)
	{ Write(&l, sizeof(l)); return *this; }
	inline CFile& CFile::operator<<(DWORD dw)
	{ Write(&dw, sizeof(dw)); return *this; }
	inline CFile& CFile::operator<<(float f)
	{ Write(&f, sizeof(f)); return *this; }
	inline CFile& CFile::operator<<(double d)
	{ Write(&d, sizeof(d)); return *this; }


	inline CFile& CFile::operator>>(int& i)
	{ return CFile::operator>>((LONG&)i); }
	inline CFile& CFile::operator>>(unsigned& u)
	{ return CFile::operator>>((LONG&)u); }
	inline CFile& CFile::operator>>(short& w)
	{ return CFile::operator>>((WORD&)w); }
	inline CFile& CFile::operator>>(char& ch)
	{ return CFile::operator>>((BYTE&)ch); }
	inline CFile& CFile::operator>>(BYTE& by)
	{ Read(&by, sizeof(by)); return *this; }
	inline CFile& CFile::operator>>(WORD& w)
	{ Read(&w, sizeof(w)); return *this; }
	inline CFile& CFile::operator>>(DWORD& dw)
	{ Read(&dw, sizeof(dw)); return *this; }
	inline CFile& CFile::operator>>(float& f)
	{ Read(&f, sizeof(f)); return *this; }
	inline CFile& CFile::operator>>(double& d)
	{ Read(&d, sizeof(d)); return *this; }
	inline CFile& CFile::operator>>(LONG& l)
	{ Read(&l, sizeof(l)); return *this; }

	///////////////////////////////////////////////////////////////
	//
	// Class to wrapper compressed files
	//
	class CCompressFileInternals;

	class CCompressFile : public CFile  
	{
	public:
		CCompressFile();
		virtual ~CCompressFile();

		virtual bool OpenForRead(const CString &strFileName);
		virtual bool OpenForWrite(const CString &strFileName);
		virtual bool Close(IW::IStatus *pStatus);

		virtual LONG Seek(LONG lOff, UINT nFrom);

		virtual bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0);
		virtual bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0);
		virtual bool Flush();


		virtual UINT ReadRaw(void* lpBuf, UINT nCount) 
		{ return CFile::Read(lpBuf, nCount); };

		virtual void WriteRaw(const void* lpBuf, UINT nCount)
		{ CFile::Write(lpBuf, nCount); };

	private:
		CCompressFileInternals *m_pZStream;
		bool m_bCompress;

		enum { buffer_size = IW::LoadBufferSize };
		BYTE m_buffer[buffer_size + 1];

	};

	class CFileTemp : public IW::CFile
	{
	protected:
		IW::CFilePath _pathTempFileName;
		IW::CFilePath _pathRealFileName;

	public:
		CFileTemp()
		{
		}

		~CFileTemp()
		{
			Close(IW::CNullStatus::Instance);
		}

		bool Close(IW::IStatus *pStatus)
		{
			if (_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete)
			{
				CFile::Close(pStatus); 

				if (_pathTempFileName.Exists())
				{
					bool bSuccess = false;

					if (!MoveFile(_pathTempFileName, _pathRealFileName))
					{
						if (CopyFile(_pathTempFileName, _pathRealFileName, FALSE))
						{
							DeleteFile(_pathTempFileName);
							bSuccess = true;
						}
					}
					else
					{
						bSuccess = true;
					}

					// Copy over the temp file
					// Copy over the file
					if (!bSuccess)
					{
						static TCHAR sz[1024];
						LPTSTR     lpMsgBuffer;
						DWORD dwErr = GetLastError();

						FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
							NULL,
							dwErr,
							MAKELCID(LANG_NEUTRAL, SORT_DEFAULT), // Default language
							(LPTSTR) &lpMsgBuffer,
							0,
							NULL );

						if (lpMsgBuffer)
						{
							int i = 0;

							for(; lpMsgBuffer[i] != 0 && i < 1024; ++i)
							{
								if (lpMsgBuffer[i] == '\r' || lpMsgBuffer[i] == '\n')
								{
									sz[i] = 0;
								}
								else
								{
									sz[i] = lpMsgBuffer[i];
								}
							}

							sz[i] = 0;
							pStatus->SetError(lpMsgBuffer);
							LocalFree( lpMsgBuffer );
						}

						return false;
					}
				}
				else
				{
					// Do nothing
				}

				// Delete the temp file
				DeleteFile(_pathTempFileName);
			}

			return true;
		}

		bool OpenForWrite(const CString &strFileName)
		{
			return Open(strFileName, GENERIC_WRITE, 0, CREATE_ALWAYS);
		}

		bool OpenForRead(const CString &strFileName)
		{
			return Open(strFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
		}	


		bool Abort()
		{
			if (_hFile != INVALID_HANDLE_VALUE)
			{
				// close but ignore errors
				::CloseHandle(_hFile);
				_hFile = INVALID_HANDLE_VALUE;
			}

			return true;
		}


	private:

		bool Open(const CString &strFileName, DWORD dwAccess, DWORD dwShareMode, DWORD dwCreateFlag)
		{
			_pathTempFileName.GetTempFilePath();
			_pathRealFileName = strFileName;

			return CFile::Open(_pathTempFileName, dwAccess, dwShareMode, dwCreateFlag);
		}
	};

	///////////////////////////////////////////////////////////////
	//
	// Class to sent mappi emails
	//

	class CSimpleZip
	{
	protected:
		LPVOID m_pZipFile;
		CSimpleValArray<CString> m_arrayFolderNames;

	public:
		CSimpleZip(void);
		virtual ~CSimpleZip(void);

		bool Create(const CString &strZipFileName);
		bool Close();

		//bool AddFile(IStreamOut **ppStreamOut, const CString &strFileName, IW::FileTime ft, IW::IStatus *pStatus);
		bool AddFile(IStreamIn *ppStreamIn, const CString &strFileName, IW::FileTime ft, IW::IStatus *pStatus);
		bool AddFile(const CString &strFileIn, const CString &strFileName, IW::FileTime ft, IW::IStatus *pStatus);
		bool AddFile(const CString &strFileIn);

		// Item stream implimentation
		// Item Iteration
		bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus);
		bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus);
		bool EndItem();
		bool EndFolder();

		// Called periodically to allow cancel
		bool QueryCancel();
	};

	inline bool LoadTextFile(const CString &strFile, CString &strOut)
	{
		bool bSuccess = false;
		IW::CFile f;

		if (f.OpenForRead(strFile))
		{
			FileSize size = f.GetFileSize();

			if (size > 0)
			{
				CStringA str;
				LPSTR sz = str.GetBuffer(size + 1);
				f.Read(sz, size);
				str.ReleaseBufferSetLength(size);
				strOut = str;
				bSuccess = true;
			}

			f.Close(0);
		}

		return bSuccess;
	}
}