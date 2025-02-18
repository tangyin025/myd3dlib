// Copyright (c) 2011-2024 tangyin025
// License: MIT
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

class MaterialParameter;

typedef boost::shared_ptr<MaterialParameter> MaterialParameterPtr;

class MaterialParameter
{
public:
	friend class Material;

	Material* m_Owner;

	enum ParameterType
	{
		ParameterTypeNone = 0,
		ParameterTypeInt2,
		ParameterTypeFloat,
		ParameterTypeFloat2,
		ParameterTypeFloat3,
		ParameterTypeFloat4,
		ParameterTypeTexture,
		ParameterTypeInvWorldView,
		ParameterTypeFarZ
	};

	std::string m_Name;

	D3DXHANDLE m_Handle;

	bool m_Requested;

protected:
	MaterialParameter(void)
		: m_Owner(NULL)
		, m_Name()
		, m_Handle(NULL)
		, m_Requested(false)
	{
	}

	MaterialParameter(Material* Owner, const std::string& Name)
		: m_Owner(Owner)
		, m_Name(Name)
		, m_Handle(NULL)
		, m_Requested(false)
	{
	}

public:
	virtual ~MaterialParameter(void)
	{
		_ASSERT(!IsRequested());
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

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	bool operator == (const MaterialParameter & rhs) const;

	void Init(my::Effect * shader);

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
	{
	}

	virtual void RequestResource(void)
	{
		m_Requested = true;
	}

	virtual void ReleaseResource(void)
	{
		m_Requested = false;
	}
};

class MaterialParameterInt2 : public MaterialParameter
{
public:
	friend class Material;

	CPoint m_Value;

protected:
	MaterialParameterInt2(void)
		: m_Value(0, 0)
	{
	}

	MaterialParameterInt2(Material* Owner, const std::string& Name, const CPoint& Value)
		: MaterialParameter(Owner, Name)
		, m_Value(Value)
	{
	}

public:
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

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeInt2;
	}

	virtual void Set(my::Effect* shader, LPARAM lparam, RenderPipeline::IRenderContext* pRC);
};

class MaterialParameterFloat : public MaterialParameter
{
public:
	friend class Material;

	float m_Value;

protected:
	MaterialParameterFloat(void)
		: m_Value(0)
	{
	}

	MaterialParameterFloat(Material* Owner, const std::string & Name, float Value)
		: MaterialParameter(Owner, Name)
		, m_Value(Value)
	{
	}

public:
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

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);
};

class MaterialParameterFloat2 : public MaterialParameter
{
public:
	friend class Material;

	my::Vector2 m_Value;

protected:
	MaterialParameterFloat2(void)
		: m_Value(0, 0)
	{
	}

	MaterialParameterFloat2(Material * Owner, const std::string & Name, const my::Vector2 & Value)
		: MaterialParameter(Owner, Name)
		, m_Value(Value)
	{
	}

public:
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

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);
};

class MaterialParameterFloat3 : public MaterialParameter
{
public:
	friend class Material;

	my::Vector3 m_Value;

protected:
	MaterialParameterFloat3(void)
		: m_Value(0, 0, 0)
	{
	}

	MaterialParameterFloat3(Material * Owner, const std::string & Name, const my::Vector3 & Value)
		: MaterialParameter(Owner, Name)
		, m_Value(Value)
	{
	}

public:
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

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);
};

class MaterialParameterFloat4 : public MaterialParameter
{
public:
	friend class Material;

	my::Vector4 m_Value;

protected:
	MaterialParameterFloat4(void)
		: m_Value(0, 0, 0, 1)
	{
	}

	MaterialParameterFloat4(Material * Owner, const std::string & Name, const my::Vector4 & Value)
		: MaterialParameter(Owner, Name)
		, m_Value(Value)
	{
	}

public:
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

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);
};

class MaterialParameterTexture : public MaterialParameter
{
public:
	friend class Material;

	std::string m_TexturePath;

	my::BaseTexturePtr m_Texture;

protected:
	MaterialParameterTexture(void)
	{
	}

	MaterialParameterTexture(Material * Owner, const std::string & Name, const std::string & Path)
		: MaterialParameter(Owner, Name)
		, m_TexturePath(Path)
	{
	}

public:
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

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);
};

class MaterialParameterInvWorldView : public MaterialParameter
{
protected:
	friend class Material;

	MaterialParameterInvWorldView(void)
	{
	}

	MaterialParameterInvWorldView(Material * Owner, const std::string & Name)
		: MaterialParameter(Owner, Name)
	{
	}

public:
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

	virtual void Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);
};

class MaterialParameterFarZ : public MaterialParameter
{
protected:
	friend class Material;

	MaterialParameterFarZ(void)
	{
	}

	MaterialParameterFarZ(Material* Owner, const std::string& Name)
		: MaterialParameter(Owner, Name)
	{
	}

public:
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
	}

	virtual ParameterType GetParameterType(void) const
	{
		return ParameterTypeFarZ;
	}

	virtual void Set(my::Effect* shader, LPARAM lparam, RenderPipeline::IRenderContext* pRC);
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

class Material
	: public my::ShaderResourceBase
	, public boost::enable_shared_from_this<Material>
{
public:
	Component* m_Cmp;

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

	bool m_AlphaTestEnable;

	DWORD m_AlphaRef;

	DWORD m_AlphaFunc;

	DWORD m_BlendMode;

	typedef std::vector<MaterialParameterPtr> MaterialParameterPtrList;

	MaterialParameterPtrList m_ParameterList;

public:
	Material(void)
		: m_Cmp(NULL)
		, m_PassMask(0)
		, m_CullMode(D3DCULL_CW)
		, m_ZEnable(true)
		, m_ZWriteEnable(true)
		, m_ZFunc(D3DCMP_GREATEREQUAL)
		, m_AlphaTestEnable(false)
		, m_AlphaRef(0x00000001)
		, m_AlphaFunc(D3DCMP_GREATEREQUAL)
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

	void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC);

	void ParseShaderParameters(void);

	MaterialParameter * GetParameter(const char* Name) const;

	template <typename T>
	void SetParameter(const char* Name, const T& Value);
};

template <>
void Material::SetParameter<CPoint>(const char* Name, const CPoint& Value);

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
