#pragma once

void PrintCallStack(const CONTEXT * pContext, std::ostringstream & ostr);

void WriteMiniDump(struct _EXCEPTION_POINTERS * pException, const TCHAR * FileName);
