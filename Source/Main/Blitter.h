///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Blitter.cpp: implementation of the CRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


//////////////////////////////////////////////////////////////////////////////////
	/// Blintter interface

class CBlitter
{
public:


	CBlitter() {};
	~CBlitter() {};

	static DWORD Make4(DWORD dw)
	{
		DWORD dwOut = ((dw & 0xff) << 8) | (dw & 0xff);
		return dwOut | (dwOut << 16);
	}


	inline void RenderAlphaLine(COLORREF *pLineOut, const COLORREF *pLineIn, int nLength)
	{
		DWORD cIn, cOut, aIn, aOut, r, g, b, a;
		const COLORREF *pLineInEnd = pLineIn + nLength;

		while(pLineIn < pLineInEnd)
		{
			cIn = *pLineIn;
			aIn = IW::GetA(cIn);
			aOut = 0xff - aIn;

			if (aIn == 0xff)
			{
				*pLineOut = cIn;
			}
			else if (aOut != 0x00)
			{					
				cOut = *pLineOut;			

				r = (IW::GetR(cIn) * aIn) + (IW::GetR(cOut) * aOut);
				g = (IW::GetG(cIn) * aIn) + (IW::GetG(cOut) * aOut);
				b = (IW::GetB(cIn) * aIn) + (IW::GetB(cOut) * aOut);
				a = (IW::GetA(cIn) * aIn) + (IW::GetA(cOut) * aOut);

				*pLineOut = IW::RGBA((r >> 8), (g >> 8), (b >> 8), (a >> 8));
			}

			++pLineIn;
			++pLineOut;
		}
	}

	inline void RenderTransparentLine(COLORREF *pLineOut, const BYTE *pLineIn, int nLength, const COLORREF *pRGB, const DWORD dwTrans)
	{
		DWORD cc;
		const BYTE *pLineInEnd = pLineIn + nLength;

		while(pLineIn < pLineInEnd)
		{
			cc = *pLineIn;

			if (cc != dwTrans)
			{
				*pLineOut = pRGB[cc] | 0xFF000000;
			}

			++pLineIn;
			++pLineOut;
		}
	}

	void InterpolateLine(COLORREF *pLineOut, COLORREF *pLineInScaled1, COLORREF *pLineInScaled2, const DWORD &a1, const DWORD &a2, const int nWidth)
	{
		const COLORREF *pLineOutEnd = pLineOut + nWidth;
		DWORD c1, c2;

		while(pLineOut < pLineOutEnd)
		{
			c1 = *pLineInScaled1++;
			c2 = *pLineInScaled2++;

			*pLineOut++ = IW::RGBA(
				(((IW::GetR(c1) * a1) + (IW::GetR(c2) * a2)) >> 0x8), 
				(((IW::GetG(c1) * a1) + (IW::GetG(c2) * a2)) >> 0x8), 
				(((IW::GetB(c1) * a1) + (IW::GetB(c2) * a2)) >> 0x8), 
				0xff);
		}
	}

	void InterpolateLine(COLORREF *pLineOut, COLORREF *pLineIn, DWORD *pLookupX, DWORD *pLookupXDiff, int nSize)
	{
		DWORD c1, c2, a1, a2;

		for(int x = 0; x < nSize; x++)
		{
			c1 = pLineIn[LOWORD(pLookupX[x])];
			c2 = pLineIn[HIWORD(pLookupX[x])];

			a1 = LOWORD(pLookupXDiff[x]);
			a2 = HIWORD(pLookupXDiff[x]);

			pLineOut[x] = IW::RGBA(
				(((IW::GetR(c1) * a1) + (IW::GetR(c2) * a2)) >> 0x8), 
				(((IW::GetG(c1) * a1) + (IW::GetG(c2) * a2)) >> 0x8), 
				(((IW::GetB(c1) * a1) + (IW::GetB(c2) * a2)) >> 0x8), 
				0xff);
		}
	}

	void Blend32(LPDWORD pDst, LPDWORD pSrc, int nSize)
	{
		LPDWORD pDstEnd = pDst + nSize;

		_asm
		{
			mov         edi,dword ptr pDst
			mov         esi,dword ptr pSrc
			mov			edx,dword ptr pDstEnd

	BlendLoop:
			cmp         edi,edx
			jae         EndBlend

			mov         ebx,dword ptr [edi]
			and         ebx,0FEFEFEFEh
			shr         ebx,1
			
			LODSD

			and         eax,0FEFEFEFEh
			shr         eax,1
			add         eax,ebx

			STOSD

			jmp         BlendLoop
	EndBlend:
		}
	}

