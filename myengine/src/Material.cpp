#include "Material.h"
#include "myEffect.h"
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

BOOST_CLASS_EXPORT(Material)

void Material::CopyFrom(const Material & rhs)
{
	m_Shader = rhs.m_Shader;
	m_PassMask = rhs.m_PassMask;
	m_MeshColor = rhs.m_MeshColor;
	m_MeshTexture = rhs.m_MeshTexture;
	m_NormalTexture = rhs.m_NormalTexture;
	m_SpecularTexture = rhs.m_SpecularTexture;
	m_ReflectTexture = rhs.m_ReflectTexture;
}

MaterialPtr Material::Clone(void) const
{
	MaterialPtr ret(new Material());
	ret->CopyFrom(*this);
	return ret;
}

void Material::RequestResource(void)
{
	if (!m_MeshTexture.m_Path.empty())
	{
		m_MeshTexture.RequestResource();
	}

	if (!m_NormalTexture.m_Path.empty())
	{
		m_NormalTexture.RequestResource();
	}

	if (!m_SpecularTexture.m_Path.empty())
	{
		m_SpecularTexture.RequestResource();
	}

	if (!m_ReflectTexture.m_Path.empty())
	{
		m_ReflectTexture.RequestResource();
	}
}

void Material::ReleaseResource(void)
{
	m_MeshTexture.ReleaseResource();
	m_NormalTexture.ReleaseResource();
	m_SpecularTexture.ReleaseResource();
	m_ReflectTexture.ReleaseResource();
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
	shader->SetVector("g_MeshColor", m_MeshColor);
	shader->SetVector("g_RepeatUV", m_RepeatUV);
	shader->SetTexture("g_MeshTexture", m_MeshTexture.m_Res.get());
	shader->SetTexture("g_NormalTexture", m_NormalTexture.m_Res.get());
	shader->SetTexture("g_SpecularTexture", m_SpecularTexture.m_Res.get());
	shader->SetTexture("g_ReflectTexture", m_ReflectTexture.m_Res.get());
}
