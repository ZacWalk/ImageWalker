#pragma once

#include "Layout.h"

class FrameEditableProperty : public Frame
{
protected:
	FrameText _value;
	FrameText _title;

public:

	FrameEditableProperty(IFrameParent *pParent, const CString &strValueKey, const CString &strTitle, const CString &strDescription, const CString &strValue, DWORD dwFlags) : 
	  Frame(pParent), _title(pParent, strTitle, IW::Style::Color::TaskTextBold), _value(pParent, strValue)
	{
		_title._stringformat = IW::Style::Text::Property;
	}
	
	~FrameEditableProperty()
	{
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		_rect = rectIn;
		CRect rectTitle(rectIn);
		CRect rectValue(rectIn);

		int cxTitle = IW::Min(120, rectIn.Width() / 2);
		rectTitle.right -= cxTitle + padding;
		rectValue.left += cxTitle + padding;

		_title.SetPosition(positions, render, rectTitle);
		_value.SetPosition(positions, render, rectValue);

		_rect.bottom = _rect.top + IW::Max(_title._rect.Height(), _value._rect.Height());	

		_title._rect.OffsetRect(cxTitle - padding - _title._rect.Width(), 0);
	}

	void Accept(FrameVisitor &visitor) 
	{
		visitor.Visit(this); 
		
		_title.Accept(visitor);
		_value.Accept(visitor);
		
		visitor.AfterVisit(this);
	};

};

class FrameDelay : public Frame, public FrameSlider::IChange
{
public:

	FrameText _title;
	FrameSlider _slider;
	FrameText _text;	

	FrameDelay(IFrameParent *pParent) : Frame(pParent), 
		_slider(pParent, this), 
		_title(pParent, _T("Delay:")), 
		_text(pParent)
	{
		_slider.SetRange(0, 30, false);
		_slider.SetPos(App.Options.m_nDelay, false);

		UpdateText();
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		const int cxTitle = _title.GetSize(render, 100).cx;
		const int cxText = cxTitle;

		CRect rectTitle(rectIn.left, rectIn.top + padding, rectIn.left + cxTitle, rectIn.bottom);
		CRect rectSlider(rectIn.left + cxTitle,  rectIn.top, rectIn.right - cxText, rectIn.bottom);
		CRect rectDelay(rectIn.right - cxText, rectIn.top + padding, rectIn.right, rectIn.bottom);

		_title.SetPosition(positions, render, rectTitle);	
		_slider.SetPosition(positions, render, rectSlider);	
		_text.SetPosition(positions, render, rectDelay);
		_text._rect.right = _text._rect.left + cxText;

		_rect = rectIn;
		_rect.bottom = _rect.top + 16; 
	}

	void Accept(FrameVisitor &visitor) 
	{
		visitor.Visit(this); 
		
		_title.Accept(visitor);
		_slider.Accept(visitor);
		_text.Accept(visitor);
		
		visitor.AfterVisit(this);
	}

	void PosChanged(FrameSlider *pSender, int pos)
	{
		App.Options.m_nDelay = pos;
		UpdateText();
	}

	void UpdateText()
	{
		CString str;
		str.Format(_T("%ds"), App.Options.m_nDelay);
		_text.SetText(str);

		_pParent->InvalidateRect(_rect);
	}
};



class FrameEditableText : 
	public FrameHover<FrameTextT<FrameEditableText> >
{
public:

	typedef FrameHover<FrameTextT<FrameEditableText> > BaseClass;

	class IChange
	{
	public:
		virtual void OnEditText(FrameEditableText *pChange) = 0;
	};

	CString _str;
	IW::Style::Text::Style _stringformat;
	IChange *_pChangeEvents;	

	FrameEditableText(IFrameParent *pParent, IChange *pChangeEvents = 0) : 
		BaseClass(pParent),
		_pChangeEvents(pChangeEvents),
		_stringformat(IW::Style::Text::Normal)
	{
	}

	int GetMaxTextLengt() const
	{
		return INT_MAX;
	}

	void SetText(const CString &str)
	{	
		_str = str;
		_pParent->ResetLayout();
	}

	CString GetText() const
	{
		return _str.IsEmpty() ? _T("Add description") : _str;
	}

	DWORD GetRenameWindowStyle() const
	{
		return WS_BORDER | WS_CHILD | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL;
	}

	IW::Style::Font::Type GetFont()
	{
		return _bHover ? IW::Style::Font::LinkHover : IW::Style::Font::Link;
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		_rect = render.MeasureString(GetText(), rectIn, GetFont(), _stringformat);
		//_rect.OffsetRect(0, rectIn.Height() - _rect.Height()); 
	}

	void MouseLeftButtonUp(const CPoint &point)
	{
		if (_rect.PtInRect(point))
		{
			if (_pChangeEvents) _pChangeEvents->OnEditText(this);
		}
	}
};

