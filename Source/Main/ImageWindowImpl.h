#pragma once

#include "ImageTools.h"
#include "ImageFrames.h"
#include "ImageFilter.h"

class FrameVisitor;


template<class T>
class ImageCommandImpl
{
public:
	void AddSlideShowCommands()
	{
		T* pT = static_cast<T*>(this);
		ImageState &imageState = pT->_state.Image;

		// Effects
		pT->AddCommand(ID_SLIDESHOW_TOOLBAR, new CommandShowToolBar<T>(pT));
		pT->AddCommand(ID_SLIDESHOW_EFFECTS, new CommandShowEffects<T>(pT));	
		pT->AddCommand(ID_SLIDESHOW_RECURSSUBFOLDERS, new CommandShowRecurseSubFolders<T>(pT));
		pT->AddCommand(ID_SLIDESHOW_REPEAT, new CommandShowRepeat<T>(pT));
		pT->AddCommand(ID_SLIDESHOW_SHUFFLE, new CommandShowShuffle<T>(pT));

		// Delay
		pT->AddCommand(ID_SLIDESHOW_NODELAY, new CommandShowDelay<T>(pT, 0));
		pT->AddCommand(ID_SLIDESHOW_DELAYBETWEENIMAGES_10SECONDS, new CommandShowDelay<T>(pT, 10));
		pT->AddCommand(ID_SLIDESHOW_DELAYBETWEENIMAGES_1SECOND, new CommandShowDelay<T>(pT, 1));
		pT->AddCommand(ID_SLIDESHOW_DELAYBETWEENIMAGES_2SECOND, new CommandShowDelay<T>(pT, 2));
		pT->AddCommand(ID_SLIDESHOW_DELAYBETWEENIMAGES_30SECONDS, new CommandShowDelay<T>(pT, 30));
		pT->AddCommand(ID_SLIDESHOW_DELAYBETWEENIMAGES_5SECONDS, new CommandShowDelay<T>(pT, 5));

		pT->AddCommand(ID_IMAGE_COPY, new  CommandCopy<T>(pT));	
		pT->AddCommand(ID_IMAGE_EDITDESCRIPTION, new  CommandEditDescription<T>(pT));	
		pT->AddCommand(ID_IMAGE_SETASWAL, new  CommandSetAsWallPaper<T>(pT));	
		pT->AddCommand(ID_IMAGE_OPENINFLICKR, new  CommandOpenInFlickr(pT->_state));
		pT->AddCommand(ID_FLICKR_DOWNLOADIMAGE, new  CommandDownloadFromFlickr(pT->_state));		
		pT->AddCommand(ID_IMAGE_UPLOADTOFLICKR, new  CommandUploadToFlickr(pT->_state));		
		pT->AddCommand(ID_IMAGE_UNLOAD, new  CommandUnload<T>(pT));
		pT->AddCommand(ID_SCALE_100, new  CommandScale<T>(pT, _T("100%")));		
		pT->AddCommand(ID_SCALE_200, new  CommandScale<T>(pT, _T("200%")));		
		pT->AddCommand(ID_SCALE_50, new  CommandScale<T>(pT, _T("50%")));		
		pT->AddCommand(ID_SCALE_DOWN, new  CommandScale<T>(pT, _T("Down")));		
		pT->AddCommand(ID_SCALE_FIT, new  CommandScale<T>(pT, _T("Fit")));		
		pT->AddCommand(ID_SCALE_FILL, new  CommandScale<T>(pT, _T("Fill")));		
		pT->AddCommand(ID_SCALE_UP, new  CommandScale<T>(pT, _T("Up")));		
		pT->AddCommand(ID_SCALE_TOGGLE, new  CommandScaleToggle<T>(pT));		

		pT->AddCommand(ID_EDIT_UNDO_IMAGE, new  CommandUndo<T>(pT));	
		pT->AddCommand(ID_EDIT_SAVE, new  CommandSave<T>(pT));		
		pT->AddCommand(ID_EDIT_SAVE_MOVENEXT, new  CommandSaveMoveNext<T>(pT));		
		pT->AddCommand(ID_EDIT_SAVEAS, new  CommandSaveAs<T>(pT));		

	}

