#include "Material.h"
#include "myEffect.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "RenderPipeline.h"
#include "libc.h"
#include "Actor.h"
#include <fstream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
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
	if (GetParameterType() == rhs.GetParameterType() && m_Name == rhs.m_Name)
	{
		switch (GetParameterType())
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

void MaterialParameterFloat2::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 2);
}

void MaterialParameterFloat3::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 3);
}

void MaterialParameterFloat4::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 4);
}

MaterialParameterTexture::~MaterialParameterTexture(void)
{
	if (!m_TexturePath.empty())
	{
		_ASSERT(!m_Texture); ReleaseResource();
	}
}

void MaterialParameterTexture::OnTextureReady(my::DeviceResourceBasePtr res)
{
	m_Texture = boost::dynamic_pointer_cast<my::BaseTexture>(res);
}

void MaterialParameterTexture::RequestResource(void)
{
	if (!m_TexturePath.empty())
	{
		_ASSERT(!m_Texture);

		my::ResourceMgr::getSingleton().LoadTextureAsync(m_TexturePath.c_str(), boost::bind(&MaterialParameterTexture::OnTextureReady, this, boost::placeholders::_1));
	}
}

void MaterialParameterTexture::ReleaseResource(void)
{
	if (!m_TexturePath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_TexturePath, boost::bind(&MaterialParameterTexture::OnTextureReady, this, boost::placeholders::_1));

		m_Texture.reset();
	}
}

void MaterialParameterTexture::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetTexture(m_Handle, m_Texture.get());
}

template<class Archive>
void Material::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Shader);
	ar << BOOST_SERIALIZATION_NVP(m_PassMask);
	ar << BOOST_SERIALIZATION_NVP(m_CullMode);
	ar << BOOST_SERIALIZATION_NVP(m_ZEnable);
	ar << BOOST_SERIALIZATION_NVP(m_ZWriteEnable);
	ar << BOOST_SERIALIZATION_NVP(m_BlendMode);
	ar << BOOST_SERIALIZATION_NVP(m_ParameterList);
}

template<class Archive>
void Material::load(Archive & ar, const unsigned int version)
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

MaterialPtr Material::Clone(void) const
{
	std::stringstream sstr;
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(sstr, ".txt");
	*oa << boost::serialization::make_nvp(__FUNCTION__, shared_from_this());

	MaterialPtr ret(new Material());
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(sstr, ".txt", "");
	*ia >> boost::serialization::make_nvp(__FUNCTION__, ret);
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
	//                     12       3        4        5        6             7                               8             9
	boost::regex reg("^\\s*((float)|(float2)|(float3)|(float4)|(texture))\\s+(\\w+)\\s*:\\s*MaterialParameter(\\s*<[^>]+>)?(\\s*=\\s*[^;]+)?;");
	boost::match_results<const char *> what;
	const char * start = (const char *)&(*cache)[0];
	const char * end = (const char *)&(*cache)[cache->size() - 1];
	m_ParameterList.clear();
	while (boost::regex_search(start, end, what, reg, boost::match_default))
	{
		std::string Name = what[7];
		std::string Annotations = what[8];
		std::string Initialize = what[9];
		if (what[2].matched)
		{
			float Value = 0.0f;
			boost::regex reg_value("-?\\d+(\\.\\d+)?");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
			{
				Value = boost::lexical_cast<float>(what2[0]);
			}
			AddParameter(Name, Value);
		}
		else if (what[3].matched)
		{
			Vector2 Value(0, 0);
			boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
			{
				Value.x = boost::lexical_cast<float>(what2[1]);
				Value.y = boost::lexical_cast<float>(what2[3]);
			}
			AddParameter(Name, Value);
		}
		else if (what[4].matched)
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
			AddParameter(Name, Value);
		}
		else if (what[5].matched)
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
			AddParameter(Name, Value);
		}
		else if (what[6].matched)
		{
			std::string Path;
			boost::regex reg_value("string\\s+Initialize\\s*=\\s*\\\"([^\"]+)\\\"");
			boost::match_results<std::string::const_iterator> what2;
			if (boost::regex_search(Annotations, what2, reg_value, boost::match_default))
			{
				Path = what2[1];
			}
			AddParameter(Name, Path);
		}
		start = what[0].second;
	}

	//                             12                3                4               5                    6                7                     8                      9
	boost::regex reg_pass("pass\\s+((PassTypeShadow)|(PassTypeNormal)|(PassTypeLight)|(PassTypeBackground)|(PassTypeOpaque)|(PassTypeTransparent))(\\s*<[^>]+>)?\\s*{\\s*(\\w*)[^}]*}");
	start = (const char *)&(*cache)[0];
	end = (const char *)&(*cache)[cache->size() - 1];
	m_PassMask = 0;
	while (boost::regex_search(start, end, what, reg_pass, boost::match_default))
	{
		std::string assignment = what[9];
		if (what[2].matched)
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeShadow;
			}
		}
		else if (what[3].matched)
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeNormal;
			}
		}
		else if (what[4].matched)
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeLight;
			}
		}
		else if (what[5].matched)
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeBackground;
			}
		}
		else if (what[6].matched)
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeOpaque;
			}
		}
		else if (what[7].matched)
		{
			if (!assignment.empty())
			{
				m_PassMask |= 1 << RenderPipeline::PassTypeTransparent;
			}
		}
		start = what[0].second;
	}
}

