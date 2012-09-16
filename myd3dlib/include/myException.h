#pragma once

#include <string>
#include <Windows.h>

namespace my
{
	class Exception
	{
	protected:
		std::string m_file;

		int m_line;

	public:
		Exception(const std::string & file, int line)
			: m_file(file)
			, m_line(line)
		{
			MessageBeep(-1); //_ASSERT(false);
		}

		virtual ~Exception(void)
		{
		}

		virtual std::string GetDescription(void) const throw() = 0;

		std::string GetFullDescription(void) const;
	};

	class ComException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		ComException(HRESULT hres, const std::string & file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		std::string GetDescription(void) const throw();
	};

	class D3DException : public ComException
	{
	public:
		D3DException(HRESULT hres, const std::string & file, int line)
			: ComException(hres, file, line)
		{
		}

		std::string GetDescription(void) const throw();
	};

	class DInputException : public ComException
	{
	public:
		DInputException(HRESULT hres, const std::string & file, int line)
			: ComException(hres, file, line)
		{
		}

		std::string GetDescription(void) const throw();
	};

	class DSoundException : public ComException
	{
	public:
		DSoundException(HRESULT hres, const std::string & file, int line)
			: ComException(hres, file, line)
		{
		}

		std::string GetDescription(void) const throw();
	};

	class WinException : public Exception
	{
	protected:
		DWORD m_code;

	public:
		WinException(DWORD code, const std::string & file, int line)
			: Exception(file, line)
			, m_code(code)
		{
		}

		std::string GetDescription(void) const throw();
	};

	class CustomException : public Exception
	{
	protected:
		std::string m_desc;

	public:
		CustomException(const std::string & desc, const std::string & file, int line)
			: Exception(file, line)
			, m_desc(desc)
		{
		}

		std::string GetDescription(void) const throw();
	};
};

#define THROW_COMEXCEPTION(hres) throw my::ComException((hres), __FILE__, __LINE__)

#define THROW_D3DEXCEPTION(hres) throw my::D3DException((hres), __FILE__, __LINE__)

#define THROW_DINPUTEXCEPTION(hres) throw my::DInputException((hres), __FILE__, __LINE__)

#define THROW_DSOUNDEXCEPTION(hres) throw my::DSoundException((hres), __FILE__, __LINE__)

#define THROW_WINEXCEPTION(code) throw my::WinException((code), __FILE__, __LINE__)

#define THROW_CUSEXCEPTION(info) throw my::CustomException((info), __FILE__, __LINE__)

#ifdef _DEBUG
#define V(expr) _ASSERT(SUCCEEDED(hr = (expr)))
#else
#define V(expr) expr
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if (p) { delete (p); (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p)=NULL; } }
#endif
