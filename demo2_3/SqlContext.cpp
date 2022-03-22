#include "stdafx.h"
#include "SqlContext.h"
#include <sqlite3.h>
#include <boost/scope_exit.hpp>

using namespace my;
//
//#pragma push_macro("max")
//#undef max
//// Using SQLite with std::iostream, https://stackoverflow.com/questions/3839158/using-sqlite-with-stdiostream
//std::string getIostreamVFSName() {
//    // a mutex protects the body of this function because we don't want to register the VFS twice
//    static Mutex mutex;
//    MutexLock lock(mutex);
//
//    // check if the VFS is already registered, in this case we directly return
//    static const char* vfsName = "iostream_vfs_handler";
//    if (sqlite3_vfs_find(vfsName) != nullptr)
//        return vfsName;
//
//    // this is the structure that will store all the custom informations about an opened file
//    // all the functions get in fact pointer to an sqlite3_file object
//    // we give SQLite the size of this structure and SQLite will allocate it for us
//    // 'xOpen' will have to call all the members' constructors (using placement-new), and 'xClose' will call all the destructors
//    struct File : sqlite3_file {
//        std::shared_ptr<std::iostream>      stream;         // pointer to the source stream
//        int                                 lockLevel;      // level of lock by SQLite ; goes from 0 (not locked) to 4 (exclusive lock)
//    };
//
//    // making sure that the 'sqlite3_file' structure is at offset 0 in the 'File' structure
//    static_assert(offsetof(File, pMethods) == 0, "Wrong data alignment in custom SQLite3 VFS, lots of weird errors will happen during runtime");
//
//    // structure which contains static functions that we are going to pass to SQLite
//    // TODO: VC++2010 doesn't support lambda function treated as regular functions, or we would use this
//    struct Functions {
//        // opens a file by filling a sqlite3_file structure
//        // the name of the file should be the offset in memory where to find a "std::shared_ptr<std::iostream>"
//        // eg. you create a "std::shared_ptr<std::iostream>" whose memory location is 0x12345678
//        //      you have to pass "12345678" as the file name
//        // this function will make a copy of the shared_ptr and store it in the sqlite3_file
//        static int xOpen(sqlite3_vfs*, const char* zName, sqlite3_file* fileBase, int flags, int* pOutFlags) {
//
//            // filling a structure with a list of methods that will be used by SQLite3 for this particular file
//            static sqlite3_io_methods methods;
//            methods.iVersion = 1;
//            methods.xClose = &xClose;
//            methods.xRead = &xRead;
//            methods.xWrite = &xWrite;
//            methods.xTruncate = &xTruncate;
//            methods.xSync = &xSync;
//            methods.xFileSize = &xFileSize;
//            methods.xLock = &xLock;
//            methods.xUnlock = &xUnlock;
//            methods.xCheckReservedLock = &xCheckReservedLock;
//            methods.xFileControl = &xFileControl;
//            methods.xSectorSize = &xSectorSize;
//            methods.xDeviceCharacteristics = &xDeviceCharacteristics;
//            fileBase->pMethods = &methods;
//
//            // SQLite allocated a buffer large enough to use it as a "File" object (see above)
//            auto fileData = static_cast<File*>(fileBase);
//            fileData->lockLevel = 0;
//
//            // if the name of the file doesn't contain a lexical_cast'ed pointer, then this is not our main DB file
//            //  (note: the flags can also be used to determine this)
//            if (zName == nullptr || strlen(zName) != sizeof(void*) * 2) {
//
//                assert(flags | SQLITE_OPEN_CREATE);
//                // if this is not our main DB file, we create a temporary stringstream that will be deleted when the file is closed
//                // this behavior is different than expected from a file system (where file are permanent)
//                //   but SQLite seems to accept it
//                new (&fileData->stream) std::shared_ptr<std::iostream>(std::make_shared<std::stringstream>(std::ios_base::in | std::ios_base::out | std::ios_base::binary));
//
//            }
//            else {
//                // decoding our pointer, ie. un-lexical_cast'ing it
//                std::stringstream filenameStream(zName);
//                void* sharedPtrAddress = nullptr;
//                filenameStream >> sharedPtrAddress;
//                // our pointer points to a shared_ptr<std::iostream>, we make a copy of it
//                new (&fileData->stream) std::shared_ptr<std::iostream>(*static_cast<std::shared_ptr<std::iostream>*>(sharedPtrAddress));
//            }
//
//            assert(fileData->stream->good());
//
//            // I don't really know what to output as flags
//            // the "winOpen" implementation only sets either "readwrite" or "readonly"
//            if (pOutFlags != nullptr)
//                *pOutFlags = SQLITE_OPEN_READWRITE;
//            return SQLITE_OK;
//        }
//
//        static int xClose(sqlite3_file* fileBase) {
//            auto fileData = static_cast<File*>(fileBase);
//            assert(!fileData->stream->fail());
//
//            // we have to manually call the destructors of the objects in the structure
//            //   because we created them with placement-new
//            fileData->stream.~shared_ptr();
//            return SQLITE_OK;
//        }
//
//        static int xRead(sqlite3_file* fileBase, void* buffer, int quantity, sqlite3_int64 offset) {
//            auto fileData = static_cast<File*>(fileBase);
//            assert(fileData->stream);
//            assert(fileData->stream->good());
//
//            fileData->stream->sync();
//
//            // we try to seek to the offset we want to read
//            fileData->stream->seekg(offset, std::ios::beg);
//            // if this fails, we'll just tell SQLite that we couldn't read the quantity it wanted
//            if (fileData->stream->fail()) {
//                fileData->stream->clear();
//                memset(static_cast<char*>(buffer), 0, quantity);
//                return SQLITE_IOERR_SHORT_READ;
//            }
//
//            // reading data
//            fileData->stream->read(static_cast<char*>(buffer), quantity);
//            fileData->stream->clear();
//
//            // if we reached EOF, gcount will be < to the quantity we have to read
//            // if this happens, SQLite asks us to fill the rest of the buffer with 0s
//            const auto gcount = fileData->stream->gcount();
//            if (gcount < quantity) {
//                memset(static_cast<char*>(buffer) + gcount, 0, static_cast<size_t>(quantity - gcount));
//                return SQLITE_IOERR_SHORT_READ;
//            }
//
//            return SQLITE_OK;
//        }
//
//        static int xWrite(sqlite3_file* fileBase, const void* buffer, int quantity, sqlite3_int64 offset) {
//            auto fileData = static_cast<File*>(fileBase);
//            assert(!fileData->stream->fail());
//
//            fileData->stream->sync();
//
//            // contrary to reading operating, SQLite doesn't accept partial writes
//            // either we succeed or we fail
//            fileData->stream->seekp(offset, std::ios::beg);
//            if (fileData->stream->fail()) {
//                fileData->stream->clear();
//                return SQLITE_IOERR_WRITE;
//            }
//
//            fileData->stream->write(static_cast<const char*>(buffer), quantity);
//
//            if (fileData->stream->fail()) {
//                fileData->stream->clear();
//                return SQLITE_IOERR_WRITE;
//            }
//
//            return SQLITE_OK;
//        }
//
//        static int xTruncate(sqlite3_file* fileBase, sqlite3_int64 size) {
//            // it is not possible to truncate a stream
//            // it makes sense to truncate a file or a buffer, but not a generic stream
//            // however it is possible to implement the xTruncate function as a no-op
//            return SQLITE_OK;
//        }
//
//        static int xSync(sqlite3_file* fileBase, int) {
//            // the flag passed as parameter is supposed to make a difference between a "partial sync" and a "full sync"
//            // we don't care and just call sync
//            auto fileData = static_cast<File*>(fileBase);
//            return fileData->stream->sync();
//        }
//
//        static int xFileSize(sqlite3_file* fileBase, sqlite3_int64* outputSize) {
//            // this function outputs the size of the file, wherever the read pointer or write pointer is
//
//            auto fileData = static_cast<File*>(fileBase);
//            assert(!fileData->stream->fail());
//
//            // we don't care about restoring the previous read pointer location,
//            //   since the next operation will move it anyway
//            *outputSize = fileData->stream->seekg(0, std::ios::end).tellg();
//            assert(*outputSize != -1);
//
//            if (fileData->stream->fail())
//                fileData->stream->clear();
//
//            return SQLITE_OK;
//        }
//
//        static int xLock(sqlite3_file* fileBase, int level) {
//            auto fileData = static_cast<File*>(fileBase);
//            assert(level < std::numeric_limits<decltype(fileData->lockLevel)>::max());
//            fileData->lockLevel = level;
//            return SQLITE_OK;
//        }
//
//        static int xUnlock(sqlite3_file* fileBase, int level) {
//            auto fileData = static_cast<File*>(fileBase);
//            assert(level >= 0);
//            fileData->lockLevel = level;
//            return SQLITE_OK;
//        }
//
//        static int xCheckReservedLock(sqlite3_file* fileBase, int* pResOut) {
//            // this function outputs "true" if the file is locked,
//            //   ie. if its lock level is >= 1
//            auto fileData = static_cast<File*>(fileBase);
//            *pResOut = (fileData->lockLevel >= 1);
//            return SQLITE_OK;
//        }
//
//        static int xFileControl(sqlite3_file* fileBase, int op, void* pArg) {
//            // this function is bit weird because it's supposed to handle generic operations
//            // the 'op' parameter is the operation code, and 'pArg' points to the arguments of the operation
//
//            auto fileData = static_cast<File*>(fileBase);
//
//            switch (op) {
//            case SQLITE_FCNTL_LOCKSTATE:
//                // outputs the current lock level of the file in reinterpret_cast<int*>(pArg)
//                *reinterpret_cast<int*>(pArg) = fileData->lockLevel;
//                break;
//
//            case SQLITE_FCNTL_SIZE_HINT:
//                // gives a hint about the size of the final file in reinterpret_cast<int*>(pArg)
//                break;
//
//            case SQLITE_FCNTL_CHUNK_SIZE:
//                // gives a hint about the size of blocks of data that SQLite will write at once
//                break;
//
//                // some operations are not documented (and not used in practice),
//                //   so I'll leave them alone
//            case SQLITE_GET_LOCKPROXYFILE:      return SQLITE_ERROR;
//            case SQLITE_SET_LOCKPROXYFILE:      return SQLITE_ERROR;
//            case SQLITE_LAST_ERRNO:             return SQLITE_ERROR;
//            }
//
//            return SQLITE_OK;
//        }
//
//        static int xSectorSize(sqlite3_file*) {
//            // returns the size of a sector of the HDD,
//            //   we just return a dummy value
//            return 512;
//        }
//
//        static int xDeviceCharacteristics(sqlite3_file*) {
//            // returns the capabilities of the HDD
//            // see http://www.sqlite.org/c3ref/c_iocap_atomic.html
//            return SQLITE_IOCAP_ATOMIC | SQLITE_IOCAP_SAFE_APPEND | SQLITE_IOCAP_SEQUENTIAL;
//        }
//
//        static int xDelete(sqlite3_vfs*, const char* zName, int syncDir) {
//            // deletes a file ; this is called on 'journal' or 'wal' files
//            // these files are treated temporary by 'xOpen' (see above) and are destroyed when 'xClose' is called anyway
//            return SQLITE_OK;
//        }
//
//        static int xAccess(sqlite3_vfs*, const char* zName, int flags, int* pResOut) {
//            // depending on the value of 'flags':
//            //   * outputs true if the file exists
//            //   * outputs true if the file can be read
//            //   * outputs true if the file can be written
//            // we handle all cases at once by returning true only if the file is the name of our main database
//            *pResOut = (strlen(zName) == sizeof(void*) * 2);
//            return SQLITE_OK;
//        }
//
//        static int xFullPathname(sqlite3_vfs*, const char* zName, int nOut, char* zOut) {
//            // this function turns a relative path into an absolute path
//            // since our file names are just lexical_cast'ed pointers, we just strcpy
//            strcpy_s(zOut, nOut, zName);
//            return SQLITE_OK;
//        }
//
//        static int xRandomness(sqlite3_vfs*, int nByte, char* zOut) {
//            // this function generates a random serie of characters to write in 'zOut'
//            // we use C++0x's <random> features
//            //static std::mt19937 randomGenerator;
//            //static std::uniform_int<char> randomDistributor;
//
//            for (auto i = 0; i < nByte; ++i)
//                //zOut[i] = randomDistributor(randomGenerator);
//                zOut[i] = Random(UCHAR_MAX);
//            return SQLITE_OK;
//        }
//
//        static int xSleep(sqlite3_vfs*, int microseconds) {
//            //std::this_thread::sleep(std::chrono::microseconds(microseconds));
//            ::Sleep(microseconds);
//            return SQLITE_OK;
//        }
//
//        static int xCurrentTime(sqlite3_vfs*, double* output) {
//            // this function should return the number of days elapsed since
//            //   "noon in Greenwich on November 24, 4714 B.C according to the proleptic Gregorian calendar"
//            // I picked this constant from sqlite3.c which will make our life easier
//            static const double unixEpoch = 2440587.5;
//
//            *output = unixEpoch + double(time(nullptr)) / (60. * 60. * 24.);
//            return SQLITE_OK;
//        }
//
//        static int xCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64* output) {
//            // this function should return the number of milliseconds elapsed since
//            //   "noon in Greenwich on November 24, 4714 B.C according to the proleptic Gregorian calendar"
//            // I picked this constant from sqlite3.c which will make our life easier
//            // note: I wonder if it is not hundredth of seconds instead
//            static const sqlite3_int64 unixEpoch = 24405875 * sqlite3_int64(60 * 60 * 24 * 100);
//
//            *output = unixEpoch + time(nullptr) * 1000;
//            return SQLITE_OK;
//        }
//
//    };
//
//
//    // creating the VFS structure
//    // TODO: some functions are not implemented due to lack of documentation ; I'll have to read sqlite3.c to find out
//    static sqlite3_vfs readStructure;
//    memset(&readStructure, 0, sizeof(readStructure));
//    readStructure.iVersion = 2;
//    readStructure.szOsFile = sizeof(File);
//    readStructure.mxPathname = 256;
//    readStructure.zName = vfsName;
//    readStructure.pAppData = nullptr;
//    readStructure.xOpen = &Functions::xOpen;
//    readStructure.xDelete = &Functions::xDelete;
//    readStructure.xAccess = &Functions::xAccess;
//    readStructure.xFullPathname = &Functions::xFullPathname;
//    /*readStructure.xDlOpen = &Functions::xOpen;
//    readStructure.xDlError = &Functions::xOpen;
//    readStructure.xDlSym = &Functions::xOpen;
//    readStructure.xDlClose = &Functions::xOpen;*/
//    readStructure.xRandomness = &Functions::xRandomness;
//    readStructure.xSleep = &Functions::xSleep;
//    readStructure.xCurrentTime = &Functions::xCurrentTime;
//    //readStructure.xGetLastError = &Functions::xOpen;
//    readStructure.xCurrentTimeInt64 = &Functions::xCurrentTimeInt64;
//
//
//    // the second parameter of this function tells if
//    //   it should be made the default file system
//    sqlite3_vfs_register(&readStructure, false);
//
//    return vfsName;
//}
//#pragma pop_macro("max")

