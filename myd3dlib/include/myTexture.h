// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "mySingleton.h"
#include <d3d9.h>
#include <d3dx9tex.h>
#include <atlbase.h>
#include <atltypes.h>

namespace my
{
	class Surface : public D3DDeviceResource<IDirect3DSurface9>
	{
	public:
		Surface(void)
		{
		}

		void Create(IDirect3DSurface9 * ptr);

		void CreateDepthStencilSurface(
			UINT Width,
			UINT Height,
			D3DFORMAT Format,
			D3DMULTISAMPLE_TYPE MultiSample = D3DMULTISAMPLE_NONE,
			DWORD MultisampleQuality = 0,
			BOOL Discard = TRUE);

		void CreateOffscreenPlainSurface(
			UINT Width,
			UINT Height,
			D3DFORMAT Format,
			D3DPOOL Pool = D3DPOOL_DEFAULT);

		CComPtr<IUnknown> GetContainer(REFIID riid);

		HDC GetDC(void);

		D3DSURFACE_DESC GetDesc(void);

		D3DLOCKED_RECT LockRect(const RECT *pRect, DWORD Flags = 0);

		void ReleaseDC(HDC hdc);

		void UnlockRect(void);
	};

	typedef boost::shared_ptr<Surface> SurfacePtr;

	class BaseTexture : public D3DDeviceResource<IDirect3DBaseTexture9>
	{
	public:
		BaseTexture(void)
		{
		}

		void Create(IDirect3DBaseTexture9 * ptr);

		void GenerateMipSubLevels(void);

		D3DTEXTUREFILTERTYPE GetAutoGenFilterType(void);

		DWORD GetLevelCount(void);

		DWORD GetLOD(void);

		void SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType);

		DWORD SetLOD(DWORD LODNew);

		virtual D3DSURFACE_DESC GetLevelDesc(UINT Level = 0) const = 0;
	};

	typedef boost::shared_ptr<BaseTexture> BaseTexturePtr;

	class Texture2D : public BaseTexture
	{
	public:
		Texture2D(void)
		{
		}

		void CreateTexture(
			UINT Width,
			UINT Height,
			UINT MipLevels = 0,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateAdjustedTexture(
			UINT Width,
			UINT Height,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateTextureFromFile(
			LPCTSTR pSrcFile,
			UINT Width = D3DX_DEFAULT_NONPOW2,
			UINT Height = D3DX_DEFAULT_NONPOW2,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DX_DEFAULT,
			DWORD MipFilter = D3DX_DEFAULT,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

		void CreateTextureFromFileInMemory(
			LPCVOID pSrcData,
			UINT SrcDataSize,
			UINT Width = D3DX_DEFAULT_NONPOW2,
			UINT Height = D3DX_DEFAULT_NONPOW2,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DX_DEFAULT,
			DWORD MipFilter = D3DX_DEFAULT,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

		void AddDirtyRect(CONST CRect * pDirtyRect = NULL);

		D3DSURFACE_DESC GetLevelDesc(UINT Level = 0) const;

		CComPtr<IDirect3DSurface9> GetSurfaceLevel(UINT Level = 0);

		D3DLOCKED_RECT LockRect(const RECT *pRect = NULL, DWORD Flags = 0, UINT Level = 0);

		void UnlockRect(UINT Level = 0);
	};

	typedef boost::shared_ptr<Texture2D> Texture2DPtr;

	class CubeTexture : public BaseTexture
	{
	public:
		CubeTexture(void)
		{
		}

		void CreateCubeTexture(
			UINT EdgeLength,
			UINT Levels = 0,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateAdjustedCubeTexture(
			UINT Size,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateCubeTextureFromFile(
			LPCTSTR pSrcFile,
			UINT Size = D3DX_DEFAULT,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DX_DEFAULT,
			DWORD MipFilter = D3DX_DEFAULT,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

		void CreateCubeTextureFromFileInMemory(
			LPCVOID pSrcData,
			UINT SrcDataSize,
			UINT Size = D3DX_DEFAULT,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DX_DEFAULT,
			DWORD MipFilter = D3DX_DEFAULT,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

		void AddDirtyRect(
			D3DCUBEMAP_FACES FaceType,
			CONST RECT * pDirtyRect = NULL);

		CComPtr<IDirect3DSurface9> GetCubeMapSurface(
			D3DCUBEMAP_FACES FaceType,
			UINT Level = 0);

		D3DSURFACE_DESC GetLevelDesc(
			UINT Level = 0) const;

		D3DLOCKED_RECT LockRect(
			D3DCUBEMAP_FACES FaceType,
			const RECT *pRect,
			DWORD Flags = 0,
			UINT Level = 0);

		void UnlockRect(
			D3DCUBEMAP_FACES FaceType,
			UINT Level = 0);
	};

	typedef boost::shared_ptr<CubeTexture> CubeTexturePtr;
}
