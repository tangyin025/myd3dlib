
#include "myMesh.h"
#include <boost/property_tree/detail/rapidxml.hpp>
#include <atlbase.h>
#include "myException.h"

namespace my
{
#define DEFINE_XML_NODE(node_v, node_p, node_s) \
	node_v = node_p->first_node(#node_s); \
	if(NULL == node_v) \
		THROW_CUSEXCEPTION(_T("cannot find ") _T(#node_s))

#define DEFINE_XML_NODE_SIMPLE(node_s, parent_s) \
	rapidxml::xml_node<std::string::value_type> * node_##node_s; \
	DEFINE_XML_NODE(node_##node_s, node_##parent_s, node_s)

#define DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s) \
	attr_v = node_p->first_attribute(#attr_s); \
	if(NULL == attr_v) \
		THROW_CUSEXCEPTION(_T("cannot find ") _T(#attr_s))

#define DEFINE_XML_ATTRIBUTE_SIMPLE(attr_s, parent_s) \
	rapidxml::xml_attribute<std::string::value_type> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE(attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUIT_INT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = atoi(attr_v->value())

#define DEFINE_XML_ATTRIBUIT_INT_SIMPLE(attr_s, parent_s) \
	int attr_s; \
	rapidxml::xml_attribute<std::string::value_type> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUIT_INT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUIT_FLOAT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = (float)atof(attr_v->value())

#define DEFINE_XML_ATTRIBUIT_FLOAT_SIMPLE(attr_s, parent_s) \
	float attr_s; \
	rapidxml::xml_attribute<std::string::value_type> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUIT_FLOAT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

	void LoadMeshFromOgreMesh(
		std::string & strOgreMeshXml,
		LPDIRECT3DDEVICE9 pd3dDevice,
		DWORD * pNumSubMeshes,
		LPD3DXMESH * ppMesh,
		DWORD dwMeshOptions /*= D3DXMESH_SYSTEMMEM*/)
	{
		rapidxml::xml_document<std::string::value_type> doc;
		doc.parse<0>(&strOgreMeshXml[0]);
		rapidxml::xml_node<std::string::value_type> * node_mesh = doc.first_node("mesh");

		DEFINE_XML_NODE_SIMPLE(sharedgeometry, mesh);
		DEFINE_XML_ATTRIBUIT_INT_SIMPLE(vertexcount, sharedgeometry);
		DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
		DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
		int facecount = 0;
		for(; node_submesh != NULL; node_submesh = node_submesh->next_sibling())
		{
			DEFINE_XML_NODE_SIMPLE(faces, submesh);
			DEFINE_XML_ATTRIBUIT_INT_SIMPLE(count, faces);
			facecount += count;
		}

		CComPtr<ID3DXMesh> mesh;
		FAILED_THROW_D3DEXCEPTION(D3DXCreateMeshFVF(
			facecount, vertexcount, dwMeshOptions, OgreMeshVertex::FVF, pd3dDevice, &mesh));

		OgreMeshVertex * pVertices;
		FAILED_THROW_D3DEXCEPTION(mesh->LockVertexBuffer(0, (VOID **)&pVertices));

		DEFINE_XML_NODE_SIMPLE(vertexbuffer, sharedgeometry);
		DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
		for(int i = 0; node_vertex != NULL && i < vertexcount; node_vertex = node_vertex->next_sibling(), i++)
		{
			float tmp;
			DEFINE_XML_NODE_SIMPLE(position, vertex);
			rapidxml::xml_attribute<std::string::value_type> * position_x;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, position_x, node_position, x);
			pVertices[i].position.x = -tmp;
			rapidxml::xml_attribute<std::string::value_type> * position_y;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, position_y, node_position, y);
			pVertices[i].position.y = tmp;
			rapidxml::xml_attribute<std::string::value_type> * position_z;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, position_z, node_position, z);
			pVertices[i].position.z = tmp;

			DEFINE_XML_NODE_SIMPLE(normal, vertex);
			rapidxml::xml_attribute<std::string::value_type> * normal_x;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, normal_x, node_normal, x);
			pVertices[i].normal.x = -tmp;
			rapidxml::xml_attribute<std::string::value_type> * normal_y;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, normal_y, node_normal, y);
			pVertices[i].normal.y = tmp;
			rapidxml::xml_attribute<std::string::value_type> * normal_z;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, normal_z, node_normal, z);
			pVertices[i].normal.z = tmp;

			DEFINE_XML_NODE_SIMPLE(texcoord, vertex);
			rapidxml::xml_attribute<std::string::value_type> * texcoord_u;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, texcoord_u, node_texcoord, u);
			pVertices[i].tu = tmp;
			rapidxml::xml_attribute<std::string::value_type> * texcoord_v;
			DEFINE_XML_ATTRIBUIT_FLOAT(tmp, texcoord_v, node_texcoord, v);
			pVertices[i].tv = tmp;
		}

		FAILED_THROW_D3DEXCEPTION(mesh->UnlockVertexBuffer());

		WORD * pIndices;
		FAILED_THROW_D3DEXCEPTION(mesh->LockIndexBuffer(0, (VOID **)&pIndices));

		DWORD * pAttrBuffer;
		FAILED_THROW_D3DEXCEPTION(mesh->LockAttributeBuffer(0, &pAttrBuffer));

		int submesh_i = 0;
		node_submesh = node_submeshes->first_node("submesh");
		for(int face_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			DEFINE_XML_NODE_SIMPLE(faces, submesh);
			DEFINE_XML_ATTRIBUIT_INT_SIMPLE(count, faces);

			DEFINE_XML_NODE_SIMPLE(face, faces);
			for(; node_face != NULL && face_i < facecount; node_face = node_face->next_sibling(), face_i++)
			{
				rapidxml::xml_attribute<std::string::value_type> * attr_v1;
				DEFINE_XML_ATTRIBUIT_INT(pIndices[face_i * 3 + 0], attr_v1, node_face, v1);
				rapidxml::xml_attribute<std::string::value_type> * attr_v2;
				DEFINE_XML_ATTRIBUIT_INT(pIndices[face_i * 3 + 2], attr_v2, node_face, v2);
				rapidxml::xml_attribute<std::string::value_type> * attr_v3;
				DEFINE_XML_ATTRIBUIT_INT(pIndices[face_i * 3 + 1], attr_v3, node_face, v3);

				pAttrBuffer[face_i] = submesh_i;
			}
		}

		FAILED_THROW_D3DEXCEPTION(mesh->UnlockAttributeBuffer());

		FAILED_THROW_D3DEXCEPTION(mesh->UnlockIndexBuffer());

		_ASSERT(NULL != pNumSubMeshes);
		*pNumSubMeshes = submesh_i;

		_ASSERT(NULL != ppMesh);
		*ppMesh = mesh.Detach();
	}
};