template <>
void Material::AddParameter<float>(const std::string& Name, const float& Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat(Name, Value)));
}

template <>
void Material::AddParameter<my::Vector2>(const std::string& Name, const my::Vector2& Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat2(Name, Value)));
}

template <>
void Material::AddParameter<my::Vector3>(const std::string& Name, const my::Vector3& Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat3(Name, Value)));
}

template <>
void Material::AddParameter<my::Vector4>(const std::string& Name, const my::Vector4& Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat4(Name, Value)));
}

template <>
void Material::AddParameter<std::string>(const std::string& Name, const std::string& Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterTexture(Name, Value)));
}

MaterialParameterPtr Material::GetParameter(const std::string& Name)
{
	MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		if ((*param_iter)->m_Name == Name)
		{
			return *param_iter;
		}
	}
	return MaterialParameterPtr();
}

template <>
void Material::SetParameter<float>(const char* Name, const float& Value)
{
	MaterialParameterPtr param = GetParameter(Name);
	if (!param || param->GetParameterType() != MaterialParameter::ParameterTypeFloat)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have float param: %s", Name).c_str());
		return;
	}
	boost::dynamic_pointer_cast<MaterialParameterFloat>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<my::Vector2>(const char* Name, const my::Vector2& Value)
{
	MaterialParameterPtr param = GetParameter(Name);
	if (!param || param->GetParameterType() != MaterialParameter::ParameterTypeFloat2)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Vector2 param: %s", Name).c_str());
		return;
	}
	boost::dynamic_pointer_cast<MaterialParameterFloat2>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<my::Vector3>(const char* Name, const my::Vector3& Value)
{
	MaterialParameterPtr param = GetParameter(Name);
	if (!param || param->GetParameterType() != MaterialParameter::ParameterTypeFloat3)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Vector3 param: %s", Name).c_str());
		return;
	}
	boost::dynamic_pointer_cast<MaterialParameterFloat3>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<my::Vector4>(const char* Name, const my::Vector4& Value)
{
	MaterialParameterPtr param = GetParameter(Name);
	if (!param || param->GetParameterType() != MaterialParameter::ParameterTypeFloat4)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Vector4 param: %s", Name).c_str());
		return;
	}
	boost::dynamic_pointer_cast<MaterialParameterFloat4>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<std::string>(const char* Name, const std::string& Value)
{
	MaterialParameterPtr param = GetParameter(Name);
	if (!param || param->GetParameterType() != MaterialParameter::ParameterTypeTexture)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Texture param: %s", Name).c_str());
		return;
	}
	_ASSERT(!ResourceMgr::getSingleton().FindIORequestCallback(boost::bind(&MaterialParameterTexture::OnTextureReady, param.get(), boost::placeholders::_1)));
	_ASSERT(!boost::dynamic_pointer_cast<MaterialParameterTexture>(param)->m_Texture);
	boost::dynamic_pointer_cast<MaterialParameterTexture>(param)->m_TexturePath = Value;
}
