#pragma once

class BaseParameter
{
public:
	std::string name;

	BaseParameter(const std::string & _name)
		: name(_name)
	{
	}

	virtual ~BaseParameter(void)
	{
	}

	virtual void SetParameter(my::Effect * effect) const = 0;
};

template <class ParameterType>
class Parameter : public BaseParameter
{
public:
	ParameterType value;

	Parameter(const std::string & _name, const ParameterType & _value)
		: BaseParameter(_name)
		, value(_value)
	{
	}

	virtual void SetParameter(my::Effect * effect) const;
};

typedef boost::shared_ptr<BaseParameter> BaseParameterPtr;

class ParameterMap : public std::map<std::string, BaseParameterPtr>
{
public:
	void AddBool(const std::string & name, bool value);

	void AddFloat(const std::string & name, float value);

	void AddInt(const std::string & name, int value);

	void AddVector(const std::string & name, const my::Vector4 & value);

	void AddMatrix(const std::string & name, const my::Matrix4 & value);

	void AddString(const std::string & name, const std::string & value);

	void AddTexture(const std::string & name, my::BaseTexturePtr value);
};

class Material : public ParameterMap
{
public:
	my::EffectPtr m_Effect;

public:
	Material(void)
	{
	}

	~Material(void)
	{
	}

	void ApplyParameterBlock(void);

	UINT Begin(DWORD Flags = 0);

	void End(void);

	void BeginPass(UINT Pass);

	void EndPass(void);
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
