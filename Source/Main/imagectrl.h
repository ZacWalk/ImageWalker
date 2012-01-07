#pragma once

class CDescriptionWindow;
class CImageLoad;

#include "View.h"
#include "ImageFrames.h"
#include "ScaleBar.h"
#include "Dialogs.h"
#include "Items.h"
#include "ImageLoaderThread.h"
#include "Flickr.h"
#include "LoadAny.h"
#include "FolderCtrl.h"  
#include "ImageDataObject.h"
#include "ImageNavigation.h"
#include "ProgressDlg.h"

class CImageCtrl :
	public CWindowImpl<CImageCtrl>,
	public CView<CImageCtrl>,
	public CPaletteImpl<CImageCtrl>,
	public CDropTargetImpl<CImageCtrl>,	
	public CImageWindowImpl<CImageCtrl>
{
public:

	typedef CImageCtrl ThisClass;
	typedef CImageWindowImpl<ThisClass> ImageWindowBase;

	State &_state;

	FrameImageCtrl<CImageCtrl> _frameControls;
	FrameImageInfo _frameImageInfo;
	FrameFlickr _frameFlickr;

	FrameImageWalker _frameImageWalker;

	CImageCtrl(Coupling *pCoupling, State &state) :
		CImageWindowImpl<CImageCtrl>(pCoupling, state),
		_state(state),		
		_frameImageInfo(&_frameParent, state),
		_frameControls(&_frameParent, this, state),
		_frameFlickr(&_frameParent, state),
		_frameImageWalker(&_frameParent)
	{
		_frames.push_back(&_frameImageWalker);
		_frames.push_back(&_frameImageInfo);
		_frames.push_back(&_frameFlickr);
		_frames.push_back(&_frameControls);		
	}

	~CImageCtrl()
	{
	}

	void OnNewImage(bool bScrollToCenter)
	{
		Refresh(bScrollToCenter);
		RefreshDescription();
		OnResetFrames();
	}

	void OnActivate()
	{		
		RefreshDescription();
		OnResetFrames();
	}

	void OnResetFrames()
	{
		_frameImageInfo.Refresh();	
		_frameImageWalker.Refresh();

		ShowFrames();
		SizeClients();
		ImageWindowBase::ShowMouseMoveFeedback();
		Invalidate();
	}

	void RefreshDescription()
	{
		_frameImageInfo.Refresh();
	}

	void LayoutFrames(int cx, int cy)
	{
		SizeClients();
		Invalidate();
	}	

	void OnCommand(WORD id)
	{
		_pCoupling->Command(id);
	}	

	BEGIN_MSG_MAP(CImageCtrl)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		CHAIN_MSG_MAP(CPaletteImpl<CImageCtrl>)
		CHAIN_MSG_MAP(ImageWindowBase)
		CHAIN_MSG_MAP(CView<CImageCtrl>)

	END_MSG_MAP()

public:

	void OnOptionsChanged()
	{
		Invalidate();
	}	

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RegisterDropTarget();
		_state.ResetFrames.Bind(this, &ThisClass::OnResetFrames);
		ShowFrames();
		bHandled = false;
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy CImageCtrl\n"));
		RevokeDropTarget();
		bHandled = false;
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SizeClients();

		bHandled = FALSE;
		return 1;
	}

	void LoadImageFromHGlobal(HGLOBAL hGlobal)
	{
		IW::Image image;
		image.Copy(hGlobal);
		_state.Image.SetImage(image, g_szEmptyString);
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

	CString GetToolTipText(int idCtrl)
	{
		FrameVisitorTooltip visitor;
		Accept(visitor);
		if (visitor.Handled) idCtrl = visitor.Id;
		return CView<CImageCtrl>::GetToolTipText(idCtrl);
	}

	void OnKeyDown(int nChar)
	{
		switch(nChar)
		{
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

	void Redraw()
	{
		LayoutFrames(0, 0);
		DoSize();		
	}

	COLORREF GetBackGroundColor() const
	{
		return IW::Style::Color::Window;
	}

	void ShowFrames()
	{		
		_frameControls.SetVisible(CanShowControls());
		_frameImageInfo.SetVisible(CanShowImageInfo());
		_frameImageWalker.SetVisible(CanShowHelp());
		_frameFlickr.SetVisible(CanShowFlickr());

		Accept(FrameVisitorActivate(m_hWnd));

		ImageWindowBase::ShowFrames();
	}	

	CRect GetFrameSliderRect() const
	{
		const int nWidth = 2;
		const CSize sizeClient = GetClientSize();
		return CRect(nWidth, 0, nWidth, sizeClient.cy);
	}

	bool CanShowControls() const 
	{
		return _state.Image.IsImageShown();
	}

	bool CanShowFlickr() const
	{
		return App.IsOnline() && 
			App.Options.ShowFlickrPicOfInterest &&
			!_state.Image.IsImageEditMode();
	}

	bool CanShowHelp() const
	{
		return !_state.Image.IsImageShown();
	}

	bool CanShowImageInfo() const
	{
		return App.Options.ShowDescription &&
			_state.Image.IsImageShown() &&
			!_state.Image.IsImageEditMode();
	}

	void OnScaleChange()
	{
		_frameNavigation.SetVisible(CanShowNavigation());
		_frameControls.OnScaleChange();
	}
};

