#pragma once

class PivotController
{
public:
	enum PivotDragMode
	{
		PivotDragNone = 0,
		PivotDragMoveX,
		PivotDragMoveY,
		PivotDragMoveZ,
		PivotDragRotX,
		PivotDragRotY,
		PivotDragRotZ,
	};

	my::Vector3 m_Pos;

	PivotDragMode m_PivotDragMode;

	float m_Scale;

	my::Matrix4 m_World;

	my::Vector3 m_DragPos;

	my::Vector3 m_DragPt;

	my::Plane m_DragPlane;

public:
	PivotController(void);

	~PivotController(void);

	void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);

	void UpdateScale(const my::Matrix4 & View);

	void UpdateWorld(void);

	bool OnLButtonDown(const my::Ray & ray);

	bool OnMouseMove(const my::Ray & ray);

	bool OnLButtonUp(const my::Ray & ray);
};
