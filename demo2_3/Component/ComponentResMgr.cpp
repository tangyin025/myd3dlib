#include "StdAfx.h"
#include "ComponentResMgr.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <fstream>

using namespace my;

void ComponentResMgr::OnMaterialDiffuseTextureLoaded(
	boost::weak_ptr<Material> weak_mat_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	MaterialPtr mat_ptr = weak_mat_ptr.lock();
	if (mat_ptr)
	{
		mat_ptr->m_DiffuseTexture.second = boost::dynamic_pointer_cast<BaseTexture>(res);
	}
}

void ComponentResMgr::OnMeshComponentLODMaterialLoaded(
	boost::weak_ptr<MeshComponent::MeshLOD> weak_lod_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId,
	bool bInstance)
{
	MeshComponent::MeshLODPtr lod_ptr = weak_lod_ptr.lock();
	if (lod_ptr)
	{
		lod_ptr->m_Material = boost::dynamic_pointer_cast<Material>(res);

		if (!lod_ptr->m_Material->m_DiffuseTexture.first.empty())
		{
			LoadTextureAsync(lod_ptr->m_Material->m_DiffuseTexture.first,
				boost::bind(&ComponentResMgr::OnMaterialDiffuseTextureLoaded, this, lod_ptr->m_Material, _1));
		}
	}
}

void ComponentResMgr::OnMeshComponentLODMeshLoaded(
	boost::weak_ptr<MeshComponent::MeshLOD> weak_lod_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId,
	bool bInstance)
{
	MeshComponent::MeshLODPtr lod_ptr = weak_lod_ptr.lock();
	if (lod_ptr)
	{
		lod_ptr->m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
		lod_ptr->m_AttribId = AttribId;
		lod_ptr->m_bInstance = bInstance;

		if (AttribId < lod_ptr->m_Mesh->m_MaterialNameList.size())
		{
			LoadMaterialAsync(str_printf("material/%s.xml", lod_ptr->m_Mesh->m_MaterialNameList[AttribId].c_str()),
				boost::bind(&ComponentResMgr::OnMeshComponentLODMaterialLoaded, this, lod_ptr, _1, AttribId, bInstance));
		}
	}
}

void ComponentResMgr::OnEmitterComponentEmitterLoaded(
	boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId)
{
	EmitterMeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		if (cmp_ptr->m_EmitterList.size() <= AttribId)
		{
			cmp_ptr->m_EmitterList.resize(AttribId + 1);
		}
		cmp_ptr->m_EmitterList[AttribId] = boost::dynamic_pointer_cast<Emitter>(res);
	}
}

void ComponentResMgr::OnEmitterComponentMaterialLoaded(
	boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId)
{
	EmitterMeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		if (cmp_ptr->m_MaterialList.size() <= AttribId)
		{
			cmp_ptr->m_MaterialList.resize(AttribId + 1);
		}
		cmp_ptr->m_MaterialList[AttribId] = boost::dynamic_pointer_cast<Material>(res);

		if (!cmp_ptr->m_MaterialList[AttribId]->m_DiffuseTexture.first.empty())
		{
			LoadTextureAsync(cmp_ptr->m_MaterialList[AttribId]->m_DiffuseTexture.first,
				boost::bind(&ComponentResMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_MaterialList[AttribId], _1));
		}
	}
}

class MaterialIORequest : public IORequest
{
public:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	MaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if (callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if (m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
			try
			{
				MaterialPtr res;
				membuf mb((char *)&(*m_cache)[0], m_cache->size());
				std::istream ims(&mb);
				boost::archive::xml_iarchive ia(ims);
				ia >> boost::serialization::make_nvp("Material", res);
				m_res = res;
			}
			catch (...)
			{
			}
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_res)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
	}
};

void ComponentResMgr::LoadMaterialAsync(const std::string & path, const my::ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MaterialIORequest(callback, path, this)), false);
}

boost::shared_ptr<Material> ComponentResMgr::LoadMaterial(const std::string & path)
{
	return LoadResource<Material>(path, IORequestPtr(new MaterialIORequest(ResourceCallback(), path, this)));
}

void ComponentResMgr::SaveMaterial(const std::string & path, boost::shared_ptr<Material> material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Material", material);
}

MeshComponentPtr ComponentResMgr::CreateMeshComponentFromFile(const std::string & path)
{
	MeshComponentPtr ret(new MeshComponent());
	MeshComponent::MeshLODPtr lod(new MeshComponent::MeshLOD(ret.get()));
	LoadMeshAsync(path, boost::bind(&ComponentResMgr::OnMeshComponentLODMeshLoaded, this, lod, _1, 0, false));
	ret->m_lods.push_back(lod);
	return ret;
}

EmitterMeshComponentPtr ComponentResMgr::CreateEmitterComponentFromFile(const std::string & path)
{
	EmitterMeshComponentPtr ret(new EmitterMeshComponent());
	LoadEmitterAsync(path, boost::bind(&ComponentResMgr::OnEmitterComponentEmitterLoaded, this, ret, _1, 0));
	LoadMaterialAsync("material/lambert1.xml", boost::bind(&ComponentResMgr::OnEmitterComponentMaterialLoaded, this, ret, _1, 0));
	return ret;
}
