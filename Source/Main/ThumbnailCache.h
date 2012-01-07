#pragma once

#include "Threads.h"
#include "Items.h"

struct sqlite3;
class ThumbTransaction;


namespace IW
{
class FolderItem;
class ThumbnailCache;

class ThumbnailCacheThread : public IW::Thread
{
private:
	CEvent _event;	
	CCriticalSection _cs;
	ThumbnailCache *_pCache;

public:

	ThumbnailCacheThread(ThumbnailCache *pCache);
	~ThumbnailCacheThread();

	void SyncCache() { _event.Set(); };
	void QueueThumbnail(FolderItem &item);

	void ProcessItems();
	void Process();

};

class ThumbnailCache
{
private:
	sqlite3 *_db;
	CCriticalSection _cs;
	CString _strDatabaseFileName;

	static CString GetDefaultDatabaseFileName();	
	void CreateDatabase();

	ThumbTransaction *_pThumbTransaction;
	ThumbTransaction *GetThumbTransaction();

	ThumbnailCacheThread _thread;

	
	typedef std::map<CString, FolderItem> ITEMMAP;
	ITEMMAP _itemsToUpdate;

	ThumbnailCache(const ThumbnailCache &);
	void operator=(const ThumbnailCache &);

public:

	ThumbnailCache(CString strDatabaseFileName = GetDefaultDatabaseFileName());
	~ThumbnailCache(void);	

	CString GetFilePath() const { return _strDatabaseFileName; };
	void Close();
	void CloseTransactions();
	void Vacuum();
	void Clear();
	int  GetThumbnailCount() const;
	
	typedef std::vector<FolderItem> ITEMLIST;
	bool GetNextUpdateThumbnails(ITEMLIST &thumbList);

	void QueueThumbCacheUpdate(FolderItem &thumb);

	int Exec(LPTSTR szSQL);


	void StartThread();
	void StopThread();

	bool FindAndLoadThumbFromCache(FolderItem &thumb);
	void UpdateThumbInCache(FolderItem &thumb);	
	void SyncCache() { _thread.SyncCache(); };
};


} // namespace // IW