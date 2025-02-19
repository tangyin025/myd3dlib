// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "Material.h"
#include "myEffect.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "RenderPipeline.h"
#include "myUtility.h"
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
#include <boost/range/algorithm/find_if.hpp>

using namespace my;

BOOST_CLASS_EXPORT(MaterialParameter)

BOOST_CLASS_EXPORT(MaterialParameterInt2)

BOOST_CLASS_EXPORT(MaterialParameterFloat)

BOOST_CLASS_EXPORT(MaterialParameterFloat2)

BOOST_CLASS_EXPORT(MaterialParameterFloat3)

BOOST_CLASS_EXPORT(MaterialParameterFloat4)

BOOST_CLASS_EXPORT(MaterialParameterTexture)

BOOST_CLASS_EXPORT(MaterialParameterInvWorldView)

BOOST_CLASS_EXPORT(MaterialParameterFarZ)

BOOST_CLASS_EXPORT(Material)

bool MaterialParameter::operator == (const MaterialParameter & rhs) const
{
	if (GetParameterType() == rhs.GetParameterType() && m_Name == rhs.m_Name)
	{
		switch (GetParameterType())
		{
		case ParameterTypeNone:
			return true;
		case ParameterTypeInt2:
			return static_cast<const MaterialParameterInt2 &>(*this).m_Value == static_cast<const MaterialParameterInt2 &>(rhs).m_Value;
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
		case ParameterTypeInvWorldView:
			return true;
		}
		_ASSERT(false);
	}
	return false;
}

void MaterialParameter::Init(my::Effect * shader)
{
	if (NULL == (m_Handle = shader->GetParameterByName(NULL, m_Name.c_str())))
	{
		THROW_CUSEXCEPTION(m_Name);
	}
}

template<class Archive>
void MaterialParameterInt2::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
	ar << BOOST_SERIALIZATION_NVP(m_Value.x);
	ar << BOOST_SERIALIZATION_NVP(m_Value.y);
}

template<class Archive>
void MaterialParameterInt2::load(Archive& ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(MaterialParameter);
	ar >> BOOST_SERIALIZATION_NVP(m_Value.x);
	ar >> BOOST_SERIALIZATION_NVP(m_Value.y);
}

void MaterialParameterInt2::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	_ASSERT(m_Handle);
	shader->SetIntArray(m_Handle, (int*)&m_Value.x, 2);
}

void MaterialParameterFloat::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	_ASSERT(m_Handle);
	shader->SetFloat(m_Handle, m_Value);
}

void MaterialParameterFloat2::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 2);
}

void MaterialParameterFloat3::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 3);
}

void MaterialParameterFloat4::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	_ASSERT(m_Handle);
	shader->SetFloatArray(m_Handle, &m_Value.x, 4);
}

MaterialParameterTexture::~MaterialParameterTexture(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void MaterialParameterTexture::OnTextureReady(my::DeviceResourceBasePtr res)
{
	m_Texture = boost::dynamic_pointer_cast<my::BaseTexture>(res);
}

void MaterialParameterTexture::RequestResource(void)
{
	MaterialParameter::RequestResource();

	if (!m_TexturePath.empty())
	{
		_ASSERT(!m_Texture && m_Owner && m_Owner->m_Cmp);

		my::ResourceMgr::getSingleton().LoadTextureAsync(m_TexturePath.c_str(), boost::bind(&MaterialParameterTexture::OnTextureReady, this, boost::placeholders::_1),
			(m_Owner->m_Cmp->m_LodMask & Component::LOD0) ? Component::ResPriorityLod0 : (m_Owner->m_Cmp->m_LodMask & Component::LOD1) ? Component::ResPriorityLod1 : Component::ResPriorityLod2);
	}
}

void MaterialParameterTexture::ReleaseResource(void)
{
	MaterialParameter::ReleaseResource();

	if (!m_TexturePath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_TexturePath, boost::bind(&MaterialParameterTexture::OnTextureReady, this, boost::placeholders::_1));

		m_Texture.reset();
	}
}

void MaterialParameterTexture::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	_ASSERT(m_Handle);
	shader->SetTexture(m_Handle, m_Texture.get());
}

