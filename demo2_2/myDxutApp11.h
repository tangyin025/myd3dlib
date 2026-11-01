// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "myThread.h"
#include <boost/signals2.hpp>
#include <dxgi.h>
#include <d3d11.h>
#include <assert.h>
#include <new.h>      // for placement new
#include <atlbase.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

//--------------------------------------------------------------------------------------
// A growable array
//--------------------------------------------------------------------------------------
template<typename TYPE> class CGrowableArray2
{
public:
    CGrowableArray2() { m_pData = NULL; m_nSize = 0; m_nMaxSize = 0; }
    CGrowableArray2(const CGrowableArray2<TYPE>& a) { for (int i = 0; i < a.m_nSize; i++) Add(a.m_pData[i]); }
    ~CGrowableArray2() { RemoveAll(); }

    const TYPE& operator[](int nIndex) const { return GetAt(nIndex); }
    TYPE& operator[](int nIndex) { return GetAt(nIndex); }

    CGrowableArray2& operator=(const CGrowableArray2<TYPE>& a) { if (this == &a) return *this; RemoveAll(); for (int i = 0; i < a.m_nSize; i++) Add(a.m_pData[i]); return *this; }

    HRESULT SetSize(int nNewMaxSize);
    HRESULT Add(const TYPE& value);
    HRESULT Insert(int nIndex, const TYPE& value);
    HRESULT SetAt(int nIndex, const TYPE& value);
    TYPE& GetAt(int nIndex) const { assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex]; }
    int     GetSize() const { return m_nSize; }
    TYPE* GetData() { return m_pData; }
    bool    Contains(const TYPE& value) { return (-1 != IndexOf(value)); }

    int     IndexOf(const TYPE& value) { return (m_nSize > 0) ? IndexOf(value, 0, m_nSize) : -1; }
    int     IndexOf(const TYPE& value, int iStart) { return IndexOf(value, iStart, m_nSize - iStart); }
    int     IndexOf(const TYPE& value, int nIndex, int nNumElements);

    int     LastIndexOf(const TYPE& value) { return (m_nSize > 0) ? LastIndexOf(value, m_nSize - 1, m_nSize) : -1; }
    int     LastIndexOf(const TYPE& value, int nIndex) { return LastIndexOf(value, nIndex, nIndex + 1); }
    int     LastIndexOf(const TYPE& value, int nIndex, int nNumElements);

    HRESULT Remove(int nIndex);
    void    RemoveAll() { SetSize(0); }
    void	Reset() { m_nSize = 0; }

protected:
    TYPE* m_pData;      // the actual array of data
    int m_nSize;        // # of elements (upperBound - 1)
    int m_nMaxSize;     // max allocated

    HRESULT SetSizeInternal(int nNewMaxSize);  // This version doesn't call ctor or dtor.
};

//--------------------------------------------------------------------------------------
// Implementation of CGrowableArray2
//--------------------------------------------------------------------------------------

