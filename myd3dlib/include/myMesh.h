
#pragma once

#include <d3d9.h>
#include <d3dx9mesh.h>
#include <string>

namespace my
{
	void LoadMeshFromOgreMesh(
		std::string & strOgreMeshXml,
		LPDIRECT3DDEVICE9 pd3dDevice,
		DWORD * pNumSubMeshes,
		LPD3DXMESH * ppMesh,
		DWORD dwMeshOptions = D3DXMESH_SYSTEMMEM);
};
