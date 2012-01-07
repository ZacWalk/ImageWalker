///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


#pragma once

#include "Base.h"
#include "FileTime.h"

class PluginState;

namespace IW
{
	struct MetaDataTypes
	{
		enum 
		{
			UNKNOWN,
			IMAGE,

			PROFILE_COMMENT,
			PROFILE_IPTC,
			PROFILE_EXIF,
			PROFILE_ICC,
			PROFILE_XMP,

			JPEG_IMAGE,

			JPEG_APP00,
			JPEG_APP01,
			JPEG_APP02,
			JPEG_APP03,
			JPEG_APP04,
			JPEG_APP05,
			JPEG_APP06,
			JPEG_APP07,
			JPEG_APP08,
			JPEG_APP09,
			JPEG_APP10,
			JPEG_APP11,
			JPEG_APP12,
			JPEG_APP13,
			JPEG_APP14,
			JPEG_APP15,
			JPEG_APP16,

			GIF_IMAGE,
			PNG_IMAGE
		};
	};
	
	typedef struct tagMETADATA 
	{
		DWORD dwSize;
		IAtlMemMgr *pMemMgr;
		volatile long nRef;		
	} 
	METADATA, *PMETADATA;

	class MetaData : public Blob<METADATA>
	{
	private:
		DWORD _dwType;

	public:

		MetaData(DWORD dwType = 0) : _dwType(dwType)
		{
		}

		MetaData(DWORD dwType, LPCVOID p, DWORD dwSize) : _dwType(dwType)
		{
			Alloc(dwSize);
			IW::MemCopy(GetData(), p, dwSize);
		}

		MetaData(DWORD dwType, IW::IStreamIn *pStreamIn) : _dwType(dwType)
		{
			LoadFromStream(pStreamIn);
		}	

		MetaData(const MetaData &other) : _dwType(other._dwType)
		{
			Copy(other._pBlob);
		}

		MetaData &operator=(const MetaData &other)
		{
			Copy(other._pBlob);
			_dwType = other._dwType;
			return *this;
		}

		~MetaData()
		{
			Copy(static_cast<METADATA*>(NULL));
		}

		MetaData Clone() const
		{
			MetaData md(_dwType);
			md.Copy(*this);
			return md;
		}

		void SetType(DWORD dwType) 
		{ 
			_dwType = dwType;
		};

		bool IsImageData() const
		{
			return _dwType == MetaDataTypes::JPEG_IMAGE ||
				_dwType == MetaDataTypes::GIF_IMAGE ||
				_dwType == MetaDataTypes::PNG_IMAGE;
		}

		DWORD GetType() const { return _dwType; };
		bool IsType(const DWORD dw) const { return _dwType == dw; };

		template<class TArchive>
		void Serialize(TArchive &archive)
		{
			archive.Blob(IW::Serialize::NamedType::MetaData, *this);
			archive(IW::Serialize::NamedType::Type, _dwType);		
		}
	};

	struct PageFlags
	{
		enum 
		{
			HasBackGround = 0x02,
			HasTransparent = 0x04,
			HasTimeDelay = 0x08,

			DisposeToBackground  = 0x100,
			DisposeToPrevious  = 0x200
		};
	};

	struct ImageFlags
	{
		enum 
		{
			Animate = 0x01,
			Loop    = 0x02,
			HasError = 0x04,
			HasIPTC = 0x08,
			HasXmp = 0x010,
			HasExif = 0x020,
			HasICC = 0x040,
		};
	};

	class PixelFormat
	{
	public:
		typedef enum 
		{
			PF1,
			PF2,
			PF4,
			PF8,
			PF8Alpha,
			PF8GrayScale,
			PF555,
			PF565,
			PF24,
			PF32,
			PF32Alpha

		} Format;

		Format _pf;

		PixelFormat() : _pf(PF24)
		{
		}

		PixelFormat(Format pf) : _pf(pf)
		{
		}

		PixelFormat(const PixelFormat &other) : _pf(other._pf)
		{
		}	

		void operator=(Format pf)
		{
			_pf = pf;
		}

		void operator=(const PixelFormat &other)
		{
			_pf = other._pf;
		}

		bool operator==(const PixelFormat &other) const
		{
			return _pf == other._pf;
		}

		bool operator!=(const PixelFormat &other) const
		{
			return _pf != other._pf;
		}

		static PixelFormat FromBpp(int nBpp);
		int ToBpp() const;
		int ToStorageBpp() const;
		LPCTSTR ToString() const;
		bool HasAlpha() const;
		bool HasPalette() const;
		int NumberOfPaletteEntries() const;
	};

	struct Orientation
	{
		enum 
		{
			TopLeft = 1,
			TopRight, 
			BottomRight,
			BottomLeft,
			LeftTop,
			RightTop,
			RightBottom,
			LeftBottom
		};
	};

