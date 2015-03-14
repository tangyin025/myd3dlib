#pragma once

class Material;

class RenderPipeline
{
public:
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

	enum MeshType
	{
		MeshTypeStatic,
		MeshTypeAnimation,
		MeshTypeParticle,
	};

	enum DrawStage
	{
		DrawStageShadow,
		DrawStageNBuffer,
		DrawStageDBuffer,
		DrawStageCBuffer,
	};

	class IShaderSetter
	{
	public:
		virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;
	};

	struct MeshAtom
	{
		my::Mesh * mesh;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<MeshAtom> MeshAtomList;

	MeshAtomList m_OpaqueMeshList;

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

	MeshInstanceAtomMap m_OpaqueMeshInstanceMap;

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

	IndexedPrimitiveUPAtomList m_OpaqueIndexedPrimitiveUPList;

	struct EmitterAtom
	{
		my::Emitter * emitter;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<EmitterAtom> EmitterAtomList;

	EmitterAtomList m_QpaqueEmitterList;

public:
	virtual my::Effect * QueryShader(MeshType mesh_type, DrawStage draw_stage, bool bInstance, const Material * material) = 0;

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
		double fTime,
		float fElapsedTime);

	void DrawOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void DrawOpaqueMeshInstance(
		IDirect3DDevice9 * pd3dDevice,
		my::Mesh * mesh,
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter,
		MeshInstanceAtom & atom);

	void DrawOpaqueIndexedPrimitiveUP(
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

	void DrawOpaqueEmitter(IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushOpaqueMeshInstance(my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter);

	void PushOpaqueIndexedPrimitiveUP(
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

	void PushOpaqueEmitter(my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void ClearAllRenderObjs(void);
};

class Material
	: public my::DeviceRelatedObjectBase
{
public:
	std::pair<std::string, boost::shared_ptr<my::BaseTexture> > m_DiffuseTexture;

	std::pair<std::string, boost::shared_ptr<my::BaseTexture> > m_NormalTexture;

	std::pair<std::string, boost::shared_ptr<my::BaseTexture> > m_SpecularTexture;

public:
	Material(void)
	{
	}

	virtual void OnResetDevice(void)
	{
	}

	virtual void OnLostDevice(void)
	{
	}

	virtual void OnDestroyDevice(void)
	{
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_DiffuseTexture), m_DiffuseTexture.first);
		ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_NormalTexture), m_NormalTexture.first);
		ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_SpecularTexture), m_SpecularTexture.first);
	}

	virtual my::Effect * QueryShader(
		RenderPipeline * pipeline,
		RenderPipeline::DrawStage stage,
		RenderPipeline::MeshType mesh_type,
		bool bInstance);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;
