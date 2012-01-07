#pragma once


class StillImage
{
public:
	class IEvents
	{
	public:
		virtual bool TwainAcquire(LPCTSTR szTwainName) = 0;
	};

	static CString GetExePath();
	static CString GetAppName();

	static bool RegisterApplication(HINSTANCE hInstace);
	static bool UnregisterApplication(HINSTANCE hInstace);
	static bool IsLaunchedByEventMonitor(HINSTANCE hInstace, IEvents *pFrame);

};