template<class THost>
class FrameScale : public Frame, public FrameSlider::IChange
{
public:

	FrameText _title;
	FrameSlider _slider;
	FrameTextCommand _text;	

	THost *_pHost;

	FrameScale(IFrameParent *pParent, THost *pHost) : 
		_pHost(pHost), 
		Frame(pParent),
		_slider(pParent, this, App.Options.m_nDelay), 
		_title(pParent, _T("Scale:")), 
		_text(pParent, _T(""), ID_SCALE_TOGGLE)
	{
		OnScaleChange();
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		const int cxTitle = _title.GetSize(render, 100).cx;
		const int cxText = cxTitle;

		CRect rectTitle(rectIn.left, rectIn.top + padding, rectIn.left + cxTitle, rectIn.bottom);
		CRect rectSlider(rectIn.left + cxTitle,  rectIn.top, rectIn.right - cxText, rectIn.bottom);
		CRect rectDelay(rectIn.right - cxText, rectIn.top + padding, rectIn.right, rectIn.bottom);

		_title.SetPosition(positions, render, rectTitle);	
		_slider.SetPosition(positions, render, rectSlider);	
		_text.SetPosition(positions, render, rectDelay);
		_text._rect.right = _text._rect.left + cxText;

		_rect = rectIn;
		_rect.bottom = _rect.top + 16; 
	}

	void Accept(FrameVisitor &visitor) 
	{
		visitor.Visit(this); 
		
		_title.Accept(visitor);
		_slider.Accept(visitor);
		_text.Accept(visitor);
		
		visitor.AfterVisit(this);
	}

	void PosChanged(FrameSlider *pSender, int pos)
	{
		_pHost->SetScale(pos);
	}

	void OnScaleChange()
	{
		_text.SetText(_pHost->GetScaleText());
		Invalidate();
	}	
};


class  FrameButtonBar : public FrameArray
{
public:	

	enum { padding = 4 };
	
	class FrameButton : public FrameTracking<FrameHover<Frame, true> >
	{
	public:

		typedef FrameTracking<FrameHover<Frame, true> > BaseClass;

		int _id;
		int _nImage;

		FrameButton(IFrameParent *pParent, const CString &str, int id, int nImage) : 
			BaseClass(pParent), 
			_id(id),
			_nImage(nImage)
		{
		}

		void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
		{
			_rect = rectIn;
			_rect.left = _rect.right - (16 + (padding * 2));
			_rect.top = _rect.bottom - (16 + (padding * 2));
		}

		void Render(IW::CRender &render)
		{
			int x = _rect.left + padding;
			int y = _rect.top + padding;

			if (_bTracking)
			{
				x += 1;
				y += 1;
			}

			render.DrawImageList(App.GetGlobalBitmap(), _nImage, x, y, 16, 16);
		}

		void Erase(IW::CRender &render)
		{
			if (_bHover || _bTracking)
			{
				render.DrawRect(_rect, IW::Emphasize(IW::Style::Color::Highlight), IW::Style::Color::Highlight, 1, true, _bTracking);
			}
		}

		void GetToolTipId(int &id, bool &bHandled)
		{
			if (_bHover)
			{
				id = _id;
				bHandled = true;
			}
		}

		void MouseLeftButtonUp(const CPoint &point)
		{
			if (_rect.PtInRect(point) && _bTracking)
			{
				_pParent->SignalCommand(_id);
			}
			SetTracking(false);
		}
	};

	FrameButtonBar(IFrameParent *pParent) : FrameArray(pParent)
	{
	}

	void AddLink(const CString &str, int id, int nImage)
	{
		FrameButton *pCommand = new FrameButton(_pParent, str, id, nImage);
		AddFrame(pCommand);
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		CRect r = _rect = rectIn;
		int top = r.bottom;
		int left = r.right;

		for(FRAMES::iterator i = _frames.begin(); i != _frames.end(); i++)
		{
			Frame *pFrame = *i;
			pFrame->SetPosition(positions, render, r);
			left = pFrame->_rect.left;
			r.right = left;
			top = IW::Min(top, pFrame->_rect.top);
		} 

		_rect.left = left;
		_rect.top = top;
	}
};

template<class THost>
class FrameSlideshow : public FrameHover<Frame, false>
{
protected:
    
public:

	THost *_pHost;	
	FrameScale<THost> _scale;
	FrameDelay _delay;
	FrameButtonBar _links;
	ScalarAnimation _fade;
	ScalarAnimation _grow;

