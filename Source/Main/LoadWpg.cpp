#include "stdafx.h"
#include "LoadWpg.h"

CLoadWpg::CLoadWpg()
{
}

CLoadWpg::~CLoadWpg()
{
}

#include <pshpack1.h>

typedef struct
{
	unsigned char
		red,
		blue,
		green;
} ColorPalette;

typedef struct
{
	unsigned int FileId;
	unsigned int DataOffset;
	unsigned short ProductType;
	unsigned short FileType;
	unsigned char MajorVersion;
	unsigned char MinorVersion;
	unsigned short EncryptKey;
	unsigned short Reserved;
} WPGHeader;

typedef struct
{
	unsigned char  RecType;
	unsigned long   RecordLength;
} WPGRecord;

typedef struct
{
	unsigned char  Class;
	unsigned char    RecType;
	unsigned long   Extension;
	unsigned long   RecordLength;
} WPG2Record;

typedef struct
{
	unsigned  short HorizontalUnits;
	unsigned  short VerticalUnits;
	unsigned char PosSizePrecision;
} WPG2Start;

typedef struct
{
	unsigned short Width;
	unsigned short Heigth;
	unsigned short Depth;
	unsigned short HorzRes;
	unsigned short VertRes;
} WPGBitmapType1;

typedef struct
{
	unsigned short Width;
	unsigned short Heigth;
	unsigned char Depth;
	unsigned char Compression;
} WPG2BitmapType1;

typedef struct
{
	unsigned short RotAngle;
	unsigned short LowLeftX;
	unsigned short LowLeftY;
	unsigned short UpRightX;
	unsigned short UpRightY;
	unsigned short Width;
	unsigned short Heigth;
	unsigned short Depth;
	unsigned short HorzRes;
	unsigned short VertRes;
} WPGBitmapType2;

typedef struct
{
	unsigned int StartIndex;
	unsigned int NumOfEntries;
} WPGColorMapRec;

typedef struct {
	unsigned long PS_unknown1;
	unsigned int PS_unknown2;
	unsigned int PS_unknown3;
} WPGPSl1Record;

#include <poppack.h>

static bool IsWPG(const unsigned char *magick,const size_t length)
{
	if (length < 4)
		return false;

	if (memcmp(magick,"\377WPC",4) == 0)
		return true;

	return false;
}


static void Rd_WP_DWORD(IW::IStreamIn *pStreamIn, unsigned long *d)
{
	unsigned char b;

	b=ReadBlobByte(pStreamIn);
	*d=b;
	if (b < 0xFF)
		return;
	b=ReadBlobByte(pStreamIn);
	*d=(unsigned long) b;
	b=ReadBlobByte(pStreamIn);
	*d+=(unsigned long) b*256l;
	if (*d < 0x8000)
		return;
	*d=(*d & 0x7FFF) << 16;
	b=ReadBlobByte(pStreamIn);
	*d+=(unsigned long) b;
	b=ReadBlobByte(pStreamIn);
	*d+=(unsigned long) b*256l;
	return;
}


#define InsertByte(b) \
{ \
	BImgBuff.Set(x, b); \
	x++; \
	if((long) x>=ldblk) \
{ \
	if (bpp == 24) IW::ConvertRGBtoBGR(BImgBuff, BImgBuff, width); \
	pImageOut->SetBitmap(y, BImgBuff);\
	x=0; \
	y++; \
} \
}

