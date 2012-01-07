///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Load.h: interface for the CLoad class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


template<class T>
class ImageFilter
{
public:
	CRect _rectSelection;
	bool _bSelection;

public:
	ImageFilter()
	{
		_bSelection = false;
	}

	~ImageFilter()
	{
	}

	bool IterateProperties(IW::IPropertyStream *pStreamOut) const
	{
		return true;
	};

	IW::IPropertyStream* InsertProperties()
	{
		return 0;
	}

	// Set the selection
	void SetSelection(LPCRECT pRect)
	{
		if (pRect)
		{
			_rectSelection = *pRect;
			_bSelection = true;
		}
	}

	bool DisplaySettingsDialog(const IW::Image &image)
	{
		return true;
	}

	void Read(const IW::IPropertyArchive *pArchive)
	{
		return;
	};

	void Write(IW::IPropertyArchive *pArchive) const
	{
		return;
	};

	bool CreatePreview(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		T *pT = static_cast<T*>(this);
		return pT->ApplyFilter(imageIn, imageOut, pStatus);
	}

	bool IsPreviewScaleIndependant() const
	{
		return false;
	}

	bool RequiresSelection() const
	{
		return false;
	}

	void OnReset() 
	{
	}

	void OnHelp() const
	{
		App.InvokeHelp(IW::GetMainWindow(), T::GetHelpID());
	}

	bool RequiresPersist() const
	{
		return true;
	}
};

class  CFilterColorAdjust :  public ImageFilter<CFilterColorAdjust>
{

public:
	CFilterColorAdjust();
	~CFilterColorAdjust();

	CString GetKey() { return _T("ColorAdjust"); };
	CString GetTitle() { return App.LoadString(IDS_COLOR_ADJUST); };
	CString GetDescription() { return App.LoadString(IDS_COLOR_ADJUST_DESC); };
	CString GetSection() { return App.LoadString(IDS_COLOR); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE | IW::ImageFilterFlags::OPTIONS; };
	DWORD GetIcon() { return ImageIndex::FilterColor; };
	DWORD GetHelpID() { return HELP_FILTER_COLOR_ADJUST; };

	int m_nContrast;
	int m_nBrightness;
	int m_nRed;
	int m_nGreen;
	int m_nBlue;
	int m_nHue;
	int m_nSaturation;
	int m_nLightness;

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
	bool ProcessRGB(LPCOLORREF pOut, IW::LPCCOLORREF pIn, int nLength);
	bool DisplaySettingsDialog(const IW::Image &image);
	void Read(const IW::IPropertyArchive *pArchive);
	void Write(IW::IPropertyArchive *pArchive) const;
};


class  CFilterRedEye : public ImageFilter<CFilterRedEye>
{
public:
	CFilterRedEye()  {};

	CString GetKey() { return _T("RedEye"); };
	CString GetTitle() { return App.LoadString(IDS_RED_EYE_REDUCTION); };
	CString GetDescription() { return App.LoadString(IDS_RED_EYE_REDUCTION_DESC); };
	CString GetSection() { return App.LoadString(IDS_COLOR ); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return ImageIndex::FilterRedEye; };
	DWORD GetHelpID() { return HELP_FILTER_RED_EYE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
};

class  CFilterContrastStretch : public ImageFilter<CFilterContrastStretch>
{
public:
	CFilterContrastStretch()  {};

	CString GetKey() { return _T("ContrastStretch"); };
	CString GetTitle() { return App.LoadString(IDS_CONTRAST_STRETCH); };
	CString GetDescription() { return App.LoadString(IDS_CONTRAST_STRETCH_DESC); };
	CString GetSection() { return App.LoadString(IDS_COLOR); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_CONTRAST_STRETCH; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
};



class  CFilterDither : public ImageFilter<CFilterDither>
{
public:
	CFilterDither()  {};

	CString GetKey() { return _T("Dither"); };
	CString GetTitle() { return App.LoadString(IDS_DITHER); };
	CString GetDescription() { return App.LoadString(IDS_DITHER_DESC); };
	CString GetSection() { return App.LoadString(IDS_COLOR); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_DITHER; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return IW::Dither(imageIn, imageOut, pStatus);
	}	
};

class  CFilterNegate : public ImageFilter<CFilterNegate>
{
public:
	CFilterNegate()  
	{
	};

	CString GetKey() { return _T("Negate"); };
	CString GetTitle() { return App.LoadString(IDS_NEGATE); };
	CString GetDescription() { return App.LoadString(IDS_NEGATE_DESC); };
	CString GetSection() { return App.LoadString(IDS_COLOR); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_NEGATE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
};

class  CFilterQuantize : public ImageFilter<CFilterQuantize>
{
public:
	CFilterQuantize()  
	{
	};

