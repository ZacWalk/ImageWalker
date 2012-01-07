///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// ctwain.cpp - implementation of application i/f to TWAIN
//
// Version 2.03  2001.03.20
// Copyright (C) 1998-2001 Dosadi.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TwainImpl.h"


#ifdef _WIN32
#define DSM_FILENAME "TWAIN_32.DLL"
#define DSM_ENTRYPOINT "DSM_Entry"
#else
#define DSM_FILENAME "TWAIN.DLL"
#define DSM_ENTRYPOINT "DSM_ENTRY"
#endif

#define IsValidHandle(h) ((h) != NULL)

#define ATLTRACE_ERROR(e) { if (bTrace) ATLTRACE(_T("TWAIN: Error(%d)\n"), e); TwainError(e); }

static int			iAvailable;       // TWAIN available: 0:unknown, -1:No, 1:Yes
static DSMENTRYPROC	pSM_Entry;        // entry point of Data Source Manager (TWAIN.DLL)
static HINSTANCE	hSMLib;           // handle of SM

// default application identity structure:
const TW_IDENTITY	DefAppId = {
	0,									// Id, filled in by SM
	{ 1, 0, TWLG_USA, TWCY_USA, ""},	// Version
	TWON_PROTOCOLMAJOR,
	TWON_PROTOCOLMINOR,
	DG_IMAGE | DG_CONTROL,
	"Dosadi",							// Mfg
	"CTwain application",				// Family
	"TWAIN Application"					// Product
};
static TW_IDENTITY	SourceId;			// source identity structure


// Misc. helper stuff
double ppm2dpi(long ppm)
// converts pels/meter (ppm) to dots/inch (dpi)
{
	return 0.0254 * ppm;
}

long dpi2ppm(double dpi)
// converts dpi to ppm
{
	return (long)(dpi / 0.0254 + 0.5);
}


// CTwain class members
CTwain::CTwain(void) : hwndDef(NULL), nState(NO_TWAIN_STATE), xferMode(XFER_NATIVE)
{
	bShowUI = TRUE;
	hDib = NULL;
	rc = TWRC_SUCCESS;
	// turn off if you get tired of seeing the output:
	bTrace = TRUE;

	AppId = DefAppId;			// provide default registration
	SetState(PRE_SESSION);
}


void CTwain::RegisterApp(	// record application information
						 int   nMajorNum, int nMinorNum,	// major and incremental revision of application. E.g.
						 // for version 2.1, nMajorNum == 2 and nMinorNum == 1
						 int   nLanguage,                 // language of this version (use TWLG_xxx from TWAIN.H)
						 int   nCountry,                  // country of this version (use TWCY_xxx from TWAIN.H)
						 LPSTR	lpszVersion,               // version info string e.g. "1.0b3 Beta release"
						 LPSTR	lpszMfg,                   // name of manufacturer/developer e.g. "Crazbat Software"
						 LPSTR	lpszFamily,                // product family e.g. "BitStomper"
						 LPSTR	lpszProduct)               // specific product e.g. "BitStomper Deluxe Pro"
{
	AppId.Id = 0;						// init to 0, but Source Manager will assign real value
	AppId.Version.MajorNum = (TW_UINT16)nMajorNum;
	AppId.Version.MinorNum = (TW_UINT16)nMinorNum;
	AppId.Version.Language = (TW_UINT16)nLanguage;
	AppId.Version.Country  = (TW_UINT16)nCountry;
	strcpy_s(AppId.Version.Info, 32, lpszVersion);

	AppId.ProtocolMajor =    TWON_PROTOCOLMAJOR;
	AppId.ProtocolMinor =    TWON_PROTOCOLMINOR;
	AppId.SupportedGroups =  DG_IMAGE | DG_CONTROL;
	strcpy_s(AppId.Manufacturer, 32, lpszMfg);
	strcpy_s(AppId.ProductFamily, 32, lpszFamily);
	strcpy_s(AppId.ProductName, 32, lpszProduct);

}

CTwain::~CTwain()
{
	// shut down the Twain connection
	CloseSourceManager();
	UnloadSourceManager();
}

int CTwain::TwainAvailable(void)
{
	if (pSM_Entry) return TRUE;		// SM currently loaded

	if (iAvailable == 0) {
		if (LoadSourceManager()) {
			iAvailable = 1;
		} else {
			iAvailable = -1;
		}
	}
	return (iAvailable > 0);
}


TW_STATE CTwain::State(void)
{
	return nState;
}


void CTwain::SetState(TW_STATE nS)
{
	if (nState != nS) {
#ifdef _DEBUG
		if (bTrace) {
			LPCTSTR pzState[] = {
				g_szEmptyString,
				_T("1(PRE_SESSION)"),
				_T("2(SOURCE_MANAGER_LOADED)"),
				_T("3(SOURCE_MANAGER_OPEN)"),
				_T("4(SOURCE_OPEN)"),
				_T("5(SOURCE_ENABLED)"),
				_T("6(TRANSFER_READY)"),
				_T("7(TRANSFERRING)")
			};

			ATLTRACE(_T("TWAIN:State %s -> %s\n"), pzState[nState], pzState[nS]);
		}
#endif

		nState = nS;				// update the 'global'
		StateChange(nS);			// notify derived classes via callback
	}
} // SetState


