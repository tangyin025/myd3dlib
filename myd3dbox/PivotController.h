#pragma once

class PivotController
{
protected:
	my::Vector3 m_Pos;

public:
	PivotController(void)
		: m_Pos(0,0,0)
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

	virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
