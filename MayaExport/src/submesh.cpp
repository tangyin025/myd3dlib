#include "submesh.h"

namespace OgreMayaExporter
{
	/***** Class Submesh *****/
	// constructor
	Submesh::Submesh(const MString& name)
	{
		clear();
		m_name = name;
	}

	// destructor
	Submesh::~Submesh()
	{
		clear();
	}

	// clear data
	void Submesh::clear()
	{
		m_name = "";
		m_numTriangles = 0;
		m_pMaterial = NULL;
		m_vertices.clear();
		m_faces.clear();
		m_uvsets.clear();
		m_use32bitIndexes = false;
		std::fill_n(m_rot, _countof(m_rot), 0.0f);
		std::fill_n(m_scale, _countof(m_scale), 1.0f);
	}

	// return number of triangles composing the mesh
	long Submesh::numTriangles()
	{
		return m_numTriangles;
	}

	// return number of vertices composing the mesh
	long Submesh::numVertices()
	{
		return m_numVertices;
	}

	// return submesh name
	MString& Submesh::name()
	{
		return m_name;
	}

/***** load data *****/
	MStatus Submesh::loadMaterial(MObject& shader,MStringArray& uvsets,ParamList& params)
	{
		MPlug plug;
		MPlugArray srcplugarray;
		bool foundShader = false;
		MStatus stat;
		MFnLambertShader* pShader;
		//get shader from shading group
		MFnDependencyNode shadingGroup(shader);
		plug = shadingGroup.findPlug("surfaceShader");
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (int i=0; i<srcplugarray.length() && !foundShader; i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kLambert))
			{
				pShader = new MFnLambertShader(srcplugarray[i].node());
				foundShader = true;
			}
		}
		std::cout << "Found material: " << pShader->name().asChar() << "\n";
		//check if this material has already been created
		Material* pMaterial = MaterialSet::getSingleton().getMaterial(pShader->name());
		//if the material has already been created, update the pointer
		if (pMaterial)
			m_pMaterial = pMaterial;
		//else create it and add it to the material set
		else
		{
			pMaterial = new Material();
			pMaterial->load(pShader,uvsets,params);
			m_pMaterial = pMaterial;
			MaterialSet::getSingleton().addMaterial(pMaterial);
		}
		//delete temporary shader
		delete pShader;
		//loading complete
		return MS::kSuccess;
	}

	MStatus Submesh::load(std::vector<face>& faces, std::vector<vertexInfo>& vertInfo, MFloatPointArray& points, 
		MFloatVectorArray& normals, MStringArray& texcoordsets,ParamList& params,bool opposite)
	{
		//save uvsets info
		for (int i=m_uvsets.size(); i<texcoordsets.length(); i++)
		{
			uvset uv;
			uv.size = 2;
			m_uvsets.push_back(uv);
		}
		//iterate over faces array, to retrieve vertices info
		for (int i=0; i<faces.size(); i++)
		{
			face newFace;
			// if we are using shared geometry, indexes refer to the vertex buffer of the whole mesh
			if (params.useSharedGeom)
			{
				if(opposite)
				{	// reverse order of face vertices for correct culling
					newFace.v[0] = faces[i].v[2];
					newFace.v[1] = faces[i].v[1];
					newFace.v[2] = faces[i].v[0];
				}
				else
				{
					newFace.v[0] = faces[i].v[0];
					newFace.v[1] = faces[i].v[1];
					newFace.v[2] = faces[i].v[2];
				}
			}
			// otherwise we create a vertex buffer for this submesh
			else
			{	// faces are triangles, so retrieve index of the three vertices
				for (int j=0; j<3; j++)
				{
					vertex v;
					vertexInfo vInfo = vertInfo[faces[i].v[j]];
					// save vertex coordinates
					v.x = points[vInfo.pointIdx].x;
					v.y = points[vInfo.pointIdx].y;
					v.z = points[vInfo.pointIdx].z;
					// save vertex normal
					if (opposite)
					{
						v.n.x = -normals[vInfo.normalIdx].x;
						v.n.y = -normals[vInfo.normalIdx].y;
						v.n.z = -normals[vInfo.normalIdx].z;
					}
					else
					{
						v.n.x = normals[vInfo.normalIdx].x;
						v.n.y = normals[vInfo.normalIdx].y;
						v.n.z = normals[vInfo.normalIdx].z;
					}
					v.n.normalize();
					// save vertex color
					v.r = vInfo.r;
					v.g = vInfo.g;
					v.b = vInfo.b;
					v.a = vInfo.a;
					// save vertex bone assignements
					for (int k=0; k<vInfo.vba.size(); k++)
					{
						vba newVba;
						newVba.jointIdx = vInfo.jointIds[k];
						newVba.weight = vInfo.vba[k];
						v.vbas.push_back(newVba);
					}
					// save texture coordinates
					for (int k=0; k<vInfo.u.size(); k++)
					{
						texcoords newTexCoords;
						newTexCoords.u = vInfo.u[k];
						newTexCoords.v = vInfo.v[k];
						newTexCoords.w = 0;
						v.texcoords.push_back(newTexCoords);
					}
					int idx;
					bool different = true;
					// check if a vertex with same attributes has been saved already
					for (int k = 0; k < m_vertices.size() && different; k++)
					{
						different = false;
						if (m_vertices[k].x != v.x || m_vertices[k].y != v.y || m_vertices[k].z != v.z)
						{
							different = true;
						}

						if (params.exportVertNorm)
						{
							if (m_vertices[k].n.x != v.n.x || m_vertices[k].n.y != v.n.y || m_vertices[k].n.z != v.n.z)
							{
								different = true;
							}
						}

						if ((params.exportVertCol) &&
							(m_vertices[k].r != v.r || m_vertices[k].g != v.g || m_vertices[k].b != v.b || m_vertices[k].a != v.a))
						{
							different = true;
						}

						if (params.exportTexCoord)
						{
							for (int j = 0; j < vInfo.u.size(); j++)
							{
								if (m_vertices[k].texcoords[j].u != v.texcoords[j].u || m_vertices[k].texcoords[j].v != v.texcoords[j].v)
								{
									different = true;
								}
							}
						}
						idx = k;
					}
					// if no identical vertex has been saved, then save the vertex info
					if (different)
					{
						// add newly created vertex to vertex list
						m_vertices.push_back(v);
						idx = m_vertices.size() - 1;
					}
					if (opposite)	// reverse order of face vertices to get correct culling
						newFace.v[2-j] = idx;
					else
						newFace.v[j] = idx;
				}
			}
			m_faces.push_back(newFace);
		}
		// set use32bitIndexes flag
		if (params.useSharedGeom || (m_vertices.size() > 65535) || (m_faces.size() > 65535))
			m_use32bitIndexes = true;
		else
			m_use32bitIndexes = false;
		return MS::kSuccess;
	}

