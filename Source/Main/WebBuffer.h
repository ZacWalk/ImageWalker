// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once




// This class implements a queued buffer data structure.

// The WRITER of the buffer string data creates the data structure with a buffer size
//    then repeatedly calls WriteString or << operator to add null-terminated strings to the buffer.
//    The class transparently handles the point where the writer needs to write past the
//    end of the buffer, at which point the class creates a new block of the buffer.
//    The WRITER must call Done() when the final data has been written

// The READER of the class subscribes to data availability via SetDataAvailableCallback.
//    Whenever a new block is ready, this class informs the reader via this callback.
// The reader can pull data from the buffer at any time using ReadData, which will
// coalesce available blocks to service the data request

// The point of this class is to eliminate buffer resizing and eliminate buffer copies
// All work is done directly on the original buffer allocated.
// Buffer blocks have a maximum size but will usually be less than the maximum size
// (they are wrapped when the first call to RequestBufferSize runs past the buffer)

//#define DEBUG_SHOW_BUFFER_CONTENTS - Use to spit out contents of buffer during debugging

typedef struct SizedBufferBlock
{
	BYTE* pData;
	ULONG dwSize;
} SizedBuffer;


class CPlugProtocol;


// DATA-AVAILABLE-CALLBACK: dwContext, dwCharsInBufferBlock
typedef HRESULT (*DATAAVAILABLECBACK)(CPlugProtocol*, ULONG);

class WebBuffer : public virtual IW::Referenced
{
// Construction/Deconstruction
public:
	WebBuffer(ULONG dwMaxBuff = 1024) : m_dwMaxBufferSize(dwMaxBuff)
	{
		_ASSERTE(m_dwMaxBufferSize > 0);

		m_bDone = false;
		m_pCallback = NULL;
		m_pCallbackThis = NULL;
		m_cbCurWriteBlockSize = 0;
		m_cbReadBlockSize = 0;
		m_pWriteBlock = NULL;
		m_pReadBlock = NULL;
		m_pCurReadSpot = NULL;
		m_bDeleteWhenWriterDone = false;
		m_bCanDelete = false;
		m_sxnBufferBlocks.Init();
	}

	~WebBuffer()
	{
		Free();

		SizedBufferBlock* block;
#ifdef _DEBUG
	if (!m_bufferblocks.empty() || m_pWriteBlock || m_pReadBlock)
#endif
		while(!m_bufferblocks.empty())
		{
			block = m_bufferblocks.front();
			delete block->pData;
			delete block;
			m_bufferblocks.pop();
		}
		if (m_pWriteBlock)
		{
			delete[] m_pWriteBlock;
		}
		if (m_pReadBlock)
		{
			delete[] m_pReadBlock;
		}
	}

// Methods
public:


	inline void Write(const CString &strW)
	{
		if (strW.IsEmpty())
			return;
		
		CStringA str = strW;
		Write((LPCVOID)(LPCSTR)str, str.GetLength()); 
	}

	//inline void WriteString(const TCHAR* strWriteMe, ULONG dwNumChars)
	//{
	//	if (NULL == strWriteMe || _tcsclen(strWriteMe) == 0)
	//		return;
	//	
	//	CStringA str(strWriteMe, dwNumChars);
	//	Write(str, str.GetLength()); 
	//}

	inline void Write(LPCVOID pWriteMe, ULONG dwSize)
	{
		if (NULL == pWriteMe || 0 == dwSize)
			return;

		ULONG dwNumBytes = dwSize;
		void* pWriteHere = RequestBufferSize(dwNumBytes); 
		IW::MemCopy(pWriteHere, pWriteMe, dwNumBytes);
	}

