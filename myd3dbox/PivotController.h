#pragma once

class PivotControllerBase
{
public:
	static const float MovePivotRadius;

	static const float MovePivotHeight;

	static const float MovePivotOffset;

	static const float MovePlaneWidth;

	static const float RotationPivotRadius;

	static const D3DCOLOR PivotAxisXColor;

	static const D3DCOLOR PivotAxisYColor;

	static const D3DCOLOR PivotAxisZColor;

	static const D3DCOLOR PivotHighLightAxisColor;

	static const D3DCOLOR PivotGrayAxisColor;

	static const my::Matrix4 mat_x2y;

	static const my::Matrix4 mat_x2z;

	struct Vertex
	{
		my::Vector3 pos;
		my::Vector3 normal;
		D3DCOLOR color;
		Vertex(void)
		{
		}
		Vertex(const my::Vector3 & _pos, const my::Vector3 & _normal, D3DCOLOR _color)
			: pos(_pos), normal(_normal), color(_color)
		{
		}
	};

	typedef std::vector<Vertex> VertexList;

	enum HighLightAxis
	{
		HighLightAxisNone,
		HighLightAxisX,
		HighLightAxisY,
		HighLightAxisZ,
		HighLightPlaneX,
		HighLightPlaneY,
		HighLightPlaneZ,
	};

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Matrix4 m_ViewTransform;

	PivotControllerBase(void)
		: m_Position(0,0,0)
		, m_Rotation(my::Quaternion::Identity())
		, m_ViewTransform(my::Matrix4::Identity())
	{
	}

	void UpdateViewTransform(const my::Matrix4 & ViewProj, UINT ViewWidth);

	void DrawMoveController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera, HighLightAxis high_light_axis);

	void DrawRotationController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera, HighLightAxis high_light_axis);
};

class PivotController : public PivotControllerBase
{
public:
	enum PivotMode
	{
		PivotModeMove,
		PivotModeRotation,
	};

	PivotMode m_PovitMode;

	bool m_bVisible;

	HighLightAxis m_HighLightAxis;

	my::Vector3 m_DragPos;

	my::Quaternion m_DragRot;

	my::Vector3 m_DragPt;

	my::Vector3 m_DragNormal;

	float m_DragDist;

public:
	PivotController(void)
		: m_PovitMode(PivotModeMove)
		, m_bVisible(false)
		, m_HighLightAxis(HighLightAxisNone)
	{
	}

	~PivotController(void)
	{
	}

	BOOL OnMoveControllerLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray);

	BOOL OnRotationControllerButtonDown(const std::pair<my::Vector3, my::Vector3> & ray);

	BOOL OnMoveControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray);

	BOOL OnRotationControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);

	virtual BOOL OnLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray);

	virtual BOOL OnMouseMove(const std::pair<my::Vector3, my::Vector3> & ray);
};