	FrameSlideshow(IFrameParent *pParent, THost *pHost, State &state) : 
		_pHost(pHost),
		FrameHover<Frame, false>(pParent), 
		_links(pParent),
		_scale(pParent, pHost),
		_delay(pParent),		
		_fade(0xA0, 0xF0, 0x10),
		_grow(50, 100, 20)
	{		
		_bLayered = true;

		_links.AddLink(_T("Exit"), ID_SLIDESHOW_STOP, ImageIndex::SlideShowStop);
		_links.AddLink(_T("Undo"), ID_EDIT_UNDO_IMAGE, ImageIndex::Undo);
		_links.AddLink(_T("Rotate Left"), ID_FILTER_ROTATELEFT, ImageIndex::FilterRotateLeft);
		_links.AddLink(_T("Rotate Right"), ID_FILTER_ROTATERIGHT, ImageIndex::FilterRotateRight);
		_links.AddLink(_T("Crop"), ID_FILTER_CROP, ImageIndex::FilterCrop);
		_links.AddLink(_T("Move"), ID_FILE_MOVETO, ImageIndex::MoveTo);		
		_links.AddLink(_T("Copy"), ID_FILE_COPYTO, ImageIndex::CopyTo);
		_links.AddLink(_T("Delete"), ID_EDIT_DELETE, ImageIndex::Delete);
		_links.AddLink(_T("Next"), ID_SLIDESHOW_NEXTIMAGE, ImageIndex::ImageNext);
		_links.AddLink(_T("Pause"), ID_SLIDESHOW_PAUSE, ImageIndex::SlideShowPause);
		_links.AddLink(_T("Play"), ID_SLIDESHOW_PLAY, ImageIndex::SlideShowPlay);
		_links.AddLink(_T("Previous"), ID_SLIDESHOW_PREVIOUSIMAGE, ImageIndex::ImageBack);
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		CRect rectFrames(rectIn); 
		rectFrames.DeflateRect(paddingLarge, paddingLarge);
		_rect = rectFrames;

		int cy = 16 + (padding * 2);

		CRect rectLinks = rectFrames;
		rectLinks.DeflateRect(padding, padding);
		rectLinks.top = rectFrames.bottom - cy;
		_links.SetPosition(positions, render, rectLinks);


		CRect rectSlider = _links._rect; 
		rectSlider.OffsetRect(0, -cy);
		_delay.SetPosition(positions, render, rectSlider);

		rectSlider.OffsetRect(0, -cy);
		_scale.SetPosition(positions, render, rectSlider);
		
		_rect.top = rectSlider.top;
		_rect.left = rectSlider.left;
		_rect.InflateRect(paddingLarge, paddingLarge);
	}

	void Render(IW::CRender &renderIn)
	{
		IW::CRender render;
		IW::CDCRender dc(renderIn);

		if (render.Create(dc, _rect))
		{
			render.DrawRect(_rect, IW::Style::Color::TaskFrame, IW::Style::Color::TaskBackground, 1, true);
			AcceptChildren(FrameVisitorErase(render));
			AcceptChildren(FrameVisitorRender(render));

			CRect r = _rect;
			int pos = _grow.Pos();
			r.top = r.bottom - ((r.Height() * pos) / 100);
			r.left = r.right - ((r.Width() * pos) / 100);
			renderIn.DrawRender(render, _fade.Pos(), r);
		}
	}

	void HoverChanged(bool bHover)
	{
		_fade.SetDirection(bHover);
		_grow.SetDirection(bHover);
	}

	void Timer()
	{
		bool bFade = _fade.Animate();
		bool bGrow = _grow.Animate();
		if (bFade || bGrow)
			Invalidate();
	}

	void Accept(FrameVisitor &visitor) 
	{		
		visitor.Visit(this);
		AcceptChildren(visitor);
		visitor.AfterVisit(this);
	}

	void AcceptChildren(FrameVisitor &visitor) 
	{
		_links.Accept(visitor);
		_scale.Accept(visitor);
		_delay.Accept(visitor);
	}

	void OnScaleChange()
	{
		_scale.OnScaleChange();
	}
};


class FrameCommands : public FrameGroup
{
public:

	FrameCommands(IFrameParent *pParent) : FrameGroup(pParent, _T("Tasks"))
	{
		AddFrame(new FrameCommand(pParent, ImageIndex::Folders, _T("Folders"), ID_VIEW_FOLDERS));		
		AddFrame(new FrameCommand(pParent, ImageIndex::Web, _T("Web"), ID_VIEW_WEB));
		AddFrame(new FrameCommand(pParent, ImageIndex::Print, _T("Print"), ID_VIEW_PRINT));
		AddFrame(new FrameCommand(pParent, ImageIndex::SlideShow, _T("Image Full Screen"), ID_VIEW_IMAGEFULLSCREEN));
	}
};