static int UnpackWPGRaster(IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, int width, int height, int bpp)
{
	long y = 0;
	register long i, x = 0;
	unsigned char bbuf, RunCount; 

	long ldblk=(long) ((bpp*width+7)/8);
	IW::CAutoFree<BYTE> BImgBuff(ldblk);

	while (y < (long) height)
	{
		bbuf=ReadBlobByte(pStreamIn);

		/*
		if not readed byte ??????
		{
		delete Raster;
		Raster=NULL;
		return(-2);
		}
		*/

		RunCount=bbuf & 0x7F;
		if(bbuf & 0x80)
		{
			if(RunCount)  /* repeat next byte runcount * */
			{
				bbuf=ReadBlobByte(pStreamIn);
				for(i=0;i<(int) RunCount;i++) InsertByte(bbuf);
			}
			else {  /* read next byte as RunCount; repeat 0xFF runcount* */
				RunCount=ReadBlobByte(pStreamIn);
				for(i=0;i<(int) RunCount;i++) InsertByte(0xFF);
			}
		}
		else 
		{
			if(RunCount)   /* next runcount byte are readed directly */
			{
				for(i=0;i < (int) RunCount;i++)
				{
					bbuf=ReadBlobByte(pStreamIn);
					InsertByte(bbuf);
				}
			}
			else 
			{  /* repeat previous line runcount* */
				RunCount=ReadBlobByte(pStreamIn);
				if(x) 
				{    /* attempt to duplicate row from x position: */
					/* I do not know what to do here */
					return(-3);
				}
				for(i=0;i < (int) RunCount;i++)
				{
					x=0;
					y++;    /* Here I need to duplicate previous row RUNCOUNT* */
					if(y<2) continue;
					if(y>(long) height)
					{
						return(-4);
					}

					if (bpp == 24) IW::ConvertRGBtoBGR(BImgBuff, BImgBuff, width);
					pImageOut->SetBitmap(y-1, BImgBuff);
				}
			}
		}
	}
	return(0);
}

static int UnpackWPG2Raster(IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, int width, int height, int bpp)
{
	char SampleSize=1;
	register long i, x = 0;
	long y = 0;
	unsigned char bbuf, RunCount, SampleBuffer[8];
	long ldblk=(long) ((bpp*width+7)/8);	
	IW::CAutoFree<BYTE> BImgBuff(ldblk);

	while(y<(long) height)
	{
		bbuf=ReadBlobByte(pStreamIn);

		switch(bbuf)
		{
		case 0x7D:
			SampleSize=ReadBlobByte(pStreamIn);  /* DSZ */
			if (SampleSize > 8)
				return(-2);
			if (SampleSize < 1)
				return(-2);
			break;
		case 0x7E:
			fprintf(stderr,"\nUnsupported WPG token XOR, please report!");
			break;
		case 0x7F:
			RunCount=ReadBlobByte(pStreamIn);   /* BLK */
			for(i=0; i < (long) SampleSize*((long)RunCount+1); i++)
			{
				InsertByte(0);
			}
			break;
		case 0xFD:
			RunCount=ReadBlobByte(pStreamIn);   /* EXT */
			for (i=0; i <= (int) RunCount; i++)
				for (bbuf=0; bbuf < SampleSize; bbuf++)
					InsertByte(SampleBuffer[bbuf]);
			break;
		case 0xFE:
			RunCount=ReadBlobByte(pStreamIn);  /* RST */
			if(x!=0)
			{
				fprintf(stderr,
					"\nUnsupported WPG2 unaligned token RST x=%ld, please report!\n"
					,x);
				return(-3);
			}
			{
				/* duplicate the previous row RunCount x */
				for(i=0;i<=RunCount;i++)
				{
					x=0;
					if (bpp == 24) IW::ConvertRGBtoBGR(BImgBuff, BImgBuff, width);
					pImageOut->SetBitmap(( y< (long) height ? y : height-1), BImgBuff);
					y++;
					if(y<2) continue;
					if(y> (long) height) return(-4);
				}
			}
			break;
		case 0xFF:
			RunCount=ReadBlobByte(pStreamIn);  /* WHT */
			for (i=0; i< (int) SampleSize*((int) RunCount+1); i++)
			{
				InsertByte(0xFF);
			}
			break;
		default:
			RunCount=bbuf & 0x7F;
			if (bbuf & 0x80)  /* REP */
			{
				for(i=0;i<(int) SampleSize;i++)
					SampleBuffer[i]=ReadBlobByte(pStreamIn);
				for(i=0;i<=(int)RunCount;i++)
					for(bbuf=0;bbuf<SampleSize;bbuf++)
						InsertByte(SampleBuffer[bbuf]);
			}
			else {  /* NRP */
				for(i=0;i<(int) SampleSize*((int)RunCount+1);i++)
				{
					bbuf=ReadBlobByte(pStreamIn);
					InsertByte(bbuf);
				}
			}
		}

	}

	return(0);
}

