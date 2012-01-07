#pragma once

#include "layout.h"

template<class TParent>
class FolderLayout
{
public:

	TParent *_pParent;

	CSize _sizeThumb;
	CSize _sizeThumbImage;
	CSize _sizeThumbCenter;

	int _nTextHeight;	


	enum { sizeFolderOffsetX = 10,
		sizeFolderOffsetY = 4 };

	FolderLayout(TParent *pParent) : _pParent(pParent), _sizeThumb(1,1)
	{
	}

	virtual void Init()
	{
		const CSize sizeThumbDefault = App.Options._sizeThumbImage;

		_sizeThumb.cx = sizeThumbDefault.cx + (THUMB_PADDING * 4);
		_sizeThumb.cy = sizeThumbDefault.cy + (THUMB_PADDING * 4);
		_sizeThumbImage = sizeThumbDefault;
		_nTextHeight = App.Options.m_annotations.GetSize() * App.m_nTextExtent;
		_sizeThumb.cy += _nTextHeight;
		_sizeThumbCenter.cx = _sizeThumb.cx / 2;
		_sizeThumbCenter.cy = (_sizeThumb.cy - _nTextHeight) /2;

		if (_nTextHeight > 0)
		{
			// introduce a little overlap
			_sizeThumb.cy -= App.m_nTextExtent;
		}
	}

	virtual void DoSize()
	{
		const int nOldThumbsX = _pParent->_nThumbsX;
		const int nOldThumbsY = _pParent->_nThumbsY;

		SetScrollSizeList(true);

		CPoint pointOffset = _pParent->GetScrollOffset();
		const int nFirstThumb = ClosestThumbFromPoint(pointOffset);
		const int nThumbCount = _pParent->GetItemCount();

		if (nThumbCount > 0)
		{
			CPoint pointThumb = GetThumbPoint(nFirstThumb);
			
			if (_pParent->_nThumbsY != nOldThumbsY)
			{
				pointOffset = pointThumb;
			}

			_pParent->SetScrollOffset(pointOffset);
		}
	}

	virtual void SetScrollSizeList(bool bChangeOffset)
	{
		const CPoint pointOffset = _pParent->GetScrollOffset();
		const CSize sizeClient = GetClientSize();
		const CPoint pointOrigin = _pParent->GetScreenOrigin();

		int cx = sizeClient.cx;
		int cy = sizeClient.cy;
		int x = pointOffset.x;
		int y = pointOffset.y;

		const CSize sizeThumb = _sizeThumb;
		const int nThumbCount = _pParent->GetItemCount();

		if (nThumbCount > 0)
		{
			int cxAvailable = cx;
			int cyAvailable = cy;

			int nThumbsX = (cxAvailable < sizeThumb.cx) ? 1 : (cxAvailable / sizeThumb.cx);
			int nThumbsY = ((nThumbCount - 1) / nThumbsX) + 1;
			x = ((nThumbsX * sizeThumb.cx) - cx) / 2;

			cx = (nThumbsX * sizeThumb.cx) + pointOrigin.x;
			cy = (nThumbsY * sizeThumb.cy) + pointOrigin.y;		

			_pParent->_nThumbsX = nThumbsX;
			_pParent->_nThumbsY = nThumbsY;
		}

		_pParent->SetScrollSize(IW::LowerLimit<1>(cx), IW::LowerLimit<1>(cy), true, bChangeOffset);

		if (bChangeOffset)
		{
			_pParent->SetScrollOffset(x, y);
		}
	}

	int ThumbColumnCount() const { return _pParent->_nThumbsX; }
	const CSize GetThumbSize() const { return _sizeThumb; }
	const CSize GetClientSize() const { return _pParent->GetClientSize(); }	

	virtual void HoverChanged(int nHover)
	{
	}

	virtual void OnTimer()
	{
	}

	CPoint GetThumbPoint(const int nThumb) const
	{
		const CPoint pointOrigin = _pParent->GetScreenOrigin();
		const CSize sizeThumb = GetThumbSize();
		const int nThumbColumnCount = ThumbColumnCount();

		int nCol = (nThumb % nThumbColumnCount);
		int nRow = (nThumb / nThumbColumnCount);

		return CPoint(pointOrigin.x + (nCol * sizeThumb.cx), pointOrigin.y + (nRow * sizeThumb.cy));
	}

	int ThumbFromPoint(const CPoint &pt) const { return ThumbFromPoint(pt.x, pt.y); };
	int ClosestThumbFromPoint(const CPoint &pt) const { return ClosestThumbFromPoint(pt.x, pt.y); };	

	virtual void UnionTextRectIfFocusItem(int nThumb, CRect &rectThumb) const
	{
		const int nFocusItem = _pParent->GetFocusItem();

		if (nThumb == nFocusItem)
		{
			CString str;
			_pParent->GetThumbText(str, nThumb, true);

			CRect rectText = GetTextRect(nThumb, str);
			UnionRect(&rectThumb, &rectThumb, &rectText);
		}
	}

