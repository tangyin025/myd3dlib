#pragma once

class Material
{
public:
	my::EffectPtr m_Effect;

	D3DXHANDLE m_Param;

public:
	Material(void)
		: m_Param(NULL)
	{
	}

	~Material(void)
	{
		//m_Effect->DeleteParameterBlock(m_Param);
	}

	void SetTexture(const std::string & param, my::TexturePtr texture)
	{
		m_Effect->SetTexture(param.c_str(), texture->m_ptr);
	}

	void BeginParameterBlock(void)
	{
		_ASSERT(NULL == m_Param);

		m_Effect->BeginParameterBlock();
	}

	void EndParameterBlock(void)
	{
		m_Param = m_Effect->EndParameterBlock();
	}
};

typedef boost::shared_ptr<Material> MaterialPtr;

class EffectMesh
{
public:
	my::MeshPtr m_Mesh;

	typedef std::vector<MaterialPtr> MaterialPtrList;

	MaterialPtrList m_materials;

public:
	EffectMesh(void)
	{
	}

	virtual ~EffectMesh(void)
	{
	}

	void InsertMaterial(MaterialPtr material)
	{
		m_materials.push_back(material);
	}

	void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);
};

typedef boost::shared_ptr<EffectMesh> EffectMeshPtr;
