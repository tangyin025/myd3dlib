// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include <sstream>
#include "RenderPipeline.h"
#include "Material.h"
#include "RenderPipeline.inl"
#include "myResource.h"
#include "myDxutApp.h"
#include "myUtility.h"
#include "myEffect.h"
#include "Component.h"
#include "Actor.h"
#include "libc.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/utility/string_view.hpp>
#include <fstream>

using namespace my;

const UINT RenderPipeline::m_ParticlePrimitiveInfo[ParticlePrimitiveTypeCount][4] = {
	{0,3,0,1},
	{3,4,3,2},
};

RenderPipeline::RenderPipeline(void)
	: SHADOW_MAP_SIZE(1024)
	, m_CascadeLayer(0.011325952596962, 0.0030774015467614, 0.00036962624290027, FLT_MIN)
	, m_CascadeLayerCent(0.022430079057813, 0.0048506590537727, 0.00068016158184037, 2.8240716346772e-05)
	, m_CascadeLayerBias(0.001f, 0.001f, 0.001f, 0.001f)
	, m_SkyLightCam(new my::OrthoCamera(30.0f, 30.0f, -100, 100))
	, m_SkyLightColor(1.0f, 1.0f, 1.0f, 1.0f)
	, m_AmbientColor(0.3f, 0.3f, 0.3f, 3.0f)
	, m_BkColor(D3DCOLOR_ARGB(0, 66, 75, 121))
	, handle_Time(NULL)
	, handle_ScreenDim(NULL)
	, handle_ShadowMapSize(NULL)
	, handle_ShadowBias(NULL)
	, handle_ShadowLayer(NULL)
	, handle_World(NULL)
	, handle_Eye(NULL)
	, handle_View(NULL)
	, handle_ViewProj(NULL)
	, handle_SkyLightDir(NULL)
	, handle_SkyLightViewProj(NULL)
	, handle_SkyLightColor(NULL)
	, handle_AmbientColor(NULL)
	, handle_NormalRT(NULL)
	, handle_SpecularRT(NULL)
	, handle_PositionRT(NULL)
	, handle_LightRT(NULL)
	, handle_OpaqueRT(NULL)
	, handle_DownFilterRT(NULL)
	, handle_DofParams(NULL)
	, m_DofParams(5.0f, 15.0f, 25.0f, 1.0f)
	, handle_LuminanceThreshold(NULL)
	, handle_BloomColor(NULL)
	, handle_BloomFactor(NULL)
	, m_LuminanceThreshold(0.5f)
	, m_BloomColor(1.0f, 1.0f, 1.0f)
	, m_BloomFactor(1.0f)
	, handle_InputTexture(NULL)
	, handle_RCPFrame(NULL)
	, handle_bias(NULL)
	, handle_intensity(NULL)
	, handle_sample_rad(NULL)
	, handle_scale(NULL)
	, handle_OcclusionRT(NULL)
	, m_SsaoBias(0.2f)
	, m_SsaoIntensity(5.0f)
	, m_SsaoRadius(100.0f)
	, m_SsaoScale(10.0f)
	, handle_FogColor(NULL)
	, handle_FogParams(NULL)
	, m_FogColor(0.518f, 0.553f, 0.608f, 1.0f)
	, m_FogParams(0.01, 0.0f, 0.0f, 0.0f)
{
	for (int i = 0; i < _countof(m_ShadowRT); i++)
	{
		m_ShadowRT[i].reset(new Texture2D());
		m_ShadowDS[i].reset(new Surface());
		handle_ShadowRT[i] = NULL;
	}
}

RenderPipeline::~RenderPipeline(void)
{
}

static size_t _hash_value(const D3DXMACRO* pDefines, const char * path)
{
	// ! maybe hash conflict
	size_t seed = 0;
	boost::hash_combine(seed, boost::string_view(path));
	if (pDefines)
	{
		const D3DXMACRO* macro_iter = pDefines;
		for (; macro_iter->Name; macro_iter++)
		{
			boost::hash_combine(seed, boost::string_view(macro_iter->Name));
			if (macro_iter->Definition)
			{
				boost::hash_combine(seed, boost::string_view(macro_iter->Definition));
			}
		}
	}
	return seed;
}

my::Effect * RenderPipeline::QueryShader(const D3DXMACRO* pDefines, const char * path, unsigned int PassID)
{
	size_t seed = _hash_value(pDefines, path);
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(seed);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	if (!ResourceMgr::getSingleton().CheckPath(path))
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("Check Path failed: %s", path).c_str());
		m_ShaderCache.insert(std::make_pair(seed, my::EffectPtr()));
		return NULL;
	}

	CachePtr cache = ResourceMgr::getSingleton().OpenIStream(path)->GetWholeCache();

	my::ResourceMgr::getSingleton().m_LocalInclude = path;
	boost::replace_all(my::ResourceMgr::getSingleton().m_LocalInclude, "/", "\\");
	PathRemoveFileSpecA(&my::ResourceMgr::getSingleton().m_LocalInclude[0]);

	CComPtr<ID3DXBuffer> err;
	CComPtr<ID3DXEffectCompiler> compiler;
	if (FAILED(D3DXCreateEffectCompiler((LPCSTR)cache->data(), (UINT)cache->size(), pDefines,
		my::ResourceMgr::getSingletonPtr(), D3DXSHADER_PACKMATRIX_COLUMNMAJOR | D3DXSHADER_OPTIMIZATION_LEVEL3 | D3DXFX_LARGEADDRESSAWARE, &compiler, &err)))
	{
		my::D3DContext::getSingleton().m_EventLog(err ? (char *)err->GetBufferPointer() : "QueryShader failed");
		m_ShaderCache.insert(std::make_pair(seed, my::EffectPtr()));
		return NULL;
	}

	if (err)
	{
		my::D3DContext::getSingleton().m_EventLog((char *)err->GetBufferPointer());
		err.Release();
	}

	CComPtr<ID3DXBuffer> buff;
	if (FAILED(compiler->CompileEffect(D3DXSHADER_OPTIMIZATION_LEVEL3, &buff, &err)))
	{
		my::D3DContext::getSingleton().m_EventLog(err ? (char *)err->GetBufferPointer() : "QueryShader failed");
		m_ShaderCache.insert(std::make_pair(seed, my::EffectPtr()));
		return NULL;
	}

	if (err)
	{
		my::D3DContext::getSingleton().m_EventLog((char *)err->GetBufferPointer());
		err.Release();
	}

	TCHAR BuffPath[MAX_PATH];
	_stprintf_s(BuffPath, _countof(BuffPath), _T("ShaderCache_%zx"), seed);
	std::ofstream ofs(BuffPath, std::ios::binary, _SH_DENYRW);
	ofs.write((char*)buff->GetBufferPointer(), buff->GetBufferSize());
	ofs.flush();

	LPD3DXEFFECT pEffect = NULL;
	if (FAILED(D3DXCreateEffect(my::D3DContext::getSingleton().m_d3dDevice,
		buff->GetBufferPointer(), buff->GetBufferSize(), NULL, NULL, D3DXSHADER_OPTIMIZATION_LEVEL3 | D3DXFX_LARGEADDRESSAWARE, my::ResourceMgr::getSingleton().m_EffectPool, &pEffect, &err)))
	{
		my::D3DContext::getSingleton().m_EventLog(err ? (char *)err->GetBufferPointer() : "QueryShader failed");
		m_ShaderCache.insert(std::make_pair(seed, my::EffectPtr()));
		return NULL;
	}

	my::EffectPtr shader(new my::Effect());
	shader->Create(pEffect);
	m_ShaderCache.insert(std::make_pair(seed, shader));
	return shader.get();
}