	CString GetKey() { return _T("Quantize"); };
	CString GetTitle() { return App.LoadString(IDS_QUANTIZE); };
	CString GetDescription() { return App.LoadString(IDS_QUANTIZE_DESC); };
	CString GetSection() { return App.LoadString(IDS_COLOR); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_QUANTIZE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut,	IW::IStatus *pStatus)
	{
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return IW::Quantize(imageIn, imageOut, pStatus);
	}	
};

class  CFilterCrop : public ImageFilter<CFilterCrop>
{
public:

	bool m_bLossLess;

	CFilterCrop()  
	{
		m_bLossLess = true;
	};

	CString GetKey() { return _T("Crop"); };
	CString GetTitle() { return App.LoadString(IDS_CROP); };
	CString GetDescription() { return App.LoadString(IDS_CROP_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::REQUIRES_SELECTION | IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::SINGLE | IW::ImageFilterFlags::OPTIONS; };
	DWORD GetIcon() { return ImageIndex::FilterCrop; };
	DWORD GetHelpID() { return HELP_FILTER_CROP; };

	void Read(const IW::IPropertyArchive *pArchive)
	{
		pArchive->Read(g_szLossLess, m_bLossLess);
	};

	void Write(IW::IPropertyArchive *pArchive) const
	{
		pArchive->Write(g_szLossLess, m_bLossLess);
	};

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
	bool DisplaySettingsDialog(const IW::Image &image);
};

class  CFilterGreyScale : public ImageFilter<CFilterGreyScale>
{
public:
	CFilterGreyScale()  {};

	CString GetKey() { return _T("GreyScale"); };
	CString GetTitle() { return App.LoadString(IDS_GREYSCALE); };
	CString GetDescription() { return App.LoadString(IDS_GREYSCALE_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_GRAY_SCALE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return GrayScale(imageIn, imageOut, pStatus);
	}



};

class  CFilterEmbos : public ImageFilter<CFilterEmbos>
{
public:
	CFilterEmbos()  {};

	CString GetKey() { return _T("Embos"); };
	CString GetTitle() { return App.LoadString(IDS_EMBOS); };
	CString GetDescription() { return App.LoadString(IDS_EMBOS_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_EMBOS; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		long kernel[]={-1,0,0,0,0,0,0,0,1};
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return Filter(imageIn, imageOut, kernel,3,0,127, pStatus);
	}

};

class  CFilterBlur : public ImageFilter<CFilterBlur>
{
public:
	CFilterBlur()  
	{
	};

