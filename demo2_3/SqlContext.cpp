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
}

static int callback(void* data, int argc, char** argv, char** azColName) {
	int i;
	for (i = 0; i < argc; i++) {
		my::D3DContext::getSingleton().m_EventLog(str_printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL").c_str());
	}
	my::D3DContext::getSingleton().m_EventLog("\n");
	return 0;
}

void SqlContext::Exec(const char* sql)
{
	char* zErrMsg = 0;
	if (SQLITE_OK != sqlite3_exec(db, sql, callback, 0, &zErrMsg))
	{
		boost::shared_ptr<char> autofree(zErrMsg, sqlite3_free);
		THROW_CUSEXCEPTION(zErrMsg);
	}
}
