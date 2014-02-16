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

	virtual bool RayTest(const std::pair<my::Vector3, my::Vector3> & ray, const my::Matrix4 & World) = 0;
};

typedef boost::shared_ptr<TreeNodeBase> TreeNodeBasePtr;

class MeshTreeNode : public TreeNodeBase
{
public:
	struct Callback
	{
		template <typename IndexType>
		static void Func(udword triangle_index, Opcode::VertexPointers& triangle, void* user_data)
		{
			Callback * cb = (Callback *)user_data;
			for(int i = 0; i < 3; i++)
			{
				IndexType vertex_index = *((IndexType *)cb->m_pIndices + triangle_index * 3 + i);
				unsigned char * pVertex = (unsigned char *)cb->m_pVertices + vertex_index * cb->m_pMesh->GetNumBytesPerVertex();
				triangle.Vertex[i] = (Point *)pVertex;
			}
		}

		my::OgreMesh * m_pMesh;
		VOID * m_pIndices;
		VOID * m_pVertices;
	};

	my::OgreMeshPtr m_Mesh;

	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

	Opcode::MeshInterface m_OpcMeshInterface;

	Callback m_OpcMeshInterfaceCB;

	Opcode::Model m_OpcMode;

public:
	MeshTreeNode(void)
	{
	}

	bool LoadFromMesh(LPCTSTR lpszMesh);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World);

	virtual void SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl);

	virtual bool RayTest(const std::pair<my::Vector3, my::Vector3> & ray, const my::Matrix4 & World);
};

typedef boost::shared_ptr<MeshTreeNode> MeshTreeNodePtr;
