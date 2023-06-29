#pragma once

#include "myMesh.h"
#include "myEmitter.h"
#include <boost/unordered_map.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

namespace my {
	class Texture2D;

	class Surface;

	class Effect;

	class Camera;
}

class Component;

class MeshComponent;

class Material;

class RenderPipeline
{
public:
	static const unsigned int PARTICLE_INSTANCE_MAX = 65536;

	static const unsigned int MESH_INSTANCE_MAX = 4096;

	enum MeshType
	{
		MeshTypeMesh			= 0,
		MeshTypeParticle		= 1,
		MeshTypeTerrain			= 2,
		MeshTypeNum
	};

	enum PassType
	{
		PassTypeShadow			= 0,
		PassTypeNormal			= 1,
		PassTypeNormalTransparent	= 2,
		PassTypeLight			= 3,
		PassTypeBackground		= 4,
		PassTypeOpaque			= 5,
		PassTypeTransparent		= 6,
		PassTypeNum
	};

	typedef boost::unordered_map<size_t, boost::shared_ptr<my::Effect> > ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	my::D3DVertexElementSet m_ParticleVertElems;

	my::D3DVertexElementSet m_ParticleInstanceElems;

	static const DWORD m_ParticleVertStride = 20;

	static const DWORD m_ParticleInstanceStride = 64;

	std::vector<D3DVERTEXELEMENT9> m_ParticleIEList;

	CComPtr<IDirect3DVertexDeclaration9> m_ParticleIEDecl;

	enum ParticlePrimitiveType
	{
		ParticlePrimitiveTri,
		ParticlePrimitiveQuad,
		ParticlePrimitiveTypeCount
	};

	enum ParticlePrimitiveColumn
	{
		ParticlePrimitiveMinVertexIndex,
		ParticlePrimitiveNumVertices,
		ParticlePrimitiveStartIndex,
		ParticlePrimitivePrimitiveCount,
		ParticlePrimitiveColumnCount
	};

	static const UINT m_ParticlePrimitiveInfo[ParticlePrimitiveTypeCount][ParticlePrimitiveColumnCount];

	my::VertexBuffer m_ParticleVb;

	my::IndexBuffer m_ParticleIb;

	my::VertexBuffer m_ParticleInstanceData;

	my::D3DVertexElementSet m_MeshInstanceElems;

	static const DWORD m_MeshInstanceStride = 80;

	std::vector<D3DVERTEXELEMENT9> m_MeshIEList;

	my::VertexBuffer m_MeshInstanceData;

	unsigned int SHADOW_MAP_SIZE;

	float SHADOW_BIAS;

	boost::shared_ptr<my::Texture2D> m_ShadowRT;

	boost::shared_ptr<my::Surface> m_ShadowDS;

	boost::shared_ptr<my::Camera> m_SkyLightCam;

	my::Vector4 m_SkyLightColor;

	my::Vector4 m_AmbientColor;

	boost::shared_ptr<my::Effect> m_SimpleSample;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_ScreenDim;

	D3DXHANDLE handle_ShadowMapSize;

	D3DXHANDLE handle_ShadowBias;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_Eye;

	D3DXHANDLE handle_View;

	D3DXHANDLE handle_ViewProj;

	D3DXHANDLE handle_SkyLightView;

	D3DXHANDLE handle_SkyLightViewProj;

	D3DXHANDLE handle_SkyLightColor;

	D3DXHANDLE handle_AmbientColor;

	D3DXHANDLE handle_ShadowRT;

	D3DXHANDLE handle_NormalRT;

	D3DXHANDLE handle_SpecularRT;

	D3DXHANDLE handle_PositionRT;

	D3DXHANDLE handle_LightRT;

	D3DXHANDLE handle_OpaqueRT;

	D3DXHANDLE handle_DownFilterRT;

	boost::shared_ptr<my::Effect> m_DofEffect;

	D3DXHANDLE handle_DofParams;

	my::Vector4 m_DofParams;

	boost::shared_ptr<my::Effect> m_BloomEffect;

	D3DXHANDLE handle_LuminanceThreshold;

	D3DXHANDLE handle_BloomColor;

	D3DXHANDLE handle_BloomFactor;

	float m_LuminanceThreshold;

	my::Vector3 m_BloomColor;

	float m_BloomFactor;

	boost::shared_ptr<my::Effect> m_FxaaEffect;

	D3DXHANDLE handle_InputTexture;

	D3DXHANDLE handle_RCPFrame;

	boost::shared_ptr<my::Effect> m_SsaoEffect;

	D3DXHANDLE handle_bias;

	D3DXHANDLE handle_intensity;

	D3DXHANDLE handle_sample_rad;

	D3DXHANDLE handle_scale;

	float m_SsaoBias;

	float m_SsaoIntensity;

	float m_SsaoRadius;

	float m_SsaoScale;

	boost::shared_ptr<my::Effect> m_FogEffect;

	D3DXHANDLE handle_FogColor;

