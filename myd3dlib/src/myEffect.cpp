#include "stdafx.h"
#include "myEffect.h"
#include "myException.h"
#include "libc.h"
#include "myResource.h"

using namespace my;

void VertexShader::OnDestroyDevice(void)
{
	SAFE_RELEASE(m_pConstantTable);

	SAFE_RELEASE(m_ptr);

	m_Device.Release();
}

void VertexShader::CreateVertexShader(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pSrcData,
	UINT srcDataLen,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags)
{
	CComPtr<ID3DXBuffer> Shader;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable = NULL;
	HRESULT hres = D3DXCompileShader(
		pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hres))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)ErrorMsgs->GetBufferPointer()));
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot create vertex shader from: %p"), pSrcData));
	}

	LPDIRECT3DVERTEXSHADER9 pVS = NULL;
	hres = pDevice->CreateVertexShader((DWORD *)Shader->GetBufferPointer(), &pVS);
	if(FAILED(hres))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hres);
	}

	Create(pVS, pConstantTable, pDevice);
}

void VertexShader::CreateVertexShaderFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCTSTR pSrcFile,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags)
{
	CComPtr<ID3DXBuffer> Shader;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable = NULL;
	HRESULT hres = D3DXCompileShaderFromFile(
		pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hres))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)ErrorMsgs->GetBufferPointer()));
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot create vertex shader from: %s"), pSrcFile));
	}

	LPDIRECT3DVERTEXSHADER9 pVS = NULL;
	hres = pDevice->CreateVertexShader((DWORD *)Shader->GetBufferPointer(), &pVS);
	if(FAILED(hres))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hres);
	}

	Create(pVS, pConstantTable, pDevice);
}

void PixelShader::OnDestroyDevice(void)
{
	SAFE_RELEASE(m_pConstantTable);

	SAFE_RELEASE(m_ptr);

	m_Device.Release();
}

void PixelShader::CreatePixelShader(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pSrcData,
	UINT srcDataLen,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags)
{
	CComPtr<ID3DXBuffer> Shader;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable = NULL;
	HRESULT hres = D3DXCompileShader(
		pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hres))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)ErrorMsgs->GetBufferPointer()));
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot create pixel shader from: %p"), pSrcData));
	}

	LPDIRECT3DPIXELSHADER9 pPS = NULL;
	hres = pDevice->CreatePixelShader((DWORD *)Shader->GetBufferPointer(), &pPS);
	if(FAILED(hres))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hres);
	}

	Create(pPS, pConstantTable, pDevice);
}

void PixelShader::CreatePixelShaderFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCTSTR pSrcFile,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags)
{
	CComPtr<ID3DXBuffer> Shader;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable = NULL;
	HRESULT hres = D3DXCompileShaderFromFile(
		pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hres))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)ErrorMsgs->GetBufferPointer()));
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot create pixel shader from: %s"), pSrcFile));
	}

	LPDIRECT3DPIXELSHADER9 pPS = NULL;
	hres = pDevice->CreatePixelShader((DWORD *)Shader->GetBufferPointer(), &pPS);
	if(FAILED(hres))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hres);
	}

	Create(pPS, pConstantTable, pDevice);
}

void Effect::CreateEffect(
	LPDIRECT3DDEVICE9 pDevice,
	LPCVOID pSrcData,
	UINT SrcDataLen,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags,
	LPD3DXEFFECTPOOL pPool)
{
	LPD3DXEFFECT pEffect = NULL;
	CComPtr<ID3DXBuffer> CompilationErrors;
	HRESULT hres = D3DXCreateEffect(
		pDevice, pSrcData, SrcDataLen, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
	if(FAILED(hres))
	{
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)CompilationErrors->GetBufferPointer()));
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot create effect from: %p"), pSrcData));
	}

	Create(pEffect);
}

void Effect::CreateEffectFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCTSTR pSrcFile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags,
	LPD3DXEFFECTPOOL pPool)
{
	LPD3DXEFFECT pEffect = NULL;
	CComPtr<ID3DXBuffer> CompilationErrors;
	HRESULT hres = D3DXCreateEffectFromFile(
		pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
	if(FAILED(hres))
	{
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)CompilationErrors->GetBufferPointer()));
		}

		THROW_CUSEXCEPTION(str_printf(_T("cannot create effect from: %s"), pSrcFile));
	}

	Create(pEffect);
}