#ifndef SQLITE_PRIVATE
# define SQLITE_PRIVATE static
#endif

typedef sqlite3_uint64 u64;
typedef unsigned char u8;

typedef struct sqlite3expert sqlite3expert;

/*
** These are the allowed modes.
*/
#define MODE_Line     0  /* One column per line.  Blank line between records */
#define MODE_Column   1  /* One record per line in neat columns */
#define MODE_List     2  /* One record per line with a separator */
#define MODE_Semi     3  /* Same as MODE_List but append ";" to each line */
#define MODE_Html     4  /* Generate an XHTML table */
#define MODE_Insert   5  /* Generate SQL "insert" statements */
#define MODE_Quote    6  /* Quote values as for SQL */
#define MODE_Tcl      7  /* Generate ANSI-C or TCL quoted elements */
#define MODE_Csv      8  /* Quote strings, numbers are plain */
#define MODE_Explain  9  /* Like MODE_Column, but do not truncate data */
#define MODE_Ascii   10  /* Use ASCII unit and record separators (0x1F/0x1E) */
#define MODE_Pretty  11  /* Pretty-print schemas */
#define MODE_EQP     12  /* Converts EXPLAIN QUERY PLAN output into a graph */
#define MODE_Json    13  /* Output JSON */
#define MODE_Markdown 14 /* Markdown formatting */
#define MODE_Table   15  /* MySQL-style table formatting */
#define MODE_Box     16  /* Unicode box-drawing characters */