	// Note: Since the writer decides the max-size, if you request a bigger size than the buffer
	// we ignore max-size and create a buffer of that size (simplifies code)
	inline BYTE* RequestBufferSize(ULONG dwSize)
	{
		_ASSERTE(0 != dwSize);
		_ASSERTE(!m_bDone); // DON'T REUSE ME

		ULONG dwStartWritePlace;
		// if space is available in open buffer, return place in current buffer block
		if ((m_cbCurWriteBlockSize + dwSize <= m_dwMaxBufferSize) && (NULL != m_pWriteBlock))
		{
			dwStartWritePlace = m_cbCurWriteBlockSize;
			m_cbCurWriteBlockSize += dwSize;
		}
		// else, need new buffer block
		else
		{	
			PushBufferBlock(false); 
			(dwSize <= m_dwMaxBufferSize) ?	NewBufferBlock() : NewBufferBlock(dwSize);
			dwStartWritePlace = 0;
			m_cbCurWriteBlockSize = dwSize;
		}

		_ASSERTE(NULL != m_pWriteBlock);
		return &(m_pWriteBlock[dwStartWritePlace]);
	}

	inline void Done()
	{
		_ASSERTE(true != m_bDone);

		PushBufferBlock(true);
		
		bool bCanDelete = false;	
		m_sxnBufferBlocks.Lock();
		{
			if (!m_bDeleteWhenWriterDone)
				m_bCanDelete = true;
			else
				bCanDelete = true;
		}
		m_sxnBufferBlocks.Unlock();
	}

	inline void Free()
	{
		bool bCanDelete = false;	
		m_sxnBufferBlocks.Lock();
		{
			bCanDelete= m_bCanDelete;
			if (!m_bCanDelete)
				m_bDeleteWhenWriterDone = true;
			else
				bCanDelete = true;
		}
		m_sxnBufferBlocks.Unlock();
	}

	inline bool IsDone()
	{
		return m_bDone;
	}

	inline HRESULT ReadData(void* pv, ULONG cb, ULONG* pcbRead)
	{
		bool bDoneReading = false;
		bool bEndOfData = false;
		BYTE* pvOut = (BYTE*)pv;
		ULONG cbRemaining = cb;
		ULONG cbDataAvail;

		*pcbRead = 0;
		while (!bDoneReading)
		{
			cbDataAvail = (ULONG)((m_pReadBlock + m_cbReadBlockSize) - m_pCurReadSpot);
			// Are we done with the last read buffer?
			if (0 == cbDataAvail)
			{
				DeleteReadBlock();

				bEndOfData = NextReadBlock();

				// We have no data left to read and we are done
				if (bEndOfData) 
					return S_FALSE;
				// still waiting on write buffer to get data
				else if (NULL == m_pReadBlock)
					if (0 != *pcbRead)
						return S_OK;
					else
						return E_PENDING;

				cbDataAvail = m_cbReadBlockSize;
			}

			// Less data than is available has been requested
			if (cbRemaining < cbDataAvail)
				cbDataAvail = cbRemaining;

			IW::MemCopy(pvOut, m_pCurReadSpot, cbDataAvail);			
			m_pCurReadSpot += cbDataAvail;
			*pcbRead += cbDataAvail;
			pvOut += cbDataAvail;
			cbRemaining -= cbDataAvail;

			if (*pcbRead == cb)
				bDoneReading = true;
		}

		return S_OK;
	}

	void SetDataAvailableCallback(DATAAVAILABLECBACK pCallback, CPlugProtocol* pCallbackThis);

// Implementation
private:
	CComCriticalSection m_sxnBufferBlocks;

	DATAAVAILABLECBACK m_pCallback;				// {Writing}
	CPlugProtocol* m_pCallbackThis;			// {Writing}
	IW::RefPtr<IInternetProtocol> m_pRefHolder;

	ULONG m_dwMaxBufferSize;		// {Writing}
	ULONG m_cbCurWriteBlockSize;	// {Writing}
	BYTE* m_pWriteBlock;			// {Writing}

	std::queue<SizedBuffer*> m_bufferblocks;	// {BOTH - must be protected by crit section}