class FrameHistorgram : public Frame
{
public:

	const IW::Histogram *_pHistogram;
	IW::Image _image;
	enum { ImageHeight = 100 };

	FrameHistorgram(IFrameParent *pParent) : Frame(pParent), _pHistogram(0)
	{
		_image.CreatePage(IW::Histogram::MaxValue, ImageHeight, IW::PixelFormat::PF32);
	}

	virtual ~FrameHistorgram() 
	{
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn) 
	{
		_rect = rectIn;
		_rect.bottom = _rect.top + 80;
	};

	void SetHistogram(const IW::Histogram &histogram)
	{
		_pHistogram = &histogram;
		RenderBitmap();
	}

	void RenderBitmap()
	{
		float max = fastSqrt((float)_pHistogram->_max);	
		float yratio = ImageHeight / max;

		for(int y = 0; y < ImageHeight; y++)
		{
			LPCOLORREF pLine = (LPCOLORREF)_image.GetFirstPage().GetBitmapLine((ImageHeight - 1) - y);

			for(int x = 0; x < IW::Histogram::MaxValue; x++)
			{
				int r = (yratio * fastSqrt((float)_pHistogram->_r[x])) > y ? 0xFF : 0x00;
				int g = (yratio * fastSqrt((float)_pHistogram->_g[x])) > y ? 0xFF : 0x00;
				int b = (yratio * fastSqrt((float)_pHistogram->_b[x])) > y ? 0xFF : 0x00;

				pLine[x] = IW::RGBA(r,g,b);
			}
		}
	}

	void Render(IW::CRender &render)
	{
		render.DrawImage(_image.GetFirstPage(), _rect);
	}

private:

	//sqrt(fX)
	inline float fastSqrt(float fX)
	{
		float tmp = fX;
		float fHalf = 0.5f*fX;
		int i = *reinterpret_cast<int*>(&fX);
		i = 0x5f3759df - (i >> 1); // This line hides a LOT of math!
		fX = *reinterpret_cast<float*>(&i);

		// repeat this statement for a better approximation
		fX = fX*(1.5f - fHalf*fX*fX); 
		fX = fX*(1.5f - fHalf*fX*fX);
		return tmp * fX;
	}

	
};

class FrameImageInfo : 
	public FrameGroup,
	public FrameEditableText::IChange
{
public:	

	FrameText _text;
	FrameHistorgram _histogram;	
	FrameEditableText _description;
	CString _str;
	State &_state;

	FrameImageInfo(IFrameParent *pParent, State &state) : 
		FrameGroup(pParent, _T("Image")),
		_text(pParent),
		_histogram(pParent),
		_description(pParent, this),
		_state(state)
	{
		AddFrame(&_histogram);
		AddFrame(&_text);
		AddFrame(&_description);
	}

	~FrameImageInfo()
	{
		Clear();
	}

	void Refresh()
	{
		_linkBar.RemoveAll();
		_linkBar.AddLink(_T("Close"), ID_VIEW_DESCRIPTION);
		_linkBar.AddLink(App.Options.ShowAdvancedImageDetails ? _T("Less") : _T("More") , ID_VIEW_ADVANCEDIMAGE);		

		_str.Empty();

		const IW::Image &image = _state.Image.GetImage(); 
		IW::CameraSettings settings = image.GetCameraSettings();

		SetText(_state.Image.GetTitle());
		_histogram.SetHistogram(_state.Image.GetHistogram());			

		AddProperty(_T("Aperture: "), settings.FormatAperture());
		AddProperty(_T("ISO: "), settings.FormatIsoSpeed());
		AddProperty(_T("Exposure: "), settings.FormatExposureTime());
		AddProperty(_T("Focal Length: "), settings.FormatFocalLength());

		_histogram.SetVisible(App.Options.ShowAdvancedImageDetails);
		_text.SetVisible(App.Options.ShowAdvancedImageDetails);
		_text.SetText(_str);
		_description.SetText(_state.Image.GetDescription());
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		CRect r = rectIn;
		r.right = r.left + 220;
		FrameGroup::SetPosition(positions, render, r);
	}

	void AddProperty(CString strTitle, CString strProperty)
	{
		if (!strProperty.IsEmpty())
		{
			if (!_str.IsEmpty()) _str += _T("\n");
			_str += strTitle;
			_str += strProperty;			
		}
	}	

	void OnEditText(FrameEditableText *pChange)
	{
		_pParent->SignalCommand(ID_IMAGE_EDITDESCRIPTION);
	}
};



class FrameMood : public FrameGroup, public FrameSlider::IChange
{
public:	

	class FrameBlackCheckBox : public FrameCheckBox
	{
	public:


