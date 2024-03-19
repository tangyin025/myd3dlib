#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include "myMath.h"
#include "mySingleton.h"
#include "myTexture.h"
#include "RenderPipeline.h"

namespace my
{
	class Effect;
};

class Actor;

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
		ParameterTypeInvWorldView
	};

	std::string m_Name;

	D3DXHANDLE m_Handle;

protected:
	MaterialParameter(void)
		: m_Name()
		, m_Handle(NULL)
	{
	}

	MaterialParameter(const std::string & Name)
		: m_Name(Name)
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
		ar & BOOST_SERIALIZATION_NVP(m_Name);
	}

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeNone;
	}

	bool operator == (const MaterialParameter & rhs) const;

	void Init(my::Effect * shader);

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor)
	{
	}

	virtual void RequestResource(void)
	{
	}

	virtual void ReleaseResource(void)
	{
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
		: MaterialParameter(Name)
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

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeFloat;
	}

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);
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
		: MaterialParameter(Name)
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

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeFloat2;
	}

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);
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
		: MaterialParameter(Name)
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

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeFloat3;
	}

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);
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
		: MaterialParameter(Name)
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

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeFloat4;
	}

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);
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
		: MaterialParameter(Name)
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

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeTexture;
	}

	void OnTextureReady(my::DeviceResourceBasePtr res);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);
};

class MaterialParameterInvWorldView : public MaterialParameter
{
protected:
	MaterialParameterInvWorldView(void)
	{
	}

public:
	MaterialParameterInvWorldView(const std::string & Name)
		: MaterialParameter(Name)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
	}

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeInvWorldView;
	}

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

class Material
	: public my::ShaderResourceBase
	, public boost::enable_shared_from_this<Material>
{
public:
	enum BlendMode
	{
		BlendModeNone = 0,
		BlendModeAlpha,
		BlendModeAdditive,
	};

	std::string m_Shader;

	enum PassMask
	{
		PassMaskNone = 0,
		PassMaskShadow = 1 << RenderPipeline::PassTypeShadow,
		PassMaskLight = 1 << RenderPipeline::PassTypeLight,
		PassMaskBackground = 1 << RenderPipeline::PassTypeBackground,
		PassMaskOpaque = 1 << RenderPipeline::PassTypeOpaque,
		PassMaskNormalOpaque = 1 << RenderPipeline::PassTypeNormal | 1 << RenderPipeline::PassTypeOpaque,
		PassMaskShadowNormalOpaque = 1 << RenderPipeline::PassTypeShadow | 1 << RenderPipeline::PassTypeNormal | 1 << RenderPipeline::PassTypeOpaque,
		PassMaskTransparent = 1 << RenderPipeline::PassTypeTransparent,
		PassMaskNormalTransparent = 1 << RenderPipeline::PassTypeNormalTransparent | 1 << RenderPipeline::PassTypeTransparent,
	};

	DWORD m_PassMask;

	DWORD m_CullMode;

	bool m_ZEnable;

	bool m_ZWriteEnable;

	DWORD m_ZFunc;

	DWORD m_BlendMode;

	typedef std::vector<MaterialParameterPtr> MaterialParameterPtrList;

	MaterialParameterPtrList m_ParameterList;

public:
	Material(void)
		: m_PassMask(0)
		, m_CullMode(D3DCULL_CW)
		, m_ZEnable(true)
		, m_ZWriteEnable(true)
		, m_ZFunc(D3DCMP_GREATEREQUAL)
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

	virtual MaterialPtr Clone(void) const;

	virtual void OnResetShader(void);

	void RequestResource(void);

	void ReleaseResource(void);

	void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC, Actor * actor);

	void ParseShaderParameters(void);

	template <typename T>
	void AddParameter(const std::string& Name, const T& Value);

	MaterialParameterPtr GetParameter(const std::string& Name);

	template <typename T>
	void SetParameter(const char* Name, const T& Value);
};

template <>
void Material::AddParameter<float>(const std::string& Name, const float& Value);

template <>
void Material::AddParameter<my::Vector2>(const std::string& Name, const my::Vector2& Value);

template <>
void Material::AddParameter<my::Vector3>(const std::string& Name, const my::Vector3& Value);

template <>
void Material::AddParameter<my::Vector4>(const std::string& Name, const my::Vector4& Value);

template <>
void Material::AddParameter<std::string>(const std::string& Name, const std::string& Value);

template <>
void Material::SetParameter<float>(const char* Name, const float& Value);

template <>
void Material::SetParameter<my::Vector2>(const char* Name, const my::Vector2& Value);

template <>
void Material::SetParameter<my::Vector3>(const char* Name, const my::Vector3& Value);

template <>
void Material::SetParameter<my::Vector4>(const char* Name, const my::Vector4& Value);

template <>
void Material::SetParameter<std::string>(const char* Name, const std::string& Value);
