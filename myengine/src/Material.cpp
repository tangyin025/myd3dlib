#include "Material.h"
#include "myEffect.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "RenderPipeline.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(MaterialParameter)

BOOST_CLASS_EXPORT(MaterialParameterFloat)

BOOST_CLASS_EXPORT(MaterialParameterFloat2)

BOOST_CLASS_EXPORT(MaterialParameterFloat3)

BOOST_CLASS_EXPORT(MaterialParameterFloat4)

BOOST_CLASS_EXPORT(MaterialParameterTexture)

BOOST_CLASS_EXPORT(Material)

bool MaterialParameter::operator == (const MaterialParameter & rhs) const
{
	if (m_Type == rhs.m_Type && m_Name == rhs.m_Name)
	{
		switch (m_Type)
		{
		case ParameterTypeNone:
			return true;
		case ParameterTypeFloat:
			return static_cast<const MaterialParameterFloat &>(*this).m_Value == static_cast<const MaterialParameterFloat &>(rhs).m_Value;
		case ParameterTypeFloat2:
			return static_cast<const MaterialParameterFloat2 &>(*this).m_Value == static_cast<const MaterialParameterFloat2 &>(rhs).m_Value;
		case ParameterTypeFloat3:
			return static_cast<const MaterialParameterFloat3 &>(*this).m_Value == static_cast<const MaterialParameterFloat3 &>(rhs).m_Value;
		case ParameterTypeFloat4:
			return static_cast<const MaterialParameterFloat4 &>(*this).m_Value == static_cast<const MaterialParameterFloat4 &>(rhs).m_Value;
		case ParameterTypeTexture:
			return static_cast<const MaterialParameterTexture &>(*this).m_TexturePath == static_cast<const MaterialParameterTexture &>(rhs).m_TexturePath;
		}
	}
	return false;
}

void MaterialParameter::Init(my::Effect * shader)
{
	m_Handle = shader->GetParameterByName(NULL, m_Name.c_str());
}

void MaterialParameterFloat::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloat(m_Handle, m_Value);
}

MaterialParameterPtr MaterialParameterFloat::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat>(new MaterialParameterFloat(m_Name, m_Value));
}

void MaterialParameterFloat2::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 2);
}

MaterialParameterPtr MaterialParameterFloat2::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat2>(new MaterialParameterFloat2(m_Name, m_Value));
}

void MaterialParameterFloat3::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 3);
}

MaterialParameterPtr MaterialParameterFloat3::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat3>(new MaterialParameterFloat3(m_Name, m_Value));
}

void MaterialParameterFloat4::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 4);
}

MaterialParameterPtr MaterialParameterFloat4::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat4>(new MaterialParameterFloat4(m_Name, m_Value));
}

void MaterialParameterTexture::OnReady(my::DeviceResourceBasePtr res)
{
	m_Texture = boost::dynamic_pointer_cast<my::BaseTexture>(res);
}

void MaterialParameterTexture::RequestResource(void)
{
	if (!m_TexturePath.empty())
	{
		_ASSERT(!m_Texture);

		my::ResourceMgr::getSingleton().LoadTextureAsync(m_TexturePath.c_str(), this);
	}
}

void MaterialParameterTexture::ReleaseResource(void)
{
	if (!m_TexturePath.empty() && !m_Texture)
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_TexturePath, this);

		m_Texture.reset();
	}
}

void MaterialParameterTexture::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetTexture(m_Handle, m_Texture.get());
}

MaterialParameterPtr MaterialParameterTexture::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterTexture>(new MaterialParameterTexture(m_Name, m_TexturePath));
}

template<>
void Material::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Shader);
	ar << BOOST_SERIALIZATION_NVP(m_PassMask);
	ar << BOOST_SERIALIZATION_NVP(m_CullMode);
	ar << BOOST_SERIALIZATION_NVP(m_ZEnable);
	ar << BOOST_SERIALIZATION_NVP(m_ZWriteEnable);
	ar << BOOST_SERIALIZATION_NVP(m_BlendMode);
	ar << BOOST_SERIALIZATION_NVP(m_ParameterList);
}

