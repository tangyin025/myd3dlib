#pragma once

class CMFCPropertyGridCtrl;

class CSimpleProp;

class TreeNodeBase : public CObject
{
public:
	static const my::Matrix4 mat_y2x;

	static void SetPropertyFloat(CSimpleProp * pProp, const float * pValue);

	static void GetPropertyFloat(const CSimpleProp * pProp, float * pValue);

	static void SetPropertyString(CSimpleProp * pProp, const CString * pValue);

	static void GetPropertyString(const CSimpleProp * pProp, CString * pValue);

	static void SetPropertyQuatX(CSimpleProp * pProp, const my::Quaternion * pValue);

	static void GetPropertyQuatX(const CSimpleProp * pProp, my::Quaternion * pValue);

	static void SetPropertyQuatY(CSimpleProp * pProp, const my::Quaternion * pValue);

	static void GetPropertyQuatY(const CSimpleProp * pProp, my::Quaternion * pValue);

	static void SetPropertyQuatZ(CSimpleProp * pProp, const my::Quaternion * pValue);

	static void GetPropertyQuatZ(const CSimpleProp * pProp, my::Quaternion * pValue);

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

	virtual void Serialize(CArchive & ar);

	virtual void SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World) = 0;

	virtual bool RayTest(const std::pair<my::Vector3, my::Vector3> & ray, const my::Matrix4 & World) = 0;
};

typedef boost::shared_ptr<TreeNodeBase> TreeNodeBasePtr;

class TreeNodeMesh : public TreeNodeBase
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
	TreeNodeMesh(void)
	{
	}

	virtual void Serialize(CArchive & ar);

	bool LoadFromFile(LPCTSTR lpszPath);

	virtual void SetupProperties(CMFCPropertyGridCtrl * pPropertyGridCtrl);

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Matrix4 & World);

	virtual bool RayTest(const std::pair<my::Vector3, my::Vector3> & ray, const my::Matrix4 & World);

	DECLARE_SERIAL(TreeNodeMesh)
};

typedef boost::shared_ptr<TreeNodeMesh> TreeNodeMeshPtr;
