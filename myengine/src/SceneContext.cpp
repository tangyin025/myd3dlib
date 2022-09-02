#include "SceneContext.h"
#include "myResource.h"
#include "Actor.h"
#include "libc.h"
#include "myUi.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

using namespace my;

void SceneContextRequest::LoadResource(void)
{
	if (ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
	{
		my::IStreamBuff buff(ResourceMgr::getSingleton().OpenIStream(m_path.c_str()));
		std::istream ifs(&buff);
		LPCSTR Ext = PathFindExtensionA(m_path.c_str());
		boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(ifs, Ext);
		boost::dynamic_pointer_cast<NamedObjectSerializationContext>(ia)->prefix = m_prefix;
		SceneContextPtr scene = boost::dynamic_pointer_cast<SceneContext>(m_res);
		*ia >> boost::serialization::make_nvp("SkyLightCam.m_Euler", scene->m_SkyLightCamEuler);
		*ia >> boost::serialization::make_nvp("SkyLightColor", scene->m_SkyLightColor);
		*ia >> boost::serialization::make_nvp("AmbientColor", scene->m_AmbientColor);
		*ia >> boost::serialization::make_nvp("DofParams", scene->m_DofParams);
		*ia >> boost::serialization::make_nvp("LuminanceThreshold", scene->m_LuminanceThreshold);
		*ia >> boost::serialization::make_nvp("BloomColor", scene->m_BloomColor);
		*ia >> boost::serialization::make_nvp("BloomFactor", scene->m_BloomFactor);
		*ia >> boost::serialization::make_nvp("SsaoBias", scene->m_SsaoBias);
		*ia >> boost::serialization::make_nvp("SsaoIntensity", scene->m_SsaoIntensity);
		*ia >> boost::serialization::make_nvp("SsaoRadius", scene->m_SsaoRadius);
		*ia >> boost::serialization::make_nvp("SsaoScale", scene->m_SsaoScale);
		*ia >> boost::serialization::make_nvp("FogColor", scene->m_FogColor);
		*ia >> boost::serialization::make_nvp("FogStartDistance", scene->m_FogStartDistance);
		*ia >> boost::serialization::make_nvp("FogHeight", scene->m_FogHeight);
		*ia >> boost::serialization::make_nvp("FogFalloff", scene->m_FogFalloff);

		DWORD ActorListSize;
		*ia >> BOOST_SERIALIZATION_NVP(ActorListSize);
		scene->m_ActorList.resize(ActorListSize);
		for (int i = 0; i < ActorListSize; i++, InterlockedExchange(&m_ActorProgress, i))
		{
			*ia >> boost::serialization::make_nvp(str_printf("Actor%d", i).c_str(), scene->m_ActorList[i]);
		}

		DWORD DialogListSize;
		*ia >> BOOST_SERIALIZATION_NVP(DialogListSize);
		scene->m_DialogList.resize(DialogListSize);
		for (int i = 0; i < DialogListSize; i++, InterlockedExchange(&m_DialogProgress, i))
		{
			*ia >> boost::serialization::make_nvp(str_printf("Dialog%d", i).c_str(), scene->m_DialogList[i]);
		}
	}
}

void SceneContextRequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if (boost::dynamic_pointer_cast<SceneContext>(m_res)->m_ActorList.empty()
		&& boost::dynamic_pointer_cast<SceneContext>(m_res)->m_DialogList.empty())
	{
		m_res.reset();
		THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
	}
}

std::string SceneContextRequest::BuildKey(const char* path)
{
	return path;
}
