#pragma once

#include "Threads.h"
#include "LoadAny.h"

class CStopper : public IW::IStatus
{
public:

	volatile bool &m_bStopper;
	CStopper(volatile bool &bStopper) : m_bStopper(bStopper) {};

	void Progress(int nCurrentStep, int TotalSteps) {  };
	bool QueryCancel() {  return m_bStopper; }; 
	void SetStatusMessage(const CString &strMessage) { }; 
	void SetHighLevelProgress(int nCurrentStep, int TotalSteps) {  };
	void SetHighLevelStatusMessage(const CString &strMessage) {  };
	void SetMessage(const CString &strMessage) {  };
	void SetWarning(const CString &strWarning) {  };
	void SetError(const CString &strError) {  };
	void SetContext(const CString &strContext) {  };

};




class ImageLoaderThread
{
protected:

	CEvent _eventRefresh;
	CCriticalSection _cs;

	HANDLE _hDecodeAndScaleThread;
	volatile bool _bExit;
	volatile bool _bStopLoading;

	IW::RefPtr<CImageLoad> _pInfo;

public:

	typedef ImageLoaderThread ThisClass;

	Coupling *_pCoupling;
	PluginState &_plugins;
	State &_state;

	ImageLoaderThread(PluginState &plugins, Coupling *pCoupling, State &state) : 
		_pCoupling(pCoupling),
		_eventRefresh(FALSE, FALSE),
		_bExit(false),
		_bStopLoading(false),
		_hDecodeAndScaleThread(INVALID_HANDLE_VALUE),
		_plugins(plugins),
		_state(state)
	{
	}

	void StartThread()
	{
		// Start worker thread
		_hDecodeAndScaleThread = (HANDLE)_beginthread(StartDecodeAndScale, 0, this); 	
		ATLASSERT(_hDecodeAndScaleThread != INVALID_HANDLE_VALUE);	
		SetThreadPriority(_hDecodeAndScaleThread, THREAD_PRIORITY_LOWEST + 1);	
	}

	void StopThread()
	{
		_bExit = true;

		WaitForSingleObject(_hDecodeAndScaleThread, 20000);
		//CloseHandle(_hDecodeAndScaleThread);
	}

	void SetStopLoading()
	{
		_bStopLoading = true;
	}

	void ShowImage(CImageLoad *pInfo)
	{
		IW::CAutoLockCS lock(_cs);

		SetStopLoading();
		
		_pInfo = pInfo;
		_eventRefresh.Set();

		CString str;
		str.Format(IDS_FMT_LOADING, IW::Path::FindFileName(pInfo->_path));
		_pCoupling->SetStatusText(str);
	}
	
	static void __cdecl StartDecodeAndScale(LPVOID lParam)
	{
		App.Log(_T("Decode And Scale thread started"));

		ThisClass *p = reinterpret_cast<ThisClass*>(lParam);

		ATLASSERT(p);

		if (p)
			p->DecodeAndScale();

		_endthread();
	}

	IW::RefPtr<CImageLoad> PopInfo()
	{
		IW::CAutoLockCS lock(_cs);
		IW::RefPtr<CImageLoad> pInfo = _pInfo;
		_pInfo = 0;
		return pInfo;
	}

	void DecodeAndScale()
	{
		IW::CCoInit coInit(true);

		CLoadAny loader(_plugins);
		IW::CShellDesktop desktop;

		HANDLE objects[2];	
		objects[0] = _eventRefresh;
		objects[1] = IW::Thread::_eventExit;

		CStopper stopperLoading(_bStopLoading);	

		while(!_bExit)
		{
			DWORD dw = WaitForMultipleObjects(2, objects, FALSE, INFINITE);		

			if (dw == (WAIT_OBJECT_0 + 0)) // load
			{
				const DWORD timeStart = GetTickCount();
				_eventRefresh.Reset();
				_bStopLoading = false;

				IW::RefPtr<CImageLoad> pInfo = PopInfo();

				if (pInfo)
				{				
					IW::ScopeLockedBool isSearching(_state.Image.IsLoading);
					App.RecordFileOperation(pInfo->_path, _T("Loading Image"));

					if (pInfo->Load(loader, &stopperLoading))
					{
						if (!_bStopLoading && pInfo->_bDelay)
						{
							WaitDelay(timeStart);								
						}
					}

					ATLTRACE(_T("Load of %s complete with StopLoading=%d\n"), pInfo->_path, _bStopLoading);

					pInfo->_bWasStopped = _bStopLoading;
					_pCoupling->SignalImageLoadComplete(pInfo.Detach());
				}
			}
			else
			{
				return;
			}
		}
	}

	void WaitDelay(const DWORD timeStart)
	{
		const DWORD dwDelay = App.Options.m_nDelay * 1000;

		while(!_bStopLoading &&
			((GetTickCount() - timeStart) < dwDelay))
		{
			Sleep(100);
		};
	}
};

