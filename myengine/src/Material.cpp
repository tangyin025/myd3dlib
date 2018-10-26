#include "Material.h"
#include "myEffect.h"
#include <boost/regex.hpp>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include "myDxutApp.h"

using namespace my;

BOOST_CLASS_EXPORT(MaterialParameter)

BOOST_CLASS_EXPORT(MaterialParameterFloat)

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

void MaterialParameterTexture::Set(my::Effect * shader)
{
	_ASSERT(m_Handle);
	shader->SetTexture(m_Handle, m_Texture.m_Res.get());
}

void MaterialParameterTexture::RequestResource(void)
{
	m_Texture.RequestResource();
}

void MaterialParameterTexture::ReleaseResource(void)
{
	m_Texture.ReleaseResource();
}

MaterialParameterPtr MaterialParameterTexture::Clone(void) const
{
	return boost::shared_ptr<MaterialParameterTexture>(new MaterialParameterTexture(m_Name.c_str(), m_Texture.m_Path.c_str()));
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
		boost::regex reg("(float\\d?|texture)\\s+(\\w+)\\s*:\\s*MaterialParameter");
		boost::match_results<const char *> what;
		const char * start = (const char *)&(*cache)[0];
		const char * end = (const char *)&(*cache)[cache->size() - 1];
		while (boost::regex_search(start, end, what, reg, boost::match_default))
		{
			std::string Type = what[1];
			std::string Name = what[2];
			if (Type == "float")
			{
				AddParameterFloat(Name.c_str(), 0.0f);
			}
			else if (Type == "texture")
			{
				AddParameterTexture(Name.c_str(), "");
			}
			start = what[0].second;
		}
	}
}

void Material::AddParameterFloat(const char * Name, float Value)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat(Name, Value)));
}

void Material::AddParameterTexture(const char * Name, const char * Path)
{
	m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterTexture(Name, Path)));
}
