
#include "stdafx.h"
#include "myd3dlib.h"

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

	return VertexShaderPtr(new VertexShader(pVS, pConstantTable, pDevice));
}

void PixelShader::OnDestroyDevice(void)
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

	return PixelShaderPtr(new PixelShader(pPS, pConstantTable, pDevice));
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
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION(std::string((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create effect from: %p", pSrcData));
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
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION(std::string((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize()));
		}

		THROW_CUSEXCEPTION(str_printf("cannot create effect from: %s", pSrcFile));
	}

	return EffectPtr(new Effect(pEffect));
}