void CTwain::SetDefWindow(HWND hwnd)
// Sets the HWND to be used as the default for all other calls that
// take a window argument - SelectSource, OpenSourceManager, etc.
// If no default window is set, AfxGetMainWnd() is used.
{
	hwndDef = hwnd;
}


HWND CTwain::DefWnd(HWND hwnd)
// Returns hwnd if it's non-null, otherwise it
// finds a safe non-null default substitute.
{
	return hwnd ? hwnd : hwndDef ? hwndDef : NULL;
} // DefWnd


int CTwain::SelectSource(HWND hwnd)
{
	TW_IDENTITY		NewSourceId;
	TW_STATE		nStartState = State();
	int				bSuccess = FALSE;

	hwnd = DefWnd(hwnd);
	if (!OpenSourceManager(hwnd)) {
		ATLTRACE_ERROR(TWERR_OPEN_DSM);
		//"Unable to load & open TWAIN Source Manager");
	} else {

		// I will settle for the system default.  Shouldn't I get a highlight
		// on system default without this call?
		SM(DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &NewSourceId);
		// now do the real thing
		bSuccess = SM(DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &NewSourceId);
	}

	DropToState(nStartState, hwnd);
	return bSuccess;
}

void CTwain::ModalAcquire(HWND hwnd)
{
	hwnd = DefWnd(hwnd);
	if (BeginAcquire(hwnd)) {
		::EnableWindow(hwnd, FALSE);
		// source is enabled, wait for transfer or source closed
		ModalEventLoop();
		::EnableWindow(hwnd, TRUE);
		DropToState(nStartState);
	} else {
		// BeginAcquire puts everything back when it fails
	}
}


int CTwain::BeginAcquire(HWND hwnd)
{
	nStartState = State();
	hwnd = DefWnd(hwnd);

	if (State() >= SOURCE_MANAGER_OPEN || OpenSourceManager(hwnd)) {
		if (State() >= SOURCE_OPEN || (OpenDefaultSource() && NegotiateCapabilities())) {
			if (State() >= SOURCE_ENABLED || EnableSource(hwnd)) {
				return TRUE;
			} else {
				ATLTRACE_ERROR(TWERR_ENABLE_SOURCE);
			}
		} else {
			ATLTRACE_ERROR(TWERR_OPEN_SOURCE);
		}
	} else {
		ATLTRACE_ERROR(TWERR_OPEN_DSM);
	}
	DropToState(nStartState);
	return FALSE;
} // BeginAcquire


int CTwain::LoadSourceManager(void)
{
	USES_CONVERSION;
	
	OFSTRUCT	of;

	if (nState >= SOURCE_MANAGER_LOADED) {
		return TRUE;			// SM already loaded
	}

	TCHAR szSM[MAX_PATH];
	GetWindowsDirectory(szSM, MAX_PATH);

	if (szSM[_tcsclen(szSM)-1] != '\\') 
	{
		_tcscat_s(szSM, MAX_PATH, _T("\\"));
	}

	_tcscat_s(szSM, MAX_PATH, _T(DSM_FILENAME));			// could crash!

	if (OpenFile(CT2CA(szSM), &of, OF_EXIST) != -1) 
	{
		hSMLib = LoadLibrary(szSM);
	} 
	else 
	{
		if (bTrace) ATLTRACE(_T("TWAIN:LoadLibrary(%s) failed\n"), szSM);
		hSMLib = NULL;
	}

	if (IsValidHandle(hSMLib)) {
		pSM_Entry = (DSMENTRYPROC) GetProcAddress(hSMLib, DSM_ENTRYPOINT);
		if (pSM_Entry) {
			iAvailable = 1;
			SetState(SOURCE_MANAGER_LOADED);
		} else {
			if (bTrace) ATLTRACE(_T("TWAIN:GetProcAddress() failed!!\n"));
			FreeLibrary(hSMLib);
			hSMLib = NULL;
		}
	} else {
		pSM_Entry = NULL;
	}

	if (nState != SOURCE_MANAGER_LOADED && bTrace) ATLTRACE(_T("TWAIN:LoadSourceManager() failed.\n"));
	return (nState >= SOURCE_MANAGER_LOADED);
} // LoadSourceManager


int CTwain::OpenSourceManager(HWND hwnd)
{
	TW_INT32 hwnd32 = (TW_INT32)(int)(DefWnd(hwnd));

	if (LoadSourceManager()) {
		SM(DG_CONTROL, DAT_PARENT, MSG_OPENDSM, &hwnd32);
		if (nState != SOURCE_MANAGER_OPEN && bTrace) ATLTRACE(_T("TWAIN:OPENDSM failed.\n"));
	}
	return (nState >= SOURCE_MANAGER_OPEN);
}


int CTwain::OpenDefaultSource(void)
{
	if (nState != SOURCE_MANAGER_OPEN) return FALSE;

	// open the system default source
	SourceId.ProductName[0] = '\0';
	SourceId.Id = 0;
	SM(DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &SourceId);
	if (nState != SOURCE_OPEN && bTrace) ATLTRACE(_T("TWAIN:OPENDS failed.\n"));

	return (nState == SOURCE_OPEN);
} // OpenDefaultSource


