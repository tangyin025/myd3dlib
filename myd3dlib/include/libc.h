// Copyright (c) 2011-2024 tangyin025
// License: MIT

#ifndef __LIBC_H__
#define __LIBC_H__

#include <string>

char * asprintf(const char * format, ...);

wchar_t * aswprintf(const wchar_t * format, ...);

char * avsprintf(const char * format, va_list args);

wchar_t * avswprintf(const wchar_t * format, va_list args);

std::basic_string<char> str_printf(const char * format, ...);

std::basic_string<wchar_t> str_printf(const wchar_t * format, ...);

std::basic_string<wchar_t> ms2ws(const char * str);

std::basic_string<char> ws2ms(const wchar_t * str);

std::basic_string<wchar_t> u8tows(const char * str);

std::basic_string<char> wstou8(const wchar_t * str);

#ifdef _UNICODE
#define ms2ts(str) ms2ws(str)
#define ws2ts(str) (str)
#define ts2ms(str) ws2ms(str)
#define ts2ws(str) (str)
#define u8tots(str) u8tows(str)
#define tstou8(str) wstou8(str)
#else
#define ms2ts(str) (str)
#define ws2ts(str) ws2ms(str)
#define ts2ms(str) (str)
#define ts2ws(str) ms2ws(str)
#define u8tots(str) ws2ms(u8tows(str).c_str())
#define tstou8(str) wstou8(ms2ws(str).c_str())
#endif

#endif // __LIBC_H__
