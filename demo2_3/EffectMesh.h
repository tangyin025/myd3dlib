#pragma once

class EffectMesh
{
public:
	my::OgreMeshPtr m_Mesh;

	typedef std::vector<my::MaterialPtr> MaterialPtrList;

	MaterialPtrList m_materials;

public:
	EffectMesh(void)
	{
	}

	virtual ~EffectMesh(void)
	{
	}

	void InsertMaterial(my::MaterialPtr material)
	{
		m_materials.push_back(material);
	}

	void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

	void DrawSubset(DWORD i, my::Effect * pEffect);
};

typedef boost::shared_ptr<EffectMesh> EffectMeshPtr;
