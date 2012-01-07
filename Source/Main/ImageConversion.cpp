///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// IW::Image : implementation file
//
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

extern DWORD masks1[];
extern int masks2[];
extern int shift2[];

void IW::IndexesToRGBA(LPCOLORREF pBitsOut, const int nSize, IW::LPCCOLORREF pPalette)
{
	for(int x = 0; x < nSize; x++)
	{
		pBitsOut[x] = pPalette[pBitsOut[x]] | 0xFF000000;
	}
}

void IW::SumLineTo32(LPBYTE pLine, const IW::RGBSUM *ps, int cx)
{
	int denom, r;

	for(int i = 0; i < cx; i++)
	{
		// blend with white
		denom = ps->c;

		if (denom > 1)
		{
			r = denom >> 1;

			pLine[0] = (r + ps->r) / denom;
			pLine[1] = (r + ps->g) / denom;
			pLine[2] = (r + ps->b) / denom;
			pLine[3] = (r + ps->a) / (ps->ac ? ps->ac : 1);
		}
		else
		{
			pLine[0] = ps->r;
			pLine[1] = ps->g;
			pLine[2] = ps->b;
			pLine[3] = ps->a / (ps->ac ? ps->ac : 1);
		}

		++ps;
		pLine += 4;
	}
}

void IW::SumLineTo16(LPBYTE pLine, const IW::RGBSUM *ps, int cx)
{
	assert(((int)pLine & 0x03) == 0); // Should be aligned to align??

	int cx2 = cx & 0xfffffffe;
	int denom, denom2;
	const IW::RGBSUM *ps2;

	LPBYTE pLineEnd = pLine + (cx * 2); 

	while(pLine < (pLineEnd - 2))
	{
		// blend with white
		denom = IW::Max(ps->c, 1u);

		ps2 = ps + 1;
		denom2 = IW::Max(ps2->c, 1u);

		*((int*)pLine) = 
			(((ps->r / denom) >> 3) & 0x0000001F) |
			(((ps->g / denom) << 2) & 0x000003E0) |
			(((ps->b / denom) << 7) & 0x00007C00) |
			(((ps2->r / denom2) << 13) & 0x001F0000) |
			(((ps2->g / denom2) << 18) & 0x03E00000) |
			(((ps2->b / denom2) << 23) & 0x7C000000);

		ps += 2;
		pLine += 4;
	}

	// May be a last one!!
	while(pLine < pLineEnd)
	{
		denom = IW::Max(ps->c, 1u);

		*((short*)pLine) = 
			(((ps->r / denom) >> (3)) & 0x0000001F) |
			(((ps->g / denom) << (2)) & 0x000003E0) |
			(((ps->b / denom) << (7)) & 0x00007C00);

		ps++;
		pLine += 2;
	}
}

void IW::ToSums(IW::LPRGBSUM pSum, LPDWORD pLineIn, int cxOut, int cxIn, bool isHighQuality, bool bFirstY)
{
	int x = (cxOut >> 1) + cxIn;

	IW::LPCRGBSUM pSumEnd = pSum + cxOut;
	IW::LPCDWORD pLineEnd = pLineIn + cxIn;
	DWORD c, a;

	if (isHighQuality)
	{
		while(pLineIn < pLineEnd) 
		{
			assert(pSum < pSumEnd); // Check for out buffer overflow

			c = *pLineIn++;
			a = IW::GetA(c);

			if (a)
			{				
				pSum->r += IW::GetR(c);
				pSum->g += IW::GetG(c);
				pSum->b += IW::GetB(c);
				pSum->c++;
				pSum->a += a;
			}

			pSum->ac++;
			x -= cxOut;
			if (x < cxOut) 
			{
				pSum++;
				x += cxIn;
			}
		}
	}
	else
	{	
		if (bFirstY)
		{
			while(pLineIn < pLineEnd) 
			{
				assert(pSum < pSumEnd); // Check for out buffer overflow

				c = *pLineIn++;
				pSum->r += IW::GetR(c);
				pSum->g += IW::GetG(c);
				pSum->b += IW::GetB(c);
				pSum->c++;

				x -= cxOut;
				if (x < cxOut) 
				{
					pSum++;
					x += cxIn;		
				}						
			}
		}
		else
		{
			for(int i = 0; i < cxOut; i++)
			{
				int xx = (i * cxIn) / cxOut;
				assert(xx < cxIn);

				c = pLineIn[xx];

				pSum->r += IW::GetR(c);
				pSum->g += IW::GetG(c);
				pSum->b += IW::GetB(c);
				pSum->c++;

				pSum++;
			}
		}
	}
}


