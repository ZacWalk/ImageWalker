#pragma once

#include "ProgressDlg.h"
#include "ImageFileDialog.h"
#include "ZStream.h"
#include "BackBuffer.h"
#include "ImageDataObject.h"
#include "ImageMetaData.h"

struct ImageEditMode
{
	typedef enum {
		None,
		Select,
		Rotate,
		Crop,
		Color,
		Resize,
		Sharpen,
		Redeye
	} Mode;
};


class CImageLoad : public IW::Referenced
{
public:

	CImageLoad(CString strPath, bool bIsSlideShow, bool bSignalEvent, long nStartItem, long nLoadItem, long nDirection, bool bDelay, bool bLoadMedia)
	{
		_path = strPath;
		_bIsSlideShow = bIsSlideShow;
		_bSignalEvent = bSignalEvent;
		_nStartItem = nStartItem;
		_nLoadItem = nLoadItem;
		_nDirection = nDirection;
		_bDelay = bDelay;
		_bLoadMedia = bLoadMedia;
		_bWasStopped = false;
	}

	~CImageLoad()
	{
	}	

	bool WasImageLoaded() const
	{
		return !_image.IsEmpty();
	}	

	bool Load(CLoadAny &loader, IW::IStatus *pStatus)
	{
		IW::CFile file;

		if (file.OpenForRead(_path))
		{
			IW::Image image;	

			if (loader.Read(IW::Path::FindExtension(_path), &file, image, pStatus) && !image.IsEmpty())
			{						
				_image = image;
				return true;										
			}
		}

		return false;
	}

	CString _path;
	IW::Image _image;

	bool _bIsSlideShow;
	bool _bSignalEvent;
	long _nStartItem;
	long _nLoadItem;
	long _nDirection;
	bool _bDelay;
	bool _bLoadMedia;
	bool _bWasStopped;	

private:
};

class CSlideShowItem : public IW::Referenced
{
public:
	CSlideShowItem()
	{
		_bSignalEvent = false;
	}

	CSlideShowItem(const CSlideShowItem &i)
	{
		_path = i._path;
		_bSignalEvent = i._bSignalEvent;
	}

	virtual ~CSlideShowItem()
	{
	}

	void operator=(const CSlideShowItem &i)
	{
		_path = i._path;
		_bSignalEvent = i._bSignalEvent;
	}

	CString  _path;
	bool _bSignalEvent;
};

class CUndo : public IW::Referenced
{
protected:
	IW::SimpleBlob _data;
	CString _strAction;
	bool _bDirty;	

public:
	CUndo(IW::Image &image, const CString &strAction = g_szEmptyString, bool bDirty = false) :  _strAction(strAction), _bDirty(bDirty)
	  {
		  IW::Serialize::ArchiveStore archiveStore;
		  image.Serialize(archiveStore);

		  IW::StreamBlob<IW::SimpleBlob>  streamOut(_data);
		  IW::zostream<IW::StreamBlob<IW::SimpleBlob> > zstreamOut(streamOut);
		  archiveStore.Store(zstreamOut);
		  zstreamOut.Close();		
	  }

	  void GetImageState(IW::Image &imageReloaded, bool &bDirty) const
	  {
		  IW::StreamConstBlob streamIn(_data);
		  IW::zistream<IW::StreamConstBlob> zstreamIn(streamIn);

		  IW::Serialize::ArchiveLoad archiveLoad(zstreamIn);
		  imageReloaded.Serialize(archiveLoad);

		  bDirty = _bDirty; 
	  }

	  CString GetAction() const
	  {
		  return _strAction;
	  }

	  virtual ~CUndo() 
	  {
	  }

	  CUndo(const CUndo &other) 
	  {
		  _data = other._data;
		  _strAction = other._strAction;
		  _bDirty = other._bDirty;
	  }

	  void operator=(const CUndo &other)
	  {
		  _data = other._data;
		  _strAction = other._strAction;
		  _bDirty = other._bDirty;
	  }
};



class ImageState
{
private:

	bool _bDirty;		
	bool _bHasFileList;	
	bool _bPlay;	
	bool _bSlideShow;	
	bool _bSlideShowFoundImages;	

