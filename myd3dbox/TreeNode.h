#pragma once

class TreeNodeBase
{
public:
	my::Matrix4 m_World;

public:
	TreeNodeBase(void)
		: m_World(my::Matrix4::Identity())
	{
	}

	virtual ~TreeNodeBase(void)
	{
	}

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World) = 0;
};

typedef boost::shared_ptr<TreeNodeBase> TreeNodeBasePtr;

class StaticMeshTreeNode : public TreeNodeBase
{
public:
	my::OgreMeshPtr m_Mesh;

	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

public:
	StaticMeshTreeNode(void)
	{
	}

	virtual ~StaticMeshTreeNode(void);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World);
};

typedef boost::shared_ptr<StaticMeshTreeNode> StaticMeshTreeNodePtr;
