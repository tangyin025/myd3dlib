
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stdlib.h>
#include <string>

#if _MSC_VER < 1400
#define vsnprintf _vsnprintf
#define vswprintf _vsnwprintf
#endif

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
#else
#define ms2ts(str) (str)
#define ws2ts(str) ws2ms(str)
#define ts2ms(str) (str)
#define ts2ws(str) ms2ws(str)
#endif

#endif // __LIBC_H__