	virtual int ClosestThumbFromPoint(int x, int y) const
	{
		const CPoint pointOrigin = _pParent->GetScreenOrigin();
		const CSize sizeThumb = GetThumbSize();
		const int nThumbCount = _pParent->GetItemCount();
		const int nThumbColumnCount = ThumbColumnCount();

		x -= pointOrigin.x;
		y -= pointOrigin.y;

		int i = IW::Clamp(x / sizeThumb.cx, 0, nThumbColumnCount - 1);
		i += (y / sizeThumb.cy) * nThumbColumnCount;

		int nMaxThumb = nThumbCount - 1;
		return IW::Clamp(i, 0, nMaxThumb);
	}

	virtual int ThumbFromPoint(int x, int y) const
	{
		const CPoint pointOrigin = _pParent->GetScreenOrigin();
		const CSize sizeThumb = GetThumbSize();
		const int nThumbCount = _pParent->GetItemCount();
		const int nThumbColumnCount = ThumbColumnCount();

		x -= pointOrigin.x;
		y -= pointOrigin.y;

		if (x < 0 || x > nThumbColumnCount * sizeThumb.cx)
			return -1;

		int i = ((x / sizeThumb.cx) + ((y / sizeThumb.cy) * nThumbColumnCount));
		return ((i < 0) || (i >= nThumbCount)) ? -1 : i;
	}

	virtual CRect GetThumbRect(int nThumb) const
	{
		CRect rectThumb(GetThumbPoint(nThumb), GetThumbSize());
		UnionTextRectIfFocusItem(nThumb, rectThumb);
		return rectThumb;
	}

	virtual CRect GetTextRect(int i, LPCTSTR sz) const
	{
		const CSize sizeThumb = GetThumbSize();
		const CPoint pointThumb = GetThumbPoint(i);

		CClientDC dc(_pParent->GetHWnd());

		IW::CRender render;
		render.Create(dc);

		const CRect rectIn(0, 0, sizeThumb.cx - (THUMB_PADDING * 2), 0);
		CRect rectOut = render.MeasureString(sz, rectIn, IW::Style::Font::Standard, IW::Style::Text::SelectedThumbnail);

		rectOut.OffsetRect(
			pointThumb.x + (sizeThumb.cx - rectOut.Width()) / 2, 
			pointThumb.y + sizeThumb.cy - (_nTextHeight + THUMB_PADDING));

		return rectOut;
	}
	
	virtual void DrawItemText(IW::CRender &render, const IW::FolderItem *pItem, const CRect &rectThumb, bool bFocus, LPCTSTR szName, const CRect &rectAlpha, const COLORREF clrText, const COLORREF clrBG)
	{
		if (szName && szName[0] != 0)
		{
			CRect rectText(
				rectThumb.left + THUMB_PADDING, 
				rectThumb.bottom - (_nTextHeight + THUMB_PADDING), 
				rectThumb.right - THUMB_PADDING, 
				rectThumb.bottom); 

			if (bFocus)
			{
				rectText.bottom = rectText.top + 10000;
			}

			const bool bDrawImage = pItem->IsImage();
			const bool bIsFolder = pItem->IsFolder();

			CRect r;

			if (bDrawImage && bIsFolder)
			{
				render.Blend(clrBG, rectText);
			}
			else if (r.IntersectRect(&rectAlpha, rectText))
			{
				render.Blend(clrBG, r);
			}

			IW::Style::Text::Style style = bFocus ? IW::Style::Text::SelectedThumbnail : IW::Style::Text::Thumbnail;
			render.DrawString(szName, rectText, IW::Style::Font::Standard, style, clrText);
		}
	}
	
	virtual void DrawItemMarkers(IW::CRender &render, const IW::FolderItem *pItem, const CRect &rectThumb, const COLORREF clrText, const COLORREF clrBG)
	{
		CRect rectMarkerText(rectThumb);
		rectMarkerText.DeflateRect(THUMB_PADDING, THUMB_PADDING);

		bool bShowMarkers = App.Options.m_bShowMarkers;
		bool bSmallMode = IW::Min(rectMarkerText.Width(), rectMarkerText.Height()) < 40;

		if (!bSmallMode && bShowMarkers)
		{
			IW::FolderItemAttributes attributes;
			pItem->GetAttributes(attributes);			

			if (attributes.HasAttribute())
			{
				const IW::Style::Text::Style markersTextStyle = IW::Style::Text::NormalRight;
				const CString strMarkers = attributes.ToString();
				CRect rectMarkerTextOut = render.MeasureString(strMarkers, rectMarkerText, IW::Style::Font::Small, markersTextStyle);

				rectMarkerTextOut.left = rectMarkerText.right - rectMarkerTextOut.Width();
				rectMarkerTextOut.right = rectMarkerText.right;

				CRect rectMarkerTextBG(rectMarkerTextOut);
				rectMarkerTextBG.InflateRect(THUMB_PADDING, THUMB_PADDING);

				render.Blend(clrBG, rectMarkerTextBG);
				render.DrawString(strMarkers, rectMarkerTextOut, IW::Style::Font::Small, markersTextStyle, IW::Emphasize(clrText));
			}
		}
	}

