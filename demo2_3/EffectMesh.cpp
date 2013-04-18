#include "StdAfx.h"
#include "EffectMesh.h"

using namespace my;

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

void EffectMesh::DrawSubset(DWORD i, Effect * pEffect)
{
	_ASSERT(m_Mesh);

	UINT cPasses = pEffect->Begin();
	for(UINT p = 0; p < cPasses; p++)
	{
		pEffect->BeginPass(p);
		m_Mesh->DrawSubset(i);
		pEffect->EndPass();
	}
	pEffect->End();
}
