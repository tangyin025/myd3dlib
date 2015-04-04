#pragma once

#include <string>
#include <tchar.h>
#include <Windows.h>

namespace my
{
	class Exception
	{
	protected:
		std::string m_file;

		int m_line;

	public:
		Exception(const char * file, int line);

		virtual ~Exception(void);

		virtual std::string what(void) const = 0;
	};

	class ComException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		ComException(HRESULT hres, const char * file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static const char * Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class D3DException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		D3DException(HRESULT hres, const char * file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static const char * Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class DInputException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		DInputException(HRESULT hres, const char * file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static const char * Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class DSoundException : public Exception
	{
	protected:
		HRESULT m_hres;

	public:
		DSoundException(HRESULT hres, const char * file, int line)
			: Exception(file, line)
			, m_hres(hres)
		{
		}

		static const char * Translate(HRESULT hres) throw();

		virtual std::string what(void) const;
	};

	class WinException : public Exception
	{
	protected:
		DWORD m_code;

	public:
		WinException(DWORD code, const char * file, int line)
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
		CustomException(const std::string & desc, const char * file, int line)
			: Exception(file, line)
			, m_desc(desc)
		{
		}

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

#define DEFINE_XML_NODE(node_v, node_p, node_s) \
	node_v = node_p->first_node(#node_s); \
	if(NULL == node_v) \
		THROW_CUSEXCEPTION("cannot find " #node_s)

#define DEFINE_XML_NODE_SIMPLE(node_s, parent_s) \
	rapidxml::xml_node<char> * node_##node_s; \
	DEFINE_XML_NODE(node_##node_s, node_##parent_s, node_s)

#define DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s) \
	attr_v = node_p->first_attribute(#attr_s); \
	if(NULL == attr_v) \
		THROW_CUSEXCEPTION("cannot find " #attr_s)

#define DEFINE_XML_ATTRIBUTE_SIMPLE(attr_s, parent_s) \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE(attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_INT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = atoi(attr_v->value())

#define DEFINE_XML_ATTRIBUTE_INT_SIMPLE(attr_s, parent_s) \
	int attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_INT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_FLOAT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = (float)atof(attr_v->value())

#define DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(attr_s, parent_s) \
	float attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_FLOAT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_BOOL(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = 0 == _stricmp(attr_v->value(), "true")

#define DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(attr_s, parent_s) \
	bool attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_BOOL(attr_s, attr_##attr_s, node_##parent_s, attr_s)
