
#include "stdafx.h"
#include "BackBuffer.h"
#include "Animator.h"


//#pragma comment(lib, "Delayimp.lib")
//#pragma comment(linker, "/DelayLoad:d3d9.dll")
//#pragma comment(lib, "d3d9.lib") 

#include <math.h>


/////////////////////////////////////////////////////////////////////////////////////
//
//

CAnimationSpooler::CAnimationSpooler() :
m_hWnd(NULL),
m_bIsAnimating(false),
m_bIsInitialized(false),
m_nBuffers(0)
{
	::ZeroMemory(m_p3DVertices, sizeof(m_p3DVertices));
	::ZeroMemory(m_p3DTextures, sizeof(m_p3DTextures));
}

CAnimationSpooler::~CAnimationSpooler()
{
	Term();
}


bool CAnimationSpooler::AddJob(HWND hWnd, ANIMJOB& job)
{
	if( hWnd != m_hWnd ) Term();
	if( !m_bIsInitialized && !Init(hWnd) ) return false;
	m_jobs.Add(job);
	return true;
}

bool CAnimationSpooler::IsAnimating() const
{
	return m_bIsAnimating;
}



bool CAnimationSpooler::PrepareAnimation(HWND hWnd)
{
	if( !m_bIsInitialized ) Init(m_hWnd);   
	m_p3DBackSurface= NULL;

	// Create the backdrop surface
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	int cx = rcClient.right - rcClient.left;
	int cy = rcClient.bottom - rcClient.top;
	HRESULT Hr = m_p3DDevice->CreateOffscreenPlainSurface(cx, cy, m_ColorFormat, D3DPOOL_SYSTEMMEM, &m_p3DBackSurface, NULL);
	if( FAILED(Hr) ) return false;

	// Paint the background
	HDC hDC = NULL;
	Hr = m_p3DBackSurface->GetDC(&hDC);
	if( FAILED(Hr) ) return false;

	::SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM) hDC, PRF_CLIENT | PRF_ERASEBKGND);
	m_p3DBackSurface->ReleaseDC(hDC);
	// Allow each job to prepare its 3D objects
	for( int i = 0; i < m_jobs.GetSize(); i++ ) 
	{
		ANIMJOB& job = m_jobs[i];
		if( job.AnimType == ANIM_FLAT ) 
		{
			if( !_PrepareJob_Flat(job) ) 
			{
				Term();
				return false;
			}
		}
	}

	// Assign start time
	DWORD dwTick = ::GetTickCount();
	for( int j = 0; j < m_jobs.GetSize(); j++ )
	{
		ANIMJOB& job = m_jobs[j];
		job.dwStartTick += dwTick;
	}
	m_bIsAnimating = true;
	return true;
}

bool CAnimationSpooler::PrepareAnimation(HWND hWnd, BackBuffer *pBackground)
{
	if (PrepareAnimation(hWnd))
	{
		HDC hDC = NULL;
		HRESULT hr = m_p3DBackSurface->GetDC(&hDC);

		if( SUCCEEDED(hr) )
		{
			pBackground->Draw(hDC);
			m_p3DBackSurface->ReleaseDC(hDC);
		}
	}

	return true;
}



