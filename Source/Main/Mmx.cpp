#include "stdafx.h"

static DWORD Make4(DWORD dw)
{
	DWORD dwOut = ((dw & 0xff) << 8) | (dw & 0xff);
	return dwOut | (dwOut << 16);
}


static bool HasMMX2()
{ 
	DWORD dwCpuFlags;  
	
	__asm 
	{ 
		pushfd
		pop eax
		
		mov ecx,eax
		
		xor eax,040000h
		push eax
		
		popfd
		pushfd
		
		pop eax
		xor eax,ecx
		jz L1			; Processor is 386
		
		push ecx
		popfd
		
		mov eax,ecx
		xor eax,200000h
		
		push eax
		popfd
		pushfd
		
		pop eax
		xor eax,ecx
		je L1
		
		pusha
		
		mov eax,1
		cpuid
		
		mov [dwCpuFlags],edx
		
		popa
		
		mov eax,[dwCpuFlags]
			
	L1:	

		mov [dwCpuFlags], eax
		
	} 
	
	
	if (dwCpuFlags & 0x800000) 
	{
		// test bit 23 of feature flag 
		// != 0 if MMX is supported 
		return true; 
	}
	
	
	
	return false; 
} 


bool IW::HasMMX()
{ 
	static bool bHas = HasMMX2();
	return bHas; 
} 