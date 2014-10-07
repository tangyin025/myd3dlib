#pragma once

#define SHADOW_MAP_SIZE 512

class RenderPipeline
{
protected:
	my::Texture2D m_ShadowMap;

	my::Surface m_ShadowMapDS;

	my::Texture2D m_GBufferN;

	my::Texture2D m_GBufferL;

	my::OctreeRootPtr m_OctScene;

public:
	RenderPipeline(void);

	virtual ~RenderPipeline(void);

	HRESULT OnCreate(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	HRESULT OnReset(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	void OnLost(void);

	void OnDestroy(void);

	void OnRender(IDirect3DDevice9 * pd3dDevice, double fTime, float fElapsedTime, const my::Matrix4 & ViewProj);

	void AddStaticComponent(my::ComponentPtr comp, float threshold = 1.0f);

	void RemoveStaticComponent(my::ComponentPtr comp);
};
