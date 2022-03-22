#pragma once

struct sqlite3;

class SqlConnection
{
public:
	sqlite3 * db;

public:
	SqlConnection(const char* filename);

	~SqlConnection(void);

	void Open(const char* filename);

	void Close(void);

	void Exec(const char* sql, int (*callback)(void*, int, char**, char**), void* data);

	void Clone(const char* other);
};
