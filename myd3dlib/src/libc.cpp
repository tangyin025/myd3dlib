#include "stdafx.h"
#include "libc.h"
#include "myException.h"

#define FAILED_THROW_CUSEXCEPTION(expr) \
	{ \
		if(!(expr)) \
		{ \
			THROW_CUSEXCEPTION((#expr)); \
		} \
	}

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
		ret_size = vswprintf_s(buffer, new_size, format, args);
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
	for(ret.resize(512); -1 == (nLen = vswprintf_s(&ret[0], ret.size(), format, args)); ret.resize(ret.size() + 512))
	{
	}
	va_end(args);

	ret.resize(nLen);
	return ret;
}

std::basic_string<wchar_t> ms2ws(const char * str)
{
	int nLen;
	if(0 == (nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, NULL, 0)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	std::basic_string<wchar_t> ret;
	ret.resize(nLen - 1);
	if(0 == (nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, &ret[0], nLen)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	ret.resize(nLen - 1);
	return ret;
}

std::basic_string<char> ws2ms(const wchar_t * str)
{
	int nLen;
	if(0 == (nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, str, -1, NULL, 0, NULL, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	std::basic_string<char> ret;
	ret.resize(nLen - 1);
	if(0 == (nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, str, -1, &ret[0], nLen, NULL, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	ret.resize(nLen - 1);
	return ret;
}

std::basic_string<wchar_t> u8tows(const char * str)
{
	int nLen;
	if(0 == (nLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, NULL, 0)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	std::basic_string<wchar_t> ret;
	ret.resize(nLen - 1);
	if(0 == (nLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, &ret[0], nLen)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	ret.resize(nLen - 1);
	return ret;
}

std::basic_string<char> wstou8(const wchar_t * str)
{
	int nLen;
	if(0 == (nLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str, -1, NULL, 0, NULL, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	std::basic_string<char> ret;
	ret.resize(nLen - 1);
	if(0 == (nLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str, -1, &ret[0], nLen, NULL, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	ret.resize(nLen - 1);
	return ret;
}
