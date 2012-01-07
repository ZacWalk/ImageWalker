#pragma once

class database_exception : public std::exception
{
public:

	database_exception(sqlite3 *db) : std::exception(sqlite3_errmsg(db))
	{
	}

};

class Transaction
{
public:

	sqlite3_stmt *_pStatementHandle;
	const void *_unusedPortionOfSql;
	sqlite3 *_db;

	Transaction(sqlite3 *db) : 
		_db(db), 
		_pStatementHandle(0),
		_unusedPortionOfSql(0)
	{
	}

	~Transaction()
	{
		if (_pStatementHandle != 0)
		{
			int returnCode = sqlite3_finalize(_pStatementHandle);
			_pStatementHandle = 0;

			if (returnCode != SQLITE_OK)
			{
				throw database_exception(_db);
			}
		}
	}

	void Prepare(LPCTSTR szSQL)
	{
		int returnCode = sqlite3_prepare16(_db,  szSQL,  -1,  &_pStatementHandle,  &_unusedPortionOfSql);

		if (returnCode != SQLITE_OK)
		{
			throw database_exception(_db);
		}
	}	

	void Bind(LPCTSTR szText, int nBindVar)
	{
		int returnCode = sqlite3_bind_text16(_pStatementHandle, nBindVar, szText, -1, SQLITE_TRANSIENT);

		if(returnCode != SQLITE_OK)
			throw database_exception(_db);
	}

	void Bind(int n, int nBindVar)
	{
		int returnCode = sqlite3_bind_int(_pStatementHandle, nBindVar, n);

		if(returnCode != SQLITE_OK)
			throw database_exception(_db);
	}

	void Bind(unsigned n, int nBindVar)
	{
		int returnCode = sqlite3_bind_int(_pStatementHandle, nBindVar, static_cast<int>(n));

		if(returnCode != SQLITE_OK)
			throw database_exception(_db);
	}

	void Bind(__int64 n, int nBindVar)
	{
		int returnCode = sqlite3_bind_int64(_pStatementHandle, nBindVar, n);

		if(returnCode != SQLITE_OK)
			throw database_exception(_db);
	}

	void Bind(LPCBYTE pData, int nLen, int nBindVar)
	{
		int returnCode = sqlite3_bind_blob(_pStatementHandle, nBindVar, pData, nLen, SQLITE_TRANSIENT);

		if(returnCode != SQLITE_OK)
			throw database_exception(_db);
	}

	

	void Reset()
	{
		if(sqlite3_reset(_pStatementHandle) != SQLITE_OK)
			throw database_exception(_db);
	}

	bool Read()
	{
		int result = sqlite3_step(_pStatementHandle);

		switch(result) 
		{
		case SQLITE_ROW:
			return true;
		case SQLITE_DONE:
			return false;
		default:
			throw database_exception(_db);
		}
	}

	bool Exec()
	{
		return Read();
		return true;
	}

	int GetInt(int nColumn)
	{
		return sqlite3_column_int(_pStatementHandle, nColumn);
	}

	__int64 GetInt64(int nColumn)
	{
		return sqlite3_column_int64(_pStatementHandle, nColumn);
	}

	LPCTSTR GetText(int nColumn)
	{
		return (LPCTSTR)sqlite3_column_text16(_pStatementHandle, nColumn);
	}

	IW::SimpleBlob GetBlob(int nColumn)
	{
		LPCBYTE pData = (LPCBYTE)sqlite3_column_blob(_pStatementHandle, nColumn);
		int nLen = sqlite3_column_bytes(_pStatementHandle, nColumn);

		return IW::SimpleBlob(pData, nLen);
	}

	sqlite_int64 GetLastId()
	{
		return sqlite3_last_insert_rowid(_db);
	}	
};


class ThumbTransaction
{
public:

	Transaction _find;
	Transaction _insert;
	Transaction _update;

	struct ColumnNumber
	{
		enum 
		{
			fileNameHash = 1, fileName, lastWriteTime, lastReadTime, imageData, id
		};
	};

