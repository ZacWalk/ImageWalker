///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// IW::CImage: implementation
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#define _USE_MATH_DEFINES  1
#include <Math.h>

//////////////////////////////////////////////////////////////////////
// Filter Functions!!


static double J1(double x)
{
	double p, q;
	register long i;
	static const double Pone[] =
	{
		0.581199354001606143928050809e+21,
		-0.6672106568924916298020941484e+20,
		0.2316433580634002297931815435e+19,
		-0.3588817569910106050743641413e+17,
		0.2908795263834775409737601689e+15,
		-0.1322983480332126453125473247e+13,
		0.3413234182301700539091292655e+10,
		-0.4695753530642995859767162166e+7,
		0.270112271089232341485679099e+4
	},
	Qone[] = {
		0.11623987080032122878585294e+22,
		0.1185770712190320999837113348e+20,
		0.6092061398917521746105196863e+17,
		0.2081661221307607351240184229e+15,
		0.5243710262167649715406728642e+12,
		0.1013863514358673989967045588e+10,
		0.1501793594998585505921097578e+7,
		0.1606931573481487801970916749e+4,
		0.1e+1
		};

		p=Pone[8];
		q=Qone[8];
		for (i=7; i >= 0; i--)
		{
			p=p*x*x+Pone[i];
			q=q*x*x+Qone[i];
		}
		return(p/q);
}

static double P1(double x)
{
	double p, q; 
	register long i;

	static const double Pone[] =
	{
		0.352246649133679798341724373e+5,
		0.62758845247161281269005675e+5,
		0.313539631109159574238669888e+5,
		0.49854832060594338434500455e+4,
		0.2111529182853962382105718e+3,
		0.12571716929145341558495e+1
	},
	Qone[] = {
		0.352246649133679798068390431e+5,
		0.626943469593560511888833731e+5,
		0.312404063819041039923015703e+5,
		0.4930396490181088979386097e+4,
		0.2030775189134759322293574e+3,
		0.1e+1
		};

		p=Pone[5];
		q=Qone[5];
		for (i=4; i >= 0; i--)
		{
			p=p*(8.0/x)*(8.0/x)+Pone[i];
			q=q*(8.0/x)*(8.0/x)+Qone[i];
		}
		return(p/q);
}

static double Q1(double x)
{
	double p, q; 
	register long i;

	static const double Pone[] =
	{
		0.3511751914303552822533318e+3,
		0.7210391804904475039280863e+3,
		0.4259873011654442389886993e+3,
		0.831898957673850827325226e+2,
		0.45681716295512267064405e+1,
		0.3532840052740123642735e-1
	},
	Qone[] = {
		0.74917374171809127714519505e+4,
		0.154141773392650970499848051e+5,
		0.91522317015169922705904727e+4,
		0.18111867005523513506724158e+4,
		0.1038187585462133728776636e+3,
		0.1e+1
		};

		p=Pone[5];
		q=Qone[5];
		for (i=4; i >= 0; i--)
		{
			p=p*(8.0/x)*(8.0/x)+Pone[i];
			q=q*(8.0/x)*(8.0/x)+Qone[i];
		}
		return(p/q);
}

static double BesselOrderOne(double x)
{
	double p, q;

	if (x == 0.0)
		return(0.0);
	p=x;
	if (x < 0.0)
		x=(-x);
	if (x < 8.0)
		return(p*J1(x));
	q=sqrt(2.0/(M_PI*x))*(P1(x)*(1.0/sqrt(2.0)*(sin(x)-cos(x)))-8.0/x*Q1(x)*
		(-1.0/sqrt(2.0)*(sin(x)+cos(x))));
	if (p < 0.0)
		q=(-q);
	return(q);
}

static double Bessel(const double x, const double support)
{
	if (x == 0.0)
		return(M_PI/4.0);
	return(BesselOrderOne(M_PI*x)/(2.0*x));
}

static double Sinc(const double x,const double support)
{
	if (x == 0.0)
		return(1.0);
	return(sin(M_PI*x)/(M_PI*x));
}

