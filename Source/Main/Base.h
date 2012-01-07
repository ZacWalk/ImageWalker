///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2002 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// IWAPI.h
//
// Contains struct definitions for generic ImageWalker 
// objects used throughout the implementation. 
// 
//////////////////////////////////////////////////////////////////////

#pragma once



#define IWINTERFACECLASS interface
#define  countof(__x)   (sizeof(__x) / sizeof(__x[0])) 

class State;

namespace IW
{
	extern SIZE_T BlobMemoryUsage;
	
	// Types used by ImageWalker
	//typedef const BYTE far            *LPCBYTE;
	typedef const COLORREF            *LPCCOLORREF;    
	typedef const DWORD				  *LPCDWORD;

	typedef unsigned char	UInt8;
	typedef unsigned short	UInt16;
	typedef short	Int16;
	typedef unsigned long	UInt32;
	typedef signed long	Int32;
	
	template<typename T>
	class TRational 
	{
	public:
		TRational() : numerator(0), denominator(0)
		{
		}

		TRational(const TRational &other) : numerator(other.numerator), denominator(other.denominator)
		{
		}   

		void operator=(const float &other)
		{
			numerator = (T)(other * 1000.0f);
			denominator = 1000;
		}

		void operator=(const TRational &other)
		{
			numerator = other.numerator;
			denominator = other.denominator;
		}

		bool operator==(const TRational &other) const
		{
			return numerator == other.numerator &&
				denominator == other.denominator;
		}

		float ToFloat() const
		{
			return (float)numerator / (float)denominator;
		}

		T numerator; 
		T denominator;
	};

	typedef TRational<UInt32> Rational;
	typedef TRational<Int32> SRational;

	typedef struct
	{
		UINT r, g, b, c, a, ac;
	} 
	RGBSUM, *LPRGBSUM;

	typedef const RGBSUM *LPCRGBSUM; 


	// Pre-defines
	IWINTERFACECLASS IStatus;
	IWINTERFACECLASS IImageLoaderFactory;
	IWINTERFACECLASS IPropertyServer;
	IWINTERFACECLASS IPropertyArchive;
	IWINTERFACECLASS IPropertyStream;
	IWINTERFACECLASS IImageLoader;

	class Image;
	class Folder;
	class FolderItem;
	class FileSize;

	enum { LoadBufferSize = 1024 * 64 };


	////////////////////////////////////////////////////////////
	//
	// Referenced
	//
	// Base call for all objects that are referenced
	//
	IWINTERFACECLASS Referenced
	{
	private:
		mutable volatile long _nRef;

	public:
		inline Referenced() : _nRef(0) {}
		virtual ~Referenced() {};

		inline void AddRef() 
		{ 
			InterlockedIncrement(&_nRef); 
		}

		inline void Release()
		{
			if (InterlockedDecrement(&_nRef) == 0) 
			{
				// Now Delete
				delete this; 
			}	
		}
	};

	////////////////////////////////////////////////////////////
	//
	// Commands
	//
	////////////////////////////////////////////////////////////
	IWINTERFACECLASS ICommandUndo
	{
		virtual ~ICommandUndo() {};
		virtual void Undo() = 0;
	};

	IWINTERFACECLASS ICommand
	{
		virtual ~ICommand() {};
		virtual void Invoke() = 0;

		virtual bool IsChecked() const = 0;
		virtual bool IsEnabled() const = 0;
	};	

	////////////////////////////////////////////////////////////
	//
	// IStreamOut & IStreamIn
	//
	// Streams are used to stream in or out binary data. The normally 
	// represent a files on disk.
	//
	IWINTERFACECLASS IStreamCommon
	{
		virtual ~IStreamCommon() {}

		typedef enum tagPOSITION
		{ 
			eBegin = FILE_BEGIN, 
			eCurrent = FILE_CURRENT, 
			eEnd = FILE_END 
		} 
		ePosition;

		virtual DWORD Seek(ePosition ePos, LONG nOff) = 0;
		virtual FileSize GetFileSize() = 0;

		virtual CString GetFileName() = 0;

		virtual bool Abort() = 0;
		virtual bool Close(IStatus *pStatus) = 0;

	};

