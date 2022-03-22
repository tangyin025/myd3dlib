#include "stdafx.h"
#include "SqlContext.h"
#include <sqlite3.h>
#include <boost/scope_exit.hpp>

using namespace my;

#pragma push_macro("max")
#undef max
// Using SQLite with std::iostream, https://stackoverflow.com/questions/3839158/using-sqlite-with-stdiostream
std::string getIostreamVFSName() {
    // a mutex protects the body of this function because we don't want to register the VFS twice
    static Mutex mutex;
    MutexLock lock(mutex);

    // check if the VFS is already registered, in this case we directly return
    static const char* vfsName = "iostream_vfs_handler";
    if (sqlite3_vfs_find(vfsName) != nullptr)
        return vfsName;

    // this is the structure that will store all the custom informations about an opened file
    // all the functions get in fact pointer to an sqlite3_file object
    // we give SQLite the size of this structure and SQLite will allocate it for us
    // 'xOpen' will have to call all the members' constructors (using placement-new), and 'xClose' will call all the destructors
    struct File : sqlite3_file {
        std::shared_ptr<std::iostream>      stream;         // pointer to the source stream
        int                                 lockLevel;      // level of lock by SQLite ; goes from 0 (not locked) to 4 (exclusive lock)
    };

    // making sure that the 'sqlite3_file' structure is at offset 0 in the 'File' structure
    static_assert(offsetof(File, pMethods) == 0, "Wrong data alignment in custom SQLite3 VFS, lots of weird errors will happen during runtime");

    // structure which contains static functions that we are going to pass to SQLite
    // TODO: VC++2010 doesn't support lambda function treated as regular functions, or we would use this
    struct Functions {
        // opens a file by filling a sqlite3_file structure
        // the name of the file should be the offset in memory where to find a "std::shared_ptr<std::iostream>"
        // eg. you create a "std::shared_ptr<std::iostream>" whose memory location is 0x12345678
        //      you have to pass "12345678" as the file name
        // this function will make a copy of the shared_ptr and store it in the sqlite3_file
        static int xOpen(sqlite3_vfs*, const char* zName, sqlite3_file* fileBase, int flags, int* pOutFlags) {

            // filling a structure with a list of methods that will be used by SQLite3 for this particular file
            static sqlite3_io_methods methods;
            methods.iVersion = 1;
            methods.xClose = &xClose;
            methods.xRead = &xRead;
            methods.xWrite = &xWrite;
            methods.xTruncate = &xTruncate;
            methods.xSync = &xSync;
            methods.xFileSize = &xFileSize;
            methods.xLock = &xLock;
            methods.xUnlock = &xUnlock;
            methods.xCheckReservedLock = &xCheckReservedLock;
            methods.xFileControl = &xFileControl;
            methods.xSectorSize = &xSectorSize;
            methods.xDeviceCharacteristics = &xDeviceCharacteristics;
            fileBase->pMethods = &methods;

            // SQLite allocated a buffer large enough to use it as a "File" object (see above)
            auto fileData = static_cast<File*>(fileBase);
            fileData->lockLevel = 0;

            // if the name of the file doesn't contain a lexical_cast'ed pointer, then this is not our main DB file
            //  (note: the flags can also be used to determine this)
            if (zName == nullptr || strlen(zName) != sizeof(void*) * 2) {

                assert(flags | SQLITE_OPEN_CREATE);
                // if this is not our main DB file, we create a temporary stringstream that will be deleted when the file is closed
                // this behavior is different than expected from a file system (where file are permanent)
                //   but SQLite seems to accept it
                new (&fileData->stream) std::shared_ptr<std::iostream>(std::make_shared<std::stringstream>(std::ios_base::in | std::ios_base::out | std::ios_base::binary));

            }
            else {
                // decoding our pointer, ie. un-lexical_cast'ing it
                std::stringstream filenameStream(zName);
                void* sharedPtrAddress = nullptr;
                filenameStream >> sharedPtrAddress;
                // our pointer points to a shared_ptr<std::iostream>, we make a copy of it
                new (&fileData->stream) std::shared_ptr<std::iostream>(*static_cast<std::shared_ptr<std::iostream>*>(sharedPtrAddress));
            }

            assert(fileData->stream->good());

            // I don't really know what to output as flags
            // the "winOpen" implementation only sets either "readwrite" or "readonly"
            if (pOutFlags != nullptr)
                *pOutFlags = SQLITE_OPEN_READWRITE;
            return SQLITE_OK;
        }

        static int xClose(sqlite3_file* fileBase) {
            auto fileData = static_cast<File*>(fileBase);
            assert(!fileData->stream->fail());

            // we have to manually call the destructors of the objects in the structure
            //   because we created them with placement-new
            fileData->stream.~shared_ptr();
            return SQLITE_OK;
        }

        static int xRead(sqlite3_file* fileBase, void* buffer, int quantity, sqlite3_int64 offset) {
            auto fileData = static_cast<File*>(fileBase);
            assert(fileData->stream);
            assert(fileData->stream->good());

            fileData->stream->sync();

            // we try to seek to the offset we want to read
            fileData->stream->seekg(offset, std::ios::beg);
            // if this fails, we'll just tell SQLite that we couldn't read the quantity it wanted
            if (fileData->stream->fail()) {
                fileData->stream->clear();
                memset(static_cast<char*>(buffer), 0, quantity);
                return SQLITE_IOERR_SHORT_READ;
            }

            // reading data
            fileData->stream->read(static_cast<char*>(buffer), quantity);
            fileData->stream->clear();

            // if we reached EOF, gcount will be < to the quantity we have to read
            // if this happens, SQLite asks us to fill the rest of the buffer with 0s
            const auto gcount = fileData->stream->gcount();
            if (gcount < quantity) {
                memset(static_cast<char*>(buffer) + gcount, 0, static_cast<size_t>(quantity - gcount));
                return SQLITE_IOERR_SHORT_READ;
            }

            return SQLITE_OK;
        }

        static int xWrite(sqlite3_file* fileBase, const void* buffer, int quantity, sqlite3_int64 offset) {
            auto fileData = static_cast<File*>(fileBase);
            assert(!fileData->stream->fail());

            fileData->stream->sync();

            // contrary to reading operating, SQLite doesn't accept partial writes
            // either we succeed or we fail
            fileData->stream->seekp(offset, std::ios::beg);
            if (fileData->stream->fail()) {
                fileData->stream->clear();
                return SQLITE_IOERR_WRITE;
            }

            fileData->stream->write(static_cast<const char*>(buffer), quantity);

            if (fileData->stream->fail()) {
                fileData->stream->clear();
                return SQLITE_IOERR_WRITE;
            }

            return SQLITE_OK;
        }

        static int xTruncate(sqlite3_file* fileBase, sqlite3_int64 size) {
            // it is not possible to truncate a stream
            // it makes sense to truncate a file or a buffer, but not a generic stream
            // however it is possible to implement the xTruncate function as a no-op
            return SQLITE_OK;
        }

        static int xSync(sqlite3_file* fileBase, int) {
            // the flag passed as parameter is supposed to make a difference between a "partial sync" and a "full sync"
            // we don't care and just call sync
            auto fileData = static_cast<File*>(fileBase);
            return fileData->stream->sync();
        }

        static int xFileSize(sqlite3_file* fileBase, sqlite3_int64* outputSize) {
            // this function outputs the size of the file, wherever the read pointer or write pointer is

            auto fileData = static_cast<File*>(fileBase);
            assert(!fileData->stream->fail());

            // we don't care about restoring the previous read pointer location,
            //   since the next operation will move it anyway
            *outputSize = fileData->stream->seekg(0, std::ios::end).tellg();
            assert(*outputSize != -1);

            if (fileData->stream->fail())
                fileData->stream->clear();

            return SQLITE_OK;
        }

        static int xLock(sqlite3_file* fileBase, int level) {
            auto fileData = static_cast<File*>(fileBase);
            assert(level < std::numeric_limits<decltype(fileData->lockLevel)>::max());
            fileData->lockLevel = level;
            return SQLITE_OK;
        }

        static int xUnlock(sqlite3_file* fileBase, int level) {
            auto fileData = static_cast<File*>(fileBase);
            assert(level >= 0);
            fileData->lockLevel = level;
            return SQLITE_OK;
        }

        static int xCheckReservedLock(sqlite3_file* fileBase, int* pResOut) {
            // this function outputs "true" if the file is locked,
            //   ie. if its lock level is >= 1
            auto fileData = static_cast<File*>(fileBase);
            *pResOut = (fileData->lockLevel >= 1);
            return SQLITE_OK;
        }

        static int xFileControl(sqlite3_file* fileBase, int op, void* pArg) {
            // this function is bit weird because it's supposed to handle generic operations
            // the 'op' parameter is the operation code, and 'pArg' points to the arguments of the operation

            auto fileData = static_cast<File*>(fileBase);

            switch (op) {
            case SQLITE_FCNTL_LOCKSTATE:
                // outputs the current lock level of the file in reinterpret_cast<int*>(pArg)
                *reinterpret_cast<int*>(pArg) = fileData->lockLevel;
                break;

            case SQLITE_FCNTL_SIZE_HINT:
                // gives a hint about the size of the final file in reinterpret_cast<int*>(pArg)
                break;

            case SQLITE_FCNTL_CHUNK_SIZE:
                // gives a hint about the size of blocks of data that SQLite will write at once
                break;

                // some operations are not documented (and not used in practice),
                //   so I'll leave them alone
            case SQLITE_GET_LOCKPROXYFILE:      return SQLITE_ERROR;
            case SQLITE_SET_LOCKPROXYFILE:      return SQLITE_ERROR;
            case SQLITE_LAST_ERRNO:             return SQLITE_ERROR;
            }

            return SQLITE_OK;
        }

        static int xSectorSize(sqlite3_file*) {
            // returns the size of a sector of the HDD,
            //   we just return a dummy value
            return 512;
        }

        static int xDeviceCharacteristics(sqlite3_file*) {
            // returns the capabilities of the HDD
            // see http://www.sqlite.org/c3ref/c_iocap_atomic.html
            return SQLITE_IOCAP_ATOMIC | SQLITE_IOCAP_SAFE_APPEND | SQLITE_IOCAP_SEQUENTIAL;
        }

        static int xDelete(sqlite3_vfs*, const char* zName, int syncDir) {
            // deletes a file ; this is called on 'journal' or 'wal' files
            // these files are treated temporary by 'xOpen' (see above) and are destroyed when 'xClose' is called anyway
            return SQLITE_OK;
        }

        static int xAccess(sqlite3_vfs*, const char* zName, int flags, int* pResOut) {
            // depending on the value of 'flags':
            //   * outputs true if the file exists
            //   * outputs true if the file can be read
            //   * outputs true if the file can be written
            // we handle all cases at once by returning true only if the file is the name of our main database
            *pResOut = (strlen(zName) == sizeof(void*) * 2);
            return SQLITE_OK;
        }

        static int xFullPathname(sqlite3_vfs*, const char* zName, int nOut, char* zOut) {
            // this function turns a relative path into an absolute path
            // since our file names are just lexical_cast'ed pointers, we just strcpy
            strcpy_s(zOut, nOut, zName);
            return SQLITE_OK;
        }

        static int xRandomness(sqlite3_vfs*, int nByte, char* zOut) {
            // this function generates a random serie of characters to write in 'zOut'
            // we use C++0x's <random> features
            //static std::mt19937 randomGenerator;
            //static std::uniform_int<char> randomDistributor;

            for (auto i = 0; i < nByte; ++i)
                //zOut[i] = randomDistributor(randomGenerator);
                zOut[i] = Random(UCHAR_MAX);
            return SQLITE_OK;
        }

        static int xSleep(sqlite3_vfs*, int microseconds) {
            //std::this_thread::sleep(std::chrono::microseconds(microseconds));
            ::Sleep(microseconds);
            return SQLITE_OK;
        }

        static int xCurrentTime(sqlite3_vfs*, double* output) {
            // this function should return the number of days elapsed since
            //   "noon in Greenwich on November 24, 4714 B.C according to the proleptic Gregorian calendar"
            // I picked this constant from sqlite3.c which will make our life easier
            static const double unixEpoch = 2440587.5;

            *output = unixEpoch + double(time(nullptr)) / (60. * 60. * 24.);
            return SQLITE_OK;
        }

        static int xCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64* output) {
            // this function should return the number of milliseconds elapsed since
            //   "noon in Greenwich on November 24, 4714 B.C according to the proleptic Gregorian calendar"
            // I picked this constant from sqlite3.c which will make our life easier
            // note: I wonder if it is not hundredth of seconds instead
            static const sqlite3_int64 unixEpoch = 24405875 * sqlite3_int64(60 * 60 * 24 * 100);

            *output = unixEpoch + time(nullptr) * 1000;
            return SQLITE_OK;
        }

    };


    // creating the VFS structure
    // TODO: some functions are not implemented due to lack of documentation ; I'll have to read sqlite3.c to find out
    static sqlite3_vfs readStructure;
    memset(&readStructure, 0, sizeof(readStructure));
    readStructure.iVersion = 2;
    readStructure.szOsFile = sizeof(File);
    readStructure.mxPathname = 256;
    readStructure.zName = vfsName;
    readStructure.pAppData = nullptr;
    readStructure.xOpen = &Functions::xOpen;
    readStructure.xDelete = &Functions::xDelete;
    readStructure.xAccess = &Functions::xAccess;
    readStructure.xFullPathname = &Functions::xFullPathname;
    /*readStructure.xDlOpen = &Functions::xOpen;
    readStructure.xDlError = &Functions::xOpen;
    readStructure.xDlSym = &Functions::xOpen;
    readStructure.xDlClose = &Functions::xOpen;*/
    readStructure.xRandomness = &Functions::xRandomness;
    readStructure.xSleep = &Functions::xSleep;
    readStructure.xCurrentTime = &Functions::xCurrentTime;
    //readStructure.xGetLastError = &Functions::xOpen;
    readStructure.xCurrentTimeInt64 = &Functions::xCurrentTimeInt64;


    // the second parameter of this function tells if
    //   it should be made the default file system
    sqlite3_vfs_register(&readStructure, false);

    return vfsName;
}
#pragma pop_macro("max")

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
        BOOST_SCOPE_EXIT(this_)
        {
            this_->Close();
        }
        BOOST_SCOPE_EXIT_END;
        
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
