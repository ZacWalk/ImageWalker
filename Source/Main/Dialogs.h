///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


#pragma once

#include <ColorButton.h>
#include "ImageStreams.h"

class PluginState;

namespace IW
{

////////////////////////////////////////////////////////////////////////////
// Focus helper

////////////////////////////////////////////////////////////////////////////
// Resize

template<class T>
class CDialogResizer
{
protected:
   CRect _rectOriginal;

   class CDialogItem
   {
   public:
      CDialogItem(const UINT nItemId,
         const UINT nFlags,
            const CRect &rect) :
         _id(nItemId),
         _flags(nFlags),
         _rect(rect)
      {
      }

	  const int Width() const
	  {
		  return _rect.right - _rect.left;
	  }

      UINT _id;
      UINT _flags;
      CRect _rect;
   };

   CSimpleArray<CDialogItem> _items;
   bool _bInit;
public:

   enum { 
      eLeft = 0x01,
      eRight = 0x02,
      eTop = 0x04,
      eBottom = 0x08,
	  eAlignBottom = 0x10
   };

   CDialogResizer()
   {
	   _bInit = false;
   }

   void _DialogInit(HWND hWnd)
   {
      //T *pT = static_cast<T*>(this);
      ::GetClientRect(hWnd, &_rectOriginal);
   }

   void ResizeAddItem(UINT nItemId, UINT nFlags)
   {
      T *pT = static_cast<T*>(this);

	  HWND hWndCtrl = pT->GetDlgItem(nItemId);

	  // Is this really a dialog item?
      ATLASSERT(hWndCtrl);

      CRect r;
      ::GetWindowRect(hWndCtrl, &r);

	  HWND hWndParent = GetParent(hWndCtrl);
      ::ScreenToClient(hWndParent, (LPPOINT)&r);
	  ::ScreenToClient(hWndParent, (LPPOINT)&r + 1);

      _items.Add(
         CDialogItem(
            nItemId,
            nFlags,
            r));

	  if (!_bInit)
	  {
		  _DialogInit(hWndParent);
		  _bInit = false;
	  }
   }

   void ResizeDialog()
   {
      T *pT = static_cast<T*>(this);
	  
	  if (_items.GetSize())
	  {
		  // turn on WS_CLIPCHILDREN
		  // This will stop all that flicker
		  //pT->ModifyStyle(0, WS_CLIPCHILDREN);

		  // We will be deferring four windows.
		  HDWP hdwp = ::BeginDeferWindowPos(_items.GetSize());

		  if (hdwp == NULL)
			  return;

		  CRect rectClient;
		  pT->GetClientRect(&rectClient);

		  for(int i = 0; i < _items.GetSize(); i++)
		  {
			 CRect r = _items[i]._rect;

			 if (_items[i]._flags & eAlignBottom)
			 {
				 r.top = (_items[i]._rect.top - _rectOriginal.bottom) + rectClient.bottom;
				 r.bottom = (_items[i]._rect.bottom - _rectOriginal.bottom) + rectClient.bottom;
			 }

			 switch(_items[i]._flags & 0x0f)
			 {
			 case eRight:
				r.right = (_items[i]._rect.right - _rectOriginal.right) + rectClient.right;
				r.left = r.right - _items[i].Width();
				break;

			 case eLeft:
				r.left = (_items[i]._rect.left - _rectOriginal.left) + rectClient.left;
				r.right = r.left + _items[i].Width();
				break;

			 case eLeft | eRight:
				r.right = (_items[i]._rect.right - _rectOriginal.right) + rectClient.right;
				r.left = (_items[i]._rect.left - _rectOriginal.left) + rectClient.left;
				break;

			 case eLeft | eRight | eBottom:
				r.right = (_items[i]._rect.right - _rectOriginal.right) + rectClient.right;
				r.left = (_items[i]._rect.left - _rectOriginal.left) + rectClient.left;
				r.bottom = (_items[i]._rect.bottom - _rectOriginal.bottom) + rectClient.bottom;
				break;

			case eBottom:
				r.bottom = (_items[i]._rect.bottom - _rectOriginal.bottom) + rectClient.bottom;
				break;

			case 0:
				break; // Nothing to do


			 default:
				// Need to impliment this 
				// sizing case?
				ATLASSERT(0);
			 }
         
			 ::DeferWindowPos(hdwp, 
				 pT->GetDlgItem(_items[i]._id),
				NULL,
				r.left,
				r.top,
				r.right - r.left,
				r.bottom - r.top,
				SWP_NOZORDER);
		  }

		  ::EndDeferWindowPos(hdwp);

		  // force repaint now
		  //pT->UpdateWindow();
		  
		  // turn off WS_CLIPCHILDREN
		  //pT->ModifyStyle(WS_CLIPCHILDREN, 0);
	  }
   }

};

template<class T>
class CDialogResizerWithGripper : public CDialogResizer<T>
{
protected:
	bool _bInit;

public:
	CDialogResizerWithGripper()
	{
		_bInit = false;
	}

