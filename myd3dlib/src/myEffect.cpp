#include "stdafx.h"
#include "myEffect.h"
#include "myException.h"
#include "libc.h"
#include "myResource.h"

using namespace my;

ConstantTable::~ConstantTable(void)
{
	SAFE_RELEASE(m_pConstantTable);
}

void ConstantTable::Create(ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice)
{
	_ASSERT(!m_pConstantTable && !m_Device);

	m_pConstantTable = pConstantTable;

	m_Device = pDevice;
}

LPVOID ConstantTable::GetBufferPointer(void)
{
	return m_pConstantTable->GetBufferPointer();
}

DWORD ConstantTable::GetBufferSize(void)
{
	return m_pConstantTable->GetBufferSize();
}

D3DXHANDLE ConstantTable::GetConstant(D3DXHANDLE hConstant, UINT Index)
{
	return m_pConstantTable->GetConstant(hConstant, Index);
}

D3DXHANDLE ConstantTable::GetConstantByName(D3DXHANDLE hConstant, LPCSTR pName)
{
	return m_pConstantTable->GetConstantByName(hConstant, pName);
}

void ConstantTable::GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC * pDesc, UINT * pCount)
{
	V(m_pConstantTable->GetConstantDesc(hConstant, pDesc, pCount));
}

D3DXHANDLE ConstantTable::GetConstantElement(D3DXHANDLE hConstant, UINT Index)
{
	return m_pConstantTable->GetConstantElement(hConstant, Index);
}

D3DXCONSTANTTABLE_DESC ConstantTable::GetDesc(void)
{
	D3DXCONSTANTTABLE_DESC desc;
	V(m_pConstantTable->GetDesc(&desc));
	return desc;
}

UINT ConstantTable::GetSamplerIndex(D3DXHANDLE hConstant)
{
	return m_pConstantTable->GetSamplerIndex(hConstant);
}

void ConstantTable::SetBool(D3DXHANDLE hConstant, BOOL b)
{
	V(m_pConstantTable->SetBool(m_Device, hConstant, b));
}

void ConstantTable::SetBoolArray(D3DXHANDLE hConstant, CONST BOOL* pB, UINT Count)
{
	V(m_pConstantTable->SetBoolArray(m_Device, hConstant, pB, Count));
}

void ConstantTable::SetDefaults(void)
{
	V(m_pConstantTable->SetDefaults(m_Device));
}

void ConstantTable::SetFloat(D3DXHANDLE hConstant, FLOAT f)
{
	V(m_pConstantTable->SetFloat(m_Device, hConstant, f));
}

void ConstantTable::SetFloatArray(D3DXHANDLE hConstant, CONST FLOAT * pf, UINT Count)
{
	V(m_pConstantTable->SetFloatArray(m_Device, hConstant, pf, Count));
}

void ConstantTable::SetInt(D3DXHANDLE hConstant, INT n)
{
	V(m_pConstantTable->SetInt(m_Device, hConstant, n));
}

void ConstantTable::SetIntArray(D3DXHANDLE hConstant, CONST INT * pn, UINT Count)
{
	V(m_pConstantTable->SetIntArray(m_Device, hConstant, pn, Count));
}

void ConstantTable::SetMatrix(D3DXHANDLE hConstant, const Matrix4 & Matrix)
{
	V(m_pConstantTable->SetMatrix(m_Device, hConstant, (D3DXMATRIX *)&Matrix));
}

void ConstantTable::SetMatrixArray(D3DXHANDLE hConstant, const Matrix4 * pMatrix, UINT Count)
{
	V(m_pConstantTable->SetMatrixArray(m_Device, hConstant, (D3DXMATRIX *)pMatrix, Count));
}

void ConstantTable::SetMatrixPointerArray(D3DXHANDLE hConstant, const Matrix4 ** ppMatrix, UINT Count)
{
	V(m_pConstantTable->SetMatrixPointerArray(m_Device, hConstant, (const D3DXMATRIX **)ppMatrix, Count));
}

void ConstantTable::SetMatrixTranspose(D3DXHANDLE hConstant, const Matrix4 & Matrix)
{
	V(m_pConstantTable->SetMatrixTranspose(m_Device, hConstant, (D3DXMATRIX *)&Matrix));
}

