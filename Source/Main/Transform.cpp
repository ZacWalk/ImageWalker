#include "StdAfx.h"
#include "BackBuffer.h"
//#include "Animator.h"



void Fade::FadeIn(DWORD timeLimit)
{
	if (!_backBuffer.IsRectEmpty())
	{
		DWORD timeStart = GetTickCount();
		int nRefreshRate = IW::LowerLimit<25>(_dcTarget.GetDeviceCaps(VREFRESH)); 
		DWORD timeFrameLimit = 1000 / nRefreshRate;				

		BackBuffer frontBuffer(_backBuffer.GetDC(), _rectClient);
		_backBuffer.Draw(_backBuffer.GetDC());
		frontBuffer.Capture(_canvas);
		
		for(int i = 0; i < 20; i++)
		{
			DWORD timeStartFrame = GetTickCount();

			if ((timeStartFrame - timeStart) > timeLimit)
				break;

			frontBuffer.Blend(64);
			_backBuffer.Flip();

			const DWORD timeTakenFrame = GetTickCount() - timeStartFrame;
			const DWORD dwSleep = (timeTakenFrame < timeFrameLimit) ? timeFrameLimit - timeTakenFrame : 0;

			Sleep(dwSleep);
		}

		_canvas.Invalidate();
	}
}