	struct Rotation
	{
		typedef enum 
		{
			None = 0,
			Left = 1,
			Right = 2
		} Direction;
	};

	struct CameraSettings
	{
		CameraSettings()
		{
			XPelsPerMeter = 2834;
			YPelsPerMeter = 2834;
			OriginalImageSize.cx = 0;
			OriginalImageSize.cy = 0;
			TimeTaken = 0;
			IsoSpeed = 0;
			WhiteBalance = 0;
			Orientation = Orientation::TopLeft;
			FocalLength35mmEquivalent = 0;
		}

		DWORD XPelsPerMeter;
		DWORD YPelsPerMeter;
		CSize OriginalImageSize;
		PixelFormat OriginalBpp;
		DWORD TimeTaken;
		IW::Rational Aperture;
		DWORD IsoSpeed;
		DWORD WhiteBalance;
		IW::Rational ExposureTime;
		IW::Rational FocalLength;
		DWORD FocalLength35mmEquivalent;
		DWORD Orientation;
		IW::FileTime DateTaken;

		bool operator!=(const CameraSettings &other) const
		{
			return !(*this == other);
		}

		bool operator==(const CameraSettings &other) const
		{
			return XPelsPerMeter == other.XPelsPerMeter &&
				YPelsPerMeter == other.YPelsPerMeter &&
				OriginalBpp == other.OriginalBpp &&
				OriginalImageSize.cx == other.OriginalImageSize.cx &&
				OriginalImageSize.cy == other.OriginalImageSize.cy &&
				TimeTaken == other.TimeTaken &&
				Aperture == other.Aperture &&
				IsoSpeed == other.IsoSpeed &&
				WhiteBalance == other.WhiteBalance &&
				ExposureTime == other.ExposureTime &&
				FocalLength == other.FocalLength &&
				Orientation == other.Orientation &&
				DateTaken == other.DateTaken &&
				FocalLength35mmEquivalent == other.FocalLength35mmEquivalent;
		}

		template<class TArchive>
		void Serialize(TArchive &archive)
		{			
			archive(IW::Serialize::NamedType::XPelsPerMeter, XPelsPerMeter);
			archive(IW::Serialize::NamedType::YPelsPerMeter, YPelsPerMeter);
			archive(IW::Serialize::NamedType::Orientation, Orientation);
			archive(IW::Serialize::NamedType::TimeTaken, TimeTaken);			
			archive(IW::Serialize::NamedType::IsoSpeed, IsoSpeed);
			archive(IW::Serialize::NamedType::WhiteBalance, WhiteBalance);			
			archive(IW::Serialize::NamedType::OriginalImageX, OriginalImageSize.cx);
			archive(IW::Serialize::NamedType::OriginalImageY, OriginalImageSize.cy);
			archive(IW::Serialize::NamedType::OriginalBpp, (int&)OriginalBpp._pf);	

			archive(IW::Serialize::NamedType::ApertureN, Aperture.numerator);
			archive(IW::Serialize::NamedType::ApertureD, Aperture.denominator);
			archive(IW::Serialize::NamedType::ExposureTimeN, ExposureTime.numerator);
			archive(IW::Serialize::NamedType::ExposureTimeD, ExposureTime.denominator);
			archive(IW::Serialize::NamedType::ExposureTime35mm, FocalLength35mmEquivalent);
			archive(IW::Serialize::NamedType::FocalLengthN, FocalLength.numerator);
			archive(IW::Serialize::NamedType::FocalLengthD, FocalLength.denominator);
			archive(IW::Serialize::NamedType::DateTaken, DateTaken);
		}

		CString FormatAperture() const;
		CString FormatIsoSpeed() const;
		CString FormatWhiteBalance() const;
		CString FormatExposureTime() const;
		CString FormatFocalLength() const;
	};

	IWINTERFACECLASS IImageSurfaceLock : public Referenced
	{
		virtual UINT GetPixel(const int x, const int y) const = 0;
		virtual void SetPixel(const int x, const int y, const UINT &c) = 0;
		virtual void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const = 0;
		virtual void SetLine(LPCCOLORREF pLineSrc, const int y, const int x, const int cx) = 0;
		virtual void RenderLine(COLORREF *pLineDst, const int y, const int x, const int cx) const = 0;
		virtual CRect GetClipRect() const = 0;
	};

	IWINTERFACECLASS IImageMetaDataClient
	{
		virtual bool AddMetaDataType(DWORD dwId, const CString &strTitle) = 0;
	};

	typedef RefPtr<IImageSurfaceLock> IImageSurfaceLockPtr;
	typedef ConstRefPtr<IImageSurfaceLock> ConstIImageSurfaceLockPtr;

