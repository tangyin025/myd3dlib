#pragma once

class PivotController
{
public:
	my::Matrix4 m_World;

public:
	PivotController(void);

	~PivotController(void);

	void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);
};