void MaterialParameterInvWorldView::Set(my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	// ! RenderPipeline::RenderAllObjects, mesh_bat_iter->second.mtl->OnSetShader(..
	_ASSERT(m_Handle && m_Owner && m_Owner->m_Cmp);
	Matrix4 InvWorldView = (m_Owner->m_Cmp->m_Actor->m_World * pRC->m_Camera->m_View).inverse();
	shader->SetMatrix(m_Handle, InvWorldView);
}

void MaterialParameterFarZ::Set(my::Effect* shader, LPARAM lparam, RenderPipeline::IRenderContext* pRC)
{
	_ASSERT(m_Handle);
	shader->SetFloat(m_Handle, pRC->m_Camera->m_Proj._43 / pRC->m_Camera->m_Proj._33);
}

template<class Archive>
void Material::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Shader);
	ar << BOOST_SERIALIZATION_NVP(m_PassMask);
	ar << BOOST_SERIALIZATION_NVP(m_CullMode);
	ar << BOOST_SERIALIZATION_NVP(m_ZEnable);
	ar << BOOST_SERIALIZATION_NVP(m_ZWriteEnable);
	ar << BOOST_SERIALIZATION_NVP(m_ZFunc);
	ar << BOOST_SERIALIZATION_NVP(m_AlphaTestEnable);
	ar << BOOST_SERIALIZATION_NVP(m_AlphaRef);
	ar << BOOST_SERIALIZATION_NVP(m_AlphaFunc);
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
	ar >> BOOST_SERIALIZATION_NVP(m_ZFunc);
	ar >> BOOST_SERIALIZATION_NVP(m_AlphaTestEnable);
	ar >> BOOST_SERIALIZATION_NVP(m_AlphaRef);
	ar >> BOOST_SERIALIZATION_NVP(m_AlphaFunc);
	ar >> BOOST_SERIALIZATION_NVP(m_BlendMode);
	ar >> BOOST_SERIALIZATION_NVP(m_ParameterList);
	MaterialParameterPtrList::iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		(*param_iter)->m_Owner = this;
	}
}

