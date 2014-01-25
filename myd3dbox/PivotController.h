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

	static const D3DCOLOR PivotHightLightAxisColor;

	enum HightLightAxis
	{
		HightLightAxisNone,
		HightLightAxisX,
		HightLightAxisY,
		HightLightAxisZ,
	};

	HightLightAxis m_HightLightAxis;

	my::Vector3 m_Pos;

	my::Matrix4 m_World;

public:
	PivotController(void)
		: m_HightLightAxis(HightLightAxisNone)
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

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, const my::Camera * camera);
};