	CString GetKey() { return _T("Blur"); };
	CString GetTitle() { return App.LoadString(IDS_BLUR); };
	CString GetDescription() { return App.LoadString(IDS_BLUR_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_BLUR; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{	
		long kernel[]={1,1,1,1,1,1,1,1,1};
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return Filter(imageIn, imageOut, kernel,3,9,0, pStatus);
	}

};

class  CFilterSoften : public ImageFilter<CFilterSoften>
{
public:
	CFilterSoften()  {};

	CString GetKey() { return _T("Soften"); };
	CString GetTitle() { return App.LoadString(IDS_SOFTEN); };
	CString GetDescription() { return App.LoadString(IDS_SOFTEN_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_SOFTEN; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		long kernel[]={1,1,1,1,8,1,1,1,1};
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return Filter(imageIn, imageOut, kernel,3,16,0, pStatus);
	}


};

class  CFilterSharpen : public ImageFilter<CFilterSharpen>
{
public:
	CFilterSharpen()  
	{
	};

	CString GetKey() { return _T("Sharpen"); };
	CString GetTitle() { return App.LoadString(IDS_SHARPEN); };
	CString GetDescription() { return App.LoadString(IDS_SHARPEN_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_SHARPEN; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		long kernel[]={-1,-1,-1,-1,15,-1,-1,-1,-1};
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return Filter(imageIn, imageOut, kernel,3,7,0, pStatus);
	}


};

class  CFilterEdge : public ImageFilter<CFilterEdge>
{
public:
	CFilterEdge()  
	{
	};

	CString GetKey() { return _T("Edge"); };
	CString GetTitle() { return App.LoadString(IDS_EDGE); };
	CString GetDescription() { return App.LoadString(IDS_EDGE_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_EDGE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		long kernel[]={-1,-1,-1,-1,8,-1,-1,-1,-1};
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return Filter(imageIn, imageOut, kernel,3,0,0, pStatus);
	}


};

class  CFilterResize : public ImageFilter<CFilterResize>
{
public:

	bool m_bHasImage;

	int m_nOriginalWidth;
	int m_nOriginalHeight;

	int m_nWidth;
	int m_nHeight;
	bool m_bKeepAspect;
	bool m_bScaleDown;

	int m_nFilter;
	int m_nType;
	DWORD m_dwXPelsPerMeter;
	DWORD m_dwYPelsPerMeter;

public:
	CFilterResize()  
	{
		m_nOriginalWidth = m_nWidth = 640;
		m_nOriginalHeight = m_nHeight = 480;
		m_nFilter = 0;
		m_nType = 0;
		m_dwXPelsPerMeter = App.Options.m_dwXPelsPerMeter;
		m_dwYPelsPerMeter = App.Options.m_dwYPelsPerMeter;
		m_bKeepAspect = true;
		m_bScaleDown = false;
		m_bHasImage = false;
	};

	~CFilterResize()
	{
	}

	CString GetKey() { return _T("Resize"); };
	CString GetTitle() { return App.LoadString(IDS_RESIZE); };
	CString GetDescription() { return App.LoadString(IDS_RESIZE_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE | IW::ImageFilterFlags::OPTIONS; };
	DWORD GetIcon() { return -1; };
	DWORD GetHelpID() { return HELP_FILTER_RESIZE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
	void Read(const IW::IPropertyArchive *pArchive);
	void Write(IW::IPropertyArchive *pArchive) const;
	bool DisplaySettingsDialog(const IW::Image &image);
};

class  CFilterFrame :  public ImageFilter<CFilterFrame>
{
protected:

public:

	CFilterFrame() 
	{
	};

	CString GetKey() { return _T("Frame"); };
	CString GetTitle() { return App.LoadString(IDS_FRAME); };
	CString GetDescription() { return App.LoadString(IDS_FRAME_DESC); };
	CString GetSection() { return App.LoadString(IDS_FILTER); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return ImageIndex::FilterFrame; };
	DWORD GetHelpID() { return HELP_FILTER_FRAME; };

	bool ApplyFilter( const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		IW::Frame(imageIn, imageOut, RGB(0,0,0), pStatus);
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);
		return true;
	}
};

class CFilterRotate : public ImageFilter<CFilterRotate>
{
public:

	// Jpeg transformation options
	bool m_bMirrorLR;
	bool m_bMirrorTB;
	bool m_bRotate90;
	bool m_bRotate180;
	bool m_bRotate270;
	bool m_bRotateX;
	bool m_bLossLess;
	int  m_nDegrees;

	CFilterRotate() 
	{
		m_bMirrorLR = false;
		m_bMirrorTB = false;
		m_bRotate90 = false;
		m_bRotate180 = false;
		m_bRotate270 = false;
		m_bRotateX = true;
		m_nDegrees = 33;
		m_bLossLess = true;
	};

	~CFilterRotate()
	{
	};

	void Read(const IW::IPropertyArchive *pArchive);
	void Write(IW::IPropertyArchive *pArchive) const;
	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
	bool DisplaySettingsDialog(const IW::Image &image);

	CString GetKey() { return _T("Rotate"); };
	CString GetTitle() { return App.LoadString(IDS_ROTATE); };
	CString GetDescription() { return App.LoadString(IDS_ROTATE_DESC); };
	CString GetSection() { return App.LoadString(IDS_ROTATE); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE | IW::ImageFilterFlags::OPTIONS; };
	DWORD GetIcon() { return ImageIndex::FilterRotate; };
	DWORD GetHelpID() { return HELP_FILTER_ROTATE; };
};

class  CFilterRotateLeft : public ImageFilter<CFilterRotateLeft>
{
public:
	CFilterRotateLeft() 
	{
	}

	CString GetKey() { return g_szRotateLeft; };
	CString GetTitle() { return App.LoadString(IDS_ROTATE_LEFT); };
	CString GetDescription() { return App.LoadString(IDS_ROTATE_LEFT_DESC); };
	CString GetSection() { return App.LoadString(IDS_ROTATE); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return ImageIndex::FilterRotateLeft; };
	DWORD GetHelpID() { return HELP_FILTER_ROTATE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);

};

class  CFilterRotateRight : public ImageFilter<CFilterRotateRight>
{
public:
	CFilterRotateRight() 
	{
	}

	CString GetKey() { return g_szRotateRight; };
	CString GetTitle() { return App.LoadString(IDS_ROTATE_RIGHT); };
	CString GetDescription() { return App.LoadString(IDS_ROTATE_RIGHT_DESC); };
	CString GetSection() { return App.LoadString(IDS_ROTATE); };
	//DWORD GetFlags() { return IW::ImageFilterFlags::MULTIPAGE | IW::ImageFilterFlags::MULTIPLE | IW::ImageFilterFlags::SINGLE; };
	DWORD GetIcon() { return ImageIndex::FilterRotateRight; };
	DWORD GetHelpID() { return HELP_FILTER_ROTATE; };

	bool ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);
};