	ImageEditMode::Mode _editMode;

	Coupling *_pCoupling;	
	Coupling *_pItems;

	CSimpleArray<IW::RefPtr<CSlideShowItem> > _items;	
	CSimpleArray<IW::RefPtr<CUndo> > _arUndo;	
	IW::CFilePath _path;
	IW::FileTime _ft;
	IW::FileSize _size;	
	IW::Histogram _histogram;	
	IW::Image _image;	
	IW::Image _imageRendered;	
	IW::Image _imageThumbnail;
	long _nSlideShowItem;
	bool _bStopped;

	PluginState &_plugins;

public:	

	volatile bool IsLoading;

	Delegate::List0 EditModeDelegates;

	ImageState(PluginState &plugins, Coupling *coupling, Coupling *pItems) : 
			_pCoupling(coupling),
			_nSlideShowItem(-1),
			_bSlideShowFoundImages(false),
			_bPlay(false),
			_bSlideShow(false),
			_bHasFileList(false),
			_bDirty(false),
			_pItems(pItems),
			_bStopped(false),
			_plugins(plugins),
			IsLoading(false),
			_editMode(ImageEditMode::None)
	  {
	  }

	  ~ImageState()
	  {
		  _arUndo.RemoveAll();
		  _image.Free();
		  _imageRendered.Free();
		  _imageThumbnail.Free();
	  }

	  void SetPlayFlag(bool bPlay)
	  {
		  if (bPlay != _bPlay)
		  {
			  _bPlay = bPlay;
			  _pCoupling->PlayStateChange(bPlay);			
		  }
	  }

	  bool IsSlideShow() const { return _bSlideShow; };
	  bool IsSlideShowPlaying() const { return _bSlideShow && _bPlay; };
	  bool IsSlideShowPaused() const { return _bSlideShow && !_bPlay; };
	  bool IsDirty() const { return _bDirty; }
	  
	  CString GetChangesList() const;
	  bool CanUndo() const { return _arUndo.GetSize() > 0; };
	  bool CanEditImage() const { return IsImageShown(); };	

	  CString GetImageFileName() const { return _path; };
	  const IW::Image &GetImage() const { return _image; }; 
	  const IW::Image &GetThumbnailImage() const { return _imageThumbnail; };
	  const IW::Image &GetRenderImage() const { return _imageRendered.IsEmpty() ? _image : _imageRendered; }; 
	  const IW::Histogram &GetHistogram() const {  return _histogram; };

	  bool UpdateFileTime()
	  {
		  IW::FileTime ft = IW::FileTime::FromFile(_path);
		  bool bHasChanged = ft > _ft;
		  _ft = ft;
		  return bHasChanged;
	  }
	  
	  bool IsPlaying() const throw()
	  {
		  return _bPlay;
	  }

	  void Play()
	  {
		  if (!_bPlay)
		  {
			  _pCoupling->ShowSlideShowView();

			  ITEMLIST itemList = _pItems->GetItemList();
			  GetItemList(itemList, true);

			  _bHasFileList = true;

			  SetPlayFlag(true);
			  _bSlideShow = true;

			  if (!_bSlideShow)
			  {
				  _nSlideShowItem = -1;
				  _bSlideShowFoundImages = false;
				  _bSlideShow = true;
			  }

			  StepImage(1, IsImageShown());
			  UpdateStatusText(); 
		  }
	  }

	  void SetItems(Coupling *pItems)
	  {
		  _pItems = pItems;
	  }

	  void Play(Coupling *pItems)
	  {
		  if (_pItems != pItems || !_bPlay)
		  {
			  _pItems = pItems;
			  Play(); 
		  }
	  }

	  void Pause()
	  {
		  _pCoupling->SetStopLoading();	
		  _bStopped = true;
		  SetPlayFlag(false);

		  UpdateStatusText(); 
	  }

	  void Stop()
	  {
		  if (_bSlideShow)
		  {
			  Pause();

			  _bSlideShow = false;
			  ResetFolderDetails();
		  }

		  UpdateStatusText(); 
	  }

	  bool IsImageShown() const
	  {
		  return !_image.IsEmpty();
	  }