	IWINTERFACECLASS IStreamOut : public virtual IStreamCommon
	{
		virtual bool Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten = 0) = 0;
		virtual bool Flush() = 0;
	};

	IWINTERFACECLASS IStreamIn : public virtual IStreamCommon
	{
		virtual bool Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead = 0) = 0;	
	};
	

	////////////////////////////////////////////////////////////
	//
	// IStatus
	//
	// Used to allow recording of error, warnings and messages
	//
	IWINTERFACECLASS IStatus
	{
		virtual void SetMessage(const CString &strMessage) = 0;
		virtual void SetWarning(const CString &strWarning) = 0;
		virtual void SetError(const CString &strError) = 0;
		virtual void SetStatusMessage(const CString &strMessage) = 0;
		virtual void SetContext(const CString &strContext) = 0;
		virtual void Progress(int nCurrentStep, int TotalSteps) = 0;
		virtual bool QueryCancel() = 0;

		virtual void SetHighLevelProgress(int nCurrentStep, int TotalSteps) = 0;
		virtual void SetHighLevelStatusMessage(const CString &strMessage) = 0; 
	};

	////////////////////////////////////////////////////////////
	//
	// IPropertyArchive, IPropertyStream, IPropertyServer
	//
	// Property interfaces for serializing properties
	//

	// Property Flags
	enum 
	{ 
		ePF_ReadOnly = 0x01
	};


	IWINTERFACECLASS IPropertyArchive
	{
		virtual bool Write(const CString &strValueKey, DWORD dwValue) = 0;
		virtual bool Read(const CString &strValueKey, DWORD& dwValue) const = 0;

		virtual bool Write(const CString &strValueKey, int nValue) = 0;
		virtual bool Read(const CString &strValueKey, int& nValue) const = 0;

		virtual bool Write(const CString &strValueKey, long nValue) = 0;
		virtual bool Read(const CString &strValueKey, long& nValue) const = 0;

		virtual bool Write(const CString &strValueKey, bool bValue) = 0;
		virtual bool Read(const CString &strValueKey, bool& bValue) const = 0;

		virtual bool Read(const CString &strValueKey, CString &str) const = 0;
		virtual bool Write(const CString &strValueKey, LPCTSTR szValue) = 0;

		virtual bool Read(const CString &strValueKey, LPVOID pValue, DWORD &dwCount) const = 0;
		virtual bool Write(const CString &strValueKey, LPCVOID pValue, DWORD dwCount) = 0;

		virtual bool StartSection(const CString &strValueKey) const = 0;
		virtual bool EndSection() const = 0;

		virtual bool StartSection(const CString &strValueKey) = 0;
		virtual bool EndSection() = 0;

		virtual bool ParseText(const CString &strValueKey) = 0;
		virtual CString ToString() const = 0;

	};

	IWINTERFACECLASS IPropertyStream
	{
		virtual bool StartSection(const CString &strValueKey) = 0;
		virtual bool Property(const CString &strValueKey, const CString &strTitle, const CString &strDescription, const CString &strValue, DWORD dwFlags) = 0;
		virtual bool EndSection() = 0;	

		virtual bool Thumbnail(LPCBYTE pData, DWORD dwSize) = 0;
	};

	////////////////////////////////////////////////////////////
	//
	// ISerialize
	//
	// Factory struct that allows object
	// to be created or persisted
	//

	IWINTERFACECLASS ISerialize
	{
		// Serialization
		virtual void Read(const IPropertyArchive *pArchive) = 0;
		virtual void Write(IPropertyArchive *pArchive) const = 0;
	};

	//////////////////////////////////////////////////////////////////////////////////
	// Memory Routines
	inline void MemCopy( LPVOID lpMemOut, LPCVOID lpMemIn, DWORD dwSize )
	{
		CopyMemory(lpMemOut, lpMemIn, dwSize);
	}

	inline void MemSet( LPVOID lpMemInOut, DWORD dwData, DWORD dwSize )
	{
		FillMemory(lpMemInOut, dwSize, dwData);
	}

	inline void MemZero( LPVOID lpMemInOut, DWORD dwSize )
	{
		ZeroMemory(lpMemInOut, dwSize);
	}

	template<class T>
		inline T* ReferencePtrAssign(T** pp, T* lp)
	{
		if (lp != NULL)
			lp->AddRef();
		if (*pp)
			(*pp)->Release();
		*pp = lp;
		return lp;
	}

	/////////////////////////////////////////////////////////////// 
	// RefObj: Used to wrap objects that are reference counted
	//
	template<class T>
	class RefObj : public T
	{
	public:	

		template<class T1> RefObj(T1 &x1) : T(x1) {};
		template<class T1, class T2> RefObj(T1 &x1, T2 &x2) : T(x1, x2) {};
		template<class T1, class T2, class T3> RefObj(T1 &x1, T2 &x2, T3 &x3) : T(x1, x2, x3) {};

		RefObj() {};	

	protected:
		virtual ~RefObj() 
		{ 
		};	
	};

	/////////////////////////////////////////////////////////////// 
	// ScopeObj: Used to wrap objects that are not reference counted
	//
	template<class T>
	class ScopeObj : public T
	{

	public:	

		ScopeObj()
		{ 
			AddRef();
		};

		template<class T1> ScopeObj(T1 &x1) :  T(x1) 
		{
		};

		template<class T1, class T2> ScopeObj(T1 &x1, T2 &x2) :  T(x1, x2) 
		{
		};

		template<class T1, class T2, class T3> ScopeObj(T1 &x1, T2 &x2, T3 &x3) :  T(x1, x2, x3) 
		{
		};

		virtual ~ScopeObj() 
		{ 		
		};
	};



	/////////////////////////////////////////////////////////////// 
	// RefPtr: Smart pointer for referenced counted objects
	//
	template <class T>
	class RefPtr
	{
	public:
		T *p;

		RefPtr() : p(0)
		{		
		}

		RefPtr(T* pIn) : p(pIn)
		{
			if (p) p->AddRef();
		}

		RefPtr(const IW::RefPtr<T>& pIn) : p(pIn)
		{
			if (p) p->AddRef();
		}

		~RefPtr()
		{
			if (p) p->Release();
		}
		void Release()
		{
			T* pTemp = p;
			if (pTemp)
			{
				p = NULL;
				pTemp->Release();
			}
		}


		T **GetPtr()
		{
			return &p;
		}

		operator T*() const
		{
			return (T*)p;
		}
		T& operator*() const
		{
			assert(p != 0);
			return *p;
		}
		T* operator->() const
		{
			assert(p != 0);
			return p;
		}
		T* operator=(T* pIn) 
		{
			ReferencePtrAssign(&p, pIn);
			return p;
		}
		T* operator=(const IW::RefPtr<T>& pIn) 
		{
			ReferencePtrAssign(&p, pIn.p);
			return p;
		}
		void Copy(T* pIn)
		{
			ReferencePtrAssign(&p, pIn);
		}
		bool operator!() const
		{
			return (p == NULL);
		}
		bool operator<(T* pT) const
		{
			return p < pT;
		}
		bool operator==(T* pT) const
		{
			return p == pT;
		}
		void Attach(T* pIn)
		{
			if (p) p->Release();
			p = pIn;
		}
		T* Detach()
		{
			T* pOut = p;
			p = NULL;
			return pOut;
		}

		// Serialization
		bool Read(const CString &strValueName, const IPropertyArchive *pArchive)
		{
			if (p && pArchive->StartSection(szValueName))
			{
				p->Read(pArchive);	
				pArchive->EndSection();
			};

			return true;
		}

		bool Write(const CString &strValueName, IPropertyArchive *pArchive) const
		{
			if (p && pArchive->StartSection(szValueName))
			{
				p->Write(pArchive);
				pArchive->EndSection();
			}

			return true;
		}
	};

	/////////////////////////////////////////////////////////////// 
	// ConstRefPtr: Smart pointer for referenced counted objects that are constant
	//
	template <class T>
	class ConstRefPtr
	{
	public:
		const T *p;

		ConstRefPtr() : p(0)
		{		
		}

		ConstRefPtr(const T* pIn) : p(pIn)
		{
			if (p) const_cast<T*>(p)->AddRef();
		}

		ConstRefPtr(const IW::RefPtr<T>& pIn) : p(pIn)
		{
			if (p) const_cast<T*>(p)->AddRef();
		}

		~ConstRefPtr()
		{
			if (p) const_cast<T*>(p)->Release();
		}
		void Release()
		{
			const T* pTemp = p;
			if (pTemp)
			{
				p = NULL;
				const_cast<T*>(pTemp)->Release();
			}
		}

		const T **GetPtr()
		{
			return &p;
		}

		operator const T*() const
		{
			return (T*)p;
		}
		const T& operator*() const
		{
			assert(p != 0);
			return *p;
		}
		const T* operator->() const
		{
			assert(p != 0);
			return p;
		}
		const T* operator=(const T* pIn) 
		{
			ReferencePtrAssign(const_cast<T**>(&p), const_cast<T*>(pIn));
			return p;
		}
		const T* operator=(T* pIn) 
		{
			ReferencePtrAssign(const_cast<T**>(&p), pIn);
			return p;
		}
		const T* operator=(const IW::RefPtr<T>& pIn) 
		{
			ReferencePtrAssign(const_cast<T**>(&p), pIn.p);
			return p;
		}
		const T* operator=(const IW::ConstRefPtr<T>& pIn) 
		{
			ReferencePtrAssign(const_cast<T**>(&p), const_cast<T*>(pIn.p));
			return p;
		}
		bool operator!() const
		{
			return (p == NULL);
		}
		bool operator<(const T* pT) const
		{
			return p < pT;
		}
		bool operator==(const T* pT) const
		{
			return p == pT;
		}
		void Attach(const T* pIn)
		{
			if (p) const_cast<T*>(p)->Release();
			p = pIn;
		}
		const T* Detach()
		{
			T* pOut = p;
			p = NULL;
			return pOut;
		}
	};



	/////////////////////////////////////////////////////////////////////////////////////
	/// Inline guid

	inline bool InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
	{
		return (
			((PLONG) &rguid1)[0] == ((PLONG) &rguid2)[0] &&
			((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
			((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
			((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
	};

	//////////////////////////////////////////////////////////////////////////////////
	// Few template helpers

	template<class T> 
		inline void Swap(T &a, T &b) { T c; c = a; a = b; b = c; };

	//////////////////////////////////////////////////////////////////////////////////
	/// CAuto : Auto pointer to delete an object when it
	/// goes out of scope.

	template<class T>
	class CAutoFree
	{
	public:
		T *_p;

		CAutoFree() : _p(0)
		{
		}

		CAutoFree(int nCount) : _p(0)
		{
			_p = (T*)IW::Alloc(nCount * sizeof(T));
		}

		CAutoFree(T *pil) : _p(pil)
		{
		}

		T* operator->() const
		{
			assert(_p != 0);
			return _p;
		}

		template<typename TCast>
		TCast* Cast()
		{
			return reinterpret_cast<TCast*>(_p);
		}

	private:

		CAutoFree(CAutoFree<T> &)
		{
		}

		void operator=(CAutoFree<T> &)
		{
		}

	public:

		~CAutoFree()
		{
			IW::Free(_p);
			_p = 0;
		}

		void Free(T *p)
		{
			IW::Free(p);
		}


		void operator=(T *pil)
		{
			Free(_p);
			_p = pil;
		}

		bool operator==(const T *pTC) const
		{
			return _p == pTC;
		}

		bool operator!=(const T *pTC) const
		{
			return _p != pTC;
		}

		operator T*()
		{
			return _p;
		}

		operator const T*() const
		{
			return _p;
		}

		operator LPVOID()
		{
			return _p;
		}

		operator const LPCVOID() const
		{
			return _p;
		}

		T &operator [](unsigned n)
		{
			return _p[n];
		}

		const T &operator[](unsigned n) const
		{
			return _p[n];
		}	

		T &operator [](int n)
		{
			return _p[n];
		}

		const T &operator[](int n) const
		{
			return _p[n];
		}

		void Set(int n, const T value)
		{
			_p[n] = value;
		}
	};

	template<class T>
	class CScopeDelete
	{
	public:

		T *_p;

		CScopeDelete(T *p) : _p(p)
		{
		}

		~CScopeDelete()
		{
			if (_p != 0) delete _p;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////
	/// Blob : Managers a misc blob of data in memory
	typedef struct tagBLOBDATA 
	{
		DWORD dwSize;
		IAtlMemMgr *pMemMgr;
		volatile long nRef;
	} 
	BLOBDATA, *PBLOBDATA;

	template<class T>
	class Blob
	{
	public:

		typedef Blob<T> ThisClass;

		Blob() : _pBlob(0)
		{
		}

		Blob(T *p) : _pBlob(0)
		{
			Copy(p);
		}

		static IAtlMemMgr *GetMemMgr()
		{
			static CWin32Heap heap(0, 0);
			return &heap;
		}

		Blob(const ThisClass &other) : _pBlob(0)
		{
			Copy(other);
		}

		Blob(LPCVOID pData, int nLength) : _pBlob(0)
		{
			CopyData(pData, nLength);			
		}

		Blob &operator=(const ThisClass &other)
		{
			Copy(other);
			return *this;
		}

		void CopyData(LPCVOID pData, int nLength)
		{
			if (!empty()) Free();

			if (nLength > 0)
			{
				ReAlloc(nLength);
				IW::MemCopy(GetData(), pData, nLength);
			}
		}

		~Blob()
		{
			Free();
		}

		inline bool empty() const 
		{
			return _pBlob == 0;
		}

		inline void Free()
		{			
			Copy(static_cast<T*>(NULL));
		}

		inline void Copy(const ThisClass &other)
		{
			Copy(other._pBlob);
		}		

		inline LPCBYTE GetData() const { assert(_pBlob); return reinterpret_cast<LPCBYTE>(_pBlob) + sizeof(T); };
		inline LPBYTE GetData() { assert(_pBlob); return reinterpret_cast<LPBYTE>(_pBlob) + sizeof(T); };	
		
		inline bool IsEmpty() const { assert(_pBlob != (T*)0xfeeefeee); return _pBlob == NULL; };

		inline DWORD GetSize() const { return IsEmpty() ? 0 : _pBlob->dwSize; };
		inline DWORD GetDataSize() const { return IsEmpty() ? 0 : GetSize() - sizeof(T); };

		inline bool operator==(const ThisClass &other) const { return Equal(other._pBlob); };
		inline bool operator!=(const ThisClass &other) const { return !Equal(other._pBlob); };

		inline bool Equal(const T* pOther) const { return _pBlob == pOther; }

		// Persistance
		inline HRESULT LoadFromStream(IW::IStreamIn *pStream)
		{
			HRESULT hr = S_OK;
			ULONG uLengthRead = 0;

			IW::FileSize size = pStream->GetFileSize();

			if (size > 0)
			{
				ReAlloc(size);
				hr = pStream->Read(GetData(), size, &uLengthRead);
			}

			return hr;
		}

		inline T *Alloc(DWORD nSize)
		{
			T *pNew = _Alloc(nSize);
			Copy(pNew);
			return pNew;
		}

		inline T *ReAlloc(DWORD nSize)
		{
			T *pNew = _Alloc(nSize);

			if (_pBlob != 0)
			{
				const DWORD nNewMemSize = pNew->dwSize;
				const DWORD nCopySize = Min(_pBlob->dwSize, nNewMemSize);
				IW::MemCopy(pNew, _pBlob, nCopySize);
				pNew->nRef = 0;
				pNew->dwSize = nNewMemSize;
			}

			Copy(pNew);
			return pNew;
		}		

	protected:

		mutable T* _pBlob;

		inline static T *_Alloc(DWORD nSize)
		{
			const int nMemSize = nSize + sizeof(T);
			IAtlMemMgr *pMemMgr = GetMemMgr();
			
			LPBYTE p = static_cast<LPBYTE>(pMemMgr->Allocate(nMemSize));

			if (!p)
			{
				throw std::bad_alloc();
			}

			T *pT = reinterpret_cast<T*>(p);
			IW::MemZero(pT, sizeof(T));
			pT->dwSize = nMemSize;
			pT->pMemMgr = pMemMgr;

			BlobMemoryUsage += nMemSize;

			return pT;
		}			

		inline static void AddRef(T *p) 
		{ 
			if (p != 0)
			{
				InterlockedIncrement(&p->nRef); 
			}			
		}

		inline static void Release(T *p)
		{
			if (p != 0)
			{
				if (InterlockedDecrement(&p->nRef) == 0) 
				{
					BlobMemoryUsage -= p->dwSize;
					p->pMemMgr->Free(p);			
				}				
			}
		}

		inline void Copy(T *pNew)
		{
			T *pOld = _pBlob;			
			AddRef(pNew);
			_pBlob = pNew;
			Release(pOld);			
		}		
	};

	typedef Blob<BLOBDATA> SimpleBlob;

	class WindowPos
	{
	public:

		typedef std::vector<WINDOWPOS> POSLIST;
		POSLIST _listPositions;


		void SetWindowPos(HWND hwnd, HWND hwndInsertAfter, LPCRECT pRect, UINT flags = SWP_NOZORDER | SWP_NOACTIVATE)
		{
			SetWindowPos(
				hwnd, hwndInsertAfter, 
				pRect->left, pRect->top, 
				pRect->right - pRect->left, pRect->bottom - pRect->top, 
				flags);
		}

		void SetWindowPos(HWND hwnd, HWND hwndInsertAfter, int x, int y, int cx, int cy, UINT flags = SWP_NOZORDER | SWP_NOACTIVATE)
		{
			WINDOWPOS pos = { hwnd, hwndInsertAfter, x, y, cx, cy, flags };
			_listPositions.push_back(pos);
		}

		void Apply()
		{
			HDWP hdwp = BeginDeferWindowPos(_listPositions.size());	

			if (hdwp)
			{
				for(POSLIST::iterator i = _listPositions.begin(); i != _listPositions.end(); i++)
				{
					::DeferWindowPos(hdwp, i->hwnd, i->hwndInsertAfter, i->x, i->y, i->cx, i->cy, i->flags); 
				}

				::EndDeferWindowPos(hdwp);
			}
			else
			{
				for(POSLIST::iterator i = _listPositions.begin(); i != _listPositions.end(); i++)
				{
					::SetWindowPos(i->hwnd, i->hwndInsertAfter, i->x, i->y, i->cx, i->cy, i->flags); 
				}
			}
		}
	};

	inline CString IToStr(int nValue)
	{
		const int nStringBufferSize = 32;
		CString str;
		_itot_s(nValue, str.GetBuffer(nStringBufferSize), nStringBufferSize, 10);
		str.ReleaseBuffer();
		return str;
	}
	
	inline CString IToStr(unsigned long nValue)
	{
		const int nStringBufferSize = 32;
		CString str;
		_ultot_s(nValue, str.GetBuffer(nStringBufferSize), nStringBufferSize, 10);
		str.ReleaseBuffer();
		return str;
	}
	
	inline CString IToStr(long nValue)
	{
		const int nStringBufferSize = 32;
		CString str;
		_ltot_s(nValue, str.GetBuffer(nStringBufferSize), nStringBufferSize, 10);
		str.ReleaseBuffer();
		return str;
	}

}; // namespace IW