	BYTE* m_pReadBlock;				// {Reading}
	ULONG m_cbReadBlockSize;		// {Reading}
	BYTE* m_pCurReadSpot;		// {Reading}

	bool m_bDone;					// Set when writer has reported they are done with the buffer
	bool m_bDeleteWhenWriterDone;	// Reader finished with object before writer is done
	bool m_bCanDelete;				// Set when object is ready for deletion

	inline void NewBufferBlock(ULONG dwBufferSize = 0)
	{
		if (NULL != m_pWriteBlock)
		{
			// ASSERTION: The only time NewBufferBlock should be called is when the
			//            buffer is not big enough, in which case we just discard it
			delete[] m_pWriteBlock;
			m_pWriteBlock = NULL;
		}

		if (0 == dwBufferSize)
			dwBufferSize = m_dwMaxBufferSize;
		
		m_cbCurWriteBlockSize = 0;
		m_pWriteBlock = new BYTE[dwBufferSize];
	}
		
	inline void PushBufferBlock(bool bLastBlock)
	{
		SizedBufferBlock* pBlock = NULL;
		ULONG cbDataAvail = 0;

		#ifdef _DEBUG
		#ifdef DEBUG_SHOW_BUFFER_CONTENTS
		OutputDebugString((LPCTSTR)m_pWriteBlock);
		#endif
		#endif

		m_sxnBufferBlocks.Lock();
		{
			if (m_bDeleteWhenWriterDone)
			{
				m_sxnBufferBlocks.Unlock();
				return;
			}

			if (NULL != m_pWriteBlock)
			{
				cbDataAvail = m_cbCurWriteBlockSize;
				pBlock = new SizedBuffer;
				pBlock->pData = m_pWriteBlock;
				pBlock->dwSize = m_cbCurWriteBlockSize;

				m_bufferblocks.push(pBlock);

				m_pWriteBlock = NULL;
				m_cbCurWriteBlockSize = 0;
			}
			m_bDone = bLastBlock;
		}
		m_sxnBufferBlocks.Unlock();

		ReportDataAvailable(cbDataAvail);

	}

	// If current buffer block is empty, get a new one from queue
	// Return true if we're all done reading everything available
	inline bool NextReadBlock()
	{
		bool bNoMoreData = false;
		ULONG nBufferBlocks;
		SizedBufferBlock* pPoppedBlock;

		// lock before checking sharing case
		m_sxnBufferBlocks.Lock();
		{
			nBufferBlocks = (ULONG)m_bufferblocks.size();
		
			if (0 != nBufferBlocks)
			{
				// pull data block off buffer
				pPoppedBlock = m_bufferblocks.front();
				m_pCurReadSpot = m_pReadBlock = pPoppedBlock->pData;
				m_cbReadBlockSize = pPoppedBlock->dwSize;
				m_bufferblocks.pop();
				delete pPoppedBlock;
			}
			// m_bDone check needs to be guarded by bufferblocks section
			else if (m_bDone)
				bNoMoreData = true;
		}
		m_sxnBufferBlocks.Unlock();

		#ifdef _DEBUG
		#ifdef DEBUG_SHOW_BUFFER_CONTENTS
		if (m_pReadBlock)
			OutputDebugString((LPCTSTR)m_pReadBlock);
		else
		if (m_pReadBlock)
			OutputDebugString((LPCTSTR)m_pCurReadSpot);
		else
		#endif
		#endif

		return bNoMoreData;
	}

	inline void DeleteReadBlock()
	{
		delete m_pReadBlock;
		m_pReadBlock = NULL;
		m_pCurReadSpot = NULL;
		m_cbReadBlockSize = 0;
	}

	inline void WebBuffer::ReportDataAvailable(ULONG cbDataAvail)
	{
		if (NULL != m_pCallback)
		{
			(m_pCallback)(m_pCallbackThis, cbDataAvail);
		}
	}
};
