#include "stdafx.h"
#include <sstream>
#include "RenderPipeline.h"

using namespace my;
RenderPipeline::IRenderContext::IRenderContext(void)
	: m_SkyLightDiffuse(1.0f,1.0f,1.0f,1.0f)
	, m_SkyLightAmbient(0.3f,0.3f,0.3f,0.0f)
	, m_BkColor(D3DCOLOR_ARGB(255,45,50,170))
	, m_SkyBoxEnable(false)
	, m_WireFrame(false)
	, m_DofEnable(false)
	, m_DofParams(5.0f,15.0f,25.0f,1.0f)
	, m_FxaaEnable(false)
	, m_SsaoEnable(false)
	, m_SsaoBias(0.2f)
	, m_SsaoIntensity(5.0f)
	, m_SsaoRadius(100.0f)
	, m_SsaoScale(10.0f)
{
}

RenderPipeline::RenderPipeline(void)
	: m_ParticleVertexStride(0)
	, m_ParticleInstanceStride(0)
	, m_MeshInstanceStride(0)
	, SHADOW_MAP_SIZE(1024)
	, SHADOW_EPSILON(0.001f)
	, m_ShadowRT(new Texture2D())
	, m_ShadowDS(new Surface())
{
}

RenderPipeline::~RenderPipeline(void)
{
}

namespace boost
{
	static size_t hash_value(const RenderPipeline::ShaderCacheKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, key.get<1>());
		boost::hash_combine(seed, key.get<2>());
		return seed;
	}
}

