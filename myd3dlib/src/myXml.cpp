// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myXml.h"
#include "myMesh.h"
#include "myDxutApp.h"
#include "libc.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>

using namespace my;

void OgreMeshHandler::on_start_element(const std::string& name, const my::Xml<char>::attr_list& attrs)
{
    if (name == "mesh")
    {
        my::Xml<char>::attr_list::const_iterator totalfaces_iter = attrs.find("totalfaces");
        if (totalfaces_iter != attrs.end())
        {
            total_faces = atoi(totalfaces_iter->second.c_str());
        }
        else
        {
            THROW_CUSEXCEPTION("invalid totalfaces");
        }

        my::Xml<char>::attr_list::const_iterator hasboneassignments_iter = attrs.find("hasboneassignments");
        if (hasboneassignments_iter != attrs.end() && boost::iequals(hasboneassignments_iter->second, "true"))
        {
            has_boneassignments = true;
        }
        else
        {
            has_boneassignments = false;
        }

        if (!(dwMeshOptions & D3DXMESH_32BIT) && (total_vertices >= USHRT_MAX || total_faces >= USHRT_MAX))
        {
            D3DContext::getSingleton().m_EventLog("facecount overflow ( >= 65535 )");
            dwMeshOptions |= D3DXMESH_32BIT;
        }
    }
    else if (name == "sharedgeometry")
    {
        my::Xml<char>::attr_list::const_iterator vertexcount_iter = attrs.find("vertexcount");
        if (vertexcount_iter != attrs.end())
        {
            total_vertices = atoi(vertexcount_iter->second.c_str());
        }
        else
        {
            THROW_CUSEXCEPTION("invalid total_vertices");
        }
    }
    else if (name == "vertexbuffer")
    {
        if (mesh->m_ptr)
        {
            return;
        }

        _ASSERT(D3DDECLTYPE_UNUSED == mesh->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Type);

        _ASSERT(0 == offset);

        my::Xml<char>::attr_list::const_iterator positions_iter = attrs.find("positions");
        if (positions_iter != attrs.end() && boost::iequals(positions_iter->second, "true"))
        {
            mesh->m_VertexElems.InsertPositionElement(0);
            offset += sizeof(Vector3);
        }
        else
        {
            THROW_CUSEXCEPTION("cannot process non-position vertex");
        }

        my::Xml<char>::attr_list::const_iterator normals_iter = attrs.find("normals");
        if (bComputeTangentFrame || normals_iter != attrs.end() && boost::iequals(normals_iter->second, "true"))
        {
            mesh->m_VertexElems.InsertNormalElement(offset);
            offset += sizeof(Vector3);
        }

        if (bComputeTangentFrame)
        {
            mesh->m_VertexElems.InsertTangentElement(offset);
            offset += sizeof(Vector3);
        }

        my::Xml<char>::attr_list::const_iterator colours_diffuse_iter = attrs.find("colours_diffuse");
        if (colours_diffuse_iter != attrs.end() && boost::iequals(colours_diffuse_iter->second, "true"))
        {
            mesh->m_VertexElems.InsertColorElement(offset);
            offset += sizeof(D3DCOLOR);
        }

        my::Xml<char>::attr_list::const_iterator texture_coords_iter = attrs.find("texture_coords");
        int texture_coords = texture_coords_iter != attrs.end() ? Min(D3DVertexElementSet::MAX_USAGE_INDEX, atoi(texture_coords_iter->second.c_str())) : 0;
        for (int i = 0; i < texture_coords; i++)
        {
            mesh->m_VertexElems.InsertTexcoordElement(offset, i);
            offset += sizeof(Vector2);
        }

        if (has_boneassignments)
        {
            mesh->m_VertexElems.InsertBlendIndicesElement(offset);
            offset += sizeof(DWORD);

            mesh->m_VertexElems.InsertBlendWeightElement(offset);
            offset += sizeof(Vector4);
        }

        std::vector<D3DVERTEXELEMENT9> velist = mesh->m_VertexElems.BuildVertexElementList(0);
        D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
        velist.push_back(ve_end);

        D3DContext::getSingleton().m_d3dDeviceSec.Enter();
        if (FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(velist.data(), &mesh->m_Decl)))
        {
            THROW_D3DEXCEPTION(hr);
        }
        D3DContext::getSingleton().m_d3dDeviceSec.Leave();

        D3DContext::getSingleton().m_d3dDeviceSec.Enter();
        mesh->CreateMesh(total_faces, total_vertices, velist.data(), dwMeshOptions);
        D3DContext::getSingleton().m_d3dDeviceSec.Leave();

        _ASSERT(!pVertices);

        D3DContext::getSingleton().m_d3dDeviceSec.Enter();
        pVertices = mesh->LockVertexBuffer();
        D3DContext::getSingleton().m_d3dDeviceSec.Leave();

        _ASSERT(!pIndices);

        D3DContext::getSingleton().m_d3dDeviceSec.Enter();
        pIndices = mesh->LockIndexBuffer();
        D3DContext::getSingleton().m_d3dDeviceSec.Leave();

        _ASSERT(!pAttrBuffer);

        D3DContext::getSingleton().m_d3dDeviceSec.Enter();
        pAttrBuffer = mesh->LockAttributeBuffer();
        D3DContext::getSingleton().m_d3dDeviceSec.Leave();

        _ASSERT(0 == vertex_i && 0 == face_i && 0 == rang.AttribId);
    }
    else if (name == "vertex")
    {
        _ASSERT(pVertices);

        pVertex = (unsigned char*)pVertices + vertex_i * offset;

        texusage_i = 0;

        if (has_boneassignments)
        {
            mesh->m_VertexElems.SetBlendIndices(pVertex, 0);
            mesh->m_VertexElems.SetBlendWeight(pVertex, Vector4::zero);
        }
    }
    else if (name == "position")
    {
        Vector3& Position = mesh->m_VertexElems.GetPosition(pVertex);

        my::Xml<char>::attr_list::const_iterator x_iter = attrs.find("x");
        if (x_iter != attrs.end())
        {
            Position.x = (float)atof(x_iter->second.c_str());
        }

        my::Xml<char>::attr_list::const_iterator y_iter = attrs.find("y");
        if (y_iter != attrs.end())
        {
            Position.y = (float)atof(y_iter->second.c_str());
        }

        my::Xml<char>::attr_list::const_iterator z_iter = attrs.find("z");
        if (z_iter != attrs.end())
        {
            Position.z = (float)atof(z_iter->second.c_str());
        }
    }
    else if (name == "normal")
    {
        Vector3& Normal = mesh->m_VertexElems.GetNormal(pVertex);

        my::Xml<char>::attr_list::const_iterator x_iter = attrs.find("x");
        if (x_iter != attrs.end())
        {
            Normal.x = (float)atof(x_iter->second.c_str());
        }

        my::Xml<char>::attr_list::const_iterator y_iter = attrs.find("y");
        if (y_iter != attrs.end())
        {
            Normal.y = (float)atof(y_iter->second.c_str());
        }

        my::Xml<char>::attr_list::const_iterator z_iter = attrs.find("z");
        if (z_iter != attrs.end())
        {
            Normal.z = (float)atof(z_iter->second.c_str());
        }
    }
    else if (name == "colour_diffuse")
    {
        my::Xml<char>::attr_list::const_iterator value_iter = attrs.find("value");
        if (value_iter != attrs.end())
        {
            std::vector<std::string> color_set;
            boost::algorithm::split(color_set, value_iter->second, boost::is_any_of(" "), boost::algorithm::token_compress_off);
            D3DXCOLOR Color(
                boost::lexical_cast<float>(color_set[0]),
                boost::lexical_cast<float>(color_set[1]),
                boost::lexical_cast<float>(color_set[2]),
                boost::lexical_cast<float>(color_set[3]));
            mesh->m_VertexElems.SetColor(pVertex, Color);
        }
    }
    else if (name == "texcoord")
    {
        Vector2& Texcoord = mesh->m_VertexElems.GetTexcoord(pVertex, texusage_i++);

        my::Xml<char>::attr_list::const_iterator u_iter = attrs.find("u");
        if (u_iter != attrs.end())
        {
            Texcoord.x = (float)atof(u_iter->second.c_str());
        }

        my::Xml<char>::attr_list::const_iterator v_iter = attrs.find("v");
        if (v_iter != attrs.end())
        {
            Texcoord.y = (float)atof(v_iter->second.c_str());
        }
    }
    else if (name == "submeshes")
    {

    }
    else if (name == "submesh")
    {

    }
    else if (name == "faces")
    {
        rang.FaceStart = face_i;

        my::Xml<char>::attr_list::const_iterator count_iter = attrs.find("count");
        if (count_iter != attrs.end())
        {
            rang.FaceCount = atoi(count_iter->second.c_str());
        }

        vmin = INT_MAX;

        vmax = INT_MIN;
    }
    else if (name == "face")
    {
        int v1;
        my::Xml<char>::attr_list::const_iterator v1_iter = attrs.find("v1");
        if (v1_iter != attrs.end())
        {
            v1 = atoi(v1_iter->second.c_str());
        }

        int v2;
        my::Xml<char>::attr_list::const_iterator v2_iter = attrs.find("v2");
        if (v2_iter != attrs.end())
        {
            v2 = atoi(v2_iter->second.c_str());
        }

        int v3;
        my::Xml<char>::attr_list::const_iterator v3_iter = attrs.find("v3");
        if (v3_iter != attrs.end())
        {
            v3 = atoi(v3_iter->second.c_str());
        }

        if (dwMeshOptions & D3DXMESH_32BIT)
        {
            *((DWORD*)pIndices + face_i * 3 + 0) = v1;
            *((DWORD*)pIndices + face_i * 3 + 1) = v2;
            *((DWORD*)pIndices + face_i * 3 + 2) = v3;
        }
        else
        {
            *((WORD*)pIndices + face_i * 3 + 0) = v1;
            *((WORD*)pIndices + face_i * 3 + 1) = v2;
            *((WORD*)pIndices + face_i * 3 + 2) = v3;
        }
        pAttrBuffer[face_i] = rang.AttribId;

        vmin = Min(vmin, Min(v1, Min(v2, v3)));

        vmax = Max(vmax, Max(v1, Max(v2, v3)));
    }
    else if (name == "geometry")
    {

    }
    else if (name == "boneassignments")
    {

    }
    else if (name == "vertexboneassignment")
    {
        int boneindex;
        my::Xml<char>::attr_list::const_iterator boneindex_iter = attrs.find("boneindex");
        if (boneindex_iter != attrs.end())
        {
            boneindex = atoi(boneindex_iter->second.c_str());
        }

        int vertexindex;
        my::Xml<char>::attr_list::const_iterator vertexindex_iter = attrs.find("vertexindex");
        if (vertexindex_iter != attrs.end())
        {
            vertexindex = atoi(vertexindex_iter->second.c_str());
        }

        float weight;
        my::Xml<char>::attr_list::const_iterator weight_iter = attrs.find("weight");
        if (weight_iter != attrs.end())
        {
            weight = (float)atof(weight_iter->second.c_str());
        }

        if (vertexindex >= total_vertices)
        {
            THROW_CUSEXCEPTION(str_printf("invalid vertex index: %d", vertexindex));
        }

        if (boneindex >= 0xff)
        {
            THROW_CUSEXCEPTION(str_printf("invalid bone index: %d", boneindex));
        }

        unsigned char* pVertex = (unsigned char*)pVertices + vertexindex * offset;
        unsigned char* pIndices = (unsigned char*)&mesh->m_VertexElems.GetBlendIndices(pVertex);
        float* pWeights = (float*)&mesh->m_VertexElems.GetBlendWeight(pVertex);

        int i = std::distance(pWeights, boost::find_if(boost::make_iterator_range_n(
            pWeights, D3DVertexElementSet::MAX_BONE_INDICES), boost::bind(std::equal_to<float>(), boost::placeholders::_1, 0.0f)));
        if (i < D3DVertexElementSet::MAX_BONE_INDICES)
        {
            pIndices[i] = boneindex;
            pWeights[i] = weight;
        }
        else
        {
            THROW_CUSEXCEPTION("too much bone assignment");
        }
    }
}

