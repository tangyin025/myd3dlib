// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <iostream>
#include <tchar.h>

void PrintCallStack(std::ostringstream & ostr);

void WriteMiniDump(struct _EXCEPTION_POINTERS * pException, const TCHAR * FileName);
