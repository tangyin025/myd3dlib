#include "myAStar.h"

using namespace my;

template <>
D3DCOLOR BilinearFiltering<D3DCOLOR>::Sample(float u, float v)
{
	float us = u * width - 0.5f;
	float vs = v * pixel.shape()[0] - 0.5f;
	int j = (int)us;
	int i = (int)vs;
	float uf = us - j;
	float vf = vs - i;
	const D3DXCOLOR n[] = {
		pixel[(i + 0) % pixel.shape()[0]][(j + 0) % width],
		pixel[(i + 0) % pixel.shape()[0]][(j + 1) % width],
		pixel[(i + 1) % pixel.shape()[0]][(j + 0) % width],
		pixel[(i + 1) % pixel.shape()[0]][(j + 1) % width]
	};
	D3DXCOLOR r[3];
	D3DXColorLerp(&r[0], D3DXColorLerp(&r[1], &n[0], &n[1], uf), D3DXColorLerp(&r[2], &n[2], &n[3], uf), vf);
	return r[0];
}