	void BlendColor32(LPDWORD pDst, COLORREF clr, int nSize)
	{
		LPDWORD pDstEnd = pDst + nSize;

		_asm
		{
			mov         edi,dword ptr pDst
			mov			edx,dword ptr pDstEnd
			mov			ebx,dword ptr clr
			and         ebx,0FEFEFEFEh
			shr         ebx,1

	BlendLoop:
			cmp         edi,edx
			jae         EndBlend

			mov         eax,dword ptr [edi]
			and         eax,0FEFEFEFEh
			shr         eax,1
			add         eax,ebx

			STOSD

			jmp         BlendLoop
	EndBlend:
		}
	}

	void Fill32(LPDWORD pDst, COLORREF clr, int nSize)
	{
		_asm
		{
			MOV EDI,pDst       ;get memory area to use
			MOV EAX,clr
			MOV ECX,nSize
			REP STOSD           ;fill result buffer
		}
	}



#include <pshpack1.h>

	typedef struct
	{
		WORD r; 
		WORD g;
		WORD b;
		WORD a;
		WORD cr; 
		WORD cg;
		WORD cb;
		WORD ca;
	}
	SUM;

#include <poppack.h>


	void ScaleDownLine(LPDWORD pSumIn, COLORREF *pLineIn, const int cxIn, const int cxOut)
	{
		int x = (cxOut >> 1) + cxIn;
		DWORD c;
		SUM *pSum = (SUM*)pSumIn;
		COLORREF *pLineEnd = pLineIn + cxIn;

		while(pLineIn < pLineEnd) 
		{
			if (pSum->cr < 0xff)
			{
				c = *pLineIn++;

				pSum->r += static_cast<WORD>(IW::GetR(c));
				pSum->g += static_cast<WORD>(IW::GetG(c));
				pSum->b += static_cast<WORD>(IW::GetB(c));
				pSum->a += static_cast<WORD>(IW::GetA(c));
				pSum->cr++;
				pSum->cg++;
				pSum->cb++;
				pSum->ca++;
				
			}
			else
			{
				++pLineIn;
			}

			x -= cxOut;
			if (x < cxOut) 
			{
				++pSum;
				x += cxIn;
			}
		}
	}

	//#pragma optimize( "", off )

	void ScaleDownLineFast(LPDWORD pSumIn, COLORREF *pLineIn, const int cxIn, const int cxOut)
	{
		int x = (cxOut >> 1) + cxIn;
		DWORD c;
		SUM *pSum = (SUM*)pSumIn;
		COLORREF *pLineEnd = pLineIn + cxIn;
		bool bFirst;

		while(pLineIn < pLineEnd) 
		{
			bFirst = true;

			if (bFirst && pSum->cr < 0xff)
			{
				c = *pLineIn;

				pSum->r += static_cast<WORD>(IW::GetR(c));
				pSum->g += static_cast<WORD>(IW::GetG(c));
				pSum->b += static_cast<WORD>(IW::GetB(c));
				pSum->a += static_cast<WORD>(IW::GetA(c));
				pSum->cr++;
				pSum->cg++;
				pSum->cb++;
				pSum->ca++;

				bFirst = false;				
			}

			++pLineIn;
			x -= cxOut;

			if (x < cxOut) 
			{
				++pSum;
				x += cxIn;
			}
		}
	}


	//#pragma optimize( "", on )

	void RenderScaleDownLine(COLORREF *pLineOut, LPDWORD pSumIn, const int nWidth)
	{
		const SUM *ps = (SUM*)pSumIn;

		DWORD rr, gg, bb, aa, aOut, c;

		for(int x = 0; x < nWidth; x++)
		{		
			/*rr = ps->r / ps->cr;
			gg = ps->g / ps->cg;
			bb = ps->b / ps->cb;
			aa = ps->a / ps->ca;		*/

			c = ps->cr;

			rr = ps->r / c;
			gg = ps->g / c;
			bb = ps->b / c;
			aa = ps->a / c;

			aOut = 0xff - aa;

			if (aa > 0xf0)
			{
				c = IW::RGBA(rr, gg, bb, 0xff);
			}
			else if (aOut != 0x00)
			{					
				c = *pLineOut;

				rr = (rr * aa) + (IW::GetR(c) * aOut);
				gg = (gg * aa) + (IW::GetG(c) * aOut);
				bb = (bb * aa) + (IW::GetB(c) * aOut);
				aa = (aa * aa) + (IW::GetA(c) * aOut);

				c = IW::RGBA((rr >> 8), (gg >> 8), (bb >> 8), (aa >> 8));
			}

			*pLineOut++ = c;			
			++ps;								
		}
	}

};