void ConstantTable::SetMatrixTransposeArray(D3DXHANDLE hConstant, const Matrix4 * pMatrix, UINT Count)
{
	V(m_pConstantTable->SetMatrixTransposeArray(m_Device, hConstant, (D3DXMATRIX *)pMatrix, Count));
}

void ConstantTable::SetMatrixTransposePointerArray(D3DXHANDLE hConstant, const Matrix4 ** ppMatrix, UINT Count)
{
	V(m_pConstantTable->SetMatrixTransposePointerArray(m_Device, hConstant, (const D3DXMATRIX **)ppMatrix, Count));
}

void ConstantTable::SetValue(D3DXHANDLE hConstant, LPCVOID pData, UINT Bytes)
{
	V(m_pConstantTable->SetValue(m_Device, hConstant, pData, Bytes));
}

void ConstantTable::SetVector(D3DXHANDLE hConstant, const Vector4 & Vector)
{
	V(m_pConstantTable->SetVector(m_Device, hConstant, (D3DXVECTOR4 *)&Vector));
}

void ConstantTable::SetVectorArray(D3DXHANDLE hConstant, const Vector4 * pVector, UINT Count)
{
	V(m_pConstantTable->SetVectorArray(m_Device, hConstant, (D3DXVECTOR4 *)pVector, Count));
}

void VertexShader::Create(IDirect3DVertexShader9 * ptr, ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;

	ConstantTable::Create(pConstantTable, pDevice);
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
	hr = D3DXCompileShader(
		pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hr))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION((char *)ErrorMsgs->GetBufferPointer());
		}

		THROW_CUSEXCEPTION(str_printf("cannot create vertex shader from: %p", pSrcData));
	}

	LPDIRECT3DVERTEXSHADER9 pVS = NULL;
	hr = pDevice->CreateVertexShader((DWORD *)Shader->GetBufferPointer(), &pVS);
	if(FAILED(hr))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hr);
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
	hr = D3DXCompileShaderFromFile(
		pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hr))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION((char *)ErrorMsgs->GetBufferPointer());
		}

		THROW_CUSEXCEPTION(str_printf("cannot create vertex shader from: %s", pSrcFile));
	}

	LPDIRECT3DVERTEXSHADER9 pVS = NULL;
	hr = pDevice->CreateVertexShader((DWORD *)Shader->GetBufferPointer(), &pVS);
	if(FAILED(hr))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hr);
	}

	Create(pVS, pConstantTable, pDevice);
}

void VertexShader::OnDestroyDevice(void)
{
	SAFE_RELEASE(m_pConstantTable);

	SAFE_RELEASE(m_ptr);

	m_Device.Release();
}

CComPtr<IDirect3DDevice9> VertexShader::GetDevice(void)
{
	CComPtr<IDirect3DDevice9> Device;
	V(m_ptr->GetDevice(&Device));
	return Device;
}

void VertexShader::GetFunction(void * pData, UINT * pSizeOfData)
{
	V(m_ptr->GetFunction(pData, pSizeOfData));
}

void PixelShader::Create(IDirect3DPixelShader9 * ptr, ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;

	ConstantTable::Create(pConstantTable, pDevice);
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
	hr = D3DXCompileShader(
		pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hr))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION((char *)ErrorMsgs->GetBufferPointer());
		}

		THROW_CUSEXCEPTION(str_printf("cannot create pixel shader from: %p", pSrcData));
	}

	LPDIRECT3DPIXELSHADER9 pPS = NULL;
	hr = pDevice->CreatePixelShader((DWORD *)Shader->GetBufferPointer(), &pPS);
	if(FAILED(hr))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hr);
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
	hr = D3DXCompileShaderFromFile(
		pSrcFile, pDefines, pInclude, pFunctionName, pProfile, Flags, &Shader, &ErrorMsgs, &pConstantTable);
	if(FAILED(hr))
	{
		if(ErrorMsgs)
		{
			THROW_CUSEXCEPTION((char *)ErrorMsgs->GetBufferPointer());
		}

		THROW_CUSEXCEPTION(str_printf("cannot create pixel shader from: %s", pSrcFile));
	}

	LPDIRECT3DPIXELSHADER9 pPS = NULL;
	hr = pDevice->CreatePixelShader((DWORD *)Shader->GetBufferPointer(), &pPS);
	if(FAILED(hr))
	{
		SAFE_RELEASE(pConstantTable);
		THROW_D3DEXCEPTION(hr);
	}

	Create(pPS, pConstantTable, pDevice);
}

