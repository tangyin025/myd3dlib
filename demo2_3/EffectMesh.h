#pragma once

class Material
{
public:
	my::EffectPtr m_Effect;

	D3DXHANDLE m_Technique;

	D3DXHANDLE m_Param;

public:
	Material(void)
		: m_Param(NULL)
	{
	}

	~Material(void)
	{
		SafeDeleteParam();
	}

	void SafeDeleteParam(void)
	{
		if(m_Param && m_Effect->m_ptr)
		{
			m_Effect->DeleteParameterBlock(m_Param);
		}
	}

	void BeginParameterBlock(const std::string & Technique)
	{
		SafeDeleteParam();

		m_Technique = m_Effect->GetTechniqueByName(Technique.c_str());

		m_Effect->SetTechnique(m_Technique);

		m_Effect->BeginParameterBlock();
	}

	void EndParameterBlock(void)
	{
		m_Param = m_Effect->EndParameterBlock();
	}

	void ApplyParameterBlock(void)
	{
		m_Effect->ApplyParameterBlock(m_Param);
	}

	UINT Begin(DWORD Flags = 0)
	{
		return m_Effect->Begin(Flags);
	}

	void End(void)
	{
		m_Effect->End();
	}

	void BeginPass(UINT Pass)
	{
		m_Effect->BeginPass(Pass);
	}

	void EndPass(void)
	{
		m_Effect->EndPass();
	}
};

typedef boost::shared_ptr<Material> MaterialPtr;

class EffectMesh
{
public:
	my::OgreMeshPtr m_Mesh;

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
