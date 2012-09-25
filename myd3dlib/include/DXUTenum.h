//--------------------------------------------------------------------------------------
// File: DXUTEnum.h
//
// Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef DXUT_ENUM_H
#define DXUT_ENUM_H

#define DXUTERR_NODIRECT3D				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0901)
#define DXUTERR_NOCOMPATIBLEDEVICES		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0902)

//--------------------------------------------------------------------------------------
// A growable array
//--------------------------------------------------------------------------------------
template<typename TYPE> class CGrowableArray
{
public:
            CGrowableArray()
            {
                m_pData = NULL; m_nSize = 0; m_nMaxSize = 0;
            }
            CGrowableArray( const CGrowableArray <TYPE>& a )
            {
                for( int i = 0; i < a.m_nSize; i++ ) Add( a.m_pData[i] );
            }
            ~CGrowableArray()
            {
                RemoveAll();
            }

    const TYPE& operator[]( int nIndex ) const
    {
        return GetAt( nIndex );
    }
    TYPE& operator[]( int nIndex )
    {
        return GetAt( nIndex );
    }

    CGrowableArray& operator=( const CGrowableArray <TYPE>& a )
    {
        if( this == &a ) return *this; RemoveAll(); for( int i = 0; i < a.m_nSize;
                                                         i++ ) Add( a.m_pData[i] ); return *this;
    }

    HRESULT SetSize( int nNewMaxSize );
    HRESULT Add( const TYPE& value );
    HRESULT Insert( int nIndex, const TYPE& value );
    HRESULT SetAt( int nIndex, const TYPE& value );
    TYPE& GetAt( int nIndex ) const
    {
        assert( nIndex >= 0 && nIndex < m_nSize ); return m_pData[nIndex];
    }
    int     GetSize() const
    {
        return m_nSize;
    }
    TYPE* GetData()
    {
        return m_pData;
    }
    bool    Contains( const TYPE& value )
    {
        return ( -1 != IndexOf( value ) );
    }

    int     IndexOf( const TYPE& value )
    {
        return ( m_nSize > 0 ) ? IndexOf( value, 0, m_nSize ) : -1;
    }
    int     IndexOf( const TYPE& value, int iStart )
    {
        return IndexOf( value, iStart, m_nSize - iStart );
    }
    int     IndexOf( const TYPE& value, int nIndex, int nNumElements );

    int     LastIndexOf( const TYPE& value )
    {
        return ( m_nSize > 0 ) ? LastIndexOf( value, m_nSize - 1, m_nSize ) : -1;
    }
    int     LastIndexOf( const TYPE& value, int nIndex )
    {
        return LastIndexOf( value, nIndex, nIndex + 1 );
    }
    int     LastIndexOf( const TYPE& value, int nIndex, int nNumElements );

    HRESULT Remove( int nIndex );
    void    RemoveAll()
    {
        SetSize( 0 );
    }
    void    Reset()
    {
        m_nSize = 0;
    }

protected:
    TYPE* m_pData;      // the actual array of data
    int m_nSize;        // # of elements (upperBound - 1)
    int m_nMaxSize;     // max allocated

    HRESULT SetSizeInternal( int nNewMaxSize );  // This version doesn't call ctor or dtor.
};

//--------------------------------------------------------------------------------------
// Implementation of CGrowableArray
//--------------------------------------------------------------------------------------