void PixelShader::OnDestroyDevice(void)
{
	SAFE_RELEASE(m_pConstantTable);

	SAFE_RELEASE(m_ptr);

	m_Device.Release();
}

CComPtr<IDirect3DDevice9> PixelShader::GetDevice(void)
{
	CComPtr<IDirect3DDevice9> Device;
	V(m_ptr->GetDevice(&Device));
	return Device;
}

void PixelShader::GetFunction(void * pData, UINT * pSizeOfData)
{
	V(m_ptr->GetFunction(pData, pSizeOfData));
}

void BaseEffect::Create(ID3DXBaseEffect * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

D3DXHANDLE BaseEffect::GetAnnotation(D3DXHANDLE hObject, UINT Index)
{
	D3DXHANDLE ret = m_ptr->GetAnnotation(hObject, Index);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName)
{
	D3DXHANDLE ret = m_ptr->GetAnnotationByName(hObject, pName);
	_ASSERT(ret);
	return ret;
}

BOOL BaseEffect::GetBool(D3DXHANDLE hParameter)
{
	BOOL ret;
	V(m_ptr->GetBool(hParameter, &ret));
	return ret;
}

void BaseEffect::GetBoolArray(D3DXHANDLE hParameter, BOOL * pB, UINT Count)
{
	V(m_ptr->GetBoolArray(hParameter, pB, Count));
}

D3DXEFFECT_DESC BaseEffect::GetDesc(void)
{
	D3DXEFFECT_DESC desc;
	V(m_ptr->GetDesc(&desc));
	return desc;
}

FLOAT BaseEffect::GetFloat(D3DXHANDLE hParameter)
{
	FLOAT ret;
	V(m_ptr->GetFloat(hParameter, &ret));
	return ret;
}

void BaseEffect::GetFloatArray(D3DXHANDLE hParameter, FLOAT * pf, UINT Count)
{
	V(m_ptr->GetFloatArray(hParameter, pf, Count));
}

D3DXHANDLE BaseEffect::GetFunction(UINT Index)
{
	D3DXHANDLE ret = m_ptr->GetFunction(Index);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetFunctionByName(LPCSTR pName)
{
	D3DXHANDLE ret = m_ptr->GetFunctionByName(pName);
	_ASSERT(ret);
	return ret;
}

D3DXFUNCTION_DESC BaseEffect::GetFunctionDesc(D3DXHANDLE hFunction)
{
	D3DXFUNCTION_DESC desc;
	V(m_ptr->GetFunctionDesc(hFunction, &desc));
	return desc;
}

INT BaseEffect::GetInt(D3DXHANDLE hParameter)
{
	INT ret;
	V(m_ptr->GetInt(hParameter, &ret));
	return ret;
}

void BaseEffect::GetIntArray(D3DXHANDLE hParameter, INT * pn, UINT Count)
{
	V(m_ptr->GetIntArray(hParameter, pn, Count));
}

Matrix4 BaseEffect::GetMatrix(D3DXHANDLE hParameter)
{
	Matrix4 ret;
	V(m_ptr->GetMatrix(hParameter, (D3DXMATRIX *)&ret));
	return ret;
}

void BaseEffect::GetMatrixArray(D3DXHANDLE hParameter, Matrix4 * pMatrix, UINT Count)
{
	V(m_ptr->GetMatrixArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
}

void BaseEffect::GetMatrixPointerArray(D3DXHANDLE hParameter, Matrix4 ** ppMatrix, UINT Count)
{
	// memory compatible
	V(m_ptr->GetMatrixPointerArray(hParameter, (D3DXMATRIX **)ppMatrix, Count));
}

Matrix4 BaseEffect::GetMatrixTranspose(D3DXHANDLE hParameter)
{
	Matrix4 ret;
	V(m_ptr->GetMatrixTranspose(hParameter, (D3DXMATRIX *)&ret));
	return ret;
}

void BaseEffect::GetMatrixTransposeArray(D3DXHANDLE hParameter, Matrix4 * pMatrix, UINT Count)
{
	V(m_ptr->GetMatrixTransposeArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
}

void BaseEffect::GetMatrixTransposePointerArray(D3DXHANDLE hParameter, Matrix4 ** ppMatrix, UINT Count)
{
	V(m_ptr->GetMatrixTransposePointerArray(hParameter, (D3DXMATRIX **)ppMatrix, Count));
}

D3DXHANDLE BaseEffect::GetParameter(D3DXHANDLE hParameter, UINT Index)
{
	D3DXHANDLE ret = m_ptr->GetParameter(hParameter, Index);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName)
{
	D3DXHANDLE ret = m_ptr->GetParameterByName(hParameter, pName);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic)
{
	D3DXHANDLE ret = m_ptr->GetParameterBySemantic(hParameter, pSemantic);
	_ASSERT(ret);
	return ret;
}

D3DXPARAMETER_DESC BaseEffect::GetParameterDesc(D3DXHANDLE hParameter)
{
	D3DXPARAMETER_DESC desc;
	V(m_ptr->GetParameterDesc(hParameter, &desc));
	return desc;
}

D3DXHANDLE BaseEffect::GetParameterElement(D3DXHANDLE hParameter, UINT ElementIndex)
{
	D3DXHANDLE ret = m_ptr->GetParameterElement(hParameter, ElementIndex);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetPass(D3DXHANDLE hTechnique, UINT Index)
{
	D3DXHANDLE ret = m_ptr->GetPass(hTechnique, Index);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetPassByName(D3DXHANDLE hTechnique, LPCSTR pName)
{
	D3DXHANDLE ret = m_ptr->GetPassByName(hTechnique, pName);
	_ASSERT(ret);
	return ret;
}

D3DXPASS_DESC BaseEffect::GetPassDesc(D3DXHANDLE hPass)
{
	D3DXPASS_DESC desc;
	V(m_ptr->GetPassDesc(hPass, &desc));
	return desc;
}

CComPtr<IDirect3DPixelShader9> BaseEffect::GetPixelShader(D3DXHANDLE hParameter)
{
	CComPtr<IDirect3DPixelShader9> PShader;
	V(m_ptr->GetPixelShader(hParameter, &PShader));
	return PShader;
}

LPCSTR BaseEffect::GetString(D3DXHANDLE hParameter)
{
	LPCSTR ret;
	V(m_ptr->GetString(hParameter, &ret));
	return ret;
}

D3DXHANDLE BaseEffect::GetTechnique(UINT Index)
{
	D3DXHANDLE ret = m_ptr->GetTechnique(Index);
	_ASSERT(ret);
	return ret;
}

D3DXHANDLE BaseEffect::GetTechniqueByName(LPCSTR pName)
{
	D3DXHANDLE ret = m_ptr->GetTechniqueByName(pName);
	_ASSERT(ret);
	return ret;
}

D3DXTECHNIQUE_DESC BaseEffect::GetTechniqueDesc(D3DXHANDLE hTechnique)
{
	D3DXTECHNIQUE_DESC desc;
	V(m_ptr->GetTechniqueDesc(hTechnique, &desc));
	return desc;
}

CComPtr<IDirect3DBaseTexture9> BaseEffect::GetTexture(D3DXHANDLE hParameter)
{
	CComPtr<IDirect3DBaseTexture9> Texture;
	V(m_ptr->GetTexture(hParameter, &Texture));
	return Texture;
}

void BaseEffect::GetValue(D3DXHANDLE hParameter, LPVOID pData, UINT Bytes)
{
	V(m_ptr->GetValue(hParameter, pData, Bytes));
}

Vector4 BaseEffect::GetVector(D3DXHANDLE hParameter)
{
	Vector4 ret;
	V(m_ptr->GetVector(hParameter, (D3DXVECTOR4 *)&ret));
	return ret;
}

void BaseEffect::GetVectorArray(D3DXHANDLE hParameter, Vector4 * pVector, UINT Count)
{
	V(m_ptr->GetVectorArray(hParameter, (D3DXVECTOR4 *)pVector, Count));
}

CComPtr<IDirect3DVertexShader9> BaseEffect::GetVertexShader(D3DXHANDLE hParameter)
{
	CComPtr<IDirect3DVertexShader9> VShader;
	V(m_ptr->GetVertexShader(hParameter, &VShader));
	return VShader;
}

void BaseEffect::SetArrayRange(D3DXHANDLE hParameter, UINT Start, UINT Stop)
{
	V(m_ptr->SetArrayRange(hParameter, Start, Stop));
}

void BaseEffect::SetBool(D3DXHANDLE hParameter, BOOL b)
{
	V(m_ptr->SetBool(hParameter, b));
}

void BaseEffect::SetBoolArray(D3DXHANDLE hParameter, const BOOL * pB, UINT Count)
{
	V(m_ptr->SetBoolArray(hParameter, pB, Count));
}

void BaseEffect::SetFloat(D3DXHANDLE hParameter, FLOAT f)
{
	V(m_ptr->SetFloat(hParameter, f));
}

void BaseEffect::SetFloatArray(D3DXHANDLE hParameter, const FLOAT * pf, UINT Count)
{
	V(m_ptr->SetFloatArray(hParameter, pf, Count));
}

void BaseEffect::SetInt(D3DXHANDLE hParameter, INT n)
{
	V(m_ptr->SetInt(hParameter, n));
}

void BaseEffect::SetIntArray(D3DXHANDLE hParameter, const INT * pn, UINT Count)
{
	V(m_ptr->SetIntArray(hParameter, pn, Count));
}

void BaseEffect::SetMatrix(D3DXHANDLE hParameter, const Matrix4 & Matrix)
{
	V(m_ptr->SetMatrix(hParameter, (D3DXMATRIX *)&Matrix));
}

void BaseEffect::SetMatrixArray(D3DXHANDLE hParameter, const Matrix4 * pMatrix, UINT Count)
{
	V(m_ptr->SetMatrixArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
}

void BaseEffect::SetMatrixPointerArray(D3DXHANDLE hParameter, const Matrix4 ** ppMatrix, UINT Count)
{
	V(m_ptr->SetMatrixPointerArray(hParameter, (const D3DXMATRIX **)ppMatrix, Count));
}

void BaseEffect::SetMatrixTranspose(D3DXHANDLE hParameter, const Matrix4 & Matrix)
{
	V(m_ptr->SetMatrixTranspose(hParameter, (D3DXMATRIX *)&Matrix));
}

void BaseEffect::SetMatrixTransposeArray(D3DXHANDLE hParameter, const Matrix4 * pMatrix, UINT Count)
{
	V(m_ptr->SetMatrixTransposeArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
}

void BaseEffect::SetMatrixTransposePointerArray(D3DXHANDLE hParameter, const Matrix4 ** ppMatrix, UINT Count)
{
	V(m_ptr->SetMatrixTransposePointerArray(hParameter, (const D3DXMATRIX **)ppMatrix, Count));
}

void BaseEffect::SetString(D3DXHANDLE hParameter, LPCSTR pString)
{
	V(m_ptr->SetString(hParameter, pString));
}

void BaseEffect::SetTexture(D3DXHANDLE hParameter, const BaseTexture * Texture)
{
	V(m_ptr->SetTexture(hParameter, Texture ? Texture->m_ptr : NULL));
}

void BaseEffect::SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes)
{
	V(m_ptr->SetValue(hParameter, pData, Bytes));
}

void BaseEffect::SetVector(D3DXHANDLE hParameter, const Vector4 & Vector)
{
	V(m_ptr->SetVector(hParameter, (D3DXVECTOR4 *)&Vector));
}

void BaseEffect::SetVector(D3DXHANDLE hParameter, const Vector3 & Vector)
{
	V(m_ptr->SetFloatArray(hParameter, &Vector.x, 3));
}

void BaseEffect::SetVectorArray(D3DXHANDLE hParameter, const Vector4 * pVector, UINT Count)
{
	V(m_ptr->SetVectorArray(hParameter, (D3DXVECTOR4 *)pVector, Count));
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
	hr = D3DXCreateEffect(
		pDevice, pSrcData, SrcDataLen, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
	if(FAILED(hr))
	{
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION((char *)CompilationErrors->GetBufferPointer());
		}

		THROW_CUSEXCEPTION(str_printf("cannot create effect from: %p", pSrcData));
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
	hr = D3DXCreateEffectFromFile(
		pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
	if(FAILED(hr))
	{
		if(CompilationErrors)
		{
			THROW_CUSEXCEPTION((char *)CompilationErrors->GetBufferPointer());
		}

		THROW_CUSEXCEPTION(str_printf("cannot create effect from: %s", pSrcFile));
	}

	Create(pEffect);
}

void Effect::OnResetDevice(void)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->OnResetDevice());
}

void Effect::OnLostDevice(void)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->OnLostDevice());
}

void Effect::ApplyParameterBlock(D3DXHANDLE hParameterBlock)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->ApplyParameterBlock(hParameterBlock));
}

UINT Effect::Begin(DWORD Flags)
{
	UINT Passes = 0;
	V(static_cast<ID3DXEffect *>(m_ptr)->Begin(&Passes, Flags));
	return Passes;
}

void Effect::BeginParameterBlock(void)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->BeginParameterBlock());
}

void Effect::BeginPass(UINT Pass)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->BeginPass(Pass));
}

