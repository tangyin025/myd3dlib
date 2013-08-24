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
			_ASSERT(false);
		}

		virtual ~Exception(void)
		{
		}

		virtual std::string what(void) const = 0;
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

		static std::string Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class D3DException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		D3DException(HRESULT hres, const std::string & file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static std::string Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class DInputException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		DInputException(HRESULT hres, const std::string & file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static std::string Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class DSoundException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		DSoundException(HRESULT hres, const std::string & file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static std::string Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
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

		static std::string Translate(DWORD code) throw();

		virtual std::string what(void) const;
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

		static std::string Translate(void) throw();

		virtual std::string what(void) const;
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
