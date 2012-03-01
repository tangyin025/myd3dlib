
#include "stdafx.h"
#include "myd3dlib.h"
#include "rapidxml.hpp"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

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

UINT D3DVERTEXELEMENT9Set::GetVertexStride(DWORD Stream) const
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
//
//VertexBuffer::VertexBuffer(LPDIRECT3DDEVICE9 pDevice, const D3DVERTEXELEMENT9Set & VertexElemSet, WORD Stream /*= 0*/)
//	: m_Device(pDevice)
//	, m_VertexElemSet(VertexElemSet)
//	, m_vertexStride(VertexElemSet.GetVertexStride(Stream))
//	, m_NumVertices(0)
//	, m_Stream(Stream)
//{
//	if(0 == m_vertexStride)
//	{
//		THROW_CUSEXCEPTION("Invalid Vertex Stride (m_vertexStride == 0)");
//	}
//}
//
//VertexBufferPtr VertexBuffer::CreateVertexBuffer(
//	LPDIRECT3DDEVICE9 pD3DDevice,
//	const D3DVERTEXELEMENT9Set & VertexElemSet,
//	WORD Stream /*= 0*/)
//{
//	return VertexBufferPtr(new VertexBuffer(pD3DDevice, VertexElemSet, Stream));
//}
//
//void VertexBuffer::OnResetDevice(void)
//{
//	_ASSERT(!m_VertexBuffer);
//	_ASSERT(!m_MemVertexBuffer.empty());
//
//	HRESULT hres = m_Device->CreateVertexBuffer(m_MemVertexBuffer.size(), 0, 0, D3DPOOL_DEFAULT, &m_VertexBuffer, NULL);
//	if(FAILED(hres))
//	{
//		THROW_D3DEXCEPTION(hres);
//	}
//
//	UpdateVertexBuffer();
//}
//
//void VertexBuffer::OnLostDevice(void)
//{
//	m_VertexBuffer.Release();
//}
//
//void VertexBuffer::OnDestroyDevice(void)
//{
//	_ASSERT(!m_VertexBuffer);
//
//	m_Device.Release();
//}
//
//void VertexBuffer::UpdateVertexBuffer(void)
//{
//	_ASSERT(m_VertexBuffer);
//
//	void * pVertices;
//	UINT LockSize = m_MemVertexBuffer.size();
//	if(SUCCEEDED(m_VertexBuffer->Lock(0, LockSize, &pVertices, 0)))
//	{
//		memcpy(pVertices, &m_MemVertexBuffer[0], LockSize);
//		m_VertexBuffer->Unlock();
//	}
//}
//
//void VertexBuffer::ResizeVertexBufferLength(UINT NumVertices)
//{
//	m_MemVertexBuffer.resize(NumVertices * m_vertexStride, 0);
//
//	m_NumVertices = NumVertices;
//}
//
//void VertexBuffer::SetPosition(int Index, const D3DVERTEXELEMENT9Set::PositionType & Position, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetPosition(&m_MemVertexBuffer[Index * m_vertexStride], Position, m_Stream, UsageIndex);
//}
//
//void VertexBuffer::SetBinormal(int Index, const D3DVERTEXELEMENT9Set::BinormalType & Binormal, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetBinormal(&m_MemVertexBuffer[Index * m_vertexStride], Binormal, m_Stream, UsageIndex);
//}
//
//void VertexBuffer::SetTangent(int Index, const D3DVERTEXELEMENT9Set::TangentType & Tangent, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetTangent(&m_MemVertexBuffer[Index * m_vertexStride], Tangent, m_Stream, UsageIndex);
//}
//
//void VertexBuffer::SetNormal(int Index, const D3DVERTEXELEMENT9Set::NormalType & Normal, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetNormal(&m_MemVertexBuffer[Index * m_vertexStride], Normal, m_Stream, UsageIndex);
//}
//
//void VertexBuffer::SetTexcoord(int Index, const D3DVERTEXELEMENT9Set::TexcoordType & Texcoord, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetTexcoord(&m_MemVertexBuffer[Index * m_vertexStride], Texcoord, m_Stream, UsageIndex);
//}
//
//void VertexBuffer::SetBlendIndices(int Index, const D3DVERTEXELEMENT9Set::BlendIndicesType & BlendIndices, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetBlendIndices(&m_MemVertexBuffer[Index * m_vertexStride], BlendIndices, m_Stream, UsageIndex);
//}
//
//void VertexBuffer::SetBlendWeights(int Index, const D3DVERTEXELEMENT9Set::BlendWeightsType & BlendWeights, BYTE UsageIndex /*= 0*/)
//{
//	m_VertexElemSet.SetBlendWeights(&m_MemVertexBuffer[Index * m_vertexStride], BlendWeights, m_Stream, UsageIndex);
//}
//
//IndexBuffer::IndexBuffer(LPDIRECT3DDEVICE9 pDevice)
//	: m_Device(pDevice)
//{
//}
//
//IndexBufferPtr IndexBuffer::CreateIndexBuffer(LPDIRECT3DDEVICE9 pD3DDevice)
//{
//	return IndexBufferPtr(new IndexBuffer(pD3DDevice));
//}
//
//void IndexBuffer::OnResetDevice(void)
//{
//	_ASSERT(!m_IndexBuffer);
//
//	HRESULT hres = m_Device->CreateIndexBuffer(m_MemIndexBuffer.size() * sizeof(UIntList::value_type), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_IndexBuffer, NULL);
//	if(FAILED(hres))
//	{
//		THROW_D3DEXCEPTION(hres);
//	}
//
//	UpdateIndexBuffer();
//}
//
//void IndexBuffer::OnLostDevice(void)
//{
//	m_IndexBuffer.Release();
//}
//
//void IndexBuffer::OnDestroyDevice(void)
//{
//	_ASSERT(!m_IndexBuffer);
//	m_IndexBuffer.Release();
//	m_Device.Release();
//}
//
//void IndexBuffer::UpdateIndexBuffer(void)
//{
//	_ASSERT(m_IndexBuffer);
//
//	void * pIndices;
//	UINT LockSize = m_MemIndexBuffer.size() * sizeof(UIntList::value_type);
//	if(SUCCEEDED(m_IndexBuffer->Lock(0, LockSize, &pIndices, 0)))
//	{
//		memcpy(pIndices, &m_MemIndexBuffer[0], LockSize);
//		m_IndexBuffer->Unlock();
//	}
//}
//
//void IndexBuffer::ResizeIndexBufferLength(UINT NumIndices)
//{
//	m_MemIndexBuffer.resize(NumIndices, 0);
//}
//
//void IndexBuffer::SetIndex(int Index, unsigned int IndexValue)
//{
//	_ASSERT(Index < (int)m_MemIndexBuffer.size());
//
//	m_MemIndexBuffer[Index] = IndexValue;
//}

