#pragma once


class BackBuffer
{
private:

	CDC			  _dc;
	CDCHandle     _dcTarget;          // Owner DC
	CBitmap       _bitmap;      // Offscreen bitmap
	CBitmapHandle _hOldBitmap;  // Originally selected bitmap
	CRect         _rc;          // Rectangle of drawing area


public:

	BackBuffer()
	{
	}

	BackBuffer(HDC hDC, LPRECT pRect = 0)
	{
		Init(hDC, pRect);
	}

	~BackBuffer()
	{
		_dc.SelectBitmap(_hOldBitmap);
	}

	void Init(HDC hDC, LPRECT pRect = 0)
	{
		ATLASSERT(hDC!=NULL);
		_dcTarget = hDC;
		if( pRect!=NULL ) _rc = *pRect; else _dcTarget.GetClipBox(&_rc);

		_dc.CreateCompatibleDC(_dcTarget);
		_dcTarget.LPtoDP(_rc);
		_bitmap.CreateCompatibleBitmap(_dcTarget, _rc.Width(), _rc.Height());
		_hOldBitmap = _dc.SelectBitmap(_bitmap);
		_dcTarget.LPtoDP(_rc);
		_dc.SetWindowOrg(_rc.TopLeft());
	}

	bool IsRectEmpty() const
	{
		return _rc.IsRectEmpty() != 0;
	}

	HDC GetDC()
	{
		return _dc;
	}

	void Capture(HWND hWnd)
	{
		::SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM)(HDC)_dc, PRF_CLIENT | PRF_CHILDREN);
	}

	void Flip()
	{
		Draw(_dcTarget);
	}

	void Draw(CDCHandle dcTarget)
	{
		// Copy the offscreen bitmap onto the screen.
		dcTarget.BitBlt(_rc.left, _rc.top, _rc.Width(), _rc.Height(), _dc, _rc.left, _rc.top, SRCCOPY);
	}

	void Blend(int rate)
	{
		BLENDFUNCTION bf = {0};
		bf.BlendOp = AC_SRC_OVER; 
		bf.SourceConstantAlpha = IW::Min(rate, 255);

		int cx = _rc.Width();
		int cy = _rc.Height();

		_dcTarget.AlphaBlend(_rc.left, _rc.top, cx, cy, _dc, _rc.left, _rc.top, cx, cy, bf);
	}
};

class Fade
{
public:

	CWindow _canvas;
	CClientDC _dcTarget;
	BackBuffer _backBuffer;
	CRect _rectClient;

	Fade(CWindow canvas) : _canvas(canvas), _dcTarget(canvas)
	{		
		_canvas.GetClientRect(_rectClient);
		
		_backBuffer.Init(_dcTarget, _rectClient);
		_backBuffer.Capture(_canvas);	
	}

	void FadeIn(DWORD timeLimit = 500);

	
};