CComPtr<ID3DXEffect> Effect::CloneEffect(LPDIRECT3DDEVICE9 pDevice)
{
	CComPtr<ID3DXEffect> Effect;
	V(static_cast<ID3DXEffect *>(m_ptr)->CloneEffect(pDevice, &Effect));
	return Effect;
}

void Effect::CommitChanges(void)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->CommitChanges());
}

void Effect::DeleteParameterBlock(D3DXHANDLE hParameterBlock)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->DeleteParameterBlock(hParameterBlock));
}

void Effect::End(void)
{
	static_cast<ID3DXEffect *>(m_ptr)->End();
}

D3DXHANDLE Effect::EndParameterBlock(void)
{
	D3DXHANDLE ret = static_cast<ID3DXEffect *>(m_ptr)->EndParameterBlock();
	_ASSERT(ret);
	return ret;
}

void Effect::EndPass(void)
{
	static_cast<ID3DXEffect *>(m_ptr)->EndPass();
}

D3DXHANDLE Effect::FindNextValidTechnique(D3DXHANDLE hTechnique)
{
	D3DXHANDLE ret;
	V(static_cast<ID3DXEffect *>(m_ptr)->FindNextValidTechnique(hTechnique, &ret));
	return ret;
}

D3DXHANDLE Effect::GetCurrentTechnique(void)
{
	D3DXHANDLE ret = static_cast<ID3DXEffect *>(m_ptr)->GetCurrentTechnique();
	_ASSERT(ret);
	return ret;
}