MeshPtr Mesh::CreateMesh(
	LPDIRECT3DDEVICE9 pD3DDevice,
	DWORD NumFaces,
	DWORD NumVertices,
	CONST LPD3DVERTEXELEMENT9 pDeclaration,
	DWORD Options /*= D3DXMESH_MANAGED*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMesh(NumFaces, NumVertices, Options, pDeclaration, pD3DDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateMeshFVF(
	LPDIRECT3DDEVICE9 pD3DDevice,
	DWORD NumFaces,
	DWORD NumVertices,
	DWORD FVF,
	DWORD Options /*= D3DXMESH_MANAGED*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMeshFVF(NumFaces, NumVertices, Options, FVF, pD3DDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateMeshFromX(
	LPDIRECT3DDEVICE9 pD3DDevice,
	LPCSTR pFilename,
	DWORD Options /*= D3DXMESH_MANAGED*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/,
	LPD3DXBUFFER * ppMaterials /*= NULL*/,
	LPD3DXBUFFER * ppEffectInstances /*= NULL*/,
	DWORD * pNumMaterials /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXLoadMeshFromXA(
		pFilename, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateMeshFromXInMemory(
	LPDIRECT3DDEVICE9 pD3DDevice,
	LPCVOID Memory,
	DWORD SizeOfMemory,
	DWORD Options /*= D3DXMESH_MANAGED*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/,
	LPD3DXBUFFER * ppMaterials /*= NULL*/,
	LPD3DXBUFFER * ppEffectInstances /*= NULL*/,
	DWORD * pNumMaterials /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXLoadMeshFromXInMemory(
		Memory, SizeOfMemory, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateMeshFromOgreXml(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPCSTR pFilename,
	bool bComputeTangentFrame /*= true*/,
	DWORD dwMeshOptions /*= D3DXMESH_MANAGED*/)
{
	FILE * fp;
	if(0 != fopen_s(&fp, pFilename, "rb"))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", pFilename));
	}

	CachePtr cache = ArchiveStreamPtr(new FileArchiveStream(fp))->GetWholeCache();

	return CreateMeshFromOgreXmlInMemory(pd3dDevice, (LPCSTR)&(*cache)[0], cache->size(), bComputeTangentFrame, dwMeshOptions);
}

MeshPtr Mesh::CreateMeshFromOgreXmlInMemory(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPCSTR pSrcData,
	UINT srcDataLen,
	bool bComputeTangentFrame /*= true*/,
	DWORD dwMeshOptions /*= D3DXMESH_MANAGED*/)
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

	D3DVERTEXELEMENT9Set elems;
	elems.insert(D3DVERTEXELEMENT9Set::CreatePositionElement(0, 0, 0));
	WORD offset = sizeof(D3DVERTEXELEMENT9Set::PositionType);

	if(normals || bComputeTangentFrame)
	{
		elems.insert(D3DVERTEXELEMENT9Set::CreateNormalElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::NormalType);
	}

	if(bComputeTangentFrame)
	{
		elems.insert(D3DVERTEXELEMENT9Set::CreateBinormalElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::BinormalType);

		elems.insert(D3DVERTEXELEMENT9Set::CreateTangentElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::TangentType);
	}

	if(texture_coords > MAXBYTE)
	{
		THROW_CUSEXCEPTION("texture coords overflow ( > 255 )");
	}

	for(int i = 0; i < texture_coords; i++)
	{
		elems.insert(D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, i));
		offset += sizeof(D3DVERTEXELEMENT9Set::TexcoordType);
	}

	rapidxml::xml_node<char> * node_boneassignments = node_mesh->first_node("boneassignments");
	WORD indicesOffset = 0, weightsOffset = 0;
	if(node_boneassignments != NULL)
	{
		elems.insert(D3DVERTEXELEMENT9Set::CreateBlendIndicesElement(0, offset, 0));
		offset += sizeof(D3DVERTEXELEMENT9Set::BlendIndicesType);

		elems.insert(D3DVERTEXELEMENT9Set::CreateBlendWeightsElement(0, offset, 0));
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

	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMesh(
		facecount, vertexcount, dwMeshOptions, (D3DVERTEXELEMENT9 *)&elems.BuildVertexElementList()[0], pd3dDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	MeshPtr mesh(new Mesh(pMesh));

	const VOID * pVertices = mesh->LockVertexBuffer();
	DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
	for(int vertex_i = 0; node_vertex != NULL && vertex_i < vertexcount; node_vertex = node_vertex->next_sibling(), vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * offset;
		if(positions)
		{
			DEFINE_XML_NODE_SIMPLE(position, vertex);
			D3DVERTEXELEMENT9Set::PositionType & Position = elems.GetPosition(pVertex);
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
			D3DVERTEXELEMENT9Set::NormalType & Normal = elems.GetNormal(pVertex);
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
			D3DVERTEXELEMENT9Set::TexcoordType & Texcoord = elems.GetTexcoord(pVertex, i);
			float tmp;
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_texcoord, u);
			Texcoord.x = tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_texcoord, v);
			Texcoord.y = tmp;
		}

		if(node_boneassignments != NULL)
		{
			elems.SetBlendIndices(pVertex, D3DCOLOR_ARGB(0, 0, 0, 0));
			elems.SetBlendWeights(pVertex, Vector4::zero);
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
			unsigned char * pIndices = (unsigned char *)&elems.GetBlendIndices(pVertex);
			float * pWeights = (float *)&elems.GetBlendWeights(pVertex);

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
	mesh->UnlockVertexBuffer();

	VOID * pIndices = mesh->LockIndexBuffer();
	DWORD * pAttrBuffer = mesh->LockAttributeBuffer();
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
	}
	mesh->UnlockAttributeBuffer();
	mesh->UnlockIndexBuffer();

	std::vector<DWORD> rgdwAdjacency(mesh->GetNumFaces() * 3);
	mesh->GenerateAdjacency(1e-6f, &rgdwAdjacency[0]);
	if(bComputeTangentFrame)
	{
		DWORD dwOptions = D3DXTANGENT_GENERATE_IN_PLACE | (normals ? 0 : D3DXTANGENT_CALCULATE_NORMALS);
		HRESULT hres = D3DXComputeTangentFrameEx(
			mesh->m_ptr, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0, D3DDECLUSAGE_BINORMAL, 0, D3DDECLUSAGE_NORMAL, 0, dwOptions, &rgdwAdjacency[0], -1.01f, -0.01f, -1.01f, NULL, NULL);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}
	}
	mesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, &rgdwAdjacency[0], NULL, NULL, NULL);

	return mesh;
}

