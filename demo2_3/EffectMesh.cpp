#include "StdAfx.h"
#include "EffectMesh.h"

void EffectMesh::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	for(DWORD i = 0; i < m_materials.size(); i++)
	{
		Material * mat = m_materials[i].get();
		mat->ApplyParameterBlock();
		UINT cPasses = mat->Begin();
		for(UINT p = 0; p < cPasses; p++)
		{
			mat->BeginPass(p);
			DrawSubset(i);
			mat->EndPass();
		}
		mat->End();
	}
}