void IW::Convert1to32(LPCOLORREF pBitsOut, LPCBYTE pBitsInIn, const int nStart, const int nSize)
{
	const int nSizeWithStart = nSize + nStart;
	IW::LPCDWORD pBitsIn = (IW::LPCDWORD)pBitsInIn;
	DWORD c = pBitsIn[nStart >> 5];
	int xn;

	for(int xx = nStart; xx < nSizeWithStart; xx++)
	{
		xn = xx & 31;
		if (xn == 0)
		{
			c = pBitsIn[xx >> 5];
		}
		
		*pBitsOut++ = (c & masks1[xn]) ? 1 : 0;
	}
}

void IW::Convert1to32(LPCOLORREF pBitsOut, LPCBYTE pBitsInIn, const int nStart, const int nSize, IW::LPCCOLORREF pPalette)
{
	IW::Convert1to32(pBitsOut, pBitsInIn, nStart, nSize);
	IW::IndexesToRGBA(pBitsOut, nSize, pPalette);
}

void IW::Convert2to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize)
{
	const int nSizeWithStart = nSize + nStart;

	for(int xx = nStart; xx < nSizeWithStart; xx++)
	{
		BYTE b = (pBitsIn[xx >> 2] & masks2[xx & 2]) >> shift2[xx & 2];
		*pBitsOut++ = b;
	}
}

void IW::Convert2to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize, IW::LPCCOLORREF pPalette)
{
	IW::Convert2to32(pBitsOut, pBitsIn, nStart, nSize);
	IW::IndexesToRGBA(pBitsOut, nSize, pPalette);
}

void IW::Convert4to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize)
{
	const int nSizeWithStart = nSize + nStart;
	int c;

	for(int xx = nStart; xx < nSizeWithStart; xx++)
	{
		c = (xx & 1) ? pBitsIn[xx >> 1] & 0x0f : pBitsIn[xx >> 1] >> 4;
		*pBitsOut++ = c;
	}
}

void IW::Convert4to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize, IW::LPCCOLORREF pPalette)
{
	IW::Convert4to32(pBitsOut, pBitsIn, nStart, nSize);
	IW::IndexesToRGBA(pBitsOut, nSize, pPalette);
}

void IW::Convert8to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize)
{
	const int nSizeWithStart = nSize + nStart;
	pBitsIn += nStart;

	for(int n = nStart; n < nSizeWithStart; n++)
	{
		*pBitsOut++ = *pBitsIn++;
	}
}

void IW::Convert8to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize, IW::LPCCOLORREF pPalette)
{
	IW::Convert8to32(pBitsOut, pBitsIn, nStart, nSize);
	IW::IndexesToRGBA(pBitsOut, nSize, pPalette);
}

void IW::Convert8Alphato32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize, IW::LPCCOLORREF pPalette)
{
	const int nSizeWithStart = nSize + nStart;
	pBitsIn += nStart;

	for(int n = nStart; n < nSizeWithStart; n++)
	{
		*pBitsOut++ = pPalette[*pBitsIn++];
	}
}