class CBlitterMMX : public CBlitter
{
public:

	CBlitterMMX() {};
	~CBlitterMMX() {};

	
	static void MemSetMMX( LPVOID lpMemInOut, DWORD dwData, DWORD dwSize )
	{

		// Aigned?
		assert(((long)lpMemInOut & 0x03) == 0);

		_asm
		{
		
		mov eax , dwData
		mov	edi, lpMemInOut
		mov	edx, dwSize
		cmp	edx, 64
		jb	SM2

		movd mm0,eax
		movq mm1,mm0
		psllq mm0,32
		por mm0,mm1

		movq mm1,mm0
		movq mm2,mm0
		movq mm3,mm1
		movq mm4,mm0
		movq mm5,mm2
		movq mm6,mm0
		movq mm7,mm3

	SM1:	
		
		movq	[edi], mm0
		movq	[edi + 8], mm1
		movq	[edi + 16], mm2
		movq	[edi + 24], mm3
		movq	[edi + 32], mm4
		movq	[edi + 40], mm5
		movq	[edi + 48], mm6
		movq	[edi + 56], mm7

		add	edi, 64
		sub edx, 64
		cmp edx, 64
		jae  SM1	

		

		and	edx, 63
		jz	SM3

	SM2:	// plain old bcopy(esi, edi, eax)
		
		mov	ecx, edx
		shr	ecx, 2
		rep stosd

		mov	ecx, edx
		and	ecx, 3
		jz	SM3

		rep stosb

	SM3:

		// Restore FPU state.
		EMMS
		}
	}

	void InterpolateLine(COLORREF *pLineOut, COLORREF *pLineInScaled1, COLORREF *pLineInScaled2, const DWORD &a1o, const DWORD &a2o, const int nWidth)
	{
		const COLORREF *pLineOutEnd = pLineOut + nWidth;

		DWORD a1 = Make4(a1o);
		DWORD a2 = Make4(a2o);	

		_asm
		{
			PXOR      MM7, MM7

			movd	  MM4,a1
			punpcklbw MM4,MM7

			movd	  MM5,a2
			punpcklbw MM5,MM7

			mov esi, pLineInScaled1
			mov ecx, pLineInScaled2
			mov edi, pLineOut
			mov edx, pLineOutEnd

		BlendLoop:				

			movd MM0,[esi]          ;load pixel 1
			movd MM1,[ecx]          ;load pixel 2

			punpcklbw MM0,MM7       ;copy the lower 32 bits of MM0 into MM0
			punpcklbw MM1,MM7       ;copy the lower 32 bits of MM1 into MM1

			PMULLW MM0,MM4
			PMULLW MM1,MM5

			paddusw MM0,MM1
			PSRLW MM0,8

			packuswb MM0,MM7
			
			movd [edi], MM0

			add esi,4
			add edi,4
			add ecx,4

			cmp         edi,edx
			jne         BlendLoop

			EMMS
		}
	}

	

