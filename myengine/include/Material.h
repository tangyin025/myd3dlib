#pragma once

#include <boost/shared_ptr.hpp>
#include "myMath.h"
#include "ResourceBundle.h"

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

	my::Vector4 m_MeshColor;

	my::Vector2 m_RepeatUV;

	ResourceBundle<my::BaseTexture> m_MeshTexture;

	ResourceBundle<my::BaseTexture> m_NormalTexture;

	ResourceBundle<my::BaseTexture> m_SpecularTexture;

	ResourceBundle<my::BaseTexture> m_ReflectTexture;

public:
	Material(void)
		: m_PassMask(0)
		, m_CullMode(D3DCULL_CW)
		, m_ZEnable(TRUE)
		, m_ZWriteEnable(TRUE)
		, m_BlendMode(BlendModeNone)
		, m_MeshColor(1, 1, 1, 1)
		, m_RepeatUV(1, 1)
	{
	}

	virtual ~Material(void)
	{
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Shader);
		ar & BOOST_SERIALIZATION_NVP(m_PassMask);
		ar & BOOST_SERIALIZATION_NVP(m_CullMode);
		ar & BOOST_SERIALIZATION_NVP(m_ZEnable);
		ar & BOOST_SERIALIZATION_NVP(m_ZWriteEnable);
		ar & BOOST_SERIALIZATION_NVP(m_BlendMode);
		ar & BOOST_SERIALIZATION_NVP(m_MeshColor);
		ar & BOOST_SERIALIZATION_NVP(m_RepeatUV);
		ar & BOOST_SERIALIZATION_NVP(m_MeshTexture);
		ar & BOOST_SERIALIZATION_NVP(m_NormalTexture);
		ar & BOOST_SERIALIZATION_NVP(m_SpecularTexture);
		ar & BOOST_SERIALIZATION_NVP(m_ReflectTexture);
	}

	void CopyFrom(const Material & rhs);

	virtual MaterialPtr Clone(void) const;

	void RequestResource(void);

	void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);
};