void IW::Convert8GrayScaleto32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize)
{
	int c;
	const int nSizeWithStart = nSize + nStart;
	pBitsIn += nStart;

	for(int n = nStart; n < nSizeWithStart; n++)
	{
		c = *pBitsIn++;
		*pBitsOut++ = RGB(c,c,c);
	}
}

void IW::Convert555to32(LPCOLORREF pBitsOut, LPCBYTE pBitsInIn, const int nStart, const int nSize)
{
	// Get a pointer to the end
	const COLORREF *pBitsOutEnd = pBitsOut + nSize;
	LPDWORD pBitsIn = (LPDWORD)(pBitsInIn + (nStart*2));
	DWORD c;

	while(pBitsOut < pBitsOutEnd)
	{
		c = *pBitsIn++;
		*pBitsOut++ = IW::RGB555to888(c) | 0xFF000000;

		if (pBitsOut >= pBitsOutEnd)
			return;

		c >>= 16;
		*pBitsOut++ = IW::RGB555to888(c) | 0xFF000000;
	}
}

void IW::Convert565to32(LPCOLORREF pBitsOut, LPCBYTE pBitsInIn, const int nStart, const int nSize)
{
	// Get a pointer to the end
	const COLORREF *pBitsOutEnd = pBitsOut + nSize;
	LPCBYTE pBitsIn = pBitsInIn + nStart*2;
	DWORD c;

	// Move to an alligned bit part of memory
	while((0 != ((int)pBitsOut & 3)) && (pBitsOut < pBitsOutEnd))
	{
		c = *reinterpret_cast<const unsigned short*>(pBitsIn);
		*pBitsOut++ = IW::RGB565to888(c) | 0xFF000000;
		pBitsIn += 2;
	}

	while(pBitsOut < pBitsOutEnd)
	{
		c = *((LPDWORD)pBitsIn);
		*pBitsOut++ = IW::RGB565to888(c) | 0xFF000000;

		if (pBitsOut >= pBitsOutEnd)
			return;

		c >>= 16;
		*pBitsOut++ = IW::RGB565to888(c) | 0xFF000000;
		pBitsIn += 4;
	}
}

