///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
//
// CTwain.h - application interface to TWAIN Protocol
//
// Version 2.04  2004.01.22
// Copyright (C) 1998-2004 Dosadi.  All rights reserved.
// *** Licensed Commercial Version **
//
// See the file "manual.htm" for programming information
//
// web page: http://www.dosadi.com/ctwain.htm
// Support:  support@dosadi.com
// Sales:    sales@dosadi.com
//
///////////////////////////////////////////////////////////////////////
//
#pragma once 
#include "twain.h"


typedef enum {
	NO_TWAIN_STATE,			// 0	internal use only
	PRE_SESSION,			   // 1	ground state, nothing loaded
	SOURCE_MANAGER_LOADED,	// 2	DSM loaded but not open
	SOURCE_MANAGER_OPEN,	   // 3	DSM open
	SOURCE_OPEN,			   // 4	some Source open - Negotiation state!
	SOURCE_ENABLED,			// 5	acquisition started
	TRANSFER_READY,			// 6	data ready to transfer
	TRANSFERRING			   // 7	transfer started
} TW_STATE;

typedef enum {
	TWERR_OPEN_DSM,			// unable to load or open Source Manager
	TWERR_OPEN_SOURCE,		// unable to open Datasource
	TWERR_ENABLE_SOURCE,	   // unable to enable Datasource
	TWERR_NOT_4,            // capability set outside state 4 (SOURCE_OPEN)
	TWERR_CAP_SET,          // capability set failed
} TW_ERR;

class CTwain
{
public:
	CTwain(void);
	~CTwain();

	// General notes:
	// 1. Once the source manager has been loaded, it is not normally unloaded
	// until this object is destroyed, or until you call UnloadSourceManager.
	// This makes normal TWAIN operations quite a bit faster.  Calling TwainAvailable
	// the first time loads the source manager.

	// attributes i.e. query functions

	enum { VERSION = 204 };
	// Major Rev * 100 + Minor Rev, access as CTwain::VERSION

	int TwainAvailable(void);
	// TRUE if the TWAIN Datasource Manager is available and can
	// be loaded.  Does not check that there are any datasources!
	// But normally, the presence of the DSM means that at least one
	// datasource has been installed.
	// IsAvailable is fast after the first call - you can use it
	// to enable or disable menu items for example.

	TW_STATE State(void);
	// Return the current (presumed) state of the Twain connection.

	unsigned ResultCode(void);
	// last result code (see twain.h)

	unsigned ConditionCode(void);
	// retrieve condition code from last triplet - see twain.h.

	// Top-level operations
	void SetDefWindow(HWND hwnd);
	// Sets the CWnd* to be used as the default for all other calls that
	// take a window argument - SelectSource, OpenSourceManager, etc.
	// If no default window is set, AfxGetMainWnd() is used.

	typedef enum { XFER_NATIVE, XFER_FILE, XFER_MEMORY } XFERMODE;
	void SetXferMode(XFERMODE xm) { xferMode = xm; }
	XFERMODE GetXferMode(void) const { return xferMode; }
	// Select one of the three TWAIN transfer modes.
	// Each native transfer will call DibReceived with a DIB handle
	// Each memory transfer will call MemXferReady, MemXferData, MemXferDone
	// Each file transfer will call FileXferReady, FileXferDone

	int SelectSource(HWND hwnd = NULL);
	// Post the standard Select Source dialog

	void ModalAcquire(HWND hwnd = NULL);
	// Acquire images from current or default source.
	// Start acquisition from current or default datasource.
	// If a source is open, uses that source - otherwise the
	// default source is opened.
	// By default, displays the u/i of the source and acquires images
	// until a CLOSEDSREQ is received from the source.
	// Acquire returns the Twain connection to the starting state
	// or to SOURCE_ENABLED, whichever is lower.

	int BeginAcquire(HWND hwnd = NULL);
	// Open and enable the default source.
	// TRUE if successful, FALSE if something goes wrong.
	// If successful, State() == SOURCE_ENABLED.
	// You must now pass all messages to MessageHook until the state
	// drops below SOURCE_OPEN: In MFC, override PreTranslateMessage.
	// If BeginAcquire fails, it returns TWAIN to the state it
	// found it in.

