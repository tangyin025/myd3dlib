// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myTexture.h"
#include "myException.h"
#include "myDxutApp.h"
#include "libc.h"

using namespace my;

void Surface::Create(IDirect3DSurface9 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void Surface::CreateDepthStencilSurface(
	UINT Width,
	UINT Height,
	D3DFORMAT Format,
	D3DMULTISAMPLE_TYPE MultiSample,
	DWORD MultisampleQuality,
	BOOL Discard)
{
	LPDIRECT3DSURFACE9 pSurface = NULL;
	hr = my::D3DContext::getSingleton().m_d3dDevice->CreateDepthStencilSurface(
		Width, Height, Format, MultiSample, MultisampleQuality, Discard, &pSurface, NULL);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pSurface);
}

void Surface::CreateOffscreenPlainSurface(
	UINT Width,
	UINT Height,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DSURFACE9 pSurface = NULL;
	hr = my::D3DContext::getSingleton().m_d3dDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, &pSurface, NULL);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pSurface);
}

CComPtr<IUnknown> Surface::GetContainer(REFIID riid)
{
	CComPtr<IUnknown> Container;
	V(m_ptr->GetContainer(riid, (void **)&Container));
	return Container;
}

HDC Surface::GetDC(void)
{
	HDC hdc;
	V(m_ptr->GetDC(&hdc));
	return hdc;
}

D3DSURFACE_DESC Surface::GetDesc(void)
{
	D3DSURFACE_DESC desc;
	V(m_ptr->GetDesc(&desc));
	return desc;
}