		FrameBlackCheckBox(IFrameParent *pParent, const CString &strCaption, bool &bData) : FrameCheckBox(pParent, strCaption, bData)
		{
		}

		void MouseLeftButtonUp(const CPoint &point)
		{
			if (_rect.PtInRect(point))
			{
				_bData = !_bData;
				Invalidate();

				IW::Style::SetMood();
				InvalidateApplication();
			}
		}

		void InvalidateApplication()
		{
			CWindow wnd(_pParent->GetHWnd());
			wnd = wnd.GetTopLevelWindow();

			wnd.SetWindowRgn(NULL);
			wnd.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			wnd.RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_FRAME);
		}
	};



	FrameSlider _slider;
	FrameBlackCheckBox _black;
	FrameBlackCheckBox _skin;

	FrameMood(IFrameParent *pParent) : FrameGroup(pParent, _T("Mood")),
		_slider(pParent, this),
		_black(pParent, _T("Black background"), App.Options.BlackBackground),
		_skin(pParent, _T("Black skin"), App.Options.BlackSkin)
	{
		_slider.SetRange(0, 255, false);
		_slider.SetPos(App.Options.Mood, false);

		AddFrame(&_slider);
		AddFrame(&_black);
		AddFrame(&_skin);
	}

	~FrameMood()
	{
		Clear();
	}

	void Accept(FrameVisitor &visitor) 
	{		
		visitor.Visit(this);

		_title.Accept(visitor);
		_slider.Accept(visitor);
		_black.Accept(visitor);
		_skin.Accept(visitor);

		visitor.AfterVisit(this);
	};

	void PosChanged(FrameSlider *pSender, int pos)
	{
		App.Options.Mood = pos;
		IW::Style::SetMood();
		InvalidateApplication();
	}

	void InvalidateApplication()
	{
		CWindow wnd(_pParent->GetHWnd());
		wnd.GetTopLevelWindow().RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
	}

	CRect GetRenderRect() const
	{
		CRect r = _rect;
		int pos = _grow.Pos();
		int scale = 10;
		int cx = (r.Width() / scale) - ((r.Width() * pos) / (100 * scale));
		int cy = (r.Height() / scale) - ((r.Height() * pos) / (100 * scale));
		r.DeflateRect(cx, cy);
		return r;
	}
};



class FrameHelp : public FrameGroup
{
public:

	FrameHelp(IFrameParent *pParent) : FrameGroup(pParent, _T("ImageWalker"))
	{
		
	}

	void Refresh()
	{
		RemoveAll();

		if (App.Options._nRegistrationSettings == RegistrationSettings::Free)
		{
			AddFrame(new FrameText(_pParent, App.LoadString(IDS_START_FREE)));
			AddFrame(new FrameCommand(_pParent, ImageIndex::Help, _T("Click here for help"), ID_HELP_FINDER));
		}
		else if (App.Options._nRegistrationSettings == RegistrationSettings::Registered || 
			App.Options._nRegistrationSettings == RegistrationSettings::Registered2)
		{
			AddFrame(new FrameText(_pParent, App.LoadString(IDS_START_REGISTERED)));
			AddFrame(new FrameCommand(_pParent, ImageIndex::Help, _T("Click here for help"), ID_HELP_FINDER));
		}
		else
		{
			AddFrame(new FrameText(_pParent, App.LoadString(IDS_START_EVAL)));
			AddFrame(new FrameCommand(_pParent, ImageIndex::Register, _T("Click here to register"), ID_HELP_REGISTRATIONWEBPAGE));
		}		
	}

	CRect GetRenderRect() const
	{
		CRect r = _rect;
		int pos = _grow.Pos();
		int scale = 10;
		int cx = (r.Width() / scale) - ((r.Width() * pos) / (100 * scale));
		int cy = (r.Height() / scale) - ((r.Height() * pos) / (100 * scale));
		r.DeflateRect(cx, cy);
		return r;
	}
};

class FrameTip : public FrameGroup
{
public:

	FrameTip(IFrameParent *pParent) : FrameGroup(pParent, _T("Tip"))
	{
		static int tips[] = {IDS_TIP1, IDS_TIP2, IDS_TIP3, IDS_TIP4, IDS_TIP5, IDS_TIP6 };

		srand(GetTickCount());
		int tip = (rand() * countof(tips)) / RAND_MAX;

		AddFrame(new FrameText(pParent, App.LoadString(tips[tip])));		
	}

	CRect GetRenderRect() const
	{
		CRect r = _rect;
		int pos = _grow.Pos();
		int scale = 10;
		int cx = (r.Width() / scale) - ((r.Width() * pos) / (100 * scale));
		int cy = (r.Height() / scale) - ((r.Height() * pos) / (100 * scale));
		r.DeflateRect(cx, cy);
		return r;
	}
};


