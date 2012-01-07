///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Load.h: interface for the CLoadBase class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define COMPILE_GIF
#define COMPILE_PNG
#define COMPILE_TIFF
#define COMPILE_PSD

class  CLoadBase  : public IW::IImageLoader
{
public:
	
	// Construct destruct
	CLoadBase();
	virtual ~CLoadBase();
	
	// Serialize
	virtual void Read(const IW::IPropertyArchive *pArchive) {};
	virtual void Write(IW::IPropertyArchive *pArchive) const {};

	virtual bool Read(const CString &strType, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus) = 0;
	virtual bool Write(const CString &strType, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus) = 0;

	bool DisplaySettingsDialog(const IW::Image &image) { return true; };
	bool CreatePreview(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);

	bool IsPreviewScaleIndependant() const
	{
		return true;
	}

	void OnReset()
	{
	}

	void OnHelp() const;

	// Properties
	bool IterateProperties(IW::IPropertyStream *pStreamOut) const;
	IW::IPropertyStream* InsertProperties();	
};

template<class T>
class  CLoad : public CLoadBase
{
public:
	CString GetKey() const { return T::_GetKey(); };
	CString GetTitle() const { return T::_GetTitle(); };
	CString GetTitleWithSettings() const { return T::_GetTitle(); };

	void OnReset() {};
};


inline unsigned short ReadMSBShort(IW::IStreamIn *pStream) 
{ 
	unsigned short n = 0;
	pStream->Read(&n, sizeof(n), NULL);

	_asm mov ax,(n);
	_asm xchg al, ah; 
	_asm mov (n),ax;

	return n;
};

inline unsigned long ReadMSBLong(IW::IStreamIn *pStream) 
{ 
	unsigned long n = 0;
	pStream->Read(&n, sizeof(n), NULL);

	_asm mov eax,(n) ;
	_asm bswap eax ;
	_asm mov (n),eax;

	return n;
};

typedef int Quantum;
typedef int IndexPacket;
typedef DWORD PixelPacket;

const int MaxTextExtent = 300;
const int MaxRGB = 255;

inline BYTE ScaleCharToQuantum(BYTE bb) { return bb; };

inline int ReadBlobLSBLong(IW::IStreamIn *pStreamIn)
{
	int value;

	DWORD dw;
	if (!pStreamIn->Read(&value, 4, &dw) || (dw != 4))
	{
		throw IW::invalid_file();
	}

	return value;
}


inline unsigned short ReadBlobLSBShort(IW::IStreamIn *pStreamIn)
{
	unsigned short value;

	DWORD dw;
	if (!pStreamIn->Read(&value, 2, &dw) || (dw != 2))
	{
		throw IW::invalid_file();
	}

	return value;
}


inline unsigned short ReadBlobMSBShort(IW::IStreamIn *pStreamIn)
{
	unsigned char buffer[2];
	unsigned short value;

	DWORD dw;
	if (!pStreamIn->Read(buffer, 2, &dw) || (dw != 2))
	{
		throw IW::invalid_file();
	}

	value=buffer[0] << 8;
	value|=buffer[1];
	return(value);
}

inline BYTE ReadBlobByte(IW::IStreamIn *pStreamIn)
{
	BYTE b;
	DWORD dw;
	if (!pStreamIn->Read(&b, 1, &dw) || (dw != 1))
	{
		throw IW::invalid_file();
	}

	return b;
}

