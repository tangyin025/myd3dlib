#pragma once

struct sqlite3;

class SqlContext
{
public:
	sqlite3 * db;

public:
	SqlContext(const char* filename);

	~SqlContext(void);

	void Open(const char* filename);

	void Close(void);

	void Exec(const char* sql, int (*callback)(void*, int, char**, char**), void* data);

	void Clone(const char* other);
};
