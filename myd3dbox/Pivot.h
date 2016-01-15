#pragma once

class Pivot
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

	PivotMode m_Mode;

	my::Vector3 m_Pos;

	my::Quaternion m_Rot;

	PivotDragAxis m_DragAxis;

	my::Vector3 m_DragPos;

	my::Quaternion m_DragRot;

	my::Vector3 m_DragPt;

	my::Plane m_DragPlane;

	my::Vector3 m_DragDeltaPos;

	my::Quaternion m_DragDeltaRot;

	bool m_Captured;

public:
	Pivot(void);

	~Pivot(void);

	void Draw(IDirect3DDevice9 * pd3dDevice, const my::BaseCamera * camera, const D3DSURFACE_DESC * desc, float Scale);

	void DrawMoveController(IDirect3DDevice9 * pd3dDevice, float Scale);

	void DrawRotController(IDirect3DDevice9 * pd3dDevice, float Scale);

	my::Matrix4 CalculateWorld(float Scale);

	bool OnLButtonDown(const my::Ray & ray, float Scale);

	bool OnMoveControllerLButtonDown(const my::Ray & ray, float Scale);

	bool OnRotControllerLButtonDown(const my::Ray & ray, float Scale);

	bool OnMouseMove(const my::Ray & ray, float Scale);

	bool OnMoveControllerMouseMove(const my::Ray & ray, float Scale);

	bool OnRotControllerMouseMove(const my::Ray & ray, float Scale);

	bool OnLButtonUp(const my::Ray & ray);
};
