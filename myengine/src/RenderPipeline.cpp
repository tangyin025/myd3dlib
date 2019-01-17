#include <sstream>
#include "RenderPipeline.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "myEmitter.h"
#include "Component.h"
#include "Actor.h"
#include "libc.h"
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

RenderPipeline::RenderPipeline(void)
	: m_ParticleVertexStride(0)
	, m_ParticleInstanceStride(0)
	, m_MeshInstanceStride(0)
	, SHADOW_MAP_SIZE(1024)
	, SHADOW_EPSILON(0.001f)
	, m_ShadowRT(new Texture2D())
	, m_ShadowDS(new Surface())
	, m_BgColor(0.7f, 0.7f, 0.7f, 1.0f)
	, m_SkyLightDiffuse(1.0f, 1.0f, 1.0f, 1.0f)
	, m_SkyLightAmbient(0.3f, 0.3f, 0.3f, 0.0f)
	, m_SkyBoxEnable(false)
	, handle_Time(NULL)
	, handle_Eye(NULL)
	, handle_ScreenDim(NULL)
	, handle_World(NULL)
	, handle_View(NULL)
	, handle_ViewProj(NULL)
	, handle_InvViewProj(NULL)
	, handle_SkyLightView(NULL)
	, handle_SkyLightViewProj(NULL)
	, handle_SkyLightDiffuse(NULL)
	, handle_SkyLightAmbient(NULL)
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
{
}

RenderPipeline::~RenderPipeline(void)
{
}

my::Effect * RenderPipeline::QueryShader(MeshType mesh_type, const char * macros, const char * path, unsigned int PassID)
{
	const char * name = PathFindFileNameA(path);
	char key[MAX_PATH];
	sprintf_s(key, sizeof(key), "%s_%u_%s", name, mesh_type, macros ? macros : "0");
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(key);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	std::vector<std::string> strmacros;
	std::vector<D3DXMACRO> d3dmacros;
	if (macros)
	{
		boost::algorithm::split(strmacros, macros, boost::algorithm::is_any_of(" _\t"), boost::algorithm::token_compress_on);
		std::vector<std::string>::const_iterator macro_iter = strmacros.begin();
		for (; macro_iter != strmacros.end(); macro_iter++)
		{
			D3DXMACRO d3dmacro;
			d3dmacro.Name = macro_iter->c_str();
			d3dmacro.Definition = NULL;
			d3dmacros.push_back(d3dmacro);
		}
	}
	D3DXMACRO end = { 0 };
	d3dmacros.push_back(end);

	struct Helper
	{
		static const char * vs_header(unsigned int mesh_type)
		{
			switch (mesh_type)
			{
			case RenderPipeline::MeshTypeAnimation:
				return "MeshSkeleton.fx";
			case RenderPipeline::MeshTypeParticle:
				return "MeshParticle.fx";
			case RenderPipeline::MeshTypeTerrain:
				return "MeshTerrain.fx";
			}
			return "MeshStatic.fx";
		}
	};

	std::ostringstream oss;
	oss << "#define SHADOW_MAP_SIZE " << SHADOW_MAP_SIZE << std::endl;
	oss << "#define SHADOW_EPSILON " << SHADOW_EPSILON << std::endl;
	oss << "#include \"CommonHeader.fx\"" << std::endl;
	oss << "#include \"" << Helper::vs_header(mesh_type) << "\"" << std::endl;
	oss << "#include \"" << name << "\"" << std::endl;
	std::string source = oss.str();

	my::ResourceMgr::getSingleton().m_EffectInclude = my::ZipIStreamDir::ReplaceSlash(path);
	PathRemoveFileSpecA(&my::ResourceMgr::getSingleton().m_EffectInclude[0]);

#ifdef _DEBUG
	CComPtr<ID3DXBuffer> buff;
	if (SUCCEEDED(D3DXPreprocessShader(source.c_str(), source.length(), &d3dmacros[0], my::ResourceMgr::getSingletonPtr(), &buff, NULL)))
	{
		my::OStreamPtr ostr = my::FileOStream::Open(ms2ts(key).c_str());
		ostr->write(buff->GetBufferPointer(), buff->GetBufferSize()-1);
	}
#endif

	my::EffectPtr shader(new my::Effect());
	try
	{
		shader->CreateEffect(source.c_str(), source.size(), &d3dmacros[0], my::ResourceMgr::getSingletonPtr(), 0, my::ResourceMgr::getSingleton().m_EffectPool);
	}
	catch (const my::Exception & e)
	{
		shader.reset();
		const std::string & what = e.what();
		my::D3DContext::getSingleton().m_EventLog(what.c_str());
	}
	m_ShaderCache.insert(std::make_pair(key, shader));
	return shader.get();
}

