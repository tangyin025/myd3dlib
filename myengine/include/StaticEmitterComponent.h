#pragma once

#include "Component.h"

class StaticEmitterChunkBuffer
	: public my::DeviceResourceBase
	, public std::vector<my::Emitter::Particle>
{
public:
	StaticEmitterChunkBuffer(void)
	{
	}
};

typedef boost::intrusive_ptr<StaticEmitterChunkBuffer> StaticEmitterChunkBufferPtr;

class StaticEmitterChunk
	: public my::OctEntity
{
public:
	int m_Row;

	int m_Col;

	bool m_Requested;

	StaticEmitterChunkBufferPtr m_buff;

protected:
	StaticEmitterChunk(void)
		: m_Requested(false)
	{
	}

public:
	StaticEmitterChunk(int Row, int Col)
		: m_Row(Row)
		, m_Col(Col)
		, m_Requested(false)
	{
	}

	virtual ~StaticEmitterChunk(void)
	{
	}

	friend class boost::serialization::access;

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

class StaticEmitterComponent
	: public EmitterComponent
	, public my::OctRoot
{
public:
	int m_EmitterRowChunks;

	int m_EmitterChunkSize;

	typedef std::map<std::pair<int, int>, StaticEmitterChunk> ChunkMap;

	ChunkMap m_Chunks;

	std::string m_ChunkPath;

	typedef std::set<StaticEmitterChunk *> ChunkSet;

	ChunkSet m_ViewedChunks;

protected:
	StaticEmitterComponent(void)
	{
	}

public:
	StaticEmitterComponent(const char* Name, int RowChunks, int ChunkSize, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType);

	virtual ~StaticEmitterComponent(void)
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

	void CopyFrom(const StaticEmitterComponent& rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos);
};

typedef boost::shared_ptr<StaticEmitterComponent> StaticEmitterComponentPtr;

class StaticEmitterStream
{
public:
	StaticEmitterComponent * m_emit;

	typedef std::map<std::pair<int, int>, StaticEmitterChunkBufferPtr> BufferMap;

	BufferMap m_buffs;

	void Release(void);

	StaticEmitterChunkBuffer * GetBuffer(int k, int l);

	void SetBuffer(int k, int l, my::DeviceResourceBasePtr res);

	void Spawn(const my::Vector3 & Position, const my::Vector3 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time);

	StaticEmitterStream(StaticEmitterComponent* emit)
		: m_emit(emit)
	{
	}

	~StaticEmitterStream(void)
	{
		Release();
	}
};