	void GripperInit()
	{
		T *pT = static_cast<T*>(this);

		CRect rectDlg;
		pT->GetClientRect(&rectDlg);

		// shouldn't exist already
		ATLASSERT(!::IsWindow(pT->GetDlgItem(ATL_IDW_STATUS_BAR)));
		if(!::IsWindow(pT->GetDlgItem(ATL_IDW_STATUS_BAR)))
		{
			CWindow wndGripper;
			wndGripper.Create(_T("SCROLLBAR"), pT->m_hWnd, rectDlg, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SBS_SIZEBOX | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN, 0, ATL_IDW_STATUS_BAR);
			ATLASSERT(wndGripper.IsWindow());
			if(wndGripper.IsWindow())
			{
				_bInit = true;
				CRect rectCtl;
				wndGripper.GetWindowRect(&rectCtl);
				::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rectCtl, 2);

				ResizeAddItem(ATL_IDW_STATUS_BAR, eAlignBottom | eRight);
			}
		}		
	}

	void ResizeDialog(WPARAM wParam)
	{
		T *pT = static_cast<T*>(this);

		if (!_bInit) GripperInit();

		CWindow wndGripper = pT->GetDlgItem(ATL_IDW_STATUS_BAR);
		if(wParam == SIZE_MAXIMIZED)
			wndGripper.ShowWindow(SW_HIDE);
		else if(wParam == SIZE_RESTORED)
			wndGripper.ShowWindow(SW_SHOW);

		CDialogResizer<T>::ResizeDialog();
	}

};

////////////////////////////////////////////////////////////////////////////
// Properties

template<int tID>
class CPropertyAddDlg : 
	public CDialogImpl<CPropertyAddDlg<tID> >,
	public IW::IImageMetaDataClient
{
public:
	CPropertyAddDlg()
	{
		_nProperty = 0;
		_bAssending = 0;
	}

	enum { IDD = tID };
	long _nProperty;
	bool _bAssending;

	BEGIN_MSG_MAP(CPropertyAddDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());

		HWND hwndCombo = GetDlgItem(IDC_PROPERTY);

		// Set the image list for the combo box.
		SendMessage(hwndCombo , CBEM_SETIMAGELIST, 0L, (LPARAM)(HIMAGELIST)App.GetGlobalBitmap());


		App.IterateMetaDataTypes(this);

		int nCount = SendMessage(hwndCombo , CB_GETCOUNT, 0, 0); 
		
		for(int i = 0; i < nCount; i++)
		{
			int nId = SendMessage(hwndCombo, CB_GETITEMDATA, i, 0L);
			
			if (_nProperty == nId)
			{
				SendMessage(hwndCombo , CB_SETCURSEL, i, 0);
				break;
			}
		}		

		CheckDlgButton(IDC_ASSENDING, _bAssending ? BST_CHECKED : BST_UNCHECKED);
		

        bHandled = false;
		return (LRESULT)TRUE;
	}
	
	bool AddMetaDataType(DWORD dwId, const CString &strTitle)
	{
		HWND hCombo = GetDlgItem(IDC_PROPERTY);
		TCHAR sz[200];

		// We need to sort!!
		int nCount = SendMessage(hCombo , CB_GETCOUNT, 0, 0); 
		int nPos = -1;
		
		for(int i = 0; i < nCount; i++)
		{
			SendMessage(hCombo, CB_GETLBTEXT, (WORD)i, (LONG)sz);
			
			if (_tcsicmp(sz, strTitle) > 0)
			{
				nPos = i;
				break;
			}
		}
		
		COMBOBOXEXITEM cbI;
		
		// Each item has text, an lParam with extra data, and an image.
		cbI.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;    
		cbI.pszText = (LPTSTR)(LPCTSTR)strTitle;
		cbI.cchTextMax = strTitle.GetLength();
		cbI.lParam = dwId;
		cbI.iItem = nPos;          // Add the item to the end of the list.
		cbI.iSelectedImage = cbI.iImage = ImageIndex::Property;
		
		// Add the item to the combo box drop-down list.
		SendMessage(hCombo, CBEM_INSERTITEM, 0L,(LPARAM)&cbI);

		return true;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		HWND hCombo = GetDlgItem(IDC_PROPERTY);

		int n = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
		_nProperty = ::SendMessage(hCombo, CB_GETITEMDATA, n, 0);
		_bAssending  = BST_CHECKED == IsDlgButtonChecked(IDC_ASSENDING);

		EndDialog(wID);
		return 0;
	}
};