const char * RenderPipeline::PassTypeToStr(unsigned int pass_type)
{
	switch (pass_type)
	{
	case PassTypeShadow: return "PassTypeShadow";
	case PassTypeNormal: return "PassTypeNormal";
	case PassTypeLight: return "PassTypeLight";
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

template<>
void RenderPipeline::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_BgColor);
	ar << BOOST_SERIALIZATION_NVP(m_SkyLightDiffuse);
	ar << BOOST_SERIALIZATION_NVP(m_SkyLightAmbient);
	ar << BOOST_SERIALIZATION_NVP(m_SkyBoxEnable);
	ar << BOOST_SERIALIZATION_NVP(m_SkyBoxTextures);
	ar << BOOST_SERIALIZATION_NVP(m_DofParams);
	ar << BOOST_SERIALIZATION_NVP(m_SsaoBias);
	ar << BOOST_SERIALIZATION_NVP(m_SsaoIntensity);
	ar << BOOST_SERIALIZATION_NVP(m_SsaoRadius);
	ar << BOOST_SERIALIZATION_NVP(m_SsaoScale);
}

template<>
void RenderPipeline::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_BgColor);
	ar >> BOOST_SERIALIZATION_NVP(m_SkyLightDiffuse);
	ar >> BOOST_SERIALIZATION_NVP(m_SkyLightAmbient);
	ar >> BOOST_SERIALIZATION_NVP(m_SkyBoxEnable);
	ar >> BOOST_SERIALIZATION_NVP(m_SkyBoxTextures);
	ar >> BOOST_SERIALIZATION_NVP(m_DofParams);
	ar >> BOOST_SERIALIZATION_NVP(m_SsaoBias);
	ar >> BOOST_SERIALIZATION_NVP(m_SsaoIntensity);
	ar >> BOOST_SERIALIZATION_NVP(m_SsaoRadius);
	ar >> BOOST_SERIALIZATION_NVP(m_SsaoScale);
}

void RenderPipeline::RequestResource(void)
{
	for (unsigned int i = 0; i < _countof(m_SkyBoxTextures); i++)
	{
		m_SkyBoxTextures[i].RequestResource();
	}
}

void RenderPipeline::ReleaseResource(void)
{
	for (unsigned int i = 0; i < _countof(m_SkyBoxTextures); i++)
	{
		m_SkyBoxTextures[i].ReleaseResource();
	}
}