/***** write data *****/
	MStatus Submesh::writeXML(ParamList &params)
	{
		// Start submesh description
		params.outMesh << "\t\t<submesh ";
		// Write material name
		params.outMesh << "material=\"" << m_pMaterial->name().asChar() << "\" ";
		// Write use32bitIndexes flag
		params.outMesh << "use32bitindexes=\"";
		if (m_use32bitIndexes)
			params.outMesh << "true";
		else
			params.outMesh << "false";
		params.outMesh << "\" ";
		// Write use32bitIndexes flag
		params.outMesh << "usesharedvertices=\"";
		if (params.useSharedGeom)
			params.outMesh << "true";
		else
			params.outMesh << "false";
		params.outMesh << "\" ";
		// Write operation type flag
		params.outMesh << "operationtype=\"triangle_list\">\n";

		// Write submesh polygons
		params.outMesh << "\t\t\t<faces count=\"" << m_faces.size() << "\">\n";
		for (int i=0; i<m_faces.size(); i++)
		{
			params.outMesh << "\t\t\t\t<face v1=\"" << m_faces[i].v[0] << "\" v2=\"" << m_faces[i].v[1] << "\" "
				<< "v3=\"" << m_faces[i].v[2] << "\"/>\n";
		}
		params.outMesh << "\t\t\t</faces>\n";

		// Write mesh geometry
		if (!params.useSharedGeom)
		{
			params.outMesh << "\t\t\t<geometry vertexcount=\"" << m_vertices.size()
				<< "\" transform=\"" << m_pos.x << " " << m_pos.y << " " << m_pos.z << " " << m_rot[0] << " " << m_rot[1] << " " << m_rot[2] << " " << m_scale[0] << " " << m_scale[1] << " " << m_scale[2] << "\">\n";
			params.outMesh << "\t\t\t\t<vertexbuffer positions=\"true\" normals=";
			if (params.exportVertNorm)
				params.outMesh << "\"true\"";
			else 
				params.outMesh << "\"false\"";
			params.outMesh << " colours_diffuse=";
			if (params.exportVertCol)
				params.outMesh << "\"true\"";
			else
				params.outMesh << "\"false\"";
			params.outMesh << " colours_specular=\"false\" texture_coords=\"";
			if (params.exportTexCoord)
				params.outMesh << m_uvsets.size() << "\">\n";
			else
				params.outMesh << 0 << "\">\n";
			//write vertex data
			for (int i=0; i<m_vertices.size(); i++)
			{
				params.outMesh << "\t\t\t\t\t<vertex>\n";
				//write vertex position
				params.outMesh << "\t\t\t\t\t\t<position x=\"" << m_vertices[i].x << "\" y=\"" 
					<< m_vertices[i].y << "\" " << "z=\"" << m_vertices[i].z << "\"/>\n";
				//write vertex normal
				if (params.exportVertNorm)
				{
					params.outMesh << "\t\t\t\t\t\t<normal x=\"" << m_vertices[i].n.x << "\" y=\"" 
						<< m_vertices[i].n.y << "\" " << "z=\"" << m_vertices[i].n.z << "\"/>\n";
				}
				//write vertex colour
				if (params.exportVertCol)
				{
					float r,g,b,a;
					if (params.exportVertColWhite)
					{
						r = g = b = a = 1.0f;
					}
					else
					{
						r = m_vertices[i].r;
						g = m_vertices[i].g;
						b = m_vertices[i].b;
						a = m_vertices[i].a;
					}

					params.outMesh << "\t\t\t\t\t\t<colour_diffuse value=\"" << r << " " << g 
						<< " " << b << " " << a << "\"/>\n";
				}//write vertex texture coordinates
				if (params.exportTexCoord)
				{
					for (int j=0; j<m_uvsets.size(); j++)
					{
						params.outMesh << "\t\t\t\t\t\t<texcoord u=\"" << m_vertices[i].texcoords[j].u << "\" v=\"" << 
							m_vertices[i].texcoords[j].v << "\"/>\n";
					}
				}
				params.outMesh << "\t\t\t\t\t</vertex>\n";
			}
			//end vertex data
			params.outMesh << "\t\t\t\t</vertexbuffer>\n";
			//end geometry description
			params.outMesh << "\t\t\t</geometry>\n";

			// Write bone assignments
			if (params.exportVBA)
			{
				params.outMesh << "\t\t\t<boneassignments>\n";
				for (int i=0; i<m_vertices.size(); i++)
				{
					for (int j=0; j<m_vertices[i].vbas.size(); j++)
					{
						if (m_vertices[i].vbas[j].weight > 0.001)
						{
							params.outMesh << "\t\t\t\t<vertexboneassignment vertexindex=\"" << i 
								<< "\" boneindex=\"" << m_vertices[i].vbas[j].jointIdx << "\" weight=\"" 
								<< m_vertices[i].vbas[j].weight <<"\"/>\n";
						}
					}
				}
				params.outMesh << "\t\t\t</boneassignments>\n";
			}
		}
		// End submesh description
		params.outMesh << "\t\t</submesh>\n";

		return MS::kSuccess;
	}


}; //end of namespace