int CTwain::OpenSource(LPCSTR pzSourceName)
{
	if (nState != SOURCE_MANAGER_OPEN) return FALSE;
	BOOL bFound = FALSE;
	// Prevent TWAIN (or some DS?) from clobbering the current dir.
	// Per David Offen 3/7/00
	TCHAR szDir[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, szDir);
	// Look for matching DS
	unsigned uMsg = MSG_GETFIRST;
	while (memset(&SourceId, 0, sizeof SourceId) &&
		SM(DG_CONTROL, DAT_IDENTITY, uMsg, &SourceId)) {
			if (_stricmp(SourceId.ProductName, pzSourceName) == 0) {
				// bingo
				bFound = TRUE;
				break;
			}
			uMsg = MSG_GETNEXT;
		} // while
		if (bFound) {
			SM(DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &SourceId);
			if (nState != SOURCE_OPEN && bTrace) ATLTRACE(_T("TWAIN:OPENDS failed.\n"));
		} else if (bTrace) {
			USES_CONVERSION;
			ATLTRACE(_T("CTwain::OpenSource - source named '%s' not found.\n"), CA2T(pzSourceName));
		}
		SetCurrentDirectory(szDir);
		return (nState == SOURCE_OPEN);
} // OpenSource


void CTwain::SetShowUI(BOOL bShow)
// Set flag for whether source should be enabled with
// user interface visible (bShow == TRUE) or not.
// At construction, ShowUI is set TRUE.
{
	bShowUI = bShow;
} // SetShowUI


BOOL CTwain::GetShowUI(void)
// Return state of ShowUI flag.
{
	return bShowUI;
} // GetShowUI


int CTwain::EnableSource(HWND hwnd)
{
	hwnd = DefWnd(hwnd);

	if (nState != SOURCE_OPEN) 
	{
		if (bTrace) ATLTRACE(_T("TWAIN:**WARNING** EnableSource() in wrong state\n")); 
		return FALSE;
	}

	SetCapOneValue(ICAP_XFERMECH, TWTY_UINT16, xferMode);

	twUI.ShowUI = (TW_BOOL)bShowUI;
	twUI.hParent = (TW_HANDLE)(hwnd);
	DS(DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, &twUI);
	if (nState != SOURCE_ENABLED && bTrace) ATLTRACE(_T("TWAIN:EnableSource failed.\n"));
	// note, source will set twUI.ModalUI.
	return (nState == SOURCE_ENABLED);
} // TWAIN_EnableSource


int CTwain::DisableSource(void)
{
	::BringWindowToTop((HWND)twUI.hParent);
	if (nState == SOURCE_ENABLED) {
		DS(DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, &twUI);
		if (nState == SOURCE_ENABLED && bTrace) {
			ATLTRACE(_T("TWAIN:DisableSource failed.\n"));
		}
	}
	return (nState < SOURCE_ENABLED);
} // DisableSource


int CTwain::CloseSource(void)
{
	rc = TWRC_SUCCESS;

	if (nState == SOURCE_ENABLED) {
		DisableSource();
	}
	if (nState == SOURCE_OPEN) {
		DS(DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, &SourceId);
		if (nState == SOURCE_OPEN && bTrace) ATLTRACE(_T("TWAIN:CloseSource failed.\n"));
	}
	return (nState < SOURCE_OPEN);
} // CloseSource


int CTwain::CloseSourceManager(HWND hwnd)
{
	CloseSource();			// close source if open

	if (nState >= SOURCE_MANAGER_OPEN) {
		TW_INT32 hwnd32 = (TW_INT32)(int)(DefWnd(hwnd));
		SM(DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &hwnd32);
		if (nState >= SOURCE_MANAGER_OPEN && bTrace) ATLTRACE(_T("TWAIN:CLOSEDSM failed.\n"));
	}
	return (nState < SOURCE_MANAGER_OPEN);
}


int CTwain::UnloadSourceManager(void)
{
	rc = TWRC_SUCCESS;

	if (nState == SOURCE_MANAGER_LOADED) {
		if (hSMLib) {
			FreeLibrary(hSMLib);
			hSMLib = NULL;
		}
		pSM_Entry = NULL;
		SetState(PRE_SESSION);
	}
	return (nState == PRE_SESSION);
} // UnloadSourceManager


int CTwain::EndXfer(void)
{
	if (nState == TRANSFERRING) {
		DS(DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &pendingXfers);
		if (nState == TRANSFERRING && bTrace) ATLTRACE(_T("TWAIN:EndXfer failed.\n"));
	} else {
		if (bTrace) ATLTRACE(_T("TWAIN:**WARNING** EndXfer in wrong state\n")); 
	}
	return nState < TRANSFERRING;
} // EndXfer


int CTwain::CancelXfers(void)
{
	EndXfer();			// if transferring, cancel it

	if (nState == TRANSFER_READY) {
		DS(DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, &pendingXfers);
		if (nState == TRANSFER_READY && bTrace) ATLTRACE(_T("TWAIN:CancelXfers failed.\n"));
	}
	return (nState < TRANSFER_READY);
} // CancelXfers