	virtual void AdjustThumbRect(CRect &rectThumb, CPoint &pointThumb, const CRect &rectClip, bool bFocus)
	{
		const CPoint pointOffset = _pParent->GetScrollOffset();
		const CSize sizeClient = GetClientSize();
		const CSize sizeThumb = _sizeThumb;

		pointThumb.x -= pointOffset.x;
		pointThumb.y -= pointOffset.y;		

		rectThumb.left = pointThumb.x;
		rectThumb.top = pointThumb.y;
		rectThumb.right = pointThumb.x + sizeThumb.cx;
		rectThumb.bottom = pointThumb.y + sizeThumb.cy;
	}

	

	void DrawThumb(IW::CRender &render, int nItem, const CRect &rectClip)
	{
		CRect  rectThumb;

		CPoint pointThumb = GetThumbPoint(nItem);
		const CPoint pointOffset = _pParent->GetScrollOffset();	
		const CSize sizeThumb = _sizeThumb;
		const IW::FolderItem *pItem = _pParent->GetItem(nItem);


		// If this thumb is selected we draw the selected rect 
		const int nFocusItem = _pParent->GetFocusItem();
		const bool bFocus = (nItem == nFocusItem);
		const bool bHighLight = (nItem == _pParent->_nDragOverItem);
		const bool bIsSelected = _pParent->IsItemSelected(nItem);		

		AdjustThumbRect(rectThumb, pointThumb, rectClip, bFocus);

		CString str;		
		_pParent->GetThumbText(str, nItem, true);

		CRect rectBack = GetThumbRect(nItem);
		rectBack.OffsetRect(-pointOffset);

		COLORREF clrText = 0;
		COLORREF clrBG = IW::Style::Color::HighlightText;

		// In the drag drop
		if (bHighLight)
		{
			clrBG = IW::Style::Color::Highlight;
			//render.Fill(clrBG, &rectBack);
			render.Fill(IW::Emphasize(IW::SwapRB(clrBG)), rectBack);
			clrText = IW::Style::Color::HighlightText;
		}
		else if (bIsSelected) // Selected?
		{
			clrBG = IW::Style::Color::Highlight;

			if (bFocus)
			{
				render.Fill(clrBG, rectBack);
			}
			else
			{
				render.Fill(IW::Average(IW::Style::Color::Highlight, IW::Style::Color::Window), rectBack);
			}

			clrText = IW::Style::Color::HighlightText;
		}
		else
		{
			clrBG = IW::Style::Color::Window;
			clrText = IW::Style::Color::WindowText;
		}

		CRect rectAlpha(0,0,0,0);

		// Render Image
		DrawThumbnail(
			render, 
			nItem, 
			pointThumb, 
			sizeThumb, 
			_sizeThumbImage, 
			_sizeThumbCenter, 
			rectAlpha);

		DrawItemText(
			render, 
			pItem, 
			rectThumb, 
			bFocus,
			str,
			rectAlpha,
			clrText, 
			clrBG);

		DrawItemMarkers(
			render, 
			pItem, 
			rectThumb, 
			clrText, 
			clrBG);

		if (bFocus)
		{
			//render.DrawFocusRect(rectBack);
		}
	}


	void DrawThumbnail(IW::CRender &render, const int nItem, const CPoint &pointThumb, const CSize &sizeThumb, const CSize &sizeThumbImageCurrent, CSize &sizeThumbCenter, CRect &rectAlpha)
	{
		IW::FolderItemAttributes attributes;		

		const IW::FolderItem *pItem = _pParent->GetItem(nItem);
		pItem->GetAttributes(attributes, true);	

		CPoint pointCenterImage;
		pointCenterImage.x = pointThumb.x + sizeThumbCenter.cx;
		pointCenterImage.y = pointThumb.y + sizeThumbCenter.cy;

		bool bDrawAsImage = attributes.bIsImage || attributes.bIsImageIcon;	

		// Draw the bitmap
		if (bDrawAsImage)
		{
			DrawImage(render,
				attributes, 
				pointCenterImage, 
				sizeThumbImageCurrent,
				rectAlpha,
				nItem == _pParent->GetHoverItem());
		}

		bool bSmallMode = IW::Min(sizeThumb.cx, sizeThumb.cy) < 40;

		if (!bDrawAsImage || attributes.bIsFolder)
		{
			int cxy = bSmallMode ? 16 : 32;		
			CPoint pointDib(pointCenterImage - CSize(cxy/2, cxy/2));

			if (attributes.bIsImageIcon)
			{
				pointDib.x -= sizeFolderOffsetX;
				pointDib.y -= sizeFolderOffsetY;
			}

			if (attributes.nImage == -1)
			{
				HICON hIcon = attributes.bIsFolder ? IW::Style::Icon::Folder : IW::Style::Icon::Default;
				render.DrawIcon(hIcon, pointDib.x, pointDib.y, cxy, cxy);
			}
			else
			{
				render.DrawImageList(App.GetShellImageList(false), attributes.nImage, pointDib.x,  pointDib.y, cxy, cxy);
			}
		}	
	}