	void RegisterApp(						// Record application information
		int		nMajorNum, int nMinorNum,	// major and incremental revision of application. E.g.
		// for version 2.1, nMajorNum == 2 and nMinorNum == 1
		int		nLanguage,					// language of this version (use TWLG_xxx from TWAIN.H)
		int		nCountry,					// country of this version (use TWCY_xxx from TWAIN.H)
		LPSTR	lpszVersion,				// version info string e.g. "1.0b3 Beta release"
		LPSTR	lpszMfg,					// name of manufacturer/developer e.g. "Crazbat Software"
		LPSTR	lpszFamily,					// product family e.g. "BitStomper"
		LPSTR	lpszProduct);				// specific product e.g. "BitStomper Deluxe Pro"
	// This is not necessary for acquisition, but is required for full
	// TWAIN compliance.

	// Level 2 operations - more detailed control of Twain State.
	int OpenSourceManager(HWND hwnd = NULL);
	// (Load and) open the Datasource Manager
	// Loads the DSM if necessary, opens it if necessary.
	// If State >= 3, does nothing and returns TRUE.

	int OpenDefaultSource(void);
	// Open the default datasource (last source selected in Select Source dialog.)
	// Invalid (returns FALSE) if State > 3 (a source is already open)
	// Invalid (returns FALSE) if State < 3 (DSM is not open)
	// If successful, returns TRUE with State == 4.
	// Otherwise, returns FALSE with State unchanged.

	int OpenSource(LPCSTR pzSourceName);
	// Open a specific named source.
	// Note: The name must match the "product name" of the source exactly,
	// as listed in the Select Source dialog
	// (without the trailing (32-32) that appears on some systems.)
	// If successful, returns TRUE with State == 4.
	// Otherwise, returns FALSE with State unchanged.

	void SetShowUI(BOOL bShow = 1);
	// Set flag for whether source should be enabled with
	// user interface visible (bShow == TRUE) or not.
	// At construction, ShowUI is set TRUE.

	BOOL GetShowUI(void);
	// Return state of ShowUI flag.

	//--------- Capability Negotiation ----------------------------

	int SetCurrentResolution(double dRes);
	int SetCurrentPixelType(int nType);

	int SetBrightness(double dBri);
	// Note: Valid range is -1000.0 to +1000.0
	int SetContrast(double dCon);
	// Note: Valid range is -1000.0 to +1000.0

	int SetXferCount(int nXfers);

	int SetCurrentUnits(unsigned uUnits);
	unsigned GetCurrentUnits(void);
	// Set/Get current unit of measure (TWUN_INCHES, ... - see twain.h)
	// Several TWAIN functions take measurements - these are always
	// interpreted in the current unit of measure.  See e.g. SetImageLayout.

	int SetCapFix32(unsigned Cap, double dVal);
	int SetCapOneValue(unsigned Cap, TW_FIX32 ItemVal);
	int SetCapOneValue(unsigned Cap, unsigned ItemType, TW_UINT32 ItemVal);

	int SetImageLayout(double left, double top, double width, double height);
	// Set the region of the next image to be transferred.
	// Only valid in state 4 (SOURCE_OPEN)
	// All measurements are in the current units (see GetCurrentUnits)
	// This is a suggestion to the source, it does not have to comply.
	int GetImageLayout(double &left, double &top, double &width, double &height);
	// Get the current image layout.
	// Only valid in states >= 4 (SOURCE_OPEN & above)
	// All measurements are in the current units.

	BOOL GetCapCurrent(unsigned Cap, short& v);
	BOOL GetCapCurrent(unsigned Cap, unsigned short& v);
	BOOL GetCapCurrent(unsigned Cap, int& v);
	BOOL GetCapCurrent(unsigned Cap, unsigned& v);
	BOOL GetCapCurrent(unsigned Cap, double& v);
	BOOL GetCapCurrent(unsigned Cap, unsigned ItemType, void FAR *pVal);