void RenderPipeline::LoadShaderCache(LPCTSTR szDir)
{
	std::basic_string<TCHAR> dir(szDir);
	dir.append(_T("\\*"));
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(dir.c_str(), &ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				boost::basic_regex<TCHAR> reg(_T("ShaderCache_([0-9a-fA-F]+)"));
				boost::match_results<const TCHAR *> what;
				if (boost::regex_search(ffd.cFileName, what, reg, boost::match_default) && what[1].matched)
				{
					std::basic_string<TCHAR> seed_str(what[1].first, what[1].second);
					std::basic_stringstream<TCHAR> ss;
					ss << std::hex << seed_str;
					size_t seed;
					ss >> seed;
					std::basic_string<TCHAR> path(szDir);
					path.append(_T("\\")).append(ffd.cFileName);
					my::EffectPtr shader(new my::Effect());
					shader->CreateEffectFromFile(path.c_str(), NULL, NULL, D3DXSHADER_OPTIMIZATION_LEVEL3, my::ResourceMgr::getSingleton().m_EffectPool);
					m_ShaderCache.insert(std::make_pair(seed, shader));
				}
			}
		} while (FindNextFile(hFind, &ffd) != 0);
	}
}

const char * RenderPipeline::PassTypeToStr(unsigned int pass_type)
{
	switch (pass_type)
	{
	case PassTypeShadow:
		return "PassTypeShadow";
	case PassTypeNormal:
		return "PassTypeNormal";
	case PassTypeLight:
		return "PassTypeLight";
	case PassTypeBackground:
		return "PassTypeBackground";
	case PassTypeOpaque:
		return "PassTypeOpaque";
	case PassTypeTransparent:
		return "PassTypeTransparent";
	}
	return "unknown pass type";
}

void RenderPipeline::UpdateQuad(QuadVertex * quad, const my::Vector2 & dim)
{
	quad[0].x = -0.5f;
	quad[0].y = -0.5f;
	quad[0].z = 1.0f;
	quad[0].rhw = 1.0f;
	quad[0].u = 0.0f;
	quad[0].v = 0.0f;

	quad[1].x = -0.5f;
	quad[1].y = dim.y - 0.5f;
	quad[1].z = 1.0f;
	quad[1].rhw = 1.0f;
	quad[1].u = 0.0f;
	quad[1].v = 1.0f;

	quad[2].x = dim.x - 0.5f;
	quad[2].y = dim.y - 0.5f;
	quad[2].z = 1.0f;
	quad[2].rhw = 1.0f;
	quad[2].u = 1.0f;
	quad[2].v = 1.0f;

	quad[3].x = dim.x - 0.5f;
	quad[3].y = -0.5f;
	quad[3].z = 1.0f;
	quad[3].rhw = 1.0f;
	quad[3].u = 1.0f;
	quad[3].v = 0.0f;
}