	void AddFileCommands()
	{
		T* pT = static_cast<T*>(this);

		pT->AddCommand(ID_EDIT_DELETE, new  CommandEditDelete<T>(pT));
		pT->AddCommand(ID_EDIT_DELETE_PERM, new  CommandEditDeletePerm<T>(pT));		
		pT->AddCommand(ID_FILE_COPYTO, new  CommandCopyToPopup<T>(pT));		
		pT->AddCommand(ID_FILE_COPYTO_POPUP, new  CommandCopyToPopup<T>(pT));		
		pT->AddCommand(ID_FILE_MOVETO, new  CommandMoveToPopup<T>(pT));		
		pT->AddCommand(ID_FILE_MOVETO_POPUP, new  CommandMoveToPopup<T>(pT));	
		pT->AddCommand(ID_MOVECOPY_CLEARLOCATIONS, new  CommandMoveToClear<T>(pT));		
		pT->AddCommand(ID_MOVETO_NEWLOCATION, new  CommandMoveToNewLocation<T>(pT));	
		pT->AddCommand(ID_COPYTO_NEWLOCATION, new  CommandCopyToNewLocation<T>(pT));		
	}

	void AddFilterCommands()
	{
		T* pT = static_cast<T*>(this);

		pT->AddCommand(ID_FILTER_BLUR, new CommandFilter<T, CFilterBlur>(pT));
		pT->AddCommand(ID_FILTER_COLORADJUST, new CommandFilter<T, CFilterColorAdjust>(pT));
		pT->AddCommand(ID_FILTER_CONTRASTSTRETCH, new CommandFilter<T, CFilterContrastStretch>(pT));
		pT->AddCommand(ID_FILTER_CROP, new CommandFilter<T, CFilterCrop>(pT));
		pT->AddCommand(ID_FILTER_DITHER, new CommandFilter<T, CFilterDither>(pT));
		pT->AddCommand(ID_FILTER_EDGE, new CommandFilter<T, CFilterEdge>(pT));
		pT->AddCommand(ID_FILTER_EMBOS, new CommandFilter<T, CFilterEmbos>(pT));
		pT->AddCommand(ID_FILTER_FRAME, new CommandFilter<T, CFilterFrame>(pT));
		pT->AddCommand(ID_FILTER_GRAYSCALE, new CommandFilter<T, CFilterGreyScale>(pT));
		pT->AddCommand(ID_FILTER_NEGATE, new CommandFilter<T, CFilterNegate>(pT));
		pT->AddCommand(ID_FILTER_QUANTIZE, new CommandFilter<T, CFilterQuantize>(pT));
		pT->AddCommand(ID_FILTER_REDEYEREDUCTION, new CommandFilter<T, CFilterRedEye>(pT));
		pT->AddCommand(ID_FILTER_RESIZE, new CommandFilter<T, CFilterResize>(pT));
		pT->AddCommand(ID_FILTER_ROTATE, new CommandFilter<T, CFilterRotate>(pT));
		pT->AddCommand(ID_FILTER_ROTATELEFT, new CommandFilter<T, CFilterRotateLeft>(pT));
		pT->AddCommand(ID_FILTER_ROTATERIGHT, new CommandFilter<T, CFilterRotateRight>(pT));
		pT->AddCommand(ID_FILTER_SHARPEN, new CommandFilter<T, CFilterSharpen>(pT));
		pT->AddCommand(ID_FILTER_SOFTEN, new CommandFilter<T, CFilterSoften>(pT));
	}
};


template<class T>
class CImageWindowImpl
{
private:
	
	bool _bPauseAnimation;	
	DWORD _dwCursorHide;

	CPoint _pointHideCursor;
	CPoint _pointOffset;
	CSize _sizeDrawnImage;

	CPoint _pointScroll;
	CSize _sizeClient;
	CSize _sizeAll;

	CRect _rectSelected;

	int _nFrame;
	int _nTimer;	
	
	ImageTool *_pToolCurrent;		

	enum { _delay = 5000 };

	Scale _scale;
	State &_state;

	ImageToolSelect<T> _toolSelect;
	ImageToolMoveSelection<T> _toolMoveSelection;
	ImageToolMoveImage<T> _toolMoveImage;

public:

	Coupling *_pCoupling;
	FrameImageNavigate<T> _frameNavigation;