	////////////////////////////////////////////////////////////
	//
	// IImageStream
	//
	// Represents an image loader object.
	//
	IWINTERFACECLASS IImageStream
	{
		virtual void CreatePage(const CRect &rc, const PixelFormat &pf, bool bIsSequential) = 0;
		virtual void SetBitmap(int y, LPCVOID pBits) = 0;
		virtual void SetPalette(LPCCOLORREF pPalette) = 0;
		virtual void Flush() = 0;

		// Attributes
		virtual CSize GetThumbnailSize() = 0;
		virtual bool WantThumbnail() = 0;
		virtual bool WantBitmap() = 0;
		virtual bool WantRawImage() = 0;

		// Image Attributes
		virtual void SetPageFlags(const DWORD dw) = 0;
		virtual void SetImageFlags(const DWORD dw) = 0;
		virtual void SetTimeDelay(DWORD dwTimeDelay) = 0;
		virtual void SetDisposalMethod(DWORD dwTimeDelay) = 0;
		virtual void SetBackGround(DWORD dwTimeDelay) = 0;
		virtual void SetTransparent(DWORD dwBackGround) = 0;

		// Meta Data Attributes
		virtual void SetStatistics(const CString &str) = 0;
		virtual void SetLoaderName(const CString &str) = 0;
		virtual void SetCameraSettings(const CameraSettings &settings) = 0;

		// Blobs
		virtual void AddImageData(IW::IStreamIn *pStreamIn, DWORD dwType) = 0;
		virtual void AddBlob(MetaData&) = 0;
		virtual void AddMetaDataBlob(IW::MetaData &iptc, IW::MetaData &xmp) = 0;
	};

	IWINTERFACECLASS IImageLoader : public ISerialize, public Referenced
	{
		// Serialization
		virtual void Read(const IPropertyArchive *pArchive) = 0;
		virtual void Write(IPropertyArchive *pArchive) const = 0;

		// Settings
		virtual CString GetTitleWithSettings() const = 0;

		virtual bool DisplaySettingsDialog(const Image &image) = 0;

		virtual bool Read(const CString &strType, IW::IStreamIn *pStreamIn, IImageStream *pImageOut, IStatus *pStatus) = 0;
		virtual bool Write(const CString &strType, IW::IStreamOut *pStreamOut, const Image &imageIn, IStatus *pStatus) = 0;
	};


	////////////////////////////////////////////////////////////
	//
	// IImageLoaderFactory
	//
	// This struct is normally implemented by a 
	// client app or plug-in.
	//
	IWINTERFACECLASS IImageLoaderFactory
	{
		// Settings
		virtual CString GetKey() const = 0;
		virtual CString GetTitle() const = 0;
		virtual CString GetSection() const = 0;
		virtual CString GetDescription() const = 0;
		virtual CString GetMimeType() const = 0;
		virtual DWORD GetFlags() const = 0;
		virtual DWORD GetIcon() const = 0;
		

		// Create the plugin object
		virtual IImageLoader * CreatePlugin() const = 0;

		// Extension attributes
		virtual CString GetExtensionDefault() const = 0;
		virtual CString GetExtensionList() const = 0;

	};

	typedef IImageLoaderFactory* IImageLoaderFactoryPtr; 

	// ImageLoader Flags
	struct ImageLoaderFlags
	{
		enum 
		{
			MULTIPAGE		= 1 << 0x01,
			METADATA		= 1 << 0x02,
			EXIF			= 1 << 0x03,
			ICC				= 1 << 0x04,
			ALPHA			= 1 << 0x05,
			SAVE			= 1 << 0x07,
			TRANSPARENCY	= 1 << 0x08,
			OPTIONS   		= 1 << 0x09,
			THUMBONLY		= 1 << 0x0A,
			HTML			= 1 << 0x0B,
			MEDIA			= 1 << 0x0C
		};
	};

	/////////////////////////////////////////////////////////////////////////
	//
	// Fixed point math helpers
	//

	inline int IntToFixed(int x)
	{
		return ((x) << 16);
	}

	inline int DoubleToFixed(double x)
	{
		return ((int)(x * 65536.0 + 0.5));
	}
	inline int FixedToInt(int x)         
	{
		return ((x) >> 16);
	}

	inline double FixedToDouble(int x)
	{
		return (((double)x) / 65536.0);
	}

	inline int RoundFixedToInt(int x)
	{
		return (((x) + 0x8000) >> 16);
	}

	enum
	{
		FIXED_ONE = 65536,
		FIXED_PI = 205887L,
		FIXED_2PI = 411775L,
		FIXED_E = 178144L,
		FIXED_ROOT2 = 74804L,
		FIXED_ROOT3 = 113512L,
		FIXED_GOLDEN = 106039L
	};

	inline int FixedMul(int x, int y)
	{
		int z;

		_asm
		{
			mov edx, x
				mov eax, y
				imul edx
				add eax, 8000h
				adc edx, 0
				shrd eax, edx, 16
				mov z, eax
		}

		return z;
	}

	inline int FixedDiv(int x, int y)
	{
		int z;

		_asm
		{
			mov edx, x
				mov ebx, y
				xor eax, eax
				shrd eax, edx, 16
				sar edx, 16
				idiv ebx
				mov z, eax
		}

		return z;
	}


