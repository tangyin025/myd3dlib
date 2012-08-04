#pragma once

#include <d3dx9.h>
#include <atlbase.h>
#include "myMath.h"
#include <DXUT.h>
#include <boost/shared_ptr.hpp>
#include "mySingleton.h"

namespace my
{
	class ConstantTable
	{
	protected:
		HRESULT hr;

		ID3DXConstantTable * m_pConstantTable;

		CComPtr<IDirect3DDevice9> m_Device;

	protected:
		ConstantTable(ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice)
			: m_pConstantTable(pConstantTable)
			, m_Device(pDevice)
		{
			_ASSERT(NULL != m_pConstantTable);
		}

	public:
		virtual ~ConstantTable(void)
		{
			SAFE_RELEASE(m_pConstantTable);
		}

		LPVOID GetBufferPointer(void)
		{
			return m_pConstantTable->GetBufferPointer();
		}

		DWORD GetBufferSize(void)
		{
			return m_pConstantTable->GetBufferSize();
		}

		D3DXHANDLE GetConstant(D3DXHANDLE hConstant, UINT Index)
		{
			return m_pConstantTable->GetConstant(hConstant, Index);
		}

		D3DXHANDLE GetConstantByName(D3DXHANDLE hConstant, LPCSTR pName)
		{
			return m_pConstantTable->GetConstantByName(hConstant, pName);
		}

		void GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC * pDesc, UINT * pCount)
		{
			V(m_pConstantTable->GetConstantDesc(hConstant, pDesc, pCount));
		}

		D3DXHANDLE GetConstantElement(D3DXHANDLE hConstant, UINT Index)
		{
			return m_pConstantTable->GetConstantElement(hConstant, Index);
		}

		D3DXCONSTANTTABLE_DESC GetDesc(void)
		{
			D3DXCONSTANTTABLE_DESC desc;
			V(m_pConstantTable->GetDesc(&desc));
			return desc;
		}

		UINT GetSamplerIndex(D3DXHANDLE hConstant)
		{
			return m_pConstantTable->GetSamplerIndex(hConstant);
		}

		void SetBool(D3DXHANDLE hConstant, BOOL b)
		{
			V(m_pConstantTable->SetBool(m_Device, hConstant, b));
		}

		void SetBoolArray(D3DXHANDLE hConstant, CONST BOOL* pB, UINT Count)
		{
			V(m_pConstantTable->SetBoolArray(m_Device, hConstant, pB, Count));
		}

		void SetDefaults(void)
		{
			V(m_pConstantTable->SetDefaults(m_Device));
		}

		void SetFloat(D3DXHANDLE hConstant, FLOAT f)
		{
			V(m_pConstantTable->SetFloat(m_Device, hConstant, f));
		}

		void SetFloatArray(D3DXHANDLE hConstant, CONST FLOAT * pf, UINT Count)
		{
			V(m_pConstantTable->SetFloatArray(m_Device, hConstant, pf, Count));
		}

		void SetInt(D3DXHANDLE hConstant, INT n)
		{
			V(m_pConstantTable->SetInt(m_Device, hConstant, n));
		}

		void SetIntArray(D3DXHANDLE hConstant, CONST INT * pn, UINT Count)
		{
			V(m_pConstantTable->SetIntArray(m_Device, hConstant, pn, Count));
		}

		void SetMatrix(D3DXHANDLE hConstant, const Matrix4 & Matrix)
		{
			V(m_pConstantTable->SetMatrix(m_Device, hConstant, (D3DXMATRIX *)&Matrix));
		}

		void SetMatrixArray(D3DXHANDLE hConstant, const Matrix4 * pMatrix, UINT Count)
		{
			V(m_pConstantTable->SetMatrixArray(m_Device, hConstant, (D3DXMATRIX *)pMatrix, Count));
		}

		void SetMatrixPointerArray(D3DXHANDLE hConstant, const Matrix4 ** ppMatrix, UINT Count)
		{
			V(m_pConstantTable->SetMatrixPointerArray(m_Device, hConstant, (const D3DXMATRIX **)ppMatrix, Count));
		}