/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg

template <class T>
class CPropertyDlgImpl : 
	public IW::IImageMetaDataClient
{
public:
	CPropertyDlgImpl()
	{
		_nProperty = 0;
	}

	~CPropertyDlgImpl()
	{
	}

	int InsertItem(HWND hLV, int nId, int nLoc = -1)
	{
		if (nLoc == -1)
			nLoc = ListView_GetItemCount( hLV );

        LVITEM lvItem;
		IW::MemZero(&lvItem,sizeof(LVITEM));

        lvItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
        lvItem.iItem = nLoc;
        lvItem.iSubItem = 0;
        lvItem.pszText = (LPTSTR)(LPCTSTR)m_mapNames[nId];
        lvItem.iImage = ImageIndex::Property;
		lvItem.lParam = nId;

        return ListView_InsertItem(hLV,&lvItem);

	}

	std::map<long, CString> m_mapNames;
	int _nProperty;

	

BEGIN_MSG_MAP(CPropertyDlgImpl)

	COMMAND_ID_HANDLER(IDC_ADD, OnAdd)
	COMMAND_ID_HANDLER(IDC_REMOVE, OnRemove)
	COMMAND_ID_HANDLER(IDC_MOVE_UP, OnMoveUp)
	COMMAND_ID_HANDLER(IDC_MOVE_DOWN, OnMoveDown)

END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void OnInitProperties(IW::CArrayDWORD *pAnnotations)
	{
		T *pT = static_cast<T*>(this);	

		// Get list of anotation names
		App.IterateMetaDataTypes(this);

		HWND hLV = pT->GetDlgItem(IDC_LIST);
        ListView_SetImageList(hLV, App.GetGlobalBitmap() ,LVSIL_SMALL);

		
        OnRevertProperties(pAnnotations);
	}

	bool AddMetaDataType(DWORD dwId, const CString &strTitle)
	{
		m_mapNames[dwId] = strTitle;
		return true;
	}

	void OnRevertProperties(IW::CArrayDWORD *pAnnotations)
    {
		T *pT = static_cast<T*>(this);

		// Assign image lists to control
		HWND hLV = pT->GetDlgItem(IDC_LIST);
		ListView_DeleteAllItems(hLV);

		for(int i = 0; i < pAnnotations->GetSize(); i++)
		{
			InsertItem(hLV, (*pAnnotations)[i]);
		}
	}

	bool OnApplyProperties(IW::CArrayDWORD *pAnnotations)
	{
		T *pT = static_cast<T*>(this);
		pAnnotations->RemoveAll();

		HWND hLV = pT->GetDlgItem(IDC_LIST);
		UINT    ix, cItems = ListView_GetItemCount( hLV );

		for ( ix = 0; ix < cItems; ix++ )
		{
			ListViewItem lvi(ix);
			
			if ( ListView_GetItem( hLV, &lvi ) )
			{
				pAnnotations->Add(lvi.lParam);
			}
		}
		
		return true;
	}

	LRESULT OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T *pT = static_cast<T*>(this);
		CPropertyAddDlg<IDD_PROPERTY_ADD> dlg;
		dlg._nProperty = _nProperty;
		
		if (IDOK == dlg.DoModal())
		{
			_nProperty = dlg._nProperty;

			HWND hLV = pT->GetDlgItem(IDC_LIST);
			InsertItem(hLV, _nProperty);

			// todo
			pT->OnChange();
		}

		return 0;
	}

	

	LRESULT OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T *pT = static_cast<T*>(this);
		HWND hLV = pT->GetDlgItem(IDC_LIST);
		int n;
		
		while (-1 != (n = ListView_GetNextItem( hLV, -1, LVNI_SELECTED )))
		{
			ListView_DeleteItem(hLV, n);
		}

		pT->OnChange();

		return 0;
	}

	LRESULT OnMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T *pT = static_cast<T*>(this);
		HWND hLV = pT->GetDlgItem(IDC_LIST);
		int n = ListView_GetSelectionMark(hLV);

		if (n > 0)
		{
			ListViewItem lvi(n);
			
			if ( ListView_GetItem( hLV, &lvi ) )
			{
				ListView_DeleteItem(hLV, n);
				InsertItem(hLV, lvi.lParam, n - 1);
				ListView_SetItemState (hLV,  n - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				ListView_SetSelectionMark(hLV,  n - 1);
				
				// todo
				pT->OnChange();
				
			}
		}

		return 0;
	}

	LRESULT OnMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T *pT = static_cast<T*>(this);
		HWND hLV = pT->GetDlgItem(IDC_LIST);
		int n = ListView_GetSelectionMark(hLV);
		int nCount = ListView_GetItemCount (hLV);

		if (n < nCount - 1)
		{
			ListViewItem lvi(n);
			
			if ( ListView_GetItem( hLV, &lvi ) )
			{
				ListView_DeleteItem(hLV, n);
				InsertItem(hLV, lvi.lParam, n + 1);
				ListView_SetItemState (hLV,  n + 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				ListView_SetSelectionMark(hLV,  n + 1);
				
				// todo
				pT->OnChange();
			}
		}

		return 0;
	}
};


