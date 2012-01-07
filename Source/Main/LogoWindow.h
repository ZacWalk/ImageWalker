#pragma once

class CLogoWindow : 
	public ATL::CWindowImpl<CLogoWindow>, 
	public CDoubleBufferImpl<CLogoWindow>
{
public:

	IW::Image _imageWorking;	
	int _nTimer;
	int m_nColorFade;
	bool m_bWorking;
	bool m_bHover;


	CLogoWindow()
	{
		m_bHover = false;
		m_bWorking = false;
		m_nColorFade = 0;
	}

	BEGIN_MSG_MAP(CLogoWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		CHAIN_MSG_MAP(CDoubleBufferImpl<CLogoWindow>)
	END_MSG_MAP()

	static int g_nCosinus[];

	//---------------------------------
	// Plasmas Stuff
	//---------------------------------

	void StepPlasma(LPBYTE pVidMem, int xx, int yy);
	void InitPlasma(COLORREF *pRgb, int &nColor);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_bHover = false;
		SetFadeStep();
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_bHover)
		{
			m_bHover = true;
			SetFadeStep();

			TRACKMOUSEEVENT tme = { 0 };
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			_TrackMouseEvent(&tme);
		}

		return 0;
	}

	void DoPaint(CDCHandle dc);

	void SetWorking(bool bWorking)
	{
		if (m_bWorking != bWorking)
		{
			m_bWorking = bWorking;
			SetFadeStep();
		}
	}

	void SetFadeStep()
	{
		m_nColorFade = (m_bWorking || m_bHover) ? 2 : -1;
	}

	void OnTimer()
	{
		IW::Page page = _imageWorking.GetFirstPage(); 


		if (m_hWnd && IsWindowVisible())
		{
			if (m_nColorFade)
			{
				COLORREF *pRgb = page.GetPalette();
				InitPlasma(pRgb, m_nColorFade);
			}

			const DWORD dwStorageWidth = page.GetStorageWidth();
			StepPlasma(page.GetBitmap(),  dwStorageWidth,  page.GetHeight());
			Invalidate();
		}
	}
};


