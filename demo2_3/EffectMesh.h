#pragma once

class Material
{
public:
	my::EffectPtr m_Effect;

	D3DXHANDLE m_Param;

public:
	Material(void)
	{
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