void IW::Convert24to32(LPCOLORREF pBitsOut, LPCBYTE pBitsIn, const int nStart, const int nSize)
{
	// Get a pointer to the end
   const COLORREF *pBitsOutEnd = pBitsOut + nSize;

   // Find start location
   if (nStart)
      pBitsIn += 3 * nStart;
   // Move to an aligned bit part of memory
   LPBYTE pOut = (LPBYTE)pBitsOut;
   // Align to 8 byte boundry
   while((0 != ((int)pBitsIn & 3)) && (pOut < (LPCBYTE)pBitsOutEnd))
   {
      *pOut++ = *pBitsIn++;
      *pOut++ = *pBitsIn++;
      *pOut++ = *pBitsIn++;
      *pOut++ = (BYTE)0xff;
   }

   pBitsOut = (LPDWORD)pOut;

   if (IW::HasMMX() && pBitsOut < (pBitsOutEnd - 8))
   {
      // MMX Version
      //
      // The theory here is use the 64 bit MMX registers 
      // to suck in 24 bit pixels; I can then use a series 
      // of rotations masks and re-packing to push out 
      // 32 bit pixels.
      //
      // I don’t really understand how to optimize this for 
      // pairing but my plan is to try not to use the same 
      // registers next to each other, I am interested if 
      // anyone has some opinions on this?

      __int64 mask0=0x0ff000000ff000000;
      __int64 mask1=0x00000ffffffff0000;

      _asm 
      {
         mov edx, pBitsOutEnd
         sub edx, 32
         mov edi, pOut
         mov esi, pBitsIn
         movq mm7, mask0
         movq mm6, mask1
TheLoop:
         movq mm0, [esi] // Pixel 1
         movq mm1, mm0
         PSRLQ mm1, 24 // Pixel 2
         movq mm2, [esi + 8]
         movq mm3, mm2
         PSRLQ mm3, 8 // Pixel 4
         PSLLQ mm2, 16
         PAND mm2, mm6 
         movq mm4, mm0
         PSRLQ mm4, 48
         POR mm2, mm4 // Pixel 3
         punpckldq mm0, mm1
         punpckldq mm2, mm3
         POR mm0, mm7
         POR mm2, mm7
         movq [edi], mm0 // Write pixels 1 and 2
         movq [edi + 8], mm2 // Write pixels 3 and 4
         movq mm0, mm3
         PSRLQ mm0, 24 // Pixel 5
         PSRLQ mm3, 48 // Pixel 6
         movq mm1, [esi + 16]
         movq mm2, mm1
         PSLLQ mm1, 8
         por mm1, mm3
         movq mm2, [esi + 16]
         PSRLQ mm2, 16 // Pixel 7
         movq mm3, mm2
         PSRLQ mm3, 24 // Pixel 8
         punpckldq mm0, mm1
         punpckldq mm2, mm3
         POR mm0, mm7
         POR mm2, mm7 
         movq [edi + 16], mm0 // Write pixels 5 and 6
         movq [edi + 24], mm2 // Write pixels 7 and 8
         add esi, 24 // Increment source pointer
         add edi, 32 // Increment destination pointer
         cmp edi, edx
         jl TheLoop
         mov pOut, edi
         mov pBitsIn, esi

         EMMS
      }
   }
   else
   {
      // Continue processing
      UINT nPixel1, nPixel2;
      IW::LPCDWORD pIn = (IW::LPCDWORD)pBitsIn;
      const COLORREF *pBitsOutEnd2 = pBitsOutEnd - 4;

      while(pBitsOut < pBitsOutEnd2) 
      {
         // DWORD 1
         nPixel1 = *pIn++;

         // pixel 1
         *pBitsOut++ = (nPixel1 & 0x00ffffff) | 0xff000000;

         // pixel 2
         nPixel2 = (nPixel1 >> 24) & 0xff;

         // DWORD 2
         nPixel1 = *pIn++;

         nPixel2 |= (nPixel1 << 8) & 0x00ffff00;

         *pBitsOut++ = nPixel2 | 0xff000000;
         // pixel 3
         nPixel2 = (nPixel1 >> 16) & 0x0000ffff;

         // DWORD 3
         nPixel1 = *pIn++;
         nPixel2 |= (nPixel1 << 16) & 0x00ff0000;
         *pBitsOut++ = nPixel2 | 0xff000000;

         // pixel 4
         *pBitsOut++ = (nPixel1 >> 8) | 0xff000000;
      }
      pBitsIn = (LPCBYTE)pIn;
      pOut = (LPBYTE)pBitsOut;
   }

   // Complete any incomplete work
   while(pOut < (LPCBYTE)pBitsOutEnd)
   {
      *pOut++ = *pBitsIn++;
      *pOut++ = *pBitsIn++;
      *pOut++ = *pBitsIn++;
      *pOut++ = (BYTE)0xff;
   }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void IW::ConvertARGBtoABGR(LPBYTE pBitsOutX, LPCBYTE pBitsInX, const int nSize)
{
	LPDWORD pBitsIn = (LPDWORD)pBitsInX;
	LPDWORD pBitsOut = (LPDWORD)pBitsOutX;
	const LPDWORD pBitsOutEnd = pBitsOut + nSize;

	while(pBitsOut< pBitsOutEnd)
	{
		*pBitsOut++ = SwapRB(*pBitsIn++);
	}
}


void IW::ConvertRGBtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
	const LPBYTE pBitsOutEnd = pBitsOut + (nSize * 3);
	BYTE r,g,b;

	while(pBitsOut< pBitsOutEnd)
	{
		r = *pBitsIn++;
		g = *pBitsIn++;
		b = *pBitsIn++;

		*pBitsOut++ = b;
		*pBitsOut++ = g;
		*pBitsOut++ = r;
	}
}