	void InterpolateLine(COLORREF *pLineOut, COLORREF *pLineIn, DWORD *pLookupX, DWORD *pLookupXDiff, int nSize)
	{
		const COLORREF *pLineOutEnd = pLineOut + nSize;

		
		_asm
		{
			PXOR            MM7, MM7

			mov esi, pLineIn
			mov edi, pLineOut

			mov ecx, pLookupX
			mov edx, pLookupXDiff
			//mov edx, pLineOutEnd

			cmp         edi,pLineOutEnd
			jae         BlendLoopEnd


		BlendLoop:				

			mov   eax, [ecx]
			mov   ebx, eax
			and   eax, 0x0ffff
			movd  MM0, [esi + eax * 4]          ;load pixel 1

			shr   ebx, 0x10
			movd  MM1, [esi + ebx * 4]          ;load pixel 2

			mov			eax, [edx]
			mov			ah, al ; eax = 0x0000abab
			mov			bx, ax
			shl			eax, 16 ; eax = 0xabab0000
			mov			ax, bx ; eax = 0xabababab
			movd		MM4,eax
			punpcklbw	MM4,MM7

			mov			EAX, 0xffffffff
			movd		MM5, eax
			punpcklbw	MM5, MM7
			psubusw		MM5, MM4

			punpcklbw MM0,MM7       ;copy the lower 32 bits of MM0 into MM0
			punpcklbw MM1,MM7       ;copy the lower 32 bits of MM1 into MM1

			PMULLW MM0,MM4
			PMULLW MM1,MM5

			paddusw MM0,MM1
			PSRLW MM0,8

			packuswb MM0,MM7
			
			movd [edi], MM0

			add edi,4
			add ecx,4
			add edx,4

			cmp         edi,pLineOutEnd
			jne         BlendLoop

		BlendLoopEnd:

			EMMS
		}
	}

	

	void Fill32(LPDWORD pDst, COLORREF clr, int nSize)
	{
		// Aigned?
		assert(((long)pDst & 0x03) == 0);
		MemSetMMX( pDst, clr, nSize * 4 );
	}

	
	
	void Blend32(LPDWORD pDst, LPDWORD pSrc, int nSize)
	{
		LPDWORD pDstEnd = pDst + nSize;

		_asm
		{
			mov         edi,dword ptr pDst
			mov         esi,dword ptr pSrc
			mov			edx,dword ptr pDstEnd
			
			mov ecx,nSize
			cmp ecx,6
			jb	BlendLoopDone

			mov eax, 0FEFEFEFEh
			movd mm0,eax
			movq mm1,mm0
			psllq mm0,32
			por mm0,mm1
			movq mm7, mm0

	BlendLoop:
			
			movq mm1, [edi]
			movq mm3, [edi+8]
			movq mm5, [edi+16]

			movq mm2, [esi]			
			movq mm4, [esi+8]			
			movq mm6, [esi+16]

			PAND mm1, mm0           
			PAND mm2, mm7
			PAND mm3, mm0           
			PAND mm4, mm7
			PAND mm5, mm0           
			PAND mm6, mm7
			
			PSRLQ mm1, 1
			PSRLQ mm2, 1
			PSRLQ mm3, 1
			PSRLQ mm4, 1
			PSRLQ mm5, 1
			PSRLQ mm6, 1

			PADDUSB mm1, mm2
			PADDUSB mm3, mm4
			PADDUSB mm5, mm6

			movq [edi], mm1
			movq [edi+8], mm3
			movq [edi+16], mm5

			add edi, 24
			add esi, 24

			sub ecx, 6
			cmp ecx, 6
			JAE BlendLoop	

			
			

	BlendLoopDone:

			cmp         edi,edx
			jae         EndBlend

			mov         ebx,dword ptr [edi]
			and         ebx,0FEFEFEFEh
			shr         ebx,1
			
			LODSD

			and         eax,0FEFEFEFEh
			shr         eax,1
			add         eax,ebx

			STOSD

			//jmp         BlendLoop

	EndBlend:

			EMMS

			
		}
	}
	

	static DWORD *GetReciprocalTable()
	{
		static DWORD table[0x100];
		static bool bInit = false;

		if (!bInit)
		{
			WORD r = 0;

			table[0] = 0;
			table[1] = 0x07fff;

			for(int i = 2; i < 0x100; ++i)
			{
				r = (WORD)((1.0 / ((double)i)) * (double)(0x7fff));
				table[i] = r;
			}

			bInit = true;
		}

		return table;
	}

	

	

