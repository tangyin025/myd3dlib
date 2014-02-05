#pragma once

class PivotController
{
public:
	static const float MovePivotRadius;

	static const float MovePivotHeight;

	static const float MovePivotOffset;

	static const float RotationPivotRadius;

	static const D3DCOLOR PivotAxisXColor;

	static const D3DCOLOR PivotAxisYColor;

	static const D3DCOLOR PivotAxisZColor;

	static const D3DCOLOR PivotDragAxisColor;

	static const D3DCOLOR PivotGrayAxisColor;

	static const my::Matrix4 mat_to_y;

	static const my::Matrix4 mat_to_z;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Matrix4 m_ViewTranslation;

	enum PivotMode
	{
		PivotModeMove,
		PivotModeRotation,
	};

	PivotMode m_PovitMode;

	enum DragAxis
	{
		DragAxisNone,
		DragAxisX,
		DragAxisY,
		DragAxisZ,
	};

	DragAxis m_DragAxis;

	my::Vector3 m_DragPos;

	my::Quaternion m_DragRot;

	my::Vector3 m_DragPt;

	my::Vector3 m_DragNormal;

	float m_DragDist;

public:
	PivotController(void)
		: m_PovitMode(PivotModeMove)
		, m_DragAxis(DragAxisNone)
		, m_Position(0,0,0)
		, m_Rotation(my::Quaternion::Identity())
		, m_ViewTranslation(my::Matrix4::Identity())
	{
	}

	~PivotController(void)
	{
	}

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

	void BuildConeVertices(VertexList & vertex_list, const float radius, const float height, const float offset, const D3DCOLOR color, const my::Matrix4 & Transform);

	void BuildCircleVertices(VertexList & vertex_list, const float radius, const D3DCOLOR color, const my::Matrix4 & Transform, const my::Vector3 & ViewPos, const float discrm);

	void UpdateViewTransform(const my::Matrix4 & ViewProj, UINT ViewWidth);

	void DrawMoveController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);

	void DrawRotationController(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);

	BOOL OnMoveControllerLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray);

	BOOL OnRotationControllerButtonDown(const std::pair<my::Vector3, my::Vector3> & ray);

	BOOL OnMoveControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray);

	BOOL OnRotationControllerMouseMove(const std::pair<my::Vector3, my::Vector3> & ray);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);

	virtual BOOL OnLButtonDown(const std::pair<my::Vector3, my::Vector3> & ray);

	virtual BOOL OnMouseMove(const std::pair<my::Vector3, my::Vector3> & ray);
};
