#pragma once

#include <boost/serialization/nvp.hpp>

class RenderPipeline;

class Material
	: public my::DeviceRelatedObjectBase
{
public:
	class ParameterValue
	{
	public:
		enum ParameterValueType
		{
			ParameterValueTypeUnknown,
			ParameterValueTypeTexture,
		};

		ParameterValueType m_Type;

	public:
		ParameterValue(ParameterValueType type)
			: m_Type(type)
		{
		}

		virtual ~ParameterValue(void)
		{
		}

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId, const char * name) = 0;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_Type);
		}
	};

	typedef boost::shared_ptr<ParameterValue> ParameterValuePtr;

	class ParameterValueTexture : public ParameterValue
	{
	public:
		std::string m_Path;

		my::Texture2DPtr m_Texture;

	public:
		ParameterValueTexture(void)
			: ParameterValue(ParameterValueTypeTexture)
		{
		}

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId, const char * name);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ParameterValue);
			ar & BOOST_SERIALIZATION_NVP(m_Path);
		}
	};

	typedef boost::shared_ptr<ParameterValueTexture> ParameterValueTexturePtr;

	class Parameter : public std::pair<std::string, boost::shared_ptr<ParameterValue> >
	{
	public:
		Parameter(void)
		{
		}

		Parameter(const std::string & name, ParameterValuePtr value)
			: pair(name, value)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(first);
			ar & BOOST_SERIALIZATION_NVP(second);
		}
	};

	typedef std::vector<Parameter> ParameterList;

	std::string m_Shader;

	ParameterList m_Params;

	unsigned int m_PassMask;

public:
	Material(void);

	virtual ~Material(void);

	virtual void OnResetDevice(void)
	{
	}

	virtual void OnLostDevice(void)
	{
	}

	virtual void OnDestroyDevice(void)
	{
	}

	void AddParameter(const std::string & name, ParameterValuePtr value)
	{
		m_Params.push_back(Parameter(name, value));
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Shader);
		ar & BOOST_SERIALIZATION_NVP(m_Params);
		ar & BOOST_SERIALIZATION_NVP(m_PassMask);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;

class RenderPipeline
{
public:
	enum MeshType
	{
		MeshTypeStatic			= 0,
		MeshTypeAnimation		= 1,
		MeshTypeParticle		= 2,
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
		PassMaskTransparent		= 1 << PassTypeShadow | 1 << PassTypeNormal | 1 << PassTypeTransparent,
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

	my::Texture2DPtr m_NormalRT;

	my::Texture2DPtr m_PositionRT;

	my::Texture2DPtr m_LightRT;

	my::Texture2DPtr m_OpaqueRT;

	my::Texture2DPtr m_DownFilterRT;

	my::Texture2DPtr m_DownFilterRT2;

	my::FirstPersonCamera m_Camera;

	my::OrthoCamera m_SkyLight;

	my::Vector4 m_SkyLightColor;

	my::EffectPtr m_SimpleSample;

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
		MeshAtomList m_MeshList;
		MeshInstanceAtomMap m_MeshInstanceMap;
		IndexedPrimitiveUPAtomList m_IndexedPrimitiveUPList;
		EmitterAtomList m_EmitterList;
	};

	boost::array<Pass, PassTypeNum> m_Pass;

public:
	RenderPipeline(void);

	virtual ~RenderPipeline(void);

	virtual my::Effect * QueryShader(MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID) = 0;

	virtual void QueryComponent(const my::Frustum & frustum, unsigned int PassMask) = 0;

	static unsigned int PassTypeToMask(unsigned int pass_type)
	{
		_ASSERT(pass_type >= 0 && pass_type < PassTypeNum); return 1 << pass_type;
	}

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
		double fTime,
		float fElapsedTime);

	void RenderAllObjects(
		unsigned int PassID,
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	void ClearAllObjects(void);

	void DrawMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void DrawMeshInstance(
		unsigned int PassID,
		IDirect3DDevice9 * pd3dDevice,
		my::Mesh * mesh,
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter,
		MeshInstanceAtom & atom);

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

	void DrawEmitter(unsigned int PassID, IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter);

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

	void PushEmitter(unsigned int PassID, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	template <class ComponentClass>
	void PushComponent(ComponentClass * cmp, MeshType mesh_type, unsigned int PassMask);
};
