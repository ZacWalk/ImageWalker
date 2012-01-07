#pragma once

#define AW_HOR_POSITIVE             0x00000001
#define AW_HOR_NEGATIVE             0x00000002
#define AW_VER_POSITIVE             0x00000004
#define AW_VER_NEGATIVE             0x00000008
#define AW_CENTER                   0x00000010
#define AW_HIDE                     0x00010000
#define AW_ACTIVATE                 0x00020000
#define AW_SLIDE                    0x00040000
#define AW_BLEND                    0x00080000

template<class T>
class AnimateWindowImpl
{
protected:
	HINSTANCE	m_hUser32;
	BOOL		(WINAPI *m_pfnAnimateWindow)(HWND, DWORD, DWORD);

public:

	AnimateWindowImpl()
	{
		m_hUser32			= NULL;
		m_pfnAnimateWindow	= NULL;

		if (m_hUser32 = LoadLibrary( _T("User32.dll")))
		{
			(FARPROC&)m_pfnAnimateWindow = GetProcAddress( m_hUser32, "AnimateWindow" );
		}
	}

	BEGIN_MSG_MAP(CMainFrame)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		Show();
		bHandled = false;
		return 0;
	}


	void Show()
	{
		T *pT = static_cast<T*>(this);
		if ( m_pfnAnimateWindow != NULL )
		{
			(*m_pfnAnimateWindow)(pT->m_hWnd, 250, AW_BLEND | AW_ACTIVATE  );
			::RedrawWindow(pT->m_hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}

	void Hide()
	{
		T *pT = static_cast<T*>(this);
		if ( m_pfnAnimateWindow != NULL )
		{
			(*m_pfnAnimateWindow)(pT->m_hWnd, 250, AW_HIDE|AW_BLEND );
		}

	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);
		Hide();		
		pT->DestroyWindow();

		bHandled = false;
		return 0;
	}
};