int CTwain::DropToState(int nS, HWND hwnd)
{
	hwnd = DefWnd(hwnd);
	while (nState > nS) {
		switch (nState) {
			case TRANSFERRING:
				if (!EndXfer()) return FALSE;
				break;
			case TRANSFER_READY:
				if (!CancelXfers()) return FALSE;
				break;
			case SOURCE_ENABLED:
				if (!DisableSource()) return FALSE;
				break;
			case SOURCE_OPEN:
				if (!CloseSource()) return FALSE;
				break;
			case SOURCE_MANAGER_OPEN:
				if (!CloseSourceManager(hwnd)) return FALSE;
				break;
			case SOURCE_MANAGER_LOADED:
				if (!UnloadSourceManager()) return FALSE;
				break;
			default:
				ATLASSERT(FALSE);
		} // switch
	} // while
	return TRUE;
}


void CTwain::ModalEventLoop(void)
{
	MSG msg;

	while ((nState >= SOURCE_ENABLED) && GetMessage((LPMSG)&msg, NULL, 0, 0)) {
		if (!TwainMessageHook ((LPMSG)&msg)) {
			TranslateMessage ((LPMSG)&msg);
			DispatchMessage ((LPMSG)&msg);
		}
	} // while
} // ModalEventLoop


int CTwain::TwainMessageHook(LPMSG lpmsg)
// returns TRUE if message processed by TWAIN
// FALSE otherwise
{
	int		bProcessed = FALSE;

	if (nState >= SOURCE_ENABLED) {
		// source enabled
		TW_EVENT	twEvent;
		twEvent.pEvent = (TW_MEMREF)lpmsg;
		twEvent.TWMessage = MSG_NULL;
		// relay message to source in case it wants it
		DS(DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &twEvent);
		bProcessed = (rc == TWRC_DSEVENT);
		switch (twEvent.TWMessage) {
			case MSG_XFERREADY:
				// notify by callback
				// default callback does transfers
				XferReady(lpmsg);
				break;

			case MSG_CLOSEDSREQ:
				// notify by callback
				// default callback closes the source
				CloseDsRequest();
				break;

			case MSG_NULL:
				// no message returned from DS
				break;
		} // switch
	}

	return bProcessed;
} // TwainMessageHook


void CTwain::FlushMessageQueue(void)
{
	MSG msg;
	while ((nState >= SOURCE_ENABLED) && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!TwainMessageHook ((LPMSG)&msg)) {
			TranslateMessage ((LPMSG)&msg);
			DispatchMessage ((LPMSG)&msg);
		}
	} // while
} // FlushMessageQueue


BOOL CTwain::FileXferReady(int& nFileFormat, char* pszFilename)
{
	USES_CONVERSION;
	ATLTRACE(_T("CTwain::FileXferReady(%d, %s)\n"), nFileFormat, CA2T(pszFilename));
	return TRUE;
} // FileXferReady


void CTwain::FileXferDone(const char* pszFilename, int nBufferSize)
{
	USES_CONVERSION;
	ATLTRACE(_T("CTwain::FileXferDone(%s)\n"), CA2T(pszFilename));
} // FileXferDone


BOOL CTwain::FileXfer(void)
{
	TW_SETUPFILEXFER twsfx;
	if (!DS(DG_CONTROL, DAT_SETUPFILEXFER, MSG_GET, &twsfx)) {
		return FALSE;
	}
	int nFF = twsfx.Format;
	if (!FileXferReady(nFF, twsfx.FileName)) {
		// application aborted transfer
		return FALSE;
	}
	twsfx.Format = (TW_UINT16)nFF;
	if (!DS(DG_CONTROL, DAT_SETUPFILEXFER, MSG_SET, &twsfx)) {
		FileXferDone(NULL, 0);
		return FALSE;
	}
	if (!DS(DG_IMAGE, DAT_IMAGEFILEXFER, MSG_GET, NULL)) {
		FileXferDone(NULL, 0);
		return FALSE;
	}

	ATLASSERT(nState >= TRANSFER_READY);
	EndXfer();				// acknowledge & end transfer
	ATLASSERT(nState == TRANSFER_READY || nState == SOURCE_ENABLED);
	FlushMessageQueue();

	FileXferDone(twsfx.FileName, 255);
	return TRUE;
}


///////////////////////////////////////////////////////////////////////
//
// Memory Transfer
//
///////////////////////////////////////////////////////////////////////

BOOL CTwain::MemXferReady(TW_IMAGEINFO info)
{
	ATLTRACE(_T("CTwain::MemXferReady()\nWidth=%d  Height=%d\nRes %f x %f\n%d bits/pixel, %d smples/pixel\n"),
		info.ImageWidth, info.ImageLength,
		Fix32ToFloat(info.XResolution), Fix32ToFloat(info.YResolution)),
		info.BitsPerPixel, info.SamplesPerPixel;
	return TRUE;
}


BOOL CTwain::MemXferAlloc(TW_SETUPMEMXFER twsx, TW_MEMORY& mem)
{
	// Allocate memory buffer
	mem.TheMem = (LPVOID)malloc(twsx.Preferred);
	if (!mem.TheMem) {
		return FALSE;
	}
	mem.Length = twsx.Preferred;
	mem.Flags = TWMF_APPOWNS | TWMF_POINTER;
	return TRUE;
} // MemXferAlloc


void CTwain::MemXferFree(TW_MEMORY& mem)
{
	free(mem.TheMem);
} // MemXferFree


