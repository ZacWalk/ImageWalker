#include "stdafx.h"
#include "LoadAny.h"
#include "ThumbnailCache.h"

#include "..\Libraries\LibSQLite\sqlite3.h"
#include "Serialize.h"
#include "ZStream.h"
#include "Transactions.h"

IW::ThumbnailCacheThread::ThumbnailCacheThread(ThumbnailCache *pCache) : 
	_pCache(pCache), 
	_event(FALSE, FALSE), 
	IW::Thread(THREAD_PRIORITY_IDLE)
{
}

IW::ThumbnailCacheThread::~ThumbnailCacheThread()
{
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void IW::ThumbnailCacheThread::ProcessItems()
{
	IW::ThumbnailCache::ITEMLIST thumbList;
	_pCache->GetNextUpdateThumbnails(thumbList);

	if (thumbList.size() > 0)
	{
		int ret = _pCache->Exec(_T("BEGIN"));

		if (ret == SQLITE_OK) 
		{
			for(IW::ThumbnailCache::ITEMLIST::iterator it = thumbList.begin(); it != thumbList.end(); it++)
			{
				_pCache->UpdateThumbInCache(*it);
			}

			_pCache->Exec(_T("COMMIT"));
		}
	}
}

void IW::ThumbnailCacheThread::Process()
{
	App.Log(_T("Thumbnail Cache thread started"));		

	HANDLE objects[2];

	objects[0] = _event;
	objects[1] = IW::Thread::_eventExit;

	while(!_bExit)
	{
		DWORD dw = WaitForMultipleObjects(2, objects, FALSE, INFINITE);			

		switch(dw)
		{
		case WAIT_OBJECT_0:
			ProcessItems();
			break;

		case WAIT_OBJECT_0 + 1:
		case WAIT_FAILED:
			_bExit = true;
			break;
		}
	}
}	


IW::ThumbnailCache::ThumbnailCache(CString strDatabaseFileName) : 
	_thread(this),
	_db(0), 
	_pThumbTransaction(0), 
	_strDatabaseFileName(strDatabaseFileName)
{
	IW::CAutoLockCS lock(_cs);

	USES_CONVERSION;	

	bool bNeedsCreate = PathFileExists(strDatabaseFileName) == 0;

	int rc = sqlite3_open16(CT2W(strDatabaseFileName), &_db);

	if( rc )
	{
		ATLTRACE(_T("Can't open database: %s\n"), sqlite3_errmsg16(_db));
		sqlite3_close(_db);
		_db = 0;
	}
	else
	{
		if (bNeedsCreate)
		{
			CreateDatabase();
		}		
	}

}

IW::ThumbnailCache::~ThumbnailCache()
{
	Close();	
}

void IW::ThumbnailCache::CloseTransactions()
{
	IW::CAutoLockCS lock(_cs);

	if (_pThumbTransaction)
	{
		delete _pThumbTransaction;
		_pThumbTransaction = 0;
	}
}

void IW::ThumbnailCache::Close()
{
	//StopThread();
	IW::CAutoLockCS lock(_cs);
	
	try
	{
		CloseTransactions();
	}
	catch(database_exception &e)
	{
		USES_CONVERSION;
		ATLTRACE(_T("Database Exception %s \n"), CA2T(e.what()));
	}

	if (_db!= 0)
	{
		sqlite3_close(_db);
		_db= 0;
	}
}

CString IW::ThumbnailCache::GetDefaultDatabaseFileName()
{
	CString strFolderName = _T("ImageWalker");
	CString strFileName = _T("ImageWalkerDatabase.DB3");
	CShellItem item;

	if (item.Open(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE))
	{
		CString strPath = CShellDesktop().GetDisplayNameOf(item, SHGDN_FORPARSING);

		if (!strPath.IsEmpty())
		{
			IW::CFilePath path = strPath;
			path += strFolderName;
			path .CreateAllDirectories();
			path += strFileName;

			return path;
		}
	}	

	return IW::Path::Combine(_T("C:\\"), strFileName);
}

void IW::ThumbnailCache::CreateDatabase()
{
	HINSTANCE hInstance = App.GetResourceInstance();
	HRSRC hrsrc = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_CREATE_SQL), _T("SQL"));

	if (!hrsrc) 
	{
		throw std::exception("Failed to load bitmap resource"); 
	}

	HGLOBAL hg = ::LoadResource(hInstance, hrsrc);

	if (!hg) 
	{
		throw std::exception("Failed to load bitmap resource");
	}

	LPCSTR szSql = (LPCSTR)::LockResource(hg);
	DWORD nSize = SizeofResource(hInstance, hrsrc);

	CStringA strCreateStatements(szSql, nSize);

	CHAR *zErrMsg = NULL;
	int ret = sqlite3_exec(_db, strCreateStatements, 0, 0, &zErrMsg);

	if (ret != SQLITE_OK) 
	{
		throw std::exception("Failed to create Thumabnail Cache");
	}

	::FreeResource(hg);	
}