bool Material::operator == (const Material & rhs) const
{
	if (m_PassMask == rhs.m_PassMask
		&& m_CullMode == rhs.m_CullMode
		&& m_ZEnable == rhs.m_ZEnable
		&& m_ZWriteEnable == rhs.m_ZWriteEnable
		&& m_ZFunc == rhs.m_ZFunc
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

	MaterialPtr ret;
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(sstr, ".txt");
	*ia >> boost::serialization::make_nvp(__FUNCTION__, ret);
	return ret;
}

void Material::OnResetShader(void)
{
	if (!m_ParameterList.empty())
	{
		m_ParameterList.front()->m_Handle = NULL;
	}
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

void Material::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam, RenderPipeline::IRenderContext * pRC)
{
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/accurately-profiling-direct3d-api-calls#appendix
	HRESULT hr;
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, m_CullMode));
	if (m_ZEnable)
	{
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, m_ZWriteEnable));
		V(pd3dDevice->SetRenderState(D3DRS_ZFUNC, m_ZFunc));
	}
	else
	{
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
	}
	if (m_AlphaTestEnable)
	{
		// https://learn.microsoft.com/en-us/windows/win32/direct3d9/alpha-testing-state
		V(pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHAREF, m_AlphaRef));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, m_AlphaFunc));
	}
	else
	{
		V(pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
	}
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
		(*param_iter)->Set(shader, lparam, pRC);
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
	//                     1 2     3       4        5        6        7         8              9                               10                11
	boost::regex reg("^\\s*((int2)|(float)|(float2)|(float3)|(float4)|(texture)|(float4x4))\\s+(\\w+)\\s*:\\s*MaterialParameter(\\s*<[^>]+>)?\\s*(=\\s*[^;]+)?;");
	boost::match_results<const char *> what;
	const char * start = (const char *)&(*cache)[0];
	const char * end = (const char *)&(*cache)[cache->size() - 1];
	MaterialParameterPtrList dummy_params;
	dummy_params.swap(m_ParameterList);
	for (; boost::regex_search(start, end, what, reg, boost::match_default); start = what[0].second)
	{
		std::string Name = what[9];
		std::string Annotations = what[10];
		std::string Initialize = what[11];
		MaterialParameterPtrList::const_iterator dummy_param_iter = boost::find_if(dummy_params,
			boost::bind(std::equal_to<std::string>(), boost::bind(&MaterialParameter::m_Name, boost::bind(&MaterialParameterPtr::get, boost::placeholders::_1)), Name));

		boost::regex reg_value("bool\\s+UseInvWorldView\\s*=\\s*true");
		boost::match_results<std::string::const_iterator> what2;
		if (boost::regex_search(Annotations, what2, reg_value, boost::match_default))
		{
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterInvWorldView(this, Name)));
		}
		else if (boost::regex_search(Annotations, what2, boost::regex("bool\\sUseFarZ\\s*=\\s*true"), boost::match_default))
		{
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFarZ(this, Name)));
		}
		else if (what[2].matched)
		{
			CPoint Value(0, 0);
			if (dummy_param_iter != dummy_params.end() && (*dummy_param_iter)->GetParameterType() == MaterialParameter::ParameterTypeInt2)
			{
				Value = boost::dynamic_pointer_cast<MaterialParameterInt2>(*dummy_param_iter)->m_Value;
			}
			else
			{
				boost::regex reg_value("(-?\\d+)\\s*,\\s*(-?\\d+)");
				if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
				{
					Value.x = boost::lexical_cast<int>(what2[1]);
					Value.y = boost::lexical_cast<int>(what2[2]);
				}
			}
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterInt2(this, Name, Value)));
		}
		else if (what[3].matched)
		{
			float Value = 0.0f;
			if (dummy_param_iter != dummy_params.end() && (*dummy_param_iter)->GetParameterType() == MaterialParameter::ParameterTypeFloat)
			{
				Value = boost::dynamic_pointer_cast<MaterialParameterFloat>(*dummy_param_iter)->m_Value;
			}
			else
			{
				boost::regex reg_value("-?\\d+(\\.\\d+)?");
				if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
				{
					Value = boost::lexical_cast<float>(what2[0]);
				}
			}
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat(this, Name, Value)));
		}
		else if (what[4].matched)
		{
			Vector2 Value(0, 0);
			if (dummy_param_iter != dummy_params.end() && (*dummy_param_iter)->GetParameterType() == MaterialParameter::ParameterTypeFloat2)
			{
				Value = boost::dynamic_pointer_cast<MaterialParameterFloat2>(*dummy_param_iter)->m_Value;
			}
			else
			{
				boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
				if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
				{
					Value.x = boost::lexical_cast<float>(what2[1]);
					Value.y = boost::lexical_cast<float>(what2[3]);
				}
			}
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat2(this, Name, Value)));
		}
		else if (what[5].matched)
		{
			Vector3 Value(0, 0, 0);
			if (dummy_param_iter != dummy_params.end() && (*dummy_param_iter)->GetParameterType() == MaterialParameter::ParameterTypeFloat3)
			{
				Value = boost::dynamic_pointer_cast<MaterialParameterFloat3>(*dummy_param_iter)->m_Value;
			}
			else
			{
				boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
				if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
				{
					Value.x = boost::lexical_cast<float>(what2[1]);
					Value.y = boost::lexical_cast<float>(what2[3]);
					Value.z = boost::lexical_cast<float>(what2[5]);
				}
			}
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat3(this, Name, Value)));
		}
		else if (what[6].matched)
		{
			Vector4 Value(0, 0, 0, 1);
			if (dummy_param_iter != dummy_params.end() && (*dummy_param_iter)->GetParameterType() == MaterialParameter::ParameterTypeFloat4)
			{
				Value = boost::dynamic_pointer_cast<MaterialParameterFloat4>(*dummy_param_iter)->m_Value;
			}
			else
			{
				boost::regex reg_value("(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)\\s*,\\s*(-?\\d+(\\.\\d+)?)");
				if (boost::regex_search(Initialize, what2, reg_value, boost::match_default))
				{
					Value.x = boost::lexical_cast<float>(what2[1]);
					Value.y = boost::lexical_cast<float>(what2[3]);
					Value.z = boost::lexical_cast<float>(what2[5]);
					Value.w = boost::lexical_cast<float>(what2[7]);
				}
			}
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat4(this, Name, Value)));
		}
		else if (what[7].matched)
		{
			std::string Path;
			if (dummy_param_iter != dummy_params.end() && (*dummy_param_iter)->GetParameterType() == MaterialParameter::ParameterTypeTexture)
			{
				Path = boost::dynamic_pointer_cast<MaterialParameterTexture>(*dummy_param_iter)->m_TexturePath;
			}
			else
			{
				boost::regex reg_value("string\\s+path\\s*=\\s*\\\"([^\"]+)\\\"");
				if (boost::regex_search(Annotations, what2, reg_value, boost::match_default))
				{
					Path = what2[1];
				}
			}
			m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterTexture(this, Name, Path)));
		}
	}

	//                             1 2               3                4               5                    6                7                     8                      9
	boost::regex reg_pass("pass\\s+((PassTypeShadow)|(PassTypeNormal)|(PassTypeLight)|(PassTypeBackground)|(PassTypeOpaque)|(PassTypeTransparent))(\\s*<[^>]+>)?\\s*{\\s*(\\w*)[^}]*}");
	start = (const char *)&(*cache)[0];
	end = (const char *)&(*cache)[cache->size() - 1];
	m_PassMask = 0;
	for (; boost::regex_search(start, end, what, reg_pass, boost::match_default); start = what[0].second)
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
	}
}

