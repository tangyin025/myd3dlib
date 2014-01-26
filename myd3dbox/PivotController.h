#pragma once

class PivotController
{
public:
	static const float PivotRadius;

	static const float PivotHeight;

	static const float PivotOffset;

	static const D3DCOLOR PivotAxisXColor;

	static const D3DCOLOR PivotAxisYColor;

	static const D3DCOLOR PivotAxisZColor;

	static const D3DCOLOR PivotDragAxisColor;

	my::Vector3 m_Pos;

	my::Matrix4 m_World;

	enum DragAxis
	{
		DragAxisNone,
		DragAxisX,
		DragAxisY,
		DragAxisZ,
	};

	DragAxis m_DragAxis;

	my::Vector3 m_DragPos;

	my::Vector3 m_DragPt;

	my::Vector3 m_DragNormal;

	float m_DragDist;

public:
	PivotController(void)
		: m_DragAxis(DragAxisNone)
		, m_Pos(0,0,0)
		, m_World(my::Matrix4::Identity())
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

	void BuildConeVertices(VertexList & vertex_list, const float radius, const float height, const float offset, const D3DCOLOR color);

	void UpdateWorld(const my::Matrix4 & ViewProj, UINT ViewWidth);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);
};