BOOL CTwain::MemXferBuffer(int iRow, TW_IMAGEMEMXFER& mx)
{
	ATLTRACE(_T("CTwain::MemXferBuffer  %d rows\n"), mx.Rows);
	LPBYTE prow = (LPBYTE)mx.Memory.TheMem;
	ATLASSERT(prow);
	for (TW_UINT32 i = 0; i < mx.Rows; i++) {
		MemXferRow(iRow, prow, mx);
		iRow++;
		prow += mx.BytesPerRow;
	}
	return TRUE;
}

BOOL CTwain::MemXferRow(int iRow, LPBYTE prow, TW_IMAGEMEMXFER& mx)
{
	return TRUE;
} // MemXferRow


void CTwain::MemXferDone(BOOL bSuccess)
{
	ATLTRACE(_T("CTwain::MemXferDone(%s)\n"), bSuccess ? _T("TRUE") : _T("FALSE"));
}


BOOL CTwain::MemoryXfer(void)
{
	ATLASSERT(nState == TRANSFER_READY);
	TW_IMAGEINFO    info;
	TW_SETUPMEMXFER twsx;
	TW_IMAGEMEMXFER mx;

	if (!DS(DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, &twsx)) {
		return FALSE;
	}

	// Get actual image info
	if (!DS(DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &info)) {
		return FALSE;
	}

	ATLASSERT(nState == TRANSFER_READY);
	if (!MemXferReady(info)) {
		// application aborted transfer
		return FALSE;
	}

	if (!MemXferAlloc(twsx, mx.Memory)) {
		MemXferDone(FALSE);
		return FALSE;
	}

	int iRow = 0;
	while (DS(DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, &mx)) {
		// We got a buffer - make sure DS doesn't
		// feed us rows past the end of the image:
		mx.Rows = min(mx.Rows, (unsigned)(info.ImageLength - iRow));
		// Don't call the buffer-handler with 0 rows
		if (mx.Rows != 0) {
			if (!MemXferBuffer(iRow, mx)) {
				// Buffer callback says to abort
				break;
			}
			iRow += mx.Rows;
		}
		if (ResultCode() == TWRC_XFERDONE) {
			// DS says that was the last buffer:
			iRow = info.ImageLength;        // no matter what, we're done
			break;
		}
	}

	MemXferFree(mx.Memory);

	ATLASSERT(nState >= TRANSFER_READY);
	EndXfer();				// acknowledge & end transfer
	ATLASSERT(nState == TRANSFER_READY || nState == SOURCE_ENABLED);
	FlushMessageQueue();

	MemXferDone(iRow == info.ImageLength);
	return TRUE;
}


BOOL CTwain::NativeXfer(void)
{
	TW_UINT32		hNative;
	// Get next native image 
	DS(DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hNative);

	if (TWRC_XFERDONE != rc) {
		// transfer failed for some reason or another
		hNative = NULL;
		if (bTrace) ATLTRACE(_T("TWAIN:NativeXfer failed.\n"));
	}
	ATLASSERT(nState >= TRANSFER_READY);
	EndXfer();				// acknowledge & end transfer
	ATLASSERT(nState == TRANSFER_READY || nState == SOURCE_ENABLED);
	FlushMessageQueue();
	if (hNative) {
		// call back with DIB
		if (bTrace) ATLTRACE(_T("TWAIN: calling DibReceived...\n"));
		DibReceived((HGLOBAL)hNative);
	}
	return (hNative != NULL);
} // NativeXfer


void CTwain::XferReady(LPMSG lpmsg)
// Default transfer-ready handler
// Based on last selected XferMode, calls
// back a transfer handler repeatedly
// until there are no more images pending.
{
	ATLASSERT(nState == TRANSFER_READY);

	do {
		switch (xferMode) {
		case XFER_NATIVE:
			NativeXfer();
			break;
		case XFER_FILE:
			FileXfer();
			break;
		case XFER_MEMORY:
			MemoryXfer();
			break;
		default:
			ATLASSERT(!"corrupted transfer mode");
			break;
		} // switch
		if (nState > TRANSFER_READY) {
			// Something went wrong during transfer, cancel/abort
			DropToState(SOURCE_ENABLED);
			break;
		}
	} while (nState == TRANSFER_READY);
}

void CTwain::CloseDsRequest(void)
{
	CloseSource();
}


int CTwain::NegotiateCapabilities(void)
// called after source is successfully opened.
// Use this call-back to negotiate any special settings
// (capabilities) for the session.  Default does nothing.
{
	return TRUE;
}

void CTwain::StateChange(int nState)
{
}

void CTwain::TwainError(TW_ERR e)
{
}


void CTwain::DibReceived(HGLOBAL hDib)
{
	GlobalFree(hDib);
}


///////////////////////////////////////////////////////////////////
// Capability Negotiation

int CTwain::SetCurrentResolution(double dRes)
// Negotiate the current resolution for acquisition.
// Negotiation is only allowed in State 4 (TWAIN_SOURCE_OPEN)
// The source may select this resolution, but don't assume it will.
{
	return SetCapFix32(ICAP_XRESOLUTION, dRes);
} // SetCurrentResolution


int CTwain::SetCurrentPixelType(int nType)
{
	return SetCapOneValue(ICAP_PIXELTYPE, TWTY_UINT16, (TW_UINT16)nType);
} // SetCurrentPixelType


