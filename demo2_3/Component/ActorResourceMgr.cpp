#include "StdAfx.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>
#include "ActorResourceMgr.h"
#include "Actor.h"

using namespace my;

void ActorResourceMgr::OnMaterialParameterValueTextureLoaded(
	boost::weak_ptr<Material::ParameterValueTexture> weak_value_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	Material::ParameterValueTexturePtr value_ptr = weak_value_ptr.lock();
	if (value_ptr)
	{
		value_ptr->m_Texture = boost::dynamic_pointer_cast<Texture2D>(res);
	}
}

void ActorResourceMgr::OnMeshComponentMaterialLoaded(
	boost::weak_ptr<MeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId,
	bool bInstance)
{
	MeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		if (cmp_ptr->m_MaterialList.size() <= AttribId)
		{
			cmp_ptr->m_MaterialList.resize(AttribId + 1);
		}
		cmp_ptr->m_MaterialList[AttribId] = boost::dynamic_pointer_cast<Material>(res);
	}
}

void ActorResourceMgr::OnMeshComponentMeshLoaded(
	boost::weak_ptr<MeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	bool bInstance)
{
	MeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
		cmp_ptr->m_MaterialList.resize(cmp_ptr->m_Mesh->m_MaterialNameList.size());
		cmp_ptr->m_bInstance = bInstance;
		for (unsigned int i = 0; i < cmp_ptr->m_Mesh->m_MaterialNameList.size(); i++)
		{
			MaterialPtr mat = CreateMaterial(str_printf("material/%s.xml", cmp_ptr->m_Mesh->m_MaterialNameList[i].c_str()));
			if (mat)
			{
				OnMeshComponentMaterialLoaded(cmp_ptr, mat, i, bInstance);
			}
		}
	}
}

void ActorResourceMgr::OnEmitterComponentEmitterLoaded(
	boost::weak_ptr<EmitterComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	EmitterComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Emitter = boost::dynamic_pointer_cast<Emitter>(res);
		MaterialPtr mat = CreateMaterial(str_printf("material/%s.xml", cmp_ptr->m_Emitter->m_MaterialName.c_str()));
		if (mat)
		{
			OnEmitterComponentMaterialLoaded(cmp_ptr, mat);
		}
	}
}

void ActorResourceMgr::OnEmitterComponentMaterialLoaded(
	boost::weak_ptr<EmitterComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	EmitterComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Material = boost::dynamic_pointer_cast<Material>(res);
	}
}

void ActorResourceMgr::OnAnimatorSkeletonLoaded(
	boost::weak_ptr<Animator> weak_ani_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	AnimatorPtr ani_ptr = weak_ani_ptr.lock();
	if (ani_ptr)
	{
		OgreSkeletonAnimationPtr skel = boost::dynamic_pointer_cast<OgreSkeletonAnimation>(res);

		ani_ptr->m_Skeleton = skel;
	}
}

HRESULT ActorResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if (FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}
	return S_OK;
}

HRESULT ActorResourceMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if (FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.begin();
	for (; shader_iter != m_ShaderCache.end(); shader_iter++)
	{
		if (shader_iter->second)
		{
			shader_iter->second->OnResetDevice();
		}
	}
	return S_OK;
}

void ActorResourceMgr::OnLostDevice(void)
{
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.begin();
	for (; shader_iter != m_ShaderCache.end(); shader_iter++)
	{
		if (shader_iter->second)
		{
			shader_iter->second->OnLostDevice();
		}
	}

	ResourceMgr::OnLostDevice();
}

void ActorResourceMgr::OnDestroyDevice(void)
{
	ClearAllShaders();
	ResourceMgr::OnDestroyDevice();
}

void ActorResourceMgr::ClearAllShaders(void)
{
	m_ShaderCache.clear();
}

boost::shared_ptr<my::Emitter> ActorResourceMgr::CreateEmitter(const std::string & path)
{
	try
	{
		EmitterPtr ret;
		CachePtr cache = OpenIStream(path)->GetWholeCache();
		membuf mb((char *)&(*cache)[0], cache->size());
		std::istream istr(&mb);
		boost::archive::xml_iarchive ar(istr);
		ar >> boost::serialization::make_nvp("Emitter", ret);
		return ret;
	}
	catch (...)
	{
		OnResourceFailed(str_printf("CreateEmitter failed: %s", path.c_str()));
	}
	return EmitterPtr();
}

void ActorResourceMgr::SaveEmitter(const std::string & path, boost::shared_ptr<Emitter> emitter)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Emitter", emitter);
}

boost::shared_ptr<Material> ActorResourceMgr::CreateMaterial(const std::string & path)
{
	try
	{
		MaterialPtr ret;
		CachePtr cache = OpenIStream(path)->GetWholeCache();
		membuf mb((char *)&(*cache)[0], cache->size());
		std::istream istr(&mb);
		boost::archive::xml_iarchive ar(istr);
		ar >> boost::serialization::make_nvp("Material", ret);

		Material::ParameterList::iterator param_iter = ret->m_Params.begin();
		for (; param_iter != ret->m_Params.end(); param_iter++)
		{
			switch (param_iter->second->m_Type)
			{
			case Material::ParameterValue::ParameterValueTypeTexture:
				{
					Material::ParameterValueTexturePtr value = boost::dynamic_pointer_cast<Material::ParameterValueTexture>(param_iter->second);
					LoadTextureAsync(value->m_Path, boost::bind(&ActorResourceMgr::OnMaterialParameterValueTextureLoaded, this, value, _1));
				}
				break;
			}
		}
		return ret;
	}
	catch (...)
	{
		OnResourceFailed(str_printf("CreateMaterial failed: %s", path.c_str()));
	}
	return MaterialPtr();
}

void ActorResourceMgr::SaveMaterial(const std::string & path, boost::shared_ptr<Material> material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Material", material);
}
