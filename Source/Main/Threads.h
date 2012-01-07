#pragma once

namespace IW
{
	class Thread
	{
	private:
		Thread(const Thread&);
		void operator=(const Thread&);


	protected:
		typedef Thread ThisClass;

		HANDLE _hThread;	
		volatile bool _bExit;
		int _nPriority;

	public:

		Thread(int nPriority = THREAD_PRIORITY_LOWEST) : 
		  _hThread(INVALID_HANDLE_VALUE), 
			  _bExit(false), 
			  _nPriority(nPriority)
		  {
		  }

		  void StartThread()
		  {
			  _hThread = (HANDLE)_beginthread(Start, 0, static_cast<ThisClass*>(this));

			  ATLASSERT(_hThread != INVALID_HANDLE_VALUE);

			  SetThreadPriority(_hThread, _nPriority);
		  }

		  void StopThread()
		  {
			  _bExit = true;
			  WaitForSingleObject(_hThread, 30000);
		  }

		  static void __cdecl Start(LPVOID lParam)
		  {
			  IW::CCoInit coInit(true);
			  Sleep(100);

			  reinterpret_cast<ThisClass*>(lParam)->Process();
			  _endthread();
		  }

		  virtual void Process() = 0;

		  static CEvent _eventExit;
	};


	__declspec(selectany) CEvent Thread::_eventExit(TRUE, FALSE);
}