MaterialParameter * Material::GetParameter(const char* Name) const
{
	MaterialParameterPtrList::const_iterator param_iter = m_ParameterList.begin();
	for (; param_iter != m_ParameterList.end(); param_iter++)
	{
		if ((*param_iter)->m_Name == Name)
		{
			return param_iter->get();
		}
	}
	return NULL;
}

template <>
void Material::SetParameter<CPoint>(const char* Name, const CPoint& Value)
{
	MaterialParameter * param = GetParameter(Name);
	if (!param)
	{
		m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterInt2(this, Name, Value)));
		return;
	}
	else if (param->GetParameterType() != MaterialParameter::ParameterTypeInt2)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have int2 param: %s", Name).c_str());
		return;
	}
	dynamic_cast<MaterialParameterInt2*>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<float>(const char* Name, const float& Value)
{
	MaterialParameter * param = GetParameter(Name);
	if (!param)
	{
		m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat(this, Name, Value)));
		return;
	}
	else if (param->GetParameterType() != MaterialParameter::ParameterTypeFloat)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have float param: %s", Name).c_str());
		return;
	}
	dynamic_cast<MaterialParameterFloat*>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<my::Vector2>(const char* Name, const my::Vector2& Value)
{
	MaterialParameter * param = GetParameter(Name);
	if (!param)
	{
		m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat2(this, Name, Value)));
		return;
	}
	else if (param->GetParameterType() != MaterialParameter::ParameterTypeFloat2)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Vector2 param: %s", Name).c_str());
		return;
	}
	dynamic_cast<MaterialParameterFloat2*>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<my::Vector3>(const char* Name, const my::Vector3& Value)
{
	MaterialParameter * param = GetParameter(Name);
	if (!param)
	{
		m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat3(this, Name, Value)));
		return;
	}
	else if (param->GetParameterType() != MaterialParameter::ParameterTypeFloat3)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Vector3 param: %s", Name).c_str());
		return;
	}
	dynamic_cast<MaterialParameterFloat3*>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<my::Vector4>(const char* Name, const my::Vector4& Value)
{
	MaterialParameter * param = GetParameter(Name);
	if (!param)
	{
		m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterFloat4(this, Name, Value)));
		return;
	}
	else if (param->GetParameterType() != MaterialParameter::ParameterTypeFloat4)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Vector4 param: %s", Name).c_str());
		return;
	}
	dynamic_cast<MaterialParameterFloat4*>(param)->m_Value = Value;
}

template <>
void Material::SetParameter<std::string>(const char* Name, const std::string& Value)
{
	MaterialParameter * param = GetParameter(Name);
	if (!param)
	{
		m_ParameterList.push_back(MaterialParameterPtr(new MaterialParameterTexture(this, Name, Value)));
		return;
	}
	else if (param->GetParameterType() != MaterialParameter::ParameterTypeTexture)
	{
		my::D3DContext::getSingleton().m_EventLog(str_printf("dose not have Texture param: %s", Name).c_str());
		return;
	}
	_ASSERT(!ResourceMgr::getSingleton().FindIORequestCallback(boost::bind(&MaterialParameterTexture::OnTextureReady, param, boost::placeholders::_1)));
	_ASSERT(!dynamic_cast<MaterialParameterTexture*>(param)->m_Texture);
	dynamic_cast<MaterialParameterTexture*>(param)->m_TexturePath = Value;
}
