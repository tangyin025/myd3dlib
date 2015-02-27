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

	struct OpaqueMesh
	{
		my::MeshInstance * mesh;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;

		bool operator ==(const OpaqueMesh & rhs) const
		{
			return mesh == rhs.mesh
				&& AttribId == rhs.AttribId
				&& shader == rhs.shader
				&& setter == rhs.setter;
		}
	};

	typedef std::vector<OpaqueMesh> OpaqueMeshList;

	OpaqueMeshList m_OpaqueMeshList;

	typedef boost::unordered_map<OpaqueMesh, my::TransformList> OpaqueMeshInstanceMap;

	OpaqueMeshInstanceMap m_OpaqueMeshInstanceMap;

	struct OpaqueIndexedPrimitiveUP
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

	typedef std::vector<OpaqueIndexedPrimitiveUP> OpaqueIndexedPrimitiveUPList;

	OpaqueIndexedPrimitiveUPList m_OpaqueIndexedPrimitiveUPList;

	struct EmitterAtom
	{
		my::Emitter * emitter;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<EmitterAtom> EmitterAtomList;

	EmitterAtomList m_EmitterAtomList;

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

	void DrawOpaqueMesh(my::MeshInstance * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void DrawOpaqueMeshInstance(my::MeshInstance * mesh, DWORD AttribId, const my::TransformList & worlds, my::Effect * shader, IShaderSetter * setter);

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

	void DrawEmitterAtom(IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, my::Effect * shader, IShaderSetter * setter);

	void PushOpaqueMesh(my::MeshInstance * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushOpaqueMeshInstance(my::MeshInstance * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter);

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

	void PushEmitter(my::Emitter * emitter, my::Effect * shader, IShaderSetter * setter);

	void ClearAllRenderObjs(void);
};

class Material
	: public my::DeviceRelatedObjectBase
{
public:
	boost::shared_ptr<my::BaseTexture> m_DiffuseTexture;

	boost::shared_ptr<my::BaseTexture> m_NormalTexture;

	boost::shared_ptr<my::BaseTexture> m_SpecularTexture;

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

	virtual my::Effect * QueryShader(
		RenderPipeline * pipeline,
		RenderPipeline::DrawStage stage,
		RenderPipeline::MeshType mesh_type,
		bool bInstance);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;