	////////////////////////////////////////////////////////////////////
	// Images

	enum
	{
		IMAGE_X = 100,
		IMAGE_Y = 100
	};

	// Some inline helpers for image work

	inline BYTE ByteClamp(const int n)	
	{
		return (n > 255) ? 255 : (n < 0) ? 0 : static_cast<BYTE>(n);
	}

	inline int Min(const int x, const int y)	
	{
		return x < y ? x : y;
	}

	inline int Max(const int x, const int y)	
	{
		return x > y ? x : y;
	}

	template<int TLimit>
	inline int LowerLimit(const int x)	
	{
		return x < TLimit ? TLimit : x;
	}

	template<int TLimit>
	inline int UpperLimit(const int x)	
	{
		return x > TLimit ? TLimit : x;
	}
	

	// round up
	inline int iceil(int i, int d)
	{
		int nRet = i / d;

		if (i % d)
			return nRet + 1;

		return nRet;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Pixels Conversions

	void IndexesToRGBA(LPCOLORREF pBitsOut, const int nSize, IW::LPCCOLORREF pPalette);
	void SumLineTo32(LPBYTE pLine, const RGBSUM *ps, int cx);
	void SumLineTo16(LPBYTE pLine, const RGBSUM *ps, int cx);
	void ToSums(LPRGBSUM pSum, LPDWORD pLineIn, int cxOut, int cxIn, bool isHighQuality, bool bFirstY);

	void Convert1to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert2to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert4to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert8to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert8Alphato32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);

