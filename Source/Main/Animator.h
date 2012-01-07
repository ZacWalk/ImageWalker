#pragma once

#include <d3d9.h>


typedef enum
{
   ANIM_FLAT,
   ANIM_SWIPE,
} ANIMTYPE;


typedef enum
{
   INTERPOLATE_LINEAR,
   INTERPOLATE_COS,
} INTERPOLATETYPE;

struct CUSTOMVERTEX 
{
   FLOAT x, y, z;
   FLOAT rhw;
   DWORD color;
   FLOAT tu, tv;
};
typedef CUSTOMVERTEX CUSTOMFAN[4];

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

typedef struct PLOTMATRIX
{
   void SetEmpty()
   {
      zrot = 0.0;
      xtrans = ytrans = ztrans = 0;
      alpha = 255;
   }
   int xtrans;
   int ytrans;
   int ztrans;
   int alpha;
   float zrot;
} PLOTMATRIX;

class ANIMJOB 
{
public:
   ANIMJOB()
   {
   }
   ANIMJOB(int _AnimType, DWORD _dwDuration, DWORD _dwStartTick = 0)
   {
      AnimType = _AnimType;
      dwStartTick = _dwStartTick;
      dwDuration = _dwDuration;
   }
   ANIMJOB(const ANIMJOB &other)
   {
	   Copy(other);
   }
   void operator=(const ANIMJOB &other)
   {
	   Copy(other);
   }

   void SetPlot(COLORREF _clrBack, CRect _rcFrom, PLOTMATRIX _mFrom)
   {
      data.plot.clrBack = _clrBack;
      data.plot.rcFrom = _rcFrom;
      data.plot.mFrom = _mFrom;
      data.plot.iInterpolate = INTERPOLATE_COS;
   }

   void Copy(const ANIMJOB &other)
   {
	   AnimType = other.AnimType;
	   dwStartTick = other.dwStartTick;
	   dwDuration = other.dwDuration;
	   iBufferStart = other.iBufferStart;
	   iBufferEnd = other.iBufferEnd;
	   data.plot = other.data.plot;
	   image = other.image;
   }

   int AnimType;
   DWORD dwStartTick;
   DWORD dwDuration;
   int iBufferStart;
   int iBufferEnd;
   IW::Image image;

   union
   {
      struct 
      {
         COLORREF clrBack;
         RECT rcFrom;
         PLOTMATRIX mFrom;
         INTERPOLATETYPE iInterpolate;
      } plot;
   } data;
};



class CAnimationSpooler
{
public:
   CAnimationSpooler();
   ~CAnimationSpooler();

   enum { MAX_BUFFERS = 256 };

   bool PrepareAnimation(HWND hWnd);
   bool PrepareAnimation(HWND hWnd, BackBuffer *pBackground);
   bool Render();

   bool IsAnimating() const;
   bool AddJob(HWND hWnd, ANIMJOB& job);
   
    bool Init(HWND hWnd);

protected:
  
   void Term();

   bool _PrepareJob_Flat(ANIMJOB& job);
   bool _RenderJob_Flat(ANIMJOB& job, DWORD dwTick);

protected:
   HWND m_hWnd;
   bool m_bIsAnimating;
   bool m_bIsInitialized;
   CSimpleArray<ANIMJOB> m_jobs;
   D3DFORMAT m_ColorFormat;

   CComPtr<IDirect3D9> m_pD3D;
   CComPtr<IDirect3DDevice9> m_p3DDevice;
   CComPtr<IDirect3DSurface9> m_p3DBackSurface;
   CComPtr<IDirect3DSurface9> m_p3DTargetSurface;
   
   LPDIRECT3DVERTEXBUFFER9 m_p3DVertices[MAX_BUFFERS];
   LPDIRECT3DTEXTURE9 m_p3DTextures[MAX_BUFFERS];
   CUSTOMFAN m_fans[MAX_BUFFERS];
   int m_nBuffers;
};



