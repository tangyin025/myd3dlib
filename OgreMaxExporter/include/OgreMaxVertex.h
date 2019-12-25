/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under 
the terms of the GNU Lesser General Public License as published by the Free Software 
Foundation; either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with 
this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
Place - Suite 330, Boston, MA 02111-1307, USA, or go to 
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#if !defined(__OGREMAX_VERTEX_H__)
#define __OGREMAX_VERTEX_H__
//
//#include "OgreColourValue.h"
//#include "OgreVector3.h"
#include "../../myd3dlib/include/myMath.h"
#include <list>

namespace OgreMax {

	typedef std::map<int, my::Vector3> TexCoordMap;

	typedef std::map<int, float> BoneWeightMap;

	class Vertex {
	public:

		Vertex(float x, float y, float z);

		void setNormal(float x, float y, float z);
		void setColour(float r, float g, float b, float a = 1.0);
		void addTexCoord(int mapIndex, float u, float v, float w = 0.0);
		void addBoneWeight(int boneIndex, float weight);

		bool operator==(Vertex& other);

		const my::Vector3& getPosition() const { return m_position; }
		const my::Vector3& getNormal() const { return m_normal; }
		const my::Vector4& getColour() const { return m_colour; }
		const my::Vector3& getUVW(int mapIndex) const { return m_uvwMap.find(mapIndex)->second; }
		const float getBoneWeight(int boneIndex) const { return m_boneMap.find(boneIndex)->second; }
	
	//private:
		bool hasSameTexCoords(std::map<int, my::Vector3>& uvwMap) ;
		my::Vector3					m_position;
		my::Vector3					m_normal;
		my::Vector4					m_colour;
		TexCoordMap					m_uvwMap;
		BoneWeightMap				m_boneMap;
	};

	class VertexList {
	public:
		// returns the index into the list for the inserted element
		unsigned int add(Vertex& v);
		size_t size() { return m_vertexList.size(); }
		//const Vertex& front() { return m_vertexList.front(); }
		//void pop() { m_vertexList.pop_front(); }

	//private:
		std::vector<Vertex> m_vertexList;

	};

}


#endif