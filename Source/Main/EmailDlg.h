#pragma once

#include "email.h"

class CEmail : public IW::CSimpleMapi 
{
protected:

	State &_state;
	CLoadAny _loader;
	CSimpleValArray<CString> m_arrayFolderNames;
	CSimpleValArray<CString> m_arrayTempFiles;

	IW::CSimpleZip m_zip;
	IW::CFilePath _pathZip;

	bool _bZip;
	bool _bScale;
	int _nSize;

public:
	CEmail(State &state, bool bZip, bool bScale, int nSize) : 
	  _state(state), 
	  _loader(state.Plugins), 
	  _bZip(bZip), 
	  _bScale(bScale), 
	  _nSize(nSize)
	{
	}

	bool Open()
	{	
		if (!IW::CSimpleMapi::Open())
		{
			return false;
		}

		if (_bZip)
		{			
			_pathZip.GetTempFilePath();
			m_zip.Create(_pathZip);			
		}

		return true;
	}

	bool Close()
	{
		if (_bZip)
		{
			if (m_zip.Close())
			{
				AddFile(_pathZip, _T("Files.zip"));
				m_arrayTempFiles.Add(_pathZip);
			}
		}		
		
		return true;		
	}

	virtual ~CEmail(void)
	{
		
		for(int i = 0; i < m_arrayTempFiles.GetSize(); i++)
		{
			::DeleteFile(m_arrayTempFiles[i]);
		}

		m_arrayTempFiles.RemoveAll();
	}

	bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus)
	{
		// Pop on this folder name	
		m_arrayFolderNames.Add(pFolder->GetFolderName());

		// May need to create
		pFolder->IterateItems(this, pStatus);

		// pop off this folder name
		m_arrayFolderNames.RemoveAt(m_arrayFolderNames.GetSize() - 1);

		return true;
	}

	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
	{
		CString strFileName = pItem->GetFileName();
		CString strFilePath = pItem->GetFilePath();

		// If scale
		if (_bScale)
		{
			// Scale the image
			IW::Image image;

			LPCTSTR szExt = IW::Path::FindExtension(strFileName);
			App.RecordFileOperation(strFileName, _T("Loading Image"));

			IW::ImageStream<IW::IImageStream> imageOut(image);
			if (_loader.LoadImage(strFilePath, &imageOut, pStatus) && !image.IsEmpty())
			{
				const CRect r = image.GetBoundingRect();

				if (r.Width() > _nSize || r.Height() > _nSize)
				{
					CSize size(_nSize, _nSize);
					IW::Image imageOut;

					IW::ImageStreamScale<IW::CNull> thumbnailStream(imageOut, Search::Any, size); 
					IW::IterateImage(image, thumbnailStream, pStatus);
					
					IW::CFilePath path;
					path.GetTempFilePath();
					IW::CFile f;

					if (f.OpenForWrite(path))
					{
						m_arrayTempFiles.Add(path);

						if (_loader.Write(szExt, &f, imageOut, pStatus))
						{
							// The temp file is the one we want
							strFilePath = (CString&)path;
						}

						f.Close(pStatus);
					}
				}
			}
		}

		if (_bZip)
		{
			CString strSubFolder;

			for(int i = 0; i < m_arrayFolderNames.GetSize(); i++)
			{
				strSubFolder += m_arrayFolderNames[i];
				strSubFolder += _T("/");
			}

			strSubFolder += strFileName;
			m_zip.AddFile(strFilePath, strSubFolder, pItem->GetLastWriteTime(), pStatus);
		}
		else
		{
			if (!AddFile(strFilePath, strFileName))
			{
				// Failed to add this file?
				return false;
			}
		}

		pStatus->SetMessage(App.LoadString(IDS_SENT));


		return true;
	}

	bool EndItem()
	{
		return true;
	}

	bool EndFolder()
	{
		return true;
	}

	// Called periodically to allow cancel
	bool QueryCancel()
	{
		return false;
	}
};

class CEmailDlg : public CDialogImpl<CEmailDlg>
{
protected:
public:
	CEmailDlg()
	{		
		_nSize = 1024;
		_bZip = false;
		_bShrink = false;
	}

	enum { IDD = IDD_EMAIL };
	
	int _nSize;
	bool _bZip;
	bool _bShrink;
	
	BEGIN_MSG_MAP(CEmailDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_SHRINK, OnShrink)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()	

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());	

		CPropertyArchiveRegistry archive(App.GetRegKey());
		
		if (archive.StartSection(g_szEmail))
		{
			archive.Read(g_szSize, _nSize);
			archive.Read(g_szZip, _bZip);
			archive.Read(g_szShrink, _bShrink);
		}

		CComboBox comboSize = GetDlgItem(IDC_RESIZE_LENGTH);
		int resizeValues[] = { 800, 1024, 1280, 1600, 2048, -1 };
		IW::SetItems(comboSize, resizeValues, _nSize);

		CheckDlgButton(IDC_ZIP, _bZip ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SHRINK, _bShrink ? BST_CHECKED : BST_UNCHECKED);

		DoEnable();

        bHandled = false;
		return (LRESULT)TRUE;
	}

	void DoEnable()
	{
		bool bEnable  = BST_CHECKED == IsDlgButtonChecked(IDC_SHRINK);
		::EnableWindow(GetDlgItem(IDC_RESIZE_LENGTH), bEnable);
	}	
	
	LRESULT OnShrink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DoEnable();
		return 0;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		_nSize = GetDlgItemInt(IDC_RESIZE_LENGTH);
		_bZip  = BST_CHECKED == IsDlgButtonChecked(IDC_ZIP);
		_bShrink  = BST_CHECKED == IsDlgButtonChecked(IDC_SHRINK);

		CPropertyArchiveRegistry archive(App.GetRegKey(), true);
		
		if (archive.StartSection(g_szEmail))
		{
			archive.Write(g_szSize, _nSize);
			archive.Write(g_szZip, _bZip);
			archive.Write(g_szShrink, _bShrink);
		}
		
		EndDialog(wID);
		return 0;
	}
};