void IW::ConvertYCbCrtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
	
}

void IW::ConvertCMYKtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
	int xIn = 0; 
	int xOut = 0;
	int C, M, Y, K;

	for(int i = 0; i < nSize; i++)
	{
		C = pBitsIn[xIn + 0];
		M = pBitsIn[xIn + 1];
		Y = pBitsIn[xIn + 2];
		K = pBitsIn[xIn + 3];

		pBitsOut[xOut + 0] = IW::ByteClamp(255 - (Y + K));
		pBitsOut[xOut + 1] = IW::ByteClamp(255 - (M + K));
		pBitsOut[xOut + 2] = IW::ByteClamp(255 - (C + K));

		xIn += 4;
		xOut += 3;
	}
	
}

void IW::ConvertYCCKtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
}

void IW::ConvertCIELABtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
	int x = 0;
	int R, G, B;
	int L, a, b;

	for(int i = 0; i < nSize; i++)
	{
		L = pBitsIn[x + 2];
		a = pBitsIn[x + 1];
		b = pBitsIn[x + 0];

		IW::LABtoRGB(L, a, b, R, G, B);

		pBitsOut[x + 0] = R;
		pBitsOut[x + 1] = G;
		pBitsOut[x + 2] = B;
		x += 3;
	}

	
}

void IW::ConvertICCLABtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
	IW::ConvertCIELABtoBGR(pBitsOut, pBitsIn, nSize);
}

void IW::ConvertITULABtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
	IW::ConvertCIELABtoBGR(pBitsOut, pBitsIn, nSize);
}

void IW::ConvertLOGLtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
}