D3DLOCKED_RECT Surface::LockRect(const RECT *pRect, DWORD Flags)
{
	D3DLOCKED_RECT lr;
	if(FAILED(hr = m_ptr->LockRect(&lr, pRect, Flags)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return lr;
}

void Surface::ReleaseDC(HDC hdc)
{
	V(m_ptr->ReleaseDC(hdc));
}

void Surface::UnlockRect(void)
{
	V(m_ptr->UnlockRect());
}

void BaseTexture::Create(IDirect3DBaseTexture9 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void BaseTexture::GenerateMipSubLevels(void)
{
	m_ptr->GenerateMipSubLevels();
}

D3DTEXTUREFILTERTYPE BaseTexture::GetAutoGenFilterType(void)
{
	return m_ptr->GetAutoGenFilterType();
}

DWORD BaseTexture::GetLevelCount(void)
{
	return m_ptr->GetLevelCount();
}

DWORD BaseTexture::GetLOD(void)
{
	return m_ptr->GetLOD();
}

void BaseTexture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
{
	V(m_ptr->SetAutoGenFilterType(FilterType));
}

DWORD BaseTexture::SetLOD(DWORD LODNew)
{
	return m_ptr->SetLOD(LODNew);
}

void Texture2D::CreateTexture(
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	hr = my::D3DContext::getSingleton().m_d3dDevice->CreateTexture(
		Width, Height, MipLevels, Usage, Format, Pool, &pTexture, NULL);
	if(FAILED(hr))
	{
		THROW_CUSEXCEPTION(str_printf("CreateTexture failed: %d, %d, %u", Width, Height, hr));
	}

	Create(pTexture);
}

void Texture2D::CreateAdjustedTexture(
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	hr = D3DXCreateTexture(my::D3DContext::getSingleton().m_d3dDevice,
		Width, Height, MipLevels, Usage, Format, Pool, &pTexture);
	if(FAILED(hr))
	{
		THROW_CUSEXCEPTION(str_printf("CreateAdjustedTexture failed: %d, %d, %u", Width, Height, hr));
	}

	Create(pTexture);
}

void Texture2D::CreateTextureFromFile(
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
	hr = D3DXCreateTextureFromFileEx(my::D3DContext::getSingleton().m_d3dDevice,
		pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
	if(FAILED(hr))
	{
		THROW_CUSEXCEPTION(str_printf("CreateTextureFromFile failed: %S, %d, %d, %u", pSrcFile, Width, Height, hr));
	}

	Create(pTexture);
}

void Texture2D::CreateTextureFromFileInMemory(
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
	hr = D3DXCreateTextureFromFileInMemoryEx(my::D3DContext::getSingleton().m_d3dDevice,
		pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
	if(FAILED(hr))
	{
		THROW_CUSEXCEPTION(str_printf("D3DXCreateTextureFromFileInMemoryEx failed: %p, %d, %d, %d, %u", pSrcData, SrcDataSize, Width, Height, hr));
	}

	Create(pTexture);
}

void Texture2D::AddDirtyRect(CONST CRect * pDirtyRect)
{
	V(static_cast<IDirect3DTexture9 *>(m_ptr)->AddDirtyRect(pDirtyRect));
}

D3DSURFACE_DESC Texture2D::GetLevelDesc(UINT Level) const
{
	D3DSURFACE_DESC desc;
	V(static_cast<IDirect3DTexture9 *>(m_ptr)->GetLevelDesc(Level, &desc));
	return desc;
}

CComPtr<IDirect3DSurface9> Texture2D::GetSurfaceLevel(UINT Level)
{
	CComPtr<IDirect3DSurface9> Surface;
	V(static_cast<IDirect3DTexture9 *>(m_ptr)->GetSurfaceLevel(Level, &Surface));
	return Surface;
}

D3DLOCKED_RECT Texture2D::LockRect(const RECT *pRect, DWORD Flags, UINT Level)
{
	D3DLOCKED_RECT LockedRect;
	if(FAILED(hr = static_cast<IDirect3DTexture9 *>(m_ptr)->LockRect(Level, &LockedRect, pRect, Flags)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return LockedRect;
}

void Texture2D::UnlockRect(UINT Level)
{
	V(static_cast<IDirect3DTexture9 *>(m_ptr)->UnlockRect(Level));
}

void CubeTexture::CreateCubeTexture(
	UINT EdgeLength,
	UINT Levels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;
	hr = my::D3DContext::getSingleton().m_d3dDevice->CreateCubeTexture(
		EdgeLength, Levels, Usage, Format, Pool, &pCubeTexture, NULL);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pCubeTexture);
}

void CubeTexture::CreateAdjustedCubeTexture(
	UINT Size,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;
	hr = D3DXCreateCubeTexture(my::D3DContext::getSingleton().m_d3dDevice,
		Size, MipLevels, Usage, Format, Pool, &pCubeTexture);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pCubeTexture);
}

void CubeTexture::CreateCubeTextureFromFile(
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
	hr = D3DXCreateCubeTextureFromFileEx(my::D3DContext::getSingleton().m_d3dDevice,
		pSrcFile, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pCubeTexture);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pCubeTexture);
}

void CubeTexture::CreateCubeTextureFromFileInMemory(
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
	hr = D3DXCreateCubeTextureFromFileInMemoryEx(my::D3DContext::getSingleton().m_d3dDevice,
		pSrcData, SrcDataSize, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pCubeTexture);
	if(FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pCubeTexture);
}

void CubeTexture::AddDirtyRect(
	D3DCUBEMAP_FACES FaceType,
	CONST RECT * pDirtyRect)
{
	V(static_cast<IDirect3DCubeTexture9 *>(m_ptr)->AddDirtyRect(FaceType, pDirtyRect));
}

CComPtr<IDirect3DSurface9> CubeTexture::GetCubeMapSurface(
	D3DCUBEMAP_FACES FaceType,
	UINT Level)
{
	CComPtr<IDirect3DSurface9> ret;
	V(static_cast<IDirect3DCubeTexture9 *>(m_ptr)->GetCubeMapSurface(FaceType, Level, &ret));
	return ret;
}

D3DSURFACE_DESC CubeTexture::GetLevelDesc(
	UINT Level) const
{
	D3DSURFACE_DESC desc;
	V(static_cast<IDirect3DCubeTexture9 *>(m_ptr)->GetLevelDesc(Level, &desc));
	return desc;
}

D3DLOCKED_RECT CubeTexture::LockRect(
	D3DCUBEMAP_FACES FaceType,
	const RECT *pRect,
	DWORD Flags,
	UINT Level)
{
	D3DLOCKED_RECT LockedRect;
	if(FAILED(hr = static_cast<IDirect3DCubeTexture9 *>(m_ptr)->LockRect(FaceType, Level, &LockedRect, pRect, Flags)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return LockedRect;
}

void CubeTexture::UnlockRect(
	D3DCUBEMAP_FACES FaceType,
	UINT Level)
{
	V(static_cast<IDirect3DCubeTexture9 *>(m_ptr)->UnlockRect(FaceType, Level));
}