	  /*LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	  {
	  ATLTRACE(_T("Destroy CImageCtrl\n"));

	  if (_bSlideShow)
	  {
	  SlideShowStop();
	  }
	  }
	  */

	  bool CanDisplayNewImage(const CString &strNewImageName);

	  void OnLoadComplete(CImageLoad *pInfo)
	  {
		  if (!pInfo->_bWasStopped && !_bStopped)
		  {
			  if (pInfo->WasImageLoaded())
			  {
				  if (!CanDisplayNewImage(pInfo->_path))
					  return;

				  ResetHistory();
				  SetImage(pInfo->_image, pInfo->_path);				

				  if (pInfo->_bSignalEvent)
				  {
					  _pCoupling->SelectFolderItem(IW::Path::FindFileName(pInfo->_path));				
				  }			
			  }

			  LoadNext(pInfo);
			  UpdateStatusText();
		  }
	  }

	  void LoadImage(int nItem, bool bSignalEvent)
	  {
		  _bStopped = false;

		  try
		  {
			  IW::RefPtr<CImageLoad> pNextInfo = new CImageLoad(_pItems->GetItemPath(nItem), 
				  false, bSignalEvent, nItem, nItem, 0, false, false);		

			  _pCoupling->ShowImage(pNextInfo);
		  }
		  catch (std::exception &)
		  {
		  }
	  }

	  void LoadNext(CImageLoad *pInfo)
	  {
		  bool bCanSeek = true;
		  bool bWasLoaded = pInfo->WasImageLoaded();

		  if (!pInfo->_bWasStopped && !_bStopped)
		  {
			  if (pInfo->_bIsSlideShow)
			  {
				  if (_bSlideShow)
				  {
					  _nSlideShowItem = pInfo->_nLoadItem;
					  _bSlideShowFoundImages = _bSlideShowFoundImages || bWasLoaded;

					  long nCount = _items.GetSize();

					  if (nCount)
					  {
						  int nSeek = (pInfo->_nLoadItem + pInfo->_nDirection);
						  if (nSeek < 0) nSeek = nCount - 1;
						  if (nSeek >= nCount) nSeek = 0;

						  if (pInfo->_nStartItem == nSeek)
						  {
							  SetPlayFlag(App.Options.m_bRepeat && _bSlideShowFoundImages);
						  }

						  if (_bPlay || !bWasLoaded)
						  {
							  IW::RefPtr<CSlideShowItem> pSSI = _items[nSeek];
							  IW::RefPtr<CImageLoad> pNextInfo = new CImageLoad(pSSI->_path, 
								  true, pSSI->_bSignalEvent, pInfo->_nStartItem, nSeek,
								  pInfo->_nDirection, bWasLoaded ? true : pInfo->_bDelay, false);		

							  _pCoupling->ShowImage(pNextInfo);
						  }
					  }
				  }
			  }
			  else if (!bWasLoaded && pInfo->_nDirection != 0)
			  {
				  long nCount = _pItems->GetItemCount();

				  if (nCount)
				  {
					  int nSeek = (pInfo->_nLoadItem + pInfo->_nDirection);
					  if (nSeek < 0) nSeek = nCount - 1;
					  if (nSeek >= nCount) nSeek = 0;

					  if (pInfo->_nStartItem != nSeek)
					  {
						  IW::RefPtr<CImageLoad> pNextInfo = new CImageLoad(_pItems->GetItemPath(nSeek), 
							  false, pInfo->_bSignalEvent, pInfo->_nStartItem, nSeek, 
							  pInfo->_nDirection, false, false);		

						  _pCoupling->ShowImage(pNextInfo);
					  }
				  }
			  }
		  }		
	  }

	  void LoadNextImage(int nStep)
	  {
		  CLoadAny loader(_plugins);
		  _bStopped = false;

		  long nCount = _pItems->GetItemCount();
		  int nSeek = _pItems->GetFocusItem() + nStep;

		  for(int i = 0; i < nCount; i++)
		  {
			  if (nSeek < 0) nSeek = nCount - 1;
			  if (nSeek >= nCount) nSeek = 0;				  

			  CImageLoad info(_pItems->GetItemPath(nSeek), false, true, nSeek, nSeek, nStep, false, false);

			  if (info.Load(loader, IW::CNullStatus::Instance))
			  {
				  OnLoadComplete(&info);
				  return;
			  }

			  nSeek += nStep;
		  }
	  }