	virtual void DrawImage(IW::CRender &render, IW::FolderItemAttributes& attributes, const CPoint &pointCenterImage, const CSize &sizeThumbImageCurrent, CRect& rectAlpha, bool bHover)
	{
		IW::Page page = attributes.image.GetFirstPage();

		if (attributes.image.CanAnimate())
		{
			page = attributes.image.GetPage(attributes.nFrame);
		}

		// Image Scaled correctly?
		// We only worrk if the thumb is larger
		// than the current settings
		const CRect rectBounding = attributes.image.GetBoundingRect();
		const CPoint pointOffset = _pParent->GetScrollOffset();

		if ((int)rectBounding.Width() > sizeThumbImageCurrent.cx ||
			(int)rectBounding.Height() > sizeThumbImageCurrent.cy)
		{
			long icx = rectBounding.Width();
			long icy = rectBounding.Height();
			long nDiv = 0x1000;


			// Scale the image
			long sh = MulDiv(sizeThumbImageCurrent.cx, nDiv, icx);
			long sw = MulDiv(sizeThumbImageCurrent.cy, nDiv, icy);

			long s =  IW::Min(sh, sw);

			CSize sizeImage;
			sizeImage.cx = MulDiv(page.GetWidth(), s, nDiv);
			sizeImage.cy = MulDiv(page.GetHeight(), s, nDiv);

			// Image offset 
			CPoint pointOffset = page.GetOffset();
			pointOffset.x = MulDiv(pointOffset.x, s, nDiv);
			pointOffset.y = MulDiv(pointOffset.y, s, nDiv);

			// Draw the image
			int oy = pointCenterImage.y - (sizeImage.cy / 2) + pointOffset.y;
			int ox = pointCenterImage.x - (sizeImage.cx / 2) + pointOffset.x;

			if (attributes.bIsFolder)
			{
				ox += sizeFolderOffsetX;
				oy += sizeFolderOffsetY;
			}

			// Draw the image
			//TODO better scale
			CRect rectImage(ox, oy, ox + sizeImage.cx, oy + sizeImage.cy);
			render.DrawImage(page, rectImage);
		}
		else
		{
			CPoint pointOffset = page.GetOffset();

			int ox = pointCenterImage.x + pointOffset.x - (rectBounding.Width() / 2);
			int oy = pointCenterImage.y + pointOffset.y - (rectBounding.Height() / 2);

			if (attributes.bIsFolder)
			{
				ox += sizeFolderOffsetX;
				oy += sizeFolderOffsetY;
			}

			render.DrawImage(page, ox, oy);

			// May need to draw alpha block for text
			rectAlpha.left = ox;
			rectAlpha.top = oy;              
			rectAlpha.right = ox  + page.GetWidth();
			rectAlpha.bottom = oy + page.GetHeight();
		}
	}

	virtual void MakeItemVisible(int nThumb)
	{
		const CRect rc = GetThumbRect(nThumb);
		CPoint pointOffset = _pParent->GetScrollOffset();
		const CSize sizeClient = GetClientSize();

		if (rc.top <  pointOffset.y)
		{
			pointOffset.y = rc.top;
		}
		else if (rc.bottom > (pointOffset.y + sizeClient.cy))
		{
			pointOffset.y = rc.bottom - sizeClient.cy;
		}

		if (rc.left < pointOffset.x)
		{
			pointOffset.x = rc.left;
		}
		else if (rc.right > (pointOffset.x + sizeClient.cx))
		{
			pointOffset.x = rc.right - sizeClient.cx;
		}

		_pParent->SetScrollOffset(pointOffset);
		_pParent->UpdateBars();
	}
};


template<class TParent>
class FolderLayoutNormal : public FolderLayout<TParent>
{
public:
	typedef FolderLayout<TParent> BaseClass;
	typedef FolderLayoutNormal<TParent> ThisClass;

	FolderLayoutNormal(TParent *pParent) : BaseClass(pParent)
	{
	}
};



template<class TParent>
class FolderLayoutDetail : public FolderLayout<TParent>
{
public:
	typedef FolderLayout<TParent> BaseClass;
	typedef FolderLayoutDetail<TParent> ThisClass;

