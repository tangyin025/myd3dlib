#pragma once

#include <d3dx9.h>
#include <atlbase.h>
#include "myMath.h"
#include "mySingleton.h"
#include <boost/shared_ptr.hpp>
#include <myTexture.h>

namespace my
{
	class ConstantTable
	{
	protected:
		HRESULT hr;

		ID3DXConstantTable * m_pConstantTable;

		CComPtr<IDirect3DDevice9> m_Device;

	public:
		ConstantTable(void)
		{
		}

		virtual ~ConstantTable(void);

		void Create(ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice);

		LPVOID GetBufferPointer(void);

		DWORD GetBufferSize(void);

		D3DXHANDLE GetConstant(D3DXHANDLE hConstant, UINT Index);

		D3DXHANDLE GetConstantByName(D3DXHANDLE hConstant, LPCSTR pName);

		void GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC * pDesc, UINT * pCount);

		D3DXHANDLE GetConstantElement(D3DXHANDLE hConstant, UINT Index);

		D3DXCONSTANTTABLE_DESC GetDesc(void);

		UINT GetSamplerIndex(D3DXHANDLE hConstant);

		void SetBool(D3DXHANDLE hConstant, BOOL b);

		void SetBoolArray(D3DXHANDLE hConstant, CONST BOOL* pB, UINT Count);

		void SetDefaults(void);

		void SetFloat(D3DXHANDLE hConstant, FLOAT f);

		void SetFloatArray(D3DXHANDLE hConstant, CONST FLOAT * pf, UINT Count);

		void SetInt(D3DXHANDLE hConstant, INT n);

		void SetIntArray(D3DXHANDLE hConstant, CONST INT * pn, UINT Count);

		void SetMatrix(D3DXHANDLE hConstant, const Matrix4 & Matrix);

		void SetMatrixArray(D3DXHANDLE hConstant, const Matrix4 * pMatrix, UINT Count);

		void SetMatrixPointerArray(D3DXHANDLE hConstant, const Matrix4 ** ppMatrix, UINT Count);

		void SetMatrixTranspose(D3DXHANDLE hConstant, const Matrix4 & Matrix);

		void SetMatrixTransposeArray(D3DXHANDLE hConstant, const Matrix4 * pMatrix, UINT Count);

		void SetMatrixTransposePointerArray(D3DXHANDLE hConstant, const Matrix4 ** ppMatrix, UINT Count);

		void SetValue(D3DXHANDLE hConstant, LPCVOID pData, UINT Bytes);

		void SetVector(D3DXHANDLE hConstant, const Vector4 & Vector);

