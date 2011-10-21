
#pragma once

namespace my
{
	class Exception
	{
	protected:
		std::basic_string<_TCHAR> m_file;

		int m_line;

	public:
		Exception(const std::basic_string<_TCHAR> & file, int line);

		virtual ~Exception(void);

		virtual std::basic_string<_TCHAR> GetDescription(void) const throw() = 0;

		std::basic_string<_TCHAR> GetFullDescription(void) const;
	};

	class ComException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		ComException(HRESULT hres, const std::basic_string<_TCHAR> & file, int line);

		std::basic_string<_TCHAR> GetDescription(void) const throw();
	};

	class D3DException : public ComException
	{
	public:
		D3DException(HRESULT hres, const std::basic_string<_TCHAR> & file, int line);

		std::basic_string<_TCHAR> GetDescription(void) const throw();
	};

	class WinException : public Exception
	{
	protected:
		DWORD m_code;

	public:
		WinException(DWORD code, const std::basic_string<_TCHAR> & file, int line);

		std::basic_string<_TCHAR> GetDescription(void) const throw();
	};

	class CustomException : public Exception
	{
	protected:
		std::basic_string<_TCHAR> m_desc;

	public:
		CustomException(const std::basic_string<_TCHAR> & desc, const std::basic_string<_TCHAR> & file, int line);

		std::basic_string<_TCHAR> GetDescription(void) const throw();
	};
};

#define THROW_COMEXCEPTION(hres) throw my::ComException((hres), _T(__FILE__), __LINE__)

#define THROW_D3DEXCEPTION(hres) throw my::D3DException((hres), _T(__FILE__), __LINE__)

#define THROW_WINEXCEPTION(code) throw my::WinException((code), _T(__FILE__), __LINE__)

#define THROW_CUSEXCEPTION(info) throw my::CustomException((info), _T(__FILE__), __LINE__)

#define FAILED_THROW_COMEXCEPTION(expr) \
	{ \
		HRESULT hres; \
		if(FAILED(hres = (expr))) \
		{ \
			THROW_COMEXCEPTION(hres); \
		} \
	}

#define FAILED_THROW_D3DEXCEPTION(expr) \
	{ \
		HRESULT hres; \
		if(FAILED(hres = (expr))) \
		{ \
			THROW_D3DEXCEPTION(hres); \
		} \
	}

#define FAILED_THROW_CUSEXCEPTION(expr) \
	{ \
		if(!(expr)) \
		{ \
			THROW_CUSEXCEPTION(_T(#expr)); \
		} \
	}