bool CAnimationSpooler::Render()
{
	if( !m_bIsAnimating ) return false;
	if( !m_bIsInitialized ) return false;

	HRESULT Hr;
	// Get render target
	Hr = m_p3DDevice->GetRenderTarget(0, &m_p3DTargetSurface);
	if( FAILED(Hr) ) return false;
	Hr = m_p3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0L);
	// Copy backdrop
	Hr = m_p3DDevice->UpdateSurface(m_p3DBackSurface, NULL, m_p3DTargetSurface, NULL);
	// Here begins the rendering loop.
	Hr = m_p3DDevice->BeginScene();
	if( FAILED(Hr) ) return false;
	DWORD dwTick = ::GetTickCount();
	int nAnimated = 0;

	for( int i = 0; i < m_jobs.GetSize(); i++ ) 
	{
		DWORD dwTickNow = dwTick;
		ANIMJOB& job = m_jobs[i];
		if( dwTick < job.dwStartTick ) continue;
		if( dwTick > job.dwStartTick + job.dwDuration ) 
		{
			dwTickNow = job.dwStartTick + job.dwDuration;
			nAnimated--;
		}
		switch( job.AnimType ) 
		{
		case ANIM_FLAT:
			_RenderJob_Flat(job, dwTickNow);
		}
		nAnimated++;
	}
	m_p3DDevice->EndScene();
	m_p3DDevice->Present(NULL, NULL, NULL, NULL);
	m_p3DTargetSurface = NULL;
	// No more frames to animate?
	if( nAnimated == 0 ) Term();
	return true;
}


typedef IDirect3D9 * (WINAPI * Direct3DCreate9Func)(UINT SDKVersion);

static IDirect3D9* GetD3D()
{
	// Is DirectX v9 available at all?
	HMODULE hMod = ::LoadLibrary(_T("D3D9.DLL"));

	if( hMod != NULL ) 
	{
		// Initialize Direct3D
		//m_pD3D = ::Direct3DCreate9(D3D_SDK_VERSION);
		//if( m_pD3D == NULL ) return false;

		Direct3DCreate9Func InitD3D = (Direct3DCreate9Func)GetProcAddress(hMod, "Direct3DCreate9");

		if (InitD3D)
		{
			return InitD3D(D3D_SDK_VERSION);
		}
	}

	return 0;
}

bool CAnimationSpooler::Init(HWND hWnd)
{
	// Is window topmost?
	HWND hWndFocus = hWnd;
	while( ::GetParent(hWndFocus) != NULL ) hWndFocus = ::GetParent(hWndFocus);

	static CComPtr<IDirect3D9> pD3D = GetD3D();
	m_pD3D = pD3D;

	if( m_pD3D == NULL ) return false;

	// Gather information
	RECT rcWindow = { 0 };
	::GetWindowRect(hWnd, &rcWindow);

	HRESULT Hr;
	D3DDISPLAYMODE d3ddm = { 0 };
	Hr = m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if( FAILED(Hr) ) return false;

	m_ColorFormat = d3ddm.Format;

	Hr = m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		m_ColorFormat,
		m_ColorFormat,
		TRUE);
	if( FAILED(Hr) ) return false;

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //D3DSWAPEFFECT_FLIP
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat = m_ColorFormat;
	d3dpp.EnableAutoDepthStencil = FALSE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	Hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWndFocus,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&m_p3DDevice);
	if( FAILED(Hr) ) return false;  

	// Check device caps
	D3DCAPS9 caps;
	Hr = m_p3DDevice->GetDeviceCaps(&caps);
	if( caps.MaxTextureWidth < 256 ) return false;
	if( (caps.Caps3 & D3DCAPS3_COPY_TO_VIDMEM) == 0 ) return false;
	if( FAILED(Hr) ) return false;

	// Set viewport
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = rcWindow.right - rcWindow.left;
	vp.Height = rcWindow.bottom - rcWindow.top;
	vp.MinZ = 0.0;
	vp.MaxZ = 0.0;
	Hr = m_p3DDevice->SetViewport(&vp);
	if( FAILED(Hr) ) return false;

	// Prepare scene
	m_p3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	m_p3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_p3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	// Set the render flags.
	m_p3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_p3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_p3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	// Set miscellaneous render states
	m_p3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_p3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_p3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_p3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_p3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	m_p3DDevice->SetVertexShader(NULL);
	// Signal go...
	m_bIsInitialized = true;
	m_hWnd = hWnd;
	return true;
}