static double Blackman(const double x, const double support)
{
	return(0.42+0.5*cos(M_PI*x)+0.08*cos(2*M_PI*x));
}

static double BlackmanBessel(const double x, const double support)
{
	return(Blackman(x/support,support)*Bessel(x,support));
}

static double BlackmanSinc(const double x, const double support)
{
	return(Blackman(x/support,support)*Sinc(x,support));
}

static double Box(const double x,const double support)
{
	if (x < -0.5)
		return(0.0);
	if (x < 0.5)
		return(1.0);
	return(0.0);
}

static double Catrom(const double x, const double support)
{
	if (x < -2.0)
		return(0.0);
	if (x < -1.0)
		return(0.5*(4.0+x*(8.0+x*(5.0+x))));
	if (x < 0.0)
		return(0.5*(2.0+x*x*(-5.0-3.0*x)));
	if (x < 1.0)
		return(0.5*(2.0+x*x*(-5.0+3.0*x)));
	if (x < 2.0)
		return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
	return(0.0);
}

static double Cubic(const double x,const double support)
{
	if (x < -2.0)
		return(0.0);
	if (x < -1.0)
		return((2.0+x)*(2.0+x)*(2.0+x)/6.0);
	if (x < 0.0)
		return((4.0+x*x*(-6.0-3.0*x))/6.0);
	if (x < 1.0)
		return((4.0+x*x*(-6.0+3.0*x))/6.0);
	if (x < 2.0)
		return((2.0-x)*(2.0-x)*(2.0-x)/6.0);
	return(0.0);
}

static double Gaussian(const double x, const double support)
{
	return(exp(-2.0*x*x)*sqrt(2.0/M_PI));
}

static double Hanning(const double x, const double support)
{
	return(0.5+0.5*cos(M_PI*x));
}

static double Hamming(const double x, const double support)
{
	return(0.54+0.46*cos(M_PI*x));
}

static double Hermite(const double x, const double support)
{
	if (x < -1.0)
		return(0.0);
	if (x < 0.0)
		return((2.0*(-x)-3.0)*(-x)*(-x)+1.0);
	if (x < 1.0)
		return((2.0*x-3.0)*x*x+1.0);
	return(0.0);
}

static double Lanczos(const double x, const double support)
{
	if (x < -3.0)
		return(0.0);
	if (x < 0.0)
		return(Sinc(-x,support)*Sinc(-x/3.0,support));
	if (x < 3.0)
		return(Sinc(x,support)*Sinc(x/3.0,support));
	return(0.0);
}

static double Mitchell(const double x, const double support)
{
	const double B   =(1.0/3.0);
	const double C   =(1.0/3.0);
	const double P0  =((  6.0- 2.0*B       )/6.0);
	const double P2  =((-18.0+12.0*B+ 6.0*C)/6.0);
	const double P3  =(( 12.0- 9.0*B- 6.0*C)/6.0);
	const double Q0  =((       8.0*B+24.0*C)/6.0);
	const double Q1  =((     -12.0*B-48.0*C)/6.0);
	const double Q2  =((       6.0*B+30.0*C)/6.0);
	const double Q3  =((     - 1.0*B- 6.0*C)/6.0);

	if (x < -2.0)
		return(0.0);
	if (x < -1.0)
		return(Q0-x*(Q1-x*(Q2-x*Q3)));
	if (x < 0.0)
		return(P0+x*x*(P2-x*P3));
	if (x < 1.0)
		return(P0+x*x*(P2+x*P3));
	if (x < 2.0)
		return(Q0+x*(Q1+x*(Q2+x*Q3)));
	return(0.0);
}

static double Quadratic(const double x, const double support)
{
	if (x < -1.5)
		return(0.0);
	if (x < -0.5)
		return(0.5*(x+1.5)*(x+1.5));
	if (x < 0.5)
		return(0.75-x*x);
	if (x < 1.5)
		return(0.5*(x-1.5)*(x-1.5));
	return(0.0);
}

static double Triangle(const double x, const double support)
{
	if (x < -1.0)
		return(0.0);
	if (x < 0.0)
		return(1.0+x);
	if (x < 1.0)
		return(1.0-x);
	return(0.0);
}