	  void Reload()
	  {
		  CLoadAny loader(_plugins);
		  CImageLoad info(_path, false, true, 0, 0, 0, false, false);
			
		  if (info.Load(loader, IW::CNullStatus::Instance))
		  {
			  _bStopped = false;
			  OnLoadComplete(&info);
			  return;
		  }
	  }

	  void SaveAndMoveNext()
	  {
		  CWaitCursor wait;

		  if (Save())
		  {
			  LoadNextImage(1);
		  }
	  }
	
	  void SaveAndMovePrevious()
	  {
		  CWaitCursor wait;

		  if (Save())
		  {
			  LoadNextImage(-1);
		  }
	  }

	  void ResetHistory()
	  {
		  _bDirty = false;
		  _arUndo.RemoveAll();
	  }	

	  void StepImage(int nStep, bool bDelay = false)
	  {
		  _bStopped = false;

		  if (_bSlideShow)
		  {
			  CheckImageList();

			  long nCount = _items.GetSize();
			  long nFocusItem = _nSlideShowItem;

			  if (nCount)
			  {
				  int nSeek = (nFocusItem + nStep);
				  if (nSeek < 0) nSeek = nCount - 1;
				  if (nSeek >= nCount) nSeek = 0;

				  IW::RefPtr<CSlideShowItem> pSSI = _items[nSeek];
				  IW::RefPtr<CImageLoad> pNextInfo = new CImageLoad(pSSI->_path, 
					  true, pSSI->_bSignalEvent, nSeek, nSeek, 
					  nStep, bDelay, false);		

				  _pCoupling->ShowImage(pNextInfo);
			  }
		  }
		  else
		  {
			  long nCount = _pItems->GetItemCount();

			  if (nCount)
			  {
				  int nSeek = (_pItems->GetFocusItem() + nStep);
				  if (nSeek < 0) nSeek = nCount - 1;
				  if (nSeek >= nCount) nSeek = 0;

				  IW::RefPtr<CImageLoad> pNextInfo = new CImageLoad(_pItems->GetItemPath(nSeek), false, true, nSeek, nSeek, nStep, false, false);
				  _pCoupling->ShowImage(pNextInfo);
			  }
		  }
	  }

	  void Home()
	  {
		  if (_bSlideShow)
		  {
			  CheckImageList();
			  _nSlideShowItem = _items.GetSize() - 1;
		  }
		  else
		  {
			  long nCount = _pItems->GetItemCount();

			  if (nCount)
			  {
				  _pItems->SetFocusItem(nCount - 1, true);
			  }
		  }
	  }

	  void End()
	  {
		  if (_bSlideShow)
		  {
			  CheckImageList();
			  _nSlideShowItem = 0;
		  }
		  else
		  {
			  _pItems->SetFocusItem(0, true);
		  }

		  PreviousImage();
	  }

	  void PreviousImage()
	  {
		  Pause();
		  StepImage(-1);
	  }	

	  void NextImage()
	  {
		  Pause();
		  StepImage(1);
	  }

	  void ReloadFileList()
	  {
		  _bSlideShowFoundImages = false;
		  _nSlideShowItem = -1;
		  _bHasFileList = false;
		  _items.RemoveAll();

		  CheckImageList();
	  }

	  void ResetFolderDetails()
	  {
		  _bSlideShowFoundImages = false;
		  _nSlideShowItem = -1;
		  _bHasFileList = false;
		  _items.RemoveAll();
	  }

	  void CheckImageList()
	  {
		  if (!_bHasFileList)
		  {
			  ITEMLIST itemList = _pItems->GetItemList();

			  GetItemList(itemList, true);
			  _bHasFileList = true;
		  }
	  }