void CAnimationSpooler::Term()
{
	m_jobs.RemoveAll();

	// Release Direct3D references
	for( int i = 0; i < m_nBuffers; i++ ) 
	{
		m_p3DVertices[i]->Release();
		m_p3DTextures[i]->Release();
	}

	m_nBuffers = 0;
	m_p3DTargetSurface = NULL;
	m_p3DBackSurface = NULL;
	m_p3DDevice = NULL;
	m_pD3D = NULL;
	// Almost done...
	m_bIsAnimating = false;
	m_bIsInitialized = false;
}


const double PI = 3.1415926535897932384626433832795029L;

double LinearInterpolate(double y1, double y2, double mu)
{
	return y1 * (1.0 - mu) + y2 * mu;
}

double CosineInterpolate(double y1, double y2, double mu)
{
	double mu2 = (1.0 - cos(mu * PI)) / 2.0;
	return y1 * (1.0 - mu2) + y2 * mu2;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

bool CAnimationSpooler::_PrepareJob_Flat(ANIMJOB& job)
{
	RECT rc = job.data.plot.rcFrom;
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.left;
	float z = 0.1f;
	float rhw = 1.0f / (z * 990.0f + 10.0f);
	D3DCOLOR col = 0xffffffff;
	HRESULT Hr;
	// Determine texture size
	int iTexSize = 128;
	if( cx < 128 ) iTexSize = 64;
	if( cx < 64 ) iTexSize = 32;
	float fTexSize = (float) iTexSize;
	// Start building tiles
	job.iBufferStart = m_nBuffers;

	for( int x = rc.left; x < rc.right; x += iTexSize ) 
	{
		for( int y = rc.top; y < rc.bottom; y += iTexSize )
		{
			RECT rcTile = { x, y, min(rc.right, x + iTexSize), min(rc.bottom, y + iTexSize) };
			// Adjust texture corrdinate, because last tile may only use parts
			// of the texture...
			float tcoordx = (iTexSize - (x + fTexSize - rc.right)) / fTexSize;
			float tcoordy = (iTexSize - (y + fTexSize - rc.bottom)) / fTexSize;
			if( tcoordx > 1.0f ) tcoordx = 1.0f;
			if( tcoordy > 1.0f ) tcoordy = 1.0f;
			// Create the vertex buffer
			CUSTOMFAN verts = 
			{
				{ rcTile.left - 0.5f,  rcTile.top - 0.5f,    z, rhw, col, 0.0f, 0.0f },
				{ rcTile.right - 0.5f, rcTile.top - 0.5f,    z, rhw, col, tcoordx, 0.0f },
				{ rcTile.right - 0.5f, rcTile.bottom - 0.5f, z, rhw, col, tcoordx, tcoordy },
				{ rcTile.left - 0.5f,  rcTile.bottom - 0.5f, z, rhw, col, 0.0f, tcoordy }
			};
			Hr = m_p3DDevice->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_p3DVertices[m_nBuffers], NULL);
			if( FAILED(Hr) ) return false;
			memcpy(m_fans[m_nBuffers], verts, sizeof(verts));
			// Create the texture
			Hr = m_p3DDevice->CreateTexture(iTexSize, iTexSize, 1, 0, m_ColorFormat, D3DPOOL_DEFAULT, &m_p3DTextures[m_nBuffers], NULL);
			if( FAILED(Hr) ) return false;
			LPDIRECT3DSURFACE9 pTexSurf = NULL;
			Hr = m_p3DTextures[m_nBuffers]->GetSurfaceLevel(0, &pTexSurf);
			if( FAILED(Hr) ) return false;
			POINT pt = { 0, 0 };
			Hr = m_p3DDevice->UpdateSurface(m_p3DBackSurface, &rcTile, pTexSurf, &pt);
			pTexSurf->Release();
			if( FAILED(Hr) ) return false;
			m_nBuffers++;
		}
	}
	job.iBufferEnd = m_nBuffers;
	assert(m_nBuffers<MAX_BUFFERS);
	// Clear the background so the sprite can take its place
	COLORREF clrBack = job.data.plot.clrBack;
	if( clrBack != CLR_INVALID) 
	{
		HDC hDC = NULL;
		Hr = m_p3DBackSurface->GetDC(&hDC);
		if( FAILED(Hr) ) return false;
		HBRUSH hBrush = ::CreateSolidBrush(clrBack);
		::FillRect(hDC, &rc, hBrush);
		::DeleteObject(hBrush);
		m_p3DBackSurface->ReleaseDC(hDC);
	}
	return true;
}

