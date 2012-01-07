#pragma once

#include <cassert>

#if defined(_DEBUG) 

#include <malloc.h>
#include <crtdbg.h>

#endif

namespace IW
{

#define IW_ALLOCA(type, size) static_cast<type>(_alloca(size))
extern SIZE_T MemoryUsage;

	//////////////////////////////////////////////////////////////////////////////////
	// Safe allocation

#if defined(_DEBUG) 

	LPVOID Alloc(SIZE_T dwBytes, LPCSTR szFile = __FILE__, int nLine = __LINE__);
	LPVOID ReAlloc(LPVOID lpMem, SIZE_T dwBytes, LPCSTR szFile = __FILE__, int nLine = __LINE__);
	void Free(LPVOID lpMem);

	/*

	// Operators used for allocation
	inline void* operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
	{
	return IW::Alloc(nSize, lpszFileName, nLine); 
	}

	inline void operator delete(void* lpMem, LPCSTR lpszFileName, int nLine) 
	{ 
	IW::Free(lpMem); 
	}

	*/

#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW


#else

	LPVOID Alloc(SIZE_T dwBytes);
	LPVOID ReAlloc(LPVOID lpMem, SIZE_T dwBytes);
	void Free(LPVOID lpMem);

#endif

}

//inline void * operator new(size_t nSize){  return IW::Alloc(nSize);}
//inline void * operator new[](size_t nSize){  return IW::Alloc(nSize);}
//inline void operator delete(void * ptr){  IW::Free(ptr);}
//inline void operator delete[](void * ptr){  IW::Free(ptr);}


namespace IW {


	inline LPTSTR StrDup(LPCTSTR sz)
	{
		if (sz == 0) return 0;

		int nLen = _tcslen(sz) + 1;
		LPTSTR szCopy = static_cast<LPTSTR>(Alloc(nLen * sizeof(TCHAR)));
		_tcscpy_s(szCopy, nLen, sz);
		return szCopy;
	}

	class Pool
	{
	protected:
		union HEADER {
			struct {
				HEADER* m_phdrPrev;
				SIZE_T  m_cb;
			};
			WCHAR alignment;
		};

	private:
		enum { MIN_CBCHUNK = 32000, MAX_ALLOC = 1024*1024 };

		HEADER* m_phdrCur;   // current block
		DWORD   m_dwGranularity;

	public:
		Pool() : m_phdrCur(NULL)
		{
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			m_dwGranularity = RoundUp(sizeof(HEADER) + MIN_CBCHUNK, si.dwAllocationGranularity);
		}

		~Pool()
		{
			clear();
		}

		void clear()
		{
			HEADER* phdr = m_phdrCur;
			while (phdr) {
				HEADER hdr = *phdr;
				VirtualFree(hdr.m_phdrPrev, hdr.m_cb, MEM_RELEASE);
				phdr = hdr.m_phdrPrev;
			}
		}

		static inline DWORD RoundUp(DWORD cb, DWORD units)
		{
			return ((cb + units - 1) / units) * units;
		}

		inline HEADER *Alloc(size_t cch)
		{
			static std::bad_alloc outOffMem;
			if (cch > MAX_ALLOC) throw(outOffMem);

			DWORD cbAlloc = RoundUp(cch * sizeof(TCHAR) + sizeof(HEADER), m_dwGranularity);
			BYTE* pbNext = reinterpret_cast<BYTE*>(VirtualAlloc(NULL, cbAlloc, MEM_COMMIT, PAGE_READWRITE));

			if (!pbNext) throw(outOffMem);

			HEADER* phdrCur = reinterpret_cast<HEADER*>(pbNext);
			phdrCur->m_phdrPrev = m_phdrCur;
			phdrCur->m_cb = cbAlloc;

			return m_phdrCur = phdrCur;
		}


	};

	class StringPool : public Pool
	{
	public:

		StringPool() : m_pchNext(NULL), m_pchLimit(NULL)
		{
		}


		LPCTSTR AllocString(LPCTSTR pszBegin, LPCTSTR pszEnd)
		{
			size_t cch = pszEnd - pszBegin + 1;
			LPTSTR psz = m_pchNext;

			if (m_pchNext + cch <= m_pchLimit) 
			{
				m_pchNext += cch;
				_tcsncpy_s(psz, cch, pszBegin, cch - 1);
				return psz;
			}

			HEADER* phdrCur = Alloc(cch);		
			m_pchNext = reinterpret_cast<TCHAR*>(phdrCur + 1);
			m_pchLimit = reinterpret_cast<TCHAR*>(reinterpret_cast<BYTE*>(phdrCur) + phdrCur->m_cb);

			return AllocString(pszBegin, pszEnd);
		}

		LPCTSTR LoadString(HINSTANCE hInstance, UINT nID)
		{
			const ATL::ATLSTRINGRESOURCEIMAGE* pStringImage = ATL::AtlGetStringResourceImage(hInstance, nID);
			if(pStringImage == NULL )
			{
				throw std::exception("Invalid string table entry");
			}

			USES_CONVERSION_EX;
			LPCTSTR sz = W2T_EX((LPWSTR)pStringImage->achString, pStringImage->nLength);
			return AllocString(sz, sz + pStringImage->nLength);
		}

	private:
		TCHAR*  m_pchNext;   // first available byte
		TCHAR*  m_pchLimit;  // one past last available byte
	};

}; // namespace IW