CComPtr<IDirect3DDevice9> Effect::GetDevice(void)
{
	CComPtr<IDirect3DDevice9> Device;
	V(static_cast<ID3DXEffect *>(m_ptr)->GetDevice(&Device));
	return Device;
}

CComPtr<ID3DXEffectPool> Effect::GetPool(void)
{
	CComPtr<ID3DXEffectPool> Pool;
	V(static_cast<ID3DXEffect *>(m_ptr)->GetPool(&Pool));
	return Pool;
}

CComPtr<ID3DXEffectStateManager> Effect::GetStateManager(void)
{
	CComPtr<ID3DXEffectStateManager> Manager;
	V(static_cast<ID3DXEffect *>(m_ptr)->GetStateManager(&Manager));
	return Manager;
}

BOOL Effect::IsParameterUsed(D3DXHANDLE hParameter, D3DXHANDLE hTechnique)
{
	return static_cast<ID3DXEffect *>(m_ptr)->IsParameterUsed(hParameter, hTechnique);
}

void Effect::SetRawValue(D3DXHANDLE Handle, void * pData, DWORD OffsetInBytes, DWORD Bytes)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->SetRawValue(Handle, pData, OffsetInBytes, Bytes));
}

void Effect::SetStateManager(CComPtr<ID3DXEffectStateManager> Manager)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->SetStateManager(Manager));
}

void Effect::SetTechnique(D3DXHANDLE hTechnique)
{
	V(static_cast<ID3DXEffect *>(m_ptr)->SetTechnique(hTechnique));
}

bool Effect::ValidateTechnique(D3DXHANDLE hTechnique)
{
	return SUCCEEDED(static_cast<ID3DXEffect *>(m_ptr)->ValidateTechnique(hTechnique));
}
