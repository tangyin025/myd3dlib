#pragma once

#include <boost/shared_ptr.hpp>
#include "mySingleton.h"
#include <d3d9.h>
#include <atlbase.h>
#include <atltypes.h>
#include <DXUT.h>
#include "myException.h"

namespace my
{
	class Surface : public DeviceRelatedObject<IDirect3DSurface9>
	{
	public:
		Surface(void)
		{
		}

		void Create(IDirect3DSurface9 * ptr)
		{
			_ASSERT(!m_ptr);

			m_ptr = ptr;
		}

		void CreateDepthStencilSurface(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Width,
			UINT Height,
			D3DFORMAT Format,
			D3DMULTISAMPLE_TYPE MultiSample = D3DMULTISAMPLE_NONE,
			DWORD MultisampleQuality = 0,
			BOOL Discard = TRUE);

		void CreateOffscreenPlainSurface(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Width,
			UINT Height,
			D3DFORMAT Format,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		CComPtr<IUnknown> GetContainer(REFIID riid)
		{
			CComPtr<IUnknown> Container;
			V(m_ptr->GetContainer(riid, (void **)&Container));
			return Container;
		}

		HDC GetDC(void)
		{
			HDC hdc;
			V(m_ptr->GetDC(&hdc));
			return hdc;
		}

		D3DSURFACE_DESC GetDesc(void)
		{
			D3DSURFACE_DESC desc;
			V(m_ptr->GetDesc(&desc));
			return desc;
		}

		D3DLOCKED_RECT LockRect(const CRect & rect, DWORD Flags = 0)
		{
			_ASSERT(!IsRectEmpty(&rect));

			D3DLOCKED_RECT lr;
			if(FAILED(hr = m_ptr->LockRect(&lr, &rect, Flags)))
			{
				THROW_D3DEXCEPTION(hr);
			}
			return lr;
		}

		void ReleaseDC(HDC hdc)
		{
			V(m_ptr->ReleaseDC(hdc));
		}

		void UnlockRect(void)
		{
			V(m_ptr->UnlockRect());
		}
	};

	typedef boost::shared_ptr<Surface> SurfacePtr;

	class BaseTexture : public DeviceRelatedObject<IDirect3DBaseTexture9>
	{
	public:
		BaseTexture(void)
		{
		}

		void Create(IDirect3DBaseTexture9 * ptr)
		{
			_ASSERT(!m_ptr);

			m_ptr = ptr;
		}

		void GenerateMipSubLevels(void)
		{
			m_ptr->GenerateMipSubLevels();
		}

		D3DTEXTUREFILTERTYPE GetAutoGenFilterType(void)
		{
			return m_ptr->GetAutoGenFilterType();
		}

		DWORD GetLevelCount(void)
		{
			return m_ptr->GetLevelCount();
		}

		DWORD GetLOD(void)
		{
			return m_ptr->GetLOD();
		}

		void SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
		{
			V(m_ptr->SetAutoGenFilterType(FilterType));
		}

		DWORD SetLOD(DWORD LODNew)
		{
			return m_ptr->SetLOD(LODNew);
		}
	};

	class Texture : public BaseTexture
	{
	public:
		Texture(void)
		{
		}

		void CreateTexture(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Width,
			UINT Height,
			UINT MipLevels = 0,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateAdjustedTexture(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Width,
			UINT Height,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateTextureFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcFile,
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
			LPDIRECT3DDEVICE9 pDevice,
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

		void AddDirtyRect(CONST CRect * pDirtyRect = NULL)
		{
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->AddDirtyRect(pDirtyRect));
		}

		D3DSURFACE_DESC GetLevelDesc(UINT Level = 0)
		{
			D3DSURFACE_DESC desc;
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->GetLevelDesc(Level, &desc));
			return desc;
		}

		CComPtr<IDirect3DSurface9> GetSurfaceLevel(UINT Level)
		{
			CComPtr<IDirect3DSurface9> Surface;
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->GetSurfaceLevel(Level, &Surface));
			return Surface;
		}

		D3DLOCKED_RECT LockRect(const CRect & rect, DWORD Flags = 0, UINT Level = 0)
		{
			_ASSERT(!IsRectEmpty(&rect)); // ! D3DPOOL_MANAGED unsupport locking empty rect

			D3DLOCKED_RECT LockedRect;
			if(FAILED(hr = static_cast<IDirect3DTexture9 *>(m_ptr)->LockRect(Level, &LockedRect, &rect, Flags)))
			{
				THROW_D3DEXCEPTION(hr);
			}
			return LockedRect;
		}

		void UnlockRect(UINT Level = 0)
		{
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->UnlockRect(Level));
		}
	};

	typedef boost::shared_ptr<Texture> TexturePtr;

	class CubeTexture : public BaseTexture
	{
	public:
		CubeTexture(void)
		{
		}

		void CreateCubeTexture(
			LPDIRECT3DDEVICE9 pDevice,
			UINT EdgeLength,
			UINT Levels,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CubeAdjustedTexture(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Size,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		void CreateCubeTextureFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcFile,
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
			LPDIRECT3DDEVICE9 pDevice,
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
	};

	typedef boost::shared_ptr<CubeTexture> CubeTexturePtr;
}
