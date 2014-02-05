#pragma once

class CMFCPropertyGridCtrl;

class TreeNodeBase
{
public:
	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

public:
	TreeNodeBase(void)
		: m_Position(0,0,0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1,1,1)
	{
	}

	virtual ~TreeNodeBase(void)
	{
	}

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World) = 0;

	virtual void SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl) = 0;
};

typedef boost::shared_ptr<TreeNodeBase> TreeNodeBasePtr;

class MeshTreeNode : public TreeNodeBase
{
public:
	my::OgreMeshPtr m_Mesh;

	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

public:
	MeshTreeNode(void)
	{
	}

	virtual ~MeshTreeNode(void);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World);

	virtual void SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl);
};

typedef boost::shared_ptr<MeshTreeNode> MeshTreeNodePtr;