///////////////////////////////////////////////////////////////////////////////////

typedef struct _FilterInfo
{
	double (*function)(const double, const double),
		support;
} FilterInfo;

static const FilterInfo filters[] =
{
	{ Box, 0.0 },
	{ Box, 1.0 },
	{ Box, 0.5 },
	{ Triangle, 1.0 },
	{ Hermite, 1.0 },
	{ Hanning, 1.0 },
	{ Hamming, 1.0 },
	{ Blackman, 1.0 },
	{ Gaussian, 1.25 },
	{ Quadratic, 1.5 },
	{ Cubic, 2.0 },
	{ Catrom, 2.0 },
	{ Mitchell, 2.0 },
	{ Lanczos, 3.0 },
	{ BlackmanBessel, 3.2383 },
	{ BlackmanSinc, 4.0 }
};

//	image rescaling routine

typedef struct tagImageContrib
{
	int	nPixel;
	int	dWeight;
} ImageContrib;

////////////////////////////////////////////////////////////////////////////////////////



bool IW::Scale(const CSize &sizeIn, int nFilterType, const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	const CRect rcBounding = imageIn.GetBoundingRect();    
	const int cxBounding = rcBounding.Width();
	const int cyBounding  = rcBounding.Height(); 

	// zoom scale factors 
	const double dScaleX = (double) sizeIn.cx / (double) cxBounding;
	const double dScaleY = (double) sizeIn.cy / (double) cyBounding;

	if (nFilterType >= countof(filters)) nFilterType = 0;

	int nProgress = 0;
	int nProgressMax = 0;
	for(unsigned nPage = 0; nPage < imageIn.Pages.size(); nPage++)
	{
		int cy = imageIn.Pages[nPage].GetHeight();
		nProgressMax += cy;
		nProgressMax += MulDiv(cy, sizeIn.cy, cyBounding);
	}

	for(unsigned nPage = 0; nPage < imageIn.Pages.size(); nPage++)
	{
		const IW::Page &pageIn = imageIn.GetPage(nPage);
		IW::ConstRefPtr<IW::IImageSurfaceLock> pLockIn = pageIn.GetSurfaceLock();

		double dCenter, dWeight;	// Filter calculation variables
		int nLeft, nRight, nTotal;
		double dWidth, dScale;	// Filter calculation variables	

		const CRect rcIn = pageIn.GetPageRect();

		const CRect rcOut(
			MulDiv(rcIn.left, sizeIn.cx, cxBounding), 
			MulDiv(rcIn.top, sizeIn.cy, cyBounding), 
			MulDiv(rcIn.right, sizeIn.cx, cxBounding), 
			MulDiv(rcIn.bottom, sizeIn.cy, cyBounding));

		const int cxIn = rcIn.right - rcIn.left;
		const int cyIn  = rcIn.bottom - rcIn.top;	
		const int cxOut = rcOut.right - rcOut.left;
		const int cyOut  = rcOut.bottom - rcOut.top;
		double (*FilterFunction)(double, double) = filters[nFilterType].function;
		const double dFilterSupport = filters[nFilterType].support;

		// array of Contribution lists 
		//int nContribListHeight = IW::Max(size.cx, size.cy);
		const int nContribListWidth = IW::Max((dScaleX < 1.0) ? (dFilterSupport / dScaleX) * 2 + 1 : dFilterSupport * 2 + 1, (dScaleY < 1.0) ? (dFilterSupport / dScaleY) * 2 + 1 : dFilterSupport * 2 + 1);

		IW::CAutoFree<DWORD> pTemp(cxOut * cyIn);
		IW::CAutoFree<int> pContrCountX(cxOut);
		IW::CAutoFree<int> pContrCountY(cyOut);
		IW::CAutoFree<ImageContrib> pContrX(cxOut * nContribListWidth);		
		IW::CAutoFree<ImageContrib> pContrY(cyOut * nContribListWidth);

		// pre-calculate Filter pContributions for a row 
		if(dScaleX < 1.0) 
		{
			dWidth = dFilterSupport / dScaleX;
			dScale = 1.0 / dScaleX;

			int nRoundFix = (cxOut / cxIn) >> 1;

			for(int i = 0; i < cxOut; ++i) 
			{
				pContrCountX[i] = 0;
				dCenter = (double) (i - nRoundFix) / dScaleX;
				nLeft = (int)ceil(dCenter - dWidth);
				nRight = (int)floor(dCenter + dWidth);
				nTotal = 0;
				for(int j = nLeft; j <= nRight; ++j) 
				{
					dWeight = dCenter - (double) j;
					dWeight = (*FilterFunction)(dWeight / dScale, dFilterSupport) / dScale;
					int k = pContrCountX[i]++ + i * nContribListWidth;
					pContrX[k].nPixel = IW::Clamp(j, 0, cxIn-1);
					nTotal += pContrX[k].dWeight = IW::DoubleToFixed(dWeight);
				}

				// Normalize
				for(int j = 0; j < pContrCountX[i]; ++j)  
				{
					int k = j + i * nContribListWidth;
					pContrX[k].dWeight = IW::FixedDiv(pContrX[k].dWeight, nTotal);
				}
			}
		} 
		else 
		{
			int nRoundFix = (cxOut / cxIn) >> 1;

			for(int i = 0; i < cxOut; ++i) 
			{
				pContrCountX[i] = 0;
				dCenter = (double) (i - nRoundFix) / dScaleX;
				nLeft = (int)ceil(dCenter - dFilterSupport);
				nRight = (int)floor(dCenter + dFilterSupport);
				nTotal = 0;
				for(int j = nLeft; j <= nRight; ++j) 
				{
					dWeight = dCenter - (double) j;
					dWeight = (*FilterFunction)(dWeight, dFilterSupport);
					int k = pContrCountX[i]++ + i * nContribListWidth;
					pContrX[k].nPixel = IW::Clamp(j, 0, cxIn-1);
					nTotal += pContrX[k].dWeight = IW::DoubleToFixed(dWeight);
				}

				// Normalize
				for(int j = 0; j < pContrCountX[i]; ++j)  
				{
					int k = j + i * nContribListWidth;
					pContrX[k].dWeight = IW::FixedDiv(pContrX[k].dWeight, nTotal);						
				}
			}
		}

		// pre-calculate Filter pContributions for a column 
		if(dScaleY < 1.0) 
		{
			int nRoundFix = (cyOut / cyIn) >> 1;

			dWidth = dFilterSupport / dScaleY;
			dScale = 1.0 / dScaleY;
			nTotal = 0;
			for(int i = 0; i < cyOut; ++i) 
			{
				pContrCountY[i] = 0;
				dCenter = (double) (i - nRoundFix) / dScaleY;
				nLeft = (int)ceil(dCenter - dWidth);
				nRight = (int)floor(dCenter + dWidth);
				nTotal = 0;
				for(int j = nLeft; j <= nRight; ++j) 
				{
					dWeight = dCenter - (double) j;
					dWeight = (*FilterFunction)(dWeight / dScale, dFilterSupport) / dScale;
					int k = pContrCountY[i]++ + i * nContribListWidth;
					pContrY[k].nPixel = IW::Clamp(j, 0, cyIn-1) * cxOut;
					nTotal += pContrY[k].dWeight = IW::DoubleToFixed(dWeight);
				}

				// Normalize
				for(int j = 0; j < pContrCountY[i]; ++j)  
				{
					int k = j + i * nContribListWidth;
					pContrY[k].dWeight = IW::FixedDiv(pContrY[k].dWeight, nTotal);						
				}

				if (pStatus->QueryCancel())
				{
					return false;
				}
			}
		} 
		else 
		{
			int nRoundFix = (cyOut / cyIn) >> 1;

			for(int i = 0; i < cyOut; ++i) 
			{
				pContrCountY[i] = 0;
				dCenter = (double) (i - nRoundFix) / dScaleY;
				nLeft = (int)ceil(dCenter - dFilterSupport);
				nRight = (int)floor(dCenter + dFilterSupport);
				nTotal = 0;
				for(int j = nLeft; j <= nRight; ++j) 
				{
					dWeight = dCenter - (double) j;
					dWeight = (*FilterFunction)(dWeight, dFilterSupport);
					int k = pContrCountY[i]++ + i * nContribListWidth;
					pContrY[k].nPixel = IW::Clamp(j, 0, cyIn-1) * cxOut;
					nTotal += pContrY[k].dWeight = IW::DoubleToFixed(dWeight);
				}

				// Normalize
				for(int j = 0; j < pContrCountY[i]; ++j)  
				{
					int k = j + i * nContribListWidth;
					pContrY[k].dWeight = IW::FixedDiv(pContrY[k].dWeight, nTotal);						
				}

				if (pStatus->QueryCancel())
				{
					return false;
				}
			}
		}

		// we have to loop for every pane
		if (pStatus->QueryCancel())
		{
			return false;
		}		


		IW::PixelFormat pf(pageIn.GetPixelFormat().HasAlpha() ? IW::PixelFormat::PF32Alpha : IW::PixelFormat::PF24);
		IW::Page pageOut = imageOut.CreatePage(rcOut, pf);

		// Scale the line
		IW::RefPtr<IW::IImageSurfaceLock> pLockOut = pageOut.GetSurfaceLock();
		DWORD *pLineBuffer = IW_ALLOCA(LPDWORD, sizeof(DWORD) * IW::Max(cxIn, cxOut));

		for(int y = 0; (y < cyIn); y++) 
		{
			LPDWORD pLineOut = pTemp + (cxOut * y);
			pLockIn->RenderLine(pLineBuffer, y, 0, cxIn);

			for(int x = 0; x < cxOut; x++) 
			{
				int r = 0, g = 0, b = 0, a = 0;

				for(int i = 0; i < pContrCountX[x]; ++i) 
				{
					int k = i + x * nContribListWidth;
					DWORD color = pLineBuffer[pContrX[k].nPixel];
					int dWeight = pContrX[k].dWeight;

					r += dWeight * IW::GetB(color);
					g += dWeight * IW::GetG(color);
					b += dWeight * IW::GetR(color);
					a += dWeight * IW::GetA(color); 
				}

				// Check for overflow
				assert(pLineOut < pTemp + (cxOut * (y+1)));

				// Place result in destination pixel
				*pLineOut = IW::SaturateRGBA(IW::FixedToInt(b), 
					IW::FixedToInt(g), 
					IW::FixedToInt(r), 
					IW::FixedToInt(a)); 

				pLineOut++;
			}

			pStatus->Progress(nProgress++, nProgressMax);
		}

		// apply Filter to zoom vertically from pImage to pDst
		for(int y = 0; (y < cyOut) ; y++) 
		{
			LPCBYTE pLineOut = (LPCBYTE)pLineBuffer;

			for(int x = 0; x < cxOut; x++) 
			{
				int r = 0, g = 0, b = 0, a = 0;
				int k = y * nContribListWidth;

				for(int i = 0; i < pContrCountY[y]; ++i) 
				{

					DWORD color = pTemp[pContrY[k].nPixel + x];
					int nWeight = pContrY[k].dWeight;

					r += nWeight * IW::GetR(color);
					g += nWeight * IW::GetG(color);
					b += nWeight * IW::GetB(color);
					a += nWeight * (color >> 24); 

					k += 1;
				}

				*((LPDWORD)pLineOut) = IW::SaturateRGBA(IW::FixedToInt(r), 
					IW::FixedToInt(g), 
					IW::FixedToInt(b), 
					IW::FixedToInt(a)); // Place result in destination pixel

				pLineOut += 4;
			}

			pLockOut->SetLine(pLineBuffer, y, 0, cxOut);

			pStatus->Progress(nProgress++, nProgressMax);
			if (pStatus->QueryCancel()) 
				return false;
		}

		// copy over page attributes
		pageOut.CopyExtraInfo(pageIn);
	}

	return true;
};

