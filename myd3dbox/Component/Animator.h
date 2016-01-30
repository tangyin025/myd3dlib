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

class Animator
{
public:
	ResourceBundle<my::OgreSkeletonAnimation> m_SkeletonRes;

	my::TransformList m_DualQuats;

public:
	Animator(void)
	{
	}

	virtual ~Animator(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_SkeletonRes);
	}

	virtual void RequestResource(void)
	{
		m_SkeletonRes.RequestResource();
	}

	virtual void ReleaseResource(void)
	{
		m_SkeletonRes.ReleaseResource();
	}

	virtual void Update(float fElapsedTime)
	{
	}
};

typedef boost::shared_ptr<Animator> AnimatorPtr;

class SimpleAnimator
	: public Animator
{
public:
	float m_Time;

public:
	SimpleAnimator(void)
		: m_Time(0)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Animator);
	}

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<SimpleAnimator> SimpleAnimatorPtr;
