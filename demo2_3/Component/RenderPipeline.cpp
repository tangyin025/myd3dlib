#include "stdafx.h"
#include "RenderPipeline.h"

using namespace my;

void RenderPipeline::OnRender(IDirect3DDevice9 * pd3dDevice, double fTime, float fElapsedTime)
{
	OpaqueRenderAtomList::iterator atom_iter = m_OpaqueList.begin();
	for (; atom_iter != m_OpaqueList.end(); atom_iter++)
	{
		RenderOpaqueMesh(atom_iter->get<0>(), atom_iter->get<1>(), atom_iter->get<2>(), atom_iter->get<3>());
	}
}

void RenderPipeline::RenderOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	shader->SetTechnique("RenderScene");
	UINT passes = shader->Begin(0);
	setter->OnSetShader(shader, AttribId);
	for (UINT p = 0; p < passes; p++)
	{
		shader->BeginPass(p);
		mesh->DrawSubset(AttribId);
		shader->EndPass();
	}
	shader->End();
}

void RenderPipeline::PushOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter)
{
	m_OpaqueList.push_back(boost::make_tuple(mesh, AttribId, shader, setter));
}

void RenderPipeline::ClearAllOpaqueMeshes(void)
{
	m_OpaqueList.clear();
}
