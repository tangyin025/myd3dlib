#pragma once

class PivotController
{
public:
	enum PivotDragAxis
	{
		PivotDragNone = 0,
		PivotDragAxisX,
		PivotDragAxisY,
		PivotDragAxisZ,
	};

	enum PivotMode
	{
		PivotModeMove,
		PivotModeRot,
	};

	my::Vector3 m_Pos;

	my::Quaternion m_Rot;

	PivotDragAxis m_PivotDragAxis;

	PivotMode m_PivotMode;

	float m_Scale;

	my::Matrix4 m_World;

	my::Vector3 m_DragPos;

	my::Quaternion m_DragRot;

	my::Vector3 m_DragPt;

	my::Plane m_DragPlane;

	bool m_Captured;

public:
	PivotController(void);

	~PivotController(void);

	void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera, const D3DSURFACE_DESC * desc);

	void DrawMoveController(IDirect3DDevice9 * pd3dDevice);

	void DrawRotController(IDirect3DDevice9 * pd3dDevice);

	void UpdateScale(const my::Camera * camera, const D3DSURFACE_DESC * desc);

	void UpdateWorld(void);

	bool OnLButtonDown(const my::Ray & ray);

	bool OnMoveControllerLButtonDown(const my::Ray & ray);

	bool OnRotControllerLButtonDown(const my::Ray & ray);

	bool OnMouseMove(const my::Ray & ray);

	bool OnMoveControllerMouseMove(const my::Ray & ray);

	bool OnRotControllerMouseMove(const my::Ray & ray);

	bool OnLButtonUp(const my::Ray & ray);
};
