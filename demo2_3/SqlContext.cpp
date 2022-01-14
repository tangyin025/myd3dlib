#include "stdafx.h"
#include "SqlContext.h"
#include <sqlite3.h>

SqlContext::SqlContext(void)
	: db(NULL)
{
}

SqlContext::~SqlContext(void)
{
	if (db)
	{
		Close();
	}
}

void SqlContext::Open(const char* filename)
{
	_ASSERT(!db);

	if (SQLITE_OK != sqlite3_open(filename, &db))
	{
		THROW_CUSEXCEPTION(sqlite3_errmsg(db));
	}
}

void SqlContext::Close(void)
{
	_ASSERT(db);

	if (SQLITE_OK != sqlite3_close(db))
	{
		THROW_CUSEXCEPTION("sqlite3_close failed");
	}

	db = NULL;
}

void SqlContext::Exec(const char* sql, int (*callback)(void*, int, char**, char**), void* data)
{
	char* zErrMsg = 0;
	if (SQLITE_OK != sqlite3_exec(db, sql, callback, data, &zErrMsg))
	{
		boost::shared_ptr<char> autofree(zErrMsg, sqlite3_free);
		THROW_CUSEXCEPTION(zErrMsg);
	}
}
