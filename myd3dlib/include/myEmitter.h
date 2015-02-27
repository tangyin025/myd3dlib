#pragma once

//#include "myTexture.h"
//#include "myMesh.h"
#include "mySingleton.h"
#include "myMath.h"
#include "mySpline.h"
#include <deque>
#include <set>
#include <boost/serialization/nvp.hpp>

#define PARTICLE_INSTANCE_MAX 4096u

namespace my
{
	//class ParticleInstance;

	class Emitter
		: public DeviceRelatedObjectBase
	{
	public:
		class Particle
		{
		public:
			Vector3 m_Position;

			Vector3 m_Velocity;

			D3DCOLOR m_Color;

			Vector4 m_Texcoord1;

			Vector4 m_Texcoord2;

		public:
			Particle(const Vector3 & Position, const Vector3 & Velocity)
				: m_Position(Position)
				, m_Velocity(Velocity)
				, m_Color(D3DCOLOR_ARGB(255,255,255,255))
				, m_Texcoord1(1,1,0,1)
				, m_Texcoord2(0,0,0,0)
			{
			}
		};

		typedef std::deque<std::pair<float, Particle> > ParticlePairList;

		enum WorldType
		{
			WorldTypeWorld,
			WorldTypeLocal,
		};

		WorldType m_WorldType;

		enum DirectionType
		{
			DirectionTypeCamera,
			DirectionTypeVertical,
			DirectionTypeHorizontal,
		};

		DirectionType m_DirectionType;

		Vector3 m_Position;

		Quaternion m_Orientation;

		float m_ParticleLifeTime;

		Spline m_ParticleColorA;

		Spline m_ParticleColorR;

		Spline m_ParticleColorG;

		Spline m_ParticleColorB;

		Spline m_ParticleSizeX;

		Spline m_ParticleSizeY;

		Spline m_ParticleAngle;

		float m_ParticleAnimFPS;

		unsigned char m_ParticleAnimColumn;

		unsigned char m_ParticleAnimRow;

		//BaseTexturePtr m_Texture;

		ParticlePairList m_ParticleList;

	public:
		Emitter(void)
			: m_WorldType(WorldTypeWorld)
			, m_DirectionType(DirectionTypeCamera)
			, m_Position(0,0,0)
			, m_Orientation(Quaternion::Identity())
			, m_ParticleLifeTime(10)
			, m_ParticleAnimFPS(1)
			, m_ParticleAnimColumn(1)
			, m_ParticleAnimRow(1)
		{
		}

		virtual ~Emitter(void);

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_WorldType);
			ar & BOOST_SERIALIZATION_NVP(m_DirectionType);
			ar & BOOST_SERIALIZATION_NVP(m_Position);
			ar & BOOST_SERIALIZATION_NVP(m_Orientation);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleColorA);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleColorR);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleColorG);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleColorB);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleSizeX);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleSizeY);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleAngle);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleAnimFPS);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleAnimColumn);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleAnimRow);
		}

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void Reset(void);

		void Spawn(const Vector3 & Position, const Vector3 & Velocity);

		virtual void Update(double fTime, float fElapsedTime);

		void UpdateParticle(Particle & particle, float time, float fElapsedTime);

		//DWORD BuildInstance(
		//	ParticleInstance * pInstance,
		//	double fTime,
		//	float fElapsedTime);

		//void Draw(
		//	ParticleInstance * pInstance,
		//	const Matrix4 & View,
		//	double fTime,
		//	float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;

	class SphericalEmitter
		: public Emitter
	{
	public:
		float m_Time;

		float m_SpawnInterval;

		float m_RemainingSpawnTime;

		Vector3 m_HalfSpawnArea;

		float m_SpawnSpeed;

		Spline m_SpawnInclination;

		Spline m_SpawnAzimuth;

		float m_SpawnLoopTime;

	public:
		SphericalEmitter(void)
			: m_Time(0)
			, m_SpawnInterval(5)
			, m_RemainingSpawnTime(0)
			, m_HalfSpawnArea(0,0,0)
			, m_SpawnSpeed(0)
			, m_SpawnLoopTime(5)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Emitter);
			ar & BOOST_SERIALIZATION_NVP(m_Time);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnInterval);
			ar & BOOST_SERIALIZATION_NVP(m_RemainingSpawnTime);
			ar & BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnInclination);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
			ar & BOOST_SERIALIZATION_NVP(m_SpawnLoopTime);
		}

		virtual void Update(double fTime, float fElapsedTime);
	};

	typedef boost::shared_ptr<SphericalEmitter> SphericalEmitterPtr;

	//class ParticleInstance : public DeviceRelatedObjectBase
	//{
	//public:
	//	HRESULT hr;

	//	CComPtr<IDirect3DDevice9> m_Device;

	//	D3DVertexElementSet m_VertexElems;

	//	VertexBuffer m_VertexBuffer;

	//	IndexBuffer m_IndexData;

	//	D3DVertexElementSet m_InstanceElems;

	//	VertexBuffer m_InstanceData;

	//	std::vector<D3DVERTEXELEMENT9> m_velist;

	//	DWORD m_VertexStride;

	//	DWORD m_InstanceStride;

	//	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	//public:
	//	ParticleInstance(void);

	//	virtual ~ParticleInstance(void);

	//	virtual void OnResetDevice(void);

	//	virtual void OnLostDevice(void);

	//	virtual void OnDestroyDevice(void);

	//	void CreateInstance(IDirect3DDevice9 * pd3dDevice);

	//	unsigned char * LockInstanceData(DWORD NumInstances);

	//	void UnlockInstanceData(void);

	//	virtual void Begin(void) = 0;

	//	virtual void End(void) = 0;

	//	virtual void SetWorld(const Matrix4 & World) = 0;

	//	virtual void SetViewProj(const Matrix4 & ViewProj) = 0;

	//	virtual void SetTexture(const BaseTexturePtr & Texture) = 0;

	//	virtual void SetDirection(const Vector3 & Dir, const Vector3 & Up, const Vector3 & Right) = 0;

	//	virtual void SetAnimationColumnRow(unsigned char Column, unsigned char Row) = 0;

	//	virtual void DrawInstance(DWORD NumInstances);
	//};

	//typedef boost::shared_ptr<ParticleInstance> ParticleInstancePtr;
}
