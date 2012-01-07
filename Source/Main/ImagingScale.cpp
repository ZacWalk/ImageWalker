#include "StdAfx.h"
#include "ImageStreams.h"
#include "Blitter.h"
#include "RenderSurface.h"




bool IW::Scale(const IW::Image &imageIn, IW::Image &imageOut, CSize size, IW::IStatus *pStatus)
{
	const CRect rcBounding = imageIn.GetBoundingRect();   

	int cxBounding = rcBounding.Width();
	int cyBounding  = rcBounding.Height(); 

	// zoom scale factors 
	const double dScaleX = (double) size.cx / (double) cxBounding;
	const double dScaleY = (double) size.cy / (double) cyBounding;		

	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		CRect rcIn = pageIn->GetPageRect();

		CRect rcOut(
			MulDiv(rcIn.left, size.cx, cxBounding), 
			MulDiv(rcIn.top, size.cy, cyBounding), 
			MulDiv(rcIn.right, size.cx, cxBounding), 
			MulDiv(rcIn.bottom, size.cy, cyBounding));

		IW::PixelFormat pf(pageIn->GetPixelFormat().HasAlpha() ? IW::PixelFormat::PF32Alpha : IW::PixelFormat::PF24);
		IW::Page &pageOut = imageOut.CreatePage(rcOut, pf);
		IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

		if (CanUseMMX()) 
		{
			RenderImage<IW::IImageSurfaceLock, CBlitterMMX> render(*pLockOut, CBlitterMMX());
			render.DrawImage(*pageIn, rcOut, rcIn);
		}	
		else
		{
			RenderImage<IW::IImageSurfaceLock, CBlitter> render(*pLockOut, CBlitter());
			render.DrawImage(*pageIn, rcOut, rcIn);
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	imageOut.Normalize();
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	return true;
};
