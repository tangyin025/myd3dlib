#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include "myMath.h"
#include "mySingleton.h"
#include "myTexture.h"

namespace my
{
	class Effect;
};

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
		ParameterTypeFloat3,
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

	MaterialParameter(ParameterType Type, const std::string & Name)
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

	bool operator == (const MaterialParameter & rhs) const;

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
		return MaterialParameterPtr(new MaterialParameter(m_Type, m_Name));
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
	MaterialParameterFloat(const std::string & Name, float Value)
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
		: m_Value(0, 0)
	{
	}

public:
	MaterialParameterFloat2(const std::string & Name, const my::Vector2 & Value)
		: MaterialParameter(ParameterTypeFloat2, Name)
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
		: m_Value(0, 0, 0)
	{
	}

public:
	MaterialParameterFloat3(const std::string & Name, const my::Vector3 & Value)
		: MaterialParameter(ParameterTypeFloat3, Name)
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
		: m_Value(0, 0, 0, 1)
	{
	}

public:
	MaterialParameterFloat4(const std::string & Name, const my::Vector4 & Value)
		: MaterialParameter(ParameterTypeFloat4, Name)
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
	friend class RenderPipeline;

	std::string m_TexturePath;

	my::BaseTexturePtr m_Texture;

protected:
	MaterialParameterTexture(void)
	{
	}

public:
	MaterialParameterTexture(const std::string & Name, const std::string & Path)
		: MaterialParameter(ParameterTypeTexture, Name)
		, m_TexturePath(Path)
	{
	}

	~MaterialParameterTexture(void);

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
		ar & BOOST_SERIALIZATION_NVP(m_TexturePath);
	}

	void OnTextureReady(my::DeviceResourceBasePtr res);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Set(my::Effect * shader);

	virtual MaterialParameterPtr Clone(void) const;
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

class Material : public boost::enable_shared_from_this<Material>
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

	bool operator == (const Material & rhs) const;

	void CopyFrom(const Material & rhs);

	virtual MaterialPtr Clone(void) const;

	void RequestResource(void);

	void ReleaseResource(void);

	void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	void ParseShaderParameters(void);

	void AddParameterFloat(const std::string & Name, float Value);

	void AddParameterFloat2(const std::string & Name, const my::Vector2 & Value);

	void AddParameterFloat3(const std::string & Name, const my::Vector3 & Value);

	void AddParameterFloat4(const std::string & Name, const my::Vector4 & Value);

	void AddParameterTexture(const std::string & Name, const std::string & Path);

	void SetParameterTexture(const std::string & Name, const std::string & Path);
};
