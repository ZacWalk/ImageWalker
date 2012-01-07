///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// SlideShowView.h: interface for the CWebPreview class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Layout.h"
#include "ImageFrames.h"




class ScreenSaveActive
{
public:

	DWORD _dwScreenSaverSetting;
	DWORD _dwLowPowerSetting;
	DWORD _dwPowerOffSetting;
	DWORD _dwAppBarState;

	ScreenSaveActive() : 
		_dwScreenSaverSetting(0),
		_dwLowPowerSetting(0),
		_dwPowerOffSetting(0),
		_dwAppBarState(0)
	{
	}

	void TurnOff()
	{
		SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &_dwScreenSaverSetting, 0); 
		SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &_dwLowPowerSetting, 0); 
		SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &_dwPowerOffSetting, 0); 

		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, SPIF_SENDWININICHANGE);
		SystemParametersInfo(SPI_SETLOWPOWERACTIVE,0,NULL,0); 
		SystemParametersInfo(SPI_SETPOWEROFFACTIVE,0,NULL,0);

		//_dwAppBarState = GetAppBarState();
		//SetAppBarState(ABS_AUTOHIDE);
	}

	void Restore()
	{
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, _dwScreenSaverSetting, 0, SPIF_SENDWININICHANGE);
		SystemParametersInfo(SPI_SETLOWPOWERACTIVE,_dwLowPowerSetting,NULL,0); 
		SystemParametersInfo(SPI_SETPOWEROFFACTIVE,_dwPowerOffSetting,NULL,0);
		//SetAppBarState(_dwAppBarState);
	}

	void SetAppBarState(DWORD state)
	{
		APPBARDATA TaskBar;
		TaskBar.hWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
		TaskBar.cbSize = sizeof(TaskBar);
		TaskBar.lParam = state;
		SHAppBarMessage(ABM_SETSTATE, &TaskBar);
	}

	DWORD GetAppBarState()
	{
		APPBARDATA TaskBar;
		TaskBar.hWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
		TaskBar.cbSize = sizeof(TaskBar);
		return SHAppBarMessage(ABM_GETSTATE, &TaskBar) & (ABS_AUTOHIDE|ABS_ALWAYSONTOP);
	}
};

