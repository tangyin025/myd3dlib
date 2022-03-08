#include "myAStar.h"

using namespace my;

template <>
D3DCOLOR BilinearFiltering<D3DCOLOR>::Sample(float u, float v)
{
	D3DCOLOR n[4];
	float uf, vf;
	SampleFourNeighbors(u, v, n[0], n[1], n[2], n[3], uf, vf);
	D3DXCOLOR r[3];
	D3DXColorLerp(&r[0], D3DXColorLerp(&r[1], &D3DXCOLOR(n[0]), &D3DXCOLOR(n[1]), uf), D3DXColorLerp(&r[2], &D3DXCOLOR(n[2]), &D3DXCOLOR(n[3]), uf), vf);
	return r[0];
}