HRESULT RenderPipeline::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_ParticleVertexElems.InsertTexcoordElement(0);

	m_ParticleInstanceElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_ParticleInstanceElems.InsertNormalElement(offset);
	offset += sizeof(Vector3);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 1);
	offset += sizeof(Vector4);
	m_ParticleInstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2);
	offset += sizeof(Vector4);

	m_ParticleVEList = m_ParticleVertexElems.BuildVertexElementList(0);
	std::vector<D3DVERTEXELEMENT9> IEList = m_ParticleInstanceElems.BuildVertexElementList(1);
	m_ParticleVEList.insert(m_ParticleVEList.end(), IEList.begin(), IEList.end());
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	m_ParticleVEList.push_back(ve_end);

	m_ParticleVertexStride = D3DXGetDeclVertexSize(&m_ParticleVEList[0], 0);
	m_ParticleInstanceStride = D3DXGetDeclVertexSize(&m_ParticleVEList[0], 1);

	HRESULT hr;
	if(FAILED(hr = pd3dDevice->CreateVertexDeclaration(&m_ParticleVEList[0], &m_ParticleDecl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

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
	m_MeshInstanceStride = offset;

	if (!(m_SimpleSample = my::ResourceMgr::getSingleton().LoadEffect("shader/SimpleSample.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SimpleSample failed");
	}

	BOOST_VERIFY(handle_Time = m_SimpleSample->GetParameterByName(NULL, "g_Time"));
	BOOST_VERIFY(handle_ScreenDim = m_SimpleSample->GetParameterByName(NULL, "g_ScreenDim"));
	BOOST_VERIFY(handle_World = m_SimpleSample->GetParameterByName(NULL, "g_World"));
	BOOST_VERIFY(handle_Eye = m_SimpleSample->GetParameterByName(NULL, "g_Eye"));
	BOOST_VERIFY(handle_View = m_SimpleSample->GetParameterByName(NULL, "g_View"));
	BOOST_VERIFY(handle_ViewProj = m_SimpleSample->GetParameterByName(NULL, "g_ViewProj"));
	BOOST_VERIFY(handle_InvViewProj = m_SimpleSample->GetParameterByName(NULL, "g_InvViewProj"));
	BOOST_VERIFY(handle_SkyLightView = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightView"));
	BOOST_VERIFY(handle_SkyLightViewProj = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightViewProj"));
	BOOST_VERIFY(handle_SkyLightDiffuse = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightDiffuse"));
	BOOST_VERIFY(handle_SkyLightAmbient = m_SimpleSample->GetParameterByName(NULL, "g_SkyLightAmbient"));
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
	return S_OK;
}

HRESULT RenderPipeline::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	m_ParticleVertexBuffer.CreateVertexBuffer(m_ParticleVertexStride * 4, 0, 0, D3DPOOL_DEFAULT);
	unsigned char * pVertices = (unsigned char *)m_ParticleVertexBuffer.Lock(0, m_ParticleVertexStride * 4);
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 0, Vector2(0,0));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 1, Vector2(0,1));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 2, Vector2(1,1));
	m_ParticleVertexElems.SetTexcoord(pVertices + m_ParticleVertexStride * 3, Vector2(1,0));
	m_ParticleVertexBuffer.Unlock();

	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	m_ParticleIndexBuffer.CreateIndexBuffer(sizeof(WORD) * 4, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT);
	WORD * pIndices = (WORD *)m_ParticleIndexBuffer.Lock(0, sizeof(WORD) * 4);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 3;
	m_ParticleIndexBuffer.Unlock();

	_ASSERT(!m_ParticleInstanceData.m_ptr);
	m_ParticleInstanceData.CreateVertexBuffer(m_ParticleInstanceStride * Emitter::PARTICLE_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	_ASSERT(!m_MeshInstanceData.m_ptr);
	m_MeshInstanceData.CreateVertexBuffer(m_MeshInstanceStride * MESH_INSTANCE_MAX, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT);

	m_ShadowRT->CreateAdjustedTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! 所有的 render target必须使用具有相同 multisample的 depth stencil
	m_ShadowDS->CreateDepthStencilSurface(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ParticleVertexBuffer.OnDestroyDevice();
	m_ParticleIndexBuffer.OnDestroyDevice();
	m_ParticleInstanceData.OnDestroyDevice();
	m_MeshInstanceData.OnDestroyDevice();
	m_ShadowRT->OnDestroyDevice();
	m_ShadowDS->OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
	_ASSERT(!m_ParticleVertexBuffer.m_ptr);
	_ASSERT(!m_ParticleIndexBuffer.m_ptr);
	_ASSERT(!m_ParticleInstanceData.m_ptr);
	_ASSERT(!m_MeshInstanceData.m_ptr);

	m_ShaderCache.clear();

	m_ParticleDecl.Release();

	ClearAllObjects();

	unsigned int PassID = 0;
	for (; PassID < m_Pass.size(); PassID++)
	{
		//MeshInstanceAtomMap::iterator mesh_inst_iter = m_MeshInstanceMap.begin();
		//for (; mesh_inst_iter != m_MeshInstanceMap.end(); mesh_inst_iter++)
		//{
		//	mesh_inst_iter->second.m_Decl.Release();
		//}
		m_Pass[PassID].m_MeshInstanceMap.clear();
	}
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
	V(pd3dDevice->SetRenderState(D3DRS_FILLMODE, pRC->m_WireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_SkyLightCam->m_ViewProj), this, PassTypeToMask(PassTypeShadow));

	CComPtr<IDirect3DSurface9> ShadowSurf = m_ShadowRT->GetSurfaceLevel(0);
	m_SimpleSample->SetFloat(handle_Time, my::D3DContext::getSingleton().m_fTotalTime);
	m_SimpleSample->SetVector(handle_Eye, pRC->m_Camera->m_Eye);
	m_SimpleSample->SetMatrix(handle_View, pRC->m_Camera->m_View);
	m_SimpleSample->SetMatrix(handle_ViewProj, pRC->m_Camera->m_ViewProj);
	m_SimpleSample->SetMatrix(handle_InvViewProj, pRC->m_Camera->m_InverseViewProj);
	m_SimpleSample->SetMatrix(handle_SkyLightView, pRC->m_SkyLightCam->m_View); // ! RH -z
	m_SimpleSample->SetMatrix(handle_SkyLightViewProj, pRC->m_SkyLightCam->m_ViewProj);
	V(pd3dDevice->SetRenderTarget(0, ShadowSurf));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowDS->m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeShadow, pd3dDevice, fTime, fElapsedTime);
	ShadowSurf.Release();

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_Camera->m_ViewProj), this, PassTypeToMask(PassTypeNormal) | PassTypeToMask(PassTypeLight) | PassTypeToMask(PassTypeOpaque) | PassTypeToMask(PassTypeTransparent));

	CComPtr<IDirect3DSurface9> NormalSurf = pRC->m_NormalRT->GetSurfaceLevel(0);
	CComPtr<IDirect3DSurface9> PositionSurf = pRC->m_PositionRT->GetSurfaceLevel(0);
	m_SimpleSample->SetVector(handle_SkyLightDiffuse, m_SkyLightDiffuse);
	m_SimpleSample->SetVector(handle_SkyLightAmbient, m_SkyLightAmbient);
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
			m_SkyLightAmbient.x, m_SkyLightAmbient.y, m_SkyLightAmbient.z, m_SkyLightAmbient.w), 0, 0));
	}
	RenderAllObjects(PassTypeLight, pd3dDevice, fTime, fElapsedTime);

	m_SimpleSample->SetTexture(handle_LightRT, pRC->m_LightRT.get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
	const D3DXCOLOR bgcolor = D3DCOLOR_COLORVALUE(m_BgColor.x, m_BgColor.y, m_BgColor.z, m_BgColor.w);
	if (m_SkyBoxEnable)
	{
		struct CUSTOMVERTEX
		{
			float x, y, z;
			D3DCOLOR color;
			FLOAT tu, tv;
		};
		CUSTOMVERTEX vertices[] =
		{
			{-1,  1, -1, bgcolor, 0, 0},
			{-1, -1, -1, bgcolor, 0, 1},
			{ 1, -1, -1, bgcolor, 1, 1},
			{ 1,  1, -1, bgcolor, 1, 0},
		};
		pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
		V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&pRC->m_Camera->m_View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&pRC->m_Camera->m_Proj));
		V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
		V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
		V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
		V(pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE));
		my::Matrix4 transforms[6] =
		{
			my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationY(D3DXToRadian(180)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationY(D3DXToRadian( 90)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationY(D3DXToRadian(270)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationX(D3DXToRadian( 90)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationX(D3DXToRadian(270)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
		};
		for (unsigned int i = 0; i < _countof(m_SkyBoxTextures); i++)
		{
			if (m_SkyBoxTextures[i].m_Texture)
			{
				V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&transforms[i]));
				V(pd3dDevice->SetTexture(0, m_SkyBoxTextures[i].m_Texture->m_ptr));
				V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &vertices, sizeof(vertices[0])));
			}
		}
	}
	else
	{
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, bgcolor, 1.0f, 0)); // ! d3dmultisample will not work
	}
	RenderAllObjects(PassTypeOpaque, pd3dDevice, fTime, fElapsedTime);

	RenderAllObjects(PassTypeTransparent, pd3dDevice, fTime, fElapsedTime);

	pRC->m_OpaqueRT.Flip();

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

	IndexedPrimitiveUPAtomList::iterator indexed_prim_iter = m_Pass[PassID].m_IndexedPrimitiveUPList.begin();
	for (; indexed_prim_iter != m_Pass[PassID].m_IndexedPrimitiveUPList.end(); indexed_prim_iter++)
	{
		DrawIndexedPrimitiveUP(
			PassID,
			pd3dDevice,
			indexed_prim_iter->pDecl,
			indexed_prim_iter->PrimitiveType,
			indexed_prim_iter->MinVertexIndex,
			indexed_prim_iter->NumVertices,
			indexed_prim_iter->PrimitiveCount,
			indexed_prim_iter->pIndexData,
			indexed_prim_iter->IndexDataFormat,
			indexed_prim_iter->pVertexStreamZeroData,
			indexed_prim_iter->VertexStreamZeroStride,
			indexed_prim_iter->shader,
			indexed_prim_iter->cmp,
			indexed_prim_iter->mtl,
			indexed_prim_iter->lparam);
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
			DrawMeshInstance(
				PassID,
				pd3dDevice,
				mesh_inst_iter->first.get<0>(),
				mesh_inst_iter->first.get<1>(),
				mesh_inst_iter->first.get<2>(),
				mesh_inst_iter->first.get<3>(),
				mesh_inst_iter->first.get<4>(),
				mesh_inst_iter->second);
			m_PassDrawCall[PassID]++;
		}
	}

	EmitterAtomList::iterator emitter_iter = m_Pass[PassID].m_EmitterList.begin();
	for (; emitter_iter != m_Pass[PassID].m_EmitterList.end(); emitter_iter++)
	{
		if (!emitter_iter->emitter->m_ParticleList.empty())
		{
			DrawEmitter(PassID, pd3dDevice, emitter_iter->emitter, emitter_iter->shader, emitter_iter->cmp, emitter_iter->mtl, emitter_iter->lparam);
			m_PassDrawCall[PassID]++;
		}
	}

	WorldEmitterAtomMap::iterator world_emitter_iter = m_Pass[PassID].m_WorldEmitterMap.begin();
	for (; world_emitter_iter != m_Pass[PassID].m_WorldEmitterMap.end(); world_emitter_iter++)
	{
		if (!world_emitter_iter->second.emitters.empty() && world_emitter_iter->second.TotalParticles > 0)
		{
			DrawWorldEmitter(
				PassID,
				pd3dDevice,
				world_emitter_iter->first.get<0>(),
				world_emitter_iter->first.get<1>(),
				world_emitter_iter->first.get<2>(),
				world_emitter_iter->second);
			m_PassDrawCall[PassID]++;
		}
	}
}

