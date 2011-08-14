
#include "stdafx.h"
#include "libc.h"
#include <cstdio>
#include <stdarg.h>
#include <crtdbg.h>
#include "myException.h"

char * asprintf(const char * format, ...)
{
	static const size_t def_size = 512;
	static const size_t inc_size = 512;

	_ASSERT(inc_size <= def_size);
	size_t new_size = def_size - inc_size;
	size_t ret_size = def_size;
	char * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += inc_size;
		FAILED_THROW_CUSEXCEPTION(NULL != (buffer = (char *)realloc(buffer, new_size * sizeof(char))));

		va_list args;
		va_start(args, format);
		ret_size = vsnprintf_s(buffer, new_size, new_size, format, args);
		va_end(args);
	}

	return buffer;
}

wchar_t * aswprintf(const wchar_t * format, ...)
{
	static const size_t def_size = 512;
	static const size_t inc_size = 512;

	_ASSERT(inc_size <= def_size);
	size_t new_size = def_size - inc_size;
	size_t ret_size = def_size;
	wchar_t * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += inc_size;
		FAILED_THROW_CUSEXCEPTION(NULL != (buffer = (wchar_t *)realloc(buffer, new_size * sizeof(wchar_t))));

		va_list args;
		va_start(args, format);
		ret_size = vswprintf(buffer, new_size, format, args);
		va_end(args);
	}

	return buffer;
}

char * avsprintf(const char * format, va_list args)
{
	static const size_t def_size = 512;
	static const size_t inc_size = 512;

	_ASSERT(inc_size <= def_size);
	size_t new_size = def_size - inc_size;
	size_t ret_size = def_size;
	char * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += inc_size;
		FAILED_THROW_CUSEXCEPTION(NULL != (buffer = (char *)realloc(buffer, new_size * sizeof(char))));

		ret_size = vsnprintf_s(buffer, new_size, new_size, format, args);
	}

	return buffer;
}

wchar_t * avswprintf(const wchar_t * format, va_list args)
{
	static const size_t def_size = 512;
	static const size_t inc_size = 512;

	_ASSERT(inc_size <= def_size);
	size_t new_size = def_size - inc_size;
	size_t ret_size = def_size;
	wchar_t * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += inc_size;
		FAILED_THROW_CUSEXCEPTION(NULL != (buffer = (wchar_t *)realloc(buffer, new_size * sizeof(wchar_t))));

		ret_size = vswprintf(buffer, new_size, format, args);
	}

	return buffer;
}

std::basic_string<char> str_printf(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	std::basic_string<char> ret;
	int nLen;
	for(ret.resize(512); -1 == (nLen = vsnprintf_s(&ret[0], ret.size(), ret.size(), format, args)); ret.resize(ret.size() + 512))
	{
	}
	va_end(args);

	ret.resize(nLen); // NOTE: not include the terminating null !
	return ret;
}

std::basic_string<wchar_t> str_printf(const wchar_t * format, ...)
{
	va_list args;
	va_start(args, format);
	std::basic_string<wchar_t> ret;
	int nLen;
	for(ret.resize(512); -1 == (nLen = vswprintf(&ret[0], ret.size(), format, args)); ret.resize(ret.size() + 512))
	{
	}
	va_end(args);

	ret.resize(nLen);
	return ret;
}

std::basic_string<wchar_t> mstringToWString(const std::basic_string<char> & mstr)
{
	int nLen;
	if(0 == (nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mstr.c_str(), -1, NULL, 0)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	std::basic_string<wchar_t> ret;
	ret.resize(nLen - 1);
	if(0 == (nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mstr.c_str(), -1, &ret[0], nLen)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	ret.resize(nLen - 1);
	return ret;
}

std::basic_string<char> wstringToMString(const std::basic_string<wchar_t> & wstr)
{
	int nLen;
	if(0 == (nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, wstr.c_str(), -1, NULL, 0, NULL, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	std::basic_string<char> ret;
	ret.resize(nLen - 1);
	if(0 == (nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, wstr.c_str(), -1, &ret[0], nLen, NULL, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	ret.resize(nLen - 1);
	return ret;
}