	ThumbTransaction(sqlite3 *db) : _find(db), _insert(db), _update(db)
	{
		LPCTSTR szFind = _T("select filename, lastWriteTime, lastReadTime, imageData, id from thumbnail where fileNameHash=?"); 
		_find.Prepare(szFind);

		LPCTSTR szInsert = _T("insert into thumbnail (fileNameHash, fileName, lastWriteTime, lastReadTime, imageData) values (?, ?, ?, ?, ?);");
		_insert.Prepare(szInsert);

		LPCTSTR szUpdate = _T("update thumbnail set fileNameHash=?, fileName=?, lastWriteTime=?, lastReadTime=?, imageData=? where id=?;");
		_update.Prepare(szUpdate);
	}

	~ThumbTransaction()
	{
	}

	inline int HashKey(LPCTSTR key) const
	{
		int nHash = 0;
		
		while (*key)
			nHash = (nHash<<4) + nHash + *key++;

		return nHash;
	}

	bool Find(IW::FolderItem &thumb, bool bLoad)
	{
		IW::CFilePath pathFile = thumb.GetCacheName();
		pathFile.NormalizeForCompare();

		int nHash = HashKey(pathFile);

		_find.Reset();
		_find.Bind(nHash, 1); 

		while(_find.Read())
		{
			CString strPath = _find.GetText(ColumnNumber::fileName - 2);

			if (strPath == pathFile.ToString())
			{
				thumb.SetCachedThumbnailId(_find.GetInt64(ColumnNumber::id - 2));

				if (bLoad)
				{
					IW::FileTime cachedLastWriteTime = _find.GetInt64(ColumnNumber::lastWriteTime - 2);

					if (cachedLastWriteTime == thumb.GetLastWriteTime())
					{
						IW::SimpleBlob &data = _find.GetBlob(ColumnNumber::imageData - 2);

						if (!data.empty())
						{
							LoadThumbImage(data, thumb);							
							thumb.ModifyFlags(0, THUMB_LOADED | THUMB_INVALIDATE);
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	void Save(IW::FolderItem &thumb)
	{
		if (thumb.GetCachedThumbnailId() == -1)
		{
			InsertThumb(thumb);
		}
		else
		{
			UpdateThumb(thumb);
		}
	}

	void InsertThumb(IW::FolderItem &thumb)
	{
		_insert.Reset();
		BindThumbValues(_insert, thumb);
		_insert.Exec();

		thumb.SetCachedThumbnailId(_insert.GetLastId());
	}

	void UpdateThumb(const IW::FolderItem &thumb)
	{
		_update.Reset();
		BindThumbValues(_update, thumb);
		_update.Bind(thumb.GetCachedThumbnailId(), ColumnNumber::id);
		_update.Exec();
	}

	void BindThumbValues(Transaction &transaction, const IW::FolderItem &thumb)
	{
		IW::CFilePath pathFile = thumb.GetCacheName();
		pathFile.NormalizeForCompare();

		IW::FileTime now = IW::FileTime::Now();

		transaction.Bind(HashKey(pathFile), ColumnNumber::fileNameHash);
		transaction.Bind(pathFile, ColumnNumber::fileName);
		transaction.Bind(thumb.GetLastWriteTime().ToInt64(), ColumnNumber::lastWriteTime);
		transaction.Bind(now.ToInt64(), ColumnNumber::lastReadTime);

		BindThumbImage(transaction, thumb.GetImage());
	}

	void BindThumbImage(Transaction &transaction, const IW::Image &image)
	{ 
		if (!image.IsEmpty())
		{
			IW::Serialize::ArchiveStore archive;
			const_cast<IW::Image*>(&image)->Serialize(archive);

			IW::SimpleBlob data;
			IW::StreamBlob<IW::SimpleBlob>  streamOut(data);
			IW::zostream<IW::StreamBlob<IW::SimpleBlob> > zstreamOut(streamOut);
			archive.Store(zstreamOut);
			zstreamOut.Close();

			int nSize = streamOut.GetFileSize();
			transaction.Bind(data.GetData(), nSize, ColumnNumber::imageData);
		}
	}

	void LoadThumbImage(IW::SimpleBlob &data, IW::FolderItem &thumb)
	{
		IW::StreamConstBlob streamIn(data);
		IW::zistream<IW::StreamConstBlob> zstreamIn(streamIn);
		IW::Serialize::ArchiveLoad archive(zstreamIn);
		IW::Image image; 
		image.Serialize(archive);
		thumb.SetImage(image);
	}
};