unsigned LoadWPG2Flags(IW::IStreamIn *pStreamIn,char Precision,float *Angle)
{
	const unsigned char TPR=1,TRN=2,SKW=4,SCL=8,ROT=0x10,OID=0x20,LCK=0x80;
	long  x;
	unsigned DenX;
	unsigned Flags;
	float CTM[3][3];        /* current transform matrix (currently ignored) */

	IW::MemZero(&CTM, sizeof(CTM));     /* CTM.erase();CTM.resize(3,3) */
	CTM[0][0]=1;
	CTM[1][1]=1;
	CTM[2][2]=1;

	Flags=ReadBlobLSBShort(pStreamIn);
	if(Flags & LCK) x=ReadBlobLSBLong(pStreamIn);  /* Edit lock */
	if(Flags & OID)
	{
		if(Precision==0)
		{x=ReadBlobLSBShort(pStreamIn);}  /* ObjectID */
		else
		{x=ReadBlobLSBLong(pStreamIn);}  /* ObjectID (Double precision). */
	}
	if(Flags & ROT)
	{
		x=ReadBlobLSBLong(pStreamIn);  /* Rot Angle. */
		if(Angle) *Angle=x/65536.0;
	}
	if(Flags & (ROT|SCL))
	{
		x=ReadBlobLSBLong(pStreamIn); /* Sx*cos(). */
		CTM[0][0]=x;
		x=ReadBlobLSBLong(pStreamIn); /* Sy*cos(). */
		CTM[1][1]=x;
	}
	if(Flags & (ROT|SKW))
	{
		x=ReadBlobLSBLong(pStreamIn);       /* Kx*sin(). */
		CTM[1][0]=x;
		x=ReadBlobLSBLong(pStreamIn);       /* Ky*sin(). */
		CTM[0][1]=x;
	}
	if(Flags & TRN)
	{
		x=ReadBlobLSBLong(pStreamIn); DenX=ReadBlobLSBLong(pStreamIn);  /* Tx */
		CTM[0][2]=(float)x + ((x >= 0)?1:-1)*(float)DenX/0x10000;
		x=ReadBlobLSBLong(pStreamIn); DenX=ReadBlobLSBLong(pStreamIn);  /* Ty */
		CTM[1][2]=(float)x + ((x >= 0)?1:-1)*(float)DenX/0x10000;
	}
	if(Flags & TPR)
	{
		x=ReadBlobLSBLong(pStreamIn);  /* Px. */
		CTM[2][0]=x;
		x=ReadBlobLSBLong(pStreamIn);  /* Py. */
		CTM[2][1]=x;
	}
	return(Flags);
}

