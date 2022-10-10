#pragma once

#include "myMath.h"
#include "mySingleton.h"
#include "myResource.h"

class Actor;

namespace my
{
	class Dialog;
}

class SceneContext : public my::DeviceResourceBase
{
public:
	my::Vector3 m_SkyLightCamEuler;
	my::Vector4 m_SkyLightColor;
	my::Vector4 m_AmbientColor;
	my::Vector4 m_DofParams;
	float m_LuminanceThreshold;
	my::Vector3 m_BloomColor;
	float m_BloomFactor;
	float m_SsaoBias;
	float m_SsaoIntensity;
	float m_SsaoRadius;
	float m_SsaoScale;
	my::Vector4 m_FogColor;
	float m_FogStartDistance;
	float m_FogHeight;
	float m_FogFalloff;
	typedef std::vector<boost::shared_ptr<Actor> > ActorPtrList;
	ActorPtrList m_ActorList;
	typedef std::vector<boost::shared_ptr<my::Dialog> > DialogPtrList;
	DialogPtrList m_DialogList;

public:
	SceneContext(void)
	{
	}

	void OnResetDevice(void)
	{
	}

	void OnLostDevice(void)
	{
	}

	void OnDestroyDevice(void)
	{
	}
};

typedef boost::shared_ptr<SceneContext> SceneContextPtr;

class SceneContextRequest : public my::IORequest
{
protected:
	friend class Client;

	std::string m_path;

	std::string m_prefix;

	volatile LONG m_ActorListSize;

	volatile LONG m_ActorProgress;

	volatile LONG m_DialogListSize;

	volatile LONG m_DialogProgress;

public:
	SceneContextRequest(const char* path, const char* prefix, int Priority)
		: IORequest(Priority)
		, m_path(path)
		, m_prefix(prefix)
		, m_ActorListSize(LONG_MAX)
		, m_ActorProgress(0)
		, m_DialogListSize(LONG_MAX)
		, m_DialogProgress(0)
	{
	}

	virtual void LoadResource(void);

	virtual void CreateResource(LPDIRECT3DDEVICE9 pd3dDevice);

	static std::string BuildKey(const char* path, const char* prefix);
};