template <class T>
class CImageLoaderDlgImpl
{
public:

	CComboBoxEx _combo;
	int _nDefaultSelection;
	CString _strDefaultSelection;

	PluginState &_plugins;

	// Loaders and Filters
	IW::IImageLoaderFactoryPtr m_pLoaderFactory;
	IW::RefPtr<IW::IImageLoader> m_pLoader;

	CImageLoaderDlgImpl(PluginState &plugins) : _plugins(plugins)
	{
	}

	~CImageLoaderDlgImpl() 
	{
	};


BEGIN_MSG_MAP(CImageLoaderDlgImpl<T>)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_COMBOBOXEX, CBN_SELENDOK, OnImageLoaderChange)
	COMMAND_ID_HANDLER(IDC_OPTIONS, OnOptions)

END_MSG_MAP()

	void OnImageLoaderChange()
	{
		T *pT = static_cast<T*>(this);

		int nDefaultSelection = _combo.GetCurSel();
		IW::IImageLoaderFactoryPtr pFactory = (IW::IImageLoaderFactoryPtr)_combo.GetItemData(nDefaultSelection);
				
		EnableWindow(pT->GetDlgItem(IDC_OPTIONS), (IW::ImageLoaderFlags::OPTIONS & pFactory->GetFlags()));

		OnChangedLoader();
		pT->OnChange();
	}

	LRESULT OnImageLoaderChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		OnImageLoaderChange();
		return 0;
	}

	LRESULT OnOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T *pT = static_cast<T*>(this);

		// Just in case change event failed
		OnChangedLoader();
		m_pLoader->DisplaySettingsDialog(IW::Image());
		return 0;
	}	


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);

		_combo = pT->GetDlgItem(IDC_COMBOBOXEX);

		_combo.SetImageList(App.GetGlobalBitmap());
		_nDefaultSelection = 0;
		
		// Serialize
		CPropertyArchiveRegistry archive(App.GetRegKey());
		
		if (archive.StartSection(pT->GetPropertySection()))
		{
			archive.Read(g_szLoader, _strDefaultSelection);
		}

		_plugins.IterateImageLoaders(this);
		_combo.SetCurSel(_nDefaultSelection);
		OnImageLoaderChange();

        bHandled = false;
		return 0;
	}

	void OnChange()
	{
	}

	void OnChangedLoader()
	{
		T *pT = static_cast<T*>(this);

		_nDefaultSelection = _combo.GetCurSel();
		IW::IImageLoaderFactoryPtr pNewFactory = (IW::IImageLoaderFactoryPtr)_combo.GetItemData(_nDefaultSelection);
		
		// Get ready for a new filter
		if (pNewFactory != m_pLoaderFactory)
		{
			m_pLoaderFactory = pNewFactory;
			m_pLoader = 0;
		}
		
		if (m_pLoader == 0)
		{
			m_pLoader = m_pLoaderFactory->CreatePlugin();
			
			// Serialize
			CPropertyArchiveRegistry archive(App.GetRegKey());
			
			if (archive.StartSection(pT->GetPropertySection()))
			{
				if (archive.StartSection(m_pLoaderFactory->GetKey()))
				{
					m_pLoader->Read(&archive);
					archive.EndSection();
				}
				archive.EndSection();
			}
		}
	}

	bool OnApplyLoader()
	{
		OnChangedLoader();

		if (m_pLoader)
		{
			T *pT = static_cast<T*>(this);

			// Serialize
			CPropertyArchiveRegistry archive(App.GetRegKey());
			
			if (archive.StartSection(pT->GetPropertySection()))
			{
				archive.Write(g_szLoader, m_pLoaderFactory->GetKey());

				if (archive.StartSection(m_pLoaderFactory->GetKey()))
				{
					m_pLoader->Write(&archive);
					archive.EndSection();
				}
				archive.EndSection();
			}
		}
		
		return true;
	}
	
	
	// IImageLoaderFactoryClient
	bool AddLoader(IW::IImageLoaderFactoryPtr pFactory)
	{
		if (IW::ImageLoaderFlags::SAVE & pFactory->GetFlags())
		{
			CString str = pFactory->GetExtensionDefault();
			str += " - ";
			str += pFactory->GetTitle();
			
			COMBOBOXEXITEM cbI;
			
			// Each item has text, an lParam with extra data, and an image.
			cbI.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;    
			cbI.pszText = (LPTSTR)(LPCTSTR)str;
			cbI.cchTextMax = str.GetLength();
			cbI.lParam = (LPARAM)pFactory;
			cbI.iItem = -1;          // Add the item to the end of the list.
			cbI.iSelectedImage = cbI.iImage = ImageIndex::Loader;
			
			// Add the item to the combo box drop-down list.
			int i = _combo.InsertItem(&cbI);

			if (_tcsicmp(pFactory->GetKey(), _strDefaultSelection) == 0)
			{
				_nDefaultSelection = i;
			}
		}
		
		return true;
	}
	
};

