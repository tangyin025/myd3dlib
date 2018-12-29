#pragma once

#include "myMesh.h"
#include "myEffect.h"
#include "myUtility.h"
#include "Material.h"
#include <boost/unordered_map.hpp>
#include <boost/array.hpp>

namespace my
{
	class Emitter;
};

class Component;

class RenderPipeline
{
public:
	enum MeshType
	{
		MeshTypeStatic			= 0,
		MeshTypeAnimation		= 1,
		MeshTypeParticle		= 2,
		MeshTypeTerrain			= 3,
		MeshTypeNum
	};

	enum PassType
	{
		PassTypeShadow			= 0,
		PassTypeNormal			= 1,
		PassTypeLight			= 2,
		PassTypeOpaque			= 3,
		PassTypeTransparent		= 4,
		PassTypeNum
	};

	enum PassMask
	{
		PassMaskNone = 0,
		PassMaskLight = 1 << PassTypeLight,
		PassMaskOpaque = 1 << PassTypeShadow | 1 << PassTypeNormal | 1 << PassTypeOpaque,
		PassMaskTransparent = 1 << PassTypeTransparent,
	};

	typedef std::tuple<MeshType, bool, std::string> ShaderCacheKey;

	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	my::D3DVertexElementSet m_ParticleVertexElems;

	my::D3DVertexElementSet m_ParticleInstanceElems;

	std::vector<D3DVERTEXELEMENT9> m_ParticleVEList;

	DWORD m_ParticleVertexStride;

	DWORD m_ParticleInstanceStride;

	CComPtr<IDirect3DVertexDeclaration9> m_ParticleDecl;

	my::VertexBuffer m_ParticleVertexBuffer;

	my::IndexBuffer m_ParticleIndexBuffer;

	my::VertexBuffer m_ParticleInstanceData;

	my::D3DVertexElementSet m_MeshInstanceElems;

	std::vector<D3DVERTEXELEMENT9> m_MeshIEList;

	DWORD m_MeshInstanceStride;

	my::VertexBuffer m_MeshInstanceData;

	unsigned int SHADOW_MAP_SIZE;

	float SHADOW_EPSILON;

	my::Texture2DPtr m_ShadowRT;

	my::SurfacePtr m_ShadowDS;

	my::Vector4 m_BgColor;

	my::CameraPtr m_SkyLightCam;

	my::Vector4 m_SkyLightDiffuse;

	my::Vector4 m_SkyLightAmbient;

	bool m_SkyBoxEnable;

	MaterialParameterTexture m_SkyBoxTextures[6];

	my::EffectPtr m_SimpleSample;

	my::EffectPtr m_DofEffect;

	my::Vector4 m_DofParams;

	my::EffectPtr m_FxaaEffect;

	my::EffectPtr m_SsaoEffect;

	float m_SsaoBias;

	float m_SsaoIntensity;

	float m_SsaoRadius;

	float m_SsaoScale;

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

		bool operator == (const MeshInstanceAtomKey & rhs) const;
	};

	typedef boost::unordered_map<MeshInstanceAtomKey, MeshInstanceAtom> MeshInstanceAtomMap;

	struct EmitterAtom
	{
		my::Emitter * emitter;
		my::Effect * shader;
		Component * cmp;
		Material * mtl;
		LPARAM lparam;
	};

	typedef std::vector<EmitterAtom> EmitterAtomList;

	struct WorldEmitterAtom
	{
		typedef std::vector<std::pair<my::Emitter *, Component *> > EmitterPairList;
		EmitterPairList emitters;
		DWORD TotalParticles;
		WorldEmitterAtom()
			: TotalParticles(0)
		{}
	};

	class WorldEmitterAtomKey : public boost::tuple<my::Effect *, Material *, LPARAM>
	{
	public:
		WorldEmitterAtomKey(my::Effect * shader, Material * mtl, LPARAM lparam)
			: tuple(shader, mtl, lparam)
		{
		}

		bool operator == (const WorldEmitterAtomKey & rhs) const;
	};

	typedef boost::unordered_map<WorldEmitterAtomKey, WorldEmitterAtom> WorldEmitterAtomMap;

	struct Pass
	{
		IndexedPrimitiveAtomList m_IndexedPrimitiveList;
		IndexedPrimitiveUPAtomList m_IndexedPrimitiveUPList;
		MeshAtomList m_MeshList;
		MeshInstanceAtomMap m_MeshInstanceMap;
		EmitterAtomList m_EmitterList;
		WorldEmitterAtomMap m_WorldEmitterMap;
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

	my::Effect * QueryShader(MeshType mesh_type, bool bInstance, const char * path, unsigned int PassID);

	static unsigned int PassTypeToMask(unsigned int pass_type)
	{
		_ASSERT(pass_type >= 0 && pass_type < PassTypeNum); return 1 << pass_type;
	}

	static const char * PassTypeToStr(unsigned int pass_type);

	static void UpdateQuad(QuadVertex * quad, const my::Vector2 & dim);

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

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

	void DrawMeshInstance(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, Material * mtl, LPARAM lparam, MeshInstanceAtom & atom);

	void DrawEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

	void DrawWorldEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Effect * shader, Material * mtl, LPARAM lparam, WorldEmitterAtom & atom);

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

	void PushEmitter(unsigned int PassID, my::Emitter * emitter, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);

	void PushWorldEmitter(unsigned int PassID, my::Emitter * emitter, my::Effect * shader, Component * cmp, Material * mtl, LPARAM lparam);
};
