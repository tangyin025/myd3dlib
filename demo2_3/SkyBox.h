#pragma once

#include <myD3dLib.h>

class SkyBox;

typedef boost::shared_ptr<SkyBox> SkyBoxPtr;

class SkyBox : public my::DeviceRelatedObjectBase
{
protected:
	CComPtr<IDirect3DDevice9> m_Device;

	my::CubeTexturePtr m_cubeTexture;

	my::VertexBufferPtr m_vertBuffer;

	CComPtr<IDirect3DVertexDeclaration9> m_vertDecl;

	my::EffectPtr m_effect;

	SkyBox(LPDIRECT3DDEVICE9 pD3DDevice);

public:
	virtual ~SkyBox(void);

	static SkyBoxPtr CreateSkyBox(LPDIRECT3DDEVICE9 pD3DDevice);

	void OnResetDevice(void);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	void UpdateVertexBuffer(void);

	void Render(float fElapsedTime, const my::Matrix4 & mWorldViewProj);
};