int CTwain::SetXferCount(int nXfers)
{
	return SetCapOneValue(CAP_XFERCOUNT, TWTY_INT16, nXfers);
} // SetXferCount


int CTwain::SetBrightness(double dBri)
{
	return SetCapFix32(ICAP_BRIGHTNESS, dBri);
} // SetBrightness


int CTwain::SetContrast(double dCon)
{
	return SetCapFix32(ICAP_CONTRAST, dCon);
} // SetContrast


int CTwain::SetCurrentUnits(unsigned uUnits)
{
	return SetCapOneValue(ICAP_UNITS, TWTY_UINT16, uUnits);
} // SetCurrentUnits


unsigned CTwain::GetCurrentUnits(void)
{
	unsigned short usUnits = TWUN_INCHES;
	GetCapCurrent(ICAP_UNITS, usUnits);
	return usUnits;
} // GetCurrentUnits


int CTwain::SetImageLayout(double left, double top, double width, double height)
{
	if (nState != SOURCE_OPEN) {
		ATLTRACE_ERROR(TWERR_NOT_4);
		return FALSE;
	}

	TW_IMAGELAYOUT layout;
	layout.Frame.Left = ToFix32(left);
	layout.Frame.Top = ToFix32(top);
	layout.Frame.Right = ToFix32(left + width);
	layout.Frame.Bottom = ToFix32(top + height);
	// I hope these are ignored:
	layout.DocumentNumber = 0;
	layout.PageNumber = 0;
	layout.FrameNumber = 0;
	return DS(DG_IMAGE, DAT_IMAGELAYOUT, MSG_SET, &layout);
} // SetImageLayout


int CTwain::GetImageLayout(double &left, double &top, double &width, double &height)
{
	TW_IMAGELAYOUT layout;
	if (DS(DG_IMAGE, DAT_IMAGELAYOUT, MSG_GET, &layout)) {
		left = Fix32ToFloat(layout.Frame.Left);
		top = Fix32ToFloat(layout.Frame.Top);
		width = Fix32ToFloat(layout.Frame.Right) - left;
		height = Fix32ToFloat(layout.Frame.Bottom) - top;
		return TRUE;
	} else {
		left = top = width = height = 0.0;
		return FALSE;
	}
} // SetImageLayout

//----- Capability Helper Functions

int CTwain::SetCapFix32(unsigned Cap, double dVal)
{
	return SetCapOneValue(Cap, ToFix32(dVal));
} // SetCapFix32


TW_FIX32 CTwain::ToFix32(double r)
{
	TW_FIX32 fix;
	TW_INT32 val = (TW_INT32)(r * 65536.0 + (r < 0 ? -0.5 : 0.5));
	fix.Whole = (TW_INT16)(val >> 16);			// most significant 16 bits
	fix.Frac = (TW_UINT16)(val & 0xffff);		// least
	return fix;
} // ToFix32


double CTwain::Fix32ToFloat(TW_FIX32 fix)
{
	TW_INT32 val = ((TW_INT32)fix.Whole << 16) | ((TW_UINT32)fix.Frac & 0xffff);
	return val / 65536.0;
} // Fix32ToFloat


int CTwain::SetCapOneValue(unsigned Cap, TW_FIX32 ItemVal)
{
	return SetCapOneValue(Cap, TWTY_FIX32, *(TW_UINT32*)&ItemVal);
} // SetCapOneValue


int CTwain::SetCapOneValue(unsigned Cap, unsigned ItemType, TW_UINT32 ItemVal)
{
	TW_CAPABILITY	cap;
	pTW_ONEVALUE	pv;
	BOOL			bSuccess;

	if (nState != SOURCE_OPEN) {
		ATLTRACE_ERROR(TWERR_NOT_4);
		return FALSE;
	}

	cap.Cap = (TW_UINT16)Cap;		    // capability id
	cap.ConType = TWON_ONEVALUE;		// container type
	do 
	{
		cap.hContainer = GlobalAlloc(GHND, sizeof (TW_ONEVALUE));

		if (!cap.hContainer) 
		{
			if (IDCANCEL == MessageBox(NULL, _T("Internal error while preparing for TWAIN acquire."), _T("ImageWalker"), MB_RETRYCANCEL | MB_ICONEXCLAMATION)) 
			{
				return FALSE;
			}
		}
	} while (!cap.hContainer);
	pv = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
	pv->ItemType = (TW_UINT16)ItemType;
	pv->Item = ItemVal;
	GlobalUnlock(cap.hContainer);
	bSuccess = DS(DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&cap);
	GlobalFree(cap.hContainer);
	return bSuccess;
} // SetCapOneValue


BOOL CTwain::GetCapCurrent(unsigned Cap, short& sVal)
{
	return GetCapCurrent(Cap, TWTY_INT16, &sVal);
}


BOOL CTwain::GetCapCurrent(unsigned Cap, int& nVal)
{
	return GetCapCurrent(Cap, TWTY_INT32, &nVal);
}


BOOL CTwain::GetCapCurrent(unsigned Cap, unsigned short &wVal)
{
	return GetCapCurrent(Cap, TWTY_UINT16, &wVal);
}


