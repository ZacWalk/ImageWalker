///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// CompressFile.cpp: implementation of the IW::CCompressFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "..\Libraries\FreeImage\Source\zlib\zlib.h"

namespace IW
{


	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////



	void ThrowIfError(int err) 
	{ 
		if (err != Z_OK) 
		{ 
			assert(0);
			throw std::exception("Invalid File");
		}
	}


	class IW::CCompressFileInternals : public z_stream
	{
	public:
		IW::CCompressFileInternals()
		{
			zalloc = (alloc_func)0;
			zfree = (free_func)0;
			opaque = (voidpf)0;
		};
	};



	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////



	IW::CCompressFile::CCompressFile()
	{
		m_pZStream = 0;
	}

	IW::CCompressFile::~CCompressFile()
	{
		if (m_pZStream)
			delete m_pZStream;

	}

	bool IW::CCompressFile::OpenForWrite(const CString &strFileName)
	{
		if (0 == m_pZStream)
		{
			m_pZStream = new IW::CCompressFileInternals();
		}

		if (!CFile::OpenForWrite(strFileName))
			return FALSE;

		int err = deflateInit(m_pZStream, Z_BEST_COMPRESSION);
		ThrowIfError(err);

		m_pZStream->next_out = m_buffer;
		m_pZStream->avail_out = buffer_size;
		m_bCompress = TRUE;

		return TRUE;
	}

	bool IW::CCompressFile::OpenForRead(const CString &strFileName)
	{
		if (0 == m_pZStream)
		{
			m_pZStream = new IW::CCompressFileInternals();
		}

		if (!CFile::OpenForRead(strFileName))
			return FALSE;

		int err = inflateInit(m_pZStream);
		ThrowIfError(err);

		m_pZStream->avail_in = 0;

		m_bCompress = FALSE;

		return TRUE;
	}



	bool IW::CCompressFile::Close(IStatus *pStatus)
	{
		int err = Z_OK;

		if (m_bCompress)
		{
			while(err == Z_OK) 
			{
				err = deflate(m_pZStream, Z_FINISH);

				DWORD nOut = buffer_size - m_pZStream->avail_out;

				if (nOut > 0)
				{
					DWORD nWritten;
					if (!::WriteFile(_hFile, &m_buffer, nOut, &nWritten, NULL))
					{
						throw std::exception("Invalid File");
					}

					// Win32s will not return an error all the time (usually DISK_FULL)
					if (nWritten != nOut)
					{
						throw IW::invalid_file();
					}

					m_pZStream->next_out = m_buffer;
					m_pZStream->avail_out = buffer_size;
				}

				if (err == Z_STREAM_END) break;

				ThrowIfError(err);

			}

			deflateEnd(m_pZStream);
		}
		else
		{
			err = inflateEnd(m_pZStream);
			ThrowIfError(err);
		}

		return CFile::Close(pStatus);
	}

	bool IW::CCompressFile::Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead)
	{
		assert(this);
		assert(_hFile != INVALID_HANDLE_VALUE);

		if (nCount == 0)
			return 0;   // avoid Win32 null-read

		assert(lpBuf != NULL);

		m_pZStream->next_out = (LPBYTE)lpBuf;
		m_pZStream->avail_out = nCount;
		DWORD dwRead;

		while(m_pZStream->avail_out > 0)
		{
			if (m_pZStream->avail_in == 0)
			{
				if (!::ReadFile(_hFile,  m_buffer, buffer_size, &dwRead, NULL))
					throw IW::invalid_file();

				m_pZStream->avail_in = dwRead;
				m_pZStream->next_in = m_buffer;

				if (dwRead == 0)
					break;
			}

			int err = inflate(m_pZStream, Z_NO_FLUSH);
			if (err == Z_STREAM_END) break;
			ThrowIfError(err);
		}

		DWORD dw = (UINT)nCount - m_pZStream->avail_out;

		if (pdwRead)
		{
			*pdwRead = dw;
		}

		return nCount == dw;
	}

	bool IW::CCompressFile::Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten)
	{
		DWORD nWritten = 0;
		DWORD nWrittenTotal = 0;

		assert(this);
		assert(_hFile != INVALID_HANDLE_VALUE);
		assert(m_bCompress);

		if (nCount == 0)
			return true;     // avoid Win32 null-write option

		assert(lpBuf != NULL);

		m_pZStream->next_in  = (Bytef*)lpBuf;
		m_pZStream->avail_in = nCount;

		while (m_pZStream->avail_in != 0)
		{
			int err = deflate(m_pZStream, Z_NO_FLUSH);
			ThrowIfError(err);

			if (m_pZStream->avail_out == 0)
			{

				if (!::WriteFile(_hFile, &m_buffer, buffer_size, &nWritten, NULL))
					throw IW::invalid_file();

				// Win32s will not return an error all the time (usually DISK_FULL)
				if (nWritten != buffer_size)
					throw IW::invalid_file();

				m_pZStream->next_out = m_buffer;
				m_pZStream->avail_out = buffer_size;
			}
		}

		if (pdwWritten)
		{
			*pdwWritten = nCount;
		}

		return true;
	}

	bool IW::CCompressFile::Flush()
	{
		DWORD nWritten = 0;
		DWORD nWrittenTotal = 0;

		assert(this);
		assert(_hFile != INVALID_HANDLE_VALUE);
		assert(m_bCompress);

		do
		{
			int nCount = m_pZStream->next_out - m_buffer;

			if (nCount)
			{
				if (::WriteFile(_hFile, &m_buffer, nCount, &nWritten, NULL))
					throw IW::invalid_file();

				m_pZStream->next_out = m_buffer;
				m_pZStream->avail_out = buffer_size;
			}

			int err = deflate(m_pZStream, Z_SYNC_FLUSH);
			ThrowIfError(err);
		}
		while (m_pZStream->next_out != m_buffer);


		return ::FlushFileBuffers(_hFile) != 0;
	}

	LONG IW::CCompressFile::Seek(LONG lOff, UINT nFrom)
	{
		// Nothing we can do?
		assert(0);	
		return 0;
	}

}; // namespace IW