static const char* modeDescr[] = {
  "line",
  "column",
  "list",
  "semi",
  "html",
  "insert",
  "quote",
  "tcl",
  "csv",
  "explain",
  "ascii",
  "prettyprint",
  "eqp",
  "json",
  "markdown",
  "table",
  "box"
};

/*
** These are the column/row/line separators used by the various
** import/export modes.
*/
#define SEP_Column    "|"
#define SEP_Row       "\n"
#define SEP_Tab       "\t"
#define SEP_Space     " "
#define SEP_Comma     ","
#define SEP_CrLf      "\r\n"
#define SEP_Unit      "\x1F"
#define SEP_Record    "\x1E"

/*
** These are the allowed shellFlgs values
*/
#define SHFLG_Pagecache      0x00000001 /* The --pagecache option is used */
#define SHFLG_Lookaside      0x00000002 /* Lookaside memory is used */
#define SHFLG_Backslash      0x00000004 /* The --backslash option is used */
#define SHFLG_PreserveRowid  0x00000008 /* .dump preserves rowid values */
#define SHFLG_Newlines       0x00000010 /* .dump --newline flag */
#define SHFLG_CountChanges   0x00000020 /* .changes setting */
#define SHFLG_Echo           0x00000040 /* .echo or --echo setting */
#define SHFLG_HeaderSet      0x00000080 /* showHeader has been specified */
#define SHFLG_DumpDataOnly   0x00000100 /* .dump show data only */
#define SHFLG_DumpNoSys      0x00000200 /* .dump omits system tables */