	TW_FIX32 static ToFix32(double r);
	// Convert a float value to TWAIN's 32-bit fixed point format

	double static Fix32ToFloat(TW_FIX32 fix);
	// Convert a TWAIN fixed-point value to floating point.

	//--------- State Change Operations --------------------------

	int EnableSource(HWND hwnd = NULL);
	// Enable the open source, which allows image acquisition to begin.
	// Invalid if State != 4 (source open).
	// If successful, returns TRUE with State == 5.
	// Otherwise, returns FALSE with State unchanged.

	int DisableSource(void);
	// Disable the current source.
	// If State == 5, disables the current source and returns TRUE.
	// If State < 5, does nothing and returns TRUE.
	// If State > 5, does nothing and returns FALSE.

	int CloseSource(void);
	int CloseSourceManager(HWND hwnd = NULL);
	int UnloadSourceManager(void);

	int DropToState(int nS, HWND hwnd = NULL);

	// low-level primitives
	int LoadSourceManager(void);
	// loads the DSM (Datasource Manager) into process space

	void ModalEventLoop(void);
	// get and dispatch messages until source is disabled.

	int TwainMessageHook(LPMSG lpmsg);

	int EndXfer(void);
	// In State 7, ends the current transfer by sending MSG_ENDXFER.
	// If successful, goes to State 6 if there are more transfers
	// available (pendingXfers != 0), or to State 5 if not.
	// Returns TRUE if the resulting State < 7.

	int CancelXfers(void);
	// In State 6, cancels any pending transfers.
	// (In State 7, does an EndXfer first)
	// If successful, goes to State 5.
	// Returns TRUE if the resulting State < 6.

	// bottom-level primitives
	int DS(unsigned long dg, unsigned dat, unsigned msg, void FAR *pd);
	// send a triplet to the current Datasource.
	// returns TRUE if the result code (rc) == RC_SUCCESS, FALSE otherwise.
	// Note that this is not meaningful with some triplets.
	// Does ASSERT(nState < 4);

	int SM(unsigned long dg, unsigned dat, unsigned msg, void FAR *pd);
	// Send a triplet to the Source Manager.
	// returns TRUE if the result code (rc) == RC_SUCCESS, FALSE otherwise.
	// Note that this is not meaningful with some triplets.
	// Does ASSERT(nState > 1);

	//-------- call-backs
	virtual int NegotiateCapabilities(void);
	// called after source is successfully opened.
	// Use this call-back to negotiate any special settings
	// (capabilities) for the session.  Return TRUE if successful.
	// The default method just returns TRUE.

	virtual void XferReady(LPMSG lpmsg);
	// called when source has one or more xfers ready.
	// Calls one of these three methods once per image:
	virtual BOOL FileXfer(void);
	virtual BOOL MemoryXfer(void);
	virtual BOOL NativeXfer(void);

	virtual void CloseDsRequest(void);
	// called when the open source asks to be 'closed'
	// It is sufficient to disable the source on this request,
	// but this default handler calls CloseSource.

	virtual void DibReceived(HGLOBAL hDib);
	// called by the default XferReady handler
	// when it has successfully transferred a DIB.
	// This default handler just calls GlobalFree(hDib).

	//-------- Memory Mode Transfer call-backs
	//
	virtual BOOL MemXferReady(TW_IMAGEINFO info);
	// Called when a memory transfer is ready to start.
	// info is the TWAIN block that contains the image parameters.
	// Return TRUE to continue, FALSE to abort the transfer.

	virtual BOOL MemXferBuffer(int iRow, TW_IMAGEMEMXFER& mx);
	// Called when a buffer has been received.
	// The first row in this buffer is row 'iRow' of the image.
	// mx is the TWAIN structure that describes the data block.
	// Return TRUE to continue, FALSE to abort the transfer.
	// By default, calls MemXferRow with each row.