ThumbTransaction *IW::ThumbnailCache::GetThumbTransaction()
{
	IW::CAutoLockCS lock(_cs);

	if (_pThumbTransaction == 0) 
	{
		_pThumbTransaction = new ThumbTransaction(_db);
	}

	return _pThumbTransaction;
}

void IW::ThumbnailCache::StartThread() 
{ 
	_thread.StartThread(); 
};

void IW::ThumbnailCache::StopThread() 
{ 
	{
		IW::CAutoLockCS lock(_cs);
		_itemsToUpdate.clear();
	}

	_thread.StopThread(); 
};

bool IW::ThumbnailCache::FindAndLoadThumbFromCache(IW::FolderItem &thumb)
{
	try
	{
		IW::CAutoLockCS lock(_cs);
		IW::CFilePath path = thumb.GetFilePath();
		path.NormalizeForCompare();

		ITEMMAP::iterator it = _itemsToUpdate.find(path);

		if (it != _itemsToUpdate.end())
		{
			thumb.SetImage(it->second.GetImage());
			thumb.ModifyFlags(0, THUMB_LOADED | THUMB_INVALIDATE);
			return true;
		}
		else
		{
			return GetThumbTransaction()->Find(thumb, true);
		}
	}
	catch(database_exception &e)
	{
		USES_CONVERSION;
		ATLTRACE(_T("Database Exception %s \n"), CA2T(e.what()));
	}

	return false;
}

void IW::ThumbnailCache::QueueThumbCacheUpdate(FolderItem &thumb)
{
	IW::CAutoLockCS lock(_cs);
	IW::CFilePath path = thumb.GetFilePath();
	path.NormalizeForCompare();
	_itemsToUpdate[path] = thumb;

	if (_itemsToUpdate.size() > 64)
	{
		SyncCache();
	}
}

bool IW::ThumbnailCache::GetNextUpdateThumbnails(ITEMLIST &thumbList)
{
	IW::CAutoLockCS lock(_cs);
	if (_itemsToUpdate.size() == 0) return false;

	for(ITEMMAP::iterator it = _itemsToUpdate.begin(); it != _itemsToUpdate.end(); it++)
	{
		thumbList.push_back(it->second);
	}

	_itemsToUpdate.clear();

	return true;
}

void TraceMessage(database_exception &e)
{
	USES_CONVERSION;
	ATLTRACE(_T("Failed to Update Thumb in Cache because '%s' \n"), CA2T(e.what()));
}

void IW::ThumbnailCache::UpdateThumbInCache(IW::FolderItem &thumb)
{
	try
	{
		IW::CAutoLockCS lock(_cs);
		GetThumbTransaction()->Save(thumb);
	}
	catch(database_exception &e)
	{
		try
		{
			CloseTransactions();
		}
		catch(database_exception &e)
		{
			TraceMessage(e);
		}

		TraceMessage(e);
	}
}

int IW::ThumbnailCache::GetThumbnailCount() const
{
	CWaitCursor wait;

	try
	{
		IW::ThumbnailCache *pThis = const_cast<ThumbnailCache*>(this);
		IW::CAutoLockCS lock(pThis->_cs);		

		int nCount = 0;

		Transaction count(_db);
		count.Prepare(_T("SELECT COUNT(*) FROM thumbnail;"));

		if (count.Read()) 
		{
			nCount = count.GetInt(0);
		}

		return nCount;
	}
	catch(database_exception &e)
	{
		TraceMessage(e);
	}

	return -1;
}

int IW::ThumbnailCache::Exec(LPTSTR szSQL)
{
	IW::CAutoLockCS lock(_cs);
	USES_CONVERSION;
	return sqlite3_exec(_db, CT2CA(szSQL), NULL, NULL, NULL);
}

void IW::ThumbnailCache::Vacuum()
{
	CWaitCursor wait;

	try
	{
		IW::CAutoLockCS lock(_cs);
		CloseTransactions();
		Exec(_T("VACUUM"));
	}
	catch(database_exception &e)
	{
		TraceMessage(e);
	}	
}

void IW::ThumbnailCache::Clear()
{
	CWaitCursor wait;

	try
	{
		IW::CAutoLockCS lock(_cs);
		CloseTransactions();
		
		int returnCode = Exec(_T("delete from thumbnail"));

		if(returnCode != SQLITE_OK)
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(CString(sqlite3_errmsg(_db)), MB_OK|MB_ICONERROR);
		}
	}
	catch(database_exception &e)
	{
		TraceMessage(e);
	}
}