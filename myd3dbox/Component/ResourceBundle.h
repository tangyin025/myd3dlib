#pragma once

template <class T>
class ResourceBundle : public my::IResourceCallback
{
public:
	std::string m_Path;

	boost::shared_ptr<T> m_Res;

public:
	ResourceBundle(const char * Path)
		: m_Path(Path)
	{
	}

	ResourceBundle(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Path);
	}

	virtual void OnReady(my::DeviceResourceBasePtr res)
	{
		m_Res = boost::dynamic_pointer_cast<T>(res);
	}

	ResourceBundle & operator = (const ResourceBundle<T> & rhs)
	{
		m_Path = rhs.m_Path;
		return *this;
	}

	void RequestResource(void);

	void ReleaseResource(void)
	{
		if (IsRequested())
		{
			my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_Path, this);
		}
		m_Res.reset();
	}
};
