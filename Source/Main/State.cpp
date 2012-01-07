#include "stdafx.h"
#include "state.h"
#include "YesNoDlg.h"

bool ImageState::CanDisplayNewImage(const CString &strNewImageName)
{
	IW::Focus preserveFocus;

	if (IsDirty())
	{
		if (App.Options.m_bAutoSave)
		{
			_pCoupling->SetStatusText(App.LoadString(IDS_SAVINGIMAGEFILE));
			return Save();
		}
		else
		{
			return QuerySave();
		}
	}

	return true;
}

CString ImageState::GetChangesList() const
{
	CString str;
	for(int i = 0; i < _arUndo.GetSize(); i++)
	{
		IW::AddToList(str, _arUndo[i]->GetAction());
	}
	return str;
}

bool ImageState::QuerySave()
{
	if (IsDirty())
	{
		CString str = IW::Format(IDS_SAVE_CHANGES,  IW::Path::FindFileName(GetImageFileName()), GetChangesList());
		CYesNoDlg dlg(GetImage(), str);
		int nRet = dlg.DoModal();
		if (nRet == IDCANCEL)
		{
			return false;
		}
		else if (nRet == IDYES)
		{
			return Save();
		}
	}

	return true;
}