typedef struct ExpertInfo ExpertInfo;
struct ExpertInfo {
    sqlite3expert* pExpert;
    int bVerbose;
};

/* A single line in the EQP output */
typedef struct EQPGraphRow EQPGraphRow;
struct EQPGraphRow {
    int iEqpId;           /* ID for this row */
    int iParentId;        /* ID of the parent row */
    EQPGraphRow* pNext;   /* Next row in sequence */
    char zText[1];        /* Text to display for this row */
};

/* All EQP output is collected into an instance of the following */
typedef struct EQPGraph EQPGraph;
struct EQPGraph {
    EQPGraphRow* pRow;    /* Linked list of all rows of the EQP output */
    EQPGraphRow* pLast;   /* Last element of the pRow list */
    char zPrefix[100];    /* Graph prefix */
};

/*
** State information about the database connection is contained in an
** instance of the following structure.
*/
typedef struct ShellState ShellState;
struct ShellState {
    sqlite3* db;           /* The database */
    u8 autoExplain;        /* Automatically turn on .explain mode */
    u8 autoEQP;            /* Run EXPLAIN QUERY PLAN prior to seach SQL stmt */
    u8 autoEQPtest;        /* autoEQP is in test mode */
    u8 autoEQPtrace;       /* autoEQP is in trace mode */
    u8 scanstatsOn;        /* True to display scan stats before each finalize */
    u8 openMode;           /* SHELL_OPEN_NORMAL, _APPENDVFS, or _ZIPFILE */
    u8 doXdgOpen;          /* Invoke start/open/xdg-open in output_reset() */
    u8 nEqpLevel;          /* Depth of the EQP output graph */
    u8 eTraceType;         /* SHELL_TRACE_* value for type of trace */
    u8 bSafeMode;          /* True to prohibit unsafe operations */
    u8 bSafeModePersist;   /* The long-term value of bSafeMode */
    unsigned statsOn;      /* True to display memory stats before each finalize */
    unsigned mEqpLines;    /* Mask of veritical lines in the EQP output graph */
    int outCount;          /* Revert to stdout when reaching zero */
    int cnt;               /* Number of records displayed so far */
    int lineno;            /* Line number of last line read from in */
    int openFlags;         /* Additional flags to open.  (SQLITE_OPEN_NOFOLLOW) */
    FILE* in;              /* Read commands from this stream */
    FILE* out;             /* Write results here */
    FILE* traceOut;        /* Output for sqlite3_trace() */
    int nErr;              /* Number of errors seen */
    int mode;              /* An output mode setting */
    int modePrior;         /* Saved mode */
    int cMode;             /* temporary output mode for the current query */
    int normalMode;        /* Output mode before ".explain on" */
    int writableSchema;    /* True if PRAGMA writable_schema=ON */
    int showHeader;        /* True to show column names in List or Column mode */
    int nCheck;            /* Number of ".check" commands run */
    unsigned nProgress;    /* Number of progress callbacks encountered */
    unsigned mxProgress;   /* Maximum progress callbacks before failing */
    unsigned flgProgress;  /* Flags for the progress callback */
    unsigned shellFlgs;    /* Various flags */
    unsigned priorShFlgs;  /* Saved copy of flags */
    sqlite3_int64 szMax;   /* --maxsize argument to .open */
    char* zDestTable;      /* Name of destination table when MODE_Insert */
    char* zTempFile;       /* Temporary file that might need deleting */
    char zTestcase[30];    /* Name of current test case */
    char colSeparator[20]; /* Column separator character for several modes */
    char rowSeparator[20]; /* Row separator character for MODE_Ascii */
    char colSepPrior[20];  /* Saved column separator */
    char rowSepPrior[20];  /* Saved row separator */
    int* colWidth;         /* Requested width of each column in columnar modes */
    int* actualWidth;      /* Actual width of each column */
    int nWidth;            /* Number of slots in colWidth[] and actualWidth[] */
    char nullValue[20];    /* The text to print when a NULL comes back from
                           ** the database */
    char outfile[FILENAME_MAX]; /* Filename for *out */
    sqlite3_stmt* pStmt;   /* Current statement if any. */
    FILE* pLog;            /* Write log output here */
    struct AuxDb {         /* Storage space for auxiliary database connections */
        sqlite3* db;               /* Connection pointer */
        const char* zDbFilename;   /* Filename used to open the connection */
        char* zFreeOnClose;        /* Free this memory allocation on close */
#if defined(SQLITE_ENABLE_SESSION)
        int nSession;              /* Number of active sessions */
        OpenSession aSession[4];   /* Array of sessions.  [0] is in focus. */
#endif
    } aAuxDb[5],           /* Array of all database connections */
        * pAuxDb;             /* Currently active database connection */
    int* aiIndent;         /* Array of indents used in MODE_Explain */
    int nIndent;           /* Size of array aiIndent[] */
    int iIndent;           /* Index of current op in aiIndent[] */
    char* zNonce;          /* Nonce for temporary safe-mode excapes */
    EQPGraph sGraph;       /* Information for the graphical EXPLAIN QUERY PLAN */
    ExpertInfo expert;     /* Valid if previous command was ".expert OPT..." */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
static int strlen30(const char* z) {
    const char* z2 = z;
    while (*z2) { z2++; }
    return 0x3fffffff & (int)(z2 - z);
}

/*
** Render output like fprintf().  This should not be used on anything that
** includes string formatting (e.g. "%s").
*/
#if !defined(raw_printf)
# define raw_printf fprintf
#endif

/* Indicate out-of-memory and exit. */
static void shell_out_of_memory(void) {
    raw_printf(stderr, "Error: out of memory\n");
    exit(1);
}

/*
** On Windows systems we have to know if standard output is a console
** in order to translate UTF-8 into MBCS.  The following variable is
** true if translation is required.
*/
static int stdout_is_console = 1;

extern "C" {
    /* string conversion routines only needed on Win32 */
    extern char* sqlite3_win32_mbcs_to_utf8_v2(const char*, int);
    extern char* sqlite3_win32_utf8_to_mbcs_v2(const char*, int);
}

/*
** Render output like fprintf().  Except, if the output is going to the
** console and if this is running on a Windows machine, translate the
** output from UTF-8 into MBCS.
*/
#if defined(_WIN32) || defined(WIN32)
void utf8_printf(FILE* out, const char* zFormat, ...) {
    va_list ap;
    va_start(ap, zFormat);
    if (stdout_is_console && (out == stdout || out == stderr)) {
        char* z1 = sqlite3_vmprintf(zFormat, ap);
        char* z2 = sqlite3_win32_utf8_to_mbcs_v2(z1, 0);
        sqlite3_free(z1);
        fputs(z2, out);
        sqlite3_free(z2);
    }
    else {
        vfprintf(out, zFormat, ap);
    }
    va_end(ap);
}
#elif !defined(utf8_printf)
# define utf8_printf fprintf
#endif

/*
** A callback for the sqlite3_log() interface.
*/
static void shellLog(void* pArg, int iErrCode, const char* zMsg) {
    ShellState* p = (ShellState*)pArg;
    if (p->pLog == 0) return;
    utf8_printf(p->pLog, "(%d) %s\n", iErrCode, zMsg);
    fflush(p->pLog);
}

/*
** Attempt to close the databaes connection.  Report errors.
*/
void close_db(sqlite3* db) {
    int rc = sqlite3_close(db);
    if (rc) {
        utf8_printf(stderr, "Error: sqlite3_close() returns %d: %s\n",
            rc, sqlite3_errmsg(db));
    }
}

/*
** Internal check:  Verify that the SQLite is uninitialized.  Print a
** error message if it is initialized.
*/
static void verify_uninitialized(void) {
    if (sqlite3_config(-1) == SQLITE_MISUSE) {
        utf8_printf(stdout, "WARNING: attempt to configure SQLite after"
            " initialization.\n");
    }
}

/*
** Try to transfer data for table zTable.  If an error is seen while
** moving forward, try to go backwards.  The backwards movement won't
** work for WITHOUT ROWID tables.
*/
static void tryToCloneData(
    ShellState* p,
    sqlite3* newDb,
    const char* zTable
) {
    sqlite3_stmt* pQuery = 0;
    sqlite3_stmt* pInsert = 0;
    char* zQuery = 0;
    char* zInsert = 0;
    int rc;
    int i, j, n;
    int nTable = strlen30(zTable);
    int k = 0;
    int cnt = 0;
    const int spinRate = 10000;

    zQuery = sqlite3_mprintf("SELECT * FROM \"%w\"", zTable);
    rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
    if (rc) {
        utf8_printf(stderr, "Error %d: %s on [%s]\n",
            sqlite3_extended_errcode(p->db), sqlite3_errmsg(p->db),
            zQuery);
        goto end_data_xfer;
    }
    n = sqlite3_column_count(pQuery);
    zInsert = (char*)sqlite3_malloc64(200 + nTable + n * 3);
    if (zInsert == 0) shell_out_of_memory();
    sqlite3_snprintf(200 + nTable, zInsert,
        "INSERT OR IGNORE INTO \"%s\" VALUES(?", zTable);
    i = strlen30(zInsert);
    for (j = 1; j < n; j++) {
        memcpy(zInsert + i, ",?", 2);
        i += 2;
    }
    memcpy(zInsert + i, ");", 3);
    rc = sqlite3_prepare_v2(newDb, zInsert, -1, &pInsert, 0);
    if (rc) {
        utf8_printf(stderr, "Error %d: %s on [%s]\n",
            sqlite3_extended_errcode(newDb), sqlite3_errmsg(newDb),
            zQuery);
        goto end_data_xfer;
    }
    for (k = 0; k < 2; k++) {
        while ((rc = sqlite3_step(pQuery)) == SQLITE_ROW) {
            for (i = 0; i < n; i++) {
                switch (sqlite3_column_type(pQuery, i)) {
                case SQLITE_NULL: {
                    sqlite3_bind_null(pInsert, i + 1);
                    break;
                }
                case SQLITE_INTEGER: {
                    sqlite3_bind_int64(pInsert, i + 1, sqlite3_column_int64(pQuery, i));
                    break;
                }
                case SQLITE_FLOAT: {
                    sqlite3_bind_double(pInsert, i + 1, sqlite3_column_double(pQuery, i));
                    break;
                }
                case SQLITE_TEXT: {
                    sqlite3_bind_text(pInsert, i + 1,
                        (const char*)sqlite3_column_text(pQuery, i),
                        -1, SQLITE_STATIC);
                    break;
                }
                case SQLITE_BLOB: {
                    sqlite3_bind_blob(pInsert, i + 1, sqlite3_column_blob(pQuery, i),
                        sqlite3_column_bytes(pQuery, i),
                        SQLITE_STATIC);
                    break;
                }
                }
            } /* End for */
            rc = sqlite3_step(pInsert);
            if (rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
                utf8_printf(stderr, "Error %d: %s\n", sqlite3_extended_errcode(newDb),
                    sqlite3_errmsg(newDb));
            }
            sqlite3_reset(pInsert);
            cnt++;
            if ((cnt % spinRate) == 0) {
                printf("%c\b", "|/-\\"[(cnt / spinRate) % 4]);
                fflush(stdout);
            }
        } /* End while */
        if (rc == SQLITE_DONE) break;
        sqlite3_finalize(pQuery);
        sqlite3_free(zQuery);
        zQuery = sqlite3_mprintf("SELECT * FROM \"%w\" ORDER BY rowid DESC;",
            zTable);
        rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
        if (rc) {
            utf8_printf(stderr, "Warning: cannot step \"%s\" backwards", zTable);
            break;
        }
    } /* End for(k=0...) */

end_data_xfer:
    sqlite3_finalize(pQuery);
    sqlite3_finalize(pInsert);
    sqlite3_free(zQuery);
    sqlite3_free(zInsert);
}


/*
** Try to transfer all rows of the schema that match zWhere.  For
** each row, invoke xForEach() on the object defined by that row.
** If an error is encountered while moving forward through the
** sqlite_schema table, try again moving backwards.
*/
static void tryToCloneSchema(
    ShellState* p,
    sqlite3* newDb,
    const char* zWhere,
    void (*xForEach)(ShellState*, sqlite3*, const char*)
) {
    sqlite3_stmt* pQuery = 0;
    char* zQuery = 0;
    int rc;
    const unsigned char* zName;
    const unsigned char* zSql;
    char* zErrMsg = 0;

    zQuery = sqlite3_mprintf("SELECT name, sql FROM sqlite_schema"
        " WHERE %s", zWhere);
    rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
    if (rc) {
        utf8_printf(stderr, "Error: (%d) %s on [%s]\n",
            sqlite3_extended_errcode(p->db), sqlite3_errmsg(p->db),
            zQuery);
        goto end_schema_xfer;
    }
    while ((rc = sqlite3_step(pQuery)) == SQLITE_ROW) {
        zName = sqlite3_column_text(pQuery, 0);
        zSql = sqlite3_column_text(pQuery, 1);
        printf("%s... ", zName); fflush(stdout);
        sqlite3_exec(newDb, (const char*)zSql, 0, 0, &zErrMsg);
        if (zErrMsg) {
            utf8_printf(stderr, "Error: %s\nSQL: [%s]\n", zErrMsg, zSql);
            sqlite3_free(zErrMsg);
            zErrMsg = 0;
        }
        if (xForEach) {
            xForEach(p, newDb, (const char*)zName);
        }
        printf("done\n");
    }
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(pQuery);
        sqlite3_free(zQuery);
        zQuery = sqlite3_mprintf("SELECT name, sql FROM sqlite_schema"
            " WHERE %s ORDER BY rowid DESC", zWhere);
        rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
        if (rc) {
            utf8_printf(stderr, "Error: (%d) %s on [%s]\n",
                sqlite3_extended_errcode(p->db), sqlite3_errmsg(p->db),
                zQuery);
            goto end_schema_xfer;
        }
        while (sqlite3_step(pQuery) == SQLITE_ROW) {
            zName = sqlite3_column_text(pQuery, 0);
            zSql = sqlite3_column_text(pQuery, 1);
            printf("%s... ", zName); fflush(stdout);
            sqlite3_exec(newDb, (const char*)zSql, 0, 0, &zErrMsg);
            if (zErrMsg) {
                utf8_printf(stderr, "Error: %s\nSQL: [%s]\n", zErrMsg, zSql);
                sqlite3_free(zErrMsg);
                zErrMsg = 0;
            }
            if (xForEach) {
                xForEach(p, newDb, (const char*)zName);
            }
            printf("done\n");
        }
    }
end_schema_xfer:
    sqlite3_finalize(pQuery);
    sqlite3_free(zQuery);
}