///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg


template<class TDialog, class TParent>
class CDialogHolder : public TDialog
{
public:

	TParent *_pParent;	

	CDialogHolder(TParent *pParent) : _pParent(pParent)
	{
	}

	template<class TInjection>
	CDialogHolder(TParent *pParent, TInjection &injectThis) : TDialog(injectThis), _pParent(pParent)
	{		
	}

BEGIN_MSG_MAP(CDialogHolder)

	COMMAND_CODE_HANDLER(EN_SETFOCUS, OnSetFocus)
	COMMAND_CODE_HANDLER(CBN_SETFOCUS, OnSetFocus)
	COMMAND_CODE_HANDLER(BN_SETFOCUS, OnSetFocus)
	COMMAND_CODE_HANDLER(LBN_SETFOCUS, OnSetFocus)

	CHAIN_MSG_MAP(TDialog)

ALT_MSG_MAP(1)
	
	CHAIN_MSG_MAP_ALT(TDialog, 1)

END_MSG_MAP()

	LRESULT OnSetFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		_pParent->OnDlgItemFocus(wID);
		bHandled = FALSE;
		return 0;
	};
};

template<class TDialog>
class CDialogScroll :
	public CWindowImpl< CDialogScroll<TDialog> >,
	public CScrollImpl< CDialogScroll<TDialog> >
{
	typedef CDialogScroll	ThisClass;
	typedef CScrollImpl< CDialogScroll > BaseClass;
	
public:
		
	
	CDialogHolder<TDialog, CDialogScroll> m_dialog;
	CRect _rectClient;
	
	CDialogScroll() : m_dwIcon(IDR_MAINFRAME), m_dialog(this)
	{		
	}

	template<class TInjection>
	CDialogScroll(TInjection &injectThis) : m_dwIcon(IDR_MAINFRAME), m_dialog(this, injectThis)
	{		
	}

	TDialog &GetDialog() { return m_dialog; };
	
	bool PreTranslateMessage(MSG* pMsg)
	{
		return m_dialog.PreTranslateMessage(pMsg);
	}

	void OnDlgItemFocus(WORD wID)
	{
		CRect rc;
		
		::GetWindowRect(m_dialog.GetDlgItem(wID), &rc);
		ScreenToClient(&rc);
		//::OffsetRect(&rc, -m_ptOffset.x, -m_ptOffset.y);
		MakeItemVisible(rc);
	}	
	
	void MakeItemVisible(const CRect &rc)
	{
		int y = 0;
		
		if (rc.top <  0)
		{
			y = rc.top;
		}
		else if (rc.bottom > m_sizeClient.cy)
		{
			y = rc.bottom - m_sizeClient.cy;
		}	
		
		if (y)
		{
			SetScrollPos(SB_VERT, m_ptOffset.y + y, TRUE);
			ScrollWindowEx(0, -y, m_uScrollFlags);
			m_ptOffset.y += y;
		}		
	}
	
    DECLARE_WND_CLASS(_T("CDialogScroll"))

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		
		CHAIN_MSG_MAP(BaseClass)

	ALT_MSG_MAP(1)

		CHAIN_MSG_MAP_ALT_MEMBER(m_dialog, 1)
	END_MSG_MAP()
		
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		int cx = LOWORD(lParam);
		int cy = HIWORD(lParam);

		DoSize(cx, cy);

		bHandled = false;
		
		return 0;
	}

	void DoSize()
	{
		CRect rc;
		GetClientRect(&rc);
		DoSize(rc.right - rc.left, rc.bottom - rc.top);
	}

	void DoSize(int cx, int cy)
	{
		
		if (cy)
		{
			CPoint ptOffset = m_ptOffset;
			int nHeight = _rectClient.bottom - _rectClient.top;

			if (nHeight < cy)
			{
				nHeight = cy;
				ptOffset.y = 0;
			}
			else if ((nHeight - ptOffset.y) < cy)
			{
				ptOffset.y = nHeight - cy;
			}
			
			SetScrollSize(1, nHeight);
			SetScrollOffset(ptOffset);  
			
			m_dialog.MoveWindow(0,-ptOffset.y, cx, nHeight);
		}
	}
	
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		return 0;
	}
	
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HICON hIconSmall = (HICON)::LoadImage(App.GetBitmapResourceInstance(), MAKEINTRESOURCE(m_dwIcon), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);
		
		m_dialog.Create(m_hWnd);
		m_dialog.ShowWindow(SW_SHOW);
		m_dialog.GetWindowRect(&_rectClient);

		ModifyStyleEx(0,WS_EX_CONTROLPARENT,0);
		m_dialog.ModifyStyleEx(0,WS_EX_CONTROLPARENT,0);


		DoSize();
		
		return 0;
	}
	
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
	}
	