void RenderPipeline::ClearAllObjects(void)
{
	unsigned int PassID = 0;
	for (; PassID < m_Pass.size(); PassID++)
	{
		m_Pass[PassID].m_IndexedPrimitiveList.clear();
		m_Pass[PassID].m_IndexedPrimitiveUPList.clear();
		m_Pass[PassID].m_MeshList.clear();
		//MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
		//for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
		//{
		//	mesh_inst_iter->second.cmps.clear(); // ! mtl in hash key may invalid
		//}
		m_Pass[PassID].m_MeshInstanceMap.clear();
		m_Pass[PassID].m_EmitterList.clear();
		m_Pass[PassID].m_WorldEmitterMap.clear();
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

void RenderPipeline::DrawMeshInstance(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Material * mtl, LPARAM lparam, MeshInstanceAtom & atom)
{
	_ASSERT(AttribId < atom.m_AttribTable.size());
	const DWORD NumInstances = atom.cmps.size();
	_ASSERT(NumInstances <= MESH_INSTANCE_MAX);

	Matrix4 * trans = (Matrix4 *)m_MeshInstanceData.Lock(0, NumInstances * m_MeshInstanceStride, D3DLOCK_DISCARD);
	for (DWORD i = 0; i < atom.cmps.size(); i++)
	{
		trans[i] = atom.cmps[i]->m_Actor->m_World;
	}
	m_MeshInstanceData.Unlock();

	CComPtr<IDirect3DVertexBuffer9> vb = mesh->GetVertexBuffer();
	CComPtr<IDirect3DIndexBuffer9> ib = mesh->GetIndexBuffer();

	atom.cmps[0]->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, vb, 0, atom.m_VertexStride));
		V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
		V(pd3dDevice->SetStreamSource(1, m_MeshInstanceData.m_ptr, 0, m_MeshInstanceStride));
		V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
		V(pd3dDevice->SetVertexDeclaration(atom.m_Decl));
		V(pd3dDevice->SetIndices(ib));
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
			atom.m_AttribTable[AttribId].VertexStart,
			atom.m_AttribTable[AttribId].VertexCount,
			atom.m_AttribTable[AttribId].FaceStart * 3,
			atom.m_AttribTable[AttribId].FaceCount));
		V(pd3dDevice->SetStreamSourceFreq(0, 1));
		V(pd3dDevice->SetStreamSourceFreq(1, 1));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	const DWORD NumInstances = my::Min(emitter->m_ParticleList.size(), Emitter::PARTICLE_INSTANCE_MAX);
	_ASSERT(m_ParticleInstanceStride == sizeof(Emitter::ParticleList::value_type));
	unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, m_ParticleInstanceStride * NumInstances, D3DLOCK_DISCARD);
	_ASSERT(pVertices);
	int NumRemaining = NumInstances;
	Emitter::ParticleList::const_array_range array_one = emitter->m_ParticleList.array_one();
	int count_one = my::Min<int>(NumRemaining, array_one.second);
	if (count_one > 0)
	{
		size_t length = count_one * sizeof(Emitter::ParticleList::value_type);
		memcpy(pVertices, array_one.first, length);
		pVertices += length;
		NumRemaining -= count_one;
	}
	Emitter::ParticleList::const_array_range array_two = emitter->m_ParticleList.array_two();
	int count_two = my::Min<int>(NumRemaining, array_two.second);
	if (count_two > 0)
	{
		size_t length = count_two * sizeof(Emitter::ParticleList::value_type);
		memcpy(pVertices, array_two.first, length);
		pVertices += length;
		NumRemaining -= count_two;
	}
	m_ParticleInstanceData.Unlock();

	cmp->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, m_ParticleVertexBuffer.m_ptr, 0, m_ParticleVertexStride));
		V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
		V(pd3dDevice->SetStreamSource(1, m_ParticleInstanceData.m_ptr, 0, m_ParticleInstanceStride));
		V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
		V(pd3dDevice->SetVertexDeclaration(m_ParticleDecl));
		V(pd3dDevice->SetIndices(m_ParticleIndexBuffer.m_ptr));
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));
		V(pd3dDevice->SetStreamSourceFreq(0,1));
		V(pd3dDevice->SetStreamSourceFreq(1,1));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawWorldEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Effect * shader, Material * mtl, LPARAM lparam, WorldEmitterAtom & atom)
{
	const DWORD NumInstances = my::Min(atom.TotalParticles, (DWORD)Emitter::PARTICLE_INSTANCE_MAX);
	_ASSERT(m_ParticleInstanceStride == sizeof(Emitter::ParticleList::value_type));
	unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, m_ParticleInstanceStride * NumInstances, D3DLOCK_DISCARD);
	_ASSERT(pVertices);
	int NumRemaining = NumInstances;
	WorldEmitterAtom::EmitterPairList::const_iterator emitter_pair_iter = atom.emitters.begin();
	for (; NumRemaining > 0 && emitter_pair_iter != atom.emitters.end(); emitter_pair_iter++)
	{
		Emitter::ParticleList::const_array_range array_one = emitter_pair_iter->first->m_ParticleList.array_one();
		int count_one = my::Min(NumRemaining, (int)array_one.second);
		if (count_one > 0)
		{
			size_t length = count_one * sizeof(Emitter::ParticleList::value_type);
			memcpy(pVertices, array_one.first, length);
			pVertices += length;
			NumRemaining -= count_one;
		}
		Emitter::ParticleList::const_array_range array_two = emitter_pair_iter->first->m_ParticleList.array_two();
		int count_two = my::Min(NumRemaining, (int)array_two.second);
		if (count_two > 0)
		{
			size_t length = count_two * sizeof(Emitter::ParticleList::value_type);
			memcpy(pVertices, array_two.first, length);
			pVertices += length;
			NumRemaining -= count_two;
		}
	}
	m_ParticleInstanceData.Unlock();

	atom.emitters[0].second->OnSetShader(pd3dDevice, shader, lparam);
	const UINT passes = shader->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	_ASSERT(PassID < passes);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, m_ParticleVertexBuffer.m_ptr, 0, m_ParticleVertexStride));
		V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
		V(pd3dDevice->SetStreamSource(1, m_ParticleInstanceData.m_ptr, 0, m_ParticleInstanceStride));
		V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
		V(pd3dDevice->SetVertexDeclaration(m_ParticleDecl));
		V(pd3dDevice->SetIndices(m_ParticleIndexBuffer.m_ptr));
		mtl->OnSetShader(pd3dDevice, shader, lparam);
		shader->CommitChanges();
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));
		V(pd3dDevice->SetStreamSourceFreq(0, 1));
		V(pd3dDevice->SetStreamSourceFreq(1, 1));
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

