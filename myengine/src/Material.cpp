#include "Material.h"
#include "myEffect.h"
#include "myDxutApp.h"
#include "myResource.h"
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
	return boost::shared_ptr<MaterialParameterFloat>(new MaterialParameterFloat(m_Name.c_str(), m_Value));
}

void MaterialParameterFloat2::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 2);
}

MaterialParameterPtr MaterialParameterFloat2::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat2>(new MaterialParameterFloat2(m_Name.c_str(), m_Value));
}

void MaterialParameterFloat3::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 3);
}

MaterialParameterPtr MaterialParameterFloat3::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat3>(new MaterialParameterFloat3(m_Name.c_str(), m_Value));
}

void MaterialParameterFloat4::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 4);
}

MaterialParameterPtr MaterialParameterFloat4::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterFloat4>(new MaterialParameterFloat4(m_Name.c_str(), m_Value));
}

void MaterialParameterTexture::OnReady(my::DeviceResourceBasePtr res)
{
	m_Texture = boost::dynamic_pointer_cast<my::BaseTexture>(res);
}

void MaterialParameterTexture::RequestResource(void)
{
	if (!m_TexturePath.empty())
	{
		my::ResourceMgr::getSingleton().LoadTextureAsync(m_TexturePath.c_str(), this);
	}
}

void MaterialParameterTexture::ReleaseResource(void)
{
	if (!m_TexturePath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_TexturePath, this);
	}
}

void MaterialParameterTexture::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetTexture(m_Handle, m_Texture.get());
}

MaterialParameterPtr MaterialParameterTexture::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterTexture>(new MaterialParameterTexture(m_Name.c_str(), m_TexturePath.c_str()));
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

void Material::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId)
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

void Material::ParseShaderParamters(void)
{
	if (!m_Shader.empty())
	{
		CachePtr cache = my::ResourceMgr::getSingleton().OpenIStream(m_Shader.c_str())->GetWholeCache();
		cache->push_back(0);
		boost::regex reg("(float\\d?|texture)\\s+(\\w+)\\s*:\\s*MaterialParameter(\\s*<[^>]+>)?(\\s*=\\s*[^;]+)?;");
		boost::match_results<const char *> what;
		const char * start = (const char *)&(*cache)[0];
		const char * end = (const char *)&(*cache)[cache->size() - 1];
		while (boost::regex_search(start, end, what, reg, boost::match_default))
		{
			std::string Type = what[1];
			std::string Name = what[2];
			std::string Annotations = what[3];
			std::string Initialize = what[4];
			if (Type == "float")
			{
				float Value = 0.0f;
				boost::regex reg2("-?\\d+(\\.\\d+)?");
				boost::match_results<std::string::const_iterator> what2;
				if (boost::regex_search(Initialize, what2, reg2, boost::match_default))
				{
					Value = boost::lexical_cast<float>(what2[0]);
				}
				AddParameterFloat(Name.c_str(), Value);
			}
			else if (Type == "float2")
			{
				Vector2 Value(0, 0);
				boost::regex reg2("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
				boost::match_results<std::string::const_iterator> what2;
				if (boost::regex_search(Initialize, what2, reg2, boost::match_default))
				{
					Value.x = boost::lexical_cast<float>(what2[1]);
					Value.y = boost::lexical_cast<float>(what2[3]);
				}
				AddParameterFloat2(Name.c_str(), Value);
			}
			else if (Type == "float3")
			{
				Vector3 Value(0, 0, 0);
				boost::regex reg2("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
				boost::match_results<std::string::const_iterator> what2;
				if (boost::regex_search(Initialize, what2, reg2, boost::match_default))
				{
					Value.x = boost::lexical_cast<float>(what2[1]);
					Value.y = boost::lexical_cast<float>(what2[3]);
					Value.z = boost::lexical_cast<float>(what2[5]);
				}
				AddParameterFloat3(Name.c_str(), Value);
			}
			else if (Type == "float4")
			{
				Vector4 Value(0, 0, 0, 1);
				boost::regex reg2("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
				boost::match_results<std::string::const_iterator> what2;
				if (boost::regex_search(Initialize, what2, reg2, boost::match_default))
				{
					Value.x = boost::lexical_cast<float>(what2[1]);
					Value.y = boost::lexical_cast<float>(what2[3]);
					Value.z = boost::lexical_cast<float>(what2[5]);
					Value.w = boost::lexical_cast<float>(what2[7]);
				}
				AddParameterFloat4(Name.c_str(), Value);
			}
			else if (Type == "texture")
			{
				std::string Path;
				boost::regex reg2("string\\s+Initialize\\s*=\\s*\\\"([^\"]+)\\\"");
				boost::match_results<std::string::const_iterator> what2;
				if (boost::regex_search(Annotations, what2, reg2, boost::match_default))
				{
					Path = what2[1];
				}
				AddParameterTexture(Name.c_str(), Path.c_str());
			}
			start = what[0].second;
		}
	}
}

void Material::AddParameterFloat(const char * Name, float Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat(Name, Value)));
}

void Material::AddParameterFloat2(const char * Name, const my::Vector2 & Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat2(Name, Value)));
}

void Material::AddParameterFloat3(const char * Name, const my::Vector3 & Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat3(Name, Value)));
}

void Material::AddParameterFloat4(const char * Name, const my::Vector4 & Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat4(Name, Value)));
}

void Material::AddParameterTexture(const char * Name, const char * Path)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterTexture(Name, Path)));
}
