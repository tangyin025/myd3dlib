#include "stdafx.h"
#include "myEffect.h"
#include "myException.h"
#include "libc.h"
#include "myResource.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

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
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create vertex shader from: %p", pSrcData));
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
	LPCSTR pSrcFile,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags)
{
	CComPtr<ID3DXBuffer> Shader;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable = NULL;
	HRESULT hres = D3DXCompileShaderFromFileA(
		pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hres))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create vertex shader from: %s", pSrcFile));
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
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create pixel shader from: %p", pSrcData));
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
	LPCSTR pSrcFile,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags)
{
	CComPtr<ID3DXBuffer> Shader;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable = NULL;
	HRESULT hres = D3DXCompileShaderFromFileA(
		pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hres))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION(std::string((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create pixel shader from: %s", pSrcFile));
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
			THROW_CUSEXCEPTION(std::string((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create effect from: %p", pSrcData));
	}

	Create(pEffect);
}

void Effect::CreateEffectFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pSrcFile,
	CONST D3DXMACRO * pDefines,
	LPD3DXINCLUDE pInclude,
	DWORD Flags,
	LPD3DXEFFECTPOOL pPool)
{
	LPD3DXEFFECT pEffect = NULL;
	CComPtr<ID3DXBuffer> CompilationErrors;
	HRESULT hres = D3DXCreateEffectFromFileA(
		pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
	if(FAILED(hres))
	{
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION(std::string((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create effect from: %s", pSrcFile));
	}

	Create(pEffect);
}