// This version doesn't call ctor or dtor.
template<typename TYPE> HRESULT CGrowableArray <TYPE>::SetSizeInternal( int nNewMaxSize )
{
    if( nNewMaxSize < 0 || ( nNewMaxSize > INT_MAX / sizeof( TYPE ) ) )
    {
        assert( false );
        return E_INVALIDARG;
    }

    if( nNewMaxSize == 0 )
    {
        // Shrink to 0 size & cleanup
        if( m_pData )
        {
            free( m_pData );
            m_pData = NULL;
        }

        m_nMaxSize = 0;
        m_nSize = 0;
    }
    else if( m_pData == NULL || nNewMaxSize > m_nMaxSize )
    {
        // Grow array
        int nGrowBy = ( m_nMaxSize == 0 ) ? 16 : m_nMaxSize;

        // Limit nGrowBy to keep m_nMaxSize less than INT_MAX
        if( ( UINT )m_nMaxSize + ( UINT )nGrowBy > ( UINT )INT_MAX )
            nGrowBy = INT_MAX - m_nMaxSize;

        nNewMaxSize = __max( nNewMaxSize, m_nMaxSize + nGrowBy );

        // Verify that (nNewMaxSize * sizeof(TYPE)) is not greater than UINT_MAX or the realloc will overrun
        if( sizeof( TYPE ) > UINT_MAX / ( UINT )nNewMaxSize )
            return E_INVALIDARG;

        TYPE* pDataNew = ( TYPE* )realloc( m_pData, nNewMaxSize * sizeof( TYPE ) );
        if( pDataNew == NULL )
            return E_OUTOFMEMORY;

        m_pData = pDataNew;
        m_nMaxSize = nNewMaxSize;
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::SetSize( int nNewMaxSize )
{
    int nOldSize = m_nSize;

    if( nOldSize > nNewMaxSize )
    {
        assert( m_pData );
        if( m_pData )
        {
            // Removing elements. Call dtor.

            for( int i = nNewMaxSize; i < nOldSize; ++i )
                m_pData[i].~TYPE();
        }
    }

    // Adjust buffer.  Note that there's no need to check for error
    // since if it happens, nOldSize == nNewMaxSize will be true.)
    HRESULT hr = SetSizeInternal( nNewMaxSize );

    if( nOldSize < nNewMaxSize )
    {
        assert( m_pData );
        if( m_pData )
        {
            // Adding elements. Call ctor.

            for( int i = nOldSize; i < nNewMaxSize; ++i )
                ::new ( &m_pData[i] ) TYPE;
        }
    }

    return hr;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::Add( const TYPE& value )
{
    HRESULT hr;
    if( FAILED( hr = SetSizeInternal( m_nSize + 1 ) ) )
        return hr;

    // Construct the new element
    ::new ( &m_pData[m_nSize] ) TYPE;

    // Assign
    m_pData[m_nSize] = value;
    ++m_nSize;

    return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::Insert( int nIndex, const TYPE& value )
{
    HRESULT hr;

    // Validate index
    if( nIndex < 0 ||
        nIndex > m_nSize )
    {
        assert( false );
        return E_INVALIDARG;
    }

    // Prepare the buffer
    if( FAILED( hr = SetSizeInternal( m_nSize + 1 ) ) )
        return hr;

    // Shift the array
    MoveMemory( &m_pData[nIndex + 1], &m_pData[nIndex], sizeof( TYPE ) * ( m_nSize - nIndex ) );

    // Construct the new element
    ::new ( &m_pData[nIndex] ) TYPE;

    // Set the value and increase the size
    m_pData[nIndex] = value;
    ++m_nSize;

    return S_OK;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::SetAt( int nIndex, const TYPE& value )
{
    // Validate arguments
    if( nIndex < 0 ||
        nIndex >= m_nSize )
    {
        assert( false );
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
template<typename TYPE> int CGrowableArray <TYPE>::IndexOf( const TYPE& value, int iStart, int nNumElements )
{
    // Validate arguments
    if( iStart < 0 ||
        iStart >= m_nSize ||
        nNumElements < 0 ||
        iStart + nNumElements > m_nSize )
    {
        assert( false );
        return -1;
    }

    // Search
    for( int i = iStart; i < ( iStart + nNumElements ); i++ )
    {
        if( value == m_pData[i] )
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
template<typename TYPE> int CGrowableArray <TYPE>::LastIndexOf( const TYPE& value, int iEnd, int nNumElements )
{
    // Validate arguments
    if( iEnd < 0 ||
        iEnd >= m_nSize ||
        nNumElements < 0 ||
        iEnd - nNumElements < 0 )
    {
        assert( false );
        return -1;
    }

    // Search
    for( int i = iEnd; i > ( iEnd - nNumElements ); i-- )
    {
        if( value == m_pData[i] )
            return i;
    }

    // Not found
    return -1;
}


//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CGrowableArray <TYPE>::Remove( int nIndex )
{
    if( nIndex < 0 ||
        nIndex >= m_nSize )
    {
        assert( false );
        return E_INVALIDARG;
    }

    // Destruct the element to be removed
    m_pData[nIndex].~TYPE();

    // Compact the array and decrease the size
    MoveMemory( &m_pData[nIndex], &m_pData[nIndex + 1], sizeof( TYPE ) * ( m_nSize - ( nIndex + 1 ) ) );
    --m_nSize;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Finding valid device settings
//--------------------------------------------------------------------------------------
enum DXUT_MATCH_TYPE
{
    DXUTMT_IGNORE_INPUT = 0,  // Use the closest valid value to a default 
    DXUTMT_PRESERVE_INPUT,    // Use input without change, but may cause no valid device to be found
    DXUTMT_CLOSEST_TO_INPUT   // Use the closest valid value to the input 
};

struct DXUTMatchOptions
{
    DXUT_MATCH_TYPE eAPIVersion;
    DXUT_MATCH_TYPE eAdapterOrdinal;
    DXUT_MATCH_TYPE eOutput;
    DXUT_MATCH_TYPE eDeviceType;
    DXUT_MATCH_TYPE eWindowed;
    DXUT_MATCH_TYPE eAdapterFormat;
    DXUT_MATCH_TYPE eVertexProcessing;
    DXUT_MATCH_TYPE eResolution;
    DXUT_MATCH_TYPE eBackBufferFormat;
    DXUT_MATCH_TYPE eBackBufferCount;
    DXUT_MATCH_TYPE eMultiSample;
    DXUT_MATCH_TYPE eSwapEffect;
    DXUT_MATCH_TYPE eDepthFormat;
    DXUT_MATCH_TYPE eStencilFormat;
    DXUT_MATCH_TYPE ePresentFlags;
    DXUT_MATCH_TYPE eRefreshRate;
    DXUT_MATCH_TYPE ePresentInterval;
};

struct DXUTD3D9DeviceSettings
{
	UINT AdapterOrdinal;
	D3DDEVTYPE DeviceType;
	D3DFORMAT AdapterFormat;
	DWORD BehaviorFlags;
	D3DPRESENT_PARAMETERS pp;
};


//--------------------------------------------------------------------------------------
// Functions to get bit depth from formats
//--------------------------------------------------------------------------------------
UINT WINAPI DXUTGetD3D9ColorChannelBits( D3DFORMAT fmt );
UINT WINAPI DXUTGetAlphaChannelBits( D3DFORMAT fmt );
UINT WINAPI DXUTGetStencilBits( D3DFORMAT fmt );
UINT WINAPI DXUTGetDepthBits( D3DFORMAT fmt );


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
class CD3D9EnumAdapterInfo;
class CD3D9EnumDeviceInfo;
struct CD3D9EnumDeviceSettingsCombo;
struct CD3D9EnumDSMSConflict;


//--------------------------------------------------------------------------------------
// Enumerates available Direct3D9 adapters, devices, modes, etc.
// Use DXUTGetD3D9Enumeration() to access global instance
//--------------------------------------------------------------------------------------
class CD3D9Enumeration
{
public:
    // These should be called before Enumerate(). 
    //
    // Use these calls and the IsDeviceAcceptable to control the contents of 
    // the enumeration object, which affects the device selection and the device settings dialog.
    void                    SetRequirePostPixelShaderBlending( bool bRequire )
    {
        m_bRequirePostPixelShaderBlending = bRequire;
    }
    void                    SetResolutionMinMax( UINT nMinWidth, UINT nMinHeight, UINT nMaxWidth, UINT nMaxHeight );
    void                    SetRefreshMinMax( UINT nMin, UINT nMax );
    void                    SetMultisampleQualityMax( UINT nMax );
    void                    GetPossibleVertexProcessingList( bool* pbSoftwareVP, bool* pbHardwareVP,
                                                             bool* pbPureHarewareVP, bool* pbMixedVP );
    void                    SetPossibleVertexProcessingList( bool bSoftwareVP, bool bHardwareVP, bool bPureHarewareVP,
                                                             bool bMixedVP );
    CGrowableArray <D3DFORMAT>* GetPossibleDepthStencilFormatList();
    CGrowableArray <D3DMULTISAMPLE_TYPE>* GetPossibleMultisampleTypeList();
    CGrowableArray <UINT>* GetPossiblePresentIntervalList();
    void                    ResetPossibleDepthStencilFormats();
    void                    ResetPossibleMultisampleTypeList();
    void                    ResetPossiblePresentIntervalList();

    // Call Enumerate() to enumerate available D3D adapters, devices, modes, etc.
    bool                    HasEnumerated()
    {
        return m_bHasEnumerated;
    }
    HRESULT                 Enumerate( IDirect3D9 * pD3D, HWND hWnd );

    // These should be called after Enumerate() is called
    CGrowableArray <CD3D9EnumAdapterInfo*>* GetAdapterInfoList();
    CD3D9EnumAdapterInfo* GetAdapterInfo( UINT AdapterOrdinal );
    CD3D9EnumDeviceInfo* GetDeviceInfo( UINT AdapterOrdinal, D3DDEVTYPE DeviceType );
    CD3D9EnumDeviceSettingsCombo* GetDeviceSettingsCombo( DXUTD3D9DeviceSettings* pD3D9DeviceSettings )
    {
        return GetDeviceSettingsCombo( pD3D9DeviceSettings->AdapterOrdinal, pD3D9DeviceSettings->DeviceType,
                                       pD3D9DeviceSettings->AdapterFormat, pD3D9DeviceSettings->pp.BackBufferFormat,
                                       pD3D9DeviceSettings->pp.Windowed );
    }
    CD3D9EnumDeviceSettingsCombo* GetDeviceSettingsCombo( UINT AdapterOrdinal, D3DDEVTYPE DeviceType,
                                                          D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                                          BOOL Windowed );

                            ~CD3D9Enumeration();

protected:
    // Use DXUTGetD3D9Enumeration() to access global instance
                            CD3D9Enumeration();

    bool m_bHasEnumerated;
    IDirect3D9* m_pD3D;
	HWND m_hWnd;
	virtual bool IsDeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed) = 0;
    bool m_bRequirePostPixelShaderBlending;
    CGrowableArray <D3DFORMAT> m_DepthStencilPossibleList;
    CGrowableArray <D3DMULTISAMPLE_TYPE> m_MultiSampleTypeList;
    CGrowableArray <UINT> m_PresentIntervalList;

    bool m_bSoftwareVP;
    bool m_bHardwareVP;
    bool m_bPureHarewareVP;
    bool m_bMixedVP;

    UINT m_nMinWidth;
    UINT m_nMaxWidth;
    UINT m_nMinHeight;
    UINT m_nMaxHeight;
    UINT m_nRefreshMin;
    UINT m_nRefreshMax;
    UINT m_nMultisampleQualityMax;

    // Array of CD3D9EnumAdapterInfo* with unique AdapterOrdinals
    CGrowableArray <CD3D9EnumAdapterInfo*> m_AdapterInfoList;

    HRESULT                 EnumerateDevices( CD3D9EnumAdapterInfo* pAdapterInfo,
                                              CGrowableArray <D3DFORMAT>* pAdapterFormatList );
    HRESULT                 EnumerateDeviceCombos( CD3D9EnumAdapterInfo* pAdapterInfo,
                                                   CD3D9EnumDeviceInfo* pDeviceInfo,
                                                   CGrowableArray <D3DFORMAT>* pAdapterFormatList );
    void                    BuildDepthStencilFormatList( CD3D9EnumDeviceSettingsCombo* pDeviceCombo );
    void                    BuildMultiSampleTypeList( CD3D9EnumDeviceSettingsCombo* pDeviceCombo );
    void                    BuildDSMSConflictList( CD3D9EnumDeviceSettingsCombo* pDeviceCombo );
    void                    BuildPresentIntervalList( CD3D9EnumDeviceInfo* pDeviceInfo,
                                                      CD3D9EnumDeviceSettingsCombo* pDeviceCombo );
    void                    ClearAdapterInfoList();
};

CD3D9Enumeration*   WINAPI DXUTGetD3D9Enumeration( bool bForceEnumerate = false );


//--------------------------------------------------------------------------------------
// A class describing an adapter which contains a unique adapter ordinal 
// that is installed on the system
//--------------------------------------------------------------------------------------
class CD3D9EnumAdapterInfo
{
public:
            ~CD3D9EnumAdapterInfo();

    UINT AdapterOrdinal;
    D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
    WCHAR   szUniqueDescription[256];

    CGrowableArray <D3DDISPLAYMODE> displayModeList; // Array of supported D3DDISPLAYMODEs
    CGrowableArray <CD3D9EnumDeviceInfo*> deviceInfoList; // Array of CD3D9EnumDeviceInfo* with unique supported DeviceTypes
};


//--------------------------------------------------------------------------------------
// A class describing a Direct3D device that contains a 
//       unique supported device type 
//--------------------------------------------------------------------------------------
class CD3D9EnumDeviceInfo
{
public:
    ~CD3D9EnumDeviceInfo();

    UINT AdapterOrdinal;
    D3DDEVTYPE DeviceType;
    D3DCAPS9 Caps;

    // List of CD3D9EnumDeviceSettingsCombo* with a unique set 
    // of AdapterFormat, BackBufferFormat, and Windowed
    CGrowableArray <CD3D9EnumDeviceSettingsCombo*> deviceSettingsComboList;
};


//--------------------------------------------------------------------------------------
// A struct describing device settings that contains a unique combination of 
// adapter format, back buffer format, and windowed that is compatible with a 
// particular Direct3D device and the app.
//--------------------------------------------------------------------------------------
struct CD3D9EnumDeviceSettingsCombo
{
    UINT AdapterOrdinal;
    D3DDEVTYPE DeviceType;
    D3DFORMAT AdapterFormat;
    D3DFORMAT BackBufferFormat;
    BOOL Windowed;

    CGrowableArray <D3DFORMAT> depthStencilFormatList; // List of D3DFORMATs
    CGrowableArray <D3DMULTISAMPLE_TYPE> multiSampleTypeList; // List of D3DMULTISAMPLE_TYPEs
    CGrowableArray <DWORD> multiSampleQualityList; // List of number of quality levels for each multisample type
    CGrowableArray <UINT> presentIntervalList; // List of D3DPRESENT flags
    CGrowableArray <CD3D9EnumDSMSConflict> DSMSConflictList; // List of CD3D9EnumDSMSConflict

    CD3D9EnumAdapterInfo* pAdapterInfo;
    CD3D9EnumDeviceInfo* pDeviceInfo;
};


//--------------------------------------------------------------------------------------
// A depth/stencil buffer format that is incompatible with a
// multisample type.
//--------------------------------------------------------------------------------------
struct CD3D9EnumDSMSConflict
{
    D3DFORMAT DSFormat;
    D3DMULTISAMPLE_TYPE MSType;
};


#endif
