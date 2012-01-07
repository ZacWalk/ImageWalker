#pragma once

struct ScaleMode
{
	typedef enum  { Fit, Fill, Up, Down, Normal } Type;
};

class Scale
{
protected:	

	ScaleMode::Type m_eScaleType;
	int m_nScaleSet;

public:

	Scale() : 
	  m_eScaleType(ScaleMode::Normal),
	  m_nScaleSet(100)
	{
	}

	enum { DIV = 0x10000 };

	void SetScale(ScaleMode::Type type)
	{
		m_eScaleType = type;
		m_nScaleSet = 100;
	}

	void SetScale(int s)
	{
		m_nScaleSet = IW::LowerLimit<1>(s);
		m_nScaleSet = IW::UpperLimit<1000>(m_nScaleSet);
		m_eScaleType = ScaleMode::Normal;
	}

	bool IsResizing() const
	{
		return m_eScaleType != ScaleMode::Normal;
	}

	ScaleMode::Type GetScaleType() const 
	{
		return m_eScaleType;
	}

	CSize CalcSize(const CSize &sizeIn, const CSize &sizeClient, const CSize &sizeWindow) const
	{		
		CSize sizeOut;

		int s = CalcScale(sizeIn, sizeClient, sizeWindow);

		sizeOut.cx = IW::LowerLimit<1>(MulDiv(sizeIn.cx, s, DIV));
		sizeOut.cy = IW::LowerLimit<1>(MulDiv(sizeIn.cy, s, DIV));

		return sizeOut;
	}

	int CalcScalePercent(const CSize &sizeIn, const CSize &sizeClient, const CSize &sizeWindow) const
	{
		return MulDiv(100, CalcScale(sizeIn, sizeClient, sizeWindow), DIV);
	}


	int CalcScale(const CSize &sizeIn, const CSize &sizeClient, const CSize &sizeWindow) const
	{
		int s = MulDiv(m_nScaleSet, DIV, 100);
		bool bCalc = false;
		CSize sizeMax = sizeClient;

		if (m_eScaleType == ScaleMode::Down)
		{
			bCalc = sizeClient.cx < sizeIn.cx ||
				sizeClient.cy < sizeIn.cy;
		}
		else  if (m_eScaleType == ScaleMode::Up)
		{
			bCalc = sizeClient.cx > sizeIn.cx &&
				sizeClient.cy > sizeIn.cy;
		}
		else if (m_eScaleType == ScaleMode::Fill)
		{
			sizeMax = sizeWindow;
			bCalc = true;
		}
		else if (m_eScaleType == ScaleMode::Fit)
		{
			bCalc = true;
		}

		if (bCalc)
		{
			int sh = MulDiv(sizeMax.cy, DIV, sizeIn.cy);
			int sw = MulDiv(sizeMax.cx, DIV, sizeIn.cx);

			s =  IW::Min(sh, sw);
		}

		return s;
	}

	CString GetScaleText()
	{
		CString str;

		if (m_eScaleType == ScaleMode::Fit)
		{
			str = App.LoadString(IDS_FIT);
		}
		else if (m_eScaleType == ScaleMode::Fill)
		{
			str = App.LoadString(IDS_FILL);
		}
		else if (m_eScaleType == ScaleMode::Up)
		{
			str = App.LoadString(IDS_UP);
		}
		else if (m_eScaleType == ScaleMode::Down)
		{
			str = App.LoadString(IDS_DOWN);
		}
		else
		{
			str.Format(_T("%d%%"), m_nScaleSet);
		}

		return str;
	}

	bool Parse(LPCTSTR szScale)
	{
		int nFind;

		CString strScale(szScale);
		CString strFit; strFit.LoadString(IDS_FIT);
		CString strFitToWindow; strFitToWindow.LoadString(IDS_FILL);
		CString strUp; strUp.LoadString(IDS_UP);
		CString strDown; strDown.LoadString(IDS_DOWN);

		strScale.MakeLower();
		strFit.MakeLower();
		strFitToWindow.MakeLower();
		strUp.MakeLower();
		strDown.MakeLower();

		ScaleMode::Type eScaleType = m_eScaleType;
		int nScaleSet = m_nScaleSet;

		if (strScale.Find(strFitToWindow) != -1)
		{
			eScaleType = ScaleMode::Fill;
		}
		else if (strScale.Find(strFit) != -1)
		{
			eScaleType = ScaleMode::Fit;
		}
		else if (strScale.Find(strUp) != -1)
		{
			eScaleType = ScaleMode::Up;
			nScaleSet = 100;
		}
		else if (strScale.Find(strDown) != -1)
		{
			eScaleType = ScaleMode::Down;
			nScaleSet = 100;
		}
		else if ((nFind = strScale.Find(':')) != -1)
		{
			int n = _ttoi(strScale);
			int d = IW::LowerLimit<1>(_ttoi(strScale.GetString()+nFind+1));		
			int ns = MulDiv(n, 100, d);

			nScaleSet = IW::LowerLimit<1>(1);
			nScaleSet = IW::UpperLimit<1000>(nScaleSet);
			eScaleType = ScaleMode::Normal;      
		}
		else
		{
			int ns = _ttoi(strScale);

			nScaleSet = IW::LowerLimit<1>(ns);
			nScaleSet = IW::UpperLimit<1000>(nScaleSet);
			eScaleType = ScaleMode::Normal;
		}

		if (eScaleType != m_eScaleType || nScaleSet != m_nScaleSet)
		{
			m_eScaleType = eScaleType;
			m_nScaleSet = nScaleSet;

			return true;
		}

		return false;
	}

	void Toggle(bool canDown)
	{
		if (m_eScaleType == ScaleMode::Normal)
		{
			SetScale(ScaleMode::Fit);
		}
		else if (m_eScaleType == ScaleMode::Fit && canDown)
		{
			SetScale(ScaleMode::Down);
		}
		else
		{
			SetScale(100);
		}		
	}
};