	virtual BOOL MemXferRow(int iRow, LPBYTE prow, TW_IMAGEMEMXFER& mx);
	// Called by MemXferBuffer for each row of each memory buffer.
	// iRow is the row number in the image.
	// prow points to the row data in the buffer.
	// mx is the TWAIN structure describing the current buffer.
	// Return TRUE to continue, FALSE to abort the transfer.

	virtual void MemXferDone(BOOL bSuccess);
	// Called after a memory transfer has ended.
	// bSuccess tells whether the transfer completed successfully.
	// MemXferDone will always be called exactly once after each
	// successful call to MemXferReady.

	virtual BOOL MemXferAlloc(TW_SETUPMEMXFER twsx, TW_MEMORY& mem);
	virtual void MemXferFree(TW_MEMORY& mem);
	// These two functions are called back to allocate and deallocate
	// the working memory buffer during a memory transfer.
	// By default they provide a buffer of the optimum size via malloc/free.

	//-------- File Mode Transfer call-backs
	//
	virtual BOOL FileXferReady(int& nFileFormat, char* pszFilename);
	// called by default XferReady when a file transfer is ready to start.
	// Parameters are the default file format and file name (with path).
	// Override this function to specify the file format or name/path.
	// The format must be a TWFF_* value, and Source must support it.
	virtual void FileXferDone(const char* pszFilename, int nBufferSize);
	// called by default XferReady when file transfer is done.
	// If non-null, pszFilename points to the full path of the image file.
	// If null, the transfer failed or was cancelled.

	virtual void StateChange(int nState);
	// called after each Twain State transition.
	// nState is the new State.  When this callback
	// occurs, the state transition has already happened.
	// Note - first call is the transition to State 1
	// which occurs at construction.

	virtual void TwainError(TW_ERR e);
	// Called when an unexpected TWAIN malfunction occurs.
	// See TW_ERR declaration at beginning of this file.

	BOOL Tracing(void) { return bTrace; }
	void Trace(BOOL bOn = TRUE) { bTrace = bOn; }

private:

	void SetState(TW_STATE nS);				// assume Twain in State nS

	HWND DefWnd(HWND hwnd = NULL);
	// Returns pWnd if it's non-null, otherwise it
	// finds a safe non-null default substitute.

	void FlushMessageQueue(void);

	BOOL              bTrace;        // enable TRACE output
	TW_STATE          nState;        // current state
	TW_STATE          nStartState;   // starting state for some operation
	TW_IDENTITY       AppId;         // application identity structure
	TW_INT16          rc;            // last result code       
	TW_USERINTERFACE  twUI;
	BOOL              bShowUI;
	TW_PENDINGXFERS   pendingXfers;
	HANDLE            hDib;          // bitmap returned by native transfer
	HWND			  hwndDef;       // default window
	XFERMODE          xferMode;      // requested transfer mode
};


// Some useful helpers
double ppm2dpi(long ppm);
// converts pels/meter (ppm) to dots/inch (dpi)

long dpi2ppm(double dpi);
// converts dpi to ppm


template<class TEvents>
class CTwainImpl :  public CTwain
{
protected:
	bool _bInitTwain;

public:
	// Event Sink
	TEvents *m_pEvents;
	IW::Image _image;


	CTwainImpl(TEvents *pEvents) : m_pEvents(pEvents)
	{
		_bInitTwain = false;
	}

	~CTwainImpl()
	{
		CloseTwain();
	}



	void InitTwain()
	{
		// Try to load TWAIN now
		if (!_bInitTwain)
		{
			HWND hWnd = m_pEvents->m_hWnd;

			// Do we have twain?
			if (TwainAvailable()) 
			{
				SetDefWindow(hWnd);
				if (!OpenSourceManager(hWnd)) 
				{
					// OpenSourceManager does not report errors
					TwainError(TWERR_OPEN_DSM);
				}
				else
				{
					_bInitTwain = true;
				}

			}
		}
	}

	void CloseTwain()
	{
		// Close twain
		if (_bInitTwain)
		{
			CloseSourceManager();
			UnloadSourceManager();
		}

		_bInitTwain = false;
	}

