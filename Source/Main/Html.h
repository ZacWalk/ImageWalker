// WebSettings.h: interface for the CWebSettings class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "Items.h"

class  CURLProperties : public IW::IPropertyArchive
{
protected:
	typedef std::map<CString, CString> ARGUMENTSMAP;
	ARGUMENTSMAP m_mapArguments;

public:

	CURLProperties();
	virtual ~CURLProperties();	

	bool Write(const CString &strKey, DWORD dwValue);
	bool Read(const CString &strKey, DWORD& dwValue) const;

	bool Write(const CString &strKey, int nValue);
	bool Read(const CString &strKey, int& nValue) const;

	bool Write(const CString &strKey, long nValue);
	bool Read(const CString &strKey, long& nValue) const;

	bool Write(const CString &strKey, bool bValue);
	bool Read(const CString &strKey, bool& bValue) const;

	bool Read(const CString &strKey, CString &str) const;
	bool Write(const CString &strKey, LPCTSTR szValue);

	bool Read(const CString &strKey, LPVOID pValue, DWORD &dwCount) const;
	bool Write(const CString &strKey, LPCVOID pValue, DWORD dwCount);

	bool StartSection(const CString &strKey) const { return false; };
	bool EndSection() const { return false; };

	bool StartSection(const CString &strKey) { return false; };
	bool EndSection() { return false; };

	bool ParseText(const CString &strKey);
	CString ToString() const;
};

class CWebSettings
{
public:
	CWebSettings();
	virtual ~CWebSettings() {};

	CWebSettings(const CWebSettings &s) { Copy(s); };
	void operator=(const CWebSettings &s)  { Copy(s); };
	void Copy(const CWebSettings &s);

	COLORREF m_clrBackGround;
	IW::CArrayDWORD m_annotations;
	CSize _sizeRowsColumns;
	long m_nImageBorderWidth;
	int m_nTemplateSelection;
	int m_nStyleSheetSelection;
	int m_nDelay;
	bool m_bFrame;
	bool m_bShadow;
	bool m_bShowImagesOnly;
	bool m_bFitImages;
	bool m_bBreadCrumbs;

	// Serialisation
	void Read(const IW::IPropertyArchive *pArchive);
	void Write(IW::IPropertyArchive *pArchive) const;
};

typedef CSimpleValArray<CString> CStringArray;


class CAddressPolicy
{
public:
	virtual CString GetIndexURL(const CString &strLocation, int i, bool bFromIndex) = 0;
	virtual CString GetImageURL(const CString &strLocation, const CString &strName, bool bFromIndex) = 0;
	virtual CString GetFolderURL(const CString &strLocation, const CString &strName, bool bFromIndex) = 0;
	virtual CString GetThumbnailURL(const CString &strLocation, const CString &strName, COLORREF clrBG, bool bFromIndex) = 0;
	virtual CString GetFileURL(const CString &strLocation, const CString &strName, bool bFromIndex) = 0;
	virtual CString GetImageFileURL(const CString &strLocation, const CString &strName, bool bFromIndex) = 0;
	virtual CString GetTemplateURL(const CString &strLocation, bool bFromIndex) = 0;
	virtual CString GetBreadCrumbs(const CString &strLocation, bool bFromIndex) = 0;
	virtual CString GetHome() = 0;

	virtual CSize ThumbnailSize(IW::FolderItemPtr pItem, CLoadAny *pLoader) = 0;
	virtual IW::ITEMLIST GetItemList(IW::Folder *pFolder, bool bCannotRecurse) = 0;
	virtual void ReplaceConstants(CString &strOut) = 0;
};

class CGalleryClient
{
public:

	virtual bool Init(bool bHasFrameHost) = 0;

	virtual bool OnFolder(const CString &strLocation, const CString &strPageOut, IW::IStatus *pStatus) = 0;

	virtual bool OnRequiredFile(const CString &strLocation, bool bCanBeInAuxFolder) = 0;
	virtual bool OnFrameHost(const CString &strLocation, const CString &strPageOut, IW::IStatus *pStatus) = 0;

	virtual bool OnIndex(const CString &strLocation, int nPage, const CString &strPageOut, IW::IStatus *pStatus) = 0;
	virtual bool OnImage(const CString &strLocation, const CString &strImage, bool bIsFolder, const CString &strPageOut, IW::IStatus *pStatus) = 0;
	virtual bool OnThumbnail(CLoadAny *pLoader, const CString &strLocation, const CString &strFileName, COLORREF clrBG, IW::IStatus *pStatus) = 0;
};

