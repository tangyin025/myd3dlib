#pragma once

template <class T>
class ResourceBundle : public my::IResourceCallback
{
public:
	bool m_Ready;

	std::string m_Path;

	boost::shared_ptr<T> m_Res;

public:
	ResourceBundle(const char * Path)
		: m_Ready(false)
		, m_Path(Path)
	{
	}

	ResourceBundle(void)
		: m_Ready(false)
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
		m_Ready = true;
	}

	bool IsReady(void) const
	{
		return m_Ready;
	}

	void RequestResource(void);

	void ReleaseResource(void)
	{
		if (IsRequested())
		{
			my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_Path, this);
		}
	}
};
