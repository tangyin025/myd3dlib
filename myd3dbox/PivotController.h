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

public:
	PivotController(void);

	~PivotController(void);

	void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);

	void UpdateScale(const my::Matrix4 & View);

	void UpdateWorld(void);

	bool OnLButtonDown(const my::Ray & ray);
};