	D3DXHANDLE handle_FogStartDistance;

	D3DXHANDLE handle_FogHeight;

	D3DXHANDLE handle_FogFalloff;

	my::Vector4 m_FogColor;

	float m_FogStartDistance;

	float m_FogHeight;

	float m_FogFalloff;

	struct RTChain
	{
		typedef boost::array<boost::shared_ptr<my::Texture2D>, 2> RTArray;

		RTArray m_RenderTarget;

		int m_Next;

		RTChain(void)
			: m_Next(0)
		{
		}

		void Flip(void)
		{
			m_Next = 1 - m_Next;
		}

		boost::shared_ptr<my::Texture2D>  GetPrevTarget(void)
		{
			return m_RenderTarget[1 - m_Next];
		}

		boost::shared_ptr<my::Texture2D>  GetPrevSource(void)
		{
			return m_RenderTarget[m_Next];
		}

		boost::shared_ptr<my::Texture2D> & GetNextTarget(void)
		{
			return m_RenderTarget[m_Next];
		}

		boost::shared_ptr<my::Texture2D> & GetNextSource(void)
		{
			return m_RenderTarget[1 - m_Next];
		}
	};

	enum RenderTargetType
	{
		RenderTargetNormal,
		RenderTargetPosition,
		RenderTargetSpecular,
		RenderTargetLight,
		RenderTargetOpaque,
		RenderTargetDownFilter
	};

	class IRenderContext
	{
	public:
		boost::shared_ptr<my::Camera> m_Camera;

		bool m_WireFrame;

		bool m_DofEnable;

		bool m_BloomEnable;

		bool m_FxaaEnable;

		bool m_SsaoEnable;

		bool m_FogEnable;

		boost::shared_ptr<my::Texture2D> m_NormalRT;

		boost::shared_ptr<my::Texture2D> m_SpecularRT;

		boost::shared_ptr<my::Texture2D> m_PositionRT;

		boost::shared_ptr<my::Texture2D> m_LightRT;

		RTChain m_OpaqueRT;

		RTChain m_DownFilterRT;

		RenderTargetType m_RTType;

		IRenderContext(void)
			: m_WireFrame(false)
			, m_DofEnable(false)
			, m_BloomEnable(false)
			, m_FxaaEnable(false)
			, m_SsaoEnable(false)
			, m_FogEnable(false)
			, m_RTType(RenderTargetOpaque)
		{
		}

		virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask) = 0;
	};

	struct IndexedPrimitiveAtom
	{
		IDirect3DVertexDeclaration9* pDecl;
		IDirect3DVertexBuffer9 * pVB;
		IDirect3DIndexBuffer9 * pIB;
		D3DPRIMITIVETYPE PrimitiveType;
		INT BaseVertexIndex;
		UINT MinVertexIndex;
		UINT NumVertices;
		UINT VertexStride;
		UINT StartIndex;
		UINT PrimitiveCount;
		my::Effect * shader;
		Component * cmp;
		Material * mtl;
		LPARAM lparam;
	};

	typedef std::vector<IndexedPrimitiveAtom> IndexedPrimitiveAtomList;

	struct IndexedPrimitiveInstanceAtom
	{
		IDirect3DVertexDeclaration9* pDecl;
		IDirect3DVertexBuffer9* pVB;
		IDirect3DIndexBuffer9* pIB;
		IDirect3DVertexBuffer9* pInstance;
		D3DPRIMITIVETYPE PrimitiveType;
		INT BaseVertexIndex;
		UINT MinVertexIndex;
		UINT NumVertices;
		UINT VertexStride;
		UINT StartIndex;
		UINT PrimitiveCount;
		UINT NumInstance;
		UINT InstanceStride;
		my::Effect* shader;
		Component* cmp;
		Material* mtl;
		LPARAM lparam;
	};

	typedef std::vector<IndexedPrimitiveInstanceAtom> IndexedPrimitiveInstanceAtomList;

	struct IndexedPrimitiveUPAtom
	{
		IDirect3DVertexDeclaration9* pDecl;
		D3DPRIMITIVETYPE PrimitiveType;
		UINT MinVertexIndex;
		UINT NumVertices;
		UINT PrimitiveCount;
		CONST void* pIndexData;
		D3DFORMAT IndexDataFormat;
		CONST void* pVertexStreamZeroData;
		UINT VertexStreamZeroStride;
		my::Effect * shader;
		Component * cmp;
		Material * mtl;
		LPARAM lparam;
	};

	typedef std::vector<IndexedPrimitiveUPAtom> IndexedPrimitiveUPAtomList;

	struct MeshAtom
	{
		my::OgreMesh * mesh;
		DWORD AttribId;
		my::Effect * shader;
		Component * cmp;
		Material * mtl;
		LPARAM lparam;
	};

	typedef std::vector<MeshAtom> MeshAtomList;

	struct MeshInstanceAtom
	{
		std::vector<D3DVERTEXELEMENT9> m_velist;
		DWORD m_VertexStride;
		CComPtr<IDirect3DVertexDeclaration9> m_Decl;
		std::vector<MeshComponent *> cmps;
	};

	class MeshInstanceAtomKey : public boost::tuple<my::OgreMesh *, DWORD, my::Effect *, Material *, LPARAM>
	{
	public:
		MeshInstanceAtomKey(my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, Material * mtl, LPARAM lparam)
			: tuple(mesh, AttribId, shader, mtl, lparam)
		{
		}

		bool operator == (const MeshInstanceAtomKey & rhs) const;
	};

	typedef boost::unordered_map<MeshInstanceAtomKey, MeshInstanceAtom> MeshInstanceAtomMap;

	struct MeshBatchAtom
	{
		my::Effect * shader;
		Material * mtl;
		LPARAM lparam;
		std::vector<boost::tuple<Component *, unsigned int> > cmps;
	};

	typedef boost::unordered_map<my::OgreMesh *, MeshBatchAtom> MeshBatchAtomMap;

	struct EmitterInstanceAtom
	{
		D3DPRIMITIVETYPE PrimitiveType;
		UINT NumVertices;
		UINT StartIndex;
		UINT PrimitiveCount;
		std::vector<boost::tuple<Component*, my::Emitter::Particle*, unsigned int> > cmps;
	};

	class EmitterInstanceAtomKey : public boost::tuple<
		IDirect3DVertexBuffer9 *,
		IDirect3DIndexBuffer9 *,
		UINT,
		const my::Matrix4 *,
		my::Effect *,
		Material *,
		LPARAM>
	{
	public:
		EmitterInstanceAtomKey(
			IDirect3DVertexBuffer9 * pVB,
			IDirect3DIndexBuffer9 * pIB,
			UINT MinVertexIndex,
			const my::Matrix4 * world,
			my::Effect * shader,
			Material* mtl,
			LPARAM lparam)
			: tuple(pVB, pIB, MinVertexIndex, world, shader, mtl, lparam)
		{
		}

		bool operator == (const EmitterInstanceAtomKey & rhs) const;
	};

	typedef boost::unordered_map<EmitterInstanceAtomKey, EmitterInstanceAtom> EmitterInstanceAtomMap;

	struct Pass
	{
		IndexedPrimitiveAtomList m_IndexedPrimitiveList;
		IndexedPrimitiveInstanceAtomList m_IndexedPrimitiveInstanceList;
		IndexedPrimitiveUPAtomList m_IndexedPrimitiveUPList;
		MeshAtomList m_MeshList;
		MeshInstanceAtomMap m_MeshInstanceMap;
		MeshBatchAtomMap m_MeshBatchMap;
		EmitterInstanceAtomMap m_EmitterInstanceMap;
	};

	boost::array<Pass, PassTypeNum> m_Pass;

	boost::array<int, PassTypeNum> m_PassDrawCall;

	boost::array<int, PassTypeNum> m_PassBatchDrawCall;

	struct QuadVertex
	{
		float x, y, z, rhw;
		float u, v;
	};

public:
	RenderPipeline(void);

	virtual ~RenderPipeline(void);

	my::Effect * QueryShader(MeshType mesh_type, const D3DXMACRO* pDefines, const char * path, unsigned int PassID);

	void LoadShaderCache(LPCTSTR szDir);

	static unsigned int PassTypeToMask(unsigned int pass_type)
	{
		_ASSERT(pass_type >= 0 && pass_type < PassTypeNum); return 1 << pass_type;
	}

	static const char * PassTypeToStr(unsigned int pass_type);

	static void UpdateQuad(QuadVertex * quad, const my::Vector2 & dim);

	HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		IDirect3DSurface9 * ScreenSurf,
		IDirect3DSurface9 * ScreenDepthStencilSurf,
		const D3DSURFACE_DESC * ScreenSurfDesc,
		IRenderContext * pRC,
		double fTime,
		float fElapsedTime);

	void RenderAllObjects(
		IDirect3DDevice9 * pd3dDevice,
		unsigned int PassID,
		IRenderContext * pRC,
		double fTime,
		float fElapsedTime);

	void ClearAllObjects(void);

	void ClearShaderCache(void);

	void DrawIndexedPrimitive(
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
		LPARAM lparam);

	void DrawIndexedPrimitiveInstance(
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
		LPARAM lparam);

	void DrawIndexedPrimitiveUP(
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
		LPARAM lparam);

	void PushIndexedPrimitive(
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
		LPARAM lparam);

	void PushIndexedPrimitiveInstance(
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
		LPARAM lparam);

	void PushIndexedPrimitiveUP(
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
		LPARAM lparam);

	void PushMesh(unsigned int PassID, my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

	void PushMeshInstance(unsigned int PassID, my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, MeshComponent * mesh_cmp, Material * mtl, LPARAM lparam);

	void PushMeshBatch(unsigned int PassID, my::OgreMesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

	void PushEmitter(
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
		LPARAM lparam);
};