	bool _showDescriptions;

	FolderLayoutDetail(TParent *pParent) : BaseClass(pParent)
	{
		_showDescriptions = App.Options.ShowDescriptions;
	}

	void Init()
	{		
		const CSize sizeThumbDefault =  App.Options._sizeThumbImage;

		_sizeThumb.cx = (sizeThumbDefault.cx + (THUMB_PADDING * 4));
		_sizeThumb.cy = (sizeThumbDefault.cy + (THUMB_PADDING * 4));

		_sizeThumbImage = sizeThumbDefault;
		
		_nTextHeight = 0;
		_sizeThumbCenter.cx = _sizeThumb.cx / 2;
		_sizeThumbCenter.cy = _sizeThumb.cy / 2;
	}

	void AdjustThumbRect(CRect &rectThumb, CPoint &pointThumb, const CRect &rectClip, bool bFocus)
	{
		const CPoint pointOffset = _pParent->GetScrollOffset();
		const CSize sizeClient = GetClientSize();
		const CSize sizeThumb = _sizeThumb;

		pointThumb.x -= pointOffset.x;
		pointThumb.y -= pointOffset.y;

		int cx = IW::Max(_pParent->GetSizeAll().cx, sizeClient.cx);

		rectThumb.left = pointThumb.x;
		rectThumb.top = pointThumb.y;
		rectThumb.right = pointThumb.x + cx;
		rectThumb.bottom = pointThumb.y + sizeThumb.cy;
	}

	void SetScrollSizeList(bool bChangeOffset)
	{
		const CPoint pointOffset = _pParent->GetScrollOffset();
		const CSize sizeClient = GetClientSize();
		const CPoint pointOrigin = _pParent->GetScreenOrigin();

		int cx = sizeClient.cx;
		int cy = sizeClient.cy;
		int x = pointOffset.x;
		int y = pointOffset.y;
		
		const CSize sizeThumb = _sizeThumb;
		const int nThumbCount = _pParent->GetItemCount();

		if (nThumbCount > 0)
		{
			_pParent->_nThumbsX = 1;

			cx = IW::Max(sizeThumb.cx + 200 + pointOrigin.x, sizeClient.cx);
			cy = (((nThumbCount - 1) / _pParent->_nThumbsX)* sizeThumb.cy) + sizeThumb.cy + pointOrigin.y;
		}

		_pParent->SetScrollSize(IW::LowerLimit<1>(cx), IW::LowerLimit<1>(cy), true, bChangeOffset);

		if (bChangeOffset)
		{
			_pParent->SetScrollOffset(x, y);
		}
	}

	int ClosestThumbFromPoint(const CPoint &pt) const { return ClosestThumbFromPoint(pt.x, pt.y); };	

	int ClosestThumbFromPoint(int x, int y) const
	{
		const CSize sizeThumb = GetThumbSize();
		const CPoint pointOrigin = _pParent->GetScreenOrigin();
		const int nThumbCount = _pParent->GetItemCount();

		x -= pointOrigin.x;
		y -= pointOrigin.y;

		int i = y / sizeThumb.cy;
		int nMaxThumb = nThumbCount - 1;
		return IW::Clamp(i, 0, nMaxThumb);
	}

	int ThumbFromPoint(int x, int y) const
	{
		const CSize sizeThumb = GetThumbSize();
		const CSize sizeClient = GetClientSize();
		const CPoint pointOrigin = _pParent->GetScreenOrigin();
		const int nThumbCount = _pParent->GetItemCount();

		x -= pointOrigin.x;
		y -= pointOrigin.y;

		if (x < 0 || x > sizeClient.cx)
			return -1;

		int i = (y / sizeThumb.cy);
		return ((i < 0) || (i >= nThumbCount)) ? -1 : i;
	}

	CRect GetThumbRect(int nThumb) const
	{
		const CSize sizeThumb = GetThumbSize();
		const CSize sizeClient = GetClientSize();
		const CPoint pointThumb = GetThumbPoint(nThumb);

		const CSize sizeDetail(IW::Max(_pParent->GetSizeAll().cx, sizeClient.cx), sizeThumb.cy);
		return CRect(pointThumb, sizeDetail);
	}

	CRect GetTextRect(int i, LPCTSTR sz) const
	{		
		const CSize sizeClient = GetClientSize();
		const CSize sizeThumb = GetThumbSize();
		const CPoint pointThumb = GetThumbPoint(i);

		CClientDC dc(_pParent->GetHWnd());

		IW::CRender render;
		render.Create(dc);

		int x = sizeThumb.cx + THUMB_PADDING;
		CRect rectIn(x, pointThumb.y, sizeClient.cx, 0);
		
		return render.MeasureString(sz, rectIn, IW::Style::Font::Standard, IW::Style::Text::SelectedThumbnail);
	}


