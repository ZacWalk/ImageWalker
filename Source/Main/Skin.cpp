#include "stdafx.h"
#include "Skin.h"
#include "ShellMenu.h"

void IW::Skin::DrawGradient(CDCHandle dc, const CRect &r, DWORD c1, DWORD c2)
{
	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = r.left;
	vert [0] .y      = r.top;
	vert [0] .Red    = IW::GetR(c1) << 8;
	vert [0] .Green  = IW::GetG(c1) << 8;
	vert [0] .Blue   = IW::GetB(c1) << 8;
	vert [0] .Alpha  = 0x0000;
	vert [1] .x      = r.right;
	vert [1] .y      = r.bottom; 
	vert [1] .Red    = IW::GetR(c2) << 8;
	vert [1] .Green  = IW::GetG(c2) << 8;
	vert [1] .Blue   = IW::GetB(c2) << 8;
	vert [1] .Alpha  = 0x0000;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	GradientFill(dc ,vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void IW::Skin::DrawArrow(CDCHandle& dc, const CRect &rc)
{
	// Draw arrow
	CPen pen; pen.CreatePen(PS_SOLID, 0, IW::Emphasize(IW::Style::Color::WindowText));
	CBrush brush; brush.CreateSolidBrush(IW::Style::Color::WindowText);

	HPEN hOldPen = dc.SelectPen(pen);
	HBRUSH hOldBrush = dc.SelectBrush(brush);
	int xMiddle = rc.left - 1 + (rc.right - rc.left) / 2;
	int yMiddle = rc.top + (rc.bottom - rc.top) / 2;
	const int ARROW_SIZE = 2;
	POINT pt[] = {
		{ xMiddle - ARROW_SIZE, yMiddle - ARROW_SIZE },
		{ xMiddle + ARROW_SIZE, yMiddle - ARROW_SIZE },
		{ xMiddle, yMiddle + 1 }
	};
	dc.Polygon(pt, 3);
	dc.SelectPen(hOldPen);
	dc.SelectBrush(hOldBrush);
}

void IW::Skin::DrawBitmapDisabled(CDCHandle& dc, HIMAGELIST hImageList, POINT point, int nImage)
{
	IMAGELISTDRAWPARAMS ildp = { 0 };
	ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
	ildp.himl = hImageList;
	ildp.i = nImage;
	ildp.hdcDst = dc;
	ildp.x = point.x;
	ildp.y = point.y;
	ildp.cx = 0;
	ildp.cy = 0;
	ildp.xBitmap = 0;
	ildp.yBitmap = 0;
	ildp.fStyle = ILD_TRANSPARENT;
	ildp.fState = ILS_SATURATE;
	ildp.Frame = 0;
	::ImageList_DrawIndirect(&ildp);
}

void IW::Skin::DrawButton(LPNMTBCUSTOMDRAW lpTBCustomDraw, HIMAGELIST hImageList, const CSize &sizeImage, bool bDrawArrows)
{
	CRect r = lpTBCustomDraw->nmcd.rc;
	CToolBarCtrl tb(lpTBCustomDraw->nmcd.hdr.hwndFrom);
	CDCHandle dc = lpTBCustomDraw->nmcd.hdc;			

	const int cchText = 200;
	TCHAR szText[cchText] = { 0 };
	TBBUTTONINFO tbbi = { 0 };
	tbbi.cbSize = sizeof(TBBUTTONINFO);
	tbbi.dwMask = TBIF_TEXT | TBIF_IMAGE | TBIF_STYLE | TBIF_STATE;
	tbbi.pszText = szText;
	tbbi.cchText = cchText;			
	tb.GetButtonInfo((int)lpTBCustomDraw->nmcd.dwItemSpec, &tbbi);

	UINT uItemState = lpTBCustomDraw->nmcd.uItemState;
	bool bSelected = (uItemState & ODS_SELECTED) != 0;
	bool bHotlight = (uItemState & ODS_HOTLIGHT) != 0;
	bool bChecked = (tbbi.fsState & TBSTATE_CHECKED) != 0;
	bool bPressed = (tbbi.fsState & TBSTATE_PRESSED) != 0;
	bool bDisabled = (tbbi.fsState & TBSTATE_ENABLED) == 0;
	bool bDropDown = (tbbi.fsStyle & TBSTYLE_DROPDOWN) != 0;
	bool bDropWhole = (tbbi.fsStyle & BTNS_WHOLEDROPDOWN) != 0;
	bool bList = ((tb.GetStyle() & TBSTYLE_LIST) == TBSTYLE_LIST);

	CString str = tbbi.pszText;

	COLORREF clrHighlight = IW::Style::Color::Highlight;
	COLORREF clrHighlightText = IW::Style::Color::HighlightText;
	COLORREF clrText = IW::Style::Color::WindowText;

	int cxThumb = 10;
	CRect rArrow = r;
	rArrow.left = rArrow.right - cxThumb;			

	if(bDisabled)
	{
		lpTBCustomDraw->clrText = 0x808080;
	} 
	else if (bPressed)
	{
		dc.FillSolidRect(r, IW::Style::Color::Highlight);
		dc.FrameRect(r, IW::Style::Brush::EmphasizedHighlight);
		lpTBCustomDraw->clrText = clrHighlightText;
	}
	else if(bHotlight)
	{
		dc.FillSolidRect(r, IW::Average(IW::Style::Color::Highlight, IW::Style::Color::Window));
		dc.FrameRect(r, IW::Style::Brush::EmphasizedHighlight);
		lpTBCustomDraw->clrText = clrHighlightText;
	}
	else if (bChecked)
	{
		dc.FillSolidRect(r, IW::Average(IW::Style::Color::Highlight, IW::Style::Color::Window));
		lpTBCustomDraw->clrText = clrText;
	}
	else 
	{
		lpTBCustomDraw->clrText = clrText;
	}	

	HFONT hFont = tb.GetFont();
	HFONT hFontOld = NULL;
	if(hFont != NULL)
		hFontOld = dc.SelectFont(hFont);

	if (bDrawArrows)
	{
		if (bDropDown && (bPressed || bHotlight))
		{
			dc.FrameRect(rArrow, IW::Style::Brush::EmphasizedHighlight);
		}

		if (bDropWhole || bDropDown) 
		{
			// Paint arrow				
			DrawArrow(dc, rArrow);
			r.right -= cxThumb;
		}
	}

	int y = r.top + ((r.Height() - sizeImage.cy) / 2);

	if (!str.IsEmpty())
	{
		CRect rText(r);
		dc.DrawText(str, -1, rText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
		int width = rText.Width();
		if (tbbi.iImage != 0) width += sizeImage.cx + 2;
		if (width < r.Width()) r.left += (r.Width() - width) / 2;
	}
	else
	{
		r.left += ((r.Width() - sizeImage.cx) / 2);
	}

	if (hImageList != NULL)
	{
		if (bDisabled)
		{
			DrawBitmapDisabled(dc, hImageList, CPoint(r.left, y), tbbi.iImage);
		}
		else
		{
			ImageList_Draw(hImageList, tbbi.iImage, dc, r.left, y, ILD_TRANSPARENT);
		}

		r.left += sizeImage.cx + 2;
	}

	if (!str.IsEmpty())
	{
		dc.SetTextColor(lpTBCustomDraw->clrText);
		dc.SetBkMode(lpTBCustomDraw->nStringBkMode);				
		dc.DrawText(str, -1, r, DT_SINGLELINE | DT_VCENTER);				
	}

	if(hFont != NULL)
		dc.SelectFont(hFontOld);
}

struct _MenuItemData	// menu item data
{
	DWORD dwMagic;
	LPTSTR lpstrText;
	UINT fType;
	UINT fState;
	int iButton;

	_MenuItemData() { dwMagic = 0x1313; }
	bool IsCmdBarMenuItem() { return (dwMagic == 0x1313); }
};

void DrawMenuText(CDCHandle& dc, RECT& rc, LPCTSTR lpstrText, COLORREF color)
{
	const bool m_bShowKeyboardCues = true;
	int nTab = -1;
	for(int i = 0; i < lstrlen(lpstrText); i++)
	{
		if(lpstrText[i] == _T('\t'))
		{
			nTab = i;
			break;
		}
	}
	dc.SetTextColor(color);
	dc.DrawText(lpstrText, nTab, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));
	if(nTab != -1)
		dc.DrawText(&lpstrText[nTab + 1], -1, &rc, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));
}

void IW::Skin::DrawMenuItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	_MenuItemData* pMI = (_MenuItemData*)lpDrawItemStruct->itemData;
	ShellMenuItem* pSMI = (ShellMenuItem*)lpDrawItemStruct->itemData;

	CDCHandle dc = lpDrawItemStruct->hDC;
	const RECT& rcItem = lpDrawItemStruct->rcItem;

	CSize m_szBitmap(16, 16);
	CSize m_szButton(m_szBitmap.cx + 2 * 3, m_szBitmap.cy + 2 * 3);


	BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
	BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
	BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;

	// paint background
	if(bSelected)
	{
		dc.FillSolidRect(&rcItem, IW::Style::Color::Highlight);
		dc.FrameRect(&rcItem, IW::Style::Brush::EmphasizedHighlight);
	}
	else
	{	
		dc.FillSolidRect(&rcItem, IW::Style::Color::MenuBackground);
	}

	if(pMI->fType & MFT_SEPARATOR)
	{
		// draw separator
		CRect rc = rcItem;		
		rc.bottom = rc.top = (rc.bottom + rc.top) / 2;
		rc.InflateRect(-2, 1);

		dc.FillSolidRect(&rc, IW::Emphasize(IW::Style::Color::MenuBackground, 64));   // draw separator line
	}
	else		// not a separator
	{
		if(LOWORD(lpDrawItemStruct->itemID) == (WORD)-1)
			bSelected = FALSE;
		CRect rcButn(rcItem.left, rcItem.top, rcItem.left + m_szButton.cx, rcItem.top + m_szButton.cy);   // button rect
		rcButn.OffsetRect(0, ((rcItem.bottom - rcItem.top) - (rcButn.bottom - rcButn.top)) / 2);          // center vertically

		// draw background and border for checked items
		if(bChecked)
		{
			CRect rcCheck = rcButn;
			rcCheck.DeflateRect(1, 1);
			dc.FrameRect(&rcCheck, IW::Style::Brush::EmphasizedHighlight);
		}		

		// calc drawing point
		SIZE sz = { rcButn.right - rcButn.left - m_szBitmap.cx, rcButn.bottom - rcButn.top - m_szBitmap.cy };
		sz.cx /= 2;
		sz.cy /= 2;
		POINT point = { rcButn.left + sz.cx, rcButn.top + sz.cy };

		HIMAGELIST hImageList = App.GetGlobalBitmap();

		if (pSMI->IsValid())
		{
			hImageList = App.GetShellImageList(true);
		}

		int iButton = pMI->iButton;
		if(iButton >= 0)
		{
			// draw disabled or normal
			if(!bDisabled)
			{
				::ImageList_Draw(hImageList, iButton, dc, point.x, point.y, ILD_TRANSPARENT);
			}
			else
			{
				DrawBitmapDisabled(dc, hImageList, point, iButton);
			}
		}
		else
		{
			if (bChecked)
			{
				CRect r(point, m_szBitmap);
				dc.FillSolidRect(r, IW::Average(IW::Style::Color::Highlight, IW::Style::Color::Window));
			}
		}

		const int s_kcxGap = 1;
		const int s_kcxTextMargin = 2;


		// draw item text
		int cxButn = m_szButton.cx;
		// calc text rectangle and colors
		RECT rcText = rcItem;
		rcText.left += cxButn + s_kcxGap + s_kcxTextMargin;
		rcText.right -= cxButn;
		dc.SetBkMode(TRANSPARENT);

		COLORREF colorText = bDisabled ?  IW::Emphasize(IW::Style::Color::MenuBackground, 100) : (bSelected ? IW::Style::Color::HighlightText : IW::Style::Color::WindowText);
		DrawMenuText(dc, rcText, pMI->lpstrText, colorText); // finally!
	}
}