	  bool GetItemList(ITEMLIST &itemList, bool bReset)
	  {
		  CWaitCursor wait;

		  if (bReset)
		  {
			  _bSlideShowFoundImages = false;
			  _nSlideShowItem = -1;
		  }

		  long nCount = itemList.size();
		  long nFocusItem = _pItems->GetFocusItem();

		  if (nCount == 0)
			  return false;

		  CSimpleArray<CString> folders;
		  _items.RemoveAll();	

		  for(ITEMLIST::iterator i = itemList.begin(); i != itemList.end() && IW::GetMainWindow(); i++)
		  {
			  IW::FolderItemPtr pItem = (*i); 
			  CString strItemPath = pItem->GetFilePath();

			  if (pItem->IsFolder())
			  {
				  folders.Add(strItemPath);
			  }
			  else
			  {
				  IW::RefPtr<CSlideShowItem> pSSI = new IW::RefObj<CSlideShowItem>;
				  pSSI->_path = strItemPath;
				  pSSI->_bSignalEvent = true;
				  AddItem(pSSI);

				  int n = i - itemList.begin();

				  if (nFocusItem == n)
				  {
					  _nSlideShowItem = _items.GetSize() - 1;
				  }				
			  }
		  }

		  if (App.Options.m_bRecursSubFolders)
		  {
			  int nStep = 0;

			  CProgressDlg pd;	
			  pd.Create(IW::GetMainWindow(), IDS_BUILDING_FILE_LIST);
			  int nSize = folders.GetSize();

			  for (int i=0; i < nSize && !pd.QueryCancel() && IW::GetMainWindow(); i++)
			  {
				  RecursFiles(folders[i]);
				  pd.Progress(nStep++, nSize);
			  }

			  pd.End();
		  }

		  return true;
	  }

	  void AddItem(IW::RefPtr<CSlideShowItem> pSSI)
	  {
		  if (!App.Options.m_bShuffle || _items.GetSize() < 3)
		  {
			  _items.Add(pSSI); 
		  }
		  else
		  {
			  int i = MulDiv(rand(), _items.GetSize() - 2, RAND_MAX) + 1;
			  IW::RefPtr<CSlideShowItem> pSSI2 = _items[i];
			  _items.Add(pSSI2);
			  _items[i] = pSSI;
		  }	
	  }

	  bool IsValidFile(CString strFileName) const
	  {
		  return strFileName != ".." &&
			  strFileName != ".";
	  }

	  void RecursFiles(CString strFolder)
	  {
		  WIN32_FIND_DATA findData;
		  CString strSearch = IW::Path::Combine(strFolder, _T("\\*.*"));
		  HANDLE hSearch = FindFirstFile(strSearch, &findData);

		  if (hSearch != INVALID_HANDLE_VALUE) 
		  {
			  do
			  {
				  if (IsValidFile(findData.cFileName))
				  {
					  CString strFilePath = IW::Path::Combine(strFolder, findData.cFileName);

					  if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					  {
						  RecursFiles(strFilePath);
					  }
					  else
					  {
						  IW::RefPtr<CSlideShowItem> pSSI = new IW::RefObj<CSlideShowItem>;
						  pSSI->_path = strFilePath;
						  AddItem(pSSI);
					  }
				  }
			  }
			  while(FindNextFile(hSearch, &findData)); 

			  FindClose(hSearch);
		  }
	  }

	  void RenderImageForAnimation()
	  {
		  _imageRendered.Free();

		  const IW::Image &image = GetImage();

		  if (image.NeedRenderForDisplay())
		  {
			  IW::Image imageRendered;

			  if (GetImage().Render(imageRendered))
			  {
				  _imageRendered = imageRendered;
			  }
		  }
	  }

	  void SetImage(IW::Image &image, const CString &strImageFileName)
	  {
		  ResetHistory();

		  if (IW::GetMainWindow())
		  {
			  IW::Focus preserveFocus;
			  Fade fade(_pCoupling->GetImageWindow());

			  SetImage(image);
			  UpdateFileInfo(strImageFileName);			  
			  OnNewImage(true);

			  if (App.Options.m_bUseEffects)
			  {
				  fade.FadeIn();
			  }	
		  }
	  }

	  void UpdateFileInfo(const CString &strImageFileName)
	  {
		  _path = strImageFileName;
		  _ft = IW::FileTime::FromFile(strImageFileName);
		  _size = IW::FileSize::FromFile(strImageFileName);
	  }

