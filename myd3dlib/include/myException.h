
#pragma once

namespace my
{
	class Exception
	{
	protected:
		std::string m_file;

		int m_line;

	public:
		Exception(const std::string & file, int line);

		virtual ~Exception(void);

		virtual std::string GetDescription(void) const throw() = 0;

		std::string GetFullDescription(void) const;
	};

	class ComException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		ComException(HRESULT hres, const std::string & file, int line);

		std::string GetDescription(void) const throw();
	};

	class D3DException : public ComException
	{
	public:
		D3DException(HRESULT hres, const std::string & file, int line);

		std::string GetDescription(void) const throw();
	};

	class WinException : public Exception
	{
	protected:
		DWORD m_code;

	public:
		WinException(DWORD code, const std::string & file, int line);

		std::string GetDescription(void) const throw();
	};

	class CustomException : public Exception
	{
	protected:
		std::string m_desc;

	public:
		CustomException(const std::string & desc, const std::string & file, int line);

		std::string GetDescription(void) const throw();
	};
};

#define THROW_COMEXCEPTION(hres) throw my::ComException((hres), __FILE__, __LINE__)

#define THROW_D3DEXCEPTION(hres) throw my::D3DException((hres), __FILE__, __LINE__)

#define THROW_WINEXCEPTION(code) throw my::WinException((code), __FILE__, __LINE__)

#define THROW_CUSEXCEPTION(info) throw my::CustomException((info), __FILE__, __LINE__)

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
			THROW_CUSEXCEPTION((#expr)); \
		} \
	}
