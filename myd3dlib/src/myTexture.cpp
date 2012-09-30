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
	LPCTSTR pSrcFile,
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
	HRESULT hres = D3DXCreateTextureFromFileEx(
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

void CubeTexture::CreateAdjustedCubeTexture(
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
	LPCTSTR pSrcFile,
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
	HRESULT hres = D3DXCreateCubeTextureFromFileEx(
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
