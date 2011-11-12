
#include "stdafx.h"
#include "myd3dlib.h"

namespace my
{
	VertexBuffer::VertexBuffer(LPDIRECT3DDEVICE9 pDevice, const VertexElementList & vertexElemList)
		: m_Device(pDevice)
		, m_NumVertices(0)
	{
		WORD Offset = 0;
		std::vector<D3DVERTEXELEMENT9> VEList;
		VertexElementList::const_iterator vert_elem_iter = vertexElemList.begin();
		for(; vert_elem_iter != vertexElemList.end(); vert_elem_iter++)
		{
			D3DVERTEXELEMENT9 elem = vert_elem_iter->BuildD3DVertexElement(Offset);
			_ASSERT(m_VertexElemSet.end() == m_VertexElemSet.find(elem));

			m_VertexElemSet.insert(elem);
			VEList.push_back(elem);
			Offset += vert_elem_iter->GetElementSize();
		}

		D3DVERTEXELEMENT9 elem = {0xFF,0,D3DDECLTYPE_UNUSED,0,0,0};
		VEList.push_back(elem);
		HRESULT hres = m_Device->CreateVertexDeclaration(&VEList[0], &m_VertexDecl);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		m_vertexStride = Offset;
	}

	void VertexBuffer::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		_ASSERT(!m_VertexBuffer);
		_ASSERT(!m_MemVertexBuffer.empty());

