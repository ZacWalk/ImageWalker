

CREATE TABLE thumbnail (
	id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	fileNameHash INTEGER,
	fileName TEXT,
	lastWriteTime INTEGER64,
	lastReadTime INTEGER64,
	imageData BLOB
);

CREATE INDEX imageNameIndex ON thumbnail(fileNameHash);
