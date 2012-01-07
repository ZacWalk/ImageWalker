#pragma once

template<class TParent>
class Capture
{
public:
	bool m_bWantCursor;
	unsigned m_nFormatSelection;		
	unsigned m_nMode;	

	TParent *_pParent;

	Capture(TParent *pParent) :
		_pParent(pParent),
		m_bWantCursor(false),
		m_nFormatSelection(0),
		m_nMode(0)
	{
	}

	static HCURSOR GetCurrentCursorHandle()
	{
		CPoint pt;
		HWND hWnd;        
		DWORD dwThreadID, dwCurrentThreadID;
		HCURSOR hCursor = NULL;

		// Find out which window owns the cursor
		GetCursorPos(&pt);
		hWnd = WindowFromPoint(pt);

		// Get the thread ID for the cursor owner.
		dwThreadID = GetWindowThreadProcessId(hWnd, NULL);

		// Get the thread ID for the current thread
		dwCurrentThreadID = GetCurrentThreadId();

		// If the cursor owner is not us then we must attach to 
		// the other thread in so that we can use GetCursor() to 
		// return the correct hCursor
		if (dwCurrentThreadID != dwThreadID) {

			// Attach to the thread that owns the cursor
			if (AttachThreadInput(dwCurrentThreadID, dwThreadID, TRUE)) {

				// Get the handle to the cursor
				hCursor = GetCursor();

				// Detach from the thread that owns the cursor
				AttachThreadInput(dwCurrentThreadID, dwThreadID, FALSE);
			}
		} else
			hCursor = GetCursor();

		return hCursor;
	} 


	void ScreenCapture(HWND hwnd)
	{
		HDC hdc = NULL;
		CRect r;
		bool bUsingWindowDc = true;

		// What to capture
		switch(m_nMode)
		{
		case 0: // Active window

			if (hwnd == NULL)
				hwnd = GetDesktopWindow();

			hdc = ::GetWindowDC(hwnd);
			::GetWindowRect(hwnd, &r);
			break;

		case 1:
			// client area only

			if (hwnd == NULL)
				hwnd = GetDesktopWindow();

			hdc = ::GetDC(hwnd);
			::GetClientRect(hwnd, &r);
			::ClientToScreen(hwnd, (LPPOINT)&r);
			::ClientToScreen(hwnd, ((LPPOINT)&r) + 1);
			break;

		case 2: // Are around cursor
			{
				CPoint point;
				::GetCursorPos(&point); 

				r.left = IW::LowerLimit<0>(point.x - 200);
				r.top = IW::LowerLimit<0>(point.y - 200);
				r.right = IW::Min(point.x + 200, GetSystemMetrics(SM_CXFULLSCREEN));
				r.bottom = IW::Min(point.y + 200, GetSystemMetrics(SM_CYFULLSCREEN));
				
				hwnd = GetDesktopWindow();
				hdc = ::GetWindowDC(hwnd);
				bUsingWindowDc = false;
			}
			break;

		case 4: // Window under cursor
			{
				CPoint pt;
				GetCursorPos(&pt);
				hwnd = WindowFromPoint(pt);

				if (hwnd == NULL)
					hwnd = GetDesktopWindow();

				hdc = ::GetWindowDC(hwnd);
				::GetWindowRect(hwnd, &r);
			}
			break;

		default: // Desktop
			hwnd = GetDesktopWindow();
			hdc = ::GetWindowDC(hwnd);
			::GetWindowRect(hwnd, &r);
			bUsingWindowDc = false;
			break;
		}

		if (hwnd && hdc)
		{
			// get the cursor before we set the wait cursor
			HCURSOR hCur = GetCurrentCursorHandle();

			CWaitCursor wait;

			int nWidth = r.right - r.left;
			int nHeight = r.bottom - r.top;
			
			CBitmap bitmap;
			CDC dcCompatible;
			HBITMAP hBitmapOld = NULL;
			
			if (bitmap.CreateCompatibleBitmap(hdc, nWidth, nHeight) && 
				dcCompatible.CreateCompatibleDC(hdc) &&
				(hBitmapOld = dcCompatible.SelectBitmap(bitmap)))
			{
				if (bUsingWindowDc)
				{
					dcCompatible.BitBlt(0, 0, 
						nWidth, nHeight, hdc, 0,0, SRCCOPY);
				}
				else
				{
					dcCompatible.BitBlt(0, 0, 
						nWidth, nHeight, hdc, r.left, r.top, SRCCOPY);
				}
				
				// Do we need to draw a cursor?
				if (m_bWantCursor)
				{
					CPoint point;
					::GetCursorPos(&point); 
					
					if (hCur)
					{
						ICONINFO ii;
						GetIconInfo(hCur, &ii);
						
						DrawIconEx(dcCompatible,
							point.x - r.left - ii.xHotspot,
							point.y - r.top - ii.yHotspot,
							hCur,
							0,
							0,
							0,
							0,
							DI_NORMAL|DI_DEFAULTSIZE);
					}
				}
				
				dcCompatible.SelectBitmap(hBitmapOld); 
				
				
				IW::Image dib;
				IW::Page page = dib.CreatePage(nWidth, nHeight, IW::PixelFormat::PF24);

				IW::CImageBitmapInfoHeader bmi(page);
				::GetDIBits(hdc, bitmap, 0, nHeight, page.GetBitmap(), bmi, DIB_RGB_COLORS);
				_pParent->SaveNewImage(dib);
			}

			::ReleaseDC(hwnd, hdc);
		}		
	}

};