protected:
	DWORD m_dwIcon;
};

class CDialogScrollFrameOkCancelToolbar
{
public:

	static TBBUTTON* GetToolbarButtons()
	{		
		// Create the toolbar
		static TBBUTTON tbButtonsModes [ ] = 
		{
			{ 2, ID_OK, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0L, 0},
			//{ 4, ID_CANCEL, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0L, 1},		
			{ 0, 0, 0, 0, {0}, 0L, 0}
		};
		
		
		
		return tbButtonsModes;
	}
};

template<class TDialog, class TToolBar = CDialogScrollFrameOkCancelToolbar>
class CDialogScrollFrame : 
    public CFrameWindowImpl< CDialogScrollFrame<TDialog> >
{
	typedef CFrameWindowImpl< CDialogScrollFrame<TDialog> > BaseClass;
	typedef CDialogScrollFrame<TDialog>  ThisClass;

public:
	IW::CDialogScroll<TDialog> m_scroller;
	HWND m_hWndToolBarLocal;

	TDialog &GetDialog() { return m_scroller.m_dialog; };
	const TDialog &GetDialog() const { return m_scroller.m_dialog; };

	bool PreTranslateMessage(MSG* pMsg)
	{
		return m_scroller.m_dialog.PreTranslateMessage(pMsg);
	}

	BEGIN_MSG_MAP(ThisClass)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)

		CHAIN_MSG_MAP(BaseClass)
		CHAIN_MSG_MAP_ALT_MEMBER(m_scroller, 1)		

	ALT_MSG_MAP(1)

		CHAIN_MSG_MAP_ALT_MEMBER(m_scroller, 1)		
	END_MSG_MAP()


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		TCHAR sz[MAX_PATH + 1];
		
		m_hWndClient = m_scroller.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE);
		
		// Create the toolbar

		TBBUTTON *tbButtonsModes = TToolBar::GetToolbarButtons();

		int nButtonCount = 0;
		while(0 != tbButtonsModes[nButtonCount].idCommand)
		{
			nButtonCount++;
		}
		

		m_hWndToolBarLocal = CreateToolbarEx (
			m_hWnd, 
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | 
			CCS_LEFT | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_ADJUSTABLE |
			TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_CUSTOMERASE | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | TBSTYLE_FLAT, 
			99, 
			0, 
			0, 
			0, 
			tbButtonsModes, 
			nButtonCount, 
			0, 0, 32, 32, 
			sizeof (TBBUTTON));
		
		SendMessage(m_hWndToolBarLocal, TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)App.GetGlobalBitmap());
		
		for(int i = 0; i < nButtonCount; i++)
		{
			if (::LoadString(App.GetResourceInstance(), tbButtonsModes[i].idCommand, sz, MAX_PATH))
			{
				LPTSTR szCR = _tcschr(sz, _T('\n'));
				if (szCR) *szCR = 0;
				sz[_tcsclen(sz) + 1] = 0; // double terminate!!
				
				int nIndex = SendMessage(m_hWndToolBarLocal, TB_ADDSTRING, 0, (LPARAM)sz); 
			}
		}

		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		AddSimpleReBarBand(m_hWndToolBarLocal);		
		
		return 0;
	}

};

