#include "stdafx.h"
#include "myMesh.h"
#include "myResource.h"
#include "libc.h"
#include "rapidxml.hpp"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

#define DEFINE_XML_NODE(node_v, node_p, node_s) \
	node_v = node_p->first_node(#node_s); \
	if(NULL == node_v) \
		THROW_CUSEXCEPTION("cannot find " #node_s)

#define DEFINE_XML_NODE_SIMPLE(node_s, parent_s) \
	rapidxml::xml_node<char> * node_##node_s; \
	DEFINE_XML_NODE(node_##node_s, node_##parent_s, node_s)

#define DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s) \
	attr_v = node_p->first_attribute(#attr_s); \
	if(NULL == attr_v) \
		THROW_CUSEXCEPTION("cannot find " #attr_s)

#define DEFINE_XML_ATTRIBUTE_SIMPLE(attr_s, parent_s) \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE(attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_INT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = atoi(attr_v->value())

#define DEFINE_XML_ATTRIBUTE_INT_SIMPLE(attr_s, parent_s) \
	int attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_INT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_FLOAT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = (float)atof(attr_v->value())

#define DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(attr_s, parent_s) \
	float attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_FLOAT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_BOOL(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = 0 == _stricmp(attr_v->value(), "true")

#define DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(attr_s, parent_s) \
	bool attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_BOOL(attr_s, attr_##attr_s, node_##parent_s, attr_s)

std::vector<D3DVERTEXELEMENT9> D3DVERTEXELEMENT9Set::BuildVertexElementList(void) const
{
	std::vector<D3DVERTEXELEMENT9> ret;
	const_iterator elem_iter = begin();
	for(; elem_iter != end(); elem_iter++)
	{
		ret.push_back(*elem_iter);
	}

	D3DVERTEXELEMENT9 elem_end = {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0};
	ret.push_back(elem_end);
	return ret;
}

UINT D3DVERTEXELEMENT9Set::CalculateVertexStride(DWORD Stream) const
{
	return D3DXGetDeclVertexSize(&BuildVertexElementList()[0], Stream);
}

CComPtr<IDirect3DVertexDeclaration9> D3DVERTEXELEMENT9Set::CreateVertexDeclaration(LPDIRECT3DDEVICE9 pDevice) const
{
	CComPtr<IDirect3DVertexDeclaration9> ret;

	HRESULT hres = pDevice->CreateVertexDeclaration((D3DVERTEXELEMENT9 *)&BuildVertexElementList()[0], &ret);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return ret;
}

void VertexBuffer::CreateVertexBuffer(
	LPDIRECT3DDEVICE9 pD3DDevice,
	const D3DVERTEXELEMENT9Set & VertexElemSet,
	WORD Stream)
{
	Create(pD3DDevice, VertexElemSet, Stream);
}

void VertexBuffer::OnResetDevice(void)
{
	_ASSERT(!m_VertexBuffer);
	_ASSERT(!m_MemVertexBuffer.empty());

	HRESULT hres = m_Device->CreateVertexBuffer(m_MemVertexBuffer.size(), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_VertexBuffer, NULL);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	UpdateVertexBuffer();
}

void VertexBuffer::OnLostDevice(void)
{
	m_VertexBuffer.Release();
}

void VertexBuffer::OnDestroyDevice(void)
{
	_ASSERT(!m_VertexBuffer);

	m_Device.Release();
}

void VertexBuffer::UpdateVertexBuffer(void)
{
	_ASSERT(m_VertexBuffer);

	void * pVertices;
	UINT LockSize = m_MemVertexBuffer.size();
	if(SUCCEEDED(m_VertexBuffer->Lock(0, LockSize, &pVertices, 0)))
	{
		memcpy(pVertices, &m_MemVertexBuffer[0], LockSize);
		m_VertexBuffer->Unlock();
	}
}

void VertexBuffer::ResizeVertexBufferLength(UINT NumVertices)
{
	m_MemVertexBuffer.resize(NumVertices * m_vertexStride, 0);

	m_NumVertices = NumVertices;
}

void VertexBuffer::SetPosition(UINT Index, const D3DVERTEXELEMENT9Set::PositionType & Position, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetPosition(&m_MemVertexBuffer[Index * m_vertexStride], Position, m_Stream, UsageIndex);
}

void VertexBuffer::SetBinormal(UINT Index, const D3DVERTEXELEMENT9Set::BinormalType & Binormal, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetBinormal(&m_MemVertexBuffer[Index * m_vertexStride], Binormal, m_Stream, UsageIndex);
}

void VertexBuffer::SetTangent(UINT Index, const D3DVERTEXELEMENT9Set::TangentType & Tangent, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetTangent(&m_MemVertexBuffer[Index * m_vertexStride], Tangent, m_Stream, UsageIndex);
}

void VertexBuffer::SetNormal(UINT Index, const D3DVERTEXELEMENT9Set::NormalType & Normal, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetNormal(&m_MemVertexBuffer[Index * m_vertexStride], Normal, m_Stream, UsageIndex);
}

void VertexBuffer::SetTexcoord(UINT Index, const D3DVERTEXELEMENT9Set::TexcoordType & Texcoord, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetTexcoord(&m_MemVertexBuffer[Index * m_vertexStride], Texcoord, m_Stream, UsageIndex);
}

void VertexBuffer::SetBlendIndices(UINT Index, const D3DVERTEXELEMENT9Set::BlendIndicesType & BlendIndices, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetBlendIndices(&m_MemVertexBuffer[Index * m_vertexStride], BlendIndices, m_Stream, UsageIndex);
}

void VertexBuffer::SetBlendWeights(UINT Index, const D3DVERTEXELEMENT9Set::BlendWeightsType & BlendWeights, BYTE UsageIndex)
{
	_ASSERT(Index < m_NumVertices && m_MemVertexBuffer.size() == m_NumVertices * m_vertexStride);

	m_VertexElemSet.SetBlendWeights(&m_MemVertexBuffer[Index * m_vertexStride], BlendWeights, m_Stream, UsageIndex);
}

void IndexBuffer::CreateIndexBuffer(LPDIRECT3DDEVICE9 pD3DDevice)
{
	Create(pD3DDevice);
}

void IndexBuffer::OnResetDevice(void)
{
	_ASSERT(!m_IndexBuffer);

	HRESULT hres = m_Device->CreateIndexBuffer(m_MemIndexBuffer.size() * sizeof(UIntList::value_type), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_IndexBuffer, NULL);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	UpdateIndexBuffer();
}

void IndexBuffer::OnLostDevice(void)
{
	m_IndexBuffer.Release();
}

void IndexBuffer::OnDestroyDevice(void)
{
	_ASSERT(!m_IndexBuffer);
	m_IndexBuffer.Release();
	m_Device.Release();
}

void IndexBuffer::UpdateIndexBuffer(void)
{
	_ASSERT(m_IndexBuffer);

	void * pIndices;
	UINT LockSize = m_MemIndexBuffer.size() * sizeof(UIntList::value_type);
	if(SUCCEEDED(m_IndexBuffer->Lock(0, LockSize, &pIndices, 0)))
	{
		memcpy(pIndices, &m_MemIndexBuffer[0], LockSize);
		m_IndexBuffer->Unlock();
	}
}

void IndexBuffer::ResizeIndexBufferLength(UINT NumIndices)
{
	m_MemIndexBuffer.resize(NumIndices, 0);
}

void IndexBuffer::SetIndex(UINT Index, unsigned int IndexValue)
{
	_ASSERT(Index < m_MemIndexBuffer.size());

	m_MemIndexBuffer[Index] = IndexValue;
}

void Mesh::CreateMesh(
	LPDIRECT3DDEVICE9 pD3DDevice,
	DWORD NumFaces,
	DWORD NumVertices,
	CONST LPD3DVERTEXELEMENT9 pDeclaration,
	DWORD Options)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMesh(NumFaces, NumVertices, Options, pDeclaration, pD3DDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFVF(
	LPDIRECT3DDEVICE9 pD3DDevice,
	DWORD NumFaces,
	DWORD NumVertices,
	DWORD FVF,
	DWORD Options)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMeshFVF(NumFaces, NumVertices, Options, FVF, pD3DDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFromX(
	LPDIRECT3DDEVICE9 pD3DDevice,
	LPCSTR pFilename,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXLoadMeshFromXA(
		pFilename, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFromXInMemory(
	LPDIRECT3DDEVICE9 pD3DDevice,
	LPCVOID Memory,
	DWORD SizeOfMemory,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXLoadMeshFromXInMemory(
		Memory, SizeOfMemory, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateBox(
	LPDIRECT3DDEVICE9 pd3dDevice,
	FLOAT Width,
	FLOAT Height,
	FLOAT Depth,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateBox(pd3dDevice, Width, Height, Depth, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateCylinder(
	LPDIRECT3DDEVICE9 pd3dDevice,
	FLOAT Radius1,
	FLOAT Radius2,
	FLOAT Length,
	UINT Slices,
	UINT Stacks,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateCylinder(pd3dDevice, Radius1, Radius2, Length, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreatePolygon(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT Length,
	UINT Sides,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreatePolygon(pDevice, Length, Sides, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateSphere(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT Radius,
	UINT Slices,
	UINT Stacks,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateSphere(pDevice, Radius, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateTeapot(
	LPDIRECT3DDEVICE9 pDevice,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateTeapot(pDevice, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateTorus(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT InnerRadius,
	FLOAT OuterRadius,
	UINT Sides,
	UINT Rings,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateTorus(pDevice, InnerRadius, OuterRadius, Sides, Rings, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void OgreMesh::CreateMeshFromOgreXml(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPCSTR pFilename,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	FILE * fp;
	if(0 != fopen_s(&fp, pFilename, "rb"))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", pFilename));
	}

	CachePtr cache = ArchiveStreamPtr(new FileArchiveStream(fp))->GetWholeCache();

	CreateMeshFromOgreXmlInMemory(pd3dDevice, (LPCSTR)&(*cache)[0], cache->size(), bComputeTangentFrame, dwMeshOptions);
}

void OgreMesh::CreateMeshFromOgreXmlInMemory(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPCSTR pSrcData,
	UINT srcDataLen,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	std::string xmlStr(pSrcData, srcDataLen);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(&xmlStr[0]);
	}
	catch(rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(e.what());
	}

	rapidxml::xml_node<char> * node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(sharedgeometry, mesh);
	DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexcount, sharedgeometry);
	DEFINE_XML_NODE_SIMPLE(vertexbuffer, sharedgeometry);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(positions, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(normals, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_diffuse, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_specular, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_INT_SIMPLE(texture_coords, vertexbuffer);

	if((dwMeshOptions & ~D3DXMESH_32BIT) && vertexcount >= USHRT_MAX)
	{
		THROW_CUSEXCEPTION("facecount overflow ( >= 2^16 - 1 )");
	}

	if(!positions)
	{
		THROW_CUSEXCEPTION("cannot process non-position vertex");
	}

	_ASSERT(m_VertexElemSet.empty());
	m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreatePositionElement(0, 0, 0));
	WORD offset = sizeof(D3DVERTEXELEMENT9Set::PositionType);

	if(normals || bComputeTangentFrame)
	{
		m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateNormalElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::NormalType);
	}

	if(bComputeTangentFrame)
	{
		m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateTangentElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::TangentType);
	}

	if(texture_coords > MAXBYTE)
	{
		THROW_CUSEXCEPTION("texture coords overflow ( > 255 )");
	}

	for(int i = 0; i < texture_coords; i++)
	{
		m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, i));
		offset += sizeof(D3DVERTEXELEMENT9Set::TexcoordType);
	}

	rapidxml::xml_node<char> * node_boneassignments = node_mesh->first_node("boneassignments");
	WORD indicesOffset = 0, weightsOffset = 0;
	if(node_boneassignments != NULL)
	{
		m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateBlendIndicesElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::BlendIndicesType);

		m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateBlendWeightsElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::BlendWeightsType);
	}

	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	int facecount = 0;
	for(; node_submesh != NULL; node_submesh = node_submesh->next_sibling())
	{
		DEFINE_XML_NODE_SIMPLE(faces, submesh);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
		facecount += count;
	}

	CreateMesh(pd3dDevice, facecount, vertexcount, (D3DVERTEXELEMENT9 *)&m_VertexElemSet.BuildVertexElementList()[0], dwMeshOptions);

	const VOID * pVertices = LockVertexBuffer();
	DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
	for(int vertex_i = 0; node_vertex != NULL && vertex_i < vertexcount; node_vertex = node_vertex->next_sibling(), vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * offset;
		if(positions)
		{
			DEFINE_XML_NODE_SIMPLE(position, vertex);
			D3DVERTEXELEMENT9Set::PositionType & Position = m_VertexElemSet.GetPosition(pVertex);
			float tmp;
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_position, x);
			Position.x = tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_position, y);
			Position.y = tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_position, z);
			Position.z = -tmp;
		}

		if(normals)
		{
			DEFINE_XML_NODE_SIMPLE(normal, vertex);
			D3DVERTEXELEMENT9Set::NormalType & Normal = m_VertexElemSet.GetNormal(pVertex);
			float tmp;
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_normal, x);
			Normal.x = tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_normal, y);
			Normal.y = tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_normal, z);
			Normal.z = -tmp;
		}

		rapidxml::xml_node<char> * node_texcoord = node_vertex->first_node("texcoord");
		for(int i = 0; i < texture_coords && node_texcoord != NULL; i++, node_texcoord = node_texcoord->next_sibling())
		{
			D3DVERTEXELEMENT9Set::TexcoordType & Texcoord = m_VertexElemSet.GetTexcoord(pVertex, i);
			float tmp;
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_texcoord, u);
			Texcoord.x = tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_texcoord, v);
			Texcoord.y = tmp;
		}

		if(node_boneassignments != NULL)
		{
			m_VertexElemSet.SetBlendIndices(pVertex, D3DCOLOR_ARGB(0, 0, 0, 0));
			m_VertexElemSet.SetBlendWeights(pVertex, Vector4::zero);
		}
	}

	if(node_boneassignments != NULL)
	{
		rapidxml::xml_node<char> * node_vertexboneassignment = node_boneassignments->first_node("vertexboneassignment");
		for(; node_vertexboneassignment != NULL; node_vertexboneassignment = node_vertexboneassignment->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexindex, vertexboneassignment);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(boneindex, vertexboneassignment);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(weight, vertexboneassignment);

			if(vertexindex >= vertexcount)
			{
				THROW_CUSEXCEPTION(str_printf("invalid vertex index: %d", vertexindex));
			}

			if(boneindex >= 0xff)
			{
				THROW_CUSEXCEPTION(str_printf("invalid bone index: %d", boneindex));
			}

			unsigned char * pVertex = (unsigned char *)pVertices + vertexindex * offset;
			unsigned char * pIndices = (unsigned char *)&m_VertexElemSet.GetBlendIndices(pVertex);
			float * pWeights = (float *)&m_VertexElemSet.GetBlendWeights(pVertex);

			int i = 0;
			for(; i < D3DVERTEXELEMENT9Set::MAX_BONE_INDICES; i++)
			{
				if(pWeights[i] == 0)
				{
					pIndices[i] = boneindex;
					pWeights[i] = weight;
					break;
				}
			}

			if(i >= D3DVERTEXELEMENT9Set::MAX_BONE_INDICES)
			{
				THROW_CUSEXCEPTION("too much bone assignment");
			}
		}
	}
	UnlockVertexBuffer();

	VOID * pIndices = LockIndexBuffer();
	DWORD * pAttrBuffer = LockAttributeBuffer();
	int submesh_i = 0;
	node_submesh = node_submeshes->first_node("submesh");
	for(int face_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(material, submesh);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(use32bitindexes, submesh);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(usesharedvertices, submesh);
		DEFINE_XML_ATTRIBUTE_SIMPLE(operationtype, submesh);
		if(!usesharedvertices || 0 != _stricmp(attr_operationtype->value(), "triangle_list"))
		{
			THROW_CUSEXCEPTION("!usesharedvertices || !triangle_list");
		}

		DEFINE_XML_NODE_SIMPLE(faces, submesh);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);

		DEFINE_XML_NODE_SIMPLE(face, faces);
		for(; node_face != NULL && face_i < facecount; node_face = node_face->next_sibling(), face_i++)
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v1, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v2, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v3, face);

			if(dwMeshOptions & D3DXMESH_32BIT)
			{
				*((DWORD *)pIndices + face_i * 3 + 0) = v1;
				*((DWORD *)pIndices + face_i * 3 + 2) = v2;
				*((DWORD *)pIndices + face_i * 3 + 1) = v3;
			}
			else
			{
				*((WORD *)pIndices + face_i * 3 + 0) = v1;
				*((WORD *)pIndices + face_i * 3 + 2) = v2;
				*((WORD *)pIndices + face_i * 3 + 1) = v3;
			}
			pAttrBuffer[face_i] = submesh_i;
		}

		m_MaterialNameList.push_back(attr_material->value());
	}
	UnlockAttributeBuffer();
	UnlockIndexBuffer();

	std::vector<DWORD> rgdwAdjacency(GetNumFaces() * 3);
	GenerateAdjacency(EPSILON_E6, &rgdwAdjacency[0]);
	if(bComputeTangentFrame)
	{
		//DWORD dwOptions = D3DXTANGENT_GENERATE_IN_PLACE;
		//if(!normals)
		//	dwOptions |= D3DXTANGENT_CALCULATE_NORMALS;
		//HRESULT hres = D3DXComputeTangentFrameEx(
		//	m_ptr, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0, D3DX_DEFAULT, 0, D3DDECLUSAGE_NORMAL, 0, dwOptions, &rgdwAdjacency[0], -1.01f, -0.01f, -1.01f, NULL, NULL);
		//if(FAILED(hres))
		//{
		//	THROW_D3DEXCEPTION(hres);
		//}
		ComputeTangentFrame();
	}
	OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, &rgdwAdjacency[0], NULL, NULL, NULL);
}

void OgreMesh::ComputeTangentFrame(void)
{
	std::vector<Vector3> tan1(GetNumVertices(), Vector3::zero);
	std::vector<Vector3> tan2(GetNumVertices(), Vector3::zero);
	WORD VertexStride = m_VertexElemSet.CalculateVertexStride();

	VOID * pIndices = LockIndexBuffer();
	const VOID * pVertices = LockVertexBuffer();

	for(DWORD face_i = 0; face_i < GetNumFaces(); face_i++)
	{
		int i1, i2, i3;
		if(GetOptions() & D3DXMESH_32BIT)
		{
			i1 = *((DWORD *)pIndices + face_i * 3 + 0);
			i2 = *((DWORD *)pIndices + face_i * 3 + 1);
			i3 = *((DWORD *)pIndices + face_i * 3 + 2);
		}
		else
		{
			i1 = *((WORD *)pIndices + face_i * 3 + 0);
			i2 = *((WORD *)pIndices + face_i * 3 + 1);
			i3 = *((WORD *)pIndices + face_i * 3 + 2);
		}

		unsigned char * pv1 = (unsigned char *)pVertices + i1 * VertexStride;
		unsigned char * pv2 = (unsigned char *)pVertices + i2 * VertexStride;
		unsigned char * pv3 = (unsigned char *)pVertices + i3 * VertexStride;

		const Vector3 & v1 = m_VertexElemSet.GetPosition(pv1);
		const Vector3 & v2 = m_VertexElemSet.GetPosition(pv2);
		const Vector3 & v3 = m_VertexElemSet.GetPosition(pv3);

		const Vector2 & w1 = m_VertexElemSet.GetTexcoord(pv1);
		const Vector2 & w2 = m_VertexElemSet.GetTexcoord(pv2);
		const Vector2 & w3 = m_VertexElemSet.GetTexcoord(pv3);

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for(DWORD vertex_i = 0; vertex_i < GetNumVertices(); vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * VertexStride;
		const Vector3 & n = m_VertexElemSet.GetNormal(pVertex);
		const Vector3 & t = tan1[vertex_i];

		// Gram-Schmidt orthogonalize
		m_VertexElemSet.GetTangent(pVertex) = (t - n * n.dot(t)).normalize();
	}

	UnlockVertexBuffer();
	UnlockIndexBuffer();
}
