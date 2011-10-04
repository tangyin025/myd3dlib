
#include "stdafx.h"
#include "myMesh.h"
#include <boost/property_tree/detail/rapidxml.hpp>
#include <atlbase.h>
#include <vector>
#include "myException.h"
#include "libc.h"

namespace my
{
#define RETURN_COM_ERROR(hres, info) \
	{ \
		if(NULL != ppErrorMsgs) \
		{ \
			std::basic_string<char> str(info); \
			D3DXCreateBuffer(str.length() + 1, ppErrorMsgs); \
			memcpy((*ppErrorMsgs)->GetBufferPointer(), &str[0], (*ppErrorMsgs)->GetBufferSize()); \
		} \
		return hres; \
	}

#define DEFINE_XML_NODE(node_v, node_p, node_s) \
	node_v = node_p->first_node(#node_s); \
	if(NULL == node_v) \
		RETURN_COM_ERROR(E_FAIL, "cannot find " #node_s)

#define DEFINE_XML_NODE_SIMPLE(node_s, parent_s) \
	rapidxml::xml_node<char> * node_##node_s; \
	DEFINE_XML_NODE(node_##node_s, node_##parent_s, node_s)

#define DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s) \
	attr_v = node_p->first_attribute(#attr_s); \
	if(NULL == attr_v) \
		RETURN_COM_ERROR(E_FAIL, "cannot find " #attr_s)

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

	static WORD CalculateD3DDeclTypeSize(D3DDECLTYPE type)
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
			return 4 * sizeof(float);
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

	static HRESULT FillPositionFromOgreVertexPosition(
		D3DXVECTOR3 * pPosition,
		rapidxml::xml_node<char> * node_position,
		LPD3DXBUFFER * ppErrorMsgs)
	{
		float tmp;
		rapidxml::xml_attribute<char> * position_x;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, position_x, node_position, x);
		pPosition->x = -tmp;
		rapidxml::xml_attribute<char> * position_y;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, position_y, node_position, y);
		pPosition->y = tmp;
		rapidxml::xml_attribute<char> * position_z;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, position_z, node_position, z);
		pPosition->z = tmp;

		return S_OK;
	}

	static HRESULT FillNormalFromOgreVertexNormal(
		D3DXVECTOR3 * pNormal,
		rapidxml::xml_node<char> * node_normal,
		LPD3DXBUFFER * ppErrorMsgs)
	{
		float tmp;
		rapidxml::xml_attribute<char> * normal_x;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, normal_x, node_normal, x);
		pNormal->x = -tmp;
		rapidxml::xml_attribute<char> * normal_y;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, normal_y, node_normal, y);
		pNormal->y = tmp;
		rapidxml::xml_attribute<char> * normal_z;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, normal_z, node_normal, z);
		pNormal->z = tmp;

