#pragma once

#include <myd3dlib.h>
#include "Camera.h"

class BaseScene
{
public:
	virtual ~BaseScene(void)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime) = 0;

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime) = 0;
};

typedef boost::shared_ptr<BaseScene> BaseScenePtr;

class Scene : public BaseScene
{
public:
	my::MeshPtr m_mesh;

	my::TexturePtr m_texture;

	my::EffectPtr m_effect;

	BaseCameraPtr m_camera;

	BaseCameraPtr GetCamera(void) const
	{
		return m_camera;
	}

	void SetCamera(BaseCameraPtr camera)
	{
		m_camera = camera;
	}

public:
	Scene(void);

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);
};