/*
static Image *ExtractPostscript(IW::IStreamIn *pStreamIn,const ImageInfo *image_info,
MagickOffsetType PS_Offset,long PS_Size,ExceptionInfo *exception)
{
const MagicInfo
*magic_info;

FILE
*file;

ImageInfo
*clone_info;

int
c,
unique_file;

Image
*image2;

register int
i;

unsigned char
magick[2*MaxTextExtent];

if ((clone_info=CloneImageInfo(image_info)) == NULL)
return(GetFirstImageInList(image));
SetImageInfoBlob(clone_info,(void *) NULL,0);
file=(FILE *) NULL;
unique_file=AcquireUniqueFileResource((char *) clone_info->filename);
if (unique_file != -1)
file=fdopen(unique_file,"wb");
if ((unique_file == -1) || (file == NULL))
goto FINISH;
(void) SeekBlob(image,PS_Offset,SEEK_SET);
for (i=0; i < PS_Size; i++)
{
c=ReadBlobByte(pStreamIn);
if (i <  (long) (2*MaxTextExtent))
magick[i]=c;
(void) fputc(c,file);
}
(void) fclose(file);
magic_info=GetMagicInfo(magick,2*MaxTextExtent,exception);
if ((magic_info == (const MagicInfo *) NULL) ||
(GetMagicName(magic_info) == (char *) NULL) ||
(exception->severity != UndefinedException))
goto FINISH_UNL;
(void) CopyMagickString(clone_info->magick,GetMagicName(magic_info),
MaxTextExtent);
image2=ReadImage(clone_info,exception);
if (image2 == (Image *) NULL)
goto FINISH_UNL;
(void) CopyMagickString(image2->filename,image->filename,MaxTextExtent);
(void) CopyMagickString(image2->magick_filename,image->magick_filename,
MaxTextExtent);
(void) CopyMagickString(image2->magick,image->magick,MaxTextExtent);
image2->depth=image->depth;
DestroyBlob(image2);
image2->blob=ReferenceBlob(image->blob);
if ((height == 0) || (width == 0))
DeleteImageFromList(&image);
AppendImageToList(&image,image2);

FINISH_UNL:
(void) RelinquishUniqueFileResource(clone_info->filename);
FINISH:
clone_info=DestroyImageInfo(clone_info);
return(GetFirstImageInList(image));
}

*/

DWORD TellBlob(IW::IStreamIn *pStreamIn)
{
	return pStreamIn->Seek(IW::IStreamCommon::eCurrent, 0);
}

