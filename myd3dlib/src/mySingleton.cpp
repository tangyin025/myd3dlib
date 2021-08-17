#include "mySingleton.h"
#include "myDxutApp.h"
#include "myResource.h"
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

unsigned int NamedObject::UniqueNameIndex = 0;

NamedObject::NamedObject(const char * Name)
	: m_Name(NULL)
{
	SetName(Name);
}

NamedObject::~NamedObject(void)
{
	SetName(NULL);
}

std::string NamedObject::MakeUniqueName(const char * Prefix)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	_ASSERT(Prefix);

	boost::regex reg("(\\d+)$");
	boost::match_results<const char *> what;
	if (boost::regex_search(Prefix, what, reg, boost::match_default) && what[1].matched)
	{
		UniqueNameIndex = Max(UniqueNameIndex, boost::lexical_cast<unsigned int>(what[1]) + 1);
		std::string remove_postfix(Prefix, what[0].first);
		if (!remove_postfix.empty())
		{
			return MakeUniqueName(remove_postfix.c_str());
		}
	}

	for (; ; UniqueNameIndex++)
	{
		std::string ret = str_printf("%s%u", Prefix, UniqueNameIndex);
		if (!D3DContext::getSingleton().GetNamedObject(ret.c_str()))
		{
			return ret;
		}
	}

	THROW_CUSEXCEPTION("MakeUniqueName failed");
}

void NamedObject::ResetUniqueNameIndex(void)
{
	UniqueNameIndex = 0;
}

void NamedObject::SetName(const char * Name)
{
	if (m_Name)
	{
		D3DContext::getSingleton().UnregisterNamedObject(m_Name, this);

		_ASSERT(!m_Name);
	}

	if (Name && Name[0] != '\0')
	{
		D3DContext::getSingleton().RegisterNamedObject(Name, this);

		_ASSERT(m_Name);
	}
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

		SetName((pxar->prefix + Name).c_str());
	}
}
