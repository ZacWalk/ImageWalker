///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// WallPaperDlg.h : Declaration of the CWallPaperDlg

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWallPaperDlg
class CWallPaperDlg : 
	public CDialogImpl<CWallPaperDlg>
{
public:
	const IW::Image  &_image;

	bool _bWallPaperScale;
	bool _bWallPaperKeepAspect;

	CWallPaperDlg(const IW::Image &image) : _image(image)
	{
		_bWallPaperScale = true;
		_bWallPaperKeepAspect = true;
	}

	~CWallPaperDlg()
	{
	}

	enum { IDD = IDD_WALLPAPER };

	BEGIN_MSG_MAP(CWallPaperDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_SCALE, OnScale)
		COMMAND_ID_HANDLER(IDC_ASPECT, OnAspect)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)

		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)

	END_MSG_MAP()

	CSize GetDestSize()
	{
		// Get the rectangle that bounds the size of the screen
		// minus any docked (but not-autohidden) AppBars.
		CRect r(0,0, ::GetSystemMetrics(SM_CXFULLSCREEN), ::GetSystemMetrics(SM_CYFULLSCREEN));
		//::SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
		::GetWindowRect ( ::GetDesktopWindow(), &r );
		IW::Page page = _image.GetFirstPage();

		if (_bWallPaperScale)
		{
			CSize size(r.Size()); 

			if (_bWallPaperKeepAspect)
			{
				const CRect rectBounding = _image.GetBoundingRect();

				long icx = rectBounding.Width();
				long icy = rectBounding.Height();
				long nDiv = 0x1000;

				long sw = MulDiv(r.Width(), nDiv, icx);
				long sh = MulDiv(r.Height(), nDiv, icy);

				long s =  IW::Min(sh, sw);

				size.cx = MulDiv(page.GetWidth(), s, nDiv);
				size.cy = MulDiv(page.GetHeight(), s, nDiv);
			}

			return size;
		}


		return page.GetSize();
	}

	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		int nIDCtl = wParam;

		if (IDC_PREVIEW == nIDCtl)
		{
			CRect r(lpDrawItemStruct->rcItem);

			CRect rectScreen(0,0, ::GetSystemMetrics(SM_CXFULLSCREEN), ::GetSystemMetrics(SM_CYFULLSCREEN));
			//::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
			::GetWindowRect ( ::GetDesktopWindow(), &rectScreen );

			CSize sizeOnScreen(GetDestSize());

			CSize size(
				(sizeOnScreen.cx * r.Width()) / rectScreen.Width(),
				(sizeOnScreen.cy * r.Height()) / rectScreen.Height());

			::FillRect(lpDrawItemStruct->hDC, &r, (HBRUSH)LongToPtr(COLOR_DESKTOP + 1));

			CRect r2(r.CenterPoint() - CSize(size.cx / 2, size.cy / 2), size);	
			IW::Page page = _image.GetFirstPage();
			IW::CRender::DrawToDC(lpDrawItemStruct->hDC, page, r2);

			/*CDibDC ddc;
			ddc.Create(r.Width(), r.Height(), 24);

			ddc.FillSolidRect(r, 0x808000);

			CRect r2(r.CenterPoint() - CSize(size.cx / 2, size.cy / 2), size);
			ddc.Draw(*m_pImage, r2);

			dc.Attach(lpDrawItemStruct->hDC);
			ddc.GetDib().Draw(&dc, r.TopLeft(), r);
			dc.Detach();*/
		}

		return 0;
	}




	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();

		CPropertyArchiveRegistry archive(App.GetRegKey());
		
		if (archive.StartSection(g_szWallPaper))
		{
			archive.Read(g_szWallPaperScale, _bWallPaperScale);
			archive.Read(g_szWallPaperKeepAspect, _bWallPaperKeepAspect);
		}

		CheckDlgButton(IDC_SCALE, _bWallPaperScale);
		CheckDlgButton(IDC_ASPECT, _bWallPaperKeepAspect);

		return 1;
	}

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CPropertyArchiveRegistry archive(App.GetRegKey(), true);
		
		if (archive.StartSection(g_szWallPaper))
		{
			archive.Write(g_szWallPaperScale, _bWallPaperScale);
			archive.Write(g_szWallPaperKeepAspect, _bWallPaperKeepAspect);
		}

		EndDialog(wID);

		return 0;
	}

	LRESULT OnScale(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		_bWallPaperScale = IsDlgButtonChecked(IDC_SCALE) == BST_CHECKED;
		::InvalidateRect(GetDlgItem(IDC_PREVIEW), NULL, false);

		return 0;
	}

	LRESULT OnAspect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		_bWallPaperKeepAspect = IsDlgButtonChecked(IDC_ASPECT) == BST_CHECKED;
		::InvalidateRect(GetDlgItem(IDC_PREVIEW), NULL, false);

		return 0;
	}

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_WALLPAPER);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

};
