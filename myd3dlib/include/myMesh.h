
#pragma once

#include <d3d9.h>
#include <d3dx9mesh.h>
#include <string>

namespace my
{
	struct OgreMeshVertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		FLOAT tu;
		FLOAT tv;

		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	};

	void LoadMeshFromOgreMesh(
		std::string & strOgreMeshXml,
		LPDIRECT3DDEVICE9 pd3dDevice,
		DWORD * pNumSubMeshes,
		LPD3DXMESH * ppMesh,
		DWORD dwMeshOptions = D3DXMESH_SYSTEMMEM);
};