template<class T>
class CPreviewThread : public IW::IStatus
{
public :

	enum { eMaxSize = 32 };

	T *_pPreview;
	IW::Image _imagePreview;
	IW::Image _imageFiltered;
	HANDLE _hThread;
	bool _bExit;
	bool _bError;
	CCriticalSection _cs;
	CEvent _event;
	CSize _size;

	CPreviewThread(T *pPreview) : 
		_hThread(0), 
		_pPreview(pPreview),
		_bExit(false), 
		_bError(false), 
		_event(FALSE,FALSE),
		_size(100, 100)
	{
	}

	~CPreviewThread() 
	{
		End();
	}

	void Progress(int nCurrentStep, int TotalSteps) {  };
	bool QueryCancel() { return _bExit; }; 
	void SetStatusMessage(const CString &strMessage) { }; 
	void SetHighLevelProgress(int nCurrentStep, int TotalSteps) {  };
	void SetHighLevelStatusMessage(const CString &strMessage) {  };
	void SetMessage(const CString &strMessage) {  };
	void SetWarning(const CString &strWarning) {  };
	void SetError(const CString &strError) {  };
	void SetContext(const CString &strContext) {  };

	bool Create(IW::Image &image, CSize &size)
	{
		_size = size;
		_imagePreview = image;
		Start();
		_event.Set();

		return true; 
	};

	void Start() 
	{
		DWORD id;
		_hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&CPreviewThreadProc,(LPVOID)this,0,&id);
	}

	void End() 
	{
		_bExit = true;

		_event.Set();

		if (_hThread )
		{
			WaitForSingleObject(_hThread,INFINITE);
			CloseHandle(_hThread);
			_hThread = 0;
		}
	}

	void ApplyFilter()
	{
		if (_pPreview && !_imagePreview.IsEmpty())
		{
			IW::Image image, imageToFilter;
			bool bSuccess = false;			

			{
				IW::CAutoLockCS lock(_cs);
				CRect rc = _imagePreview.GetBoundingRect();

				if (_pPreview->IsPreviewScaleIndependant() &&
					(rc.Width() > _size.cx || rc.Height() > _size.cy))
				{			
					IW::ImageStreamScale<IW::CNull> thumbnailStream(imageToFilter, Search::Any, _size); 
					IW::IterateImage(_imagePreview, thumbnailStream, this);
				}
				else
				{
					imageToFilter = _imagePreview;
				}
			}


			bSuccess = _pPreview->CreatePreview(imageToFilter, image, this);

			if (bSuccess)
			{
				CRect rc = image.GetBoundingRect();

				if ((rc.right - rc.left) > _size.cx ||
					(rc.bottom - rc.top) > _size.cy)
				{			
					IW::Image imageScaled;
					IW::ImageStreamScale<IW::CNull> thumbnailStream(imageScaled, Search::Any, _size); 
					IW::IterateImage(image, thumbnailStream, this);
					image = imageScaled;
				}		

				SetImage(image);
			}
		}

		if (_pPreview)
		{
			_pPreview->InvalidatePreview();
		}
	}

	void SetImage(IW::Image &image)
	{
		IW::CAutoLockCS lock(_cs);		

		_imageFiltered = image;	

		if (_pPreview)
		{
			_pPreview->InvalidatePreview();
		}
	}

