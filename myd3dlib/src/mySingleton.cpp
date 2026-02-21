// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "mySingleton.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "myMath.h"
#include "libc.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace my;

BOOST_CLASS_EXPORT(NamedObject)

DeviceResourceBase::DeviceResourceBase(void)
	: m_Key(NULL)
{
	CriticalSectionLock lock(D3DContext::getSingleton().m_DeviceObjectsSec);

	D3DContext::getSingleton().m_DeviceObjects.insert(this);
}

DeviceResourceBase::~DeviceResourceBase(void)
{
	CriticalSectionLock lock(D3DContext::getSingleton().m_DeviceObjectsSec);

	D3DContext::getSingleton().m_DeviceObjects.erase(this);
}

ShaderResourceBase::ShaderResourceBase(void)
{
	CriticalSectionLock lock(D3DContext::getSingleton().m_ShaderObjectsSec);

	D3DContext::getSingleton().m_ShaderObjects.insert(this);
}

ShaderResourceBase::~ShaderResourceBase(void)
{
	CriticalSectionLock lock(D3DContext::getSingleton().m_ShaderObjectsSec);

	D3DContext::getSingleton().m_ShaderObjects.erase(this);
}

CachePtr my::IStream::GetWholeCache(void)
{
	CachePtr cache(new Cache(GetSize()));
	if (0 == cache->size())
	{
		THROW_CUSEXCEPTION("read stream cache failed");
	}

	int ret = read(&(*cache)[0], cache->size());
	if (ret != cache->size())
	{
		THROW_CUSEXCEPTION("read stream cache failed");
	}
	return cache;
}

unsigned int NamedObject::postfix_i = 0;

NamedObject::NamedObject(void)
	: m_Name(NULL)
{
	D3DContext::getSingleton().OnNamedObjectCreate(this);
}

NamedObject::NamedObject(const char * Name)
	: m_Name(NULL)
{
	SetName(Name);

	D3DContext::getSingleton().OnNamedObjectCreate(this);
}

NamedObject::~NamedObject(void)
{
	D3DContext::getSingleton().OnNamedObjectDestroy(this);

	SetName(NULL);
}

// access private member using template trick, https://stackoverflow.com/questions/12993219/access-private-member-using-template-trick
template<typename Tag, typename Tag::type M>
struct Rob {
	friend typename Tag::type get(Tag) {
		return M;
	}
};

struct string_Eos {
	typedef void (std::string::* type)(size_t);
	friend type get(string_Eos);
};

template struct Rob<string_Eos, &std::string::_Eos>;

std::string NamedObject::MakeUniqueName(const char * Prefix)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	_ASSERT(Prefix);

	std::string name_without_postfix;
	boost::regex reg("\\.?(\\d+)$");
	boost::match_results<const char *> what;
	if (boost::regex_search(Prefix, what, reg, boost::match_default) && what[1].matched)
	{
		postfix_i = boost::lexical_cast<unsigned int>(what[1]);
		name_without_postfix.assign(Prefix, what[0].first);
	}
	else
		name_without_postfix.assign(Prefix);

	// ! If high-probability numbers are concentrated at the front of the sequence, using sequential guessing.
	const size_t max_size = Max((size_t)128, name_without_postfix.length() + 16);
	std::string ret(max_size, '\0');
	for (; ; postfix_i++)
	{
		// ! Make sure that std::string::resize does not modify the capacity.
		(ret.*get(string_Eos()))(_snprintf(&ret[0], max_size, "%s%u", name_without_postfix.c_str(), postfix_i));
		if (!D3DContext::getSingleton().GetNamedObject(ret))
		{
			return ret;
		}
	}

	THROW_CUSEXCEPTION("MakeUniqueName failed");
}

void NamedObject::SetName(const char * Name)
{
	if (m_Name)
	{
		BOOST_VERIFY(D3DContext::getSingleton().UnregisterNamedObject(m_Name, this));
	}

	if (Name && Name[0] != '\0')
	{
		if (!D3DContext::getSingleton().RegisterNamedObject(Name, this))
		{
			THROW_CUSEXCEPTION("NamedObject::SetName failed");
		}
	}
}

const char * NamedObject::GetName(void) const
{
	return m_Name ? m_Name : "";
}

template<class Archive>
void NamedObject::save(Archive & ar, const unsigned int version) const
{
	std::string Name(m_Name);
	ar << BOOST_SERIALIZATION_NVP(Name);
}

template<class Archive>
void NamedObject::load(Archive & ar, const unsigned int version)
{
	std::string Name;
	ar >> BOOST_SERIALIZATION_NVP(Name);
	if (!Name.empty())
	{
		NamedObjectSerializationContext * pxar = dynamic_cast<NamedObjectSerializationContext *>(&ar);
		_ASSERT(pxar);

		if (pxar->make_unique)
		{
			_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

			_ASSERT(pxar->prefix.empty());

			SetName(MakeUniqueName(Name.c_str()).c_str());
		}
		else
		{
			SetName((pxar->prefix + Name).c_str());
		}
	}
}