/*
** Open a new database file named "zNewDb".  Try to recover as much information
** as possible out of the main database (which might be corrupt) and write it
** into zNewDb.
*/
static void tryToClone(ShellState* p, const char* zNewDb) {
    int rc;
    sqlite3* newDb = 0;
    if (access(zNewDb, 0) == 0) {
        utf8_printf(stderr, "File \"%s\" already exists.\n", zNewDb);
        return;
    }
    rc = sqlite3_open(zNewDb, &newDb);
    if (rc) {
        utf8_printf(stderr, "Cannot create output database: %s\n",
            sqlite3_errmsg(newDb));
    }
    else {
        sqlite3_exec(p->db, "PRAGMA writable_schema=ON;", 0, 0, 0);
        sqlite3_exec(newDb, "BEGIN EXCLUSIVE;", 0, 0, 0);
        tryToCloneSchema(p, newDb, "type='table'", tryToCloneData);
        tryToCloneSchema(p, newDb, "type!='table'", 0);
        sqlite3_exec(newDb, "COMMIT;", 0, 0, 0);
        sqlite3_exec(p->db, "PRAGMA writable_schema=OFF;", 0, 0, 0);
    }
    close_db(newDb);
}

/*
** Prompt strings. Initialized in main. Settable with
**   .prompt main continue
*/
static char mainPrompt[20];     /* First line prompt. default: "sqlite> "*/
static char continuePrompt[20]; /* Continuation prompt. default: "   ...> " */

/*
** Initialize the state information in data
*/
static void main_init(ShellState* data) {
    memset(data, 0, sizeof(*data));
    data->normalMode = data->cMode = data->mode = MODE_List;
    data->autoExplain = 1;
    data->pAuxDb = &data->aAuxDb[0];
    memcpy(data->colSeparator, SEP_Column, 2);
    memcpy(data->rowSeparator, SEP_Row, 2);
    data->showHeader = 0;
    data->shellFlgs = SHFLG_Lookaside;
    verify_uninitialized();
    sqlite3_config(SQLITE_CONFIG_URI, 1);
    sqlite3_config(SQLITE_CONFIG_LOG, shellLog, data);
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
    sqlite3_snprintf(sizeof(mainPrompt), mainPrompt, "sqlite> ");
    sqlite3_snprintf(sizeof(continuePrompt), continuePrompt, "   ...> ");
}

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

void SqlContext::Clone(const char* other)
{
    ShellState data;
    main_init(&data);
    data.db = db;
    tryToClone(&data, other);
}
