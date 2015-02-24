#pragma once

class Material;

class RenderPipeline
{
public:
	enum MeshType
	{
		MeshTypeStatic,
		MeshTypeAnimation,
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

public:
	virtual my::Effect * QueryShader(MeshType mesh_type, DrawStage draw_stage, bool bInstance, const Material * material) = 0;

	void OnRender(IDirect3DDevice9 * pd3dDevice, double fTime, float fElapsedTime);

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

	void ClearAllOpaqueObjs(void);
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

	virtual void OnQueryMesh(
		RenderPipeline * pipeline,
		RenderPipeline::DrawStage stage,
		RenderPipeline::MeshType mesh_type,
		my::MeshInstance * mesh,
		DWORD AttribId,
		RenderPipeline::IShaderSetter * setter);

	virtual void OnQueryMeshInstance(
		RenderPipeline * pipeline,
		RenderPipeline::DrawStage stage,
		RenderPipeline::MeshType mesh_type,
		my::MeshInstance * mesh,
		DWORD AttribId,
		const my::Matrix4 & World,
		RenderPipeline::IShaderSetter * setter);

	virtual void OnQueryIndexedPrimitiveUP(
		RenderPipeline * pipeline,
		RenderPipeline::DrawStage stage,
		RenderPipeline::MeshType mesh_type,
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
		RenderPipeline::IShaderSetter * setter);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;
