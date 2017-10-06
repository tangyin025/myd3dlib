#pragma once

class Material;

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
		PassMaskNone			= 0,
		PassMaskLight			= 1 << PassTypeLight,
		PassMaskOpaque			= 1 << PassTypeShadow | 1 << PassTypeNormal | 1 << PassTypeOpaque,
		PassMaskTransparent		= 1 << PassTypeTransparent,
	};

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

	my::EffectPtr m_SimpleSample;

	my::EffectPtr m_DofEffect;

	my::EffectPtr m_FxaaEffect;

	my::EffectPtr m_SsaoEffect;

	class IShaderSetter
	{
	public:
		virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId) = 0;
	};

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
		DWORD m_BkColor;

		my::CameraPtr m_Camera;

		my::CameraPtr m_SkyLightCam;

		my::Vector4 m_SkyLightDiffuse;

		my::Vector4 m_SkyLightAmbient;

		bool m_WireFrame;

		bool m_DofEnable;

		my::Vector4 m_DofParams;

		bool m_FxaaEnable;

		bool m_SsaoEnable;

		float m_SsaoBias;

		float m_SsaoIntensity;

		float m_SsaoRadius;

		float m_SsaoScale;

		my::Texture2DPtr m_NormalRT;

		my::Texture2DPtr m_PositionRT;

		my::Texture2DPtr m_LightRT;

		RTChain m_OpaqueRT;

		RTChain m_DownFilterRT;

		IRenderContext(void);

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
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
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
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<IndexedPrimitiveUPAtom> IndexedPrimitiveUPAtomList;

	struct MeshAtom
	{
		my::Mesh * mesh;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<MeshAtom> MeshAtomList;

	struct MeshInstanceAtom
	{
		IShaderSetter * setter;
		std::vector<D3DXATTRIBUTERANGE> m_AttribTable;
		std::vector<D3DVERTEXELEMENT9> m_velist;
		DWORD m_VertexStride;
		CComPtr<IDirect3DVertexDeclaration9> m_Decl;
		my::TransformList m_TransformList;
	};

	typedef boost::tuple<my::Mesh *, DWORD, my::Effect *> MeshInstanceAtomKey;

	typedef boost::unordered_map<MeshInstanceAtomKey, MeshInstanceAtom> MeshInstanceAtomMap;

	struct EmitterAtom
	{
		my::Emitter * emitter;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<EmitterAtom> EmitterAtomList;

	struct Pass
	{
		IndexedPrimitiveAtomList m_IndexedPrimitiveList;
		IndexedPrimitiveUPAtomList m_IndexedPrimitiveUPList;
		MeshAtomList m_MeshList;
		MeshInstanceAtomMap m_MeshInstanceMap;
		EmitterAtomList m_EmitterList;
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

	virtual my::Effect * QueryShader(MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID) = 0;

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

	void OnFrameRender(
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
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter);

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
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter);

	void DrawMesh(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void DrawMeshInstance(
		unsigned int PassID,
		IDirect3DDevice9 * pd3dDevice,
		my::Mesh * mesh,
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter,
		MeshInstanceAtom & atom);

	void DrawEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

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
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter);

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
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter);

	void PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter);

	void PushEmitter(unsigned int PassID, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);
};