bool CLoadWpg::Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
{
	try
	{
		static const ColorPalette WPG1Palette[256]=
		{
			{  0,   0,   0}, {  0,   0, 168}, {  0, 168,   0}, {  0, 168, 168},
			{168,   0,   0}, {168,   0, 168}, {168,  84,   0}, {168, 168, 168},
			{ 84,  84,  84}, { 84,  84, 252}, { 84, 252,  84}, { 84, 252, 252},
			{252,  84,  84}, {252,  84, 252}, {252, 252,  84}, {252, 252, 252},
			{  0,   0,   0}, { 20,  20,  20}, { 32,  32,  32}, { 44,  44,  44},
			{ 56,  56,  56}, { 68,  68,  68}, { 80,  80,  80}, { 96,  96,  96},
			{112, 112, 112}, {128, 128, 128}, {144, 144, 144}, {160, 160, 160},
			{180, 180, 180}, {200, 200, 200}, {224, 224, 224}, {252, 252, 252},
			{  0,   0, 252}, { 64,   0, 252}, {124,   0, 252}, {188,   0, 252},
			{252,   0, 252}, {252,   0, 188}, {252,   0, 124}, {252,   0,  64},
			{252,   0,   0}, {252,  64,   0}, {252, 124,   0}, {252, 188,   0},
			{252, 252,   0}, {188, 252,   0}, {124, 252,   0}, { 64, 252,   0},
			{  0, 252,   0}, {  0, 252,  64}, {  0, 252, 124}, {  0, 252, 188},
			{  0, 252, 252}, {  0, 188, 252}, {  0, 124, 252}, {  0,  64, 252},
			{124, 124, 252}, {156, 124, 252}, {188, 124, 252}, {220, 124, 252},
			{252, 124, 252}, {252, 124, 220}, {252, 124, 188}, {252, 124, 156},
			{252, 124, 124}, {252, 156, 124}, {252, 188, 124}, {252, 220, 124},
			{252, 252, 124}, {220, 252, 124}, {188, 252, 124}, {156, 252, 124},
			{124, 252, 124}, {124, 252, 156}, {124, 252, 188}, {124, 252, 220},
			{124, 252, 252}, {124, 220, 252}, {124, 188, 252}, {124, 156, 252},
			{180, 180, 252}, {196, 180, 252}, {216, 180, 252}, {232, 180, 252},
			{252, 180, 252}, {252, 180, 232}, {252, 180, 216}, {252, 180, 196},
			{252, 180, 180}, {252, 196, 180}, {252, 216, 180}, {252, 232, 180},
			{252, 252, 180}, {232, 252, 180}, {216, 252, 180}, {196, 252, 180},
			{180, 220, 180}, {180, 252, 196}, {180, 252, 216}, {180, 252, 232},
			{180, 252, 252}, {180, 232, 252}, {180, 216, 252}, {180, 196, 252},
			{  0,   0, 112}, { 28,   0, 112}, { 56,   0, 112}, { 84,   0, 112},
			{112,   0, 112}, {112,   0,  84}, {112,   0,  56}, {112,   0,  28},
			{112,   0,   0}, {112,  28,   0}, {112,  56,   0}, {112,  84,   0},
			{112, 112,   0}, { 84, 112,   0}, { 56, 112,   0}, { 28, 112,   0},
			{  0, 112,   0}, {  0, 112,  28}, {  0, 112,  56}, {  0, 112,  84},
			{  0, 112, 112}, {  0,  84, 112}, {  0,  56, 112}, {  0,  28, 112},
			{ 56,  56, 112}, { 68,  56, 112}, { 84,  56, 112}, { 96,  56, 112},
			{112,  56, 112}, {112,  56,  96}, {112,  56,  84}, {112,  56,  68},
			{112,  56,  56}, {112,  68,  56}, {112,  84,  56}, {112,  96,  56},
			{112, 112,  56}, { 96, 112,  56}, { 84, 112,  56}, { 68, 112,  56},
			{ 56, 112,  56}, { 56, 112,  69}, { 56, 112,  84}, { 56, 112,  96},
			{ 56, 112, 112}, { 56,  96, 112}, { 56,  84, 112}, { 56,  68, 112},
			{ 80,  80, 112}, { 88,  80, 112}, { 96,  80, 112}, {104,  80, 112},
			{112,  80, 112}, {112,  80, 104}, {112,  80,  96}, {112,  80,  88},
			{112,  80,  80}, {112,  88,  80}, {112,  96,  80}, {112, 104,  80},
			{112, 112,  80}, {104, 112,  80}, { 96, 112,  80}, { 88, 112,  80},
			{ 80, 112,  80}, { 80, 112,  88}, { 80, 112,  96}, { 80, 112, 104},
			{ 80, 112, 112}, { 80, 114, 112}, { 80,  96, 112}, { 80,  88, 112},
			{  0,   0,  64}, { 16,   0,  64}, { 32,   0,  64}, { 48,   0,  64},
			{ 64,   0,  64}, { 64,   0,  48}, { 64,   0,  32}, { 64,   0,  16},
			{ 64,   0,   0}, { 64,  16,   0}, { 64,  32,   0}, { 64,  48,   0},
			{ 64,  64,   0}, { 48,  64,   0}, { 32,  64,   0}, { 16,  64,   0},
			{  0,  64,   0}, {  0,  64,  16}, {  0,  64,  32}, {  0,  64,  48},
			{  0,  64,  64}, {  0,  48,  64}, {  0,  32,  64}, {  0,  16,  64},
			{ 32,  32,  64}, { 40,  32,  64}, { 48,  32,  64}, { 56,  32,  64},
			{ 64,  32,  64}, { 64,  32,  56}, { 64,  32,  48}, { 64,  32,  40},
			{ 64,  32,  32}, { 64,  40,  32}, { 64,  48,  32}, { 64,  56,  32},
			{ 64,  64,  32}, { 56,  64,  32}, { 48,  64,  32}, { 40,  64,  32},
			{ 32,  64,  32}, { 32,  64,  40}, { 32,  64,  48}, { 32,  64,  56},
			{ 32,  64,  64}, { 32,  56,  64}, { 32,  48,  64}, { 32,  40,  64},
			{ 44,  44,  64}, { 48,  44,  64}, { 52,  44,  64}, { 60,  44,  64},
			{ 64,  44,  64}, { 64,  44,  60}, { 64,  44,  52}, { 64,  44,  48},
			{ 64,  44,  44}, { 64,  48,  44}, { 64,  52,  44}, { 64,  60,  44},
			{ 64,  64,  44}, { 60,  64,  44}, { 52,  64,  44}, { 48,  64,  44},
			{ 44,  64,  44}, { 44,  64,  48}, { 44,  64,  52}, { 44,  64,  60},
			{ 44,  64,  64}, { 44,  60,  64}, { 44,  55,  64}, { 44,  48,  64},
			{  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
			{  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}
		};

		WPGHeader Header;
		WPGRecord Rec;
		WPG2Record Rec2;
		WPG2Start StartWPG;
		WPGBitmapType1 BitmapHeader1;
		WPG2BitmapType1 Bitmap2Header1;
		WPGBitmapType2 BitmapHeader2;
		WPGColorMapRec WPG_Palette;

		int i, bpp, WPG2Flags;
		long ldblk;
		size_t count;

		if (!pStreamIn->Read(&Header, sizeof(Header)))
			throw IW::invalid_file();

		if (Header.FileId!=0x435057FF || (Header.ProductType>>8)!=0x16)
			throw IW::invalid_file();

		if (Header.EncryptKey!=0)
			throw IW::invalid_file();

		//image->colors = 0;
		bpp=0;
		bool eof = false;


		int width = 0, height = 0;
		int x = 0, y = 0;
		int colors = 0;
		COLORREF palette[256];

		switch(Header.FileType)
		{
		case 1:     /*WPG level 1*/
			while(!eof) /* object parser loop */
			{
				pStreamIn->Seek(IW::IStreamCommon::eBegin, Header.DataOffset);

				if(eof != false)
					break;

				Rec.RecType=(i=ReadBlobByte(pStreamIn));
				if(i==EOF)
					break;
				Rd_WP_DWORD(pStreamIn,&Rec.RecordLength);
				if(eof != false)
					break;

				Header.DataOffset=TellBlob(pStreamIn) + Rec.RecordLength;

				switch(Rec.RecType)
				{
				case 0x0B: // bitmap type 1 

					if (!pStreamIn->Read(&BitmapHeader1, sizeof(BitmapHeader1)))
						throw IW::invalid_file();

					if(BitmapHeader1.HorzRes && BitmapHeader1.VertRes)
					{
						/*image->units=PixelsPerCentimeterResolution;
						image->x_resolution=BitmapHeader1.HorzRes/470.0;
						image->y_resolution=BitmapHeader1.VertRes/470.0;*/
					}
					width=BitmapHeader1.Width;
					height=BitmapHeader1.Heigth;
					bpp=BitmapHeader1.Depth;

					pImageOut->CreatePage(CRect(0,0,width,height), IW::PixelFormat::FromBpp(bpp), false);

					if (colors == 0 && bpp!=24)
					{
						for (i=0; (i < colors) && (i < 256) ;i++)
						{
							palette[i] = RGB(
								ScaleCharToQuantum(WPG1Palette[i].red),
								ScaleCharToQuantum(WPG1Palette[i].green),
								ScaleCharToQuantum(WPG1Palette[i].blue));
						}
					}

					if (bpp == 1)
					{
						if(palette[0] == 0 && palette[1] == 0)
						{  
							/*fix crippled monochrome palette*/
							palette[1] = 0xFFFFFF;
						}
					}

					if (bpp!=24)
					{
						for(int i = 0; i < 256; i++) palette[1] = IW::SwapRB(palette[1]);
						pImageOut->SetPalette(palette);
					}

					if(UnpackWPGRaster(pStreamIn, pImageOut, width, height, bpp) < 0)
					{
						throw IW::invalid_file();
					}

					break;

				case 0x0E:  /*Color palette */
					WPG_Palette.StartIndex=ReadBlobLSBShort(pStreamIn);
					WPG_Palette.NumOfEntries=ReadBlobLSBShort(pStreamIn);

					colors = WPG_Palette.NumOfEntries;

					for (i=WPG_Palette.StartIndex; i < colors; i++)
					{
						palette[i] = RGB(
							ScaleCharToQuantum(ReadBlobByte(pStreamIn)),
							ScaleCharToQuantum(ReadBlobByte(pStreamIn)),
							ScaleCharToQuantum(ReadBlobByte(pStreamIn)));


					}

					//pImageOut->SetPalette(palette);
					break;

				case 0x11:  /* Start PS l1 */
					/*if (Rec.RecordLength > 8)
					image=ExtractPostscript(image,image_info,TellBlob(pStreamIn)+8,
					(long) Rec.RecordLength-8,exception);*/
					break;

				case 0x14:  // bitmap type 2 

					if (!pStreamIn->Read(&BitmapHeader2, sizeof(BitmapHeader2)))
						throw IW::invalid_file();

					//image->units=PixelsPerCentimeterResolution;

					/*width=(unsigned long)((BitmapHeader2.LowLeftX-BitmapHeader2.UpRightX)/470.0);
					height=(unsigned long)((BitmapHeader2.LowLeftX-BitmapHeader2.UpRightY)/470.0);
					x=(int) (BitmapHeader2.LowLeftX/470.0);
					y=(int) (BitmapHeader2.LowLeftX/470.0);*/

					/*if(BitmapHeader2.HorzRes && BitmapHeader2.VertRes)
					{
					image->x_resolution=BitmapHeader2.HorzRes/470.0;
					image->y_resolution=BitmapHeader2.VertRes/470.0;
					}*/

					width=BitmapHeader2.Width;
					height=BitmapHeader2.Heigth;
					bpp=BitmapHeader2.Depth;

					pImageOut->CreatePage(CRect(0,0,width,height),  IW::PixelFormat::FromBpp(bpp), false);

					if (colors == 0 && bpp!=24)
					{
						for (i=0; (i < colors) && (i < 256) ;i++)
						{
							palette[i] = RGB(
								ScaleCharToQuantum(WPG1Palette[i].red),
								ScaleCharToQuantum(WPG1Palette[i].green),
								ScaleCharToQuantum(WPG1Palette[i].blue));
						}
					}

					if (bpp == 1)
					{
						if(palette[0] == 0 && palette[1] == 0)
						{  
							/*fix crippled monochrome palette*/
							palette[1] = 0xFFFFFF;
						}
					}

					if (bpp!=24)
					{
						for(int i = 0; i < 256; i++) palette[1] = IW::SwapRB(palette[1]);
						pImageOut->SetPalette(palette);
					}

					if(UnpackWPGRaster(pStreamIn, pImageOut, width, height, bpp) < 0)
					{
						throw IW::invalid_file();
					}

					/* Allocate next image structure. */
					/*AllocateNextImage(image_info,image);
					image->depth=8;
					if (GetNextImageInList(image) == (Image *) NULL)
					goto Finish;
					image=SyncNextImageInList(image);
					width=height=0;
					image->colors=0;*/

					width = height = x = y = colors = 0;
					break;

				case 0x1B:  /*Postscript l2*/
					/*if (Rec.RecordLength > 0x3C)
					image=ExtractPostscript(image,image_info,TellBlob(pStreamIn)+0x3C,
					(long) Rec.RecordLength-0x3C,exception);*/
					break;
				}
			}
			break;

		case 2:  // WPG level 2
			(void) IW::MemZero(&StartWPG,sizeof(StartWPG));
			while(!eof) /* object parser loop */

			{
				pStreamIn->Seek(IW::IStreamCommon::eBegin, Header.DataOffset);

				if(eof != false)
					break;

				Rec2.Class=(i=ReadBlobByte(pStreamIn));

				if(i==EOF)
					break;

				Rec2.RecType=(i=ReadBlobByte(pStreamIn));

				if(i==EOF)
					break;

				Rd_WP_DWORD(pStreamIn,&Rec2.Extension);
				Rd_WP_DWORD(pStreamIn,&Rec2.RecordLength);

				if(eof != false)
					break;

				Header.DataOffset=TellBlob(pStreamIn)+Rec2.RecordLength;

				switch(Rec2.RecType)
				{
				case 1:

					if (!pStreamIn->Read(&StartWPG, sizeof(StartWPG)))
						throw IW::invalid_file();

					break;
				case 0x0C:    /*Color palette */
					WPG_Palette.StartIndex=ReadBlobLSBShort(pStreamIn);
					WPG_Palette.NumOfEntries=ReadBlobLSBShort(pStreamIn);

					colors=WPG_Palette.NumOfEntries;

					for (i=WPG_Palette.StartIndex; i < (int)WPG_Palette.NumOfEntries; i++)
					{
						palette[i] = RGB(
							ScaleCharToQuantum(WPG1Palette[i].red),
							ScaleCharToQuantum(WPG1Palette[i].green),
							ScaleCharToQuantum(WPG1Palette[i].blue));

						(void) ReadBlobByte(pStreamIn);   /*Opacity??*/
					}
					break;
				case 0x0E:

					if (!pStreamIn->Read(&Bitmap2Header1, sizeof(Bitmap2Header1)))
						throw IW::invalid_file();

					if(Bitmap2Header1.Compression>1)
						continue; /*Unknown compression method */
					switch(Bitmap2Header1.Depth)
					{
					case 1:
						bpp=1;
						break;
					case 2:
						bpp=2;
						break;
					case 3:
						bpp=4;
						break;
					case 4:
						bpp=8;
						break;
					case 8:
						bpp=24;
						break;
					default:
						continue;  /*Ignore raster with unknown depth*/
					}
					width=Bitmap2Header1.Width;
					height=Bitmap2Header1.Heigth;

					if (colors == 0 && bpp!=24)
					{
						colors = 1 << bpp;
					}

					switch(Bitmap2Header1.Compression)
					{
					case 0:    /*Uncompressed raster*/
						{
							ldblk=(long) ((bpp*width+7)/8);
							IW::CAutoFree<BYTE> BImgBuff(ldblk);

							for(i=0;i<(long) height;i++)
							{
								count=pStreamIn->Read(BImgBuff,ldblk);
								if (bpp == 24) IW::ConvertRGBtoBGR(BImgBuff, BImgBuff, width);
								pImageOut->SetBitmap(i, BImgBuff);
							}
							break;
						}
					case 1:    /*RLE for WPG2 */
						{
							if( UnpackWPG2Raster(pStreamIn, pImageOut, width, height, bpp) < 0)
								throw IW::invalid_file();
							break;
						}
					}

					/* Allocate next image structure. */
					/*AllocateNextImage(image_info,image);
					image->depth=8;
					if (GetNextImageInList(image) == (Image *) NULL)
					goto Finish;
					image=SyncNextImageInList(image);
					width=height=0;
					image->colors=0;*/
					break;

				case 0x12:  /* Postscript WPG2*/
					/*i=ReadBlobLSBShort(pStreamIn);
					if ((long) Rec2.RecordLength > i)
					image=ExtractPostscript(image,image_info,TellBlob(pStreamIn)+i,
					(long) (Rec2.RecordLength-i-2),exception);*/
					break;
				case 0x1B:          /*bitmap rectangle*/
					WPG2Flags = LoadWPG2Flags(pStreamIn, StartWPG.PosSizePrecision, NULL);
					break;
				}
			}

			break;

		default:
			throw IW::invalid_file();
		}
	} 
	catch(std::exception &)
	{
	}

	return true;
}

bool CLoadWpg::Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus)
{
	return false;
}

