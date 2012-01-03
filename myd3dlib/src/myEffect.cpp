
#include "stdafx.h"
#include "myd3dlib.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

namespace my
{
	void VertexShader::OnD3D9DestroyDevice(void)
	{
		SAFE_RELEASE(m_pConstantTable);
		SAFE_RELEASE(m_ptr);
		m_Device.Release();
	}

	VertexShaderPtr VertexShader::CreateVertexShader(
		LPDIRECT3DDEVICE9 pDevice,
		LPCSTR pSrcData,
		UINT srcDataLen,
		LPCSTR pFunctionName,
		LPCSTR pProfile,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/)
	{
		CComPtr<ID3DXBuffer> Shader;
		CComPtr<ID3DXBuffer> ErrorMsgs;
		LPD3DXCONSTANTTABLE pConstantTable = NULL;
		HRESULT hres = D3DXCompileShader(
			pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		LPDIRECT3DVERTEXSHADER9 pVS = NULL;
		hres = pDevice->CreateVertexShader((DWORD *)Shader->GetBufferPointer(), &pVS);
		if(FAILED(hres))
		{
			SAFE_RELEASE(pConstantTable);
			THROW_D3DEXCEPTION(hres);
		}

		return VertexShaderPtr(new VertexShader(pVS, pConstantTable, pDevice));
	}

	VertexShaderPtr VertexShader::CreateVertexShaderFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		LPCSTR pSrcFile,
		LPCSTR pFunctionName,
		LPCSTR pProfile,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/)
	{
		CComPtr<ID3DXBuffer> Shader;
		CComPtr<ID3DXBuffer> ErrorMsgs;
		LPD3DXCONSTANTTABLE pConstantTable = NULL;
		HRESULT hres = D3DXCompileShaderFromFileA(
			pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		LPDIRECT3DVERTEXSHADER9 pVS = NULL;
		hres = pDevice->CreateVertexShader((DWORD *)Shader->GetBufferPointer(), &pVS);
		if(FAILED(hres))
		{
			SAFE_RELEASE(pConstantTable);
			THROW_D3DEXCEPTION(hres);
		}

		return VertexShaderPtr(new VertexShader(pVS, pConstantTable, pDevice));
	}

	void PixelShader::OnD3D9DestroyDevice(void)
	{
		SAFE_RELEASE(m_pConstantTable);
		SAFE_RELEASE(m_ptr);
		m_Device.Release();
	}

	PixelShaderPtr PixelShader::CreatePixelShader(
		LPDIRECT3DDEVICE9 pDevice,
		LPCSTR pSrcData,
		UINT srcDataLen,
		LPCSTR pFunctionName,
		LPCSTR pProfile,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/)
	{
		CComPtr<ID3DXBuffer> Shader;
		CComPtr<ID3DXBuffer> ErrorMsgs;
		LPD3DXCONSTANTTABLE pConstantTable = NULL;
		HRESULT hres = D3DXCompileShader(
			pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		LPDIRECT3DPIXELSHADER9 pPS = NULL;
		hres = pDevice->CreatePixelShader((DWORD *)Shader->GetBufferPointer(), &pPS);
		if(FAILED(hres))
		{
			SAFE_RELEASE(pConstantTable);
			THROW_D3DEXCEPTION(hres);
		}

		return PixelShaderPtr(new PixelShader(pPS, pConstantTable, pDevice));
	}

	PixelShaderPtr PixelShader::CreatePixelShaderFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		LPCSTR pSrcFile,
		LPCSTR pFunctionName,
		LPCSTR pProfile,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/)
	{
		CComPtr<ID3DXBuffer> Shader;
		CComPtr<ID3DXBuffer> ErrorMsgs;
		LPD3DXCONSTANTTABLE pConstantTable = NULL;
		HRESULT hres = D3DXCompileShaderFromFileA(
			pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		LPDIRECT3DPIXELSHADER9 pPS = NULL;
		hres = pDevice->CreatePixelShader((DWORD *)Shader->GetBufferPointer(), &pPS);
		if(FAILED(hres))
		{
			SAFE_RELEASE(pConstantTable);
			THROW_D3DEXCEPTION(hres);
		}

		return PixelShaderPtr(new PixelShader(pPS, pConstantTable, pDevice));
	}

	void Effect::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		OnResetDevice();
	}

	void Effect::OnD3D9LostDevice(void)
	{
		OnLostDevice();
	}

	EffectPtr Effect::CreateEffect(
		LPDIRECT3DDEVICE9 pDevice,
		LPCVOID pSrcData,
		UINT SrcDataLen,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/,
		LPD3DXEFFECTPOOL pPool /*= NULL*/)
	{
		LPD3DXEFFECT pEffect = NULL;
		CComPtr<ID3DXBuffer> CompilationErrors;
		HRESULT hres = D3DXCreateEffect(
			pDevice, pSrcData, SrcDataLen, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(std::string((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize()));
		}

		return EffectPtr(new Effect(pEffect));
	}

	EffectPtr Effect::CreateEffectFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		LPCSTR pSrcFile,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/,
		LPD3DXEFFECTPOOL pPool /*= NULL*/)
	{
		LPD3DXEFFECT pEffect = NULL;
		CComPtr<ID3DXBuffer> CompilationErrors;
		HRESULT hres = D3DXCreateEffectFromFileA(
			pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(std::string((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize()));
		}

		return EffectPtr(new Effect(pEffect));
	}
}
