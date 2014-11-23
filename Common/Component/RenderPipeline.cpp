#include "StdAfx.h"
#include "RenderPipeline.h"

#define SHADOW_MAP_SIZE 512

using namespace my;

RenderPipeline::RenderPipeline(void)
	: m_ResMgr(NULL)
{
}

RenderPipeline::~RenderPipeline(void)
{
	OnDestroyDevice();
}

HRESULT RenderPipeline::OnCreateDevice(my::ResourceMgr * res_mgr, IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(res_mgr);

	m_ResMgr = res_mgr;

	if (!(m_SimpleSample = m_ResMgr->LoadEffect("shader/SimpleSample.fx", EffectMacroPairList())))
	{
		S_FALSE;
	}

	return S_OK;
}

HRESULT RenderPipeline::OnResetDevice(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_ShadowMap.CreateAdjustedTexture(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! ���е� render target����ʹ�þ�����ͬ multisample�� depth stencil
	m_ShadowMapDS.CreateDepthStencilSurface(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	m_GBufferN.CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_GBufferL.CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ShadowMap.OnDestroyDevice();

	m_ShadowMapDS.OnDestroyDevice();

	m_GBufferN.OnDestroyDevice();

	m_GBufferL.OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
	m_SimpleSample.reset();

	m_ShaderCache.clear();
}

void RenderPipeline::OnMeshLodMaterialLoaded(my::DeviceRelatedObjectBasePtr res, boost::weak_ptr<MeshLOD> weak_mesh_lod, unsigned int i)
{
	MeshLODPtr mesh_lod = weak_mesh_lod.lock();
	if (mesh_lod)
	{
		mesh_lod->m_Materials[i] = boost::dynamic_pointer_cast<Material>(res);
	}
}

void RenderPipeline::OnMeshLodMeshLoaded(my::DeviceRelatedObjectBasePtr res, boost::weak_ptr<MeshLOD> weak_mesh_lod)
{
	MeshLODPtr mesh_lod = weak_mesh_lod.lock();
	if (mesh_lod)
	{
		mesh_lod->m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);

		if (mesh_lod->m_Mesh)
		{
			mesh_lod->m_Materials.resize(mesh_lod->m_Mesh->m_MaterialNameList.size());
			for(DWORD i = 0; i < mesh_lod->m_Mesh->m_MaterialNameList.size(); i++)
			{
				m_ResMgr->LoadMaterialAsync(str_printf("material/%s.xml", mesh_lod->m_Mesh->m_MaterialNameList[i].c_str()), boost::bind(&RenderPipeline::OnMeshLodMaterialLoaded, this, _1, mesh_lod, i));
			}
		}
	}
}

void RenderPipeline::LoadMeshLodAsync(MeshLODPtr mesh_lod, const std::string & mesh_path)
{
	m_ResMgr->LoadMeshAsync(mesh_path, boost::bind(&RenderPipeline::OnMeshLodMeshLoaded, this, _1, mesh_lod));
}

void RenderPipeline::OnShaderLoaded(my::DeviceRelatedObjectBasePtr res, ShaderKeyType key)
{
	m_ShaderCache.insert(ShaderCacheMap::value_type(key, boost::dynamic_pointer_cast<my::Effect>(res)));
}

static size_t hash_value(const RenderPipeline::ShaderKeyType & key)
{
	size_t seed = 0;
	boost::hash_combine(seed, key.get<0>());
	boost::hash_combine(seed, key.get<1>());
	boost::hash_combine(seed, key.get<2>());
	return seed;
}

my::EffectPtr RenderPipeline::QueryShader(MeshComponent::MeshType mesh_type, DrawStage draw_stage, const my::Material * material)
{
	// ! make sure hash_value(ShaderKeyType ..) is valid
	ShaderKeyType key = boost::make_tuple(mesh_type, draw_stage, material);

	_ASSERT(material);

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(key);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second;
	}

	switch (draw_stage)
	{
	case DrawStageCBuffer:
		{
			EffectMacroPairList macros;
			if (mesh_type == MeshComponent::MeshTypeAnimation)
			{
				macros.push_back(EffectMacroPair("VS_SKINED_DQ",""));
			}

			std::string path = "shader/SimpleSample.fx";
			std::string key_str = ResourceMgr::EffectIORequest::BuildKey(path, macros);
			ResourceCallback callback = boost::bind(&RenderPipeline::OnShaderLoaded, this, _1, key);
			m_ResMgr->LoadResourceAsync(key_str, IORequestPtr(new ResourceMgr::EffectIORequest(callback, path, macros, m_ResMgr)), true);
		}
		break;
	}

	return my::EffectPtr();
}

void RenderPipeline::DrawMesh(MeshComponent * mesh_cmp, DWORD lod)
{
	_ASSERT(lod < mesh_cmp->m_Lod.size());

	MeshLOD * mesh_lod = mesh_cmp->m_Lod[lod].get();
	if (mesh_lod)
	{
		mesh_cmp->OnPreRender(m_SimpleSample.get(), DrawStageCBuffer);

		DrawMeshLOD(mesh_cmp->m_MeshType, mesh_lod);
	}
}

void RenderPipeline::DrawMeshLOD(MeshComponent::MeshType mesh_type, MeshLOD * mesh_lod)
{
	_ASSERT(mesh_lod);

	for (DWORD i = 0; i < mesh_lod->m_Materials.size(); i++)
	{
		_ASSERT(mesh_lod->m_Mesh);
		if (mesh_lod->m_Materials[i])
		{
			my::EffectPtr shader = QueryShader(mesh_type, DrawStageCBuffer, mesh_lod->m_Materials[i].get());
			if (shader)
			{
				mesh_lod->OnPreRender(shader.get(), DrawStageCBuffer, i);

				UINT passes = shader->Begin();
				for (UINT p = 0; p < passes; p++)
				{
					shader->BeginPass(p);
					mesh_lod->m_Mesh->DrawSubset(i);
					shader->EndPass();
				}
				shader->End();
			}
		}
	}
}
