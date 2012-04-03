
#ifndef __LIBC_H__
#define __LIBC_H__

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

std::basic_string<wchar_t> mstringToWString(const char * mstr);

std::basic_string<char> wstringToMString(const wchar_t * wstr);

#ifdef _UNICODE
#define mstringToTString(str) mstringToWString(str)
#define wstringToTString(str) (str)
#define tstringToMString(str) wstringToMString(str)
#define tstringToWString(str) (str)
#else
#define mstringToTString(str) (str)
#define wstringToTString(str) wstringToMString(str)
#define tstringToMString(str) (str)
#define tstringToWString(str) mstringToWString(str)
#endif

#endif // __LIBC_H__