HRESULT RenderPipeline::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	WORD offset = 0;
	m_ParticleVertElems.InsertPositionElement(offset);
	offset += sizeof(my::Vector3);
	m_ParticleVertElems.InsertTexcoordElement(offset, 0);
	offset += sizeof(my::Vector2);
	m_ParticleVertElems.InsertNormalElement(offset, 0);
	offset += sizeof(my::Vector3);
	m_ParticleVertElems.InsertTangentElement(offset, 0);
	offset += sizeof(my::Vector3);
	_ASSERT(m_ParticleVertStride == offset);

	offset = 0;
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 1);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 2);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 3);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 4);
	offset += sizeof(Vector4);
	_ASSERT(m_ParticleInstanceStride == offset);

	m_ParticleIEList = m_ParticleVertElems.BuildVertexElementList(0);
	std::vector<D3DVERTEXELEMENT9> elems = m_ParticleInstanceElems.BuildVertexElementList(1);
	m_ParticleIEList.insert(m_ParticleIEList.end(), elems.begin(), elems.end());
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	m_ParticleIEList.push_back(ve_end);

	_ASSERT(D3DXGetDeclVertexSize(&m_ParticleIEList[0], 0) == m_ParticleVertStride);
	_ASSERT(D3DXGetDeclVertexSize(&m_ParticleIEList[0], 1) == m_ParticleInstanceStride);

	offset = 0;
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 1);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 2);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 3);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 4);
	offset += sizeof(Vector4);
	m_MeshInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_COLOR, 1);
	offset += sizeof(Vector4);
	_ASSERT(m_MeshInstanceStride == offset);

	m_MeshIEList = m_MeshInstanceElems.BuildVertexElementList(1);

	if (!(m_SimpleSample = my::ResourceMgr::getSingleton().LoadEffect("shader/SimpleSample.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SimpleSample failed");
	}

	BOOST_VERIFY(handle_Time = m_SimpleSample->GetParameterByName(NULL, "g_Time"));
	BOOST_VERIFY(handle_ScreenDim = m_SimpleSample->GetParameterByName(NULL, "g_ScreenDim"));
	BOOST_VERIFY(handle_ShadowMapSize = m_SimpleSample->GetParameterByName(NULL, "g_ShadowMapSize"));
	BOOST_VERIFY(handle_ShadowBias = m_SimpleSample->GetParameterByName(NULL, "g_ShadowBias"));
	BOOST_VERIFY(handle_ShadowLayer = m_SimpleSample->GetParameterByName(NULL, "g_ShadowLayer"));
	BOOST_VERIFY(handle_World = m_SimpleSample->GetParameterByName(NULL, "g_World"));
	BOOST_VERIFY(handle_Eye = m_SimpleSample->GetParameterByName(NULL, "g_Eye"));
	BOOST_VERIFY(handle_View = m_SimpleSample->GetParameterByName(NULL, "g_View"));
	BOOST_VERIFY(handle_ViewProj = m_SimpleSample->GetParameterByName(NULL, "g_ViewProj"));
	BOOST_VERIFY(handle_SkyLightDir = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightDir"));
	BOOST_VERIFY(handle_SkyLightViewProj = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightViewProj"));
	BOOST_VERIFY(handle_SkyLightColor = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightColor"));
	BOOST_VERIFY(handle_AmbientColor = m_SimpleSample->GetParameterByName(NULL, "g_AmbientColor"));
	for (int i = 0; i < _countof(handle_ShadowRT); i++)
	{
		char buff[64];
		sprintf_s(buff, _countof(buff), "g_ShadowRT%d", i);
		BOOST_VERIFY(handle_ShadowRT[i] = m_SimpleSample->GetParameterByName(NULL, buff));
	}
	BOOST_VERIFY(handle_NormalRT = m_SimpleSample->GetParameterByName(NULL, "g_NormalRT"));
	BOOST_VERIFY(handle_SpecularRT = m_SimpleSample->GetParameterByName(NULL, "g_SpecularRT"));
	BOOST_VERIFY(handle_PositionRT = m_SimpleSample->GetParameterByName(NULL, "g_PositionRT"));
	BOOST_VERIFY(handle_LightRT = m_SimpleSample->GetParameterByName(NULL, "g_LightRT"));
	BOOST_VERIFY(handle_OpaqueRT = m_SimpleSample->GetParameterByName(NULL, "g_OpaqueRT"));
	BOOST_VERIFY(handle_DownFilterRT = m_SimpleSample->GetParameterByName(NULL, "g_DownFilterRT"));
	return S_OK;
}

HRESULT RenderPipeline::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_ParticleIEDecl && !m_ParticleIEList.empty());
	HRESULT hr;
	V(pd3dDevice->CreateVertexDeclaration(&m_ParticleIEList[0], &m_ParticleIEDecl));
		
	_ASSERT(!m_ParticleVb.m_ptr);
	m_ParticleVb.CreateVertexBuffer((m_ParticlePrimitiveInfo[ParticlePrimitiveTri][ParticlePrimitiveNumVertices]
		+ m_ParticlePrimitiveInfo[ParticlePrimitiveQuad][ParticlePrimitiveNumVertices]) * m_ParticleVertStride, 0, 0, D3DPOOL_DEFAULT);
	unsigned char * pVertices = (unsigned char *)m_ParticleVb.Lock(0, 0, 0);
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 0, Vector3(0.0f, 1.0f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 0, Vector2(0.5f, 0.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 0, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 0, Vector3(1.0f, 0.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 1, Vector3(-0.5f, 0.0f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 1, Vector2(0.0f, 1.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 1, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 1, Vector3(1.0f, 0.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 2, Vector3(0.5f, 0.0f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 2, Vector2(1.0f, 1.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 2, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 2, Vector3(1.0f, 0.0f, 0.0f));

	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 3, Vector3(-0.5f, 0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 3, Vector2(0.0f, 0.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 3, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 3, Vector3(1.0f, 0.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 4, Vector3(-0.5f, -0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 4, Vector2(0.0f, 1.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 4, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 4, Vector3(1.0f, 0.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 5, Vector3(0.5f, 0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 5, Vector2(1.0f, 0.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 5, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 5, Vector3(1.0f, 0.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 6, Vector3(0.5f, -0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 6, Vector2(1.0f, 1.0f));
	m_ParticleVertElems.SetNormal(pVertices + m_ParticleVertStride * 6, Vector3(0.0f, 0.0f, 1.0f));
	m_ParticleVertElems.SetTangent(pVertices + m_ParticleVertStride * 6, Vector3(1.0f, 0.0f, 0.0f));
	m_ParticleVb.Unlock();

	_ASSERT(!m_ParticleIb.m_ptr);
	m_ParticleIb.CreateIndexBuffer((m_ParticlePrimitiveInfo[ParticlePrimitiveTri][ParticlePrimitivePrimitiveCount]
		+ m_ParticlePrimitiveInfo[ParticlePrimitiveQuad][ParticlePrimitivePrimitiveCount]) * 3 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT);
	WORD * pIndices = (WORD *)m_ParticleIb.Lock(0, 0, 0);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pIndices[3] = 3 + 0;
	pIndices[4] = 3 + 1;
	pIndices[5] = 3 + 2;
	pIndices[6] = 3 + 2;
	pIndices[7] = 3 + 1;
	pIndices[8] = 3 + 3;
	m_ParticleIb.Unlock();

	_ASSERT(!m_ParticleInstanceData.m_ptr);
	m_ParticleInstanceData.CreateVertexBuffer(m_ParticleInstanceStride * PARTICLE_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	_ASSERT(!m_MeshInstanceData.m_ptr);
	m_MeshInstanceData.CreateVertexBuffer(m_MeshInstanceStride * MESH_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	for (int i = 0; i < _countof(m_ShadowRT); i++)
	{
		m_ShadowRT[i]->CreateTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

		// ! 所有的 render target必须使用具有相同 multisample的 depth stencil
		m_ShadowDS[i]->CreateDepthStencilSurface(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);
	}

	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ParticleIEDecl.Release();
	m_ParticleVb.OnDestroyDevice();
	m_ParticleIb.OnDestroyDevice();
	m_ParticleInstanceData.OnDestroyDevice();
	m_MeshInstanceData.OnDestroyDevice();
	for (int i = 0; i < _countof(m_ShadowRT); i++)
	{
		m_ShadowRT[i]->OnDestroyDevice();
		m_ShadowDS[i]->OnDestroyDevice();
	}
}

void RenderPipeline::OnDestroyDevice(void)
{
	_ASSERT(!m_ParticleVb.m_ptr);

	_ASSERT(!m_ParticleIb.m_ptr);

	_ASSERT(!m_ParticleInstanceData.m_ptr);

	_ASSERT(!m_MeshInstanceData.m_ptr);

	ClearShaderCache();

	ClearAllObjects();

	//unsigned int PassID = 0;
	//for (; PassID < m_Pass.size(); PassID++)
	//{
	//	//MeshInstanceAtomMap::iterator mesh_inst_iter = m_MeshInstanceMap.begin();
	//	//for (; mesh_inst_iter != m_MeshInstanceMap.end(); mesh_inst_iter++)
	//	//{
	//	//	mesh_inst_iter->second.m_Decl.Release();
	//	//}
	//	m_Pass[PassID].m_MeshInstanceMap.clear();
	//}
}

void RenderPipeline::OnRender(
	IDirect3DDevice9 * pd3dDevice,
	IDirect3DSurface9 * ScreenSurf,
	IDirect3DSurface9 * ScreenDepthStencilSurf,
	const D3DSURFACE_DESC * ScreenSurfDesc,
	IRenderContext * pRC,
	double fTime,
	float fElapsedTime)
{
	QuadVertex quad[4];
	UpdateQuad(quad, Vector2((float)ScreenSurfDesc->Width, (float)ScreenSurfDesc->Height));

	QuadVertex quad_quat[4];
	UpdateQuad(quad_quat, Vector2((float)ScreenSurfDesc->Width, (float)ScreenSurfDesc->Height) / 4.0f);

	HRESULT hr;
	//CComPtr<IDirect3DSurface9> ScreenSurf;
	//CComPtr<IDirect3DSurface9> ScreenDepthStencilSurf;
	//V(pd3dDevice->GetRenderTarget(0, &ScreenSurf));
	//V(pd3dDevice->GetDepthStencilSurface(&ScreenDepthStencilSurf));

	//D3DVIEWPORT9 vp = { 0, 0, ScreenSurfDesc->Width, ScreenSurfDesc->Height, 0.0f, 1.0f };
	//V(pd3dDevice->SetViewport(&vp));

	// ! Ogre & Apex模型都是顺时针，右手系应该是逆时针
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
	V(pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL));

	const Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_SkyLightCam->m_Euler.y, m_SkyLightCam->m_Euler.x, m_SkyLightCam->m_Euler.z);
	m_SimpleSample->SetFloat(handle_Time, my::D3DContext::getSingleton().m_fTotalTime);
	m_SimpleSample->SetVector(handle_ScreenDim, Vector2((float)ScreenSurfDesc->Width, (float)ScreenSurfDesc->Height));
	m_SimpleSample->SetFloat(handle_ShadowMapSize, (float)SHADOW_MAP_SIZE);
	m_SimpleSample->SetVector(handle_ShadowBias, m_CascadeLayerBias);
	Matrix4 SkyLightViewProj[_countof(m_ShadowRT)];
	for (int i = 0; i < _countof(m_ShadowRT); i++)
	{
		const Vector3 ltf = Vector3(-1.0f, 1.0f, m_CascadeLayer[i]).transformCoord(pRC->m_Camera->m_InverseViewProj);
		Vector3 eye = Vector3(0.0f, 0.0f, m_CascadeLayerCent[i]).transformCoord(pRC->m_Camera->m_InverseViewProj);
		const float radius = ltf.distance(eye);
		const Matrix4 Proj = Matrix4::OrthoOffCenterRH(-radius, radius, -radius, radius, Min(-radius, m_SkyLightCam->m_Nz), radius);
		const Matrix4 ViewProj = Rotation.inverse() * Proj;
		Vector4 ProjEye = eye.transform(ViewProj);
		ProjEye.x = floor(ProjEye.x / ProjEye.w * SHADOW_MAP_SIZE * 0.5f) * 2.0f / SHADOW_MAP_SIZE * ProjEye.w;
		ProjEye.y = floor(ProjEye.y / ProjEye.w * SHADOW_MAP_SIZE * 0.5f) * 2.0f / SHADOW_MAP_SIZE * ProjEye.w;
		eye = ProjEye.transform(ViewProj.inverse()).xyz;
		const Matrix4 View = (Rotation * Matrix4::Translation(eye)).inverse();
		SkyLightViewProj[i] = { View * Proj };

		pRC->QueryRenderComponent(Frustum::ExtractMatrix(SkyLightViewProj[i]), this, PassTypeToMask(PassTypeShadow));

		CComPtr<IDirect3DSurface9> ShadowSurf = m_ShadowRT[i]->GetSurfaceLevel(0);
		m_SimpleSample->SetVector(handle_ShadowLayer, m_CascadeLayer);
		m_SimpleSample->SetMatrix(handle_World, Matrix4::identity);
		m_SimpleSample->SetVector(handle_Eye, eye);
		m_SimpleSample->SetMatrix(handle_View, View);
		m_SimpleSample->SetMatrix(handle_ViewProj, SkyLightViewProj[i]);
		V(pd3dDevice->SetRenderTarget(0, ShadowSurf));
		V(pd3dDevice->SetDepthStencilSurface(m_ShadowDS[i]->m_ptr));
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0.0f, 0));
		RenderAllObjects(pd3dDevice, PassTypeShadow, pRC, fTime, fElapsedTime);
		ShadowSurf.Release();
	}

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_Camera->m_ViewProj), this, PassTypeToMask(PassTypeNormal) | PassTypeToMask(PassTypeLight) | PassTypeToMask(PassTypeBackground) | PassTypeToMask(PassTypeOpaque) | PassTypeToMask(PassTypeTransparent));

	CComPtr<IDirect3DSurface9> NormalSurf = pRC->m_NormalRT->GetSurfaceLevel(0);
	CComPtr<IDirect3DSurface9> SpecularSurf = pRC->m_SpecularRT->GetSurfaceLevel(0);
	CComPtr<IDirect3DSurface9> PositionSurf = pRC->m_PositionRT->GetSurfaceLevel(0);
	m_SimpleSample->SetVector(handle_Eye, pRC->m_Camera->m_Eye);
	m_SimpleSample->SetMatrix(handle_View, pRC->m_Camera->m_View);
	m_SimpleSample->SetMatrix(handle_ViewProj, pRC->m_Camera->m_ViewProj);
	m_SimpleSample->SetVector(handle_SkyLightDir, Rotation.getRow<2>()); // ! RH -z, uninvertd so use transpose
	m_SimpleSample->SetMatrixArray(handle_SkyLightViewProj, SkyLightViewProj, _countof(SkyLightViewProj));
	m_SimpleSample->SetVector(handle_SkyLightColor, m_SkyLightColor);
	m_SimpleSample->SetVector(handle_AmbientColor, m_AmbientColor);
	for (int i = 0; i < _countof(handle_ShadowRT); i++)
	{
		m_SimpleSample->SetTexture(handle_ShadowRT[i], m_ShadowRT[i].get());
	}
	V(pd3dDevice->SetRenderTarget(0, NormalSurf));
	V(pd3dDevice->SetRenderTarget(1, SpecularSurf));
	V(pd3dDevice->SetRenderTarget(2, PositionSurf));
	V(pd3dDevice->SetDepthStencilSurface(ScreenDepthStencilSurf));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0.0f, 0));
	RenderAllObjects(pd3dDevice, PassTypeNormal, pRC, fTime, fElapsedTime);

	CComPtr<IDirect3DSurface9> LightSurf = pRC->m_LightRT->GetSurfaceLevel(0);
	m_SimpleSample->SetTexture(handle_NormalRT, pRC->m_NormalRT.get());
	m_SimpleSample->SetTexture(handle_SpecularRT, pRC->m_SpecularRT.get());
	m_SimpleSample->SetTexture(handle_PositionRT, pRC->m_PositionRT.get());
	V(pd3dDevice->SetRenderTarget(1, NULL));
	V(pd3dDevice->SetRenderTarget(2, NULL));
	if (pRC->m_SsaoEnable)
	{
		D3DXMACRO macro[] = { { 0 } };
		my::Effect* SsaoEffect = QueryShader(macro, "shader/SSAO.fx", PassTypeShadow);
		if (SsaoEffect)
		{
			if (!handle_bias)
			{
				BOOST_VERIFY(handle_bias = SsaoEffect->GetParameterByName(NULL, "g_bias"));
				BOOST_VERIFY(handle_intensity = SsaoEffect->GetParameterByName(NULL, "g_intensity"));
				BOOST_VERIFY(handle_sample_rad = SsaoEffect->GetParameterByName(NULL, "g_sample_rad"));
				BOOST_VERIFY(handle_scale = SsaoEffect->GetParameterByName(NULL, "g_scale"));
				BOOST_VERIFY(handle_OcclusionRT = SsaoEffect->GetParameterByName(NULL, "g_OcclusionRT"));
			}

			V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
			V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			SsaoEffect->SetFloat(handle_bias, m_SsaoBias);
			SsaoEffect->SetFloat(handle_intensity, m_SsaoIntensity);
			SsaoEffect->SetFloat(handle_sample_rad, m_SsaoRadius);
			SsaoEffect->SetFloat(handle_scale, m_SsaoScale);
			SsaoEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			SsaoEffect->BeginPass(0);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			SsaoEffect->EndPass();
			if (false)
				D3DXSaveTextureToFileA("aaa.bmp", D3DXIFF_BMP, pRC->m_OpaqueRT.GetNextTarget()->m_ptr, NULL);

			SsaoEffect->SetTexture(handle_OcclusionRT, pRC->m_OpaqueRT.GetNextTarget().get());
			V(pd3dDevice->SetRenderTarget(0, LightSurf));
			SsaoEffect->BeginPass(1);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			SsaoEffect->EndPass();
			SsaoEffect->End();
		}
	}
	else
	{
		V(pd3dDevice->SetRenderTarget(0, LightSurf));
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_COLORVALUE(
			m_AmbientColor.x, m_AmbientColor.y, m_AmbientColor.z, 0), 0, 0));
	}
	RenderAllObjects(pd3dDevice, PassTypeLight, pRC, fTime, fElapsedTime);

	m_SimpleSample->SetTexture(handle_LightRT, pRC->m_LightRT.get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
	RenderAllObjects(pd3dDevice, PassTypeBackground, pRC, fTime, fElapsedTime);
	if (m_PassDrawCall[PassTypeBackground] <= 0 && m_PassBatchDrawCall[PassTypeBackground] <= 0)
	{
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, m_BkColor, 0, 0)); // ! d3dmultisample will not work
	}

	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, !pRC->m_WireFrame ? D3DFILL_SOLID : D3DFILL_WIREFRAME));

	RenderAllObjects(pd3dDevice, PassTypeOpaque, pRC, fTime, fElapsedTime);

	if (pRC->m_FogEnable)
	{
		D3DXMACRO macro[] = { { 0 } };
		my::Effect* FogEffect = QueryShader(macro, "shader/DeferredFog.fx", PassTypeShadow);
		if (FogEffect)
		{
			if (!handle_FogColor)
			{
				BOOST_VERIFY(handle_FogColor = FogEffect->GetParameterByName(NULL, "g_FogColor"));
				BOOST_VERIFY(handle_FogParams = FogEffect->GetParameterByName(NULL, "g_FogParams"));
			}

			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
			V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
			V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
			FogEffect->SetVector(handle_FogColor, m_FogColor);
			FogEffect->SetVector(handle_FogParams, m_FogParams);
			FogEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			FogEffect->BeginPass(0);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			FogEffect->EndPass();
			FogEffect->End();
		}
	}

	V(pd3dDevice->StretchRect(pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0), NULL, pRC->m_OpaqueRT.GetNextSource()->GetSurfaceLevel(0), NULL, D3DTEXF_NONE));
	m_SimpleSample->SetTexture(handle_OpaqueRT, pRC->m_OpaqueRT.GetNextSource().get());
	RenderAllObjects(pd3dDevice, PassTypeTransparent, pRC, fTime, fElapsedTime);

	pRC->m_OpaqueRT.Flip();

	if (pRC->m_DofEnable)
	{
		D3DXMACRO macro[] = { { 0 } };
		my::Effect* DofEffect = QueryShader(macro, "shader/DofEffect.fx", PassTypeShadow);
		if (DofEffect)
		{
			if (!handle_DofParams)
			{
				BOOST_VERIFY(handle_DofParams = DofEffect->GetParameterByName(NULL, "g_DofParams"));
			}

			V(pd3dDevice->StretchRect(pRC->m_OpaqueRT.GetNextSource()->GetSurfaceLevel(0), NULL,
				pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0), NULL, D3DTEXF_NONE)); // ! d3dref only support D3DTEXF_NONE
			pRC->m_DownFilterRT.Flip();

			DofEffect->SetVector(handle_DofParams, m_DofParams);
			m_SimpleSample->SetTexture(handle_OpaqueRT, pRC->m_OpaqueRT.GetNextSource().get());
			m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
			V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			UINT passes = DofEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			DofEffect->BeginPass(0);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
			DofEffect->EndPass();
			pRC->m_DownFilterRT.Flip();

			m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
			V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
			DofEffect->BeginPass(1);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
			DofEffect->EndPass();
			pRC->m_DownFilterRT.Flip();
			if (false)
				D3DXSaveTextureToFileA("aaa.bmp", D3DXIFF_BMP, pRC->m_DownFilterRT.GetNextSource()->m_ptr, NULL);

			m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
			V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
			DofEffect->BeginPass(2);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			DofEffect->EndPass();
			DofEffect->End();
			pRC->m_OpaqueRT.Flip();
		}
	}

	if (pRC->m_BloomEnable)
	{
		D3DXMACRO macro[] = { { 0 } };
		my::Effect* BloomEffect = QueryShader(macro, "shader/Bloom.fx", PassTypeShadow);
		if (BloomEffect)
		{
			if (!handle_LuminanceThreshold)
			{
				BOOST_VERIFY(handle_LuminanceThreshold = BloomEffect->GetParameterByName(NULL, "_LuminanceThreshold"));
				BOOST_VERIFY(handle_BloomColor = BloomEffect->GetParameterByName(NULL, "_BloomColor"));
				BOOST_VERIFY(handle_BloomFactor = BloomEffect->GetParameterByName(NULL, "_BloomFactor"));
			}

			V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
			BloomEffect->SetFloat(handle_LuminanceThreshold, m_LuminanceThreshold);
			BloomEffect->SetVector(handle_BloomColor, m_BloomColor);
			BloomEffect->SetFloat(handle_BloomFactor, m_BloomFactor);
			m_SimpleSample->SetTexture(handle_OpaqueRT, pRC->m_OpaqueRT.GetNextSource().get());
			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			UINT passes = BloomEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			BloomEffect->BeginPass(0);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
			BloomEffect->EndPass();
			pRC->m_DownFilterRT.Flip();

			m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
			V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
			BloomEffect->BeginPass(1);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
			BloomEffect->EndPass();
			pRC->m_DownFilterRT.Flip();

			m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
			V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
			BloomEffect->BeginPass(2);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
			BloomEffect->EndPass();
			pRC->m_DownFilterRT.Flip();
			if (false)
				D3DXSaveTextureToFileA("aaa.bmp", D3DXIFF_BMP, pRC->m_DownFilterRT.GetNextSource()->m_ptr, NULL);

			m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
			V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
			BloomEffect->BeginPass(3);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			BloomEffect->EndPass();
			BloomEffect->End();
			pRC->m_OpaqueRT.Flip();
		}
	}

	if (pRC->m_FxaaEnable)
	{
		D3DXMACRO macro[] = { { 0 } };
		my::Effect* FxaaEffect = QueryShader(macro, "shader/FXAA.fx", PassTypeShadow);
		if (FxaaEffect)
		{
			if (!handle_InputTexture)
			{
				BOOST_VERIFY(handle_InputTexture = FxaaEffect->GetParameterByName(NULL, "InputTexture"));
				BOOST_VERIFY(handle_RCPFrame = FxaaEffect->GetParameterByName(NULL, "RCPFrame"));
			}
			V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			FxaaEffect->SetTexture(handle_InputTexture, pRC->m_OpaqueRT.GetNextSource().get());
			Vector4 RCPFrame(1.0f / ScreenSurfDesc->Width, 1.0f / ScreenSurfDesc->Height, 0.0f, 0.0f);
			FxaaEffect->SetFloatArray(handle_RCPFrame, &RCPFrame.x, sizeof(RCPFrame) / sizeof(float));
			FxaaEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			FxaaEffect->BeginPass(0);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			FxaaEffect->EndPass();
			FxaaEffect->End();
			pRC->m_OpaqueRT.Flip();
		}
	}

	switch (pRC->m_RTType)
	{
	case RenderTargetNormal:
	{
		D3DXMACRO macro[] = { { 0 } };
		my::Effect* NormalCvt = QueryShader(macro, "shader/NormalCvt.fx", PassTypeShadow);
		if (NormalCvt)
		{
			//V(pd3dDevice->StretchRect(NormalSurf, NULL, ScreenSurf, NULL, D3DTEXF_NONE));
			V(pd3dDevice->SetRenderTarget(0, ScreenSurf));
			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
			V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
			V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			NormalCvt->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			NormalCvt->BeginPass(0);
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
			NormalCvt->EndPass();
			NormalCvt->End();
		}
		break;
	}
	case RenderTargetSpecular:
		V(pd3dDevice->StretchRect(SpecularSurf, NULL, ScreenSurf, NULL, D3DTEXF_NONE));
		break;
	case RenderTargetPosition:
		V(pd3dDevice->StretchRect(PositionSurf, NULL, ScreenSurf, NULL, D3DTEXF_NONE));
		break;
	case RenderTargetLight:
		V(pd3dDevice->StretchRect(LightSurf, NULL, ScreenSurf, NULL, D3DTEXF_NONE));
		break;
	case RenderTargetOpaque:
		V(pd3dDevice->StretchRect(pRC->m_OpaqueRT.GetNextSource()->GetSurfaceLevel(0), NULL, ScreenSurf, NULL, D3DTEXF_NONE));
		break;
	case RenderTargetDownFilter:
		V(pd3dDevice->StretchRect(pRC->m_DownFilterRT.GetNextSource()->GetSurfaceLevel(0), NULL, ScreenSurf, NULL, D3DTEXF_NONE));
		break;
	}

	ClearAllObjects();
}

void RenderPipeline::RenderAllObjects(
	IDirect3DDevice9 * pd3dDevice,
	unsigned int PassID,
	IRenderContext * pRC,
	double fTime,
	float fElapsedTime)
{
	m_PassDrawCall[PassID] = 0;

	m_PassBatchDrawCall[PassID] = 0;

	IndexedPrimitiveAtomList::iterator prim_iter = m_Pass[PassID].m_IndexedPrimitiveList.begin();
	for (; prim_iter != m_Pass[PassID].m_IndexedPrimitiveList.end(); prim_iter++)
	{
		DrawIndexedPrimitive(
			pd3dDevice,
			PassID,
			pRC,
			prim_iter->pDecl,
			prim_iter->pVB,
			prim_iter->pIB,
			prim_iter->PrimitiveType,
			prim_iter->BaseVertexIndex,
			prim_iter->MinVertexIndex,
			prim_iter->NumVertices,
			prim_iter->VertexStride,
			prim_iter->StartIndex,
			prim_iter->PrimitiveCount,
			prim_iter->shader,
			prim_iter->cmp,
			prim_iter->mtl,
			prim_iter->lparam);
		m_PassDrawCall[PassID]++;
	}

	IndexedPrimitiveInstanceAtomList::iterator prim_inst_iter = m_Pass[PassID].m_IndexedPrimitiveInstanceList.begin();
	for (; prim_inst_iter != m_Pass[PassID].m_IndexedPrimitiveInstanceList.end(); prim_inst_iter++)
	{
		DrawIndexedPrimitiveInstance(
			pd3dDevice,
			PassID,
			pRC,
			prim_inst_iter->pDecl,
			prim_inst_iter->pVB,
			prim_inst_iter->pIB,
			prim_inst_iter->pInstance,
			prim_inst_iter->PrimitiveType,
			prim_inst_iter->BaseVertexIndex,
			prim_inst_iter->MinVertexIndex,
			prim_inst_iter->NumVertices,
			prim_inst_iter->VertexStride,
			prim_inst_iter->StartIndex,
			prim_inst_iter->PrimitiveCount,
			prim_inst_iter->NumInstance,
			prim_inst_iter->InstanceStride,
			prim_inst_iter->shader,
			prim_inst_iter->cmp,
			prim_inst_iter->mtl,
			prim_inst_iter->lparam);
		m_PassDrawCall[PassID]++;
	}

	IndexedPrimitiveUPAtomList::iterator prim_up_iter = m_Pass[PassID].m_IndexedPrimitiveUPList.begin();
	for (; prim_up_iter != m_Pass[PassID].m_IndexedPrimitiveUPList.end(); prim_up_iter++)
	{
		DrawIndexedPrimitiveUP(
			pd3dDevice,
			PassID,
			pRC,
			prim_up_iter->pDecl,
			prim_up_iter->PrimitiveType,
			prim_up_iter->MinVertexIndex,
			prim_up_iter->NumVertices,
			prim_up_iter->PrimitiveCount,
			prim_up_iter->pIndexData,
			prim_up_iter->IndexDataFormat,
			prim_up_iter->pVertexStreamZeroData,
			prim_up_iter->VertexStreamZeroStride,
			prim_up_iter->shader,
			prim_up_iter->cmp,
			prim_up_iter->mtl,
			prim_up_iter->lparam);
		m_PassDrawCall[PassID]++;
	}

	MeshAtomList::iterator mesh_iter = m_Pass[PassID].m_MeshList.begin();
	for (; mesh_iter != m_Pass[PassID].m_MeshList.end(); mesh_iter++)
	{
		const D3DXATTRIBUTERANGE& rang = mesh_iter->mesh->m_AttribTable[mesh_iter->AttribId];
		DrawIndexedPrimitive(
			pd3dDevice,
			PassID,
			pRC,
			mesh_iter->mesh->m_Decl,
			mesh_iter->mesh->m_Vb.m_ptr,
			mesh_iter->mesh->m_Ib.m_ptr,
			D3DPT_TRIANGLELIST,
			0, rang.VertexStart,
			rang.VertexCount,
			mesh_iter->mesh->GetNumBytesPerVertex(),
			rang.FaceStart * 3,
			rang.FaceCount,
			mesh_iter->shader,
			mesh_iter->cmp,
			mesh_iter->mtl,
			mesh_iter->lparam);
		m_PassDrawCall[PassID]++;
	}

	MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
	for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
	{
		if (!mesh_inst_iter->second.cmps.empty())
		{
			DWORD AttribId = mesh_inst_iter->first.get<1>();
			_ASSERT(AttribId < mesh_inst_iter->first.get<0>()->m_AttribTable.size());
			const UINT NumInstances = (UINT)mesh_inst_iter->second.cmps.size();

			unsigned char * pVertices = (unsigned char *)m_MeshInstanceData.Lock(0, NumInstances * m_MeshInstanceStride, D3DLOCK_DISCARD);
			for (DWORD i = 0; i < mesh_inst_iter->second.cmps.size() && i < MESH_INSTANCE_MAX; i++)
			{
				*m_MeshInstanceElems.GetVertexValue<Matrix4>(pVertices + i * m_MeshInstanceStride, D3DDECLUSAGE_POSITION, 1) = mesh_inst_iter->second.cmps[i]->m_Actor->m_World;
				*m_MeshInstanceElems.GetVertexValue<Vector4>(pVertices + i * m_MeshInstanceStride, D3DDECLUSAGE_COLOR, 1) = mesh_inst_iter->second.cmps[i]->m_MeshColor;
			}
			m_MeshInstanceData.Unlock();

			my::OgreMesh* mesh = mesh_inst_iter->first.get<0>();
			DrawIndexedPrimitiveInstance(
				pd3dDevice,
				PassID,
				pRC,
				mesh_inst_iter->second.m_Decl,
				mesh->m_Vb.m_ptr,
				mesh->m_Ib.m_ptr,
				m_MeshInstanceData.m_ptr,
				D3DPT_TRIANGLELIST,
				0, mesh->m_AttribTable[AttribId].VertexStart,
				mesh->m_AttribTable[AttribId].VertexCount,
				mesh_inst_iter->second.m_VertexStride,
				mesh->m_AttribTable[AttribId].FaceStart * 3,
				mesh->m_AttribTable[AttribId].FaceCount,
				NumInstances,
				m_MeshInstanceStride,
				mesh_inst_iter->first.get<2>(),
				mesh_inst_iter->second.cmps.front(),
				mesh_inst_iter->first.get<3>(),
				mesh_inst_iter->first.get<4>());
			m_PassDrawCall[PassID]++;
		}
	}

	MeshBatchAtomMap::iterator mesh_bat_iter = m_Pass[PassID].m_MeshBatchMap.begin();
	for (; mesh_bat_iter != m_Pass[PassID].m_MeshBatchMap.end(); mesh_bat_iter++)
	{
		if (!mesh_bat_iter->second.cmps.empty())
		{
			HRESULT hr;
			V(pd3dDevice->SetStreamSource(0, mesh_bat_iter->first->m_Vb.m_ptr, 0, mesh_bat_iter->first->GetNumBytesPerVertex()));
			V(pd3dDevice->SetVertexDeclaration(mesh_bat_iter->first->m_Decl));
			V(pd3dDevice->SetIndices(mesh_bat_iter->first->m_Ib.m_ptr));
			mesh_bat_iter->second.cmps.front().get<0>()->OnSetShader(pd3dDevice, mesh_bat_iter->second.shader, mesh_bat_iter->second.lparam);
			const UINT passes = mesh_bat_iter->second.shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
			_ASSERT(PassID < passes);
			{
				mesh_bat_iter->second.mtl->OnSetShader(pd3dDevice, mesh_bat_iter->second.shader, mesh_bat_iter->second.lparam, pRC);
				mesh_bat_iter->second.shader->BeginPass(PassID);
				std::vector<boost::tuple<Component*, unsigned int> >::iterator cmp_iter = mesh_bat_iter->second.cmps.begin();
				for (; cmp_iter != mesh_bat_iter->second.cmps.end(); cmp_iter++)
				{
					const D3DXATTRIBUTERANGE& rang = mesh_bat_iter->first->m_AttribTable[cmp_iter->get<1>()];
					V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
						0, rang.VertexStart, rang.VertexCount, rang.FaceStart * 3, rang.FaceCount));
					m_PassBatchDrawCall[PassID]++;
				}
				mesh_bat_iter->second.shader->EndPass();
			}
			mesh_bat_iter->second.shader->End();
			m_PassDrawCall[PassID]++;
		}
	}

	EmitterInstanceAtomMap::iterator emitter_inst_iter = m_Pass[PassID].m_EmitterInstanceMap.begin();
	for (; emitter_inst_iter != m_Pass[PassID].m_EmitterInstanceMap.end(); emitter_inst_iter++)
	{
		if (!emitter_inst_iter->second.cmps.empty())
		{
			_ASSERT(m_ParticleInstanceStride == sizeof(Emitter::ParticleList::value_type));
			DWORD NumTotalInstances = 0, NumParticles = 0;
			unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, 0, D3DLOCK_DISCARD);
			std::vector<boost::tuple<Component*, my::Emitter::Particle*, unsigned int> >::const_iterator cmp_iter = emitter_inst_iter->second.cmps.begin();
			for (; cmp_iter != emitter_inst_iter->second.cmps.end(); /*cmp_iter++*/)
			{
				//int Count = Min<int>(PARTICLE_INSTANCE_MAX - NumTotalInstances, cmp_iter->get<2>());
				//memcpy(pVertices + NumTotalInstances * sizeof(Emitter::Particle), cmp_iter->get<1>(), Count * sizeof(Emitter::Particle));
				//NumTotalInstances += Count;

				int Count = cmp_iter->get<2>() - NumParticles;
				if (NumTotalInstances + Count > PARTICLE_INSTANCE_MAX)
				{
					Count = PARTICLE_INSTANCE_MAX - NumTotalInstances;
					memcpy(pVertices + NumTotalInstances * sizeof(Emitter::Particle), cmp_iter->get<1>() + NumParticles, Count * sizeof(Emitter::Particle));
					m_ParticleInstanceData.Unlock();
					DrawIndexedPrimitiveInstance(
						pd3dDevice,
						PassID,
						pRC,
						emitter_inst_iter->first.get<2>(),
						emitter_inst_iter->first.get<0>(),
						emitter_inst_iter->first.get<1>(),
						m_ParticleInstanceData.m_ptr,
						emitter_inst_iter->second.PrimitiveType,
						0, emitter_inst_iter->first.get<3>(),
						emitter_inst_iter->second.NumVertices,
						emitter_inst_iter->second.VertexStride,
						emitter_inst_iter->second.StartIndex,
						emitter_inst_iter->second.PrimitiveCount,
						PARTICLE_INSTANCE_MAX,
						m_ParticleInstanceStride,
						emitter_inst_iter->first.get<5>(),
						emitter_inst_iter->second.cmps.front().get<0>(),
						emitter_inst_iter->first.get<6>(),
						emitter_inst_iter->first.get<7>());
					m_PassDrawCall[PassID]++;
					NumParticles += Count;
					NumTotalInstances = 0;
					pVertices = (unsigned char*)m_ParticleInstanceData.Lock(0, 0, D3DLOCK_DISCARD);
				}
				else
				{
					memcpy(pVertices + NumTotalInstances * sizeof(Emitter::Particle), cmp_iter->get<1>() + NumParticles, Count * sizeof(Emitter::Particle));
					NumParticles = 0;
					NumTotalInstances += Count;
					cmp_iter++;
				}
			}
			m_ParticleInstanceData.Unlock();

			if (NumTotalInstances > 0)
			{
				DrawIndexedPrimitiveInstance(
					pd3dDevice,
					PassID,
					pRC,
					emitter_inst_iter->first.get<2>(),
					emitter_inst_iter->first.get<0>(),
					emitter_inst_iter->first.get<1>(),
					m_ParticleInstanceData.m_ptr,
					emitter_inst_iter->second.PrimitiveType,
					0, emitter_inst_iter->first.get<3>(),
					emitter_inst_iter->second.NumVertices,
					emitter_inst_iter->second.VertexStride,
					emitter_inst_iter->second.StartIndex,
					emitter_inst_iter->second.PrimitiveCount,
					NumTotalInstances,
					m_ParticleInstanceStride,
					emitter_inst_iter->first.get<5>(),
					emitter_inst_iter->second.cmps.front().get<0>(),
					emitter_inst_iter->first.get<6>(),
					emitter_inst_iter->first.get<7>());
				m_PassDrawCall[PassID]++;
			}
		}
	}
}

void RenderPipeline::ClearAllObjects(void)
{
	unsigned int PassID = 0;
	for (; PassID < m_Pass.size(); PassID++)
	{
		m_Pass[PassID].m_IndexedPrimitiveList.clear();
		m_Pass[PassID].m_IndexedPrimitiveInstanceList.clear();
		m_Pass[PassID].m_IndexedPrimitiveUPList.clear();
		m_Pass[PassID].m_MeshList.clear();
		//MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
		//for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
		//{
		//	mesh_inst_iter->second.cmps.clear(); // ! key material ptr will be invalid
		//}
		m_Pass[PassID].m_MeshInstanceMap.clear();
		m_Pass[PassID].m_MeshBatchMap.clear();
		m_Pass[PassID].m_EmitterInstanceMap.clear();
	}
}

void RenderPipeline::ClearShaderCache(void)
{
	CriticalSectionLock lock(D3DContext::getSingleton().m_ShaderObjectsSec);

	m_ShaderCache.clear();

	D3DContext::ShaderResourceBaseSet::iterator obj_iter = D3DContext::getSingleton().m_ShaderObjects.begin();
	for (; obj_iter != D3DContext::getSingleton().m_ShaderObjects.end(); obj_iter++)
	{
		(*obj_iter)->OnResetShader();
	}

	handle_DofParams = NULL;
	handle_LuminanceThreshold = NULL;
	handle_InputTexture = NULL;
	handle_bias = NULL;
	handle_FogColor = NULL;
}

void RenderPipeline::DrawIndexedPrimitive(
	IDirect3DDevice9 * pd3dDevice,
	unsigned int PassID,
	IRenderContext * pRC,
	IDirect3DVertexDeclaration9* pDecl,
	IDirect3DVertexBuffer9 * pVB,
	IDirect3DIndexBuffer9 * pIB,
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	my::Effect * shader,
	Component * cmp,
	Material * mtl,
	LPARAM lparam)
{
	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, pVB, 0, VertexStride));
	V(pd3dDevice->SetVertexDeclaration(pDecl));
	V(pd3dDevice->SetIndices(pIB));
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		mtl->OnSetShader(pd3dDevice, shader, lparam, pRC);
		shader->BeginPass(PassID);
		V(pd3dDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawIndexedPrimitiveInstance(
	IDirect3DDevice9* pd3dDevice,
	unsigned int PassID,
	IRenderContext * pRC,
	IDirect3DVertexDeclaration9* pDecl,
	IDirect3DVertexBuffer9* pVB,
	IDirect3DIndexBuffer9* pIB,
	IDirect3DVertexBuffer9* pInstance,
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	UINT NumInstances,
	UINT InstanceStride,
	my::Effect* shader,
	Component* cmp,
	Material* mtl,
	LPARAM lparam)
{
	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, pVB, 0, VertexStride));
	V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
	V(pd3dDevice->SetStreamSource(1, pInstance, 0, InstanceStride));
	V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
	V(pd3dDevice->SetVertexDeclaration(pDecl));
	V(pd3dDevice->SetIndices(pIB));
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		mtl->OnSetShader(pd3dDevice, shader, lparam, pRC);
		shader->BeginPass(PassID);
		V(pd3dDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount));
		V(pd3dDevice->SetStreamSourceFreq(0, 1));
		V(pd3dDevice->SetStreamSourceFreq(1, 1));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawIndexedPrimitiveUP(
	IDirect3DDevice9 * pd3dDevice,
	unsigned int PassID,
	IRenderContext * pRC,
	IDirect3DVertexDeclaration9* pDecl,
	D3DPRIMITIVETYPE PrimitiveType,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT PrimitiveCount,
	CONST void* pIndexData,
	D3DFORMAT IndexDataFormat,
	CONST void* pVertexStreamZeroData,
	UINT VertexStreamZeroStride,
	my::Effect * shader,
	Component * cmp,
	Material * mtl,
	LPARAM lparam)
{
	HRESULT hr;
	V(pd3dDevice->SetVertexDeclaration(pDecl));
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		mtl->OnSetShader(pd3dDevice, shader, lparam, pRC);
		shader->BeginPass(PassID);
		V(pd3dDevice->DrawIndexedPrimitiveUP(
			PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::PushIndexedPrimitive(
	unsigned int PassID,
	IDirect3DVertexDeclaration9* pDecl,
	IDirect3DVertexBuffer9 * pVB,
	IDirect3DIndexBuffer9 * pIB,
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	my::Effect * shader,
	Component * cmp,
	Material * mtl,
	LPARAM lparam)
{
	IndexedPrimitiveAtom atom;
	atom.pDecl = pDecl;
	atom.pVB = pVB;
	atom.pIB = pIB;
	atom.PrimitiveType = PrimitiveType;
	atom.BaseVertexIndex = BaseVertexIndex;
	atom.MinVertexIndex = MinVertexIndex;
	atom.NumVertices = NumVertices;
	atom.VertexStride = VertexStride;
	atom.StartIndex = StartIndex;
	atom.PrimitiveCount = PrimitiveCount;
	atom.shader = shader;
	atom.cmp = cmp;
	atom.mtl = mtl;
	atom.lparam = lparam;
	m_Pass[PassID].m_IndexedPrimitiveList.push_back(atom);
}

void RenderPipeline::PushIndexedPrimitiveInstance(
	unsigned int PassID,
	IDirect3DVertexDeclaration9* pDecl,
	IDirect3DVertexBuffer9* pVB,
	IDirect3DIndexBuffer9* pIB,
	IDirect3DVertexBuffer9* pInstance,
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	UINT NumInstances,
	UINT InstanceStride,
	my::Effect* shader,
	Component* cmp,
	Material* mtl,
	LPARAM lparam)
{
	IndexedPrimitiveInstanceAtom atom;
	atom.pDecl = pDecl;
	atom.pVB = pVB;
	atom.pIB = pIB;
	atom.pInstance = pInstance;
	atom.PrimitiveType = PrimitiveType;
	atom.BaseVertexIndex = BaseVertexIndex;
	atom.MinVertexIndex = MinVertexIndex;
	atom.NumVertices = NumVertices;
	atom.VertexStride = VertexStride;
	atom.StartIndex = StartIndex;
	atom.PrimitiveCount = PrimitiveCount;
	atom.NumInstance = NumInstances;
	atom.InstanceStride = InstanceStride;
	atom.shader = shader;
	atom.cmp = cmp;
	atom.mtl = mtl;
	atom.lparam = lparam;
	m_Pass[PassID].m_IndexedPrimitiveInstanceList.push_back(atom);
}

void RenderPipeline::PushIndexedPrimitiveUP(
	unsigned int PassID,
	IDirect3DVertexDeclaration9* pDecl,
	D3DPRIMITIVETYPE PrimitiveType,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT PrimitiveCount,
	CONST void* pIndexData,
	D3DFORMAT IndexDataFormat,
	CONST void* pVertexStreamZeroData,
	UINT VertexStreamZeroStride,
	my::Effect * shader,
	Component * cmp,
	Material * mtl,
	LPARAM lparam)
{
	IndexedPrimitiveUPAtom atom;
	atom.pDecl = pDecl;
	atom.PrimitiveType = PrimitiveType;
	atom.PrimitiveCount = PrimitiveCount;
	atom.MinVertexIndex = MinVertexIndex;
	atom.NumVertices = NumVertices;
	atom.PrimitiveCount = PrimitiveCount;
	atom.pIndexData = pIndexData;
	atom.IndexDataFormat = IndexDataFormat;
	atom.pVertexStreamZeroData = pVertexStreamZeroData;
	atom.VertexStreamZeroStride = VertexStreamZeroStride;
	atom.shader = shader;
	atom.cmp = cmp;
	atom.mtl = mtl;
	atom.lparam = lparam;
	m_Pass[PassID].m_IndexedPrimitiveUPList.push_back(atom);
}

void RenderPipeline::PushMesh(unsigned int PassID, my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	MeshAtom atom;
	atom.mesh = mesh;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.cmp = cmp;
	atom.mtl = mtl;
	atom.lparam = lparam;
	m_Pass[PassID].m_MeshList.push_back(atom);
}

void RenderPipeline::PushMeshInstance(unsigned int PassID, my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, MeshComponent * mesh_cmp, Material * mtl, LPARAM lparam)
{
	MeshInstanceAtomKey key(mesh, AttribId, shader, mtl, lparam);
	std::pair<MeshInstanceAtomMap::iterator, bool> res = m_Pass[PassID].m_MeshInstanceMap.insert(std::make_pair(key, MeshInstanceAtom()));
	if (res.second)
	{
		res.first->second.m_velist.resize(MAX_FVF_DECL_SIZE);
		mesh->GetDeclaration(&res.first->second.m_velist[0]);
		unsigned int i = 0;
		for (; i < res.first->second.m_velist.size(); i++)
		{
			if (res.first->second.m_velist[i].Stream == 0xff || res.first->second.m_velist[i].Type == D3DDECLTYPE_UNUSED)
			{
				break;
			}
		}
		if (i >= res.first->second.m_velist.size())
		{
			THROW_CUSEXCEPTION("invalid vertex declaration");
		}
		res.first->second.m_velist.insert(res.first->second.m_velist.begin() + i, m_MeshIEList.begin(), m_MeshIEList.end());
		res.first->second.m_VertexStride = D3DXGetDeclVertexSize(&res.first->second.m_velist[0], 0);
		_ASSERT(D3DXGetDeclVertexSize(&res.first->second.m_velist[0], 1) == m_MeshInstanceStride);

		HRESULT hr;
		CComPtr<IDirect3DDevice9> Device = mesh->GetDevice();
		if (FAILED(hr = Device->CreateVertexDeclaration(&res.first->second.m_velist[0], &res.first->second.m_Decl)))
		{
			THROW_D3DEXCEPTION(hr);
		}
	}
	res.first->second.cmps.push_back(mesh_cmp);
}

void RenderPipeline::PushMeshBatch(unsigned int PassID, my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	std::pair<MeshBatchAtomMap::iterator, bool> res = m_Pass[PassID].m_MeshBatchMap.insert(std::make_pair(mesh, MeshBatchAtom()));
	if (res.second)
	{
		res.first->second.shader = shader;
		res.first->second.mtl = mtl;
		res.first->second.lparam = lparam;
		res.first->second.cmps.push_back(boost::make_tuple(cmp, AttribId));
	}
	else
	{
		//_ASSERT(res.first->second.shader == shader);
		//_ASSERT(boost::hash_value(*res.first->second.mtl) == boost::hash_value(*mtl));
		//_ASSERT(res.first->second.lparam == lparam);
		//_ASSERT(res.first->second.cmps.front()->m_Actor->m_World == mesh_cmp->m_Actor->m_World);
		res.first->second.cmps.push_back(boost::make_tuple(cmp, AttribId));
	}
}

void RenderPipeline::PushEmitter(
	unsigned int PassID,
	IDirect3DVertexBuffer9* pVB,
	IDirect3DIndexBuffer9* pIB,
	IDirect3DVertexDeclaration9* pDecl,
	UINT MinVertexIndex,
	UINT NumVertices,
	DWORD VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	my::Emitter::Particle* particles,
	unsigned int particle_num,
	my::Effect* shader,
	Component* cmp,
	Material* mtl,
	LPARAM lparam)
{
//#ifdef _DEBUG
//	HRESULT hr;
//	D3DVERTEXBUFFER_DESC desc;
//	V(pVB->GetDesc(&desc));
//	_ASSERT(desc.Size == NumVertices * m_ParticleVertStride);
//#endif

	EmitterInstanceAtomKey key(pVB, pIB, pDecl, MinVertexIndex,
		dynamic_cast<EmitterComponent *>(cmp)->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld ? &Matrix4::identity : &cmp->m_Actor->m_World, shader, mtl, lparam);
	std::pair<EmitterInstanceAtomMap::iterator, bool> res = m_Pass[PassID].m_EmitterInstanceMap.insert(std::make_pair(key, EmitterInstanceAtom()));
	if (res.second)
	{
		res.first->second.PrimitiveType = D3DPT_TRIANGLELIST;
		res.first->second.NumVertices = NumVertices;
		res.first->second.VertexStride = VertexStride;
		res.first->second.StartIndex = StartIndex;
		res.first->second.PrimitiveCount = PrimitiveCount;
	}
	else
	{
		_ASSERT(res.first->second.PrimitiveType == D3DPT_TRIANGLELIST);
		_ASSERT(res.first->second.NumVertices == NumVertices);
		_ASSERT(res.first->second.VertexStride = VertexStride);
		_ASSERT(res.first->second.StartIndex == StartIndex);
		_ASSERT(res.first->second.PrimitiveCount == PrimitiveCount);
		_ASSERT(!res.first->second.cmps.empty());
	}
	res.first->second.cmps.push_back(boost::make_tuple(cmp, particles, particle_num));
}