class FrameImageWalker : public FrameArray
{
public:
	FrameHelp _help;
	FrameMood _mood;
	FrameTip _tip;

	FrameImageWalker(IFrameParent *pParent) : FrameArray(pParent),
		_help(pParent),
		_mood(pParent),
		_tip(pParent)
	{
		
	}

	~FrameImageWalker()
	{
		Clear();
	}

	void Refresh()
	{
		Clear();

		AddFrame(&_help);
		AddFrame(&_mood);
		AddFrame(&_tip);

		_help.Refresh();
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		CRect r = rectIn;
		r.DeflateRect(20, 10);
		_rect = r;
		_help.SetPosition(positions, render, r);

		r.top = _help._rect.bottom;
		r.DeflateRect(20, 10);

		CRect rMood = r;
		CRect rTip = r;
		rMood.right = (rMood.left + rMood.right) / 2;
		rTip.left = (rTip.left + rTip.right) / 2;
		
		_mood.SetPosition(positions, render, rMood);
		_tip.SetPosition(positions, render, rTip);

		_rect.bottom = IW::Max(_mood._rect.bottom, _tip._rect.bottom);
	}
};
//
//class FrameHelp : public FrameGroup, public FrameSlider::IChange
//{
//public:
//
//	class FrameBlackCheckBox : public FrameCheckBox
//	{
//	public:
//
//
//		FrameBlackCheckBox(IFrameParent *pParent, const CString &strCaption, bool &bData) : FrameCheckBox(pParent, strCaption, bData)
//		{
//		}
//
//		void MouseLeftButtonUp(const CPoint &point)
//		{
//			if (_rect.PtInRect(point))
//			{
//				_bData = !_bData;
//				Invalidate();
//
//				IW::Style::SetMood();
//				InvalidateApplication();
//			}
//		}
//
//		void InvalidateApplication()
//		{
//			CWindow wnd(_pParent->GetHWnd());
//			wnd = wnd.GetTopLevelWindow();
//
//			wnd.SetWindowRgn(NULL);
//			wnd.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
//			wnd.RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_FRAME);
//		}
//	};
//
//	FrameTitle _mood;
//	FrameSlider _slider;
//	FrameBlackCheckBox _black;
//	FrameBlackCheckBox _skin;
//	FrameTitle _tip;
//
//	FrameHelp(IFrameParent *pParent) : FrameGroup(pParent, _T("ImageWalker")),
//		_slider(pParent, this),
//		_mood(pParent, _T("Mood")),
//		_tip(pParent, _T("Tip")),
//		_black(pParent, _T("Black background"), App.Options.BlackBackground),
//		_skin(pParent, _T("Black skin"), App.Options.BlackSkin)
//	{
//		if (App.Options._nRegistrationSettings == RegistrationSettings::Free)
//		{
//			AddFrame(new FrameText(pParent, App.LoadString(IDS_START_FREE)));
//			AddFrame(new FrameCommand(pParent, ImageIndex::Help, _T("Click here for help"), ID_HELP_FINDER));
//		}
//		else if (App.Options._nRegistrationSettings == RegistrationSettings::Registered || 
//			App.Options._nRegistrationSettings == RegistrationSettings::Registered2)
//		{
//			AddFrame(new FrameText(pParent, App.LoadString(IDS_START_REGISTERED)));
//			AddFrame(new FrameCommand(pParent, ImageIndex::Help, _T("Click here for help"), ID_HELP_FINDER));
//		}
//		else
//		{
//			AddFrame(new FrameText(pParent, App.LoadString(IDS_START_EVAL)));
//			AddFrame(new FrameCommand(pParent, ImageIndex::Register, _T("Click here to register"), ID_HELP_REGISTRATIONWEBPAGE));
//		}
//
//		_slider.SetRange(0, 255, false);
//		_slider.SetPos(App.Options.Mood, false);
//
//		AddFrame(&_mood);
//		AddFrame(&_slider);
//		AddFrame(&_black);
//		AddFrame(&_skin);
//		AddFrame(&_tip);
//
//		static int tips[] = {IDS_TIP1, IDS_TIP2, IDS_TIP3, IDS_TIP4, IDS_TIP5, IDS_TIP6 };
//
//		srand(GetTickCount());
//		int tip = (rand() * countof(tips)) / RAND_MAX;
//
//		AddFrame(new FrameText(pParent, App.LoadString(tips[tip])));
//	}
//
//	~FrameHelp()
//	{
//		Clear();
//	}
//
//	void PosChanged(FrameSlider *pSender, int pos)
//	{
//		App.Options.Mood = pos;
//		IW::Style::SetMood();
//		InvalidateApplication();
//	}
//
//	void InvalidateApplication()
//	{
//		CWindow wnd(_pParent->GetHWnd());
//		wnd.GetTopLevelWindow().RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
//	}
//
//	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
//	{
//		CRect r = rectIn;
//		r.DeflateRect(60, 60);
//		FrameGroup::SetPosition(positions, render, r);
//	}
//
//	CRect GetRenderRect() const
//	{
//		CRect r = _rect;
//		int pos = _grow.Pos();
//		int scale = 10;
//		int cx = (r.Width() / scale) - ((r.Width() * pos) / (100 * scale));
//		int cy = (r.Height() / scale) - ((r.Height() * pos) / (100 * scale));
//		r.DeflateRect(cx, cy);
//		return r;
//	}
//};