BOOL CTwain::GetCapCurrent(unsigned Cap, unsigned &uVal)
{
	return GetCapCurrent(Cap, TWTY_UINT32, &uVal);
}


BOOL CTwain::GetCapCurrent(unsigned Cap, double& dVal)
{
	dVal = 0.0;
	TW_FIX32 fix;
	if (GetCapCurrent(Cap, TWTY_FIX32, &fix)) {
		dVal = Fix32ToFloat(fix);
		return TRUE;
	} else {
		return FALSE;
	}
}


const size_t nTypeSize[13] =
{	sizeof (TW_INT8),
sizeof (TW_INT16),
sizeof (TW_INT32),
sizeof (TW_UINT8),
sizeof (TW_UINT16),
sizeof (TW_UINT32),
sizeof (TW_BOOL),
sizeof (TW_FIX32),
sizeof (TW_FRAME),
sizeof (TW_STR32),
sizeof (TW_STR64),
sizeof (TW_STR128),
sizeof (TW_STR255),
};

// helper function:
static int TypeMatch(unsigned nTypeA, unsigned nTypeB)
{
	// Integral types match if they are the same size.
	// All other types match only if they are equal
	return nTypeA == nTypeB ||
		(nTypeA <= TWTY_UINT32 &&
		nTypeB <= TWTY_UINT32 &&
		nTypeSize[nTypeA] == nTypeSize[nTypeB]);
} // TypeMatch


BOOL CTwain::GetCapCurrent(unsigned Cap, unsigned ItemType, void FAR *pVal)
{
	TW_CAPABILITY 	cap;
	void far *		pv = NULL;
	BOOL			   bSuccess = FALSE;

	if (nState < SOURCE_OPEN) {
		ATLTRACE_ERROR(TWERR_NOT_4);
		return FALSE;
	}

	// Fill in capability structure
	cap.Cap = (TW_UINT16)Cap;		// capability id
	cap.ConType = TWON_ONEVALUE;	// favorite type of container (should be ignored...)
	cap.hContainer = NULL;

	if (DS(DG_CONTROL, DAT_CAPABILITY, MSG_GETCURRENT, (TW_MEMREF)&cap) &&
		cap.hContainer &&
		(pv = GlobalLock(cap.hContainer))) {

			if (cap.ConType == TWON_ENUMERATION) {
				TW_ENUMERATION far *pcon = (TW_ENUMERATION far *)pv;
				TW_UINT32 index = pcon->CurrentIndex;
				if (index < pcon->NumItems && TypeMatch(pcon->ItemType, ItemType)) {
					LPSTR pitem = (LPSTR)pcon->ItemList + index*nTypeSize[ItemType];
					memcpy(pVal, pitem, nTypeSize[ItemType]);
					bSuccess = TRUE;
				}
			} else if (cap.ConType == TWON_ONEVALUE) {
				TW_ONEVALUE far *pcon = (TW_ONEVALUE far *)pv;
				if (TypeMatch(pcon->ItemType, ItemType)) {
					memcpy(pVal, &pcon->Item, nTypeSize[ItemType]);
					bSuccess = TRUE;
				}
			}
		}

		if (pv) GlobalUnlock(cap.hContainer);
		if (cap.hContainer) GlobalFree(cap.hContainer);

		return bSuccess;
} 


///////////////////////////////////////////////////////////////////
// Primitive functions

static char* pzRcName[] = {
	"TWRC_SUCCESS",
	"TWRC_FAILURE",
	"TWRC_CHECKSTATUS",
	"TWRC_CANCEL",
	"TWRC_DSEVENT",
	"TWRC_NOTDSEVENT",
	"TWRC_XFERDONE",
	"TWRC_ENDOFLIST" };

static char* pzCcName[] = {
	"TWCC_SUCCESS",       
	"TWCC_BUMMER",
	"TWCC_LOWMEMORY",     
	"TWCC_NODS",
	"TWCC_MAXCONNECTIONS",
	"TWCC_OPERATIONERROR",
	"TWCC_BADCAP",
	"<bad cc:7>",
	"<bad cc:8>",
	"TWCC_BADPROTOCOL",
	"TWCC_BADVALUE",
	"TWCC_SEQERROR",
	"TWCC_BADDEST",
	"TWCC_CAPUNSUPPORTED",
	"TWCC_CAPBADOPERATION",
	"TWCC_CAPSEQERROR" };


