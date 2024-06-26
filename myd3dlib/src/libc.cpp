// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "libc.h"
#include <stdarg.h>
#include <exception>
#include <Windows.h>

#define FAILED_THROW_STDEXCEPTION(expr) \
	{ \
		if(!(expr)) \
		{ \
			throw std::exception(#expr); \
		} \
	}

#define DEF_BUFF_SIZE 512
#define DEF_INCR_SIZE 512

char * asprintf(const char * format, ...)
{
	_ASSERT(DEF_INCR_SIZE <= DEF_BUFF_SIZE);
	size_t new_size = DEF_BUFF_SIZE - DEF_INCR_SIZE;
	size_t ret_size = DEF_BUFF_SIZE;
	char * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += DEF_INCR_SIZE;
		FAILED_THROW_STDEXCEPTION(NULL != (buffer = (char *)realloc(buffer, new_size * sizeof(char))));

		va_list args;
		va_start(args, format);
		ret_size = vsnprintf_s(buffer, new_size, new_size, format, args);
		va_end(args);
	}

	return buffer;
}

wchar_t * aswprintf(const wchar_t * format, ...)
{
	_ASSERT(DEF_INCR_SIZE <= DEF_BUFF_SIZE);
	size_t new_size = DEF_BUFF_SIZE - DEF_INCR_SIZE;
	size_t ret_size = DEF_BUFF_SIZE;
	wchar_t * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += DEF_INCR_SIZE;
		FAILED_THROW_STDEXCEPTION(NULL != (buffer = (wchar_t *)realloc(buffer, new_size * sizeof(wchar_t))));

		va_list args;
		va_start(args, format);
		ret_size = vswprintf_s(buffer, new_size, format, args);
		va_end(args);
	}

	return buffer;
}

char * avsprintf(const char * format, va_list args)
{
	_ASSERT(DEF_INCR_SIZE <= DEF_BUFF_SIZE);
	size_t new_size = DEF_BUFF_SIZE - DEF_INCR_SIZE;
	size_t ret_size = DEF_BUFF_SIZE;
	char * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += DEF_INCR_SIZE;
		FAILED_THROW_STDEXCEPTION(NULL != (buffer = (char *)realloc(buffer, new_size * sizeof(char))));

		ret_size = vsnprintf_s(buffer, new_size, new_size, format, args);
	}

	return buffer;
}

wchar_t * avswprintf(const wchar_t * format, va_list args)
{
	_ASSERT(DEF_INCR_SIZE <= DEF_BUFF_SIZE);
	size_t new_size = DEF_BUFF_SIZE - DEF_INCR_SIZE;
	size_t ret_size = DEF_BUFF_SIZE;
	wchar_t * buffer = NULL;

	while(ret_size >= new_size)
	{
		new_size += DEF_INCR_SIZE;
		FAILED_THROW_STDEXCEPTION(NULL != (buffer = (wchar_t *)realloc(buffer, new_size * sizeof(wchar_t))));

		ret_size = vswprintf_s(buffer, new_size, format, args);
	}

	return buffer;
}

std::basic_string<char> str_printf(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	std::basic_string<char> ret;
	int nLen;
	for(ret.resize(DEF_BUFF_SIZE); -1 == (nLen = vsnprintf_s(&ret[0], ret.size(), ret.size(), format, args)); ret.resize(ret.size() + DEF_INCR_SIZE))
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
	for(ret.resize(DEF_BUFF_SIZE); -1 == (nLen = vswprintf_s(&ret[0], ret.size(), format, args)); ret.resize(ret.size() + DEF_INCR_SIZE))
	{
	}
	va_end(args);

	ret.resize(nLen);
	return ret;
}

std::basic_string<wchar_t> ms2ws(const char * str)
{
	std::basic_string<wchar_t> ret;
	ret.resize(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, NULL, 0));
	if(!ret.empty())
	{
		ret.resize(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, &ret[0], ret.size()) - 1);
	}
	return ret;
}

std::basic_string<char> ws2ms(const wchar_t * str)
{
	std::basic_string<char> ret;
	ret.resize(WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, str, -1, NULL, 0, NULL, NULL));
	if(!ret.empty())
	{
		ret.resize(WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, str, -1, &ret[0], ret.size(), NULL, NULL) - 1);
	}
	return ret;
}

std::basic_string<wchar_t> u8tows(const char * str)
{
	std::basic_string<wchar_t> ret;
	ret.resize(MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0));
	if(!ret.empty())
	{
		ret.resize(MultiByteToWideChar(CP_UTF8, 0, str, -1, &ret[0], ret.size()) - 1);
	}
	return ret;
}

std::basic_string<char> wstou8(const wchar_t * str)
{
	std::basic_string<char> ret;
	ret.resize(WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL));
	if(!ret.empty())
	{
		ret.resize(WideCharToMultiByte(CP_UTF8, 0, str, -1, &ret[0], ret.size(), NULL, NULL) - 1);
	}
	return ret;
}