my::Effect * RenderPipeline::QueryShader(MeshType mesh_type, bool bInstance, const char * path, unsigned int PassID)
{
	ShaderCacheKey key(mesh_type, bInstance, path);
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(key);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	struct Header
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

	std::string name = PathFindFileNameA(path);
	my::ResourceMgr::getSingleton().m_EffectInclude = my::ZipIStreamDir::ReplaceSlash(path);
	PathRemoveFileSpecA(&my::ResourceMgr::getSingleton().m_EffectInclude[0]);

	std::ostringstream oss;
	oss << "#define SHADOW_MAP_SIZE " << SHADOW_MAP_SIZE << std::endl;
	oss << "#define SHADOW_EPSILON " << SHADOW_EPSILON << std::endl;
	oss << "#define INSTANCE " << (unsigned int)bInstance << std::endl;
	oss << "#include \"CommonHeader.fx\"" << std::endl;
	oss << "#include \"" << Header::vs_header(mesh_type) << "\"" << std::endl;
	oss << "#include \"" << name << "\"" << std::endl;
	std::string source = oss.str();

#ifdef _DEBUG
	CComPtr<ID3DXBuffer> buff;
	if (SUCCEEDED(D3DXPreprocessShader(source.c_str(), source.length(), NULL, my::ResourceMgr::getSingletonPtr(), &buff, NULL)))
	{
		std::string::size_type ext_pos = name.find_last_of(".");
		if (ext_pos != std::string::npos)
		{
			name.replace(ext_pos, 1, "\0");
		}
		std::basic_string<TCHAR> tmp_path = str_printf(_T("%S_%u_%u.fx"), name.c_str(), mesh_type, bInstance);
		my::OStreamPtr ostr = my::FileOStream::Open(tmp_path.c_str());
		ostr->write(buff->GetBufferPointer(), buff->GetBufferSize()-1);
	}
#endif

	my::EffectPtr shader(new my::Effect());
	try
	{
		shader->CreateEffect(source.c_str(), source.size(), NULL, my::ResourceMgr::getSingletonPtr(), 0, my::ResourceMgr::getSingleton().m_EffectPool);
	}
	catch (const my::Exception & e)
	{
		shader.reset();
		const std::string & what = e.what();
		my::DxutApp::getSingleton().m_EventLog(what.c_str());
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

	if (!(m_DofEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/DofEffect.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_DofEffect failed");
	}

	if (!(m_FxaaEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/FXAA.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_FxaaEffect failed");
	}

	if (!(m_SsaoEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/SSAO.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SsaoEffect failed");
	}
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

void RenderPipeline::OnFrameRender(
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
	m_SimpleSample->SetMatrix("g_View", pRC->m_SkyLightCam->m_View);
	m_SimpleSample->SetMatrix("g_ViewProj", pRC->m_SkyLightCam->m_ViewProj);
	V(pd3dDevice->SetRenderTarget(0, ShadowSurf));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowDS->m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeShadow, pd3dDevice, fTime, fElapsedTime);
	ShadowSurf.Release();

	pRC->QueryRenderComponent(Frustum::ExtractMatrix(pRC->m_Camera->m_ViewProj), this, PassTypeToMask(PassTypeNormal) | PassTypeToMask(PassTypeLight) | PassTypeToMask(PassTypeOpaque) | PassTypeToMask(PassTypeTransparent));

	CComPtr<IDirect3DSurface9> NormalSurf = pRC->m_NormalRT->GetSurfaceLevel(0);
	CComPtr<IDirect3DSurface9> PositionSurf = pRC->m_PositionRT->GetSurfaceLevel(0);
	m_SimpleSample->SetMatrix("g_View", pRC->m_Camera->m_View);
	m_SimpleSample->SetMatrix("g_ViewProj", pRC->m_Camera->m_ViewProj);
	m_SimpleSample->SetMatrix("g_InvViewProj", pRC->m_Camera->m_InverseViewProj);
	m_SimpleSample->SetVector("g_Eye", pRC->m_Camera->m_Eye);
	m_SimpleSample->SetVector("g_SkyLightDir", -pRC->m_SkyLightCam->m_View.column<2>().xyz.normalize()); // ! RH -z
	m_SimpleSample->SetMatrix("g_SkyLightViewProj", pRC->m_SkyLightCam->m_ViewProj);
	m_SimpleSample->SetVector("g_SkyLightDiffuse", pRC->m_SkyLightDiffuse);
	m_SimpleSample->SetVector("g_SkyLightAmbient", pRC->m_SkyLightAmbient);
	m_SimpleSample->SetTexture("g_ShadowRT", m_ShadowRT.get());
	V(pd3dDevice->SetRenderTarget(0, NormalSurf));
	V(pd3dDevice->SetRenderTarget(1, PositionSurf));
	V(pd3dDevice->SetDepthStencilSurface(ScreenDepthStencilSurf));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	RenderAllObjects(PassTypeNormal, pd3dDevice, fTime, fElapsedTime);
	NormalSurf.Release();
	PositionSurf.Release();

	m_SimpleSample->SetTexture("g_NormalRT", pRC->m_NormalRT.get());
	m_SimpleSample->SetTexture("g_PositionRT", pRC->m_PositionRT.get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_LightRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetRenderTarget(1, NULL));
	if (pRC->m_SsaoEnable)
	{
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		m_SsaoEffect->SetFloat("g_bias", pRC->m_SsaoBias);
		m_SsaoEffect->SetFloat("g_intensity", pRC->m_SsaoIntensity);
		m_SsaoEffect->SetFloat("g_sample_rad", pRC->m_SsaoRadius);
		m_SsaoEffect->SetFloat("g_scale", pRC->m_SsaoScale);
		m_SsaoEffect->Begin();
		m_SsaoEffect->BeginPass(0);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(quad[0])));
		m_SsaoEffect->EndPass();
		m_SsaoEffect->End();
	}
	else
	{
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_COLORVALUE(
			pRC->m_SkyLightAmbient.x, pRC->m_SkyLightAmbient.y, pRC->m_SkyLightAmbient.z, pRC->m_SkyLightAmbient.w), 0, 0));
	}
	RenderAllObjects(PassTypeLight, pd3dDevice, fTime, fElapsedTime);

	m_SimpleSample->SetTexture("g_LightRT", pRC->m_LightRT.get());
	V(pd3dDevice->SetRenderTarget(0, pRC->m_OpaqueRT.GetNextTarget()->GetSurfaceLevel(0)));
	if (pRC->m_SkyBoxEnable)
	{
		struct CUSTOMVERTEX
		{
			float x, y, z;
			D3DCOLOR color;
			FLOAT tu, tv;
		};
		CUSTOMVERTEX vertices[] =
		{
			{-1,  1, -1, pRC->m_BkColor, 0, 0},
			{-1, -1, -1, pRC->m_BkColor, 0, 1},
			{ 1, -1, -1, pRC->m_BkColor, 1, 1},
			{ 1,  1, -1, pRC->m_BkColor, 1, 0},
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
			my::Matrix4::RotationX(D3DXToRadian( 90)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationX(D3DXToRadian(180)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationX(D3DXToRadian(270)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationY(D3DXToRadian( 90)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
			my::Matrix4::RotationY(D3DXToRadian(270)) * my::Matrix4::Translation(pRC->m_Camera->m_Eye),
		};
		for (unsigned int i = 0; i < _countof(pRC->m_SkyBoxTextures); i++)
		{
			V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&transforms[i]));
			V(pd3dDevice->SetTexture(0, pRC->m_SkyBoxTextures[i] ? pRC->m_SkyBoxTextures[i]->m_ptr : NULL));
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &vertices, sizeof(vertices[0])));
		}
	}
	else
	{
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, pRC->m_BkColor, 1.0f, 0)); // ! d3dmultisample will not work
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
		m_FxaaEffect->SetTexture("InputTexture", pRC->m_OpaqueRT.GetNextSource().get());
		Vector4 RCPFrame(1.0f / pBackBufferSurfaceDesc->Width, 1.0f / pBackBufferSurfaceDesc->Height, 0.0f, 0.0f);
		m_FxaaEffect->SetFloatArray("RCPFrame", &RCPFrame.x, sizeof(RCPFrame) / sizeof(float));
		m_FxaaEffect->Begin();
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

		m_DofEffect->SetVector("g_DofParams", pRC->m_DofParams);
		m_DofEffect->SetTexture("g_OpaqueRT", pRC->m_OpaqueRT.GetNextSource().get());
		m_DofEffect->SetTexture("g_DownFilterRT", pRC->m_DownFilterRT.GetNextSource().get());
		V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		UINT passes = m_DofEffect->Begin();
		m_DofEffect->BeginPass(0);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
		m_DofEffect->EndPass();
		pRC->m_DownFilterRT.Flip();

		m_DofEffect->SetTexture("g_DownFilterRT", pRC->m_DownFilterRT.GetNextSource().get());
		V(pd3dDevice->SetRenderTarget(0, pRC->m_DownFilterRT.GetNextTarget()->GetSurfaceLevel(0)));
		m_DofEffect->BeginPass(1);
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad_quat, sizeof(quad[0])));
		m_DofEffect->EndPass();
		pRC->m_DownFilterRT.Flip();

		m_DofEffect->SetTexture("g_DownFilterRT", pRC->m_DownFilterRT.GetNextSource().get());
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
			prim_iter->AttribId,
			prim_iter->shader,
			prim_iter->setter);
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
			indexed_prim_iter->AttribId,
			indexed_prim_iter->shader,
			indexed_prim_iter->setter);
		m_PassDrawCall[PassID]++;
	}

	MeshAtomList::iterator mesh_iter = m_Pass[PassID].m_MeshList.begin();
	for (; mesh_iter != m_Pass[PassID].m_MeshList.end(); mesh_iter++)
	{
		DrawMesh(PassID, pd3dDevice, mesh_iter->mesh, mesh_iter->AttribId, mesh_iter->shader, mesh_iter->setter);
		m_PassDrawCall[PassID]++;
	}

	MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
	for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
	{
		if (!mesh_inst_iter->second.m_TransformList.empty())
		{
			DrawMeshInstance(
				PassID,
				pd3dDevice,
				mesh_inst_iter->first.get<0>(),
				mesh_inst_iter->first.get<1>(),
				mesh_inst_iter->first.get<2>(),
				mesh_inst_iter->second.setter,
				mesh_inst_iter->second);
			m_PassDrawCall[PassID]++;
		}
	}

	EmitterAtomList::iterator emitter_iter = m_Pass[PassID].m_EmitterList.begin();
	for (; emitter_iter != m_Pass[PassID].m_EmitterList.end(); emitter_iter++)
	{
		if (!emitter_iter->emitter->m_ParticleList.empty())
		{
			DrawEmitter(PassID, pd3dDevice, emitter_iter->emitter, emitter_iter->AttribId, emitter_iter->shader, emitter_iter->setter);
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
		MeshInstanceAtomMap::iterator mesh_inst_iter = m_Pass[PassID].m_MeshInstanceMap.begin();
		for (; mesh_inst_iter != m_Pass[PassID].m_MeshInstanceMap.end(); mesh_inst_iter++)
		{
			mesh_inst_iter->second.m_TransformList.clear();
		}
		m_Pass[PassID].m_EmitterList.clear();
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
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
{
	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, pVB, 0, VertexStride));
	V(pd3dDevice->SetVertexDeclaration(pDecl));
	V(pd3dDevice->SetIndices(pIB));

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(pd3dDevice, shader, AttribId);
	{
		shader->BeginPass(PassID);
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
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
{
	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(pd3dDevice, shader, AttribId);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetVertexDeclaration(pDecl));
		V(pd3dDevice->DrawIndexedPrimitiveUP(
			PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawMesh(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(pd3dDevice, shader, AttribId);
	{
		shader->BeginPass(PassID);
		mesh->DrawSubset(AttribId);
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::DrawMeshInstance(
	unsigned int PassID,
	IDirect3DDevice9 * pd3dDevice,
	my::Mesh * mesh,
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter,
	MeshInstanceAtom & atom)
{
	_ASSERT(AttribId < atom.m_AttribTable.size());
	const DWORD NumInstances = atom.m_TransformList.size();
	_ASSERT(NumInstances <= MESH_INSTANCE_MAX);

	Matrix4 * mat = (Matrix4 *)m_MeshInstanceData.Lock(0, NumInstances * m_MeshInstanceStride, D3DLOCK_DISCARD);
	memcpy(mat, &atom.m_TransformList[0], NumInstances * m_MeshInstanceStride);
	m_MeshInstanceData.Unlock();

	CComPtr<IDirect3DVertexBuffer9> vb = mesh->GetVertexBuffer();
	CComPtr<IDirect3DIndexBuffer9> ib = mesh->GetIndexBuffer();

	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, vb, 0, atom.m_VertexStride));
	V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
	V(pd3dDevice->SetStreamSource(1, m_MeshInstanceData.m_ptr, 0, m_MeshInstanceStride));
	V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
	V(pd3dDevice->SetVertexDeclaration(atom.m_Decl));
	V(pd3dDevice->SetIndices(ib));

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(pd3dDevice, shader, AttribId);
	{
		shader->BeginPass(PassID);
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
			atom.m_AttribTable[AttribId].VertexStart,
			atom.m_AttribTable[AttribId].VertexCount,
			atom.m_AttribTable[AttribId].FaceStart * 3,
			atom.m_AttribTable[AttribId].FaceCount));
		shader->EndPass();
	}
	shader->End();

	V(pd3dDevice->SetStreamSourceFreq(0,1));
	V(pd3dDevice->SetStreamSourceFreq(1,1));
}

void RenderPipeline::DrawEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	const DWORD NumInstances = emitter->m_ParticleList.size();
	_ASSERT(NumInstances <= PARTICLE_INSTANCE_MAX);
	_ASSERT(m_ParticleInstanceStride == sizeof(Emitter::ParticleList::value_type));
	unsigned char * pVertices = (unsigned char *)m_ParticleInstanceData.Lock(0, m_ParticleInstanceStride * NumInstances, D3DLOCK_DISCARD);
	_ASSERT(pVertices);
	Emitter::ParticleList::const_array_range array_one = emitter->m_ParticleList.array_one();
	if (array_one.second > 0)
	{
		size_t length = array_one.second * sizeof(Emitter::ParticleList::value_type);
		memcpy(pVertices, array_one.first, length);
		pVertices += length;
	}
	Emitter::ParticleList::const_array_range array_two = emitter->m_ParticleList.array_two();
	if (array_two.second > 0)
	{
		size_t length = array_two.second * sizeof(Emitter::ParticleList::value_type);
		memcpy(pVertices, array_two.first, length);
		pVertices += length;
	}
	m_ParticleInstanceData.Unlock();

	shader->SetTechnique("RenderScene");
	const UINT passes = shader->Begin(0);
	_ASSERT(PassID < passes);
	setter->OnSetShader(pd3dDevice, shader, AttribId);
	{
		shader->BeginPass(PassID);
		HRESULT hr;
		V(pd3dDevice->SetStreamSource(0, m_ParticleVertexBuffer.m_ptr, 0, m_ParticleVertexStride));
		V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));

		V(pd3dDevice->SetStreamSource(1, m_ParticleInstanceData.m_ptr, 0, m_ParticleInstanceStride));
		V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

		V(pd3dDevice->SetVertexDeclaration(m_ParticleDecl));
		V(pd3dDevice->SetIndices(m_ParticleIndexBuffer.m_ptr));
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));

		V(pd3dDevice->SetStreamSourceFreq(0,1));
		V(pd3dDevice->SetStreamSourceFreq(1,1));
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
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
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
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
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
	DWORD AttribId,
	my::Effect * shader,
	IShaderSetter * setter)
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
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_IndexedPrimitiveUPList.push_back(atom);
}

void RenderPipeline::PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	MeshAtom atom;
	atom.mesh = mesh;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_MeshList.push_back(atom);
}

namespace boost
{
	static size_t hash_value(const RenderPipeline::MeshInstanceAtomKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, key.get<1>());
		boost::hash_combine(seed, key.get<2>());
		return seed;
	}
}

void RenderPipeline::PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter)
{
	MeshInstanceAtomKey key(mesh, AttribId, shader);
	MeshInstanceAtomMap::iterator atom_iter = m_Pass[PassID].m_MeshInstanceMap.find(key);
	if (atom_iter == m_Pass[PassID].m_MeshInstanceMap.end())
	{
		MeshInstanceAtom & atom = m_Pass[PassID].m_MeshInstanceMap[key];
		atom.setter = setter;
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
	}
	m_Pass[PassID].m_MeshInstanceMap[key].m_TransformList.push_back(World);
}

void RenderPipeline::PushEmitter(unsigned int PassID, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	EmitterAtom atom;
	atom.emitter = emitter;
	atom.AttribId = AttribId;
	atom.shader = shader;
	atom.setter = setter;
	m_Pass[PassID].m_EmitterList.push_back(atom);
}