	void Convert1to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize, LPCCOLORREF pPalette);
	void Convert2to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize, LPCCOLORREF pPalette);
	void Convert4to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize, LPCCOLORREF pPalette);
	void Convert8to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize, LPCCOLORREF pPalette);
	void Convert8Alphato32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize, LPCCOLORREF pPalette);
	void Convert8GrayScaleto32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert555to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert565to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert24to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert32to32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);
	void Convert32Alphato32(LPCOLORREF pLineDst, LPCBYTE pLineSrc, const int nStart, const int nSize);

	// Color conversions
	void ConvertARGBtoABGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertRGBtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertYCbCrtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertCMYKtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertYCCKtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertCIELABtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertICCLABtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertITULABtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertLOGLtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);
	void ConvertLOGLUVtoBGR(LPBYTE pLineDst, LPCBYTE pLineSrc, const int nSize);

	// Color conversions for pixels
	void RGBtoHSL(int R, int G, int B, int &H, int &S, int &L);
	void HSLtoRGB(int H, int S, int L, int &r, int &g, int &b);
	void RGBtoLAB(int R, int G, int B, int &L, int &a, int &b);
	void LABtoRGB(int L, int a, int b, int &R, int &G, int &B);

	// Pixel format helpers
	inline COLORREF RGBA(const int r, const int g, const int b, const int a = 255)
	{ 
		return static_cast<COLORREF>(static_cast<BYTE>(r)) |
			((static_cast<COLORREF>(static_cast<BYTE>(g)))<<8)|
			((static_cast<COLORREF>(static_cast<BYTE>(b)))<<16)|
			((static_cast<COLORREF>(static_cast<BYTE>(a)))<<24); 
	};


	inline COLORREF SaturateRGBA(const DWORD r, const DWORD g, const DWORD b, const DWORD a)
	{ 
		return static_cast<COLORREF>(ByteClamp(r)) |
			((static_cast<COLORREF>(ByteClamp(g)))<<8)|
			((static_cast<COLORREF>(ByteClamp(b)))<<16)|
			((static_cast<COLORREF>(ByteClamp(a)))<<24); 
	};

	inline COLORREF RGB555(const COLORREF r, const COLORREF g, const COLORREF b) { return ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)); };
	inline COLORREF RGB565(const COLORREF r, const COLORREF g, const COLORREF b) { return  ((r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11)); };

	inline COLORREF RGB888to555(const COLORREF c) { return (((c&0x000000f8)>>3)|((c&0x0000f800)>>6)|((c&0x00f80000)>>9)); };
	inline COLORREF RGB555to888(const COLORREF c) { return (((c&0x0000001F)<<3)|((c&0x000003E0)<<6)|((c&0x00007C00)<<9)); };

	inline COLORREF RGB888to565(const COLORREF c) { return (((c&0x000000f8)>>3)|((c&0x0000fC00)>>5)|((c&0x00f80000)>>8)); };
	inline COLORREF RGB565to888(const COLORREF c) { return (((c&0x0000001F)<<3)|((c&0x000007E0)<<5)|((c&0x0000f800)<<8)); };

	inline int GetA(const COLORREF c) { return (BYTE)(c >> 24); };
	inline int GetR(const COLORREF c) { return ((BYTE)(c)); };
	inline int GetG(const COLORREF c) { return ((BYTE)(((WORD)(c)) >> 8)); };
	inline int GetB(const COLORREF c) { return ((BYTE)((c)>>16)); };

	inline COLORREF Lighten(const COLORREF  c, const int n = 32)
	{
		return SaturateRGBA(GetR(c) + n, GetG(c) + n, GetB(c) + n, GetA(c));
	}

	inline COLORREF Darken(const COLORREF  c, const int n = 32)
	{
		return Lighten(c, -n);
	}

	inline COLORREF Emphasize(const COLORREF  c, const int n = 32)
	{
		const bool isLight  = (GetB(c) + GetG(c) + GetR(c)) > (0x80 * 3);

		return Lighten(c, isLight ? -n : n);
	}

	inline COLORREF SwapRB(const COLORREF c) 
	{
		return RGBA(GetB(c), GetG(c), GetR(c), GetA(c));
	}
	inline COLORREF Average(const COLORREF  c1, const COLORREF  c2)
	{
		return RGBA(
			(GetR(c1) + GetR(c2)) / 2, 
			(GetG(c1) + GetG(c2)) / 2, 
			(GetB(c1) + GetB(c2)) / 2, 
			(GetA(c1) + GetA(c2)) / 2);
	}

	inline int CalcStorageWidth(int nWidth, const PixelFormat &pf)
	{
		// Fix for PF555
		const int nBpp = pf.ToStorageBpp();
		const int nBppWidth = nWidth * nBpp;
		return ((nBppWidth + ((nBppWidth % 8) ? 8 : 0))/8+3) & ~3;
	}

	//inline double DegreesToRadians(double d) { return (M_PI*d/180.0); };

	// Logical Conversions
	inline int InchToMeter(DWORD dw) { return (dw * 3937) / 100; };
	inline int MeterToInch(DWORD dw) { return MulDiv(dw, 100, 3937); };
	inline int InchToMeter(int n) { return (n * 3937) / 100; };
	inline int MeterToInch(int n) { return MulDiv(n, 100, 3937); };
	inline int InchToMeter(double f) { return static_cast<int>((f * 3937.0) / 100.0); };
	inline int MeterToInch(double f) { return MulDiv(static_cast<int>(f), 100, 3937); };

	inline int CMToMeter(DWORD dw) { return dw * 100; };
	inline int MeterToCM(DWORD dw) { return MulDiv(dw, 100, 100 * 100); };
	inline int CMToMeter(int n) { return n * 100; };
	inline int MeterToCM(int n) { return MulDiv(n, 100, 100 * 100); };
	inline int CMToMeter(double f) { return static_cast<int>(f * 100.0); };
	inline int MeterToCM(double f) { return MulDiv(static_cast<int>(f), 100, 100 * 100); };


	/////////////////////////////////////////////////////////////////////////////////////////////////
	/// Histogram

	class Histogram
	{
	public:

		Histogram()
		{
			Clear();
		}

		enum { MaxValue = 0x100 };
		
		void Clear()
		{
			_max = 0;
			IW::MemZero(_r, sizeof(int) * MaxValue);
			IW::MemZero(_g, sizeof(int) * MaxValue);
			IW::MemZero(_b, sizeof(int) * MaxValue);
		}

		void AddColor(COLORREF c)
		{
			_r[IW::GetR(c)]++;
			_g[IW::GetG(c)]++;
			_b[IW::GetB(c)]++;
		}

		void CalcMax()
		{
			int nMax = 0;

			for (int i=0; i < MaxValue; i++)
			{
				nMax = IW::Max(IW::Max(nMax, _r[i]), IW::Max(_g[i], _b[i]));
			}

			_max = nMax;
		}

		int _r[MaxValue];
		int _g[MaxValue];
		int _b[MaxValue];
		int _max;
	};


	/////////////////////////////////////////////////////////////////////////////////////////////////
	/// Images
	typedef struct tagIWBITMAPINFO 
	{
		DWORD dwSize;
		volatile long nRef;
		IAtlMemMgr *pMemMgr;

		// Attributes specific to ImageWalker
		DWORD dwFlags;
		DWORD dwTimeDelay;
		DWORD dwBackGround;
		DWORD dwTransparent;
		CRect rectPage;
		PixelFormat pf;
	} 
	IWBITMAPINFO, *PIWBITMAPINFO;

	typedef const IWBITMAPINFO *PCIWBITMAPINFO;

	class Page : public Blob<IWBITMAPINFO>
	{
	public:

		inline Page() { }
		inline Page(PIWBITMAPINFO pPage) { Copy(pPage); }
		inline Page(const Page &other) { Copy(other); }

		inline Page& operator=(const Page &other)
		{
			Copy(other._pBlob);
			return *this;
		}

		Page Clone() const
		{
			Page page;
			page.Copy(*this);
			return page;
		}

		template<class TArchive>
		void Serialize(TArchive &archive)
		{
			archive.Blob(IW::Serialize::NamedType::ImageData, *this);

			archive(IW::Serialize::NamedType::Flags, _pBlob->dwFlags);
			archive(IW::Serialize::NamedType::TimeDelay, _pBlob->dwTimeDelay);
			archive(IW::Serialize::NamedType::BackGround, _pBlob->dwBackGround);
			archive(IW::Serialize::NamedType::Transparent, _pBlob->dwTransparent);
			archive(IW::Serialize::NamedType::PageLeft, _pBlob->rectPage.left);
			archive(IW::Serialize::NamedType::PageTop, _pBlob->rectPage.top);
			archive(IW::Serialize::NamedType::PageRight, _pBlob->rectPage.right);
			archive(IW::Serialize::NamedType::PageBottom, _pBlob->rectPage.bottom);
			archive(IW::Serialize::NamedType::Format, (int&)_pBlob->pf._pf);			
		}

		inline bool IsEnd() const
		{
			return _pBlob == 0;
		}

		inline PIWBITMAPINFO GetInfo()
		{
			return _pBlob;
		}

		inline PCIWBITMAPINFO GetInfo() const
		{
			return _pBlob;
		}

		inline LPCOLORREF GetPalette()
		{
			return (LPCOLORREF)(((LPBYTE)(GetInfo())) + sizeof(IWBITMAPINFO));
		}

		inline LPCCOLORREF GetPalette() const
		{
			return (LPCCOLORREF)(((LPBYTE)(GetInfo())) + sizeof(IWBITMAPINFO));
		}

		inline PixelFormat GetPixelFormat() const 
		{ 
			return GetInfo()->pf; 
		};

		inline DWORD GetFlags() const
		{
			return _pBlob->dwFlags;
		}			

		inline void SetFlags(const DWORD dw)
		{
			_pBlob->dwFlags = dw;
		}

		inline DWORD GetTimeDelay() const
		{
			return _pBlob->dwTimeDelay;
		}

		inline void SetTimeDelay(DWORD dwTimeDelay)
		{
			_pBlob->dwTimeDelay = dwTimeDelay;
			_pBlob->dwFlags |= PageFlags::HasTimeDelay;
		}

		inline DWORD GetBackGround() const
		{
			return _pBlob->dwBackGround;
		}

		inline void SetBackGround(DWORD dwBackGround)
		{
			_pBlob->dwBackGround = dwBackGround;
			_pBlob->dwFlags |= PageFlags::HasTimeDelay;
		}

		inline DWORD GetTransparent() const
		{
			return _pBlob->dwTransparent;
		}

		inline void SetTransparent(DWORD dwTransparent)
		{
			_pBlob->dwTransparent = dwTransparent;
			_pBlob->dwFlags |= PageFlags::HasTransparent;
		}

		int GetDisposalMethod() const
		{
			return (_pBlob->dwFlags >> 8) & 0x7;
		}

		void SetDisposalMethod(const int nDisposalMethod) const
		{
			_pBlob->dwFlags = (_pBlob->dwFlags & 0x700) | (nDisposalMethod << 8);
		}

		CRect GetClipRect() const
		{
			return GetPageRect();
		}

		inline const CRect &GetPageRect() const
		{
			return _pBlob->rectPage;
		}

		void SetPageOffset(int x, int y)
		{
			CRect rc = _pBlob->rectPage;
			int cx = rc.right - rc.left;
			int cy = rc.bottom - rc.top;
			
			_pBlob->rectPage = CRect(x, y, x + cx, y + cy);
		}

		inline void SetPageRect(const CRect &rc)
		{
			_pBlob->rectPage = rc;
		}

		inline int GetWidth() const
		{
			return _pBlob->rectPage.right - _pBlob->rectPage.left;
		}

		inline int GetHeight() const
		{
			return _pBlob->rectPage.bottom - _pBlob->rectPage.top;
		}

		inline int GetStorageWidth() const
		{
			return CalcStorageWidth(GetWidth(), _pBlob->pf);
		}

		inline int GetStride() const
		{
			return -CalcStorageWidth(GetWidth(), _pBlob->pf);
		}

		inline CSize GetSize() const
		{ 
			return _pBlob->rectPage.Size();
		}

		inline CSize GetHalfSize() const
		{
			return CSize(_pBlob->rectPage.Width() / 2, _pBlob->rectPage.Height() / 2);   
		}

		inline CPoint GetOffset() const
		{
			return _pBlob->rectPage.TopLeft();   
		}

		inline LPBYTE GetBitmap()
		{
			return ((LPBYTE)(_pBlob)) + sizeof(IWBITMAPINFO) + (_pBlob->pf.NumberOfPaletteEntries() * sizeof(RGBQUAD));
		}

		inline LPCBYTE GetBitmap() const
		{
			return ((LPCBYTE)(_pBlob)) + sizeof(IWBITMAPINFO) + (_pBlob->pf.NumberOfPaletteEntries() * sizeof(RGBQUAD));
		}

		inline LPBYTE GetBitmapLine(int nLine)
		{
			const int nStorageWidth = GetStorageWidth();
			const int nHeight = GetHeight();
			const int nOffset = nStorageWidth * (nHeight - (nLine + 1));

			return GetBitmap() + nOffset;
		};

		inline LPCBYTE GetBitmapLine(int nLine) const
		{
			const int nStorageWidth = GetStorageWidth();
			const int nHeight = GetHeight();
			const int nOffset = nStorageWidth * (nHeight - (nLine + 1));

			return GetBitmap() + nOffset;
		}

		IImageSurfaceLock* GetSurfaceLock();
		const IImageSurfaceLock* GetSurfaceLock() const;
		bool CompareBits(const Page &other) const;
		bool CopyExtraInfo(const Page &other);

		template<typename InputIterator, typename Predicate, typename UnaryFunction>
			UnaryFunction ForEachLine(InputIterator first, InputIterator last, Predicate pred, UnaryFunction f)
		{
			for(;first != last; ++first)
			{
				if (pred(*first))
				{
					f(*first);
				}
			}
			return f;
		}
	};

	

	class Image
	{
	public:

		typedef std::vector<Page> PageList;
		PageList Pages;

		typedef std::vector<MetaData> BLOBLIST;
		BLOBLIST Blobs;

	protected:		

		CString _strTitle;
		CString _strTags;
		CString _strDescription;
		CString _strFlickrId;
		CString _strObjectName;
		CString _strStatistics;
		CString _strLoaderName;
		CString _strErrors;
		CString _strWarnings;
		DWORD _dwFlags;
		CameraSettings	_settings;	

	public:	

		Image()
		{
			_dwFlags = 0;			
		}

		Image(const Image &other) : _dwFlags(0)
		{
			Copy(other);
		}

		~Image()
		{	
			Free();
		}

		Page &CreatePage(int cx, int cy, const PixelFormat &pf);
		Page &CreatePage(const IW::Page &pageIn);
		Page &CreatePage(const CRect &rc, const PixelFormat &pf);

		Image& operator =(const Image &other)
		{
			Copy(other);
			return *this;
		}		

		void Copy(const Image &other);
		Image Clone() const;

		void Free()
		{
			Pages.clear();
			Blobs.clear();
		}

		void ClearMetaData()
		{
			Blobs.clear();
		}

		DWORD GetPageCount() const
		{
			return Pages.size();
		}

		bool NeedRenderForDisplay() const
		{
			return CanAnimate();
		}

		bool CanAnimate() const
		{
			// If more than one page
			return _dwFlags & ImageFlags::Animate &&
				Pages.size() > 1;
		}

		bool IsEmpty() const
		{
			return Pages.empty();
		}

		Page GetPage(int nPage) const
		{
			return Pages[nPage];
		}

		Page GetFirstPage() const
		{
			return GetPage(0);
		};

		bool Compare(const Image &image, bool bCompareMetaData = true) const;
		bool Copy(const BITMAPINFO *pInfo, LPCBYTE pBytes, int nBytesMax = -1);
		bool Copy(const BITMAPINFOHEADER *pHeader, int nBytesMax = -1);
		bool Copy(HDC hdc, HBITMAP hbm);
		bool Copy(HGLOBAL hglb);
		HGLOBAL CopyToHandle()  const;

		void FixOrientation(IStatus *pStatus);
		void Normalize();		

		// Compare
		inline bool operator==(const Image &image) const {	return Compare(image); };
		inline bool operator!=(const Image &image) const { return !Compare(image); };	

		// Render
		bool Render(Image &imageOut) const;

		void GetHistogram(Histogram &histogram) const;

		// Various Attributes
		DWORD GetFlags() const { return _dwFlags; };
		void SetFlags(DWORD dw) { _dwFlags = dw; };

		const CameraSettings &GetCameraSettings() const { return _settings; };
		void SetCameraSettings(const CameraSettings &settings) { _settings = settings; };

		DWORD GetXPelsPerMeter() const
		{
			return _settings.XPelsPerMeter;
		}

		void SetXPelsPerMeter(DWORD dwPelsPerMeter)
		{
			_settings.XPelsPerMeter = dwPelsPerMeter;
		}

		DWORD GetYPelsPerMeter() const
		{
			return _settings.YPelsPerMeter;
		}

		void SetYPelsPerMeter(DWORD dwPelsPerMeter)
		{
			_settings.YPelsPerMeter = dwPelsPerMeter;
		}		

		CRect GetBoundingRect() const
		{
			CRect rc(0, 0, 0, 0);

			PageList::const_iterator i = Pages.begin();

			if (i != Pages.end())
			{				
				rc = i->GetPageRect();				

				for(++i; i != Pages.end(); ++i)
				{
					UnionRect(&rc, &rc, &(i->GetPageRect()));
				}
			}

			return rc;
		}

		
		void SetMetaData(const MetaData &blob)
		{
			for(BLOBLIST::iterator i = Blobs.begin(); i != Blobs.end(); ++i)
			{
				if (i->IsType(blob.GetType()))
				{
					*i = blob;
					return;
				}
			}

			Blobs.push_back(blob);
		}

		template<class T>
		inline void IterateMetaData(T *pFunctor) const
		{
			for(BLOBLIST::const_iterator i = Blobs.begin(); i != Blobs.end(); ++i)
			{
				pFunctor->AddMetaDataBlob(*i);
			}
		}

		inline bool HasMetaData(const DWORD &dwType) const
		{
			for(BLOBLIST::const_iterator i = Blobs.begin(); i != Blobs.end(); ++i)
			{
				if (i->IsType(dwType))
					return true;
			}

			return false;
		}
		
		inline const MetaData GetMetaData(const DWORD &dwType) const
		{
			for(BLOBLIST::const_iterator i = Blobs.begin(); i != Blobs.end(); ++i)
			{
				if (i->IsType(dwType))
					return (*i);
			}

			return MetaData(dwType);
		}		

		CString GetTitle() const { return _strTitle; };
		void SetTitle(const CString &str) { _strTitle = str; };

		CString GetTags() const { return _strTags; };
		void SetTags(const CString &str) { _strTags = str; };

		CString GetDescription() const { return _strDescription; };
		void SetDescription(const CString &str) { _strDescription = str; };

		CString GetFlickrId() const { return _strFlickrId; };
		void SetFlickrId(const CString &str) { _strFlickrId = str; };
		
		CString GetObjectName() const { return _strObjectName; };
		void SetObjectName(const CString &str) { _strObjectName = str; };

		CString GetStatistics() const { return _strStatistics; };
		void SetStatistics(const CString &str) { _strStatistics = str; };

		CString GetLoaderName() const { return _strLoaderName; };
		void SetLoaderName(const CString &str) { _strLoaderName = str; };

		CString GetWarnings() const { return _strWarnings; };
		CString GetErrors() const { return _strErrors; };

		void AddImageData(IW::IStreamIn *pStreamIn, DWORD dwType);	
		void ConvertTo(PixelFormat pf);

		template<class TArchive>
		void Serialize(TArchive &archive)
		{			
			archive(IW::Serialize::NamedType::Title, _strTitle);
			archive(IW::Serialize::NamedType::Tags, _strTags);
			archive(IW::Serialize::NamedType::Description, _strDescription);
			archive(IW::Serialize::NamedType::FlickrId, _strFlickrId);
			archive(IW::Serialize::NamedType::ObjectName, _strObjectName);
			archive(IW::Serialize::NamedType::Statistics, _strStatistics);
			archive(IW::Serialize::NamedType::LoaderName, _strLoaderName);
			archive(IW::Serialize::NamedType::Errors, _strErrors);
			archive(IW::Serialize::NamedType::Warnings, _strWarnings);
			archive(IW::Serialize::NamedType::Flags, _dwFlags);

			_settings.Serialize(archive);

			archive.BlobList(IW::Serialize::NamedType::Pages, Pages);
			archive.BlobList(IW::Serialize::NamedType::MetaData, Blobs);
		}

		CString ToString() const;
		static Image LoadPreviewImage(PluginState &plugins);
	};


	class CImageBitmapInfoHeader
	{
	private:

		typedef struct 
		{
			BITMAPINFOHEADER    bmiHeader;
			COLORREF            bmiColors[256];
		} 
		BITMAPINFO;

		BITMAPINFO m_info;

	public:
		CImageBitmapInfoHeader(const Page &page);

		operator LPBITMAPINFO()
		{
			return (LPBITMAPINFO)&m_info;
		}		
	};


	// Manipulation
	bool Crop(const Image &imageIn, Image &imageOut, const CRect &r, IStatus *pStatus);
	bool Dither(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool Filter(const Image &imageIn, Image &imageOut, long* kernel, long Ksize, long Kfactor, long Koffset, IStatus *pStatus);
	bool Frame(const Image &imageIn, Image &imageOut, COLORREF clrFrame, IStatus *pStatus);
	bool GrayScale(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool MirrorLR(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool MirrorTB(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool Quantize(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool Rotate(const Image &imageIn, Image &imageOut, float fAngle, IStatus *pStatus);
	bool Rotate180(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool Rotate270(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool Rotate90(const Image &imageIn, Image &imageOut, IStatus *pStatus);
	bool Scale(const Image &imageIn, IW::Image &imageOut, CSize size, IStatus *pStatus); 
	bool Scale(const CSize &size, int nFilterType, const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus);

	Image CreatePreview(const Image &imageIn, CSize sizeIn);



}; // namespace IW
