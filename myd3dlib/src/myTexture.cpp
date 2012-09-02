#include "stdafx.h"
#include "myTexture.h"

using namespace my;

void Surface::CreateDepthStencilSurface(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Width,
	UINT Height,
	D3DFORMAT Format,
	D3DMULTISAMPLE_TYPE MultiSample,
	DWORD MultisampleQuality,
	BOOL Discard)
{
	LPDIRECT3DSURFACE9 pSurface = NULL;
	HRESULT hres = pDevice->CreateDepthStencilSurface(
		Width, Height, Format, MultiSample, MultisampleQuality, Discard, &pSurface, NULL);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pSurface);
}

void Surface::CreateOffscreenPlainSurface(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Width,
	UINT Height,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DSURFACE9 pSurface = NULL;
	HRESULT hres = pDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, &pSurface, NULL);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pSurface);
}

void Texture::CreateTexture(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	HRESULT hres = pDevice->CreateTexture(
		Width, Height, MipLevels, Usage, Format, Pool, &pTexture, NULL);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pTexture);
}

void Texture::CreateAdjustedTexture(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	HRESULT hres = D3DXCreateTexture(
		pDevice, Width, Height, MipLevels, Usage, Format, Pool, &pTexture);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pTexture);
}

void Texture::CreateTextureFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pSrcFile,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	DWORD Filter,
	DWORD MipFilter,
	D3DCOLOR ColorKey,
	D3DXIMAGE_INFO * pSrcInfo,
	PALETTEENTRY * pPalette)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	HRESULT hres = D3DXCreateTextureFromFileExA(
		pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pTexture);
}

void Texture::CreateTextureFromFileInMemory(
	LPDIRECT3DDEVICE9 pDevice,
	LPCVOID pSrcData,
	UINT SrcDataSize,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	DWORD Filter,
	DWORD MipFilter,
	D3DCOLOR ColorKey,
	D3DXIMAGE_INFO * pSrcInfo,
	PALETTEENTRY * pPalette)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	HRESULT hres = D3DXCreateTextureFromFileInMemoryEx(
		pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pTexture);
}

void CubeTexture::CreateCubeTexture(
	LPDIRECT3DDEVICE9 pDevice,
	UINT EdgeLength,
	UINT Levels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;
	HRESULT hres = pDevice->CreateCubeTexture(
		EdgeLength, Levels, Usage, Format, Pool, &pCubeTexture, NULL);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pCubeTexture);
}

void CubeTexture::CubeAdjustedTexture(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Size,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;
	HRESULT hres = D3DXCreateCubeTexture(
		pDevice, Size, MipLevels, Usage, Format, Pool, &pCubeTexture);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pCubeTexture);
}

void CubeTexture::CreateCubeTextureFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pSrcFile,
	UINT Size,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	DWORD Filter,
	DWORD MipFilter,
	D3DCOLOR ColorKey,
	D3DXIMAGE_INFO * pSrcInfo,
	PALETTEENTRY * pPalette)
{
	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;
	HRESULT hres = D3DXCreateCubeTextureFromFileExA(
		pDevice, pSrcFile, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pCubeTexture);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pCubeTexture);
}

void CubeTexture::CreateCubeTextureFromFileInMemory(
	LPDIRECT3DDEVICE9 pDevice,
	LPCVOID pSrcData,
	UINT SrcDataSize,
	UINT Size,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	DWORD Filter,
	DWORD MipFilter,
	D3DCOLOR ColorKey,
	D3DXIMAGE_INFO * pSrcInfo,
	PALETTEENTRY * pPalette)
{
	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;
	HRESULT hres = D3DXCreateCubeTextureFromFileInMemoryEx(
		pDevice, pSrcData, SrcDataSize, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pCubeTexture);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pCubeTexture);
}

void CubeTexture::DrawToSurface(
	LPDIRECT3DDEVICE9 pd3dDevice,
	D3DCUBEMAP_FACES FaceType,
	TexturePtr texture,
	UINT Level)
{
	CComPtr<IDirect3DSurface9> SurfaceDst = GetCubeMapSurface(FaceType, Level);

	D3DSURFACE_DESC desc;
	V(static_cast<IDirect3DCubeTexture9 *>(m_ptr)->GetLevelDesc(Level, &desc));

	struct Vertex
	{
		FLOAT x, y, z, w;
		FLOAT u, v;
	};

	Vertex v[4] =
	{
		{ 0.0f,					0.0f,				0.5f, 1.0f, 0.0f, 0.0f },
		{ (FLOAT)desc.Width,	0.0f,				0.5f, 1.0f, 1.0f, 0.0f },
		{ 0.0f,					(FLOAT)desc.Height,	0.5f, 1.0f, 0.0f, 1.0f },
		{ (FLOAT)desc.Width,	(FLOAT)desc.Height,	0.5f, 1.0f, 1.0f, 1.0f },
	};

	V(pd3dDevice->SetRenderTarget(0, SurfaceDst));
	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
		V(pd3dDevice->SetTexture(0, texture->m_ptr));
		V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0])));
		V(pd3dDevice->EndScene());
	}
}