int CTwain::DS(unsigned long dg, unsigned dat, unsigned msg, void FAR *pd)
	// Call the current source with a triplet
{
	int bSuccess = FALSE;
	ATLASSERT(nState >= SOURCE_OPEN);
	if (pSM_Entry) {
		rc = (*pSM_Entry)(&AppId, &SourceId, dg, (TW_UINT16)dat, (TW_UINT16)msg, (TW_MEMREF)pd);
		if (dat != DAT_STATUS && dat != DAT_EVENT) {
			if (rc == TWRC_SUCCESS) {
				ATLTRACE(_T("TWAIN:DS(%ul, %u, %u) => %s\n"), dg, dat, msg, CA2T(pzRcName[rc]));
			} else {
				unsigned cc = ConditionCode();
				ATLTRACE(_T("TWAIN:DS(%ul, %u, %u) => %s, %s\n"), dg, dat, msg, CA2T(pzRcName[rc]), CA2T(pzCcName[cc]));
			}
		}
		bSuccess = (TWRC_SUCCESS == rc);
		// model state changes!
		if (DG_CONTROL == dg) {
			if (DAT_EVENT == dat) {
				if (MSG_PROCESSEVENT == msg) {
					if (MSG_XFERREADY == ((TW_EVENT FAR *)pd)->TWMessage) {
						if (bTrace) ATLTRACE(_T("TWAIN: received MSG_XFERREADY\n"));
						SetState(TRANSFER_READY);
					} else if (MSG_CLOSEDSREQ == ((TW_EVENT FAR *)pd)->TWMessage) {
						// well, see TWAIN v1.5 p7-27!
						if (bTrace) ATLTRACE(_T("TWAIN: received MSG_CLOSEDSREQ\n"));
						SetState(SOURCE_ENABLED);
					}
				}
			} else if (DAT_PENDINGXFERS == dat) {
				if (MSG_RESET == msg) {
					if (bSuccess) SetState(SOURCE_ENABLED);
				} else if (MSG_ENDXFER == msg) {
					if (bSuccess) {
						SetState(((TW_PENDINGXFERS FAR *)pd)->Count ? TRANSFER_READY : SOURCE_ENABLED);
						// Note that a Count of -1 is valid,
						// indicating 'unknown number' pending.
						// TWAIN 1.6 4-22
					} else if (nState > SOURCE_ENABLED) {
						// We can only guess that the DS made a, shall we say,
						// 'unexpected' transition out of state 7 or 8.
						// Thanks to Pat O'Neil for tipping us to this.
						SetState(SOURCE_ENABLED);
						if (bTrace) ATLTRACE(_T("TWAIN: unexpected ENDXFER failure.\n"));
					}
				}
			} else if (DAT_USERINTERFACE == dat) {
				if (MSG_DISABLEDS == msg) {
					if (bSuccess) SetState(SOURCE_OPEN);
				} else if (MSG_ENABLEDS == msg) {
					bSuccess |= (TWRC_CHECKSTATUS == rc);
					if (bSuccess) {
						SetState(SOURCE_ENABLED);
					}
				}
			} else if (DAT_IDENTITY == dat) {
				if (MSG_CLOSEDS == msg) {
					if (bSuccess) SetState(SOURCE_MANAGER_OPEN);
				}
			}
		} else if (DG_IMAGE == dg) {
			if (DAT_IMAGENATIVEXFER == dat ||
				DAT_IMAGEFILEXFER == dat ||
				DAT_IMAGEMEMXFER == dat) {
					if (MSG_GET == msg) {
						switch (rc) {
			case TWRC_SUCCESS:
			case TWRC_XFERDONE:
				bSuccess = TRUE;
			case TWRC_CANCEL:
				// is this right, does TWRC_CANCEL imply state 7?
				SetState(TRANSFERRING);
				break;
			default:
				// Failure
				// If first buffer, still in state 6 otherwise state 7.
				break;
						} // switch
					}
				}
		}
	}
	return bSuccess;
} // DS



int CTwain::SM(unsigned long dg, unsigned dat, unsigned msg, void FAR *pd)
	// Call the Source Manager with a triplet
{
	int bSuccess = FALSE;
	ATLASSERT(nState >= SOURCE_MANAGER_LOADED);
	ATLASSERT(pSM_Entry);
	if (pSM_Entry) {
		rc = (*pSM_Entry)(&AppId, NULL, dg, (TW_UINT16)dat, (TW_UINT16)msg, (TW_MEMREF)pd);
		bSuccess = (TWRC_SUCCESS == rc);
		if (DG_CONTROL == dg) {
			if (DAT_PARENT == dat) {
				if (MSG_OPENDSM == msg) {
					if (bSuccess) SetState(SOURCE_MANAGER_OPEN);
				} else if (MSG_CLOSEDSM == msg) {
					if (bSuccess) SetState(SOURCE_MANAGER_LOADED);
				}
			} else if (DAT_IDENTITY == dat) {
				if (MSG_OPENDS == msg) {
					if (bSuccess) SetState(SOURCE_OPEN);
				}
			}
		}
	}
	return bSuccess;
} // SM



unsigned CTwain::ResultCode(void)
{
	return rc;
} // ResultCode



unsigned CTwain::ConditionCode(void)
{
	TW_STATUS	twStatus;
	TW_INT16    rc;         // private, not shared

	if (nState >= SOURCE_OPEN) {
		// get source status if open
		rc = (*pSM_Entry)(&AppId, &SourceId, DG_CONTROL, DAT_STATUS, MSG_GET, (TW_MEMREF)&twStatus);
	} else if (nState == SOURCE_MANAGER_OPEN) {
		// otherwise get source manager status
		rc = (*pSM_Entry)(&AppId, NULL, DG_CONTROL, DAT_STATUS, MSG_GET, (TW_MEMREF)&twStatus);
		//SM(DG_CONTROL, DAT_STATUS, MSG_GET, (TW_MEMREF)&twStatus);
	} else {
		// nothing open, not a good time to get condition code!
		return TWCC_SEQERROR;
	}
	if (rc == TWRC_SUCCESS) {
		return twStatus.ConditionCode;
	} else {
		return TWCC_BUMMER;			// what can I say. 
	}
} // ConditionCode


