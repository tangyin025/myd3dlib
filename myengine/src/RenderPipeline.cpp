#include <sstream>
#include "RenderPipeline.h"
#include "RenderPipeline.inl"
#include "myResource.h"
#include "myDxutApp.h"
#include "myEmitter.h"
#include "Component.h"
#include "Actor.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

using namespace my;

const UINT RenderPipeline::m_ParticlePrimitiveInfo[ParticlePrimitiveTypeCount][4] = {
	{0,3,0,1},
	{3,4,3,2},
};

RenderPipeline::RenderPipeline(void)
	: SHADOW_MAP_SIZE(1024)
	, SHADOW_EPSILON(0.001f)
	, m_ShadowRT(new Texture2D())
	, m_ShadowDS(new Surface())
	, m_SkyLightCam(sqrt(30 * 30 * 2.0f), 1.0f, -100, 100)
	, m_SkyLightColor(1.0f, 1.0f, 1.0f, 1.0f)
	, m_AmbientColor(0.3f, 0.3f, 0.3f, 3.0f)
	, handle_Time(NULL)
	, handle_ScreenDim(NULL)
	, handle_ShadowMapSize(NULL)
	, handle_ShadowEpsilon(NULL)
	, handle_World(NULL)
	, handle_Eye(NULL)
	, handle_View(NULL)
	, handle_ViewProj(NULL)
	, handle_InvViewProj(NULL)
	, handle_SkyLightView(NULL)
	, handle_SkyLightViewProj(NULL)
	, handle_SkyLightColor(NULL)
	, handle_AmbientColor(NULL)
	, handle_ShadowRT(NULL)
	, handle_NormalRT(NULL)
	, handle_PositionRT(NULL)
	, handle_LightRT(NULL)
	, handle_OpaqueRT(NULL)
	, handle_DownFilterRT(NULL)
	, m_DofParams(5.0f, 15.0f, 25.0f, 1.0f)
	, handle_DofParams(NULL)
	, handle_InputTexture(NULL)
	, handle_RCPFrame(NULL)
	, handle_bias(NULL)
	, handle_intensity(NULL)
	, handle_sample_rad(NULL)
	, handle_scale(NULL)
	, m_SsaoBias(0.2f)
	, m_SsaoIntensity(5.0f)
	, m_SsaoRadius(100.0f)
	, m_SsaoScale(10.0f)
	, m_FogColor(1.0f, 1.0f, 1.0f, 1.0f)
	, m_FogStartDistance(10)
	, m_FogHeight(50)
	, m_FogFalloff(0.01f)
{
}

RenderPipeline::~RenderPipeline(void)
{
}

static size_t _hash_value(RenderPipeline::MeshType mesh_type, const D3DXMACRO* pDefines, const char * name)
{
	// ! maybe hash conflict
	size_t seed = 0;
	boost::hash_combine(seed, mesh_type);
	boost::hash_combine(seed, std::string(name));
	if (pDefines)
	{
		const D3DXMACRO* macro_iter = pDefines;
		for (; macro_iter->Name; macro_iter++)
		{
			boost::hash_combine(seed, std::string(macro_iter->Name));
			if (macro_iter->Definition)
			{
				boost::hash_combine(seed, std::string(macro_iter->Definition));
			}
		}
	}
	return seed;
}