template<>
void Material::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Shader);
	ar >> BOOST_SERIALIZATION_NVP(m_PassMask);
	ar >> BOOST_SERIALIZATION_NVP(m_CullMode);
	ar >> BOOST_SERIALIZATION_NVP(m_ZEnable);
	ar >> BOOST_SERIALIZATION_NVP(m_ZWriteEnable);
	ar >> BOOST_SERIALIZATION_NVP(m_BlendMode);
	ar >> BOOST_SERIALIZATION_NVP(m_ParameterList);
}

bool Material::operator == (const Material & rhs) const
{
	if (m_PassMask == rhs.m_PassMask
		&& m_CullMode == rhs.m_CullMode
		&& m_ZEnable == rhs.m_ZEnable
		&& m_ZWriteEnable == rhs.m_ZWriteEnable
		&& m_BlendMode == rhs.m_BlendMode
		&& m_ParameterList.size() == rhs.m_ParameterList.size())
	{
		for (unsigned int i = 0; i < m_ParameterList.size(); i++)
		{
			if (*m_ParameterList[i] == *rhs.m_ParameterList[i])
			{
				continue;
			}
			return false;
		}
		return true;
	}
	return false;
}

void Material::CopyFrom(const Material & rhs)
{
	m_Shader = rhs.m_Shader;
	m_PassMask = rhs.m_PassMask;
	m_CullMode = rhs.m_CullMode;
	m_ZEnable = rhs.m_ZEnable;
	m_ZWriteEnable = rhs.m_ZWriteEnable;
	m_BlendMode = rhs.m_BlendMode;
	m_ParameterList.clear();
	MaterialParameterPtrList::const_iterator param_iter = rhs.m_ParameterList.begin();
	for (; param_iter != rhs.m_ParameterList.end(); param_iter++)
	{
		m_ParameterList.push_back((*param_iter)->Clone());
	}
}

MaterialPtr Material::Clone(void) const
{
	MaterialPtr ret(new Material());
	ret->CopyFrom(*this);
	return ret;
}

void Material::RequestResource(void)
{
	MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		(*param_iter)->RequestResource();
	}
}

void Material::ReleaseResource(void)
{
	MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		(*param_iter)->ReleaseResource();
	}
}

void Material::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	HRESULT hr;
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, m_CullMode));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, m_ZEnable));
	V(pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, m_ZWriteEnable));
	switch (m_BlendMode)
	{
	case BlendModeAlpha:
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
		V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
		break;
	case BlendModeAdditive:
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
		V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR));
		V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		break;
	default:
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		break;
	}

	if (!m_ParameterList.empty() && !m_ParameterList.front()->m_Handle)
	{
		MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
		for (; param_iter != m_ParameterList.end(); param_iter++)
		{
			(*param_iter)->Init(shader);
		}
	}

	MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		(*param_iter)->Set(shader);
	}
}

