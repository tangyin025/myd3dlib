#pragma once

#include "myPhysics.h"
#include "myTexture.h"
#include "myMesh.h"
#include "mySpline.h"
#include <deque>

namespace my
{
	class EmitterInstance;

	template <typename T>
	class EmitterParameter
		: public Spline
	{
	public:
		T m_Value;

		EmitterParameter(void)
			: m_Value(0)
		{
		}

		EmitterParameter(const T & Value)
			: m_Value(Value)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & boost::serialization::base_object<Spline>(*this);
			ar & m_Value;
		}

		T Interpolate(float s) const
		{
			if(empty())
				return m_Value;

			return (T)Spline::Interpolate(s, 0, size());
		}
	};

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

		EmitterParameter<int> m_ParticleColorA;

		EmitterParameter<int> m_ParticleColorR;

		EmitterParameter<int> m_ParticleColorG;

		EmitterParameter<int> m_ParticleColorB;

		EmitterParameter<float> m_ParticleSizeX;

		EmitterParameter<float> m_ParticleSizeY;

		EmitterParameter<float> m_ParticleAngle;

		float m_ParticleAnimFPS;

		unsigned char m_ParticleAnimColumn;

		unsigned char m_ParticleAnimRow;

		BaseTexturePtr m_Texture;

		typedef std::deque<std::pair<ParticlePtr, float> > ParticlePtrPairList;

		ParticlePtrPairList m_ParticleList;

	public:
		Emitter(void)
			: m_WorldType(WorldTypeWorld)
			, m_DirectionType(DirectionTypeCamera)
			, m_Position(0,0,0)
			, m_Orientation(Quaternion::Identity())
			, m_ParticleLifeTime(10)
			, m_ParticleColorA(255)
			, m_ParticleColorR(255)
			, m_ParticleColorG(255)
			, m_ParticleColorB(255)
			, m_ParticleSizeX(1)
			, m_ParticleSizeY(1)
			, m_ParticleAngle(0)
			, m_ParticleAnimFPS(1)
			, m_ParticleAnimColumn(1)
			, m_ParticleAnimRow(1)
		{
		}

		virtual ~Emitter(void);

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & m_WorldType;
			ar & m_DirectionType;
			ar & m_Position;
			ar & m_Orientation;
			ar & m_ParticleLifeTime;
			ar & m_ParticleColorA;
			ar & m_ParticleColorR;
			ar & m_ParticleColorG;
			ar & m_ParticleColorB;
			ar & m_ParticleSizeX;
			ar & m_ParticleSizeY;
			ar & m_ParticleAngle;
			ar & m_ParticleAnimFPS;
			ar & m_ParticleAnimColumn;
			ar & m_ParticleAnimRow;
		}

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void Reset(void);

		void Spawn(const Vector3 & Position, const Vector3 & Velocity);

		virtual void Update(double fTime, float fElapsedTime);

		DWORD BuildInstance(
			EmitterInstance * pEmitterInstance,
			double fTime,
			float fElapsedTime);

		void Draw(
			EmitterInstance * pEmitterInstance,
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

		EmitterParameter<float> m_SpawnInclination;

		EmitterParameter<float> m_SpawnAzimuth;

		float m_SpawnLoopTime;

	public:
		SphericalEmitter(void)
			: m_Time(0)
			, m_SpawnInterval(5)
			, m_RemainingSpawnTime(0)
			, m_HalfSpawnArea(0,0,0)
			, m_SpawnSpeed(0)
			, m_SpawnInclination(D3DXToRadian(0))
			, m_SpawnAzimuth(D3DXToRadian(0))
			, m_SpawnLoopTime(5)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & boost::serialization::base_object<Emitter>(*this);
			ar & m_Time;
			ar & m_SpawnInterval;
			ar & m_RemainingSpawnTime;
			ar & m_HalfSpawnArea;
			ar & m_SpawnSpeed;
			ar & m_SpawnInclination;
			ar & m_SpawnAzimuth;
			ar & m_SpawnLoopTime;
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