bool CAnimationSpooler::_RenderJob_Flat(ANIMJOB& job, DWORD dwTick)
{
	RECT rc = job.data.plot.rcFrom;
	float mu = ((float)job.dwStartTick + (float)job.dwDuration - (float)dwTick) / (float)job.dwDuration;
	float scale1 = 0.0;
	if( job.data.plot.iInterpolate == INTERPOLATE_LINEAR ) scale1 = (float) LinearInterpolate(0.0, 1.0, mu);
	if( job.data.plot.iInterpolate == INTERPOLATE_COS ) scale1 = (float) CosineInterpolate(0.0, 1.0, mu);
	float scale2 = 1.0f - scale1;
	D3DVECTOR ptCenter = { rc.left + ((rc.right - rc.left) / 2.0f), rc.top + ((rc.bottom - rc.top) / 2.0f) };
	float xtrans = (float)job.data.plot.mFrom.xtrans * scale1;
	float ytrans = (float)job.data.plot.mFrom.ytrans * scale1;
	float ztrans = 1.0f + ((float)abs(job.data.plot.mFrom.ztrans) * (job.data.plot.mFrom.ztrans >= 0.0 ? scale1 : scale2));
	float fSin = (float) sin(job.data.plot.mFrom.zrot * scale1);
	float fCos = (float) cos(job.data.plot.mFrom.zrot * scale1);
	DWORD clrAlpha = ((DWORD)(0xFF - (float)abs(job.data.plot.mFrom.alpha) * (job.data.plot.mFrom.alpha >= 0 ? scale1 : scale2)) << 24) | 0xffffff;
	HRESULT Hr = 0;
	for( int iBuffer = job.iBufferStart; iBuffer < job.iBufferEnd; iBuffer++ ) 
	{
		// Lock the vertex buffer and apply transformation
		LPDIRECT3DVERTEXBUFFER9 pVBuffer = m_p3DVertices[iBuffer];
		LPVOID pVertices = NULL;
		Hr = pVBuffer->Lock(0, sizeof(CUSTOMFAN), &pVertices, 0);
		if( FAILED(Hr) ) return false;
		CUSTOMFAN verts;
		memcpy(verts, m_fans[iBuffer], sizeof(CUSTOMFAN));
		for( int i = 0; i < 4; i++ )
		{
			verts[i].x += xtrans;
			verts[i].y += ytrans;
			verts[i].x -= ptCenter.x;
			verts[i].y -= ptCenter.y;
			verts[i].x = verts[i].x * ztrans;
			verts[i].y = verts[i].y * ztrans;
			float x = verts[i].x; float y = verts[i].y;
			verts[i].x = x * fCos - y * fSin;
			verts[i].y = x * fSin + y * fCos;
			verts[i].x += ptCenter.x;
			verts[i].y += ptCenter.y;
			verts[i].color = clrAlpha;
		}
		memcpy(pVertices, verts, sizeof(CUSTOMFAN));
		pVBuffer->Unlock();
		// Paint it
		Hr = m_p3DDevice->SetStreamSource(0, pVBuffer, 0, sizeof(CUSTOMVERTEX));
		Hr = m_p3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		Hr = m_p3DDevice->SetTexture(0, m_p3DTextures[iBuffer]);
		Hr = m_p3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	}

	return true;
}

