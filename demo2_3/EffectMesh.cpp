#include "StdAfx.h"
#include "EffectMesh.h"

void EffectMesh::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	for(DWORD i = 0; i < m_materials.size(); i++)
	{
		Material * mat = m_materials[i].get();
		mat->m_Effect->ApplyParameterBlock(mat->m_Param);
		UINT cPasses = mat->m_Effect->Begin();
		for(UINT p = 0; p < cPasses; p++)
		{
			mat->m_Effect->BeginPass(p);
			DrawSubset(i);
			mat->m_Effect->EndPass();
		}
		mat->m_Effect->End();
	}
}