	void RenderScaleDownLine(COLORREF *pLineOut, LPDWORD pSumIn, const int nWidth)
	{
		if (nWidth < 1) return;

		const COLORREF *pLineOutEnd = pLineOut + nWidth;
		const COLORREF *pLineOutEnd2 = pLineOut + (nWidth & 0x0fffc);

		SUM *pSum = (SUM*)pSumIn;

		DWORD *reciprocals = GetReciprocalTable();

		for(int x = 0; x < nWidth; ++x)
		{
			SUM *pSum2 = pSum + x;

			pSum2->cr = 
			pSum2->cg = 
			pSum2->cb = 
			pSum2->ca = static_cast<WORD>(reciprocals[static_cast<BYTE>(pSum2->cr)]);
		}

		
		_asm
		{
			PXOR            MM7, MM7
			PXOR            MM6, MM6
			PXOR            MM5, MM5
			PXOR            MM4, MM4

			mov esi, pSumIn
			mov edi, pLineOut
			mov ecx, pLineOutEnd2
			cmp      edi,ecx
			jae      TheLoopBulkEnd


		TheBulkLoop:

			movq	 MM0, [esi] 
			movq	 MM1, [esi+16] 
			movq	 MM2, [esi+32] 
			movq	 MM3, [esi+48] 

			PMULHW   MM0, [esi + 8]
			PMULHW   MM1, [esi + 24]
			PMULHW   MM2, [esi + 40]
			PMULHW   MM3, [esi + 56]

			PSLLW    MM0, 1
			PSLLW    MM1, 1
			PSLLW    MM2, 1
			PSLLW    MM3, 1

			packuswb MM0, MM4			
			packuswb MM1, MM5	
			packuswb MM2, MM6	
			packuswb MM3, MM7	
			
			movd     [edi], MM0
			movd     [edi+4], MM1
			movd     [edi+8], MM2
			movd     [edi+12], MM3

			add		 esi, 64
			add		 edi,16
			cmp      edi,ecx
			jne      TheBulkLoop

		TheLoopBulkEnd:


			mov ecx, pLineOutEnd
			cmp      edi,ecx
			jae      TheLoopEnd

		TheLoop:

			movq	 MM0, [esi] 
			PMULHW   MM0, [esi + 8]
			PSLLW    MM0, 1
			packuswb MM0, MM7			
			movd     [edi], MM0

			add		 esi, 16
			add		 edi,4
			cmp      edi,ecx
			jne      TheLoop

		TheLoopEnd:

			EMMS
		}		
	}

	
	

	void ScaleDownLine(LPDWORD pSumIn, COLORREF *pLineIn, const int cxIn, const int cxOut)
	{
		int x = (cxOut >> 1) + cxIn;
		SUM *pSum = (SUM*)pSumIn;
		COLORREF *pLineEnd = pLineIn + cxIn;

		_asm
		{
			PXOR            MM7, MM7

			mov esi, pLineIn
			mov edi, pSumIn
			mov ecx, pLineEnd
			mov edx, x

			mov		  eax, 0x01010101
			movd	  MM6, eax
			punpcklbw MM6, MM7

		TheLoop:	

			movq  MM1, [edi]
			movq  MM2, [edi + 8]
			

		TheInnerLoop:
			movd	  MM0,[esi]
			punpcklbw MM0,MM7

			paddusw MM1, MM0
			paddusw MM2, MM6			

			add esi, 4

			sub edx, cxOut
			cmp edx, cxOut

			jae TheInnerLoop

			movq  [edi], MM1
			movq  [edi + 8], MM2

			add    edi, 16	
			add    edx, cxIn;
			cmp    esi,ecx
			jne    TheLoop

			EMMS
		}		
	}

	

	void ScaleDownLineFast(LPDWORD pSumIn, COLORREF *pLineIn, const int cxIn, const int cxOut)
	{
		int x = (cxOut >> 1) + cxIn;
		SUM *pSum = (SUM*)pSumIn;
		COLORREF *pLineEnd = pLineIn + cxIn;

		_asm
		{
			PXOR            MM7, MM7

			mov esi, pLineIn
			mov edi, pSumIn
			mov ecx, pLineEnd
			mov edx, x

			mov		  eax, 0x01010101
			movd	  MM6, eax
			punpcklbw MM6, MM7

		TheLoop:	

			movq  MM1, [edi]
			movq  MM2, [edi + 8]
		
			movd	  MM0,[esi]
			punpcklbw MM0,MM7

			paddusw MM1, MM0
			paddusw MM2, MM6
			
		TheInnerLoop:

			add esi, 4

			sub edx, cxOut
			cmp edx, cxOut

			jae TheInnerLoop

			movq  [edi], MM1
			movq  [edi + 8], MM2

			add    edi, 16	
			add    edx, cxIn;
			cmp    esi,ecx
			jne    TheLoop

			EMMS
		}		
	}
};

inline bool CanUseMMX()
{
	return IW::HasMMX() && App.Options.m_bUseMMX;
}