		return S_OK;
	}

	static HRESULT FillColorFromOgreVertexColorDiffuse(
		D3DXCOLOR * pColor,
		rapidxml::xml_node<char> * node_colour_diffuse,
		LPD3DXBUFFER * ppErrorMsgs)
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(value, colour_diffuse);
		sscanf_s(attr_value->value(), "%f %f %f %f", &pColor->r, &pColor->g, &pColor->b,  &pColor->a);

		return S_OK;
	}

	static HRESULT FillTexCoordFromOgreVertexTexCoord(
		D3DXVECTOR2 * pTexCoord,
		rapidxml::xml_node<char> * node_texcoord,
		LPD3DXBUFFER * ppErrorMsgs)
	{
		float tmp;
		rapidxml::xml_attribute<char> * texcoord_u;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, texcoord_u, node_texcoord, u);
		pTexCoord->x = tmp;
		rapidxml::xml_attribute<char> * texcoord_v;
		DEFINE_XML_ATTRIBUTE_FLOAT(tmp, texcoord_v, node_texcoord, v);
		pTexCoord->y = tmp;

		return S_OK;
	}

	static HRESULT FillVertexDataFromOgreVertex(
		const D3DVERTEXELEMENT9 & d3dvertelem,
		VOID * pVertex,
		rapidxml::xml_node<char> * node_vertex,
		LPD3DXBUFFER * ppErrorMsgs)
	{
		switch(d3dvertelem.Usage)
		{
		case D3DDECLUSAGE_POSITION:
			{
				DEFINE_XML_NODE_SIMPLE(position, vertex);
				return FillPositionFromOgreVertexPosition(
					(D3DXVECTOR3 *)((unsigned char *)pVertex + d3dvertelem.Offset), node_position, ppErrorMsgs);
			}
		case D3DDECLUSAGE_NORMAL:
			{
				DEFINE_XML_NODE_SIMPLE(normal, vertex);
				return FillNormalFromOgreVertexNormal(
					(D3DXVECTOR3 *)((unsigned char *)pVertex + d3dvertelem.Offset), node_normal, ppErrorMsgs);
			}
		case D3DDECLUSAGE_TEXCOORD:
			{
				DEFINE_XML_NODE_SIMPLE(texcoord, vertex);
				for(int texcoord_i = 0;
					NULL != node_texcoord;
					node_texcoord = node_texcoord->next_sibling("texcoord"), texcoord_i++)
				{
					if(d3dvertelem.UsageIndex == texcoord_i)
					{
						return FillTexCoordFromOgreVertexTexCoord(
							(D3DXVECTOR2 *)((unsigned char *)pVertex + d3dvertelem.Offset), node_texcoord, ppErrorMsgs);
					}
				}
				break;
			}
		case D3DDECLUSAGE_COLOR:
			{
				if(0 == d3dvertelem.UsageIndex)
				{
					DEFINE_XML_NODE_SIMPLE(colour_diffuse, vertex);
					return FillColorFromOgreVertexColorDiffuse(
						(D3DXCOLOR *)((unsigned char *)pVertex + d3dvertelem.Offset), node_colour_diffuse, ppErrorMsgs);
				}
				break;
			}
		}

		RETURN_COM_ERROR(E_FAIL, "unknown vertex data type");
	}

	HRESULT LoadMeshFromOgreMesh(
		std::basic_string<char> & strOgreMeshXml,
		LPDIRECT3DDEVICE9 pd3dDevice,
		LPD3DXMESH * ppMesh,
		DWORD * pNumSubMeshes /*= NULL*/,
		DWORD dwMeshOptions /*= D3DXMESH_SYSTEMMEM*/,
		LPD3DXBUFFER * ppErrorMsgs /*= NULL*/) throw()
	{
		rapidxml::xml_document<char> doc;
		try
		{
			doc.parse<0>(&strOgreMeshXml[0]);
		}
		catch(rapidxml::parse_error & e)
		{
			RETURN_COM_ERROR(E_FAIL, e.what());
		}
		rapidxml::xml_node<char> * node_mesh = doc.first_node("mesh");

		DEFINE_XML_NODE_SIMPLE(sharedgeometry, mesh);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexcount, sharedgeometry);
		DEFINE_XML_NODE_SIMPLE(vertexbuffer, sharedgeometry);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(positions, vertexbuffer);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(normals, vertexbuffer);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_diffuse, vertexbuffer);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_specular, vertexbuffer);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(texture_coords, vertexbuffer);

		typedef std::vector<D3DVERTEXELEMENT9> D3DVertexElementList;
		D3DVertexElementList d3dvertelemList;
		WORD Offset = 0;
		if(positions)
		{
			D3DVERTEXELEMENT9 d3dvertelem;
			d3dvertelem.Stream		= 0;
			d3dvertelem.Offset		= Offset;
			d3dvertelem.Type		= D3DDECLTYPE_FLOAT3;
			d3dvertelem.Method		= D3DDECLMETHOD_DEFAULT;
			d3dvertelem.Usage		= D3DDECLUSAGE_POSITION;
			d3dvertelem.UsageIndex	= 0;
			d3dvertelemList.push_back(d3dvertelem);
			Offset += CalculateD3DDeclTypeSize((D3DDECLTYPE)d3dvertelem.Type);
		}

		if(normals)
		{
			D3DVERTEXELEMENT9 d3dvertelem;
			d3dvertelem.Stream		= 0;
			d3dvertelem.Offset		= Offset;
			d3dvertelem.Type		= D3DDECLTYPE_FLOAT3;
			d3dvertelem.Method		= D3DDECLMETHOD_DEFAULT;
			d3dvertelem.Usage		= D3DDECLUSAGE_NORMAL;
			d3dvertelem.UsageIndex	= 0;
			d3dvertelemList.push_back(d3dvertelem);
			Offset += CalculateD3DDeclTypeSize((D3DDECLTYPE)d3dvertelem.Type);
		}

		if(colours_diffuse)
		{
			D3DVERTEXELEMENT9 d3dvertelem;
			d3dvertelem.Stream		= 0;
			d3dvertelem.Offset		= Offset;
			d3dvertelem.Type		= D3DDECLTYPE_D3DCOLOR;
			d3dvertelem.Method		= D3DDECLMETHOD_DEFAULT;
			d3dvertelem.Usage		= D3DDECLUSAGE_COLOR;
			d3dvertelem.UsageIndex	= 0;
			d3dvertelemList.push_back(d3dvertelem);
			Offset += CalculateD3DDeclTypeSize((D3DDECLTYPE)d3dvertelem.Type);
		}

		if(colours_specular)
		{
			D3DVERTEXELEMENT9 d3dvertelem;
			d3dvertelem.Stream		= 0;
			d3dvertelem.Offset		= Offset;
			d3dvertelem.Type		= D3DDECLTYPE_D3DCOLOR;
			d3dvertelem.Method		= D3DDECLMETHOD_DEFAULT;
			d3dvertelem.Usage		= D3DDECLUSAGE_COLOR;
			d3dvertelem.UsageIndex	= 1;
			d3dvertelemList.push_back(d3dvertelem);
			Offset += CalculateD3DDeclTypeSize((D3DDECLTYPE)d3dvertelem.Type);
		}

		for(int i = 0; i < texture_coords; i++)
		{
			D3DVERTEXELEMENT9 d3dvertelem;
			d3dvertelem.Stream		= 0;
			d3dvertelem.Offset		= Offset;
			d3dvertelem.Type		= D3DDECLTYPE_FLOAT2;
			d3dvertelem.Method		= D3DDECLMETHOD_DEFAULT;
			d3dvertelem.Usage		= D3DDECLUSAGE_TEXCOORD;
			d3dvertelem.UsageIndex	= i;
			d3dvertelemList.push_back(d3dvertelem);
			Offset += CalculateD3DDeclTypeSize((D3DDECLTYPE)d3dvertelem.Type);
		}

		if(d3dvertelemList.empty())
		{
			RETURN_COM_ERROR(E_FAIL, "cannot create d3d vertex element list");
		}
		else
		{
			D3DVERTEXELEMENT9 d3dvertelem = D3DDECL_END();
			d3dvertelemList.push_back(d3dvertelem);
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

		CComPtr<ID3DXMesh> mesh;
		HRESULT hres;
		if(FAILED(hres = D3DXCreateMesh(
			facecount, vertexcount, dwMeshOptions, &d3dvertelemList[0], pd3dDevice, &mesh)))
		{
			RETURN_COM_ERROR(hres, "D3DXCreateMesh failed");
		}

		VOID * pVertices;
		if(FAILED(hres = mesh->LockVertexBuffer(0, &pVertices)))
		{
			RETURN_COM_ERROR(hres, "mesh->LockVertexBuffer failed");
		}

		DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
		for(int vertex_i = 0; node_vertex != NULL && vertex_i < vertexcount; node_vertex = node_vertex->next_sibling(), vertex_i++)
		{
			for(size_t elem_i = 0; elem_i < d3dvertelemList.size() - 1; elem_i++)
			{
				if(FAILED(hres = FillVertexDataFromOgreVertex(
					d3dvertelemList[elem_i],
					(unsigned char *)pVertices + vertex_i * Offset,
					node_vertex,
					ppErrorMsgs)))
				{
					return hres;
				}
			}
		}

		if(FAILED(hres = mesh->UnlockVertexBuffer()))
		{
			RETURN_COM_ERROR(hres, "mesh->UnlockVertexBuffer failed");
		}

		WORD * pIndices;
		if(FAILED(hres = mesh->LockIndexBuffer(0, (VOID **)&pIndices)))
		{
			RETURN_COM_ERROR(hres, "mesh->LockIndexBuffer failed");
		}

		DWORD * pAttrBuffer;
		if(FAILED(mesh->LockAttributeBuffer(0, &pAttrBuffer)))
		{
			RETURN_COM_ERROR(hres, "mesh->LockAttributeBuffer failed");
		}

		int submesh_i = 0;
		node_submesh = node_submeshes->first_node("submesh");
		for(int face_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			DEFINE_XML_NODE_SIMPLE(faces, submesh);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);

			DEFINE_XML_NODE_SIMPLE(face, faces);
			for(; node_face != NULL && face_i < facecount; node_face = node_face->next_sibling(), face_i++)
			{
				rapidxml::xml_attribute<char> * attr_v1;
				DEFINE_XML_ATTRIBUTE_INT(pIndices[face_i * 3 + 0], attr_v1, node_face, v1);
				rapidxml::xml_attribute<char> * attr_v2;
				DEFINE_XML_ATTRIBUTE_INT(pIndices[face_i * 3 + 2], attr_v2, node_face, v2);
				rapidxml::xml_attribute<char> * attr_v3;
				DEFINE_XML_ATTRIBUTE_INT(pIndices[face_i * 3 + 1], attr_v3, node_face, v3);

				pAttrBuffer[face_i] = submesh_i;
			}
		}

		if(FAILED(hres = mesh->UnlockAttributeBuffer()))
		{
			RETURN_COM_ERROR(hres, "mesh->UnlockAttributeBuffer failed");
		}

		if(FAILED(hres = mesh->UnlockIndexBuffer()))
		{
			RETURN_COM_ERROR(hres, "mesh->UnlockIndexBuffer failed");
		}

		if(NULL != pNumSubMeshes)
			*pNumSubMeshes = submesh_i;

		_ASSERT(NULL != ppMesh);
		*ppMesh = mesh.Detach();
		return S_OK;
	}

	MeshPtr Mesh::CreateMeshFromOgreMesh(
		LPDIRECT3DDEVICE9 pd3dDevice,
		LPCSTR pSrcData,
		UINT srcDataLen,
		DWORD dwMeshOptions /*= D3DXMESH_SYSTEMMEM*/)
	{
		std::string str(pSrcData, srcDataLen);
		LPD3DXMESH pMesh = NULL;
		DWORD NumSubMeshes = 0;
		CComPtr<ID3DXBuffer> ErrorMsgs;
		HRESULT hres = LoadMeshFromOgreMesh(str, pd3dDevice, &pMesh, &NumSubMeshes, dwMeshOptions, &ErrorMsgs);
		if(FAILED(hres))
		{
			std::basic_string<char> info((char *)ErrorMsgs->GetBufferPointer(), ErrorMsgs->GetBufferSize());
			THROW_CUSEXCEPTION(mstringToTString(info));
		}

		return MeshPtr(new Mesh(pMesh, NumSubMeshes));
	}
};