	  void SetImage(IW::Image &image)
	  {
		  _image = image;
	  }

	  void OnNewImage(bool bScrollToCenter)
	  {
		  _histogram.Clear();
		  _image.GetHistogram(_histogram);

		  RenderImageForAnimation();
		  CreateThumbnail();
		  
		  _pCoupling->NewImage(bScrollToCenter);		  
	  }

	  void CreateThumbnail()
	  {
		  _imageThumbnail.Free();
		  if (!_image.IsEmpty()) _imageThumbnail = IW::CreatePreview(GetRenderImage(), CSize(160, 160));
	  }

	  bool HasFlickrUrl() const
	  {
		  return !_image.GetFlickrId().IsEmpty();
	  }

	  void SetImageWithHistory(IW::Image &image, const CString &strAction)
	  {
		  IW::Image imageOld = GetImage();

		  SetImage(image);

		  if (_arUndo.GetSize() > 10)
		  {
			  _arUndo.RemoveAt(0);
		  }

		  if (!imageOld.IsEmpty())
		  {
			  _arUndo.Add(new IW::RefObj<CUndo>(imageOld, strAction, _bDirty));
		  }

		  OnNewImage(false);
		  _bDirty = true;
	  }

	  void Undo()
	  {
		  if (_arUndo.GetSize())
		  {
			  int nTop = _arUndo.GetSize() - 1;
			  CUndo *pUndo = _arUndo[nTop];

			  IW::CMessageBoxIndirect mb;
			  if (IDOK == mb.Show(IW::Format(IDS_QUERYUNDO, pUndo->GetAction()), MB_ICONQUESTION | MB_OKCANCEL | MB_HELP))
			  {	
				  pUndo->GetImageState(_image, _bDirty);
				  _arUndo.RemoveAt(nTop);
				  OnNewImage(false);
				  UpdateStatusText(); 
			  }
		  }
	  }

	  bool SaveFile(const IW::Image &image, const CString &strFileName, const CString &strType, IW::IStatus *pStatus)
	  {
		  bool bSucceeded = false;

		  IW::CFileTemp f;
		  if (f.OpenForWrite(strFileName))
		  {
			  CLoadAny loader(_plugins);
			  bSucceeded = loader.Write(strType, &f, image, pStatus) && f.Close(pStatus);	
		  }	

		  return bSucceeded;
	  }

	  bool Save()
	  {
		  const CString strImageName = GetImageFileName();
		  const IW::Image &image = GetImage();
		  const CString strKey = image.GetLoaderName();

		  if (strKey.IsEmpty())
		  {
			  IW::CMessageBoxIndirect mb;
			  if (IDOK == mb.Show(IDS_INVALIDSAVEFORMAT, MB_ICONQUESTION | MB_OKCANCEL | MB_HELP))
			  {
				  return SaveAs();
			  }
			  else 
			  {
				  return false;
			  }
		  }

		  return Save(strImageName, strKey);
	  }

	  bool Save(const CString &strFilePath, const CString &strKey)
	  {
		  if (!IW::CFilePath::CheckFileName(strFilePath))
		  {
			  CString str;
			  str.Format(IDS_INVALID_FILE, strFilePath);

			  IW::CMessageBoxIndirect mb;
			  mb.Show(str);

			  return false;
		  }

		  const IW::IImageLoaderFactoryPtr pFactory = _plugins.GetImageLoaderFactory(strKey);
		  const IW::Image &image = GetImage();
		  const bool bHasMetaData = image.HasMetaData(IW::MetaDataTypes::PROFILE_EXIF) || image.HasMetaData(IW::MetaDataTypes::PROFILE_IPTC);
		  const bool bSupportMetaData = (pFactory->GetFlags() & IW::ImageLoaderFlags::METADATA)!=0;

		  IW::CMessageBoxIndirect mb;
		  if (bHasMetaData &&
			  !bSupportMetaData &&
			  IDOK != mb.Show(IDS_DOESNOT_SUPPORT_METADATA, MB_ICONQUESTION | MB_OKCANCEL | MB_HELP))
		  {
			  return false;
		  }

		  bool bRet = SaveFile(image, strFilePath, strKey, IW::CNullStatus::Instance);

		  if (bRet)
		  {
			  _bDirty = false;
			  UpdateFileInfo(strFilePath);
			  UpdateStatusText();
		  }
		  else
		  {
			  mb.ShowOsErrorWithFile(strFilePath, IDS_FAILEDTO_CREATE_FILE);
		  }

		  return bRet;
	  }