class SlideShowView : 
	public CWindowImpl<SlideShowView>,
	public CImageWindowImpl<SlideShowView>,
	public FrameWindowImpl< SlideShowView>,
	public ImageCommandImpl<SlideShowView>,
	public ToolTipWindowImpl<SlideShowView>,
	public ViewBase
{
public:

	typedef SlideShowView ThisClass;
	typedef CImageWindowImpl<ThisClass> ImageImpBaseClass;
	typedef FrameWindowImpl<ThisClass> FrameImpBaseClass;
	typedef ToolTipWindowImpl<ThisClass> ToolTipType;

	State &_state;
	bool _bWasFullScreen;
	ScreenSaveActive _screenSaveActive;

	FrameImageInfo _frameImageInfo;
	FrameSlideshow<ThisClass> _frameControls;
	

	SlideShowView(Coupling *pCoupling, State &state) : 
		CImageWindowImpl<SlideShowView>(pCoupling, state),
		_state(state),
		_bWasFullScreen(false),
		_frameImageInfo(&_frameParent, state),
		_frameControls(&_frameParent, this, state)
	{
		_frames.push_back(&_frameImageInfo);
		_frames.push_back(&_frameControls);
	}	

	~SlideShowView()
	{
		ATLTRACE(_T("Delete SlideShowView\n"));
	}

	void LoadCommands()
	{
		AddCommand(ID_SLIDESHOW_NEXTIMAGE, new  CommandShowNextImage<ThisClass>(this));		
		AddCommand(ID_SLIDESHOW_PREVIOUSIMAGE, new  CommandShowPreviousImage<ThisClass>(this));
		AddCommand(ID_SLIDESHOW_PLAY, new  CommandShowPlay<ThisClass>(this));		
		AddCommand(ID_SLIDESHOW_PAUSE, new  CommandShowPause<ThisClass>(this));
		AddCommand(ID_SLIDESHOW_STOP, new  CommandShowStop<ThisClass>(this));
		AddCommand(ID_VIEW_DESCRIPTION, new CommandShowInformation<ThisClass>(this));	
		AddCommand(ID_VIEW_ADVANCEDIMAGE, new CommandShowTask(_state, App.Options.ShowAdvancedImageDetails));

		AddSlideShowCommands();
		AddFileCommands();
		AddFilterCommands();
	}

	inline bool CanCopy() const
	{
		return _state.Image.IsImageShown();
	}

	inline bool CanMove() const
	{
		return _state.Image.IsImageShown();
	}

	void OnBeforeFileOperation()
	{
		Pause();
	}

	void OnAfterDelete()
	{
		NextImage();
	}

	void OnAfterCopy(bool bMove)
	{
		if (bMove)
		{
			NextImage();
		}
	}

	CString GetSelectedFileList() const
	{
		return GetImageFileName() + ("\n\n");
	}

	void LoadDefaultSettings(IW::IPropertyArchive *pProperties)
	{
	}

	void SaveDefaultSettings(IW::IPropertyArchive *pProperties)
	{
	}

	void RefreshDescription()
	{
		_frameImageInfo.Refresh();
	}

	HWND Activate(HWND hWndParent)
	{
		if (m_hWnd == 0)
		{
			Create(hWndParent, rcDefault, NULL, IW_WS_CHILD, 0);
		}		

		ShowWindow(SW_SHOW);
		OnOptionsChanged();		
		_bWasFullScreen = _pCoupling->ViewFullScreen(true);			
		_screenSaveActive.TurnOff();
		OnResetFrames();
		SetFocus();

		return m_hWnd;
	}

	void Deactivate()
	{
		_state.Image.Pause();
		_state.Image.Stop();
		_screenSaveActive.Restore();
		_pCoupling->ViewFullScreen(_bWasFullScreen);
		ShowWindow(SW_HIDE);
	}

	bool CanEditImages() const 
	{
		return true;
	}

	bool CanShowToolbar(DWORD id)
	{
		return false;
	}

	CString GetToolTipText(int idCtrl)
	{
		FrameVisitorTooltip visitor;
		Accept(visitor);
		if (visitor.Handled) idCtrl = visitor.Id;
		return ToolTipType::GetToolTipText(idCtrl);
	}

	void OnKeyDown(int nChar)
	{
		switch(nChar)
		{
		/*case VK_ESCAPE:
			_pCoupling->SetViewNormal();
			break;*/
		case VK_END:
			_state.Image.End();
			break;

		case VK_HOME:
			_state.Image.Home();
			break;

		case VK_LEFT:
		case VK_UP:
		case VK_PRIOR:
			_state.Image.PreviousImage();
			break;

		case VK_RIGHT:
		case VK_DOWN:
		case VK_NEXT:
			_state.Image.NextImage();
			break;

		case VK_PAUSE:
			SetAnimationPaused(!IsAnimationPaused());
			break;

		case VK_SUBTRACT:
		case '-':
		case '_':
			PreviousFrame();
			break;

		case VK_ADD:
		case '=':
		case '+':
			NextFrame();				
			break;
		}
	}

	void OnTimer()
	{
		ImageImpBaseClass::OnTimer();
	}

	void OnOptionsChanged()
	{
		OnResetFrames();
		Invalidate();
	}

	BEGIN_MSG_MAP(SlideShowView)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)		

		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_SIZE, OnSize)

		CHAIN_MSG_MAP(ImageImpBaseClass)
		CHAIN_MSG_MAP(FrameImpBaseClass)
		CHAIN_MSG_MAP(ToolTipType)

	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SetScale(ScaleMode::Down);
		_state.ResetFrames.Bind(this, &ThisClass::OnResetFrames);
		bHandled = false;
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		WORD id = LOWORD(wParam);
		_pCoupling->Command(id);
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int cx = LOWORD(lParam);
		int cy = HIWORD(lParam);

		if(wParam != SIZE_MINIMIZED)
		{
			SizeClients();
			SetScrollSizes(true);
		}

		bHandled = FALSE;
		return 1;
	}	

	void OnNewFolder()
	{		
	}	

	void OnPaint(CDCHandle dc)
	{
		IW::CRender render;

		if (render.Create(dc))
		{
			render.Fill(GetBackGroundColor());

			CRect rectClip;
			dc.GetClipBox(rectClip);
			DrawImage(render, rectClip);			

			Accept(FrameVisitorErase(render));
			Accept(FrameVisitorRender(render));

			render.Flip();
		}

		DrawControls(dc);
	}

	COLORREF GetBackGroundColor() const
	{
		return IW::Style::Color::SlideShowWindow;
	}	

	void OnResetFrames()
	{
		_frameImageInfo.Refresh();

		ShowFrames();
		SizeClients();
		SetScrollSizes(false);
		ShowMouseMoveFeedback();
	}

	HWND GetImageWindow()
	{
		return m_hWnd;
	}
	
	void OnNewImage(bool bScrollToCenter)
	{
		if (m_hWnd)
		{
			Refresh(bScrollToCenter);
			_frameImageInfo.Refresh();
			OnResetFrames();
		}
	}

	virtual void OnPlayStateChange(bool bPlay) 
	{
		OnResetFrames();
	};

	void NextImage()
	{
		_state.Image.NextImage();
	}

	void PreviousImage()
	{
		_state.Image.PreviousImage();
	}

	void ReloadFileList()
	{
		_state.Image.ReloadFileList();
	}

	bool IsPlaying() const throw()
	{
		return _state.Image.IsPlaying();
	}

	void Play()
	{
		_state.Image.Play();
	}
	
	void Pause()
	{
		_state.Image.Pause();
	}

	void Stop()
	{
		_state.Image.Stop();
		_pCoupling->Command(ID_VIEW_NORMAL);
	}

	void Redraw()
	{
		OnResetFrames();
	}

	inline bool CanDelete() const throw()
	{
		return _state.Image.IsImageShown();
	}

	inline bool HasImageRectSelection() const
	{
		return HasSelection();
	}

	void ToggleScale()
	{
		ImageImpBaseClass::ToggleScale(true);
	}

	void ShowFrames()
	{		
		_frameImageInfo.SetVisible(CanImageInfo());
		_frameControls.SetVisible(CanShowControls());

		Accept(FrameVisitorActivate(m_hWnd));

		ImageImpBaseClass::ShowFrames();
	}

	bool CanShowControls() const 
	{
		return _state.Image.IsImageShown() && App.Options._bSlideShowToolBar;
	}		

	bool CanImageInfo() const
	{
		return _state.Image.IsImageShown() && 
			App.Options.m_bShowInformation &&
			!_state.Image.IsImageEditMode();
	}

	void OnScaleChange()
	{
		_frameNavigation.SetVisible(CanShowNavigation());
		_frameControls.OnScaleChange();
	}
};