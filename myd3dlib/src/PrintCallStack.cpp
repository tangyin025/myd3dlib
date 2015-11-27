#include "StdAfx.h"
#include "PrintCallStack.h"
#include <DbgHelp.h>
#include <sstream>

void PrintCallStack(const CONTEXT * pContext, std::ostringstream & ostr)
{
	HANDLE hProcess = GetCurrentProcess();

	if (!SymInitialize(hProcess, NULL, TRUE)) {
		exit(0);
	}

	HANDLE hThread = GetCurrentThread();

	CONTEXT Context = *pContext;

	struct ReadMemoryRoutine {
		static BOOL CALLBACK Proc(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead) {
			SIZE_T st;
			BOOL bRet = ReadProcessMemory(hProcess, (LPVOID)lpBaseAddress, lpBuffer, nSize, &st);
			*lpNumberOfBytesRead = st;
			return bRet;
		}
	};

	STACKFRAME64 StackFrame;
	ZeroMemory(&StackFrame, sizeof(StackFrame));
	StackFrame.AddrPC.Offset    = Context.Eip;
	StackFrame.AddrPC.Mode      = AddrModeFlat;
	StackFrame.AddrFrame.Offset = Context.Ebp;
	StackFrame.AddrFrame.Mode   = AddrModeFlat;
	StackFrame.AddrStack.Offset = Context.Esp;
	StackFrame.AddrStack.Mode   = AddrModeFlat;
	while (1) {
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &StackFrame, &Context, &ReadMemoryRoutine::Proc, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			break;
		}
		if (StackFrame.AddrPC.Offset == 0) {
			break;
		}
		DWORD64 Displacement64;
		ULONG64 buffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
		pSymbol->SizeOfStruct = sizeof(buffer);
		pSymbol->MaxNameLen = MAX_SYM_NAME;
		if (!SymFromAddr(hProcess, StackFrame.AddrPC.Offset, &Displacement64, pSymbol)) {
			strcpy_s(pSymbol->Name, MAX_SYM_NAME, "unknown symbol name");
		}
		DWORD Displacement;
		IMAGEHLP_LINE64 Line;
		ZeroMemory(&Line, sizeof(Line));
		Line.SizeOfStruct = sizeof(Line);
		if (!SymGetLineFromAddr64(hProcess, StackFrame.AddrPC.Offset, &Displacement, &Line)) {
			Line.FileName = "unknown file name";
		}
		ostr << Line.FileName << " (" << Line.LineNumber << "): " << pSymbol->Name << std::endl;
	}
}

void WriteMiniDump(struct _EXCEPTION_POINTERS * pException, const TCHAR * FileName)
{
	HANDLE hFile = CreateFile(FileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		exit(0);
	}
	MINIDUMP_EXCEPTION_INFORMATION cbif;
	cbif.ThreadId = GetCurrentThreadId();
	cbif.ClientPointers = TRUE;
	cbif.ExceptionPointers = pException;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &cbif, NULL, NULL);
	CloseHandle(hFile);
}