my::Effect * RenderPipeline::QueryShader(MeshType mesh_type, const D3DXMACRO* pDefines, const char * path, unsigned int PassID)
{
	const char * name = PathFindFileNameA(path);
	size_t seed = _hash_value(mesh_type, pDefines, name);
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(seed);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	std::ostringstream logoss;
	logoss << "Build Shader: ";
	switch (mesh_type)
	{
	case RenderPipeline::MeshTypeMesh:
		logoss << "MeshMesh";
		break;
	case RenderPipeline::MeshTypeParticle:
		logoss << "MeshParticle";
		break;
	case RenderPipeline::MeshTypeTerrain:
		logoss << "MeshTerrain";
		break;
	default:
		logoss << "MeshUnknown";
		break;
	}
	if (pDefines)
	{
		const D3DXMACRO* macro_iter = pDefines;
		for (; macro_iter->Name; macro_iter++)
		{
			logoss << "_" << macro_iter->Name;
			if (macro_iter->Definition)
			{
				logoss << "_" << macro_iter->Definition;
			}
		}
	}
	logoss << "_" << path;
	my::D3DContext::getSingleton().m_EventLog(logoss.str().c_str());

	std::ostringstream oss;
	oss << "#define SHADOW_MAP_SIZE " << SHADOW_MAP_SIZE << std::endl;
	oss << "#define SHADOW_EPSILON " << SHADOW_EPSILON << std::endl;
	oss << "#include \"CommonHeader.fx\"" << std::endl;
	oss << "#include \"";
	switch (mesh_type)
	{
	case RenderPipeline::MeshTypeMesh:
		oss << "MeshMesh.fx";
		break;
	case RenderPipeline::MeshTypeParticle:
		oss << "MeshParticle.fx";
		break;
	case RenderPipeline::MeshTypeTerrain:
		oss << "MeshTerrain.fx";
		break;
	default:
		oss << "MeshUnknown.fx";
		break;
	}
	oss << "\"" << std::endl;
	oss << "#include \"" << name << "\"" << std::endl;
	std::string source = oss.str();

	my::ResourceMgr::getSingleton().m_EffectInclude = my::ZipIStreamDir::ReplaceSlash(path);
	PathRemoveFileSpecA(&my::ResourceMgr::getSingleton().m_EffectInclude[0]);

	CComPtr<ID3DXBuffer> err;
	CComPtr<ID3DXEffectCompiler> compiler;
	if (FAILED(D3DXCreateEffectCompiler(source.c_str(), (UINT)source.length(), pDefines, my::ResourceMgr::getSingletonPtr(), D3DXSHADER_PACKMATRIX_COLUMNMAJOR | D3DXFX_LARGEADDRESSAWARE, &compiler, &err)))
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
	std::ofstream ofs(BuffPath, std::ios::binary);
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
	case PassTypeShadow: return "PassTypeShadow";
	case PassTypeNormal: return "PassTypeNormal";
	case PassTypeLight: return "PassTypeLight";
	case PassTypeBackground: return "PassTypeBackground";
	case PassTypeOpaque: return "PassTypeOpaque";
	case PassTypeTransparent: return "PassTypeTransparent";
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

	offset = 0;
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 1);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 2);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 3);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 4);
	offset += sizeof(Vector4);

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

	m_MeshIEList = m_MeshInstanceElems.BuildVertexElementList(1);

	if (!(m_SimpleSample = my::ResourceMgr::getSingleton().LoadEffect("shader/SimpleSample.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SimpleSample failed");
	}

	BOOST_VERIFY(handle_Time = m_SimpleSample->GetParameterByName(NULL, "g_Time"));
	BOOST_VERIFY(handle_ScreenDim = m_SimpleSample->GetParameterByName(NULL, "g_ScreenDim"));
	BOOST_VERIFY(handle_ShadowMapSize = m_SimpleSample->GetParameterByName(NULL, "g_ShadowMapSize"));
	BOOST_VERIFY(handle_ShadowEpsilon = m_SimpleSample->GetParameterByName(NULL, "g_ShadowEpsilon"));
	BOOST_VERIFY(handle_World = m_SimpleSample->GetParameterByName(NULL, "g_World"));
	BOOST_VERIFY(handle_Eye = m_SimpleSample->GetParameterByName(NULL, "g_Eye"));
	BOOST_VERIFY(handle_View = m_SimpleSample->GetParameterByName(NULL, "g_View"));
	BOOST_VERIFY(handle_ViewProj = m_SimpleSample->GetParameterByName(NULL, "g_ViewProj"));
	BOOST_VERIFY(handle_InvViewProj = m_SimpleSample->GetParameterByName(NULL, "g_InvViewProj"));
	BOOST_VERIFY(handle_SkyLightView = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightView"));
	BOOST_VERIFY(handle_SkyLightViewProj = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightViewProj"));
	BOOST_VERIFY(handle_SkyLightColor = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightColor"));
	BOOST_VERIFY(handle_AmbientColor = m_SimpleSample->GetParameterByName(NULL, "g_AmbientColor"));
	BOOST_VERIFY(handle_ShadowRT = m_SimpleSample->GetParameterByName(NULL, "g_ShadowRT"));
	BOOST_VERIFY(handle_NormalRT = m_SimpleSample->GetParameterByName(NULL, "g_NormalRT"));
	BOOST_VERIFY(handle_PositionRT = m_SimpleSample->GetParameterByName(NULL, "g_PositionRT"));
	BOOST_VERIFY(handle_LightRT = m_SimpleSample->GetParameterByName(NULL, "g_LightRT"));
	BOOST_VERIFY(handle_OpaqueRT = m_SimpleSample->GetParameterByName(NULL, "g_OpaqueRT"));
	BOOST_VERIFY(handle_DownFilterRT = m_SimpleSample->GetParameterByName(NULL, "g_DownFilterRT"));

	if (!(m_DofEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/DofEffect.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_DofEffect failed");
	}

	BOOST_VERIFY(handle_DofParams = m_DofEffect->GetParameterByName(NULL, "g_DofParams"));

	if (!(m_FxaaEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/FXAA.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_FxaaEffect failed");
	}

	BOOST_VERIFY(handle_InputTexture = m_FxaaEffect->GetParameterByName(NULL, "InputTexture"));
	BOOST_VERIFY(handle_RCPFrame = m_FxaaEffect->GetParameterByName(NULL, "RCPFrame"));

	if (!(m_SsaoEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/SSAO.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SsaoEffect failed");
	}

	BOOST_VERIFY(handle_bias = m_SsaoEffect->GetParameterByName(NULL, "g_bias"));
	BOOST_VERIFY(handle_intensity = m_SsaoEffect->GetParameterByName(NULL, "g_intensity"));
	BOOST_VERIFY(handle_sample_rad = m_SsaoEffect->GetParameterByName(NULL, "g_sample_rad"));
	BOOST_VERIFY(handle_scale = m_SsaoEffect->GetParameterByName(NULL, "g_scale"));

	if (!(m_FogEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/HeightFog.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_FogEffect failed");
	}

	BOOST_VERIFY(handle_FogColor = m_FogEffect->GetParameterByName(NULL, "g_FogColor"));
	BOOST_VERIFY(handle_FogStartDistance = m_FogEffect->GetParameterByName(NULL, "g_StartDistance"));
	BOOST_VERIFY(handle_FogHeight = m_FogEffect->GetParameterByName(NULL, "g_FogHeight"));
	BOOST_VERIFY(handle_FogFalloff = m_FogEffect->GetParameterByName(NULL, "g_Falloff"));
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
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 1, Vector3(-0.5f, 0.0f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 1, Vector2(0.0f, 1.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 2, Vector3(0.5f, 0.0f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 2, Vector2(1.0f, 1.0f));

	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 3, Vector3(-0.5f, 0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 3, Vector2(0.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 4, Vector3(-0.5f, -0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 4, Vector2(0.0f, 1.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 5, Vector3(0.5f, 0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 5, Vector2(1.0f, 0.0f));
	m_ParticleVertElems.SetPosition(pVertices + m_ParticleVertStride * 6, Vector3(0.5f, -0.5f, 0.0f));
	m_ParticleVertElems.SetTexcoord(pVertices + m_ParticleVertStride * 6, Vector2(1.0f, 1.0f));
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

	m_ShadowRT->CreateAdjustedTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! 所有的 render target必须使用具有相同 multisample的 depth stencil
	m_ShadowDS->CreateDepthStencilSurface(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ParticleIEDecl.Release();
	m_ParticleVb.OnDestroyDevice();
	m_ParticleIb.OnDestroyDevice();
	m_ParticleInstanceData.OnDestroyDevice();
	m_MeshInstanceData.OnDestroyDevice();
	m_ShadowRT->OnDestroyDevice();
	m_ShadowDS->OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
	_ASSERT(!m_ParticleVb.m_ptr);

	_ASSERT(!m_ParticleIb.m_ptr);

	_ASSERT(!m_ParticleInstanceData.m_ptr);

	_ASSERT(!m_MeshInstanceData.m_ptr);

	m_ShaderCache.clear();

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
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	IRenderContext * pRC,
	double fTime,
	float fElapsedTime)
{
	QuadVertex quad[4];
	UpdateQuad(quad, Vector2((float)pBackBufferSurfaceDesc->Width, (float)pBackBufferSurfaceDesc->Height));

	QuadVertex quad_quat[4];
	UpdateQuad(quad_quat, Vector2((float)pBackBufferSurfaceDesc->Width, (float)pBackBufferSurfaceDesc->Height) / 4.0f);

	HRESULT hr;
	CComPtr<IDirect3DSurface9> ScreenSurf;
	CComPtr<IDirect3DSurface9> ScreenDepthStencilSurf;
	V(pd3dDevice->GetRenderTarget(0, &ScreenSurf));
	V(pd3dDevice->GetDepthStencilSurface(&ScreenDepthStencilSurf));

	// ! Ogre & Apex模型都是顺时针，右手系应该是逆时针
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(m_SkyLightCam.m_ViewProj), this, PassTypeToMask(PassTypeShadow));

	CComPtr<IDirect3DSurface9> ShadowSurf = m_ShadowRT->GetSurfaceLevel(0);
	m_SimpleSample->SetFloat(handle_Time, my::D3DContext::getSingleton().m_fTotalTime);
	m_SimpleSample->SetVector(handle_ScreenDim, Vector2((float)pBackBufferSurfaceDesc->Width, (float)pBackBufferSurfaceDesc->Height));
	m_SimpleSample->SetFloat(handle_ShadowMapSize, (float)SHADOW_MAP_SIZE);
	m_SimpleSample->SetFloat(handle_ShadowEpsilon, SHADOW_EPSILON);
	m_SimpleSample->SetMatrix(handle_World, Matrix4::identity);
	m_SimpleSample->SetVector(handle_Eye, pRC->m_Camera->m_Eye);
	m_SimpleSample->SetMatrix(handle_View, pRC->m_Camera->m_View);
	m_SimpleSample->SetMatrix(handle_ViewProj, pRC->m_Camera->m_ViewProj);
	m_SimpleSample->SetMatrix(handle_InvViewProj, pRC->m_Camera->m_InverseViewProj);
	m_SimpleSample->SetMatrix(handle_SkyLightView, m_SkyLightCam.m_View); // ! RH -z
	m_SimpleSample->SetMatrix(handle_SkyLightViewProj, m_SkyLightCam.m_ViewProj);
	V(pd3dDevice->SetRenderTarget(0, ShadowSurf));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowDS->m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeShadow, pd3dDevice, fTime, fElapsedTime);
	ShadowSurf.Release();

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_Camera->m_ViewProj), this, PassTypeToMask(PassTypeNormal) | PassTypeToMask(PassTypeLight) | PassTypeToMask(PassTypeBackground) | PassTypeToMask(PassTypeOpaque) | PassTypeToMask(PassTypeTransparent));

	CComPtr<IDirect3DSurface9> NormalSurf = pRC->m_NormalRT->GetSurfaceLevel(0);
	CComPtr<IDirect3DSurface9> PositionSurf = pRC->m_PositionRT->GetSurfaceLevel(0);
	m_SimpleSample->SetVector(handle_SkyLightColor, m_SkyLightColor);
	m_SimpleSample->SetVector(handle_AmbientColor, m_AmbientColor);
	m_SimpleSample->SetTexture(handle_ShadowRT, m_ShadowRT.get());
	V(pd3dDevice->SetRenderTarget(0, NormalSurf));
	V(pd3dDevice->SetRenderTarget(1, PositionSurf));
	V(pd3dDevice->SetDepthStencilSurface(ScreenDepthStencilSurf));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeNormal, pd3dDevice, fTime, fElapsedTime);
	NormalSurf.Release();
	PositionSurf.Release();

	m_SimpleSample->SetTexture(handle_NormalRT, pRC->m_NormalRT.get());
	m_SimpleSample->SetTexture(handle_PositionRT, pRC->m_PositionRT.get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_LightRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetRenderTarget(1, NULL));
	if (pRC->m_SsaoEnable)
	{
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		m_SsaoEffect->SetFloat(handle_bias, m_SsaoBias);
		m_SsaoEffect->SetFloat(handle_intensity, m_SsaoIntensity);
		m_SsaoEffect->SetFloat(handle_sample_rad, m_SsaoRadius);
		m_SsaoEffect->SetFloat(handle_scale, m_SsaoScale);
		m_SsaoEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
		m_SsaoEffect->BeginPass(0);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
		m_SsaoEffect->EndPass();
		m_SsaoEffect->End();
	}
	else
	{
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_COLORVALUE(
			m_AmbientColor.x, m_AmbientColor.y, m_AmbientColor.z, 0), 0, 0));
	}
	RenderAllObjects(PassTypeLight, pd3dDevice, fTime, fElapsedTime);

	m_SimpleSample->SetTexture(handle_LightRT, pRC->m_LightRT.get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
	RenderAllObjects(PassTypeBackground, pd3dDevice, fTime, fElapsedTime);
	if (m_PassDrawCall[PassTypeBackground] <= 0)
	{
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 66, 75, 121), 1.0f, 0)); // ! d3dmultisample will not work
	}

	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, !pRC->m_WireFrame ? D3DFILL_SOLID: D3DFILL_WIREFRAME));

	RenderAllObjects(PassTypeOpaque, pd3dDevice, fTime, fElapsedTime);

	RenderAllObjects(PassTypeTransparent, pd3dDevice, fTime, fElapsedTime);

	if (pRC->m_FogEnable)
	{
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
		V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
		m_FogEffect->SetVector(handle_FogColor, m_FogColor);
		m_FogEffect->SetFloat(handle_FogStartDistance, m_FogStartDistance);
		m_FogEffect->SetFloat(handle_FogHeight, m_FogHeight);
		m_FogEffect->SetFloat(handle_FogFalloff, m_FogFalloff);
		m_FogEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
		m_FogEffect->BeginPass(0);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
		m_FogEffect->EndPass();
		m_FogEffect->End();
	}

	pRC->m_OpaqueRT.Flip();

	if (pRC->m_DofEnable)
	{
		V(pd3dDevice->StretchRect(pRC->m_OpaqueRT.GetNextSource()->GetSurfaceLevel(0), NULL,
			pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0), NULL, D3DTEXF_NONE)); // ! d3dref only support D3DTEXF_NONE
		pRC->m_DownFilterRT.Flip();

		m_DofEffect->SetVector(handle_DofParams, m_DofParams);
		m_SimpleSample->SetTexture(handle_OpaqueRT, pRC->m_OpaqueRT.GetNextSource().get());
		m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
		V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		UINT passes = m_DofEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
		m_DofEffect->BeginPass(0);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
		m_DofEffect->EndPass();
		pRC->m_DownFilterRT.Flip();

		m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
		V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
		m_DofEffect->BeginPass(1);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
		m_DofEffect->EndPass();
		pRC->m_DownFilterRT.Flip();

		m_SimpleSample->SetTexture(handle_DownFilterRT, pRC->m_DownFilterRT.GetNextSource().get());
		V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
		m_DofEffect->BeginPass(2);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
		m_DofEffect->EndPass();
		m_DofEffect->End();
		pRC->m_OpaqueRT.Flip();
	}

	if (pRC->m_FxaaEnable)
	{
		V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		m_FxaaEffect->SetTexture(handle_InputTexture, pRC->m_OpaqueRT.GetNextSource().get());
		Vector4 RCPFrame(1.0f / pBackBufferSurfaceDesc->Width, 1.0f / pBackBufferSurfaceDesc->Height, 0.0f, 0.0f);
		m_FxaaEffect->SetFloatArray(handle_RCPFrame, &RCPFrame.x, sizeof(RCPFrame) / sizeof(float));
		m_FxaaEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
		m_FxaaEffect->BeginPass(0);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
		m_FxaaEffect->EndPass();
		m_FxaaEffect->End();
		pRC->m_OpaqueRT.Flip();
	}

	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
	V(pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	V(pd3dDevice->SetRenderTarget(0, ScreenSurf));
	V(pd3dDevice->SetVertexShader(NULL));
	V(pd3dDevice->SetPixelShader(NULL));
	V(pd3dDevice->SetTexture(0, pRC->m_OpaqueRT.GetNextSource()->m_ptr));
	V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));

	ClearAllObjects();
}

void RenderPipeline::RenderAllObjects(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	m_PassDrawCall[PassID] = 0;

	IndexedPrimitiveAtomList::iterator prim_iter = m_Pass[PassID].m_IndexedPrimitiveList.begin();
	for (; prim_iter != m_Pass[PassID].m_IndexedPrimitiveList.end(); prim_iter++)
	{
		DrawIndexedPrimitive(
			PassID,
			pd3dDevice,
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
			PassID,
			pd3dDevice,
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
			PassID,
			pd3dDevice,
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
		DrawMesh(PassID, pd3dDevice, mesh_iter->mesh, mesh_iter->AttribId, mesh_iter->shader, mesh_iter->cmp, mesh_iter->mtl, mesh_iter->lparam);
		m_PassDrawCall[PassID]++;
	}

	MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
	for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
	{
		if (!mesh_inst_iter->second.cmps.empty())
		{
			DWORD AttribId = mesh_inst_iter->first.get<1>();
			_ASSERT(AttribId < mesh_inst_iter->second.m_AttribTable.size());
			const UINT NumInstances = (UINT)mesh_inst_iter->second.cmps.size();
			_ASSERT(NumInstances <= MESH_INSTANCE_MAX);

			Matrix4* trans = (Matrix4*)m_MeshInstanceData.Lock(0, NumInstances * m_MeshInstanceStride, D3DLOCK_DISCARD);
			for (DWORD i = 0; i < mesh_inst_iter->second.cmps.size(); i++)
			{
				trans[i] = mesh_inst_iter->second.cmps[i]->m_Actor->m_World;
			}
			m_MeshInstanceData.Unlock();

			my::Mesh* mesh = mesh_inst_iter->first.get<0>();
			DrawIndexedPrimitiveInstance(
				PassID,
				pd3dDevice,
				mesh_inst_iter->second.m_Decl,
				mesh->GetVertexBuffer(),
				mesh->GetIndexBuffer(),
				m_MeshInstanceData.m_ptr,
				D3DPT_TRIANGLELIST,
				0, mesh_inst_iter->second.m_AttribTable[AttribId].VertexStart,
				mesh_inst_iter->second.m_AttribTable[AttribId].VertexCount,
				mesh_inst_iter->second.m_VertexStride,
				mesh_inst_iter->second.m_AttribTable[AttribId].FaceStart * 3,
				mesh_inst_iter->second.m_AttribTable[AttribId].FaceCount,
				NumInstances,
				m_MeshInstanceStride,
				mesh_inst_iter->first.get<2>(),
				mesh_inst_iter->second.cmps.front(),
				mesh_inst_iter->first.get<3>(),
				mesh_inst_iter->first.get<4>());
			m_PassDrawCall[PassID]++;
		}
	}

	EmitterInstanceAtomMap::iterator emitter_inst_iter = m_Pass[PassID].m_EmitterInstanceMap.begin();
	for (; emitter_inst_iter != m_Pass[PassID].m_EmitterInstanceMap.end(); emitter_inst_iter++)
	{
		if (!emitter_inst_iter->second.cmps.empty())
		{
			_ASSERT(m_ParticleInstanceStride == sizeof(Emitter::ParticleList::value_type));
			DWORD NumTotalInstances = 0;
			unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, 0, D3DLOCK_DISCARD);
			std::vector<boost::tuple<Component*, my::Emitter::Particle*, unsigned int> >::const_iterator cmp_iter = emitter_inst_iter->second.cmps.begin();
			for (; cmp_iter != emitter_inst_iter->second.cmps.end(); cmp_iter++)
			{
				int Count = Min<int>(PARTICLE_INSTANCE_MAX - NumTotalInstances, cmp_iter->get<2>());
				memcpy(pVertices + NumTotalInstances * sizeof(Emitter::Particle), cmp_iter->get<1>(), Count * sizeof(Emitter::Particle));
				NumTotalInstances += Count;
			}
			m_ParticleInstanceData.Unlock();

			if (NumTotalInstances > 0)
			{
				DrawIndexedPrimitiveInstance(
					PassID,
					pd3dDevice,
					m_ParticleIEDecl,
					emitter_inst_iter->first.get<0>(),
					emitter_inst_iter->first.get<1>(),
					m_ParticleInstanceData.m_ptr,
					emitter_inst_iter->second.PrimitiveType,
					0, emitter_inst_iter->first.get<2>(),
					emitter_inst_iter->second.NumVertices,
					m_ParticleVertStride,
					emitter_inst_iter->second.StartIndex,
					emitter_inst_iter->second.PrimitiveCount,
					NumTotalInstances,
					m_ParticleInstanceStride,
					emitter_inst_iter->first.get<4>(),
					emitter_inst_iter->second.cmps.front().get<0>(),
					emitter_inst_iter->first.get<5>(),
					emitter_inst_iter->first.get<6>());
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
		m_Pass[PassID].m_EmitterInstanceMap.clear();
	}
}

void RenderPipeline::DrawIndexedPrimitive(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
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
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, pVB, 0, VertexStride));
		V(pd3dDevice->SetVertexDeclaration(pDecl));
		V(pd3dDevice->SetIndices(pIB));
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		V(pd3dDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawIndexedPrimitiveInstance(
	unsigned int PassID,
	IDirect3DDevice9* pd3dDevice,
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
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, pVB, 0, VertexStride));
		V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
		V(pd3dDevice->SetStreamSource(1, pInstance, 0, InstanceStride));
		V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
		V(pd3dDevice->SetVertexDeclaration(pDecl));
		V(pd3dDevice->SetIndices(pIB));
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		V(pd3dDevice->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount));
		V(pd3dDevice->SetStreamSourceFreq(0, 1));
		V(pd3dDevice->SetStreamSourceFreq(1, 1));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawIndexedPrimitiveUP(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
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
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetVertexDeclaration(pDecl));
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		V(pd3dDevice->DrawIndexedPrimitiveUP(
			PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawMesh(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		mesh->DrawSubset(AttribId);
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

void RenderPipeline::PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
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

void RenderPipeline::PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	MeshInstanceAtomKey key(mesh, AttribId, shader, mtl, lparam);
	std::pair<MeshInstanceAtomMap::iterator, bool> res = m_Pass[PassID].m_MeshInstanceMap.insert(std::make_pair(key, MeshInstanceAtom()));
	if (res.second)
	{
		DWORD submeshes = 0;
		mesh->GetAttributeTable(NULL, &submeshes);
		res.first->second.m_AttribTable.resize(submeshes);
		mesh->GetAttributeTable(&res.first->second.m_AttribTable[0], &submeshes);

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
	res.first->second.cmps.push_back(cmp);
}

void RenderPipeline::PushEmitter(
	unsigned int PassID,
	IDirect3DVertexBuffer9* pVB,
	IDirect3DIndexBuffer9* pIB,
	UINT MinVertexIndex,
	UINT NumVertices,
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

	EmitterInstanceAtomKey key(pVB, pIB, MinVertexIndex,
		dynamic_cast<EmitterComponent *>(cmp)->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld ? &Matrix4::identity : &cmp->m_Actor->m_World, shader, mtl, lparam);
	std::pair<EmitterInstanceAtomMap::iterator, bool> res = m_Pass[PassID].m_EmitterInstanceMap.insert(std::make_pair(key, EmitterInstanceAtom()));
	if (res.second)
	{
		res.first->second.PrimitiveType = D3DPT_TRIANGLELIST;
		res.first->second.NumVertices = NumVertices;
		res.first->second.StartIndex = StartIndex;
		res.first->second.PrimitiveCount = PrimitiveCount;
	}
	else
	{
		_ASSERT(res.first->second.PrimitiveType == D3DPT_TRIANGLELIST);
		_ASSERT(res.first->second.NumVertices == NumVertices);
		_ASSERT(res.first->second.StartIndex == StartIndex);
		_ASSERT(res.first->second.PrimitiveCount == PrimitiveCount);
		_ASSERT(!res.first->second.cmps.empty());
	}
	res.first->second.cmps.push_back(boost::make_tuple(cmp, particles, particle_num));
}
