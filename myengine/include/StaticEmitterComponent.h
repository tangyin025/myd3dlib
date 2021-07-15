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

public:
	StaticEmitterChunk(void)
		: m_Requested(false)
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
	float m_ChunkStep;

	typedef std::map<std::pair<int, int>, StaticEmitterChunk> ChunkMap;

	ChunkMap m_Chunks;

	std::string m_ChunkPath;

	typedef boost::circular_buffer<boost::shared_ptr<StaticEmitterChunk> > ChunkCircular;

	ChunkCircular m_ViewedChunks;

	template<class T>
	struct AutoReleaseResource
	{
		typedef void result_type;

		typedef T * argument_type;

		void operator()(T * x) const
		{
			_ASSERT(x->IsRequested()); x->ReleaseResource();
		}
	};

protected:
	StaticEmitterComponent(void)
	{
	}

public:
	StaticEmitterComponent(const char* Name, unsigned int Capacity, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: EmitterComponent(ComponentTypeStaticEmitter, Name, _FaceType, _SpaceTypeWorld, _VelocityType, _PrimitiveType)
		, m_ViewedChunks(10)
		, OctRoot(-1.0f, 1.0f)
	{
	}

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

	virtual void Update(float fElapsedTime);

	//void BuildChunks(void);

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos);
};

typedef boost::shared_ptr<StaticEmitterComponent> StaticEmitterComponentPtr;

class StaticEmitterStream
{
public:
	StaticEmitterComponent * m_emit;

	typedef std::map<std::pair<int, int>, StaticEmitterChunkBufferPtr> BufferMap;

	BufferMap m_buffs;

	typedef std::map<std::pair<int, int>, bool> DirtyMap;

	DirtyMap m_dirty;

	void Release(void);

	void Spawn(const my::Vector3 & pos);

	StaticEmitterStream(StaticEmitterComponent* emit)
		: m_emit(emit)
	{
	}

	~StaticEmitterStream(void)
	{
		Release();
	}
};