	void DrawItemText(IW::CRender &render, const IW::FolderItem *pItem, const CRect &rectThumb, bool bFocus, LPCTSTR szName, const CRect &rectAlpha, const COLORREF clrText, const COLORREF clrBG)
	{
		const CSize sizeThumb = _sizeThumb;

		int x = rectThumb.left + sizeThumb.cx + THUMB_PADDING;
		int y = rectThumb.top + THUMB_PADDING;
		int cx = (rectThumb.right - x) / (_showDescriptions ? 2 : 1);
		CRect rectText(x, y, x + cx, rectThumb.bottom);
		int cxTitle = 0;

		CSimpleArray<CString> arrayStr;
		pItem->GetFormatText(arrayStr, App.Options.m_columns, false);	
		const int titleGap = 4;

		for(int i = 0; i < arrayStr.GetSize(); i++)
		{
			CString strTitle = App.GetMetaDataShortTitle(App.Options.m_columns[i]) + _T(":");
			CRect r = render.MeasureString(strTitle, rectText, IW::Style::Font::Standard, IW::Style::Text::SingleLineRight);
			cxTitle = IW::Max(cxTitle, r.Width() + titleGap);
		}

		//cxTitle = IW::Min(cx / 2, cxTitle);		

		for(int i = 0; i < arrayStr.GetSize(); i++)
		{
			DWORD dwId = App.Options.m_columns[i];

			CString strText = arrayStr[i];
			CString strTitle = App.GetMetaDataShortTitle(dwId);
			strTitle += _T(":");

			CRect rect(rectText);
			rect.OffsetRect(0, App.m_nTextExtent * i);
			
			CRect rectTitle(rect); rectTitle.right = (rectTitle.left + cxTitle) - titleGap;
			render.DrawString(strTitle, rectTitle, IW::Style::Font::Standard, IW::Style::Text::SingleLineRight, clrText);

			rect.left += cxTitle;
			render.DrawString(strText, rect, IW::Style::Font::Standard, IW::Style::Text::Ellipsis, clrText);
		}

		if (_showDescriptions)
		{
			CString str = pItem->GetImage().GetDescription();

			// Load the title
			if (!str.IsEmpty())
			{
				CRect rectDescription(x + cx + 4, y, rectThumb.right, rectThumb.bottom);
				render.DrawString(str, rectDescription, IW::Style::Font::Standard, IW::Style::Text::Normal, clrText);
			}
		}
	}

	void DrawItemMarkers(IW::CRender &render, const IW::FolderItem *pItem, const CRect &rectThumb, const COLORREF clrText, const COLORREF clrBG)
	{
		CRect rectMarkerText(rectThumb);
		rectMarkerText.DeflateRect(THUMB_PADDING, THUMB_PADDING);		

		IW::FolderItemAttributes attributes;
		pItem->GetAttributes(attributes);			

		if (attributes.HasAttribute())
		{
			const IW::Style::Text::Style markersTextStyle = IW::Style::Text::Normal;
			const CString strMarkers = attributes.ToString();
			CRect rectMarkerTextOut = render.MeasureString(strMarkers, rectMarkerText, IW::Style::Font::Small, markersTextStyle);

			CRect rectMarkerTextBG(rectMarkerTextOut);
			rectMarkerTextBG.InflateRect(THUMB_PADDING, THUMB_PADDING);

			render.Blend(clrBG, rectMarkerTextBG);
			render.DrawString(strMarkers, rectMarkerTextOut, IW::Style::Font::Small, markersTextStyle, IW::Emphasize(clrText));
		}
	}

	void MakeItemVisible(int nThumb)
	{
		const CSize sizeClient = GetClientSize();
		const CRect rc = GetThumbRect(nThumb);
		CPoint pointOffset = _pParent->GetScrollOffset();

		if (rc.top <  pointOffset.y)
		{
			pointOffset.y = rc.top;
		}
		else if (rc.bottom > (pointOffset.y + sizeClient.cy))
		{
			pointOffset.y = rc.bottom - sizeClient.cy;
		}		

		_pParent->SetScrollOffset(pointOffset);
		_pParent->UpdateBars();
	}

	void DoSize()
	{
		int nOldThumbsX = _pParent->_nThumbsX;
		int nOldThumbsY = _pParent->_nThumbsY;		

		SetScrollSizeList(true);

		CPoint pointOffset = _pParent->GetScrollOffset();
		const int nThumbCount = _pParent->GetItemCount();

		if (nThumbCount > 0)
		{
			const int nFirstThumb = ClosestThumbFromPoint(pointOffset);			
			const CPoint pointThumb = GetThumbPoint(nFirstThumb);			

			if (_pParent->_nThumbsX != nOldThumbsX)
			{
				pointOffset.x = pointOffset.x;
				pointOffset.y = pointThumb.y;			
			}

			_pParent->SetScrollOffset(pointOffset);

			_showDescriptions = App.Options.ShowDescriptions && _pParent->HasDescriptions();
		}
		else
		{
			_showDescriptions = false;
		}		
	}
};

