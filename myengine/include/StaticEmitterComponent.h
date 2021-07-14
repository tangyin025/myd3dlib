#pragma once

#include "Component.h"

class StaticEmitterNode
	: public my::OctEntity
{
public:
	int i;

	int j;

	unsigned int particle_begin;

	unsigned int particle_num;
};

class StaticEmitterChunk
{
public:
	std::shared_ptr<my::Emitter::Particle[]> m_buff;

	typedef std::map<std::pair<int, int>, StaticEmitterNode> NodeMap;
	
	NodeMap m_node;

public:
	StaticEmitterChunk(void)
	{
	}

	virtual ~StaticEmitterChunk(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
	}
};

class StaticEmitterComponent
	: public EmitterComponent
	, public my::OctRoot
{
public:
	float m_ChunkStep;

	int m_ChunkSize;

	int m_NodeSize;

	typedef std::map<std::pair<int, int>, StaticEmitterChunk> ChunkMap;

	ChunkMap m_Chunks;

protected:
	StaticEmitterComponent(void)
	{
	}

public:
	StaticEmitterComponent(const char* Name, unsigned int Capacity, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: EmitterComponent(ComponentTypeStaticEmitter, Name, _FaceType, _SpaceTypeWorld, _VelocityType, _PrimitiveType)
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
