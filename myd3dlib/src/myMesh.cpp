
#include "stdafx.h"
#include "myd3dlib.h"

namespace my
{
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

		VertexElementList elems;
		elems.push_back(VertexElement::Position(0, 0));
		int offset = CalculateD3DDeclTypeSize(elems.back().Type);

		if(normals)
		{
			elems.push_back(VertexElement::Normal(0, offset));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		if(colours_diffuse)
		{
			elems.push_back(VertexElement::Color(0, offset, 0));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		if(colours_specular)
		{
			elems.push_back(VertexElement::Color(0, offset, 1));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		if(texture_coords > MAXBYTE)
		{
			THROW_CUSEXCEPTION("texture coords overflow ( > 255 )");
		}

		for(int i = 0; i < texture_coords; i++)
		{
			elems.push_back(VertexElement::Texcoord(0, offset, i));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		rapidxml::xml_node<char> * node_boneassignments = node_mesh->first_node("boneassignments");
		WORD indicesOffset = 0, weightsOffset = 0;
		if(node_boneassignments != NULL)
		{
			indicesOffset = offset;
			elems.push_back(VertexElement::BlendIndices(0, offset, 0));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);

			weightsOffset = offset;
			elems.push_back(VertexElement::BlendWeights(0, offset, 0));
			offset += CalculateD3DDeclTypeSize(elems.back().Type);
		}

		elems.push_back(VertexElement::End());

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
				}
			}
		}

		if(node_boneassignments != NULL)
		{
			for(int vertex_i = 0; vertex_i < vertexcount; vertex_i++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * offset;
				unsigned char * pIndices = (unsigned char *)(pVertex + indicesOffset);
				float * pWeights = (float *)(pVertex + weightsOffset);

				memset(pIndices, 0, sizeof(*pIndices) * MAX_BONE_INDICES);
				memset(pWeights, 0, sizeof(*pWeights) * MAX_BONE_INDICES);
			}

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