template<class TParent>
class FolderLayoutMatrix : public FolderLayout<TParent>
{
public:
	typedef FolderLayout<TParent> BaseClass;
	typedef FolderLayoutMatrix<TParent> ThisClass;

	FolderLayoutMatrix(TParent *pParent) : BaseClass(pParent), _grow(0, 100, 20)
	{
	}

	CSize FindThumbSize(const CSize &sizeCanvas, const int nThumbCount, int nCurrentSize)
	{
		const int nMinSize = 16 + THUMB_PADDING;
		const int nMaxHeight = MulDiv(nCurrentSize, 3, 2);

		while(nCurrentSize > nMinSize)
		{
			int nThumbCountX = IW::LowerLimit<1>(sizeCanvas.cx / nCurrentSize);
			int nThumbCountY = IW::LowerLimit<1>(nThumbCount / nThumbCountX);
			if ((nThumbCount % nThumbCountX) != 0) nThumbCountY++;

			int nPixelsY = nThumbCountY * nCurrentSize;

			if (nPixelsY <= sizeCanvas.cy)
			{
				CSize sizeThumb(sizeCanvas.cx / nThumbCountX, sizeCanvas.cy / nThumbCountY);
				if (sizeThumb.cy > nMaxHeight) sizeThumb.cy = nMaxHeight;
				return sizeThumb;
			}
			
			nCurrentSize -= 1;
		}
		
		return CSize(nMinSize, nMinSize);
	}

	void CalcThumbSize()
	{
		const CSize sizeThumbDefault =  App.Options._sizeThumbImage;
		const int nThumbCount = _pParent->GetItemCount();
		const CSize sizeClient = GetClientSize();
		const CPoint pointOrigin = _pParent->GetScreenOrigin();
		const CSize sizeAvailiable(IW::LowerLimit<1>(sizeClient.cx), IW::LowerLimit<1>(sizeClient.cy));
		const int nMaxSize = IW::Max(sizeThumbDefault.cx, sizeThumbDefault.cy) + THUMB_PADDING;

		CSize sizeThumb = FindThumbSize(sizeAvailiable, nThumbCount, nMaxSize);

		_sizeThumb = sizeThumb;

		_sizeThumbImage.cx = sizeThumb.cx - THUMB_PADDING;
		_sizeThumbImage.cy = sizeThumb.cy - THUMB_PADDING;

		_nTextHeight = 0;
		_sizeThumbCenter.cx = sizeThumb.cx / 2;
		_sizeThumbCenter.cy = sizeThumb.cy / 2;
	}

	virtual void Init()
	{
		CalcThumbSize();
	}

	CRect GetThumbRect(int nItem) const
	{
		CRect rect(GetThumbPoint(nItem), GetThumbSize());
		const IW::FolderItem *pItem = _pParent->GetItem(nItem);

		if (nItem == _pParent->GetHoverItem() && pItem->IsImage() && App.Options.ZoomThumbnails)
		{			
			rect |= GetFocusThumbRect(pItem->GetItemThumbRect(), rect.CenterPoint());
		}
			
		return rect;
	}

	CRect GetFocusThumbRect(const CRect rectBounding, const CPoint &pointCenterImage) const
	{
		const CPoint pointImage = pointCenterImage - rectBounding.CenterPoint();

		CRect rectZoom(pointImage, rectBounding.Size());
		CRect rectScreen(_pParent->GetScreenOrigin(), GetClientSize());
		rectScreen.OffsetRect(_pParent->GetScrollOffset());
		rectScreen.DeflateRect(1,1);
		rectZoom = IW::ClampRect(rectZoom, rectScreen);

		rectZoom.InflateRect(2, 2);
		return rectZoom;
	}
	
	virtual void DrawItemText(IW::CRender &render, const IW::FolderItem *pItem, const CRect &rectThumbIn, bool bFocus, LPCTSTR szName, const CRect &rectAlpha, const COLORREF clrText, const COLORREF clrBG)
	{
		CRect rectThumb(rectThumbIn);
		bool bSmallMode = IW::Min(rectThumb.Width(), rectThumb.Height()) < 40;

		if (szName && szName[0] != 0 && !bSmallMode)
		{
			const bool bDrawImage = pItem->IsImage();

			if (!bDrawImage)
			{
				CString strItemName = pItem->GetDisplayPath();
				CRect rectText = render.MeasureString(strItemName, rectThumb, IW::Style::Font::Standard, IW::Style::Text::Thumbnail);

				int cx = (rectThumb.Width() - rectText.Width()) / 2;
				int cy = rectThumb.Height() - rectText.Height();

				rectText.OffsetRect(cx, cy);
				
				render.Blend(clrBG, rectText);
				render.DrawString(strItemName, rectText, IW::Style::Font::Standard, IW::Style::Text::Thumbnail, clrText);
			}
		}
	}

