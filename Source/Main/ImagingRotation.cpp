#include "StdAfx.h"
#include "ImageStreams.h"

bool IW::Rotate90(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	for(IW::Image::PageList::const_iterator it = imageIn.Pages.begin(); it != imageIn.Pages.end(); ++it)
	{
		const IW::Page &pageIn = *it;
		if (pStatus->QueryCancel()) return false;

		CRect rcOrg = pageIn.GetPageRect();
		CRect rc(rcOrg.top,  rcOrg.left,  rcOrg.bottom,  rcOrg.right);
		PixelFormat pf = pageIn.GetPixelFormat();
		IW::Page pageOut = imageOut.CreatePage(rc, pf);
		int nPaletteEntries = pf.NumberOfPaletteEntries();
			
		if (nPaletteEntries != 0)
		{
			IW::MemCopy(pageOut.GetPalette(), pageIn.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
		}

		IW::ConstIImageSurfaceLockPtr pLockIn = pageIn.GetSurfaceLock();
		IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();
		
		const int nWidth = pageIn.GetWidth();
		const int nHeight = pageIn.GetHeight();

		IW::CAutoFree<COLORREF> lineBuffer(nWidth + 1);	

		for(int y = 0; y < nHeight; y++) 
		{
			pLockIn->GetLine(lineBuffer, y, 0, nWidth);

			for(int x = 0; x < nWidth; x++) 
			{
				pLockOut->SetPixel(nHeight - (y + 1), x, lineBuffer[x]);
			}

			pStatus->Progress(y, nHeight);
		}

		pageOut.CopyExtraInfo(pageIn);		

	}	

	imageOut.Normalize();
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);
	
	return true;
}

