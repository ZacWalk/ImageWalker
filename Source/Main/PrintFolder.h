///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// PrintFolder.h: interface for the CPrintFolder class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Items.h"
#include "LoadAny.h"

class CPrintFolder : public CPrintJobInfo
{
private:

	
	State &_state; 

public:

	CPrintFolder(State &state);	
	CPrintFolder(const CPrintFolder &f);

	virtual ~CPrintFolder();	
	
	void Copy(const CPrintFolder &f);
	void operator=(const CPrintFolder &f) { Copy(f); };


	bool PrintImage(CDCHandle dc, IW::FolderItem *pItem, CPoint point);
	bool PrintPage(CDCHandle dc, UINT nPage);
	bool PrintHeaders(CDCHandle dc, UINT nPage);

	int GetPageCount();
	bool CalcLayout();
	bool CalcLayout(CDCHandle dc);
	bool CalcLayout(CDCHandle dc, const CRect &rcPage);

	bool DrawImage(CDCHandle dc, CPoint point, IW::Image &image, IW::FolderItem *pItem);

	

	//print job info callback
	virtual bool IsValidPage(UINT nPage);
	virtual bool PrintPage(UINT nPage, HDC hDC);

	// Serialisation
	void Read(const IW::IPropertyArchive *pArchive);
	void Write(IW::IPropertyArchive *pArchive) const;

protected:

	// Preview
	CRect m_rcOutput;
	CSize _sizeLogPixels;
	CLoadAny _loader;

public:

	CFont m_font;
	CFont m_fontTitle;

	CDevModeT<true> m_devmode;
	CPrinterT<true> m_printer;

	IW::IStatus *_pStatus;

public:

	void GetPageRect(CRect& rc, LPRECT prc);
	void DoPaint(CDCHandle dc, CRect& rc, int nCurPage);
	bool HasPrinter() const throw() { return m_bHasPrinter; };

	

	// Content
	bool m_bCenter;
	bool m_bPrinting;
	bool m_bPrintLandscape;
	bool m_bPrintOnePerPage;	
	bool m_bPrintSelected;
	bool m_bShadow;
	bool m_bFrame;
	bool m_bShowFooter;
	bool m_bShowHeader;
	bool m_bShowPageNumbers;
	bool m_bWrap;	
	COLORREF m_clrBackGround;
	int m_nFooterHeight;
	int m_nHeaderHeight;
	int m_nPadding;
	int _nTextHeight;
	int  m_nPrintRotateBest;
	IW::CArrayDWORD m_annotations;
	CString _strFooter;
	CString _strHeader;
	CRect m_rcMargin;
	CRect _rectExtents;
	CSize _sizeSection;
	CSize _sizeThumbNail;
	CSize _sizeRowsColumns;
	bool m_bHasPrinter;

	int m_nMaxPage;
	int m_nMinPage;
	int m_nCurPage;
	int _nImageCount;

	
};