	  bool SaveAs()
	  {
		  CImageFileDialog dlg(_plugins, FALSE, NULL, GetImageFileName(), 0);
		  dlg.SetDefaults(GetImage());	

		  if(dlg.DoModal() == IDOK)
		  {
			  return Save(dlg.m_ofn.lpstrFile, dlg.GetLoaderType());
		  }

		  return false;
	  }


	  void UpdateStatusText()
	  {
		  CString str;

		  if (IsImageShown())
		  {
			  const IW::Image &image = GetImage();
			  IW::Page page = image.GetFirstPage();
			  CString originalFileName = GetImageFileName();

			  if (IsDirty())
			  {
				  str.Format(IDS_FMT_EDITING, IW::Path::FindFileName(originalFileName),
					  page.GetWidth(),
					  page.GetHeight(),
					  page.GetPixelFormat().ToBpp());
			  }
			  else
			  {
				  str.Format(IDS_FMT_VIEWING, IW::Path::FindFileName(originalFileName), image.GetStatistics());
			  }

			  if (GetImage().HasMetaData(IW::MetaDataTypes::JPEG_IMAGE))
			  {
				  str += g_szSpace;
				  str += App.LoadString(IDS_NOLOSS);
			  }


			  int nPageCount = image.GetPageCount();

			  if (nPageCount > 1)
			  {
				  CString strPages;
				  strPages.Format(_T("%d Pages"), nPageCount);

				  str += g_szSpace;
				  str += strPages;
			  }

			  _pCoupling->SetStatusText(str);
		  }

	  }

	  bool QuerySave();

	  CString GetTitle() const
	  {
		  CString strTitle = _image.GetTitle();

		  if (strTitle.IsEmpty()) 
		  {
			  CString originalFileName = GetImageFileName();
			  IW::CFilePath path = IW::Path::FindFileName(originalFileName);
			  path.StripToFilename();
			  strTitle = path.ToString();
		  }

		  return strTitle;
	  }

	  CString GetTags() const
	  {
		  return _image.GetTags();		
	  }

	  CString GetDescription() const
	  {
		  CString strDescription = _image.GetDescription();	
		  strDescription.Replace(g_szCRLF, g_szCR);
		  strDescription.Replace(g_szLF, g_szCR);
		  strDescription.Replace(g_szCR, g_szCRLF);
		  return strDescription;
	  }

	  CString GetFlickrPhotoId() const
	  {
		  return _image.GetFlickrId();
	  }

	  CString GetTaken() const
	  {
		  return g_szEmptyString;
	  }

	  void Copy()
	  {
		  if (IsImageShown())
		  {
			  CComPtr<CImageDataObject> p = IW::CreateComObject<CImageDataObject>();
			  p->Cache(GetImage());
			  OleSetClipboard(p);
		  }
	  }

	  void OnEditCopy() 
	  {
		  // Clean clipboard of contents, and copy the DIB.
		  if (OpenClipboard(IW::GetMainWindow()))
		  {
			  EmptyClipboard();
			  HGLOBAL hMem = GetImage().CopyToHandle();
			  SetClipboardData (CF_DIB, hMem);
			  CloseClipboard();
		  }
	  }	

	  void Unload()
	  {
		  IW::Image image;
		  SetImage(image, g_szEmptyString);		
	  }

	  void SetImageEditMode(ImageEditMode::Mode mode)
	  {
		  _editMode = mode;
		  CreateThumbnail();
		  EditModeDelegates.Invoke();		  
	  }

	  bool IsImageEditMode() const
	  { 
		  return _editMode != ImageEditMode::None;
	  }
	  
	  bool IsImageEditMode(ImageEditMode::Mode mode) const
	  { 
		  return _editMode == mode;
	  }
};