	void OnFileAcquireNative(LPCTSTR szTwainName = 0)
	{
		CWaitCursor wait;

		InitTwain();

		if (_bInitTwain)
		{
			HWND hWnd = IW::GetMainWindow();

			SetXferMode(XFER_FILE);
			SetShowUI(TRUE);
			BeginAcquire(hWnd);
		}
	}

	void OnFileAcquire(LPCTSTR szTwainName = 0)
	{
		CWaitCursor wait;

		InitTwain();

		if (_bInitTwain)
		{
			HWND hWnd = IW::GetMainWindow();

			SetXferMode(XFER_MEMORY);
			SetShowUI(TRUE);
			BeginAcquire(hWnd);
		}
	}

	void OnFileSelectsource()
	{
		CWaitCursor wait;

		InitTwain();

		if (_bInitTwain)
		{
			SelectSource();
		}
	}

	void DibReceived(HGLOBAL hdib)
	{
		ATLASSERT(hdib);

		if (!hdib)
			return;

		IW::Image dib;
		dib.Copy(hdib);	

		m_pEvents->_folder.SaveNewImage(dib);
		CTwain::DibReceived(hdib);
	}

	void TwainError(TW_ERR e)
	{
		CString strError;

		switch(e)
		{
		case TWERR_OPEN_DSM:
			strError.LoadString(IDS_TWAIN_ERROR_OPEN_DSM);
			break;
		case TWERR_OPEN_SOURCE:
			strError.LoadString(IDS_TWAIN_ERROR_OPEN_SOURCE);
			break;
		case TWERR_ENABLE_SOURCE:

			break;
		case TWERR_NOT_4:
			strError.LoadString(IDS_TWAIN_ERROR_NOT_4);
			break;
		case TWERR_CAP_SET:
			strError.LoadString(IDS_TWAIN_ERROR_CAP_SET);
			break;
		}

		CString str;
		str.Format(IDS_TWAIN_ERROR_FMT, strError);

		IW::CMessageBoxIndirect mb;
		mb.Show(str);
	} // TwainError


	int NegotiateCapabilities(void)
	{

		return TRUE;
	}


	BOOL FileXferReady(int& nFileFormat, char* szInOut, int nBufferSize)
	{
		USES_CONVERSION;

		TCHAR sz[MAX_PATH];
		_tcscpy_s(sz, MAX_PATH, A2T(szInOut));

		// May need to set a temp name
		if (_tcsclen(sz) == 0)
		{
			LPCTSTR szExtension = _T("jpg");

			switch(nFileFormat)
			{
			case TWFF_TIFF:
			case TWFF_TIFFMULTI:
				szExtension = _T("tiff");
				break;
			case TWFF_PICT:
				szExtension = _T("pict");
				break;
			case TWFF_BMP:
				szExtension = _T("bmp");
				break;
			case TWFF_XBM:
				szExtension = _T("xbm");
				break;
			case TWFF_JFIF:
				szExtension = _T("jpg");
				break;
			case TWFF_FPX :
				szExtension = _T("fpx");
				break;
			case TWFF_PNG:
				szExtension = _T("png");
				break;
			case TWFF_SPIFF:
				szExtension = _T("spiff");
				break;
			case TWFF_EXIF:
				szExtension = _T("exif");
				break;
			default:
				szExtension = _T("???");
				break;
			}

			IW::CFilePath path;
			path.GetTempFilePath();
			path.SetExtension(szExtension);
			path.CopyTo(szInOut, nBufferSize);
		}

		return TRUE;
	}


