#include "StdAfx.h"
#include "EffectMesh.h"

template <>
void Parameter<bool>::SetParameter(my::Effect * effect) const
{
	effect->SetBool(name.c_str(), value);
}

template <>
void Parameter<float>::SetParameter(my::Effect * effect) const
{
	effect->SetFloat(name.c_str(), value);
}

template <>
void Parameter<int>::SetParameter(my::Effect * effect) const
{
	effect->SetInt(name.c_str(), value);
}

template <>
void Parameter<my::Vector4>::SetParameter(my::Effect * effect) const
{
	effect->SetVector(name.c_str(), value);
}

template <>
void Parameter<my::Matrix4>::SetParameter(my::Effect * effect) const
{
	effect->SetMatrix(name.c_str(), value);
}

template <>
void Parameter<std::string>::SetParameter(my::Effect * effect) const
{
	effect->SetString(name.c_str(), value.c_str());
}

template <>
void Parameter<my::BaseTexturePtr>::SetParameter(my::Effect * effect) const
{
	effect->SetTexture(name.c_str(), value ? value->m_ptr : NULL);
}

void ParameterMap::AddBool(const std::string & name, bool value)
{
	operator[](name) = BaseParameterPtr(new Parameter<bool>(name, value));
}

void ParameterMap::AddFloat(const std::string & name, float value)
{
	operator[](name) = BaseParameterPtr(new Parameter<float>(name, value));
}

void ParameterMap::AddInt(const std::string & name, int value)
{
	operator[](name) = BaseParameterPtr(new Parameter<int>(name, value));
}

void ParameterMap::AddVector(const std::string & name, const my::Vector4 & value)
{
	operator[](name) = BaseParameterPtr(new Parameter<my::Vector4>(name, value));
}

void ParameterMap::AddMatrix(const std::string & name, const my::Matrix4 & value)
{
	operator[](name) = BaseParameterPtr(new Parameter<my::Matrix4>(name, value));
}

void ParameterMap::AddString(const std::string & name, const std::string & value)
{
	operator[](name) = BaseParameterPtr(new Parameter<std::string>(name, value));
}

void ParameterMap::AddTexture(const std::string & name, my::BaseTexturePtr value)
{
	operator[](name) = BaseParameterPtr(new Parameter<my::BaseTexturePtr>(name, value));
}

void Material::ApplyParameterBlock(void)
{
	const_iterator param_iter = begin();
	for(; param_iter != end(); param_iter++)
	{
		param_iter->second->SetParameter(m_Effect.get());
	}
}

UINT Material::Begin(DWORD Flags)
{
	return m_Effect->Begin(Flags);
}

void Material::End(void)
{
	m_Effect->End();
}

void Material::BeginPass(UINT Pass)
{
	m_Effect->BeginPass(Pass);
}

void Material::EndPass(void)
{
	m_Effect->EndPass();
}

void EffectMesh::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	_ASSERT(m_Mesh);

	for(DWORD i = 0; i < m_materials.size(); i++)
	{
		Material * mat = m_materials[i].get();
		if(mat->m_Effect && mat->m_Effect->m_ptr)
		{
			mat->ApplyParameterBlock();
			UINT cPasses = mat->Begin();
			for(UINT p = 0; p < cPasses; p++)
			{
				mat->BeginPass(p);
				m_Mesh->DrawSubset(i);
				mat->EndPass();
			}
			mat->End();
		}
	}
}