	void SetScrollSizeList(bool bChangeOffset)
	{
		CalcThumbSize();
		BaseClass::SetScrollSizeList(bChangeOffset);
	}	

	void DrawImage(IW::CRender &render, IW::FolderItemAttributes& attributes, const CPoint &pointCenterImage, const CSize &sizeThumbImageCurrent, CRect& rectAlpha, bool bHover)
	{
		IW::Page page = attributes.image.GetFirstPage();

		if (attributes.image.CanAnimate() || attributes.bIsFolder)
		{
			BaseClass::DrawImage(render, 
				attributes, 
				pointCenterImage, 
				sizeThumbImageCurrent, 
				rectAlpha, bHover);
		}
		else
		{
			const CRect rectBounding = attributes.image.GetBoundingRect();
			const CSize sizeImage = rectBounding.Size();
			const CSize sizeAvail = (bHover && App.Options.ZoomThumbnails) ? AnimatedImageSize(sizeImage, sizeThumbImageCurrent) : sizeThumbImageCurrent;
			CRect rectDraw, rectSrc;

			if (sizeImage.cx <= sizeAvail.cx && sizeImage.cy <= sizeAvail.cy)
			{
				const CPoint pointOffset = page.GetOffset();

				int x = pointCenterImage.x + pointOffset.x - (rectBounding.Width() / 2);
				int y = pointCenterImage.y + pointOffset.y - (rectBounding.Height() / 2);

				rectDraw = CRect(x, y, x + page.GetWidth(), y + page.GetHeight());
				rectSrc = CRect(0, 0, page.GetWidth(), page.GetHeight());
			}
			else if ((sizeImage.cx <= sizeAvail.cx && sizeImage.cy > sizeAvail.cy) ||
				(sizeImage.cx > sizeAvail.cx && sizeImage.cy <= sizeAvail.cy))
			{
				const CPoint pointOffset = page.GetOffset();

				int x = pointCenterImage.x + pointOffset.x - (rectBounding.Width() / 2);
				int y = pointCenterImage.y + pointOffset.y - (rectBounding.Height() / 2);

				CRect rectImage(x, y, x + page.GetWidth(), y + page.GetHeight());
				CRect rectClip(CPoint(pointCenterImage.x - (sizeAvail.cx / 2), pointCenterImage.y - (sizeAvail.cy / 2)), sizeAvail);

				rectDraw = IW::ClipRect(rectImage, rectClip);
				rectSrc = CRect(CPoint(rectDraw.TopLeft() - rectImage.TopLeft()), rectDraw.Size());
			}
			else if (sizeImage.cx > sizeAvail.cx || sizeImage.cy > sizeAvail.cy)
			{
				const CPoint pointSourceCenter = rectBounding.CenterPoint();
				int nRadiusIn = IW::Min(sizeImage.cx, sizeImage.cy) / 2;
				int nRadiusOut = IW::Min(sizeAvail.cx, sizeAvail.cy) / 2;

				rectSrc = CRect(pointSourceCenter.x - nRadiusIn, 
					pointSourceCenter.y - nRadiusIn,
					pointSourceCenter.x + nRadiusIn, 
					pointSourceCenter.y + nRadiusIn);

				rectDraw = CRect(pointCenterImage.x - nRadiusOut, 
					pointCenterImage.y - nRadiusOut,
					pointCenterImage.x + nRadiusOut, 
					pointCenterImage.y + nRadiusOut);
			}

			CRect rectScreen(_pParent->GetScreenOrigin(), GetClientSize());
			rectScreen.DeflateRect(1,1);
			rectDraw = IW::ClampRect(rectDraw, rectScreen);

			render.DrawImage(page, rectDraw, rectSrc);
		}
	}

	CSize AnimatedImageSize(const CSize &sizeImage, const CSize &sizeThumbImageCurrent) const
	{
		const CSize sizeRange = sizeImage - sizeThumbImageCurrent;
		int pos = _grow.Pos();
		return CSize(sizeThumbImageCurrent.cx + ((sizeRange.cx * pos) / 100),
			sizeThumbImageCurrent.cy + ((sizeRange.cy * pos) / 100));
	}

	ScalarAnimation _grow;
	int _nHover;

	virtual void HoverChanged(int nHover)
	{
		_nHover = nHover;
		_grow.Reset();
		_grow.SetDirection(true);
	}

	void OnTimer()
	{
		int nThumbCount = _pParent->GetItemCount();

		if (_nHover > nThumbCount)
			_nHover = nThumbCount - 1;

		if (_grow.Animate() && _nHover != -1)
			_pParent->InvalidateThumb(_nHover);
	}
};