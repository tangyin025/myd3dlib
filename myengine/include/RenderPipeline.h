#pragma once

#include "myMesh.h"
#include "myEffect.h"
#include "myEmitter.h"
#include "myUtility.h"
#include "Material.h"
#include <boost/unordered_map.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

class Component;

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
		PassTypeLight			= 2,
		PassTypeBackground		= 3,
		PassTypeOpaque			= 4,
		PassTypeTransparent		= 5,
		PassTypeNum
	};

	enum PassMask
	{
		PassMaskNone = 0,
		PassMaskLight = 1 << PassTypeLight,
		PassMaskBackground = 1 << PassTypeBackground,
		PassMaskOpaque = 1 << PassTypeOpaque,
		PassMaskNormalOpaque = 1 << PassTypeNormal | 1 << PassTypeOpaque,
		PassMaskShadowNormalOpaque = 1 << PassTypeShadow | 1 << PassTypeNormal | 1 << PassTypeOpaque,
		PassMaskTransparent = 1 << PassTypeTransparent,
	};

	typedef boost::unordered_map<size_t, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	my::D3DVertexElementSet m_ParticleVertElems;

	my::D3DVertexElementSet m_ParticleInstanceElems;

	static const DWORD m_ParticleVertStride = 20;

	static const DWORD m_ParticleInstanceStride = 56;

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

	my::VertexBuffer m_ParticleQuadVb;

	my::IndexBuffer m_ParticleQuadIb;

	my::VertexBuffer m_ParticleInstanceData;

	my::D3DVertexElementSet m_MeshInstanceElems;

	static const DWORD m_MeshInstanceStride = 64;

	std::vector<D3DVERTEXELEMENT9> m_MeshIEList;

	my::VertexBuffer m_MeshInstanceData;

	unsigned int SHADOW_MAP_SIZE;

	float SHADOW_EPSILON;

	my::Texture2DPtr m_ShadowRT;

	my::SurfacePtr m_ShadowDS;

	my::OrthoCamera m_SkyLightCam;

	my::Vector4 m_SkyLightColor;

	my::Vector4 m_AmbientColor;

	my::EffectPtr m_SimpleSample;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_ScreenDim;

	D3DXHANDLE handle_ShadowMapSize;

	D3DXHANDLE handle_ShadowEpsilon;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_Eye;

	D3DXHANDLE handle_View;

	D3DXHANDLE handle_ViewProj;

	D3DXHANDLE handle_InvViewProj;

	D3DXHANDLE handle_SkyLightView;

	D3DXHANDLE handle_SkyLightViewProj;

	D3DXHANDLE handle_SkyLightColor;

	D3DXHANDLE handle_AmbientColor;

	D3DXHANDLE handle_ShadowRT;

	D3DXHANDLE handle_NormalRT;

	D3DXHANDLE handle_PositionRT;

	D3DXHANDLE handle_LightRT;

	D3DXHANDLE handle_OpaqueRT;

	D3DXHANDLE handle_DownFilterRT;

	my::EffectPtr m_DofEffect;

	my::Vector4 m_DofParams;

	D3DXHANDLE handle_DofParams;

	my::EffectPtr m_FxaaEffect;

	D3DXHANDLE handle_InputTexture;

	D3DXHANDLE handle_RCPFrame;

	my::EffectPtr m_SsaoEffect;

	D3DXHANDLE handle_bias;

	D3DXHANDLE handle_intensity;

	D3DXHANDLE handle_sample_rad;

	D3DXHANDLE handle_scale;

	float m_SsaoBias;

	float m_SsaoIntensity;

	float m_SsaoRadius;

	float m_SsaoScale;

	my::EffectPtr m_FogEffect;

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
		typedef boost::array<my::Texture2DPtr, 2> RTArray;

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

		my::Texture2DPtr  GetPrevTarget(void)
		{
			return m_RenderTarget[1 - m_Next];
		}

		my::Texture2DPtr  GetPrevSource(void)
		{
			return m_RenderTarget[m_Next];
		}

		my::Texture2DPtr & GetNextTarget(void)
		{
			return m_RenderTarget[m_Next];
		}

		my::Texture2DPtr & GetNextSource(void)
		{
			return m_RenderTarget[1 - m_Next];
		}
	};

	class IRenderContext
	{
	public:
		my::CameraPtr m_Camera;

		bool m_WireFrame;

		bool m_DofEnable;

		bool m_FxaaEnable;

		bool m_SsaoEnable;

		bool m_FogEnable;

		my::Texture2DPtr m_NormalRT;

		my::Texture2DPtr m_PositionRT;

		my::Texture2DPtr m_LightRT;

		RTChain m_OpaqueRT;

		RTChain m_DownFilterRT;

		IRenderContext(void)
			: m_WireFrame(false)
			, m_DofEnable(false)
			, m_FxaaEnable(false)
			, m_SsaoEnable(false)
			, m_FogEnable(false)
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
		my::Mesh * mesh;
		DWORD AttribId;
		my::Effect * shader;
		Component * cmp;
		Material * mtl;
		LPARAM lparam;
	};

	typedef std::vector<MeshAtom> MeshAtomList;

	struct MeshInstanceAtom
	{
		std::vector<D3DXATTRIBUTERANGE> m_AttribTable;
		std::vector<D3DVERTEXELEMENT9> m_velist;
		DWORD m_VertexStride;
		CComPtr<IDirect3DVertexDeclaration9> m_Decl;
		std::vector<Component *> cmps;
	};

	class MeshInstanceAtomKey : public boost::tuple<my::Mesh *, DWORD, my::Effect *, Material *, LPARAM>
	{
	public:
		MeshInstanceAtomKey(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Material * mtl, LPARAM lparam)
			: tuple(mesh, AttribId, shader, mtl, lparam)
		{
		}

		bool operator == (const MeshInstanceAtomKey & rhs) const
		{
			return get<0>() == rhs.get<0>()
				&& get<1>() == rhs.get<1>()
				&& get<2>() == rhs.get<2>()
				&& *get<3>() == *rhs.get<3>(); // ! mtl ptr must be valid object
		}
	};

	typedef boost::unordered_map<MeshInstanceAtomKey, MeshInstanceAtom> MeshInstanceAtomMap;

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

		bool operator == (const EmitterInstanceAtomKey & rhs) const
		{
			return get<0>() == rhs.get<0>()
				&& get<1>() == rhs.get<1>()
				&& get<2>() == rhs.get<2>()
				&& get<3>() == rhs.get<3>()
				&& get<4>() == rhs.get<4>()
				&& *get<5>() == *rhs.get<5>();
		}
	};

	typedef boost::unordered_map<EmitterInstanceAtomKey, EmitterInstanceAtom> EmitterInstanceAtomMap;

	struct Pass
	{
		IndexedPrimitiveAtomList m_IndexedPrimitiveList;
		IndexedPrimitiveInstanceAtomList m_IndexedPrimitiveInstanceList;
		IndexedPrimitiveUPAtomList m_IndexedPrimitiveUPList;
		MeshAtomList m_MeshList;
		MeshInstanceAtomMap m_MeshInstanceMap;
		EmitterInstanceAtomMap m_EmitterInstanceMap;
	};

	boost::array<Pass, PassTypeNum> m_Pass;

	boost::array<int, PassTypeNum> m_PassDrawCall;

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

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		IRenderContext * pRC,
		double fTime,
		float fElapsedTime);

	void RenderAllObjects(
		unsigned int PassID,
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	void ClearAllObjects(void);

	void DrawIndexedPrimitive(
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
		LPARAM lparam);

	void DrawIndexedPrimitiveInstance(
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
		LPARAM lparam);

	void DrawIndexedPrimitiveUP(
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
		LPARAM lparam);

	void DrawMesh(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

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

	void PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

	void PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

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