	void FileXferDone(const char* pszFilename, int nBufferSize)
	{
		USES_CONVERSION;

		if (pszFilename && m_pEvents) 
		{
			MoveTempFileToCurrentFolder(CA2T(pszFilename));
		}
		else
		{
			// Null some times
			// I dont know why
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_TWAIN_ERROR);
		}

	} // FileXferDone

	bool MoveTempFileToCurrentFolder(LPCTSTR szTempFile)
	{
		// We move the file to our
		// Local folder
		CString str;
		CString strName;

		LPCTSTR szNewImage = App.LoadString(IDS_NEW_IMAGE);

		LPCTSTR szExt = IW::Path::FindExtension(szTempFile);
		str.Format(_T("%s\\%s*"), m_pEvents->GetFolderPath(), szNewImage);

		WIN32_FIND_DATA FileData; 
		int nImageNum = 0;

		HANDLE hSearch = FindFirstFile(str, &FileData);

		if (hSearch != INVALID_HANDLE_VALUE) 
		{
			do
			{
				int n = _ttoi(FileData.cFileName + 9);

				if (n >= nImageNum)
					nImageNum = n+1;
			}
			while(FindNextFile(hSearch, &FileData));

			// Close the search handle. 
			FindClose(hSearch);
		}

		if (nImageNum)
		{
			str.Format(_T("%s\\%s %d%s"), m_pEvents->GetFolderPath(), szNewImage, nImageNum, szExt);
			strName.Format(_T("%s %d%s"), szNewImage, nImageNum, szExt);
		}
		else
		{
			str.Format(_T("%s\\%s%s"), m_pEvents->GetFolderPath(), szNewImage, szExt);
			strName.Format(_T("%s%s"), szNewImage, szExt);
		}

		MoveFile(szTempFile, str);
		return true;
	}




	/////////////////////////////////////////////////////////////////////////////
	// Memory Transfer call-backs

	BOOL MemXferReady(TW_IMAGEINFO info)
	{
		if (info.Planar != 0) 
		{
			// We don't accept planar pixels (red plane, green plane, ...)
			return FALSE;
		}
		if (info.Compression != TWCP_NONE) 
		{
			// We can't accept compressed data
			return FALSE;
		}
		if (info.ImageWidth == -1 || info.ImageLength == -1) 
		{
			// We can't handle unknown image size
			return FALSE;
		}

		IW::PixelFormat pf(IW::PixelFormat::PF24);

		switch (info.BitsPerPixel) 
		{
		case 24:
			if (info.SamplesPerPixel != 3) 
			{
				return FALSE;
			}
			pf = IW::PixelFormat::PF24;
			break;
		case 8:
			if (info.PixelType != TWPT_GRAY) 
			{
				return FALSE;
			}
			if (info.SamplesPerPixel != 1) 
			{
				return FALSE;
			}
			pf = IW::PixelFormat::PF8GrayScale;
			break;
		case 1:
			if (info.PixelType != TWPT_BW) 
			{
				return FALSE;
			}
			if (info.SamplesPerPixel != 1) 
			{
				return FALSE;
			}
			pf = IW::PixelFormat::PF1;
			break;
		default:
			return FALSE;
		} // switch


		_image.SetXPelsPerMeter(IW::InchToMeter(Fix32ToFloat(info.XResolution)));
		_image.SetYPelsPerMeter(IW::InchToMeter(Fix32ToFloat(info.YResolution)));

		_image.CreatePage(info.ImageWidth, info.ImageLength, pf);

		return TRUE;

	} // MemXferReady


	BOOL MemXferRow(int nRow, LPBYTE pLineIn, TW_IMAGEMEMXFER& mx)
	{		
		if (!_image.IsEmpty())
		{
			IW::Page pageOut = _image.GetFirstPage();

			if (nRow <= pageOut.GetHeight())
			{
				LPBYTE pLineOut = pageOut.GetBitmapLine(nRow);
				const int nWidth = pageOut.GetWidth();
				const int dwStorageWidth = IW::CalcStorageWidth(nWidth, pageOut.GetPixelFormat());

				if (pageOut.GetPixelFormat()._pf != IW::PixelFormat::PF24) 
				{
					IW::MemCopy(pLineOut, pLineIn, dwStorageWidth);
				} 
				else
				{
					IW::ConvertRGBtoBGR(pLineOut, pLineIn, nWidth);
				}
			}
		}

		return TRUE;
	} // MemXferRow


	void MemXferDone(BOOL bSuccess)
	{
		if (bSuccess) 
		{
			m_pEvents->_folder.SaveNewImage(_image);
		}

		_image.Free();

	} // MemXferDone
};