void OgreMeshHandler::on_end_element(const std::string& name)
{
    if (name == "mesh")
    {
        if (pVertices)
        {
            if (bComputeTangentFrame)
            {
                OgreMesh::ComputeTangentFrame(
                    pVertices, total_vertices, offset, pIndices, !(dwMeshOptions & D3DXMESH_32BIT), total_faces, mesh->m_VertexElems);
            }

            D3DContext::getSingleton().m_d3dDeviceSec.Enter();
            mesh->UnlockVertexBuffer();
            D3DContext::getSingleton().m_d3dDeviceSec.Leave();
            pVertices = NULL;
        }

        if (pIndices)
        {
            D3DContext::getSingleton().m_d3dDeviceSec.Enter();
            mesh->UnlockIndexBuffer();
            D3DContext::getSingleton().m_d3dDeviceSec.Leave();
            pIndices = NULL;
        }

        if (pAttrBuffer)
        {
            D3DContext::getSingleton().m_d3dDeviceSec.Enter();
            mesh->UnlockAttributeBuffer();
            D3DContext::getSingleton().m_d3dDeviceSec.Leave();
            pAttrBuffer = NULL;
        }

        D3DContext::getSingleton().m_d3dDeviceSec.Enter();
        mesh->SetAttributeTable(&mesh->m_AttribTable[0], mesh->m_AttribTable.size());
        D3DContext::getSingleton().m_d3dDeviceSec.Leave();

        mesh->m_Vb.Create(mesh->GetVertexBuffer().Detach());
        mesh->m_Ib.Create(mesh->GetIndexBuffer().Detach());
    }
    else if (name == "vertex")
    {
        vertex_i++;
    }
    else if (name == "faces")
    {
        rang.VertexStart = vmin;
        rang.VertexCount = vmax - vmin + 1;
        mesh->m_AttribTable.push_back(rang);
        rang.AttribId++;
    }
    else if (name == "face")
    {
        face_i++;
    }
}

void OgreMeshHandler::on_data(const std::string& value)
{
}