void IW::ConvertLOGLUVtoBGR(LPBYTE pBitsOut, LPCBYTE pBitsIn, const int nSize)
{
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void IW::RGBtoHSL(int R, int G, int B, int &H, int &S, int &L)
{
	const int HSLMAX = 255;	/* H,L, and S vary over 0-HSLMAX */
	const int RGBMAX = 255;   /* R,G, and B vary over 0-RGBMAX */
	/* HSLMAX BEST IF DIVISIBLE BY 6 */
	/* RGBMAX, HSLMAX must each fit in a BYTE. */
	/* Hue is undefined if Saturation is 0 (grey-scale) */
	/* This value determines where the Hue scrollbar is */
	/* initially set for achromatic colors */
	const int UNDEFINED = (HSLMAX*2/3);

	WORD Rdelta,Gdelta,Bdelta;	/* intermediate value: % of spread from max*/
	const BYTE cMax = IW::Max( IW::Max(R,G), B);	/* calculate lightness */
	const BYTE cMin = IW::Min( IW::Min(R,G), B);

	L = (BYTE)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

	if (cMax==cMin)				/* r=g=b --> achromatic case */
	{			
		S = 0;					/* saturation */
		H = UNDEFINED;			/* hue */
	} 
	else 
	{							/* chromatic case */
		if (L <= (HSLMAX/2))	/* saturation */
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
		else
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
		/* hue */
		Rdelta = (WORD)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Gdelta = (WORD)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Bdelta = (WORD)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

		if (R == cMax)
			H = (BYTE)(Bdelta - Gdelta);
		else if (G == cMax)
			H = (BYTE)((HSLMAX/3) + Rdelta - Bdelta);
		else /* B == cMax */
			H = (BYTE)(((2*HSLMAX)/3) + Gdelta - Rdelta);

		if (H < 0) H += HSLMAX;
		if (H > HSLMAX) H -= HSLMAX;
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////
static float HueToRGB(float n1,float n2, float hue)
{
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	float rValue;

	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		rValue = n1 + (n2-n1)*hue/60.0f;
	else if (hue < 180)
		rValue = n2;
	else if (hue < 240)
		rValue = n1+(n2-n1)*(240-hue)/60;
	else
		rValue = n1;

	return rValue;
}

////////////////////////////////////////////////////////////////////////////////


void IW::HSLtoRGB(int H, int S, int L, int &r, int &g, int &b)
{ 
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	const float h = (float)H * 360.0f/255.0f;
	const float s = (float)S/255.0f;
	const float l = (float)L/255.0f;
	const float m2 = (l <= 0.5) ? l * (1+s) : l + s - l*s;
	const float m1 = 2 * l - m2;

	if (s == 0) 
	{
		r=g=b=(BYTE)(l*255.0f);
	} 
	else 
	{
		r = (int)(HueToRGB(m1,m2,h+120) * 255.0f);
		g = (int)(HueToRGB(m1,m2,h) * 255.0f);
		b = (int)(HueToRGB(m1,m2,h-120) * 255.0f);
	}

	return;
}


void IW::RGBtoLAB(int R, int G, int B, int &L, int &a, int &b)
{
	// Convert between RGB and CIE-Lab color spaces
	// Uses ITU-R recommendation BT.709 with D65 as reference white.
	// algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>
	double fX, fY, fZ;
	double X = 0.412453*R + 0.357580*G + 0.180423*B;
	double Y = 0.212671*R + 0.715160*G + 0.072169*B;
	double Z = 0.019334*R + 0.119193*G + 0.950227*B;

	X /= (255 * 0.950456);
	Y /=  255;
	Z /= (255 * 1.088754);

	if (Y > 0.008856)
	{
		fY = pow(Y, 1.0/3.0);
		L = static_cast<int>(116.0*fY - 16.0 + 0.5);
	}
	else
	{
		fY = 7.787*Y + 16.0/116.0;
		L = static_cast<int>(903.3*Y + 0.5);
	}

	if (X > 0.008856)
		fX = pow(X, 1.0/3.0);
	else
		fX = 7.787*X + 16.0/116.0;

	if (Z > 0.008856)
		fZ = pow(Z, 1.0/3.0);
	else
		fZ = 7.787*Z + 16.0/116.0;

	a = static_cast<int>(500.0*(fX - fY) + 0.5);
	b = static_cast<int>(200.0*(fY - fZ) + 0.5);
}

void IW::LABtoRGB(int L, int a, int b, int &R, int &G, int &B)
{
	// Convert between RGB and CIE-Lab color spaces
	// Uses ITU-R recommendation BT.709 with D65 as reference white.
	// algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>
	double X, Y, Z;
	double fY = pow((L + 16.0) / 116.0, 3.0);
	if (fY < 0.008856)
		fY = L / 903.3;
	Y = fY;

	if (fY > 0.008856)
		fY = pow(fY, 1.0/3.0);
	else
		fY = 7.787 * fY + 16.0/116.0;

	double fX = a / 500.0 + fY;          
	if (fX > 0.206893)
		X = pow(fX, 3.0);
	else
		X = (fX - 16.0/116.0) / 7.787;

	double fZ = fY - b /200.0;          
	if (fZ > 0.206893)
		Z = pow(fZ, 3.0);
	else
		Z = (fZ - 16.0/116.0) / 7.787;

	X *= (0.950456 * 255);
	Y *= 255;
	Z *= (1.088754 * 255);

	int RR = static_cast<int>(3.240479*X - 1.537150*Y - 0.498535*Z + 0.5);
	int GG = static_cast<int>(-0.969256*X + 1.875992*Y + 0.041556*Z + 0.5);
	int BB = static_cast<int>(0.055648*X - 0.204043*Y + 1.057311*Z + 0.5);

	R = RR < 0 ? 0 : RR > 255 ? 255 : RR;
	G = GG < 0 ? 0 : GG > 255 ? 255 : GG;
	B = BB < 0 ? 0 : BB > 255 ? 255 : BB;
}