void Material::ParseShaderParameters(void)
{
	if (m_Shader.empty())
	{
		return;
	}

	CachePtr cache = my::ResourceMgr::getSingleton().OpenIStream(m_Shader.c_str())->GetWholeCache();
	cache->push_back(0);
	boost::regex reg_type("(float\\d?|texture)\\s+(\\w+)\\s*:\\s*MaterialParameter(\\s*<[^>]+>)?(\\s*=\\s*[^;]+)?;");
	boost::match_results<const char *> what;
	const char * start = (const char *)&(*cache)[0];
	const char * end = (const char *)&(*cache)[cache->size() - 1];
	m_ParameterList.clear();
	while (boost::regex_search(start, end, what, reg_type, boost::match_default))
	{
		std::string Type = what[1];
		std::string Name = what[2];
		std::string Annotations = what[3];
		std::string Initialize = what[4];
		if (Type == "float")
		{
			float Value = 0.0f;
			boost::regex reg_value("-?\\d+(\\.\\d+)?");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
			{
				Value = boost::lexical_cast<float>(what2[0]);
			}
			AddParameterFloat(Name, Value);
		}
		else if (Type == "float2")
		{
			Vector2 Value(0, 0);
			boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
			{
				Value.x = boost::lexical_cast<float>(what2[1]);
				Value.y = boost::lexical_cast<float>(what2[3]);
			}
			AddParameterFloat2(Name, Value);
		}
		else if (Type == "float3")
		{
			Vector3 Value(0, 0, 0);
			boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
			{
				Value.x = boost::lexical_cast<float>(what2[1]);
				Value.y = boost::lexical_cast<float>(what2[3]);
				Value.z = boost::lexical_cast<float>(what2[5]);
			}
			AddParameterFloat3(Name, Value);
		}
		else if (Type == "float4")
		{
			Vector4 Value(0, 0, 0, 1);
			boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
			{
				Value.x = boost::lexical_cast<float>(what2[1]);
				Value.y = boost::lexical_cast<float>(what2[3]);
				Value.z = boost::lexical_cast<float>(what2[5]);
				Value.w = boost::lexical_cast<float>(what2[7]);
			}
			AddParameterFloat4(Name, Value);
		}
		else if (Type == "texture")
		{
			std::string Path;
			boost::regex reg_value("string\\s+Initialize\\s*=\\s*\\\"([^\"]+)\\\"");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Annotations, what2, reg_value, boost::match_default))
			{
				Path = what2[1];
			}
			AddParameterTexture(Name, Path);
		}
		start = what[0].second;
	}

	boost::regex reg_pass("pass\\s+(\\w+)(\\s*<[^>]+>)?\\s*{\\s*(\\w*)[^}]*}");
	start = (const char *)&(*cache)[0];
	end = (const char *)&(*cache)[cache->size() - 1];
	m_PassMask = 0;
	while (boost::regex_search(start, end, what, reg_pass, boost::match_default))
	{
		std::string Pass = what[1];
		std::string assignment = what[3];
		if (Pass == "PassTypeShadow")
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeShadow;
			}
		}
		else if (Pass == "PassTypeNormal")
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeNormal;
			}
		}
		else if (Pass == "PassTypeLight")
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeLight;
			}
		}
		else if (Pass == "PassTypeOpaque")
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeOpaque;
			}
		}
		else if (Pass == "PassTypeTransparent")
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeTransparent;
			}
		}
		start = what[0].second;
	}
}

void Material::AddParameterFloat(const std::string & Name, float Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat(Name, Value)));
}

void Material::AddParameterFloat2(const std::string & Name, const my::Vector2 & Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat2(Name, Value)));
}

void Material::AddParameterFloat3(const std::string & Name, const my::Vector3 & Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat3(Name, Value)));
}

void Material::AddParameterFloat4(const std::string & Name, const my::Vector4 & Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat4(Name, Value)));
}

void Material::AddParameterTexture(const std::string & Name, const std::string & Path)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterTexture(Name, Path)));
}

void Material::SetParameterTexture(const std::string & Name, const std::string & Path)
{
	MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		if ((*param_iter)->m_Name == Name && (*param_iter)->m_Type == MaterialParameter::ParameterTypeTexture)
		{
			// ! SetParameterTexture must be call outside resource request
			_ASSERT(!boost::dynamic_pointer_cast<MaterialParameterTexture>(*param_iter)->IsRequested());
			_ASSERT(!boost::dynamic_pointer_cast<MaterialParameterTexture>(*param_iter)->m_Texture);
			boost::dynamic_pointer_cast<MaterialParameterTexture>(*param_iter)->m_TexturePath = Path;
		}
	}
}
