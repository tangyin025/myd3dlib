#pragma once

void PrintCallStack(std::ostringstream & ostr);

void WriteMiniDump(struct _EXCEPTION_POINTERS * pException, const TCHAR * FileName);
