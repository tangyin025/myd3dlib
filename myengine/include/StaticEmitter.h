#pragma once

#include "Component.h"
#include <boost/intrusive/list.hpp>

class StaticEmitterChunkBuffer
	: public my::DeviceResourceBase
	, public std::vector<my::Emitter::Particle>
{
public:
	StaticEmitterChunkBuffer(void)
	{
	}
};

typedef boost::shared_ptr<StaticEmitterChunkBuffer> StaticEmitterChunkBufferPtr;

struct StaticEmitterTag;

class StaticEmitterChunk
	: public my::OctEntity
	, public boost::intrusive::list_base_hook<boost::intrusive::tag<StaticEmitterTag> >
{
public:
	int m_Row;

	int m_Col;

	bool m_Requested;

	int m_Lod;

	StaticEmitterChunkBufferPtr m_buff;

public:
	StaticEmitterChunk(int Row, int Col)
		: m_Row(Row)
		, m_Col(Col)
		, m_Requested(false)
		, m_Lod(INT_MAX)
	{
	}

	virtual ~StaticEmitterChunk(void);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
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

	void OnChunkBufferReady(my::DeviceResourceBasePtr res);
};

class StaticEmitter
	: public EmitterComponent
	, public my::OctRoot
{
public:
	enum { TypeID = ComponentTypeStaticEmitter };

	float m_ChunkWidth;

	std::string m_EmitterChunkPath;

	typedef std::map<std::pair<int, int>, StaticEmitterChunk> ChunkMap;

	ChunkMap m_Chunks;

	typedef boost::intrusive::list<StaticEmitterChunk, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<StaticEmitterTag> > > > ChunkSet;

	ChunkSet m_ViewedChunks;

protected:
	StaticEmitter(void)
	{
	}

public:
	StaticEmitter(const char* Name, const my::AABB & LocalRootAabb, float ChunkWidth, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType);

	virtual ~StaticEmitter(void)
	{
		ClearAllEntity();
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive& ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos);
};

typedef boost::shared_ptr<StaticEmitter> StaticEmitterPtr;

class StaticEmitterStream
{
public:
	StaticEmitter * m_emit;

	typedef std::map<std::pair<int, int>, StaticEmitterChunkBufferPtr> BufferMap;

	BufferMap m_buffs;

	std::map<std::pair<int, int>, bool> m_dirty;

	void Release(void);

	StaticEmitterChunkBuffer * GetBuffer(int i, int j);

	void Spawn(const my::Vector4 & Position, const my::Vector4 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time);

	my::Emitter::Particle * GetNearestParticle2D(float x, float z, float max_dist);

	StaticEmitterStream(StaticEmitter* emit)
		: m_emit(emit)
	{
	}

	~StaticEmitterStream(void)
	{
		Release();
	}
};