protected:

	static DWORD WINAPI CPreviewThreadProc(LPVOID s) 
	{
		CPreviewThread *self = (CPreviewThread *)s;		

		while (!self->_bExit)
		{
			WaitForSingleObject(self->_event, 60000); 
			self->ApplyFilter();
		}

		return 0;
	}
};



template<class T>
class CSettingsDialogImpl : 
	public CDialogImpl<T>, 
	public IW::CDialogResizer<T>
{
	typedef CDialogImpl<T> BaseClass;
	typedef CSettingsDialogImpl<T> ThisClass;

public:

	CPreviewThread<T> _thread;
	CSize _sizePreview;
	IW::Image _imagePreview;
	IW::CRender _render;
	bool m_bSetting;
	CPoint _originalSize;

	CSettingsDialogImpl(const IW::Image &imagePreview) : _thread(static_cast<T*>(this)), _imagePreview(imagePreview)
	{ 
		m_bSetting = false;
	}

	~CSettingsDialogImpl()
	{ 
		_thread.End();
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_RESET, OnReset)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)
	END_MSG_MAP()

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}	

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);
		pT->OnHelp();
		return 0;
	}

	LRESULT OnReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		T *pT = static_cast<T*>(this);
		pT->OnReset();
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bool bWindowPlaced = false;			

		CRect rc;
		GetDlgItem(IDC_PREVIEW).GetWindowRect(rc);
		_sizePreview = rc.Size();

		GetWindowRect(rc);
		_originalSize = rc.Size();
		_thread.Create(_imagePreview, _sizePreview);

		// Templates
		ResizeAddItem(IDOK, eRight);
		ResizeAddItem(IDCANCEL, eRight);
		if (GetDlgItem(IDC_RESET) != NULL) ResizeAddItem(IDC_RESET, eRight);
		ResizeAddItem(IDHELP, eRight);
		ResizeAddItem(IDC_PREVIEW, eBottom);	
		ResizeAddItem(IDC_PREVIEW_FRAME, eBottom);	

		DoSize();

		if (!bWindowPlaced)
			CenterWindow();		

		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		DoSize(wParam);
		bHandled = FALSE;
		return 1;
	}

	void DoSize(WPARAM wParam = 0)
	{
		ResizeDialog();
	}

	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize = _originalSize;
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy CFilterPropertyDlg\n")); 
		_thread.End();
		return 0;
	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		DrawItem((LPDRAWITEMSTRUCT)lParam);
		return (LRESULT)TRUE;
	}

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		// must be implemented
		CDCHandle dc(lpDrawItemStruct->hDC);

		IW::CAutoLockCS lock(_thread._cs);
		IW::Image image;

		/*if (IDC_ORIGINAL == lpDrawItemStruct->CtlID)
		{
		image = _thread._imageScaled;
		}
		else */if (IDC_PREVIEW == lpDrawItemStruct->CtlID)
		{
			image = _thread._imageFiltered;
		}

		if (!image.IsEmpty())
		{
			CPoint pointCenterWindow;
			pointCenterWindow.x = (lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left) / 2;
			pointCenterWindow.y = (lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top) / 2;

			const IW::Page page = image.GetFirstPage();
			const int x = pointCenterWindow.x - (page.GetWidth() / 2);
			const int y = pointCenterWindow.y - (page.GetHeight() / 2);

			_render.Create(dc, lpDrawItemStruct->rcItem);			
			_render.Fill(IW::Style::Color::Face);
			_render.DrawImage(page, x, y);
			_render.Flip();
		}
	}

	void InvalidateButton(DWORD dwId)
	{
		HWND hWndButton = GetDlgItem(dwId);

		if (hWndButton)
		{
			::InvalidateRect(hWndButton, NULL, FALSE);
		}
	}

	void InvalidatePreview()
	{
		InvalidateButton(IDC_PREVIEW);
		//InvalidateButton(IDC_ORIGINAL);
	}

	void OnChange()
	{
		_thread._event.Set();
	}
	
	void OnHelp()
	{
	}
	
	void OnReset()
	{
	}

	bool IsPreviewScaleIndependant() const
	{
		const T *pT = static_cast<const T*>(this);
		return pT->_pFilter->IsPreviewScaleIndependant();
	}

	bool CreatePreview(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus) const
	{
		const T *pT = static_cast<const T*>(this);
		return pT->_pFilter->CreatePreview(imageIn, imageOut, pStatus);
	}
};

}; // namespace IW