	CImageWindowImpl(Coupling *pCoupling, State &state) : 
	      _pCoupling(pCoupling),
		  _state(state),
		  _bPauseAnimation(false),
		  _nFrame(0),
		  _nTimer(0),
		  _dwCursorHide(GetTickCount() + _delay),
		  _pToolCurrent(0),
		  _toolSelect(*static_cast<T*>(this)),
		  _toolMoveSelection(*static_cast<T*>(this)),
		  _toolMoveImage(*static_cast<T*>(this)),
		  _frameNavigation(&static_cast<T*>(this)->_frameParent, static_cast<T*>(this), state)
	  {
		  _sizeDrawnImage.cx = 1; 
		  _sizeDrawnImage.cy = 1;
		  _pointOffset.x = 0;
		  _pointOffset.y = 0;	
	  }

	  CString GetImageFileName() const { return _state.Image.GetImageFileName(); };
	  bool CanAnimate() const { return _state.Image.GetImage().CanAnimate(); };
	  const IW::Image &GetRenderImage() const { return _state.Image.GetRenderImage(); };

	  CSize GetDrawnImageSize() const
	  {
		  return _sizeDrawnImage;
	  }  	  

	  bool CanNavigate() const
	  {
		  const T *pT = static_cast<const T*>(this);
		  return _state.Image.IsImageShown() &&
			  (_sizeDrawnImage.cx > pT->_sizeClient.cx || _sizeDrawnImage.cy > pT->_sizeClient.cy);
	  };

	  BEGIN_MSG_MAP(CImageWindowImpl<T>)

