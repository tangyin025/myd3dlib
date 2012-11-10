#include "StdAfx.h"
#include "EffectMesh.h"

template <>
void Parameter<bool>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetBool(name.c_str(), value);
}

template <>
void Parameter<float>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetFloat(name.c_str(), value);
}

template <>
void Parameter<int>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetInt(name.c_str(), value);
}

template <>
void Parameter<my::Vector4>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetVector(name.c_str(), value);
}

template <>
void Parameter<my::Matrix4>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetMatrix(name.c_str(), value);
}

template <>
void Parameter<std::string>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetString(name.c_str(), value.c_str());
}

template <>
void Parameter<my::BaseTexturePtr>::SetParameter(my::Effect * effect, const std::string & name) const
{
	effect->SetTexture(name.c_str(), value ? value->m_ptr : NULL);
}

void ParameterMap::SetBool(const std::string & name, bool value)
{
	operator[](name) = BaseParameterPtr(new Parameter<bool>(value));
}

void ParameterMap::SetFloat(const std::string & name, float value)
{
	operator[](name) = BaseParameterPtr(new Parameter<float>(value));
}

void ParameterMap::SetInt(const std::string & name, int value)
{
	operator[](name) = BaseParameterPtr(new Parameter<int>(value));
}

void ParameterMap::SetVector(const std::string & name, const my::Vector4 & value)
{
	operator[](name) = BaseParameterPtr(new Parameter<my::Vector4>(value));
}

void ParameterMap::SetMatrix(const std::string & name, const my::Matrix4 & value)
{
	operator[](name) = BaseParameterPtr(new Parameter<my::Matrix4>(value));
}

void ParameterMap::SetString(const std::string & name, const std::string & value)
{
	operator[](name) = BaseParameterPtr(new Parameter<std::string>(value));
}

void ParameterMap::SetTexture(const std::string & name, my::BaseTexturePtr value)
{
	operator[](name) = BaseParameterPtr(new Parameter<my::BaseTexturePtr>(value));
}

void Material::ApplyParameterBlock(void)
{
	const_iterator param_iter = begin();
	for(; param_iter != end(); param_iter++)
	{
		param_iter->second->SetParameter(m_Effect.get(), param_iter->first);
	}
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

			DrawSubset(i, mat->m_Effect.get());
		}
	}
}

void EffectMesh::DrawSubset(DWORD i, my::Effect * effect)
{
	_ASSERT(m_Mesh);

	UINT cPasses = effect->Begin();
	for(UINT p = 0; p < cPasses; p++)
	{
		effect->BeginPass(p);
		m_Mesh->DrawSubset(i);
		effect->EndPass();
	}
	effect->End();
}
