#pragma once

#include "myPhysics.h"
#include "myTexture.h"
#include "myMesh.h"
#include "mySpline.h"
#include <deque>
#include <set>
#include <boost/serialization/nvp.hpp>

namespace my
{
	class EmitterParticle : public Particle
	{
	public:
		D3DCOLOR m_Color;

		Vector4 m_Texcoord1;

		Vector4 m_Texcoord2;

	public:
		EmitterParticle(
			const Vector3 & position,
			const Vector3 & velocity)
			: Particle(position, velocity, Vector3::zero, Vector3::zero, 1, 1)
			, m_Color(D3DCOLOR_ARGB(255,255,255,255))
			, m_Texcoord1(1,1,0,1)
			, m_Texcoord2(0,0,0,0)
		{
		}
	};

	typedef boost::shared_ptr<EmitterParticle> EmitterParticlePtr;

	class EmitterInstance;

	class Emitter
		: public DeviceRelatedObjectBase
	{
	public:
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

		BaseTexturePtr m_Texture;

		typedef std::deque<std::pair<EmitterParticlePtr, float> > ParticlePtrPairList;

		ParticlePtrPairList m_ParticleList;

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

		void UpdateParticle(EmitterParticle * particle, float time, float fElapsedTime);

		virtual void Update(double fTime, float fElapsedTime);

		DWORD BuildInstance(
			EmitterInstance * pInstance,
			double fTime,
			float fElapsedTime);

		void Draw(
			EmitterInstance * pInstance,
			const Quaternion & ViewOrientation,
			double fTime,
			float fElapsedTime);
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

	class EmitterInstance
	{
	public:
		CComPtr<IDirect3DDevice9> m_Device;

		D3DVertexElementSet m_VertexElems;

		VertexBuffer m_VertexBuffer;

		IndexBuffer m_IndexData;

		D3DVertexElementSet m_InstanceElems;

		VertexBuffer m_InstanceData;

		std::vector<D3DVERTEXELEMENT9> m_velist;

		DWORD m_VertexStride;

		DWORD m_InstanceStride;

		CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	public:
		EmitterInstance(void);

		virtual ~EmitterInstance(void);

		HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		virtual void Begin(void)
		{
		}

		virtual void End(void)
		{
		}

		virtual void SetWorld(const Matrix4 & World)
		{
		}

		virtual void SetViewProj(const Matrix4 & ViewProj)
		{
		}

		virtual void SetTexture(const BaseTexturePtr & Texture)
		{
		}

		virtual void SetDirection(const Vector3 & Dir, const Vector3 & Up, const Vector3 & Right)
		{
		}

		virtual void SetAnimationColumnRow(unsigned char Column, unsigned char Row)
		{
		}

		virtual void DrawInstance(DWORD NumInstances);
	};

	typedef boost::shared_ptr<EmitterInstance> EmitterInstancePtr;
}
