#pragma once

#include <boost/shared_ptr.hpp>
#include "myMath.h"
#include "ResourceBundle.h"

class MaterialParameter;

typedef boost::shared_ptr<MaterialParameter> MaterialParameterPtr;

class MaterialParameter
{
public:
	enum ParameterType
	{
		ParameterTypeNone = 0,
		ParameterTypeFloat,
		ParameterTypeFloat2,
		ParameterTypeFLoat3,
		ParameterTypeFloat4,
		ParameterTypeTexture,
	};

	ParameterType m_Type;

	std::string m_Name;

	D3DXHANDLE m_Handle;

protected:
	MaterialParameter(void)
		: m_Type(ParameterTypeNone)
		, m_Name()
		, m_Handle(NULL)
	{
	}

	MaterialParameter(ParameterType Type, const char * Name)
		: m_Type(Type)
		, m_Name(Name)
		, m_Handle(NULL)
	{
	}

public:
	virtual ~MaterialParameter(void)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Type);
		ar & BOOST_SERIALIZATION_NVP(m_Name);
	}

	void Init(my::Effect * shader);

	virtual void Set(my::Effect * shader)
	{
	}

	virtual void RequestResource(void)
	{
	}

	virtual void ReleaseResource(void)
	{
	}

	virtual MaterialParameterPtr Clone(void) const
	{
		return MaterialParameterPtr(new MaterialParameter(m_Type, m_Name.c_str()));
	}
};

class MaterialParameterFloat : public MaterialParameter
{
public:
	float m_Value;

protected:
	MaterialParameterFloat(void)
		: m_Value(0)
	{
	}

public:
	MaterialParameterFloat(const char * Name, float Value)
		: MaterialParameter(ParameterTypeFloat, Name)
		, m_Value(Value)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
		ar & BOOST_SERIALIZATION_NVP(m_Value);
	}

	virtual void Set(my::Effect * shader);

	virtual MaterialParameterPtr Clone(void) const;
};

class MaterialParameterFloat2 : public MaterialParameter
{
public:
	my::Vector2 m_Value;

protected:
	MaterialParameterFloat2(void)
		: m_Value(0,0)
	{
	}

public:
	MaterialParameterFloat2(const char * Name, const my::Vector2 & Value)
		: MaterialParameter(ParameterTypeFloat, Name)
		, m_Value(Value)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
		ar & BOOST_SERIALIZATION_NVP(m_Value);
	}

	virtual void Set(my::Effect * shader);

	virtual MaterialParameterPtr Clone(void) const;
};

class MaterialParameterFloat3 : public MaterialParameter
{
public:
	my::Vector3 m_Value;

protected:
	MaterialParameterFloat3(void)
		: m_Value(0, 0)
	{
	}

public:
	MaterialParameterFloat3(const char * Name, const my::Vector3 & Value)
		: MaterialParameter(ParameterTypeFloat, Name)
		, m_Value(Value)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
		ar & BOOST_SERIALIZATION_NVP(m_Value);
	}

	virtual void Set(my::Effect * shader);

	virtual MaterialParameterPtr Clone(void) const;
};

class MaterialParameterFloat4 : public MaterialParameter
{
public:
	my::Vector4 m_Value;

protected:
	MaterialParameterFloat4(void)
		: m_Value(0, 0)
	{
	}

public:
	MaterialParameterFloat4(const char * Name, const my::Vector4 & Value)
		: MaterialParameter(ParameterTypeFloat, Name)
		, m_Value(Value)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
		ar & BOOST_SERIALIZATION_NVP(m_Value);
	}

	virtual void Set(my::Effect * shader);

	virtual MaterialParameterPtr Clone(void) const;
};

class MaterialParameterTexture : public MaterialParameter
{
public:
	ResourceBundle<my::BaseTexture> m_Texture;

protected:
	MaterialParameterTexture(void)
	{
	}

public:
	MaterialParameterTexture(const char * Name, const char * Path)
		: MaterialParameter(ParameterTypeTexture, Name)
		, m_Texture(Path)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
		ar & BOOST_SERIALIZATION_NVP(m_Texture);
	}

	virtual void Set(my::Effect * shader);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual MaterialParameterPtr Clone(void) const;
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

class Material
{
public:
	enum BlendMode
	{
		BlendModeNone = 0,
		BlendModeAlpha,
		BlendModeAdditive,
	};

	std::string m_Shader;

	DWORD m_PassMask;

	DWORD m_CullMode;

	BOOL m_ZEnable;

	BOOL m_ZWriteEnable;

	DWORD m_BlendMode;

	typedef std::vector<MaterialParameterPtr> MaterialParameterPtrList;

	MaterialParameterPtrList m_ParameterList;

public:
	Material(void)
		: m_PassMask(0)
		, m_CullMode(D3DCULL_CW)
		, m_ZEnable(TRUE)
		, m_ZWriteEnable(TRUE)
		, m_BlendMode(BlendModeNone)
	{
	}

	virtual ~Material(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void CopyFrom(const Material & rhs);

	virtual MaterialPtr Clone(void) const;

	void RequestResource(void);

	void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);

	void ParseShaderParamters(void);

	void AddParameterFloat(const char * Name, float Value);

	void AddParameterFloat2(const char * Name, const my::Vector2 & Value);

	void AddParameterFloat3(const char * Name, const my::Vector3 & Value);

	void AddParameterFloat4(const char * Name, const my::Vector4 & Value);

	void AddParameterTexture(const char * Name, const char * Path);
};