bool IW::Rotate270(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	for(IW::Image::PageList::const_iterator it = imageIn.Pages.begin(); it != imageIn.Pages.end(); ++it)
	{
		const IW::Page &pageIn = *it;
		if (pStatus->QueryCancel()) return false;

		CRect rcOrg = pageIn.GetPageRect();
		CRect rc(rcOrg.top,  rcOrg.left, rcOrg.bottom,  rcOrg.right);
		PixelFormat pf = pageIn.GetPixelFormat();
		IW::Page pageOut = imageOut.CreatePage(rc, pf);
		int nPaletteEntries = pf.NumberOfPaletteEntries();
			
		if (nPaletteEntries != 0)
		{
			IW::MemCopy(pageOut.GetPalette(), pageIn.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
		}

		IW::ConstIImageSurfaceLockPtr pLockIn = pageIn.GetSurfaceLock();
		IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();
		
		const int nWidth = pageIn.GetWidth();
		const int nHeight = pageIn.GetHeight();

		IW::CAutoFree<COLORREF> lineBuffer(nWidth + 1);	

		for(int y = 0; y < nHeight; y++) 
		{
			pLockIn->GetLine(lineBuffer, y, 0, nWidth);

			for(int x = 0; x < nWidth; x++) 
			{
				pLockOut->SetPixel(y, (nWidth - 1) - x, lineBuffer[x]);
			}

			pStatus->Progress(y, nHeight);
		}

		pageOut.CopyExtraInfo(pageIn);
	}

	imageOut.Normalize();
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	return true;
}


bool IW::Rotate180(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	for(IW::Image::PageList::const_iterator it = imageIn.Pages.begin(); it != imageIn.Pages.end(); ++it)
	{
		const IW::Page &pageIn = *it;
		if (pStatus->QueryCancel()) return false;

		CRect rcOrg = pageIn.GetPageRect();
		CRect rc(rcOrg.left, rcOrg.top, rcOrg.right, rcOrg.bottom);
		PixelFormat pf = pageIn.GetPixelFormat();
		IW::Page pageOut = imageOut.CreatePage(rc, pf);
		int nPaletteEntries = pf.NumberOfPaletteEntries();
			
		if (nPaletteEntries != 0)
		{
			IW::MemCopy(pageOut.GetPalette(), pageIn.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
		}

		IW::ConstIImageSurfaceLockPtr pLockIn = pageIn.GetSurfaceLock();
		IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();
		
		const int nWidth = pageIn.GetWidth();
		const int nHalfWidth = nWidth / 2;
		const int nHeight = pageIn.GetHeight();

		IW::CAutoFree<COLORREF> lineBuffer(nWidth + 1);	

		for(int y = 0; y < nHeight; y++) 
		{
			pLockIn->GetLine(lineBuffer, y, 0, nWidth);

			for(int x = 0; x < nHalfWidth; x++) 
			{
				IW::Swap(lineBuffer[x], lineBuffer[nWidth - (x + 1)]);
			}

			pLockOut->SetLine(lineBuffer, nHeight - (y + 1), 0, nWidth);

			pStatus->Progress(y, nHeight);
		}

		pageOut.CopyExtraInfo(pageIn);
	}

	imageOut.Normalize();
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	return true;	
}




static inline COLORREF MinusCOLORREF (const COLORREF a, COLORREF b)
{
	return IW::RGBA(
		IW::GetR(a) - IW::GetR(b),
		IW::GetG(a) - IW::GetG(b),
		IW::GetB(a) - IW::GetB(b),
		IW::GetA(a) - IW::GetA(b));
}


static inline int interp2(const unsigned a, const unsigned b, const unsigned w)
{ 
	return (b + (a-b)*w) >> 8; 
}

static inline COLORREF interp(COLORREF a, COLORREF b, const unsigned w)
{
	return IW::RGBA( 
		interp2 (IW::GetR(a), IW::GetR(b), w),
		interp2 (IW::GetG(a), IW::GetG(b), w),
		interp2 (IW::GetB(a), IW::GetB(b), w),
		interp2 (IW::GetA(a), IW::GetA(b), w));
}

void SkewLine(LPCOLORREF pLineOut, IW::LPCCOLORREF pLineIn, const int len, const int lenOut, const int iOffset, const int weight, const COLORREF clrBack)
{
	// Fill gap left of skew with background
	COLORREF pxlOldLeft = clrBack;
	int n = 0;

	for (n = 0; n < iOffset; n++) 
		pLineOut[n] = clrBack;

	for (n = 0; n < len; n++) 
	{
		const COLORREF pxlSrc = pLineIn[n];
		const COLORREF pxlLeft = interp(pxlSrc, clrBack, weight);
		const int nOut = n + iOffset;		

		if ((nOut >= 0) && (nOut < lenOut))
		{
			pLineOut[nOut] = MinusCOLORREF(pxlSrc, MinusCOLORREF(pxlLeft, pxlOldLeft));
		}

		pxlOldLeft = pxlLeft;
	}

	n = len + iOffset;  

	if (n < lenOut)
	{
		pLineOut[n] = pxlOldLeft;
		n++;
	}

	for (; n < lenOut; n++)
		pLineOut[n] = clrBack;
}

static void HorizontalSkew (
					   const IW::Page &pageIn,
					   IW::Page &pageOut,
					   unsigned y,			// Row index
					   int iOffset,			// Skew offset 
					   int weight,			// Relative weight of right COLORREF
					   COLORREF clrBack			// Background color
					   )
{
	const int cx = pageIn.GetWidth();
	const int cxOut = pageOut.GetWidth();	
	
	LPCOLORREF pLineIn = IW_ALLOCA(LPCOLORREF, cx * sizeof(COLORREF));
	LPCOLORREF pLineOut = IW_ALLOCA(LPCOLORREF, cxOut * sizeof(COLORREF));

	IW::ConstIImageSurfaceLockPtr pLockIn = pageIn.GetSurfaceLock();
	IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();
	pLockIn->RenderLine(pLineIn, y, 0, cx);

	SkewLine(pLineOut, pLineIn, cx, cxOut, iOffset, weight, clrBack);

	pLockOut->SetLine(pLineOut, y, 0, cxOut);
}


// Skews a column vertically (with filtered weights)
// Limited to 45 degree skewing only. Filters two adjacent COLORREFs.
static void VerticalSkew (
					  const IW::Page &pageIn,		// Img to skew (+ dimensions)
					  IW::Page &pageOut,				// Destination of skewed image (+ dimensions)
					  unsigned x,				// Column index
					  int iOffset,			// Skew offset 
					  int weight,			// Relative weight of right COLORREF
					  COLORREF clrBack			// Background color
					  )
{
	const int cy = pageIn.GetHeight();
	const int cyOut = pageOut.GetHeight();

	LPCOLORREF pLineIn = IW_ALLOCA(LPCOLORREF, cy * sizeof(COLORREF));
	LPCOLORREF pLineOut = IW_ALLOCA(LPCOLORREF, cyOut * sizeof(COLORREF));

	IW::ConstIImageSurfaceLockPtr pLockIn = pageIn.GetSurfaceLock();
	IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();		

	for(int y = 0; y < cy; y++)
	{
		pLineIn[y] = pLockIn->GetPixel(x, y);
	}

	SkewLine(pLineOut, pLineIn, cy, cyOut, iOffset, weight, clrBack);

	for(int y = 0; y < cyOut; y++)
	{
		pLockOut->SetPixel(x, y, pLineOut[y]);
	}
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool RotatePage(IW::Image& imageOut, const IW::Page &pageIn, const double dRadAngle, const double dTan, const double dSinE, const COLORREF clrBack, IW::IStatus *pStatus)
{
	const int cx = pageIn.GetWidth();
	const int cy = pageIn.GetHeight();		

	const int cx1 = cx + int(double(cy) * fabs(dTan) + 0.5);
	const int cy1 = cy;

	// Calc first shear (horizontal) destination image dimensions 
	IW::Image image1;
	IW::Page &page1 = imageOut.CreatePage(cx1, cy1, IW::PixelFormat::PF32);

	if (pStatus->QueryCancel()) return false;

	// Perform 1st shear (horizontal) 
	for (int u = 0; u < cy1; u++) 
	{ 	
		const double dShear = (dTan >= 0.0) ? ((u + 0.5) * dTan) : (((u - cy1) + 0.5) * dTan);
		const int iShear = (int)floor (dShear);
		const int nWeight = int(255 * (dShear - iShear) + 1) & 0xFF;

		HorizontalSkew (pageIn, page1, u, iShear, nWeight, clrBack);
	}

	// Perform 2nd shear  (vertical)		
	const int cx2 = cx1;
	const int cy2 = int((cx * fabs (dSinE)) + (cy * cos (dRadAngle)) + 0.5) + 1;

	IW::Image image2;
	IW::Page &page2 = image2.CreatePage(cx2, cy2, IW::PixelFormat::PF32);

	// Variable skew offset
	double dOffset2 = (dSinE > 0.0) ? ((cx - 1) * dSinE) : (-dSinE * (cx - cx2)); 

	if (pStatus->QueryCancel()) return false;

	for (int u = 0; u < cx2; u++, dOffset2 -= dSinE) 
	{		
		const int iShear = int (floor (dOffset2));
		const int nWeight = int(255 * (dOffset2 - iShear) + 1) & 0xFF;

		VerticalSkew (page1, page2, u, iShear, nWeight, clrBack);
	}

	// Perform 3rd shear (horizontal) 	
	const int cx3 = int ((cy * fabs (dSinE)) + (cx * cos (dRadAngle)) + 0.5) + 1;
	const int cy3 = cy2;

	IW::Page &pageOut = imageOut.CreatePage(cx3, cy3, IW::PixelFormat::PF24);

	double dOffset3 = (dSinE >= 0.0) ?  ((cx - 1) * dSinE * -dTan) 
		: (dTan * ((cx - 1) * -dSinE + (1 - cy3)));

	if (pStatus->QueryCancel()) return false;

	for (int u = 0; u < cy3; u++, dOffset3 += dTan)
	{		
		const int iShear = int (floor(dOffset3));
		const int nWeight = int(255 * (dOffset3 - iShear) + 1) & 0xFF;

		HorizontalSkew (page2, pageOut, u, iShear, nWeight, clrBack);
	}

	pageOut.CopyExtraInfo(pageIn);
	return true;
}

/*

class Vector
{
public:
    double x, y;

    Vector(double f = 0.0f) : x(f), y(f) {}
    Vector(double xIn, double yIn) : x(xIn), y(yIn) {}
	Vector(const Vector &other) : x(other.x), y(other.y) {}
	
	void operator=(const Vector &other)
	{
        x = other.x;
		y = other.y;
	}

	void operator+=(const Vector &other)
	{
        x += other.x;
		y += other.y;
	}
	
	void operator-=(const Vector &other)
	{
        x -= other.x;
		y -= other.y;
	}
	
	Vector operator+(const Vector &other) const
	{
        return Vector(x + other.x, y + other.y);
	}
	
	Vector operator-(const Vector &other) const
	{
        return Vector(x - other.x, y - other.y);
	}

	Vector MirrorLR(const Vector& center) const
	{
		return Vector(center.x + (center.x - x), y);		
	}

	Vector MirrorTB(const Vector& center) const
	{
		return Vector(y, center.y + (center.y - y));
	}

	CPoint ToPoint() const
	{
		return CPoint((int)x, (int)y);
	}

	CSize ToSize() const
	{
		return CSize((int)x, (int)y);
	}
};

class LineSegment
{
public:
    Vector begin_;
    Vector end_;

    LineSegment(const Vector& begin, const Vector& end)
        : begin_(begin), end_(end) {}

    enum IntersectResult { PARALLEL, COINCIDENT, NOT_INTERESECTING, INTERESECTING };

    IntersectResult Intersect(const LineSegment& other_line, Vector& intersection) const
    {
        double denom = ((other_line.end_.y - other_line.begin_.y)*(end_.x - begin_.x)) -
                      ((other_line.end_.x - other_line.begin_.x)*(end_.y - begin_.y));

        double nume_a = ((other_line.end_.x - other_line.begin_.x)*(begin_.y - other_line.begin_.y)) -
                       ((other_line.end_.y - other_line.begin_.y)*(begin_.x - other_line.begin_.x));

        double nume_b = ((end_.x - begin_.x)*(begin_.y - other_line.begin_.y)) -
                       ((end_.y - begin_.y)*(begin_.x - other_line.begin_.x));

        if(denom == 0.0f)
        {
            if(nume_a == 0.0f && nume_b == 0.0f)
            {
                return COINCIDENT;
            }
            return PARALLEL;
        }

        double ua = nume_a / denom;
        double ub = nume_b / denom;

        if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
        {
            // Get the intersection point.
            intersection.x = begin_.x + ua*(end_.x - begin_.x);
            intersection.y = begin_.y + ua*(end_.y - begin_.y);

            return INTERESECTING;
        }

        return NOT_INTERESECTING;
    }
};


static inline CRect CalcCropRect(double width, double height, double rotatedWidth, double rotatedHeight, double angle)
{
	Vector extent[4], min, max;
	
	int i;  
	double x = -tan(angle/2.0);
	double y = sin(angle);
	const Vector center(rotatedWidth / 2.0, rotatedHeight / 2.0);

	// Calculate the rotated image size.
	extent[0].x=(-width/2.0);
	extent[0].y=(-height/2.0);
	extent[1].x=width/2.0;
	extent[1].y=(-height/2.0);
	extent[2].x=(width/2.0);
	extent[2].y=height/2.0;
	extent[3].x=-width/2.0;
	extent[3].y=height/2.0;

	for (i=0; i < 4; i++)
	{
		extent[i].x+=x*extent[i].y;
		extent[i].y+=y*extent[i].x;
		extent[i].x+=x*extent[i].y;
		extent[i] += center;
	}
	
	const LineSegment diagonal1(center, center + Vector(-width, -height));
	const LineSegment diagonal2(center, center + Vector(width, -height));

	Vector intersection1(center);
	Vector intersection2(center);

	for(i = 0; i < 4; i++)
	{
		const LineSegment side(extent[i], extent[(i + 1) % 4]);
		diagonal1.Intersect(side, intersection1);
		diagonal2.Intersect(side, intersection2);
	}

	double cx = min(abs(center.x - intersection1.x), abs(center.x - intersection2.x));
    double cy = min(abs(center.y - intersection1.y), abs(center.y - intersection2.y));

	Vector size(cx * 2, cy * 2);
	Vector halfSize(cx, cy);

	return CRect((center - halfSize).ToPoint(), size.ToSize());
  
}
*/

bool IW::Rotate(const IW::Image &imageInRaw,  IW::Image &imageOut, float fAngle, IW::IStatus *pStatus)
{
	// Get it into the positive
	//fAngle = -fAngle;
	while(fAngle > 360.0f) fAngle -= 360.0f;
	while(fAngle < 0.0f) fAngle += 360.0f;

	if (fAngle == 0.0)
	{
		imageOut = imageInRaw;
		return true;
	}

	IW::Image imageIn = imageInRaw;

	// If we are multiple pages then do the render
	if (imageIn.NeedRenderForDisplay())
	{
		IW::Image imageTemp;
		if (!imageIn.Render(imageTemp)) return false;
		imageIn = imageTemp;
	}

	if (fAngle > 45.0 && fAngle <= 135.0) 
	{
		// Angle in (45.0 .. 135.0] 
		// Rotate image by 90 degrees into temporary image,
		// so it requires only an extra rotation angle 
		// of -45.0 .. +45.0 to complete rotation.
		fAngle -= 90.0;

		IW::Image imageTemp;
		if (!IW::Rotate90(imageIn, imageTemp, pStatus)) return false;
		imageIn = imageTemp;		
	}
	else if (fAngle > 135.0 && fAngle <= 225.0) 
	{ 
		// Angle in (135.0 .. 225.0] 
		// Rotate image by 180 degrees into temporary image,
		// so it requires only an extra rotation angle 
		// of -45.0 .. +45.0 to complete rotation.		
		fAngle -= 180.0;

		IW::Image imageTemp;
		if (!IW::Rotate180(imageIn, imageTemp, pStatus)) return false;
		imageIn = imageTemp;
	}
	else if (fAngle > 225.0 && fAngle <= 315.0) 
	{ 
		// Angle in (225.0 .. 315.0] 
		// Rotate image by 270 degrees into temporary image,
		// so it requires only an extra rotation angle 
		// of -45.0 .. +45.0 to complete rotation.
		fAngle -= 270.0;

		IW::Image imageTemp;
		if (!IW::Rotate270(imageIn, imageTemp, pStatus)) return false;
		imageIn = imageTemp;		
	}

	// If we got here, angle is in (-45.0 .. +45.0]
	const double ROTATE_PI = 3.1415926535897932384626433832795;
	const double dRadAngle = fAngle * ROTATE_PI / 180.0; // Angle in radians
	const double dSinE = sin (dRadAngle);
	const double dTan = tan (dRadAngle / 2.0);
	const COLORREF clrBack = IW::RGBA(0,0,0,0);

	// Rotate each page
	//IW::Image imageRotated;
	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		if (pStatus->QueryCancel()) return false;
		if (!RotatePage(imageOut, *pageIn, dRadAngle, dTan, dSinE, clrBack, pStatus))
			return false;
	}

	/*CRect rectRotated = imageRotated.GetBoundingRect();
	CRect rectIn = imageIn.GetBoundingRect();

	const CRect rectCrop = CalcCropRect(rectIn.Width(), rectIn.Height(), rectRotated.Width(), rectRotated.Height(), dRadAngle);
	if (!IW::Crop(imageRotated, imageOut, rectCrop, pStatus))
		return false;*/

	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	return true;
}


/*

bool IW::Rotate(const IW::Image &imageIn,  IW::Image &imageOut, float fAngle, IW::IStatus *pStatus)
{
	agg::rgba background(0, 0, 0);

	IW::Image imageRotated;
	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		const CRect rectIn = pageIn->GetPageRect();
		IW::Page &pageOut = imageOut.CreatePage(rectIn.Width(), rectIn.Height(), IW::PixelFormat::PF32);		
		const CRect rectOut = pageOut.GetPageRect();		

		agg::rendering_buffer image_buffer_in((agg::int8u*)pageIn->GetBitmap(), rectIn.Width(), rectIn.Height(), -pageIn->GetStride());
		agg::rendering_buffer image_buffer_out((agg::int8u*)pageOut.GetBitmap(), rectOut.Width(), rectOut.Height(), -pageOut.GetStride());

		typedef agg::pixfmt_bgra32 pixfmt;
		typedef agg::pixfmt_bgr24 pixfmt_in;		

		double cxIn = rectIn.Width();
		double cyIn = rectIn.Height();

		double cxOut = rectOut.Width();
		double cyOut = rectOut.Height();

		agg::trans_affine src_mtx;
		src_mtx *= agg::trans_affine_translation(-cxIn/2, -cyIn/2);
		src_mtx *= agg::trans_affine_rotation(fAngle * agg::pi / 180.0);
		src_mtx *= agg::trans_affine_translation(cxOut/2, cyOut/2 + 20);

		agg::trans_affine img_mtx = src_mtx;
		img_mtx.invert();

		typedef agg::rgba8 color_type;
		agg::span_allocator<color_type> sa;

		typedef agg::span_interpolator_linear<> interpolator_type;
		interpolator_type interpolator(img_mtx);
		
		typedef agg::image_accessor_clip<pixfmt_in> img_source_type;		

		pixfmt_in img_pixf(image_buffer_in);
		img_source_type img_src(img_pixf, background);

		
		// Version without filtering (nearest neighbor)
		//------------------------------------------
		//typedef agg::span_image_filter_rgb_nn<img_source_type, interpolator_type> span_gen_type;
		//span_gen_type sg(img_src, interpolator);
		//------------------------------------------
		

		// Version with "hardcoded" bilinear filter and without 
		// image_accessor (direct filter, the old variant)
		//------------------------------------------
		//typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt_in, interpolator_type> span_gen_type;
		//span_gen_type sg(img_pixf, agg::rgba_pre(0, 0.4, 0, 0.5), interpolator);
		//------------------------------------------

		
		// Version with arbitrary 2x2 filter
		//------------------------------------------
		//typedef agg::span_image_filter_rgb_2x2<img_source_type, interpolator_type> span_gen_type;
		//agg::image_filter<agg::image_filter_kaiser> filter;
		//span_gen_type sg(img_src, interpolator, filter);
		//------------------------------------------
		
		// Version with arbitrary filter
		//------------------------------------------
		//typedef agg::span_image_filter_rgb<img_source_type, interpolator_type> span_gen_type;
		//agg::image_filter<agg::image_filter_spline36> filter;
		//span_gen_type sg(img_src, interpolator, filter);
		//------------------------------------------

		pixfmt pixf(image_buffer_out);
		agg::renderer_base<pixfmt>     rb(pixf);
		rb.clear(background);

		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_u8 sl;

		agg::rounded_rect er(0, 0, cxIn,  cyIn, 1);
		er.normalize_radius();
		agg::conv_transform<agg::rounded_rect> tr(er, src_mtx);
		ras.add_path(tr);

		agg::render_scanlines_aa(ras, sl, rb, sa, sg);
	}

	return true;
}

*/