class CAddressPolicyPreview : public CAddressPolicy
{
protected:
	const CWebSettings &_settings;
	State &_state;

public:

	
	CAddressPolicyPreview(State &state, const CWebSettings &settings);
	
	CString GetIndexURL(const CString &strLocation, int i, bool bFromIndex);
	CString GetImageURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetFolderURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetThumbnailURL(const CString &strLocation, const CString &strName, COLORREF clrBG, bool bFromIndex);
	CString GetFileURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetImageFileURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetTemplateURL(const CString &strLocation, bool bFromIndex);
	CString GetHome();
	CString GetBreadCrumbs(const CString &strLocation, bool bFromIndex);
	
	virtual IW::ITEMLIST GetItemList(IW::Folder *pFolder, bool bCannotRecurse);
	virtual void ReplaceConstants(CString &strOut);
	virtual CSize ThumbnailSize(IW::FolderItemPtr pItem, CLoadAny *pLoader);
};

// This class is exported from the WebPage.dll
class  CWebPage
{
private:
	State &_state;

public:

	CWebPage(State &state);
	virtual ~CWebPage();

	CString GetProfileString(const CString &strSection, const CString &strEntry, const CString &strDefault);
	CString GetProfileString(const CString &strSection, const CString &strEntry, const CString &strDefault, const CString &strFileName);
	bool SetSettings(const CWebSettings &settings);

	bool ReplaceConstants(CString &str, CAddressPolicy *pAddress, const CString &strLocation, bool bIsFrameHost, bool bFromIndex);

	bool GetIndex(CAddressPolicy *pAddress, const CString &strLocation, int nPage, CString &strPageOut);
	bool GetImage(CAddressPolicy *pAddress, const CString &strLocation, const CString &strImage, int nPageReturn, CString &strPageOut);
	bool GetThumbnail(CLoadAny *pLoader, const CString &strLocation, const CString &strFileName, IW::Image &imageOut, COLORREF clrBG);

	bool IterateGallery(CGalleryClient *pClient, CAddressPolicy *pAddress, IW::Folder *pFolder, IW::IStatus *pStatus);

	void ProcessRequiredFiles(CGalleryClient* pClient);
	void ProcessStyleSheet(CGalleryClient* pClient);

	// Helpers
	int PopulateTemplates(HWND hCombo);
	int PopulateStyles(HWND hCombo);

protected:

	CCriticalSection _cs;

	// Build index
	bool GetIndex(IW::ITEMLIST &thumbs, CAddressPolicy *pAddress, const CString &strLocation, int nPage, CString &strPageOut);
	bool GetImage(IW::ITEMLIST &thumbs, CAddressPolicy *pAddress, const CString &strLocation, const CString &strImage, int nPageReturn, CString &strPageOut);
	bool ReplaceConstants(IW::ITEMLIST &thumbs, CString &str, CAddressPolicy *pAddress, const CString &strLocation, bool bIsFrameHost, bool bFromIndex);

	void LoadRequiredFilesList();

	// Template
	IW::CFilePath _pathTemplateFolder;
	IW::CFilePath _pathTemplateFile;
	IW::CFilePath _pathStyleSheetFile;

	CStringArray m_arTemplates;
	CStringArray m_arStyleSheets;

	IW::CFilePath _pathTemplateIndexFile;
	IW::CFilePath _pathTemplateImageFile;

	// Options
	CSize _sizeThumbNail;

	// Image Format
	CString _strType;

	// The Basics
	CStringArray m_arrayRequiredFiles;

	IW::CFilePath _pathTemplate;
	IW::CFilePath _pathImageTemplate;
	IW::CFilePath _pathFrameHostShort;

	// About to process text
	CString _strAboutToText;

	// Loaders and Filters
	bool m_bValid;	

	// State
	bool m_bHasImageTemplate;
	
	// Alternate Start file
	bool m_bFrameHost;
	IW::CFilePath _pathFrameHost;

	bool m_bNoThumbnails;
	bool m_bOnlyOneIndexPage;
	bool m_bOverRideCols;
	bool m_bCannotRecurse;

	

	CString _strTableEntry;
	CString _strFolderEntry;

	CWebSettings _settings;
	COLORREF m_clrBG;	

	friend class CAddressPolicyPreview;
	
};