		void SetVectorArray(D3DXHANDLE hConstant, const Vector4 * pVector, UINT Count);
	};

	class VertexShader
		: public ConstantTable
		, public DeviceRelatedObject<IDirect3DVertexShader9>
	{
	protected:
		HRESULT hr;

	public:
		VertexShader(void)
		{
		}

		void Create(IDirect3DVertexShader9 * ptr, ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice);

		void CreateVertexShader(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

		void CreateVertexShaderFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCTSTR pSrcFile,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

		void OnDestroyDevice(void);

		CComPtr<IDirect3DDevice9> GetDevice(void);

		void GetFunction(void * pData, UINT * pSizeOfData);
	};

	typedef boost::shared_ptr<VertexShader> VertexShaderPtr;

	class PixelShader
		: public ConstantTable
		, public DeviceRelatedObject<IDirect3DPixelShader9>
	{
	protected:
		HRESULT hr;

	public:
		PixelShader(void)
		{
		}

		void Create(IDirect3DPixelShader9 * ptr, ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice);

		void CreatePixelShader(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

		void CreatePixelShaderFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCTSTR pSrcFile,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

		void OnDestroyDevice(void);

		CComPtr<IDirect3DDevice9> GetDevice(void);

		void GetFunction(void * pData, UINT * pSizeOfData);
	};

	typedef boost::shared_ptr<PixelShader> PixelShaderPtr;

	class BaseEffect : public DeviceRelatedObject<ID3DXBaseEffect>
	{
	public:
		BaseEffect(void)
		{
		}

		void Create(ID3DXBaseEffect * ptr);

		D3DXHANDLE GetAnnotation(D3DXHANDLE hObject, UINT Index);

		D3DXHANDLE GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName);

		BOOL GetBool(D3DXHANDLE hParameter);

		void GetBoolArray(D3DXHANDLE hParameter, BOOL * pB, UINT Count);

		D3DXEFFECT_DESC GetDesc(void);

		FLOAT GetFloat(D3DXHANDLE hParameter);

		void GetFloatArray(D3DXHANDLE hParameter, FLOAT * pf, UINT Count);

		D3DXHANDLE GetFunction(UINT Index);

		D3DXHANDLE GetFunctionByName(LPCSTR pName);

		D3DXFUNCTION_DESC GetFunctionDesc(D3DXHANDLE hFunction);

		INT GetInt(D3DXHANDLE hParameter);

		void GetIntArray(D3DXHANDLE hParameter, INT * pn, UINT Count);

		Matrix4 GetMatrix(D3DXHANDLE hParameter);

		void GetMatrixArray(D3DXHANDLE hParameter, Matrix4 * pMatrix, UINT Count);

		void GetMatrixPointerArray(D3DXHANDLE hParameter, Matrix4 ** ppMatrix, UINT Count);

		Matrix4 GetMatrixTranspose(D3DXHANDLE hParameter);

		void GetMatrixTransposeArray(D3DXHANDLE hParameter, Matrix4 * pMatrix, UINT Count);

		void GetMatrixTransposePointerArray(D3DXHANDLE hParameter, Matrix4 ** ppMatrix, UINT Count);

		D3DXHANDLE GetParameter(D3DXHANDLE hParameter, UINT Index);

		D3DXHANDLE GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName);

		D3DXHANDLE GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic);

		D3DXPARAMETER_DESC GetParameterDesc(D3DXHANDLE hParameter);

		D3DXHANDLE GetParameterElement(D3DXHANDLE hParameter, UINT ElementIndex);

		D3DXHANDLE GetPass(D3DXHANDLE hTechnique, UINT Index);

		D3DXHANDLE GetPassByName(D3DXHANDLE hTechnique, LPCSTR pName);

		D3DXPASS_DESC GetPassDesc(D3DXHANDLE hPass);

		CComPtr<IDirect3DPixelShader9> GetPixelShader(D3DXHANDLE hParameter);

		LPCSTR GetString(D3DXHANDLE hParameter);

		D3DXHANDLE GetTechnique(UINT Index);

		D3DXHANDLE GetTechniqueByName(LPCSTR pName);

		D3DXTECHNIQUE_DESC GetTechniqueDesc(D3DXHANDLE hTechnique);

		CComPtr<IDirect3DBaseTexture9> GetTexture(D3DXHANDLE hParameter);

		void GetValue(D3DXHANDLE hParameter, LPVOID pData, UINT Bytes);

		Vector4 GetVector(D3DXHANDLE hParameter);

		void GetVectorArray(D3DXHANDLE hParameter, Vector4 * pVector, UINT Count);

		CComPtr<IDirect3DVertexShader9> GetVertexShader(D3DXHANDLE hParameter);

		void SetArrayRange(D3DXHANDLE hParameter, UINT Start, UINT Stop);

		void SetBool(D3DXHANDLE hParameter, BOOL b);

		void SetBoolArray(D3DXHANDLE hParameter, const BOOL * pB, UINT Count);

		void SetFloat(D3DXHANDLE hParameter, FLOAT f);

		void SetFloatArray(D3DXHANDLE hParameter, const FLOAT * pf, UINT Count);

		void SetInt(D3DXHANDLE hParameter, INT n);

		void SetIntArray(D3DXHANDLE hParameter, const INT * pn, UINT Count);

		void SetMatrix(D3DXHANDLE hParameter, const Matrix4 & Matrix);

		void SetMatrixArray(D3DXHANDLE hParameter, const Matrix4 * pMatrix, UINT Count);

		void SetMatrixPointerArray(D3DXHANDLE hParameter, const Matrix4 ** ppMatrix, UINT Count);

		void SetMatrixTranspose(D3DXHANDLE hParameter, const Matrix4 & Matrix);

		void SetMatrixTransposeArray(D3DXHANDLE hParameter, const Matrix4 * pMatrix, UINT Count);

		void SetMatrixTransposePointerArray(D3DXHANDLE hParameter, const Matrix4 ** ppMatrix, UINT Count);

		void SetString(D3DXHANDLE hParameter, LPCSTR pString);

		void SetTexture(D3DXHANDLE hParameter, const BaseTexture * Texture);

		void SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes);

		void SetVector(D3DXHANDLE hParameter, const Vector4 & Vector);

		void SetVector(D3DXHANDLE hParameter, const Vector3 & Vector);

		void SetVectorArray(D3DXHANDLE hParameter, const Vector4 * pVector, UINT Count);
	};

	class Effect : public BaseEffect
	{
	public:
		Effect(void)
		{
		}

		void CreateEffect(
			LPDIRECT3DDEVICE9 pDevice,
			LPCVOID pSrcData,
			UINT SrcDataLen,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0,
			LPD3DXEFFECTPOOL pPool = NULL);

		void CreateEffectFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCTSTR pSrcFile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0,
			LPD3DXEFFECTPOOL pPool = NULL);

		void OnResetDevice(void);

		void OnLostDevice(void);

		void ApplyParameterBlock(D3DXHANDLE hParameterBlock);

		UINT Begin(DWORD Flags = 0);

		void BeginParameterBlock(void);

		void BeginPass(UINT Pass);

		CComPtr<ID3DXEffect> CloneEffect(LPDIRECT3DDEVICE9 pDevice);

		void CommitChanges(void);

		void DeleteParameterBlock(D3DXHANDLE hParameterBlock);

		void End(void);

		D3DXHANDLE EndParameterBlock(void);

		void EndPass(void);

		D3DXHANDLE FindNextValidTechnique(D3DXHANDLE hTechnique = NULL);

		D3DXHANDLE GetCurrentTechnique(void);

		CComPtr<IDirect3DDevice9> GetDevice(void);

		CComPtr<ID3DXEffectPool> GetPool(void);

		CComPtr<ID3DXEffectStateManager> GetStateManager(void);

		BOOL IsParameterUsed(D3DXHANDLE hParameter, D3DXHANDLE hTechnique);

		void SetRawValue(D3DXHANDLE Handle, void * pData, DWORD OffsetInBytes, DWORD Bytes);

		void SetStateManager(CComPtr<ID3DXEffectStateManager> Manager);

		void SetTechnique(D3DXHANDLE hTechnique);

		bool ValidateTechnique(D3DXHANDLE hTechnique);
	};

	typedef boost::shared_ptr<Effect> EffectPtr;
}