		void SetMatrixTranspose(D3DXHANDLE hConstant, const Matrix4 & Matrix)
		{
			V(m_pConstantTable->SetMatrixTranspose(m_Device, hConstant, (D3DXMATRIX *)&Matrix));
		}

		void SetMatrixTransposeArray(D3DXHANDLE hConstant, const Matrix4 * pMatrix, UINT Count)
		{
			V(m_pConstantTable->SetMatrixTransposeArray(m_Device, hConstant, (D3DXMATRIX *)pMatrix, Count));
		}

		void SetMatrixTransposePointerArray(D3DXHANDLE hConstant, const Matrix4 ** ppMatrix, UINT Count)
		{
			V(m_pConstantTable->SetMatrixTransposePointerArray(m_Device, hConstant, (const D3DXMATRIX **)ppMatrix, Count));
		}

		void SetValue(D3DXHANDLE hConstant, LPCVOID pData, UINT Bytes)
		{
			V(m_pConstantTable->SetValue(m_Device, hConstant, pData, Bytes));
		}

		void SetVector(D3DXHANDLE hConstant, const Vector4 & Vector)
		{
			V(m_pConstantTable->SetVector(m_Device, hConstant, (D3DXVECTOR4 *)&Vector));
		}

		void SetVectorArray(D3DXHANDLE hConstant, const Vector4 * pVector, UINT Count)
		{
			V(m_pConstantTable->SetVectorArray(m_Device, hConstant, (D3DXVECTOR4 *)pVector, Count));
		}
	};

	class VertexShader;

	typedef boost::shared_ptr<VertexShader> VertexShaderPtr;

	class VertexShader
		: public ConstantTable
		, public DeviceRelatedObject<IDirect3DVertexShader9>
	{
	protected:
		HRESULT hr;

		VertexShader(IDirect3DVertexShader9 * ptr, ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice)
			: ConstantTable(pConstantTable, pDevice)
			, DeviceRelatedObject(ptr)
		{
			_ASSERT(m_Device == GetDevice());
		}

		void OnDestroyDevice(void);

	public:
		static VertexShaderPtr CreateVertexShader(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

		static VertexShaderPtr CreateVertexShaderFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcFile,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

	public:
		CComPtr<IDirect3DDevice9> GetDevice(void)
		{
			CComPtr<IDirect3DDevice9> Device;
			V(m_ptr->GetDevice(&Device));
			return Device;
		}

		void GetFunction(void * pData, UINT * pSizeOfData)
		{
			V(m_ptr->GetFunction(pData, pSizeOfData));
		}
	};

	class PixelShader;

	typedef boost::shared_ptr<PixelShader> PixelShaderPtr;

	class PixelShader
		: public ConstantTable
		, public DeviceRelatedObject<IDirect3DPixelShader9>
	{
	protected:
		HRESULT hr;

		PixelShader(IDirect3DPixelShader9 * ptr, ID3DXConstantTable * pConstantTable, LPDIRECT3DDEVICE9 pDevice)
			: ConstantTable(pConstantTable, pDevice)
			, DeviceRelatedObject(ptr)
		{
			_ASSERT(m_Device == GetDevice());
		}

		void OnDestroyDevice(void);

	public:
		static PixelShaderPtr CreatePixelShader(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcData,
			UINT srcDataLen,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

		static PixelShaderPtr CreatePixelShaderFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcFile,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0);

	public:
		CComPtr<IDirect3DDevice9> GetDevice(void)
		{
			CComPtr<IDirect3DDevice9> Device;
			V(m_ptr->GetDevice(&Device));
			return Device;
		}

		void GetFunction(void * pData, UINT * pSizeOfData)
		{
			V(m_ptr->GetFunction(pData, pSizeOfData));
		}
	};

	class BaseEffect;

	typedef boost::shared_ptr<BaseEffect> BaseEffectPtr;

	class BaseEffect : public DeviceRelatedObject<ID3DXBaseEffect>
	{
	public:
		BaseEffect(ID3DXBaseEffect * ptr)
			: DeviceRelatedObject(ptr)
		{
		}

	public:
		D3DXHANDLE GetAnnotation(D3DXHANDLE hObject, UINT Index)
		{
			D3DXHANDLE ret = m_ptr->GetAnnotation(hObject, Index);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName)
		{
			D3DXHANDLE ret = m_ptr->GetAnnotationByName(hObject, pName);
			_ASSERT(ret);
			return ret;
		}

		BOOL GetBool(D3DXHANDLE hParameter)
		{
			BOOL ret;
			V(m_ptr->GetBool(hParameter, &ret));
			return ret;
		}

		void GetBoolArray(D3DXHANDLE hParameter, BOOL * pB, UINT Count)
		{
			V(m_ptr->GetBoolArray(hParameter, pB, Count));
		}

		D3DXEFFECT_DESC GetDesc(void)
		{
			D3DXEFFECT_DESC desc;
			V(m_ptr->GetDesc(&desc));
			return desc;
		}

		FLOAT GetFloat(D3DXHANDLE hParameter)
		{
			FLOAT ret;
			V(m_ptr->GetFloat(hParameter, &ret));
			return ret;
		}

		void GetFloatArray(D3DXHANDLE hParameter, FLOAT * pf, UINT Count)
		{
			V(m_ptr->GetFloatArray(hParameter, pf, Count));
		}

		D3DXHANDLE GetFunction(UINT Index)
		{
			D3DXHANDLE ret = m_ptr->GetFunction(Index);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetFunctionByName(LPCSTR pName)
		{
			D3DXHANDLE ret = m_ptr->GetFunctionByName(pName);
			_ASSERT(ret);
			return ret;
		}

		D3DXFUNCTION_DESC GetFunctionDesc(D3DXHANDLE hFunction)
		{
			D3DXFUNCTION_DESC desc;
			V(m_ptr->GetFunctionDesc(hFunction, &desc));
			return desc;
		}

		INT GetInt(D3DXHANDLE hParameter)
		{
			INT ret;
			V(m_ptr->GetInt(hParameter, &ret));
			return ret;
		}

		void GetIntArray(D3DXHANDLE hParameter, INT * pn, UINT Count)
		{
			V(m_ptr->GetIntArray(hParameter, pn, Count));
		}

		Matrix4 GetMatrix(D3DXHANDLE hParameter)
		{
			Matrix4 ret;
			V(m_ptr->GetMatrix(hParameter, (D3DXMATRIX *)&ret));
			return ret;
		}

		void GetMatrixArray(D3DXHANDLE hParameter, Matrix4 * pMatrix, UINT Count)
		{
			V(m_ptr->GetMatrixArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
		}

		void GetMatrixPointerArray(D3DXHANDLE hParameter, Matrix4 ** ppMatrix, UINT Count)
		{
			// memory compatible
			V(m_ptr->GetMatrixPointerArray(hParameter, (D3DXMATRIX **)ppMatrix, Count));
		}

		Matrix4 GetMatrixTranspose(D3DXHANDLE hParameter)
		{
			Matrix4 ret;
			V(m_ptr->GetMatrixTranspose(hParameter, (D3DXMATRIX *)&ret));
			return ret;
		}

		void GetMatrixTransposeArray(D3DXHANDLE hParameter, Matrix4 * pMatrix, UINT Count)
		{
			V(m_ptr->GetMatrixTransposeArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
		}

		void GetMatrixTransposePointerArray(D3DXHANDLE hParameter, Matrix4 ** ppMatrix, UINT Count)
		{
			V(m_ptr->GetMatrixTransposePointerArray(hParameter, (D3DXMATRIX **)ppMatrix, Count));
		}

		D3DXHANDLE GetParameter(D3DXHANDLE hParameter, UINT Index)
		{
			D3DXHANDLE ret = m_ptr->GetParameter(hParameter, Index);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName)
		{
			D3DXHANDLE ret = m_ptr->GetParameterByName(hParameter, pName);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic)
		{
			D3DXHANDLE ret = m_ptr->GetParameterBySemantic(hParameter, pSemantic);
			_ASSERT(ret);
			return ret;
		}

		D3DXPARAMETER_DESC GetParameterDesc(D3DXHANDLE hParameter)
		{
			D3DXPARAMETER_DESC desc;
			V(m_ptr->GetParameterDesc(hParameter, &desc));
			return desc;
		}

		D3DXHANDLE GetParameterElement(D3DXHANDLE hParameter, UINT ElementIndex)
		{
			D3DXHANDLE ret = m_ptr->GetParameterElement(hParameter, ElementIndex);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetPass(D3DXHANDLE hTechnique, UINT Index)
		{
			D3DXHANDLE ret = m_ptr->GetPass(hTechnique, Index);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetPassByName(D3DXHANDLE hTechnique, LPCSTR pName)
		{
			D3DXHANDLE ret = m_ptr->GetPassByName(hTechnique, pName);
			_ASSERT(ret);
			return ret;
		}

		D3DXPASS_DESC GetPassDesc(D3DXHANDLE hPass)
		{
			D3DXPASS_DESC desc;
			V(m_ptr->GetPassDesc(hPass, &desc));
			return desc;
		}

		CComPtr<IDirect3DPixelShader9> GetPixelShader(D3DXHANDLE hParameter)
		{
			CComPtr<IDirect3DPixelShader9> PShader;
			V(m_ptr->GetPixelShader(hParameter, &PShader));
			return PShader;
		}

		LPCSTR GetString(D3DXHANDLE hParameter)
		{
			LPCSTR ret;
			V(m_ptr->GetString(hParameter, &ret));
			return ret;
		}

		D3DXHANDLE GetTechnique(UINT Index)
		{
			D3DXHANDLE ret = m_ptr->GetTechnique(Index);
			_ASSERT(ret);
			return ret;
		}

		D3DXHANDLE GetTechniqueByName(LPCSTR pName)
		{
			D3DXHANDLE ret = m_ptr->GetTechniqueByName(pName);
			_ASSERT(ret);
			return ret;
		}

		D3DXTECHNIQUE_DESC GetTechniqueDesc(D3DXHANDLE hTechnique)
		{
			D3DXTECHNIQUE_DESC desc;
			V(m_ptr->GetTechniqueDesc(hTechnique, &desc));
			return desc;
		}

		CComPtr<IDirect3DBaseTexture9> GetTexture(D3DXHANDLE hParameter)
		{
			CComPtr<IDirect3DBaseTexture9> Texture;
			V(m_ptr->GetTexture(hParameter, &Texture));
			return Texture;
		}

		void GetValue(D3DXHANDLE hParameter, LPVOID pData, UINT Bytes)
		{
			V(m_ptr->GetValue(hParameter, pData, Bytes));
		}

		Vector4 GetVector(D3DXHANDLE hParameter)
		{
			Vector4 ret;
			V(m_ptr->GetVector(hParameter, (D3DXVECTOR4 *)&ret));
			return ret;
		}

		void GetVectorArray(D3DXHANDLE hParameter, Vector4 * pVector, UINT Count)
		{
			V(m_ptr->GetVectorArray(hParameter, (D3DXVECTOR4 *)pVector, Count));
		}

		CComPtr<IDirect3DVertexShader9> GetVertexShader(D3DXHANDLE hParameter)
		{
			CComPtr<IDirect3DVertexShader9> VShader;
			V(m_ptr->GetVertexShader(hParameter, &VShader));
			return VShader;
		}

		void SetArrayRange(D3DXHANDLE hParameter, UINT Start, UINT Stop)
		{
			V(m_ptr->SetArrayRange(hParameter, Start, Stop));
		}

		void SetBool(D3DXHANDLE hParameter, BOOL b)
		{
			V(m_ptr->SetBool(hParameter, b));
		}

		void SetBoolArray(D3DXHANDLE hParameter, const BOOL * pB, UINT Count)
		{
			V(m_ptr->SetBoolArray(hParameter, pB, Count));
		}

		void SetFloat(D3DXHANDLE hParameter, FLOAT f)
		{
			V(m_ptr->SetFloat(hParameter, f));
		}

		void SetFloatArray(D3DXHANDLE hParameter, const FLOAT * pf, UINT Count)
		{
			V(m_ptr->SetFloatArray(hParameter, pf, Count));
		}

		void SetInt(D3DXHANDLE hParameter, INT n)
		{
			V(m_ptr->SetInt(hParameter, n));
		}

		void SetIntArray(D3DXHANDLE hParameter, const INT * pn, UINT Count)
		{
			V(m_ptr->SetIntArray(hParameter, pn, Count));
		}

		void SetMatrix(D3DXHANDLE hParameter, const Matrix4 & Matrix)
		{
			V(m_ptr->SetMatrix(hParameter, (D3DXMATRIX *)&Matrix));
		}

		void SetMatrixArray(D3DXHANDLE hParameter, const Matrix4 * pMatrix, UINT Count)
		{
			V(m_ptr->SetMatrixArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
		}

		void SetMatrixPointerArray(D3DXHANDLE hParameter, const Matrix4 ** ppMatrix, UINT Count)
		{
			V(m_ptr->SetMatrixPointerArray(hParameter, (const D3DXMATRIX **)ppMatrix, Count));
		}

		void SetMatrixTranspose(D3DXHANDLE hParameter, const Matrix4 & Matrix)
		{
			V(m_ptr->SetMatrixTranspose(hParameter, (D3DXMATRIX *)&Matrix));
		}

		void SetMatrixTransposeArray(D3DXHANDLE hParameter, const Matrix4 * pMatrix, UINT Count)
		{
			V(m_ptr->SetMatrixTransposeArray(hParameter, (D3DXMATRIX *)pMatrix, Count));
		}

		void SetMatrixTransposePointerArray(D3DXHANDLE hParameter, const Matrix4 ** ppMatrix, UINT Count)
		{
			V(m_ptr->SetMatrixTransposePointerArray(hParameter, (const D3DXMATRIX **)ppMatrix, Count));
		}

		void SetString(D3DXHANDLE hParameter, LPCSTR pString)
		{
			V(m_ptr->SetString(hParameter, pString));
		}

		void SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture)
		{
			V(m_ptr->SetTexture(hParameter, pTexture));
		}

		void SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes)
		{
			V(m_ptr->SetValue(hParameter, pData, Bytes));
		}

		void SetVector(D3DXHANDLE hParameter, const Vector4 & Vector)
		{
			V(m_ptr->SetVector(hParameter, (D3DXVECTOR4 *)&Vector));
		}

		void SetVectorArray(D3DXHANDLE hParameter, const Vector4 * pVector, UINT Count)
		{
			V(m_ptr->SetVectorArray(hParameter, (D3DXVECTOR4 *)pVector, Count));
		}
	};

	class Effect;

	typedef boost::shared_ptr<Effect> EffectPtr;

	class Effect : public BaseEffect
	{
	protected:
		Effect(ID3DXEffect * pd3dxEffect)
			: BaseEffect(pd3dxEffect)
		{
		}

	public:
		static EffectPtr CreateEffect(
			LPDIRECT3DDEVICE9 pDevice,
			LPCVOID pSrcData,
			UINT SrcDataLen,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0,
			LPD3DXEFFECTPOOL pPool = NULL);

		static EffectPtr CreateEffectFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pSrcFile,
			CONST D3DXMACRO * pDefines = NULL,
			LPD3DXINCLUDE pInclude = NULL,
			DWORD Flags = 0,
			LPD3DXEFFECTPOOL pPool = NULL);

		virtual void OnResetDevice(void)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->OnResetDevice());
		}

		virtual void OnLostDevice(void)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->OnLostDevice());
		}

	public:
		void ApplyParameterBlock(D3DXHANDLE hParameterBlock)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->ApplyParameterBlock(hParameterBlock));
		}

		UINT Begin(DWORD Flags = 0)
		{
			UINT Passes;
			V(static_cast<ID3DXEffect *>(m_ptr)->Begin(&Passes, Flags));
			return Passes;
		}

		void BeginParameterBlock(void)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->BeginParameterBlock());
		}

		void BeginPass(UINT Pass)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->BeginPass(Pass));
		}

		CComPtr<ID3DXEffect> CloneEffect(LPDIRECT3DDEVICE9 pDevice)
		{
			CComPtr<ID3DXEffect> Effect;
			V(static_cast<ID3DXEffect *>(m_ptr)->CloneEffect(pDevice, &Effect));
			return Effect;
		}

		void CommitChanges(void)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->CommitChanges());
		}

		void DeleteParameterBlock(D3DXHANDLE hParameterBlock)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->DeleteParameterBlock(hParameterBlock));
		}

		void End(void)
		{
			static_cast<ID3DXEffect *>(m_ptr)->End();
		}

		D3DXHANDLE EndParameterBlock(void)
		{
			D3DXHANDLE ret = static_cast<ID3DXEffect *>(m_ptr)->EndParameterBlock();
			_ASSERT(ret);
			return ret;
		}

		void EndPass(void)
		{
			static_cast<ID3DXEffect *>(m_ptr)->EndPass();
		}

		D3DXHANDLE FindNextValidTechnique(D3DXHANDLE hTechnique = NULL)
		{
			D3DXHANDLE ret;
			V(static_cast<ID3DXEffect *>(m_ptr)->FindNextValidTechnique(hTechnique, &ret));
			return ret;
		}

		D3DXHANDLE GetCurrentTechnique(void)
		{
			D3DXHANDLE ret = static_cast<ID3DXEffect *>(m_ptr)->GetCurrentTechnique();
			_ASSERT(ret);
			return ret;
		}

		CComPtr<IDirect3DDevice9> GetDevice(void)
		{
			CComPtr<IDirect3DDevice9> Device;
			V(static_cast<ID3DXEffect *>(m_ptr)->GetDevice(&Device));
			return Device;
		}

		CComPtr<ID3DXEffectPool> GetPool(void)
		{
			CComPtr<ID3DXEffectPool> Pool;
			V(static_cast<ID3DXEffect *>(m_ptr)->GetPool(&Pool));
			return Pool;
		}

		CComPtr<ID3DXEffectStateManager> GetStateManager(void)
		{
			CComPtr<ID3DXEffectStateManager> Manager;
			V(static_cast<ID3DXEffect *>(m_ptr)->GetStateManager(&Manager));
			return Manager;
		}

		BOOL IsParameterUsed(D3DXHANDLE hParameter, D3DXHANDLE hTechnique)
		{
			return static_cast<ID3DXEffect *>(m_ptr)->IsParameterUsed(hParameter, hTechnique);
		}

		void SetRawValue(D3DXHANDLE Handle, void * pData, DWORD OffsetInBytes, DWORD Bytes)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->SetRawValue(Handle, pData, OffsetInBytes, Bytes));
		}

		void SetStateManager(CComPtr<ID3DXEffectStateManager> Manager)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->SetStateManager(Manager));
		}

		void SetTechnique(D3DXHANDLE hTechnique)
		{
			V(static_cast<ID3DXEffect *>(m_ptr)->SetTechnique(hTechnique));
		}

		bool ValidateTechnique(D3DXHANDLE hTechnique)
		{
			return SUCCEEDED(static_cast<ID3DXEffect *>(m_ptr)->ValidateTechnique(hTechnique));
		}
	};
}