		HRESULT hres = m_Device->CreateVertexBuffer(m_MemVertexBuffer.size(), 0, 0, D3DPOOL_DEFAULT, &m_VertexBuffer, NULL);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		UpdateVertexBuffer();
	}

	void VertexBuffer::OnD3D9LostDevice(void)
	{
		m_VertexBuffer.Release();
	}

	void VertexBuffer::OnD3D9DestroyDevice(void)
	{
		_ASSERT(!m_VertexBuffer);

		m_VertexDecl.Release();
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

	void VertexBuffer::SetPosition(int Index, const VertexElement::PositionType & Position, BYTE UsageIndex /*= 0*/)
	{
		D3DVERTEXELEMENT9 elem_finder = VertexElement::CreatePositionElement(0, UsageIndex).BuildD3DVertexElement(0);
		D3DVERTEXELEMENT9Set::const_iterator vert_elem_iter = m_VertexElemSet.find(elem_finder);
		_ASSERT(m_VertexElemSet.end() != vert_elem_iter);

		unsigned char * pVertex = &m_MemVertexBuffer[Index * m_vertexStride];
		*(VertexElement::PositionType *)(pVertex + vert_elem_iter->Offset) = Position;
	}

	void VertexBuffer::SetNormal(int Index, const VertexElement::NormalType & Normal, BYTE UsageIndex /*= 0*/)
	{
		D3DVERTEXELEMENT9 elem_finder = VertexElement::CreateNormalElement(0, UsageIndex).BuildD3DVertexElement(0);
		D3DVERTEXELEMENT9Set::const_iterator vert_elem_iter = m_VertexElemSet.find(elem_finder);
		_ASSERT(m_VertexElemSet.end() != vert_elem_iter);

		unsigned char * pVertex = &m_MemVertexBuffer[Index * m_vertexStride];
		*(VertexElement::NormalType *)(pVertex + vert_elem_iter->Offset) = Normal;
	}

	void VertexBuffer::SetTexcoord(int Index, const VertexElement::TexcoordType & Texcoord, BYTE UsageIndex /*= 0*/)
	{
		D3DVERTEXELEMENT9 elem_finder = VertexElement::CreateTexcoordElement(0, UsageIndex).BuildD3DVertexElement(0);
		D3DVERTEXELEMENT9Set::const_iterator vert_elem_iter = m_VertexElemSet.find(elem_finder);
		_ASSERT(m_VertexElemSet.end() != vert_elem_iter);

		unsigned char * pVertex = &m_MemVertexBuffer[Index * m_vertexStride];
		*(VertexElement::TexcoordType *)(pVertex + vert_elem_iter->Offset) = Texcoord;
	}

	void VertexBuffer::SetBlendIndices(int Index, int SubIndex, unsigned char BlendIndex, BYTE UsageIndex /*= 0*/)
	{
		_ASSERT(SubIndex < 4);

		D3DVERTEXELEMENT9 elem_finder = VertexElement::CreateIndicesElement(0, UsageIndex).BuildD3DVertexElement(0);
		D3DVERTEXELEMENT9Set::const_iterator vert_elem_iter = m_VertexElemSet.find(elem_finder);
		_ASSERT(m_VertexElemSet.end() != vert_elem_iter);

		unsigned char * pVertex = &m_MemVertexBuffer[Index * m_vertexStride];
		((unsigned char *)(pVertex + vert_elem_iter->Offset))[SubIndex] = BlendIndex;
	}

	void VertexBuffer::SetBlendWeights(int Index, int SubIndex, float BlendWeight, BYTE UsageIndex /*= 0*/)
	{
		_ASSERT(SubIndex < 4);

		D3DVERTEXELEMENT9 elem_finder = VertexElement::CreateWeightsElement(0, UsageIndex).BuildD3DVertexElement(0);
		D3DVERTEXELEMENT9Set::const_iterator vert_elem_iter = m_VertexElemSet.find(elem_finder);
		_ASSERT(m_VertexElemSet.end() != vert_elem_iter);

		unsigned char * pVertex = &m_MemVertexBuffer[Index * m_vertexStride];
		((float *)(pVertex + vert_elem_iter->Offset))[SubIndex] = BlendWeight;
	}

	IndexBuffer::IndexBuffer(LPDIRECT3DDEVICE9 pDevice)
		: m_Device(pDevice)
	{
	}

	void IndexBuffer::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		_ASSERT(!m_IndexBuffer);

		HRESULT hres = m_Device->CreateIndexBuffer(m_MemIndexBuffer.size() * sizeof(UIntList::value_type), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_IndexBuffer, NULL);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		UpdateIndexBuffer();
	}

	void IndexBuffer::OnD3D9LostDevice(void)
	{
		m_IndexBuffer.Release();
	}

	void IndexBuffer::OnD3D9DestroyDevice(void)
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

	void IndexBuffer::SetIndex(int Index, unsigned int IndexValue)
	{
		_ASSERT(Index < (int)m_MemIndexBuffer.size());

		m_MemIndexBuffer[Index] = IndexValue;
	}

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

	WORD OgreMesh::CalculateD3DDeclTypeSize(int type)
	{
		switch(type)
		{
		case D3DDECLTYPE_FLOAT1:
			return 1 * sizeof(float);
		case D3DDECLTYPE_FLOAT2:
			return 2 * sizeof(float);
		case D3DDECLTYPE_FLOAT3:
			return 3 * sizeof(float);
		case D3DDECLTYPE_FLOAT4:
			return 4 * sizeof(float);
		case D3DDECLTYPE_D3DCOLOR:
			return sizeof(D3DCOLOR); // RGBA ?
		case D3DDECLTYPE_UBYTE4:
			return 4 * sizeof(unsigned char);
		case D3DDECLTYPE_SHORT2:
			return 2 * sizeof(short);
		case D3DDECLTYPE_SHORT4:
			return 4 * sizeof(short);
		case D3DDECLTYPE_UBYTE4N:
			return 4 * sizeof(unsigned char);
		case D3DDECLTYPE_SHORT2N:
			return 2 * sizeof(short);
		case D3DDECLTYPE_SHORT4N:
			return 4 * sizeof(short);
		case D3DDECLTYPE_USHORT2N:
			return 2 * sizeof(unsigned short);
		case D3DDECLTYPE_USHORT4N:
			return 4 * sizeof(unsigned short);
		case D3DDECLTYPE_UDEC3:
			return 3 * sizeof(unsigned short);
		case D3DDECLTYPE_DEC3N:
			return 3 * sizeof(short);
		case D3DDECLTYPE_FLOAT16_2:
			return sizeof(float);
		case D3DDECLTYPE_FLOAT16_4:
			return 2 * sizeof(float);
			//case D3DDECLTYPE_UNUSED:
		}

		return 0;
	}

	OgreMeshPtr OgreMesh::CreateOgreMesh(
		LPDIRECT3DDEVICE9 pd3dDevice,
		LPCSTR pSrcData,
		UINT srcDataLen,
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

		if(!positions)
		{
			THROW_CUSEXCEPTION("cannot process non-position vertex");
		}

		OgreVertexElementList elems;
		elems.push_back(OgreVertexElement::Position(0, 0));
		int offset = CalculateD3DDeclTypeSize(elems.back().Type);

		if(normals)
		{
			elems.push_back(OgreVertexElement::Normal(0, offset));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		if(colours_diffuse)
		{
			elems.push_back(OgreVertexElement::Color(0, offset, 0));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		if(colours_specular)
		{
			elems.push_back(OgreVertexElement::Color(0, offset, 1));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		if(texture_coords > MAXBYTE)
		{
			THROW_CUSEXCEPTION("texture coords overflow ( > 255 )");
		}

		for(int i = 0; i < texture_coords; i++)
		{
			elems.push_back(OgreVertexElement::Texcoord(0, offset, i));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		rapidxml::xml_node<char> * node_boneassignments = node_mesh->first_node("boneassignments");
		WORD indicesOffset = 0, weightsOffset = 0;
		if(node_boneassignments != NULL)
		{
			indicesOffset = offset;
			elems.push_back(OgreVertexElement::BlendIndices(0, offset, 0));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);

			weightsOffset = offset;
			elems.push_back(OgreVertexElement::BlendWeights(0, offset, 0));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		elems.push_back(OgreVertexElement::End());

		DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
		DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
		int facecount = 0;
		for(; node_submesh != NULL; node_submesh = node_submesh->next_sibling())
		{
			DEFINE_XML_NODE_SIMPLE(faces, submesh);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
			facecount += count;
		}

		if(dwMeshOptions & ~D3DXMESH_32BIT && facecount >= 65535)
		{
			THROW_CUSEXCEPTION("facecount overflow ( >= 65535 )");
		}

		LPD3DXMESH pMesh = NULL;
		HRESULT hres = D3DXCreateMesh(
			facecount, vertexcount, dwMeshOptions, (D3DVERTEXELEMENT9 *)&elems[0], pd3dDevice, &pMesh);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		OgreMeshPtr mesh(new OgreMesh(pMesh));

		const VOID * pVertices = mesh->LockVertexBuffer();
		DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
		for(int vertex_i = 0; node_vertex != NULL && vertex_i < vertexcount; node_vertex = node_vertex->next_sibling(), vertex_i++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * offset;
			for(size_t elem_i = 0; elem_i < elems.size() - 1; elem_i++)
			{
				switch(elems[elem_i].Usage)
				{
				case D3DDECLUSAGE_POSITION:
					{
						DEFINE_XML_NODE_SIMPLE(position, vertex);
						Vector3 * pPosition = (Vector3 *)(pVertex + elems[elem_i].Offset);
						float tmp;
						rapidxml::xml_attribute<char> * attr_tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_position, x);
						pPosition->x = tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_position, y);
						pPosition->y = tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_position, z);
						pPosition->z = -tmp;
					}
					break;

				case D3DDECLUSAGE_NORMAL:
					{
						DEFINE_XML_NODE_SIMPLE(normal, vertex);
						Vector3 * pNormal = (Vector3 *)(pVertex + elems[elem_i].Offset);
						float tmp;
						rapidxml::xml_attribute<char> * attr_tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_normal, x);
						pNormal->x = tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_normal, y);
						pNormal->y = tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_normal, z);
						pNormal->z = -tmp;
					}
					break;

				case D3DDECLUSAGE_COLOR:
					{
						if(0 == elems[elem_i].UsageIndex)
						{
							DEFINE_XML_NODE_SIMPLE(colour_diffuse, vertex);
							DEFINE_XML_ATTRIBUTE_SIMPLE(value, colour_diffuse);
							float r, g, b, a;
							sscanf_s(attr_value->value(), "%f %f %f %f", &r, &g, &b, &a);
							DWORD * pColor = (DWORD *)(pVertex + elems[elem_i].Offset);
							*pColor = D3DCOLOR_RGBA((int)(255 * r), (int)(255 * g), (int)(255 * b), (int)(255 * a));
						}
						else if(1 == elems[elem_i].UsageIndex)
						{
						}
					}
					break;

				case D3DDECLUSAGE_TEXCOORD:
					{
						DEFINE_XML_NODE_SIMPLE(texcoord, vertex);
						Vector2 * pTexcoord = (Vector2 *)(pVertex + elems[elem_i].Offset);
						float tmp;
						rapidxml::xml_attribute<char> * attr_tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_texcoord, u);
						pTexcoord->x = tmp;
						DEFINE_XML_ATTRIBUTE_FLOAT(tmp, attr_tmp, node_texcoord, v);
						pTexcoord->y = tmp;
					}
					break;

				case D3DDECLUSAGE_BLENDINDICES:
					{
						unsigned char * pIndices = (unsigned char *)(pVertex + elems[elem_i].Offset);
						pIndices[0] = 0;
						pIndices[1] = 0;
						pIndices[2] = 0;
						pIndices[3] = 0;
					}
					break;

				case D3DDECLUSAGE_BLENDWEIGHT:
					{
						float * pWeights = (float *)(pVertex + elems[elem_i].Offset);
						pWeights[0] = 0;
						pWeights[1] = 0;
						pWeights[2] = 0;
						pWeights[3] = 0;
					}
					break;
				}
			}
		}

		if(node_boneassignments != NULL)
		{
			DEFINE_XML_NODE_SIMPLE(vertexboneassignment, boneassignments);
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
				unsigned char * pIndices = (unsigned char *)(pVertex + indicesOffset);
				float * pWeights = (float *)(pVertex + weightsOffset);

				int i = 0;
				for(; i < MAX_BONE_INDICES; i++)
				{
					if(pWeights[i] == 0)
					{
						pIndices[i] = boneindex;
						pWeights[i] = weight;
						break;
					}
				}

				if(i >= MAX_BONE_INDICES)
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

		return mesh;
	}

	OgreMeshPtr OgreMesh::CreateOgreMeshFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		LPCTSTR pFilename,
		DWORD dwMeshOptions /*= D3DXMESH_MANAGED*/)
	{
		FILE * fp;
		if(0 != _tfopen_s(&fp, pFilename, _T("rb")))
		{
			THROW_CUSEXCEPTION(tstringToMString(str_printf(_T("cannot open file archive: %s"), pFilename)));
		}

		ArchiveStreamPtr stream(new FileArchiveStream(fp));

		CachePtr cache = ReadWholeCacheFromStream(stream);

		return CreateOgreMesh(pDevice, (LPCSTR)&(*cache)[0], cache->size(), dwMeshOptions);
	}
};