		  MESSAGE_HANDLER(WM_CREATE, OnCreate)
		  MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		  MESSAGE_HANDLER(WM_SIZE, OnSize)
		  MESSAGE_HANDLER(WM_PAINT, OnPaint)
		  MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
		  MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		  MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)

		  MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		  MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		  MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		  MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)	
		  MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		  MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		  MESSAGE_HANDLER(WM_XBUTTONUP, OnXButtonUp)		

		  COMMAND_ID_HANDLER(ID_MODE_FITTOWINDOW, OnModeFitToWindow)
		  COMMAND_ID_HANDLER(ID_MODE_SCALEDOWNTOFIT, OnModeScaleDown)
		  COMMAND_ID_HANDLER(ID_MODE_SCALEUPTOFIT, OnModeScaleUp)
		  COMMAND_ID_HANDLER(ID_MODE_ACTUALSIZE, OnModeActualSize)

		  NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnToolbarDropDown)

	  END_MSG_MAP()

	  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->_frames.push_back(&_frameNavigation);
		  _state.Image.EditModeDelegates.Bind<T>(pT, &T::OnEditModeChanged);
		  _state.Folder.RefreshDelegates.Bind(this, &T::OnFolderRefresh);
		  bHandled = false;
		  return 0;
	  }

	  void OnEditModeChanged()
	  {
		  T *pT = static_cast<T*>(this);
		  pT->OnResetFrames();
	  }

	  void OnFolderRefresh()
	  {
		  CWaitCursor wait;
		  if (_state.Image.UpdateFileTime())
		  {
			  CString str;
			  str.Format(_T("Image '%s' has changed on disk.\nDo you want to reload it?"), IW::Path::FindFileName(_state.Image.GetImageFileName()));

			  T *pT = static_cast<T*>(this);
			  int nRet = pT->MessageBox(str, _T("ImageWalker"), MB_YESNO);
			  if (nRet == IDYES)
			  {
				  _state.Image.Reload();
			  }
		  }
	  }
	  
	  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->SetShowCursor(true);
		  bHandled = false;
		  return 0;
	  }

	  LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	  {
		  // handled, no background painting needed
		  return 1;
	  }

	  LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->DoSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		  bHandled = FALSE;
		  return 0;
	  }

	  LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  if (wParam != 0)
		  {
			  pT->OnPaint((HDC)wParam);		
		  }
		  else
		  {
			  CPaintDC dc(pT->m_hWnd);
			  pT->OnPaint((HDC)dc);
		  }

		  return 0;
	  }

	  void OnPaint(CDCHandle dc)
	  {
		  T *pT = static_cast<T*>(this);
	  }	

	  LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  int nChar = (int) wParam;
		  pT->OnKeyDown(nChar);
		  return 0;
	  }

	  LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	  {
		  T* pT = static_cast<T*>(this);
		  ATLASSERT(::IsWindow(pT->m_hWnd));

		  int zDelta = ((short)HIWORD(wParam)) / 2;

		  if (App.ControlKeyDown)
		  {
			  pT->SetScale(GetScale() + (zDelta / 10));
		  }
		  else
		  {
			  if(_sizeDrawnImage.cy > _sizeDrawnImage.cx)
			  {
				  pT->ScrollTo(CPoint(_pointScroll.x, _pointScroll.y + zDelta));
			  }
			  else		
			  {
				  pT->ScrollTo(CPoint(_pointScroll.x + zDelta, _pointScroll.y));
			  }
		  }

		  return 0;
	  }

	  LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);		
		  CPoint point (GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		  _dwCursorHide = GetTickCount() + _delay;
		  pT->SetShowCursor(true); 

		  if (_pToolCurrent != 0)
		  {
			  _pToolCurrent->OnMouseMove(point);
		  }
		  else
		  {
			  pT->ShowMouseMoveFeedback(point);	
		  }

		  bHandled = false;
		  return 0;
	  }

	  LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	  {
		  T* pT = static_cast<T*>(this);
		  pT->ShowMouseMoveFeedback(CPoint(-1,-1));
		  bHandled = FALSE;
		  return 0;
	  }	  

	  LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		  if (_pToolCurrent != 0)
		  {
			  _pToolCurrent->OnLButtonUp(point);
			  _pToolCurrent = 0;

			  pT->Invalidate();
			  ReleaseCapture();			

			  pT->ShowMouseMoveFeedback(point);
		  }
		  else
		  {
			  bHandled = false;
		  }

		  _state.Image.UpdateStatusText(); 

		  return 0;
	  }	

	  LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	  {
		  T *pT = static_cast<T*>(this);
		  CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		  pT->SetFocus();

		  bool bHandled = false;
		  pT->Accept(FrameVisitorMouseLeftButtonDown(point, bHandled));

		  if (!bHandled)
		  {
			  ImageTool *pTool = pT->GetToolFromLButtonDown(point);

			  if (pTool)
			  {
				  _pToolCurrent = pTool;
				  SetCursor(_pToolCurrent->GetCursor());
				  _pToolCurrent->OnLButtonDown(point);
				  pT->SetCapture();
			  }
		  }

		  return 0;
	  }


	  LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->ToggleScale();
		  return 0;
	  }

	  LRESULT OnXButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	  {
		  int fwKeys = GET_KEYSTATE_WPARAM (wParam); 
		  int fwButton = GET_XBUTTON_WPARAM (wParam); 

		  switch(fwButton)
		  {
		  case XBUTTON1:
			  _state.Image.PreviousImage();
			  break;
		  case XBUTTON2:
			  _state.Image.NextImage();
			  break;	
		  }

		  return 0;
	  }



	  void DrawControls(CDCHandle &dc)
	  {
		  if (!_rectSelected.IsRectEmpty())
		  {
			  CRect rect = GetDeviceRect();
			  dc.DrawFocusRect(rect);
			  dc.DrawFocusRect(GetCentreThirdRect(rect));
		  }
	  }

	  void OnTimer()
	  {
		  T *pT = static_cast<T*>(this);

		  // Is the image animated?
		  if (!IsAnimationPaused() && pT->m_hWnd &&  CanAnimate())
		  {
			  _nTimer -= 100;

			  if (_nTimer <= 0)
			  {
				  // Work out page
				  int nPage = (_nFrame + 1) % pT->GetRenderImage().GetPageCount();
				  IW::Page page = pT->GetRenderImage().GetPage(nPage); 

				  _nTimer = page.GetTimeDelay();
				  _nFrame = nPage;

				  InvalidateDib();
			  }
		  }

		  if (!IsCursorOverThisWindow())
		  {
			  _dwCursorHide = GetTickCount() + _delay;
		  }

		  pT->SetShowCursor(
			  (_dwCursorHide > GetTickCount()) ||
			  App.Options._bDontHideCursor);

		  pT->Accept(FrameVisitorTimer()); 
	  }

	  bool IsCursorOverThisWindow()
	  {
		  T *pT = static_cast<T*>(this);
		  CPoint pt;

		  return ::GetCursorPos(&pt) &&
			  ::WindowFromPoint(pt) == pT->m_hWnd;
	  }

	  CRect GetImageRect() const
	  {
		  return CRect(_pointOffset, _sizeDrawnImage);
	  }

	  void InvalidateDib()
	  {
		  T *pT = static_cast<T*>(this);
		  pT->InvalidateRect(GetImageRect());
	  }

	  void DoAnimation()
	  {
		  T *pT = static_cast<T*>(this);

		  if (_state.Image.IsImageShown())
		  {
			  int nPageCount = pT->GetRenderImage().GetPageCount();

			  if (nPageCount > 1)
			  {
				  _nFrame = _nFrame % nPageCount;
				  InvalidateDib();
			  }
		  }
	  }

	  void DrawImage(IW::CRender &render, const CRect &rectClip)
	  {
		  T *pT = static_cast<T*>(this);		  

		  if (_state.Image.IsImageShown())
		  {
			  const IW::Image &image = pT->GetRenderImage();
			  IW::Page page = image.GetFirstPage();

			  if (image.CanAnimate())
			  {
				  page = image.GetPage(_nFrame);
			  }

			  pT->DrawImage(render, page, rectClip);
		  }
		  else
		  {
			  //DrawLogo(render);
		  }		  

		  pT->DrawTools(render);
	  }	 

	  void DrawTools(IW::CRender &render)
	  {
	  }

	  void DrawLogo(IW::CRender &render)
	  {
		  T *pT = static_cast<T*>(this);

		  CRect rectClient;
		  pT->GetClientRect(rectClient);

		  CPoint pointCenter = rectClient.CenterPoint();

		  CRect rectBackGround(pointCenter.x - 56, pointCenter.y - 40, pointCenter.x + 56, pointCenter.y + 40);
		  CPoint pointLogo(pointCenter.x - 16, pointCenter.y - 28);
		  CRect rectText(rectBackGround); rectText.top = pointCenter.y + 10;

		  render.DrawRect(rectBackGround, IW::Style::Color::TaskFrame, IW::Style::Color::TaskBackground, 1);
		  render.DrawIcon(IW::Style::Icon::ImageWalker, pointLogo.x, pointLogo.y, 32, 32);
		  render.DrawString(_T("ImageWalker"), rectText, IW::Style::Font::Heading, IW::Style::Text::NormalCentre, IW::Style::Color::TaskText);
	  }

	  CRect GetDrawnImageRect() const
	  {
		  const T *pT = static_cast<const T*>(this);
		  const IW::Image &image = pT->GetRenderImage();
		  return GetDrawnImageRect(image.GetBoundingRect(), image.GetFirstPage().GetPageRect());
	  }

	  CRect GetDrawnImageRect(const CRect &rcBounding, const CRect &rcPage) const
	  {
		  const T *pT = static_cast<const T*>(this);

		  const CPoint pt(
			  _pointOffset.x + MulDiv(rcPage.left - rcBounding.left, _sizeDrawnImage.cx, rcBounding.Width()),
			  _pointOffset.y + MulDiv(rcPage.top - rcBounding.top, _sizeDrawnImage.cy, rcBounding.Height()));

		  const CSize size(
			  MulDiv(rcPage.Width(), _sizeDrawnImage.cx, rcBounding.Width()),
			  MulDiv(rcPage.Height(), _sizeDrawnImage.cy, rcBounding.Height()));

		  return CRect(pt - CSize(pT->GetScrollOffset()), size);
	  }


	  void DrawImage(IW::CRender &render, IW::Page &page, const CRect &rectClip) const
	  {
		  const T *pT = static_cast<const T*>(this);

		  const CRect rcPage = page.GetPageRect();
		  const CRect rcBounding = pT->GetRenderImage().GetBoundingRect();
		  const CRect rcDraw = GetDrawnImageRect(rcBounding, rcPage);

		  render.DrawImage(page, rcDraw);			
	  }

	  void NextFrame()
	  {
		  T *pT = static_cast<T*>(this);

		  if (CanAnimate())
		  {
			  _bPauseAnimation = true;
			  _nFrame++;
			  DoAnimation();
		  }
	  }

	  void PreviousFrame()
	  {
		  T *pT = static_cast<T*>(this);

		  if (CanAnimate())
		  {
			  _bPauseAnimation = true;
			  _nFrame -= 1;
			  DoAnimation();
		  }
	  }

	  bool IsAnimationPaused() const
	  {
		  return _bPauseAnimation;
	  }	

	  void SetAnimationPaused(bool bPause)
	  {
		  _bPauseAnimation = bPause;
	  }

	  void ResetAnimation()
	  {
		  _nTimer = 0;
		  _nFrame = 0;
	  }

	  void OnShowCursor()
	  {
	  }	

	  void SetShowCursor(bool bShow)
	  {		
		  T *pT = static_cast<T*>(this);



		  if (App.CursorShown != bShow)
		  {
			  App.CursorShown = bShow;			
			  ::ShowCursor(bShow ? TRUE : FALSE);

			  pT->OnShowCursor();
		  }	
	  }

	  void SetScale(ScaleMode::Type type)
	  {
		  T *pT = static_cast<T*>(this);

		  _scale.SetScale(type);
		  SetScrollSizes(true);	
		  pT->OnScaleChange();
	  }

	  void SetScale(LPCTSTR szScale)
	  {
		  T *pT = static_cast<T*>(this);

		  if (_scale.Parse(szScale))
		  {
			  SetScrollSizes(true);
			  pT->OnScaleChange();
		  }
	  }

	  void SetScale(int s)
	  {
		  T *pT = static_cast<T*>(this);

		  _scale.SetScale(s);
		  SetScrollSizes(true);
		  pT->OnScaleChange();
	  }	  

	  ScaleMode::Type GetScaleType() const 
	  {
		  return _scale.GetScaleType();
	  }

	  int GetScale() const
	  {
		  const T *pT = static_cast<const T*>(this);
		  CSize sizeImage(0,0);

		  if (_state.Image.IsImageShown())
		  {
			  const CRect rectImage = GetRenderImage().GetBoundingRect();
			  sizeImage = rectImage.Size();
		  }

		  return _scale.CalcScalePercent(sizeImage, pT->GetImageClientRect().Size(), _sizeClient);
	  }

	  void SetScrollSizes(bool bScrollToCenter)
	  {	
		  T *pT = static_cast<T*>(this);

		  CRect rectClient; pT->GetClientRect(rectClient);
		  CRect rectImageDisplay = pT->GetImageDisplayRect();

		  CSize sizeDrawnImage = _scale.CalcSize(GetRenderImage().GetBoundingRect().Size(), pT->GetImageClientRect().Size(), rectClient.Size());

		  _sizeAll = rectImageDisplay.TopLeft() + sizeDrawnImage;
		  _sizeDrawnImage = sizeDrawnImage;
		  _pointOffset = rectImageDisplay.TopLeft();

		  int x = ((rectImageDisplay.left + rectImageDisplay.right) - sizeDrawnImage.cx) / 2;
		  int y = ((rectImageDisplay.top + rectImageDisplay.bottom) - sizeDrawnImage.cy) / 2;

		  CPoint pointScrollOffset(0,0);

		  if (x < rectImageDisplay.left)
		  {
			  _pointOffset.x = rectImageDisplay.left;
			  pointScrollOffset.x = rectImageDisplay.left - x;
		  }
		  else
		  {
			  _pointOffset.x = x;
		  }

		  if (y < rectImageDisplay.top)
		  {
			  _pointOffset.y = rectImageDisplay.top;
			  pointScrollOffset.y = rectImageDisplay.top - y;
		  }
		  else
		  {
			  _pointOffset.y = y;
		  }

		  ScrollTo(bScrollToCenter ? pointScrollOffset : _pointScroll);
	  }

	  void DoSize(int cx, int cy)
	  {
		  T *pT = static_cast<T*>(this);

		  _sizeClient.cx = cx;
		  _sizeClient.cy = cy;	

		  SetScrollSizes(_scale.IsResizing());
		  pT->OnSizeChanged();
	  }

	  void DoSize()
	  {
		  SetScrollSizes(true);
	  }

	  void OnSizeChanged()
	  {
		  T *pT = static_cast<T*>(this);

		  if (_scale.IsResizing())
		  {
			  SetScrollSizes(false);
			  pT->OnScaleChange();
		  }
	  }

	  inline int ClampRange(int v, int l, int h)	
	  {
		  if (l > h)
		  {
			  h = l;
		  }

		  return v < l ? l : (v > h ? h : v);
	  }

	  void ScrollTo(const CPoint &point)
	  {
		  T *pT = static_cast<T*>(this);

		  CSize sizeMax = _sizeAll - _sizeClient;

		  _pointScroll.x = ClampRange(point.x, 0, sizeMax.cx);
		  _pointScroll.y = ClampRange(point.y, 0, sizeMax.cy);

		  pT->Invalidate();
	  }

	  bool CanImageBeScrolled() const
	  {
		  const T *pT = static_cast<const T*>(this);

		  if (_state.Image.IsImageShown())
		  {
			  CSize sizeMax = _sizeAll - _sizeClient;
			  return sizeMax.cx > 0 || sizeMax.cy > 0;
		  }

		  return false;
	  }




	  LRESULT OnModeScaleUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->SetScale(ScaleMode::Up);
		  return 0;
	  }

	  LRESULT OnModeScaleDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->SetScale(ScaleMode::Down);
		  return 0;
	  }

	  LRESULT OnModeFitToWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->SetScale(GetScaleType() == ScaleMode::Fit ? ScaleMode::Normal : ScaleMode::Fit);
		  return 0;
	  }

	  LRESULT OnModeActualSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	  {
		  T *pT = static_cast<T*>(this);
		  pT->SetScale(100);
		  return 0;
	  }

	  LRESULT OnToolbarDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
	  {
		  NMTOOLBAR* ptb = (NMTOOLBAR *) pnmh;
		  CRect rc;

		  CToolBarCtrl tbar(pnmh->hwndFrom);
		  tbar.GetItemRect(tbar.CommandToIndex(ptb->iItem), rc);
		  tbar.MapWindowPoints(HWND_DESKTOP, rc);

		  if (ptb->iItem == ID_NAVIGATE) // Navigation
		  {
			  DoNavigate(rc);
		  }

		  return 0;
	  }

	  void DoNavigate(const CRect &rect)
	  {
		  T *pT = static_cast<T*>(this);

		  if (_state.Image.IsImageShown())
		  {
			  CImageNavigation<T>::Track(*pT, pT->m_hWnd, rect);
		  }
	  }

	  void SetDeviceRect(const CRect &rectIn)
	  {
		  // Add both offsets together
		  CRect rectImage = GetImageRect();
		  rectImage.OffsetRect(-_pointScroll.x, -_pointScroll.y);

		  IW::Page page = GetRenderImage().GetFirstPage(); 

		  // Calculate the new rect
		  _rectSelected.left = MulDiv(rectIn.left - rectImage.left, page.GetWidth(), rectImage.Width());
		  _rectSelected.top = MulDiv(rectIn.top - rectImage.top, page.GetHeight(), rectImage.Height());
		  _rectSelected.right = MulDiv(rectIn.right - rectImage.left, page.GetWidth(), rectImage.Width());
		  _rectSelected.bottom = MulDiv(rectIn.bottom - rectImage.top, page.GetHeight(), rectImage.Height());
	  }

	  CRect GetDeviceRect() const
	  {
		  if (_rectSelected.IsRectEmpty())
		  {
			  const CRect rectEmpty(0,0,0,0);
			  return rectEmpty;
		  }
		  else
		  {
			  const CSize sizeDrawnImage = GetDrawnImageSize();
			  const CPoint pointDrawnImageOffset = _pointOffset;

			  // Add both offsets together
			  CPoint pointOffset;
			  pointOffset.x = pointDrawnImageOffset.x - _pointScroll.x;
			  pointOffset.y = pointDrawnImageOffset.y - _pointScroll.y;

			  IW::Page page = GetRenderImage().GetFirstPage(); 

			  CRect r;

			  r.left = pointOffset.x + MulDiv(_rectSelected.left,
				  sizeDrawnImage.cx, page.GetWidth());

			  r.top = pointOffset.y + MulDiv(_rectSelected.top,
				  sizeDrawnImage.cy, page.GetHeight());

			  r.right = pointOffset.x + MulDiv(_rectSelected.right, 
				  sizeDrawnImage.cx, page.GetWidth());

			  r.bottom = pointOffset.y + MulDiv(_rectSelected.bottom, 
				  sizeDrawnImage.cy, page.GetHeight());

			  return r;
		  }

	  }

	  void DoModeZoom()
	  {
		  T *pT = static_cast<T*>(this);

		  if (!_rectSelected.IsRectEmpty() && IsImageShown())
		  {
			  CRect rectClient;
			  pT->GetClientRect(rectClient);

			  IW::Page page = GetRenderImage().GetFirstPage(); 		
			  CRect rectSel = CRect(CPoint(0,0), page.GetSize()) & _rectSelected;

			  int cx = MulDiv(rectClient.Width(), 100, rectSel.Width());
			  int cy = MulDiv(rectClient.Height(), 100, rectSel.Height());

			  int s = IW::Min(cx, cy);

			  CPoint pointCenter = rectSel.CenterPoint();
			  SetScale(s);

			  _pointScroll.x = MulDiv(pointCenter.x, s, 100) - (rectClient.Width() / 2);
			  _pointScroll.y = MulDiv(pointCenter.y, s, 100) - (rectClient.Height() / 2);

			  if (_pointScroll.x < 0) _pointScroll.x  = 0;
			  if (_pointScroll.y < 0) _pointScroll.y  = 0;

			  int cxMax = _sizeAll.cx - _sizeClient.cx;
			  int cyMax = _sizeAll.cy - _sizeClient.cy;

			  if(_pointScroll.x > cxMax)
			  {
				  _pointScroll.x = cxMax;
			  }
			  if(_pointScroll.y > cyMax)
			  {
				  _pointScroll.y = cyMax;
			  }
			  pT->Invalidate();
		  }
	  }

	  void ToggleScale(bool canDown = false)
	  {
		  T *pT = static_cast<T*>(this);

		  _scale.Toggle(canDown);
		  SetScrollSizes(true);
		  pT->OnScaleChange();
	  }

	  CPoint GetScrollOffset() const
	  {
		  return _pointScroll;
	  }

	  CSize GetClientSize() const
	  {
		  return _sizeClient;
	  }

	  CSize GetCanvasSize() const
	  {
		  return _sizeAll;
	  }

	  void Refresh(bool bScrollToCenter)
	  {
		  T *pT = static_cast<T*>(this);
		  ResetAnimation();
		  _rectSelected.SetRectEmpty();
		  SetScrollSizes(bScrollToCenter);
	  }

	  CString GetScaleText()
	  {
		  return _scale.GetScaleText();
	  }

	  CRect GetImageRectSelected() const
	  {
		  CRect r(0, 0, 0, 0);

		  if (!_rectSelected.IsRectEmpty())
		  {
			  return _rectSelected;
		  }
		  else if (_state.Image.IsImageShown())
		  {
			  r = GetRenderImage().GetBoundingRect();
		  }

		  return r;
	  }

	  bool HasSelection() const
	  {
		  return _state.Image.IsImageShown() && !_rectSelected.IsRectEmpty();
	  };

	  BOOL PreTranslateMessage(MSG* pMsg)
	  {	
		  T *pT = static_cast<T*>(this);
		  return FALSE;
	  }

	  void ShowMouseMoveFeedback()
	  {
		  T *pT = static_cast<T*>(this);
		  CPoint point;
		  ::GetCursorPos(&point);

		  pT->ScreenToClient(&point);
		  pT->ShowMouseMoveFeedback(point);
	  }

	  void ShowMouseMoveFeedback(const CPoint &point)
	  {
		  T *pT = static_cast<T*>(this);

		  if (pT->PointOnFrame(point))
			  return;
		  
		  CRect r = pT->GetDeviceRect();

		  if (r.PtInRect(point))
		  {
			  SetCursor(IW::Style::Cursor::Move);
		  }
		  else if (IsInDragMode())
		  {
			  SetCursor(IW::Style::Cursor::HandUp);
		  }
		  else
		  {					
			  SetCursor(IW::Style::Cursor::Normal);
		  }
	  }

	  ImageTool *GetToolFromLButtonDown(const CPoint &point)
	  {
		  T *pT = static_cast<T*>(this);

		  if (_state.Image.IsImageShown())
		  {
			  CRect r = pT->GetDeviceRect();

			  if (r.PtInRect(point))
			  {					
				  return &_toolMoveSelection;				  
			  }
			  else if (IsInDragMode())
			  {
				  return &_toolMoveImage;				  
			  }
			  else
			  {
				  return &_toolSelect;
			  }			  
		  }

		  return 0;
	  }

	  bool IsInDragMode() const
	  {
		  const T *pT = static_cast<const T*>(this);
		  return !App.ControlKeyDown && _state.Image.IsImageShown() && pT->CanImageBeScrolled();
	  }

	  void ShowFrames()
	  {
		  _frameNavigation.SetVisible(CanShowNavigation());
	  }

	  CRect GetImageDisplayRect() const
	  {
		  const T *pT = static_cast<const T*>(this);
		  CRect rectClient;		
		  pT->GetClientRect(rectClient);
		  return rectClient;
	  }

	  CRect GetImageClientRect() const
	  {
		  const T *pT = static_cast<const T*>(this);
		  CRect rectClient;		
		  pT->GetClientRect(rectClient);
		  return rectClient;
	  }

	  bool CanShowNavigation() const 
	  {
		  return _state.Image.IsImageShown() && 
			  CanNavigate();
	  }
};