MeshPtr Mesh::CreateBox(
	LPDIRECT3DDEVICE9 pd3dDevice,
	FLOAT Width /*= 1.0f*/,
	FLOAT Height /*= 1.0f*/,
	FLOAT Depth /*= 1.0f*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateBox(pd3dDevice, Width, Height, Depth, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateCylinder(
	LPDIRECT3DDEVICE9 pd3dDevice,
	FLOAT Radius1 /*= 1.0f*/,
	FLOAT Radius2 /*= 1.0f*/,
	FLOAT Length /*= 2.0f*/,
	UINT Slices /*= 20*/,
	UINT Stacks /*= 1*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateCylinder(pd3dDevice, Radius1, Radius2, Length, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreatePolygon(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT Length /*= 1.0f*/,
	UINT Sides /*= 5*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreatePolygon(pDevice, Length, Sides, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateSphere(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT Radius /*= 1.0f*/,
	UINT Slices /*= 20*/,
	UINT Stacks /*= 20*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateSphere(pDevice, Radius, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateTeapot(
	LPDIRECT3DDEVICE9 pDevice,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateTeapot(pDevice, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}

MeshPtr Mesh::CreateTorus(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT InnerRadius /*= 0.5f*/,
	FLOAT OuterRadius /*= 1.5f*/,
	UINT Sides /*= 20*/,
	UINT Rings /*= 20*/,
	LPD3DXBUFFER * ppAdjacency /*= NULL*/)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateTorus(pDevice, InnerRadius, OuterRadius, Sides, Rings, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return MeshPtr(new Mesh(pMesh));
}
