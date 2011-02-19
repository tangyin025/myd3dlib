
#pragma once

#include <tchar.h>
#include <atlstr.h>

namespace my
{
	class Exception
	{
	protected:
		CString m_file;

		int m_line;

	public:
		Exception(LPCTSTR file, int line);

		virtual ~Exception(void);

		virtual LPCTSTR GetDescription(void) const throw() = 0;

		CString GetFullDescription(void) const;
	};

	class ComException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		ComException(HRESULT hres, LPCTSTR file, int line);

		LPCTSTR GetDescription(void) const throw();
	};

	class D3DException : public ComException
	{
	public:
		D3DException(HRESULT hres, LPCTSTR file, int line);

		LPCTSTR GetDescription(void) const throw();
	};

	class CustomException : public Exception
	{
	protected:
		CString m_desc;

	public:
		CustomException(LPCTSTR desc, LPCTSTR file, int line);

		LPCTSTR GetDescription(void) const throw();
	};
};

#define THROW_COMEXCEPTION(hres) throw my::ComException((hres), _T(__FILE__), __LINE__)

#define THROW_D3DEXCEPTION(hres) throw my::D3DException((hres), _T(__FILE__), __LINE__)

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
