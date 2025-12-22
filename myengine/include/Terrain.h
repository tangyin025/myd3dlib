// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "myTexture.h"
#include "Component.h"
#include <boost/multi_array.hpp>
#include <boost/unordered_map.hpp>
#include <boost/intrusive/list.hpp>
#include <fstream>

class Terrain;

struct TerrainTag;

class TerrainChunk
	: public my::OctEntity
	, public boost::intrusive::list_base_hook<boost::intrusive::tag<TerrainTag> >
{
public:
	int m_Row;

	int m_Col;

	bool m_Requested;

	int m_Lod[5];

	my::VertexBufferPtr m_Vb;

public:
	TerrainChunk(void)
		: m_Row(0)
		, m_Col(0)
		, m_Requested(false)
	{
		m_Lod[0] = UINT_MAX;
	}

	virtual ~TerrainChunk(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Row);
		ar & BOOST_SERIALIZATION_NVP(m_Col);
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void RequestResource(void);

	void ReleaseResource(void);

	static std::string MakeChunkPath(const std::string & ChunkPath, int Row, int Col);

	void OnVertexBufferReady(my::DeviceResourceBasePtr res);
};

class TerrainStream;

class Terrain
	: public Component
	, public my::OctRoot
{
public:
	enum { TypeID = ComponentTypeTerrain };

	static const float MinBlock;

	static const float Threshold;

	int m_RowChunks;

	int m_ColChunks;

	int m_ChunkSize;

	int m_MinChunkLodSize;

	typedef boost::multi_array<unsigned int, 2> IndexTable;

	IndexTable m_IndexTable;

	struct Fragment
	{
		unsigned int VertNum;
		unsigned int PrimitiveCount;
		my::IndexBuffer ib;
	};

	typedef boost::unordered_map<unsigned int, Fragment> FragmentMap;

	FragmentMap m_Fragment;

	std::string m_ChunkPath;

	float m_ChunkLodScale;

	typedef boost::multi_array<TerrainChunk, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

	typedef boost::intrusive::list<TerrainChunk, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<TerrainTag> > > > ChunkSet;

	ChunkSet m_ViewedChunks;

	my::VertexBuffer m_rootVb;

	my::IndexBuffer m_rootIb;

	my::D3DVertexElementSet m_VertexElems;

	static const DWORD m_VertexStride = 12 + 12 + 4;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	std::string m_PxHeightFieldPath;

	boost::shared_ptr<physx::PxHeightField> m_PxHeightField;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_TerrainSize;

	int CalculateLod(int i, int j, const my::Vector3 & LocalViewPos) const;

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

protected:
	Terrain(void);

public:
	Terrain(const char * Name, int RowChunks, int ColChunks, int ChunkSize, int MinChunkLodSize);

	virtual ~Terrain(void);

	friend class boost::serialization::access;
	
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void OnResetShader(void);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	float CalculateHeightScale(void) const;

	void CreateHeightFieldShape(TerrainStream * tstr, const char * HeightFieldPath, const my::Vector3 & ActorScale);

	virtual void ClearShape(void);

	my::RayResult RayTest(const my::Ray& local_ray, const my::Vector3& LocalViewPos, CPoint& raychunkid);

	float RayTest2D(float x, float z);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;

class TerrainStream
{
public:
	Terrain* m_terrain;

	boost::multi_array<my::VertexBufferPtr, 2> m_Vbs;

	boost::multi_array<bool, 2> m_NormalDirty;

	boost::multi_array<bool, 2> m_AabbDirty;

	boost::multi_array<bool, 2> m_VertDirty;

public:
	explicit TerrainStream(Terrain* terrain);

	~TerrainStream(void);

	void Flush(void);

	static void GetIndices(const Terrain* terrain, int i, int j, int& k, int& l, int& m, int& n, int& o, int& p);

	my::VertexBuffer * GetVB(int k, int l);

	my::Vector3 GetPos(int i, int j);

	void SetPos(int i, int j, const my::Vector3& Pos);

	void SetPos(int i, int j, float height);

	void SetPos(int k, int l, int m, int n, const my::Vector3& Pos);

	D3DCOLOR GetColor(int i, int j);

	void SetColor(int i, int j, D3DCOLOR Color);

	void SetColor(int k, int l, int m, int n, D3DCOLOR Color);

	my::Vector3 GetNormal(int i, int j);

	void SetNormal(int i, int j, const my::Vector3& Normal);

	void SetNormal(int k, int l, int m, int n, const my::Vector3& Normal);

	void UpdateNormal(void);

	//my::RayResult RayTest(const my::Ray& local_ray);

	float RayTest2D(float x, float z);
};
