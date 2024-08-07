// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "PrintCallStack.h"
#include <Windows.h>
#include <DbgHelp.h>
#include <sstream>

static bool g_SymInitialized = false;

void PrintCallStack(std::ostringstream & ostr)
{
	HANDLE hProcess = GetCurrentProcess();

	if (!g_SymInitialized)
	{
		g_SymInitialized = true;
		if (!SymInitialize(hProcess, NULL, TRUE)) {
			exit(0);
		}
	}

#ifndef _WIN64
	HANDLE hThread = GetCurrentThread();

	CONTEXT Context;
	ZeroMemory( &Context, sizeof( CONTEXT ) );
	Context.ContextFlags = CONTEXT_CONTROL;
	__asm {
	Label:
		mov[Context.Ebp], ebp;
		mov[Context.Esp], esp;
		mov eax, [Label];
		mov[Context.Eip], eax;
	}

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
#ifndef _WIN64
	StackFrame.AddrPC.Offset = Context.Eip;
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Offset = Context.Ebp;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Offset = Context.Esp;
	StackFrame.AddrStack.Mode = AddrModeFlat;
#else
	StackFrame.AddrPC.Offset = Context.Rip;
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Offset = Context.Rbp;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Offset = Context.Rsp;
	StackFrame.AddrStack.Mode = AddrModeFlat;
#endif
	ULONG64 buffer[(sizeof(SYMBOL_INFO) +
		MAX_SYM_NAME * sizeof(TCHAR) +
		sizeof(ULONG64) - 1) /
		sizeof(ULONG64)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	while (1) {
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &StackFrame, &Context, &ReadMemoryRoutine::Proc, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			break;
		}
		if (StackFrame.AddrPC.Offset == 0) {
			break;
		}
		DWORD64 Displacement64;
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
#else
	// https://stackoverflow.com/questions/590160/how-to-log-stack-frames-with-windows-x64
    // Quote from Microsoft Documentation:
    // ## Windows Server 2003 and Windows XP:  
    // ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
    const int kMaxCallers = 62;

    void* callers_stack[kMaxCallers];
	unsigned short frames = CaptureStackBackTrace(0, kMaxCallers, callers_stack, NULL);
	ULONG64 buffer[(sizeof(SYMBOL_INFO) +
		MAX_SYM_NAME * sizeof(TCHAR) +
		sizeof(ULONG64) - 1) /
		sizeof(ULONG64)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

    for (unsigned int i = 0; i < frames; i++)
    {
		DWORD64 Displacement64;
		if (!SymFromAddr(hProcess, (DWORD64)(callers_stack[i]), &Displacement64, pSymbol)) {
			strcpy_s(pSymbol->Name, MAX_SYM_NAME, "unknown symbol name");
		}
		DWORD Displacement;
		IMAGEHLP_LINE64 Line;
		ZeroMemory(&Line, sizeof(Line));
		Line.SizeOfStruct = sizeof(Line);
		if (!SymGetLineFromAddr64(hProcess, (DWORD64)(callers_stack[i]), &Displacement, &Line)) {
			Line.FileName = "unknown file name";
		}
		ostr << Line.FileName << " (" << Line.LineNumber << "): " << pSymbol->Name << std::endl;
	}
#endif
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
