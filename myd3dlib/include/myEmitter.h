#pragma once

#include "mySingleton.h"
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

		EmitterParameter(const T & Value)
			: m_Value(Value)
		{
		}

		T Interpolate(float s)
		{
			if(empty())
				return m_Value;

			return (T)Spline::Interpolate(s, 0, size());
		}
	};

	class Emitter
	{
	public:
		enum DirectionType
		{
			DirectionTypeCamera,
			DirectionTypeVertical,
			DirectionTypeHorizontal,
		};

		DirectionType m_Direction;

		typedef std::deque<std::pair<ParticlePtr, float> > ParticlePtrPairList;

		ParticlePtrPairList m_ParticleList;

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

		TexturePtr m_Texture;

	public:
		Emitter(void)
			: m_Direction(DirectionTypeCamera)
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

		virtual ~Emitter(void)
		{
		}

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
		Vector3 m_Position;

		Quaternion m_Orientation;

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
			: m_Position(0,0,0)
			, m_Orientation(Quaternion::Identity())
			, m_Time(0)
			, m_SpawnInterval(10)
			, m_RemainingSpawnTime(0)
			, m_HalfSpawnArea(0,0,0)
			, m_SpawnSpeed(0)
			, m_SpawnInclination(D3DXToRadian(0))
			, m_SpawnAzimuth(D3DXToRadian(0))
			, m_SpawnLoopTime(10)
		{
		}

		virtual void Update(double fTime, float fElapsedTime);
	};

	typedef boost::shared_ptr<SphericalEmitter> SphericalEmitterPtr;

	class EmitterInstance
	{
	public:
		CComPtr<IDirect3DDevice9> m_Device;

		D3DVERTEXELEMENT9Set m_VertexElemSet;

		VertexBuffer m_VertexBuffer;

		IndexBuffer m_IndexData;

		VertexBuffer m_InstanceData;

		DWORD m_VertexStride;

		DWORD m_InstanceStride;

		CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	public:
		EmitterInstance(void)
		{
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, 0, 0));
			WORD offset = 0;
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreatePositionElement(1, 0, 0));
			offset += sizeof(D3DVERTEXELEMENT9Set::PositionType);
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateColorElement(1, offset, 0));
			offset += sizeof(D3DVERTEXELEMENT9Set::ColorType);
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateCustomElement(1, D3DDECLUSAGE_TEXCOORD, 1, offset, D3DDECLTYPE_FLOAT4));
			offset += sizeof(FLOAT) * 4;
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateCustomElement(1, D3DDECLUSAGE_TEXCOORD, 2, offset, D3DDECLTYPE_UBYTE4));
			offset += sizeof(UCHAR) * 4;

			m_VertexStride = m_VertexElemSet.CalculateVertexStride(0);
			m_InstanceStride = m_VertexElemSet.CalculateVertexStride(1);
		}

		virtual ~EmitterInstance(void)
		{
		}

		HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		virtual void Begin(void) {}

		virtual void End(void) {}

		virtual void SetWorld(const Matrix4 & World) {}

		virtual void SetViewProj(const Matrix4 & ViewProj) {}

		virtual void SetTexture(IDirect3DBaseTexture9 * pTexture) {}

		virtual void SetDirection(const Vector3 & Dir, const Vector3 & Up, const Vector3 & Right) {}

		virtual void SetAnimationColumnRow(unsigned char Column, unsigned char Row) {}

		virtual void DrawInstance(DWORD NumInstances);
	};

	typedef boost::shared_ptr<EmitterInstance> EmitterInstancePtr;
}