bool RenderPipeline::MeshInstanceAtomKey::operator == (const MeshInstanceAtomKey & rhs) const
{
	return get<0>() == rhs.get<0>()
		&& get<1>() == rhs.get<1>()
		&& get<2>() == rhs.get<2>()
		&& *get<3>() == *rhs.get<3>(); // ! mtl ptr must be valid object
}

namespace boost
{
	static size_t hash_value(const MaterialParameter & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.m_Type);
		boost::hash_combine(seed, key.m_Name);
		switch (key.m_Type)
		{
		case MaterialParameter::ParameterTypeFloat:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat &>(key).m_Value);
			break;
		case MaterialParameter::ParameterTypeFloat2:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat2 &>(key).m_Value.x);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat2 &>(key).m_Value.y);
			break;
		case MaterialParameter::ParameterTypeFloat3:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat3 &>(key).m_Value.x);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat3 &>(key).m_Value.y);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat3 &>(key).m_Value.z);
			break;
		case MaterialParameter::ParameterTypeFloat4:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.x);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.y);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.z);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.w);
			break;
		case MaterialParameter::ParameterTypeTexture:
			boost::hash_combine(seed, static_cast<const MaterialParameterTexture &>(key).m_TexturePath);
			break;
		}
		return seed;
	}

	static size_t hash_value(const Material & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.m_PassMask);
		boost::hash_combine(seed, key.m_PassMask);
		boost::hash_combine(seed, key.m_CullMode);
		boost::hash_combine(seed, key.m_ZEnable);
		boost::hash_combine(seed, key.m_ZWriteEnable);
		boost::hash_combine(seed, key.m_BlendMode);
		for (unsigned int i = 0; i < key.m_ParameterList.size(); i++)
		{
			boost::hash_combine(seed, *key.m_ParameterList[i]);
		}
		return seed;
	}

	static size_t hash_value(const RenderPipeline::MeshInstanceAtomKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, key.get<1>());
		boost::hash_combine(seed, key.get<2>());
		boost::hash_combine(seed, *key.get<3>());
		return seed;
	}
}

