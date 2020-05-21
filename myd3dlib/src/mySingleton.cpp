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

using namespace my;

BOOST_CLASS_EXPORT(NamedObject)

DeviceResourceBase::DeviceResourceBase(void)
{
	// ! boost signals is thread-safety, ref: https://www.boost.org/doc/libs/1_63_0/doc/html/signals2/thread-safety.html
	D3DContext::getSingleton().m_EventDeviceReset.connect(boost::bind(&DeviceResourceBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.connect(boost::bind(&DeviceResourceBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.connect(boost::bind(&DeviceResourceBase::OnDestroyDevice, this));
}

DeviceResourceBase::~DeviceResourceBase(void)
{
	D3DContext::getSingleton().m_EventDeviceReset.disconnect(boost::bind(&DeviceResourceBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.disconnect(boost::bind(&DeviceResourceBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.disconnect(boost::bind(&DeviceResourceBase::OnDestroyDevice, this));
}

IResourceCallback::~IResourceCallback(void)
{
	_ASSERT(!ResourceMgr::getSingleton().FindIORequestCallback(this));
}

NamedObject::NamedObject(const char * Name)
	: m_Name(NULL)
{
	SetName(Name);
}

NamedObject::~NamedObject(void)
{
	SetName(NULL);
}

void NamedObject::SetName(const char * Name)
{
	if (!m_Name)
	{
		D3DContext::getSingleton().UnregisterNamedObject(m_Name, this);
	}

	if (Name)
	{
		m_Name = D3DContext::getSingleton().RegisterNamedObject(Name, this);
	}
	else
	{
		m_Name = NULL;
	}
}

std::string NamedObject::MakeUniqueName(const char * Prefix)
{
	static unsigned int index = 0;
	for (; ; index++)
	{
		std::string ret = str_printf("%s_%u", Prefix, index);
		if (!D3DContext::getSingleton().GetNamedObject(ret.c_str()))
		{
			return ret;
		}
	}
	THROW_CUSEXCEPTION("MakeUniqueName failed");
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
	SetName(Name.c_str());
}

template
void NamedObject::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void NamedObject::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void NamedObject::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void NamedObject::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void NamedObject::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void NamedObject::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void NamedObject::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void NamedObject::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);