class FrameBigLink : public FrameHover<FrameTextT<FrameBigLink> >
{
public:

	typedef FrameHover<FrameTextT<FrameBigLink> > BaseClass;

	IW::Style::Text::Style _stringformat;
	CString _str;


	FrameBigLink(IFrameParent *pParent, const CString &str = g_szEmptyString) : 
		BaseClass(pParent),
		_str(str),
		_stringformat(IW::Style::Text::Normal)
	{
		_bHover = false;
		 
	}

	IW::Style::Font::Type GetFont()
	{
		return _bHover ? IW::Style::Font::BigHover : IW::Style::Font::Big;
	}

	const CString &GetText() const
	{
		return _str;
	}

	void SetText(const CString &str)
	{
		_str = str;
	}
};

template<class THost>
class FrameImageCtrl : 
	public FrameHover<Frame, false>
{
protected:

	FrameScale<THost> _scale;
	FrameButtonBar _links;
	ScalarAnimation _fade;
	ScalarAnimation _grow;
    
public:	

	FrameImageCtrl(IFrameParent *pParent, THost *pHost, State &state) : 
		FrameHover<Frame, false>(pParent), 
		_scale(pParent, pHost),
		_links(pParent),		
		_fade(0xA0, 0xF0, 0x10),
		_grow(50, 100, 20)
	{
		_bLayered = true;

		_links.AddLink(_T("Show"), ID_VIEW_IMAGEFULLSCREEN, ImageIndex::SlideShow);
		_links.AddLink(_T("Rotate Left"), ID_FILTER_ROTATELEFT, ImageIndex::FilterRotateLeft);
		_links.AddLink(_T("Rotate Right"), ID_FILTER_ROTATERIGHT, ImageIndex::FilterRotateRight);
		_links.AddLink(_T("Crop"), ID_FILTER_CROP, ImageIndex::FilterCrop);
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		CRect r(rectIn); 
		r.DeflateRect(paddingLarge, paddingLarge);		
		_rect = r;		

		CRect rectLinks = r;
		_links.SetPosition(positions, render, rectLinks);
		
		r.right = _links._rect.left - padding;
		r.top = _links._rect.top;
		int cx = IW::Min(r.Width(), 250);
		r.left = r.right - cx;
		_scale.SetPosition(positions, render, r);

		_rect.left = r.left;
		_rect.top = r.top;
		_rect.InflateRect(paddingLarge, paddingLarge);
	}

	void Render(IW::CRender &renderIn)
	{
		IW::CRender render;
		IW::CDCRender dc(renderIn);

		if (render.Create(dc, _rect))
		{
			render.DrawRect(_rect, IW::Style::Color::TaskFrame, IW::Style::Color::TaskBackground, 1, true);
			AcceptChildren(FrameVisitorErase(render));
			AcceptChildren(FrameVisitorRender(render));

			CRect r = _rect;
			int pos = _grow.Pos();
			r.top = r.bottom - ((r.Height() * pos) / 100);
			r.left = r.right - ((r.Width() * pos) / 100);
			renderIn.DrawRender(render, _fade.Pos(), r);
		}
	}

	void HoverChanged(bool bHover)
	{
		_fade.SetDirection(bHover);
		_grow.SetDirection(bHover);
	}

	void Timer()
	{
		bool bFade = _fade.Animate();
		bool bGrow = _grow.Animate();
		if (bFade || bGrow)
			Invalidate();
	}

	void OnScaleChange()
	{
		_scale.OnScaleChange();
	}

	void Accept(FrameVisitor &visitor) 
	{		
		visitor.Visit(this);
		AcceptChildren(visitor);		
		visitor.AfterVisit(this);
	}

	void AcceptChildren(FrameVisitor &visitor) 
	{
		_scale.Accept(visitor);
		_links.Accept(visitor);
	}
};