void RenderPipeline::PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	MeshInstanceAtomKey key(mesh, AttribId, shader, mtl, lparam);
	MeshInstanceAtomMap::iterator atom_iter = m_Pass[PassID].m_MeshInstanceMap.find(key);
	if (atom_iter == m_Pass[PassID].m_MeshInstanceMap.end())
	{
		MeshInstanceAtom & atom = m_Pass[PassID].m_MeshInstanceMap[key];
		DWORD submeshes = 0;
		mesh->GetAttributeTable(NULL, &submeshes);
		atom.m_AttribTable.resize(submeshes);
		mesh->GetAttributeTable(&atom.m_AttribTable[0], &submeshes);

		atom.m_velist.resize(MAX_FVF_DECL_SIZE);
		mesh->GetDeclaration(&atom.m_velist[0]);
		unsigned int i = 0;
		for (; i < atom.m_velist.size(); i++)
		{
			if (atom.m_velist[i].Stream == 0xff || atom.m_velist[i].Type == D3DDECLTYPE_UNUSED)
			{
				break;
			}
		}
		if (i >= atom.m_velist.size())
		{
			THROW_CUSEXCEPTION("invalid vertex declaration");
		}
		atom.m_velist.insert(atom.m_velist.begin() + i, m_MeshIEList.begin(), m_MeshIEList.end());
		atom.m_VertexStride = D3DXGetDeclVertexSize(&atom.m_velist[0], 0);
		_ASSERT(m_MeshInstanceStride == D3DXGetDeclVertexSize(&atom.m_velist[0], 1));

		HRESULT hr;
		CComPtr<IDirect3DDevice9> Device = mesh->GetDevice();
		if (FAILED(hr = Device->CreateVertexDeclaration(&atom.m_velist[0], &atom.m_Decl)))
		{
			THROW_D3DEXCEPTION(hr);
		}
		atom.cmps.push_back(cmp);
	}
	else
	{
		atom_iter->second.cmps.push_back(cmp);
	}
}

void RenderPipeline::PushEmitter(unsigned int PassID, my::Emitter * emitter, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	EmitterAtom atom;
	atom.emitter = emitter;
	atom.shader = shader;
	atom.cmp = cmp;
	atom.mtl = mtl;
	atom.lparam = lparam;
	m_Pass[PassID].m_EmitterList.push_back(atom);
}

bool RenderPipeline::WorldEmitterAtomKey::operator == (const WorldEmitterAtomKey & rhs) const
{
	return get<0>() == rhs.get<0>()
		&& *get<1>() == *rhs.get<1>();
}

namespace boost
{
	size_t hash_value(const RenderPipeline::WorldEmitterAtomKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, *key.get<1>());
		return seed;
	}
}

void RenderPipeline::PushWorldEmitter(unsigned int PassID, my::Emitter * emitter, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam)
{
	WorldEmitterAtomKey key(shader, mtl, lparam);
	WorldEmitterAtom & atom = m_Pass[PassID].m_WorldEmitterMap[key];
	atom.emitters.push_back(std::make_pair(emitter, cmp));
	atom.TotalParticles += emitter->m_ParticleList.size();
}