// This version doesn't call ctor or dtor.
template<typename TYPE> HRESULT CGrowableArray2 <TYPE>::SetSizeInternal(int nNewMaxSize)
{
    if (nNewMaxSize < 0 || (nNewMaxSize > INT_MAX / sizeof(TYPE)))
    {
        assert(false);
        return E_INVALIDARG;
    }

    if (nNewMaxSize == 0)
    {
        // Shrink to 0 size & cleanup
        if (m_pData)
        {
            free(m_pData);
            m_pData = NULL;
        }

        m_nMaxSize = 0;
        m_nSize = 0;
    }
    else if (m_pData == NULL || nNewMaxSize > m_nMaxSize)
    {
        // Grow array
        int nGrowBy = (m_nMaxSize == 0) ? 16 : m_nMaxSize;

        // Limit nGrowBy to keep m_nMaxSize less than INT_MAX
        if ((UINT)m_nMaxSize + (UINT)nGrowBy > (UINT)INT_MAX)
            nGrowBy = INT_MAX - m_nMaxSize;

        nNewMaxSize = __max(nNewMaxSize, m_nMaxSize + nGrowBy);

        // Verify that (nNewMaxSize * sizeof(TYPE)) is not greater than UINT_MAX or the realloc will overrun
        if (sizeof(TYPE) > UINT_MAX / (UINT)nNewMaxSize)
            return E_INVALIDARG;

        TYPE* pDataNew = (TYPE*)realloc(m_pData, nNewMaxSize * sizeof(TYPE));
        if (pDataNew == NULL)
            return E_OUTOFMEMORY;

        m_pData = pDataNew;
        m_nMaxSize = nNewMaxSize;
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray2 <TYPE>::SetSize(int nNewMaxSize)
{
    int nOldSize = m_nSize;

    if (nOldSize > nNewMaxSize)
    {
        assert(m_pData);
        if (m_pData)
        {
            // Removing elements. Call dtor.

            for (int i = nNewMaxSize; i < nOldSize; ++i)
                m_pData[i].~TYPE();
        }
    }

    // Adjust buffer.  Note that there's no need to check for error
    // since if it happens, nOldSize == nNewMaxSize will be true.)
    HRESULT hr = SetSizeInternal(nNewMaxSize);

    if (nOldSize < nNewMaxSize)
    {
        assert(m_pData);
        if (m_pData)
        {
            // Adding elements. Call ctor.

            for (int i = nOldSize; i < nNewMaxSize; ++i)
                ::new (&m_pData[i]) TYPE;
        }
    }

    return hr;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray2 <TYPE>::Add(const TYPE& value)
{
    HRESULT hr;
    if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
        return hr;

    assert(m_pData != NULL);

    // Construct the new element
    ::new (&m_pData[m_nSize]) TYPE;

    // Assign
    m_pData[m_nSize] = value;
    ++m_nSize;

    return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray2 <TYPE>::Insert(int nIndex, const TYPE& value)
{
    HRESULT hr;

    // Validate index
    if (nIndex < 0 ||
        nIndex > m_nSize)
    {
        assert(false);
        return E_INVALIDARG;
    }

    // Prepare the buffer
    if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
        return hr;

    // Shift the array
    MoveMemory(&m_pData[nIndex + 1], &m_pData[nIndex], sizeof(TYPE) * (m_nSize - nIndex));

    // Construct the new element
    ::new (&m_pData[nIndex]) TYPE;

    // Set the value and increase the size
    m_pData[nIndex] = value;
    ++m_nSize;

    return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray2 <TYPE>::SetAt(int nIndex, const TYPE& value)
{
    // Validate arguments
    if (nIndex < 0 ||
        nIndex >= m_nSize)
    {
        assert(false);
        return E_INVALIDARG;
    }

    m_pData[nIndex] = value;
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Searches for the specified value and returns the index of the first occurrence
// within the section of the data array that extends from iStart and contains the 
// specified number of elements. Returns -1 if value is not found within the given 
// section.
//--------------------------------------------------------------------------------------
template<typename TYPE> int CGrowableArray2 <TYPE>::IndexOf(const TYPE& value, int iStart, int nNumElements)
{
    // Validate arguments
    if (iStart < 0 ||
        iStart >= m_nSize ||
        nNumElements < 0 ||
        iStart + nNumElements > m_nSize)
    {
        assert(false);
        return -1;
    }

    // Search
    for (int i = iStart; i < (iStart + nNumElements); i++)
    {
        if (value == m_pData[i])
            return i;
    }

    // Not found
    return -1;
}


//--------------------------------------------------------------------------------------
// Searches for the specified value and returns the index of the last occurrence
// within the section of the data array that contains the specified number of elements
// and ends at iEnd. Returns -1 if value is not found within the given section.
//--------------------------------------------------------------------------------------
template<typename TYPE> int CGrowableArray2 <TYPE>::LastIndexOf(const TYPE& value, int iEnd, int nNumElements)
{
    // Validate arguments
    if (iEnd < 0 ||
        iEnd >= m_nSize ||
        nNumElements < 0 ||
        iEnd - nNumElements < 0)
    {
        assert(false);
        return -1;
    }

    // Search
    for (int i = iEnd; i > (iEnd - nNumElements); i--)
    {
        if (value == m_pData[i])
            return i;
    }

    // Not found
    return -1;
}



//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray2 <TYPE>::Remove(int nIndex)
{
    if (nIndex < 0 ||
        nIndex >= m_nSize)
    {
        assert(false);
        return E_INVALIDARG;
    }

    // Destruct the element to be removed
    m_pData[nIndex].~TYPE();

    // Compact the array and decrease the size
    MoveMemory(&m_pData[nIndex], &m_pData[nIndex + 1], sizeof(TYPE) * (m_nSize - (nIndex + 1)));
    --m_nSize;

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------
struct DXUTD3D11DeviceSettings
{
    UINT AdapterOrdinal;
    D3D_DRIVER_TYPE DriverType;
    UINT Output;
    DXGI_SWAP_CHAIN_DESC sd;
    UINT32 CreateFlags;
    UINT32 SyncInterval;
    DWORD PresentFlags;
    bool AutoCreateDepthStencil; // DXUT will create the depth stencil resource and view if true
    DXGI_FORMAT AutoDepthStencilFormat;
    D3D_FEATURE_LEVEL DeviceFeatureLevel;
};

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
class CD3D11EnumOutputInfo;
class CD3D11EnumDeviceInfo;
struct CD3D11EnumDeviceSettingsCombo;

#define DXGI_MAX_DEVICE_IDENTIFIER_STRING 128

//--------------------------------------------------------------------------------------
// A class describing an adapter which contains a unique adapter ordinal 
// that is installed on the system
//--------------------------------------------------------------------------------------
class CD3D11EnumAdapterInfo
{
    const CD3D11EnumAdapterInfo& operator = (const CD3D11EnumAdapterInfo& rhs);

public:
    ~CD3D11EnumAdapterInfo();

    UINT AdapterOrdinal;
    DXGI_ADAPTER_DESC AdapterDesc;
    WCHAR szUniqueDescription[DXGI_MAX_DEVICE_IDENTIFIER_STRING];
    IDXGIAdapter* m_pAdapter;
    bool bAdapterUnavailable;

    CGrowableArray2<CD3D11EnumOutputInfo*> outputInfoList; // Array of CD3D11EnumOutputInfo*
    CGrowableArray2<CD3D11EnumDeviceInfo*> deviceInfoList; // Array of CD3D11EnumDeviceInfo*
    // List of CD3D11EnumDeviceSettingsCombo* with a unique set 
    // of BackBufferFormat, and Windowed
    CGrowableArray2<CD3D11EnumDeviceSettingsCombo*> deviceSettingsComboList;
};

class CD3D11EnumOutputInfo
{
    const CD3D11EnumOutputInfo& operator = (const CD3D11EnumOutputInfo& rhs);

public:
    ~CD3D11EnumOutputInfo();

    UINT AdapterOrdinal;
    UINT Output;
    IDXGIOutput* m_pOutput;
    DXGI_OUTPUT_DESC Desc;

    CGrowableArray2 <DXGI_MODE_DESC> displayModeList; // Array of supported D3DDISPLAYMODEs
};

//--------------------------------------------------------------------------------------
// A class describing a Direct3D10 device that contains a 
//       unique supported driver type
//--------------------------------------------------------------------------------------
class CD3D11EnumDeviceInfo
{
    const CD3D11EnumDeviceInfo& operator =(const CD3D11EnumDeviceInfo& rhs);

public:
    ~CD3D11EnumDeviceInfo();

    UINT AdapterOrdinal;
    D3D_DRIVER_TYPE DeviceType;
    D3D_FEATURE_LEVEL SelectedLevel;
    D3D_FEATURE_LEVEL MaxLevel;
    BOOL ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x;
};

//--------------------------------------------------------------------------------------
// A struct describing device settings that contains a unique combination of 
// adapter format, back buffer format, and windowed that is compatible with a 
// particular Direct3D device and the app.
//--------------------------------------------------------------------------------------
struct CD3D11EnumDeviceSettingsCombo
{
    UINT AdapterOrdinal;
    D3D_DRIVER_TYPE DeviceType;
    DXGI_FORMAT BackBufferFormat;
    BOOL Windowed;
    UINT Output;

    CGrowableArray2 <UINT> multiSampleCountList; // List of valid sampling counts (multisampling)
    CGrowableArray2 <UINT> multiSampleQualityList; // List of number of quality levels for each multisample count

    CD3D11EnumAdapterInfo* pAdapterInfo;
    CD3D11EnumDeviceInfo* pDeviceInfo;
    CD3D11EnumOutputInfo* pOutputInfo;
};

namespace my
{
	class DxutWindow11
		: public Window
	{
	public:
		bool m_Minimized;

		bool m_Maximized;

		bool m_InSizeMove;

		typedef boost::signals2::signal<void(bool)> ActivateEvent;

		ActivateEvent m_ActivateEvent;

	public:
		DxutWindow11(void)
			: m_Minimized(false)
			, m_Maximized(false)
			, m_InSizeMove(false)
		{
		}

		BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);
	};

	class DxutApp11
		: public Application
	{
	public:
		boost::shared_ptr<DxutWindow11> m_wnd;

        DXUTD3D11DeviceSettings m_DeviceSettings;

        CComPtr<IDXGIFactory> m_DXGIFactory;

        CComPtr<IDXGIAdapter> m_Adapter;

        CComPtr<ID3D11Device> m_d3d11Device;

        CComPtr<ID3D11DeviceContext> m_d3dImmediateContext;

        D3D_FEATURE_LEVEL m_FeatureLevel;

        //UINT m_FullScreenBackBufferWidthAtModeChange;

        //UINT m_FullScreenBackBufferHeightAtModeChange;

        //UINT m_WindowBackBufferWidthAtModeChange;

        //UINT m_WindowBackBufferHeightAtModeChange;

	public:
		DxutApp11(void)
            : m_FeatureLevel(D3D_FEATURE_LEVEL_11_0)
		{
		}

		bool IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo* AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo* DeviceInfo,
			DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
		{
			return true;
		}

		bool ModifyDeviceSettings(DXUTD3D11DeviceSettings* pDeviceSettings, void* pUserContext)
		{
			return true;
		}

		HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
			void* pUserContext)
		{
			return S_OK;
		}

		HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
			const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
		{
			return S_OK;
		}

		void OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
		{
		}

		void OnFrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
			double fTime, float fElapsedTime, void* pUserContext)
		{
			//// Clear render target and the depth stencil 
			//float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };

			//ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
			//ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
			//pd3dImmediateContext->ClearRenderTargetView(pRTV, ClearColor);
			//pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
		}

		void OnD3D11ReleasingSwapChain(void* pUserContext)
		{
		}

		void OnD3D11DestroyDevice(void* pUserContext)
		{
		}

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
			bool* pbNoFurtherProcessing, void* pUserContext)
		{
			return 0;
		}

		int Run(void);

        void CreateDevice(D3D_FEATURE_LEVEL reqFL, bool bWindowed = true, int nSuggestedWidth = 0, int nSuggestedHeight = 0);

        void ChangeDevice(DXUTD3D11DeviceSettings* pNewDeviceSettings, bool bForceRecreate, bool bClipWindowToSingleAdapter);

        void Cleanup3DEnvironment11(bool bReleaseSettings);

        HRESULT Create3DEnvironment(DXUTD3D11DeviceSettings* pNewDeviceSettings);

        //HRESULT DXUTSnapDeviceSettingsToEnumDevice(DXUTD3D11DeviceSettings* pDeviceSettings, bool forceEnum, D3D_FEATURE_LEVEL forceFL);
    };
}