template<class THost>
class FrameImageNavigate : 
	public FrameTracking<FrameHover<Frame, true> >
{
private:
	State &_state;
	THost *_pHost;
	ScalarAnimation _fade;
	ScalarAnimation _grow;
    
public:	

	typedef FrameTracking<FrameHover<Frame, true> > BaseClass;

	FrameImageNavigate(IFrameParent *pParent, THost *pHost, State &state) : 
		BaseClass(pParent), _state(state), _pHost(pHost),		
		_fade(0xA0, 0xF0, 0x10), _grow(50, 100, 20)
	{
		_bLayered = true;
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		const IW::Image &image = _state.Image.GetThumbnailImage();

		CRect r(rectIn); 
		r.DeflateRect(1, 1);

		if (!image.IsEmpty())
		{
			CRect rectImage = _state.Image.GetThumbnailImage().GetBoundingRect();
			r.left = r.right - rectImage.Width();
			r.bottom = r.top + rectImage.Height();
		}

		r.InflateRect(1, 1);
		_rect = r;

	}

	CPoint GetPointDrawImage()
	{
		return CPoint( _rect.left + 1, _rect.top + 1);
	}
	
	CRect GetRectDrawImage()
	{
		const IW::Image &image = _state.Image.GetThumbnailImage();
		return CRect(GetPointDrawImage(), image.GetBoundingRect().Size());
	}

	CRect GetNavigationRect()
	{
		const IW::Image &image = _state.Image.GetThumbnailImage();

		CSize sizeCanvas = _pHost->GetCanvasSize();
		CRect rectClient(_pHost->GetScrollOffset(), _pHost->GetClientSize());
		CSize sizeWindow = image.GetBoundingRect().Size();

		return IW::MulDivRect(rectClient, sizeWindow, sizeCanvas) + GetPointDrawImage();
	}

	void Render(IW::CRender &renderIn)
	{
		IW::CRender render;
		IW::CDCRender dc(renderIn);

		if (render.Create(dc, _rect))
		{
			render.DrawRect(_rect, IW::Style::Color::TaskFrame, IW::Style::Color::TaskBackground, 1, true);
			RenderImage(render);

			CRect r = _rect;
			int pos = _grow.Pos();
			r.bottom = r.top + ((r.Height() * pos) / 100);
			r.left = r.right - ((r.Width() * pos) / 100);
			renderIn.DrawRender(render, _fade.Pos(), r);
		}
	}

	void HoverChanged(bool bHover)
	{
		_fade.SetDirection(bHover);
		_grow.SetDirection(bHover);
	}

	void Timer()
	{
		bool bFade = _fade.Animate();
		bool bGrow = _grow.Animate();
		if (bFade || bGrow)
			Invalidate();
	}

	void RenderImage(IW::CRender &render)
	{
		const IW::Image &image = _state.Image.GetThumbnailImage();

		if (!image.IsEmpty())
		{
			render.DrawImage(image.GetFirstPage(), GetPointDrawImage());

			const CRect rectImage = GetRectDrawImage();
			const CRect rectNavigation = IW::ClipRect(GetNavigationRect(), rectImage);

			render.Blend(RGB(128, 128, 128), CRect(rectImage.left, rectImage.top, rectImage.right, rectNavigation.top));
			render.Blend(RGB(128, 128, 128), CRect(rectImage.left, rectNavigation.bottom, rectImage.right, rectImage.bottom));
			render.Blend(RGB(128, 128, 128), CRect(rectImage.left, rectNavigation.top, rectNavigation.left, rectNavigation.bottom));
			render.Blend(RGB(128, 128, 128), CRect(rectNavigation.right, rectNavigation.top, rectImage.right, rectNavigation.bottom));
			render.DrawRect(rectNavigation, IW::Style::Color::Highlight);
		}
	}

	void MouseMove(const CPoint &point)
	{
		if (_bTracking) ScrollTo(point); 
		BaseClass::MouseMove(point);
	}

	void ScrollTo(const CPoint &point)
	{
		const IW::Image &image = _state.Image.GetThumbnailImage();

		const CRect rectDrawnImage = GetRectDrawImage();
		const CRect rectOld = GetNavigationRect();
		const CRect rectNew = IW::ClampRect(CRect(point - IW::Half(rectOld.Size()), rectOld.Size()), rectDrawnImage);
		const CPoint pointLocalScroll = rectNew.TopLeft() - GetPointDrawImage(); 
		const CSize sizeAll = _pHost->GetCanvasSize();

		const CPoint pointToScrollTo(
			MulDiv(pointLocalScroll.x, sizeAll.cx, rectDrawnImage.Width()),
			MulDiv(pointLocalScroll.y, sizeAll.cy, rectDrawnImage.Height()));

		_pHost->ScrollTo(pointToScrollTo);
	}
};
