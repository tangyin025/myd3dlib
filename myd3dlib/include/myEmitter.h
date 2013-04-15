#pragma once

#include "mySingleton.h"
#include "myPhysics.h"
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
		T m_ConstantValue;

		EmitterParameter(const T & Value)
			: m_ConstantValue(Value)
		{
		}

		T Interpolate(float s)
		{
			if(empty())
				return m_ConstantValue;

			return (T)Spline::Interpolate(s, 0, size());
		}
	};

	class Emitter
	{
	public:
		typedef std::deque<std::pair<ParticlePtr, float> > ParticlePtrPairList;

		ParticlePtrPairList m_ParticleList;

		float m_ParticleLifeTime;

		EmitterParameter<int> m_ParticleColorAlpha;

		EmitterParameter<int> m_ParticleColorRed;

		EmitterParameter<int> m_ParticleColorGreen;

		EmitterParameter<int> m_ParticleColorBlue;

		EmitterParameter<float> m_ParticleSizeX;

		EmitterParameter<float> m_ParticleSizeY;

		EmitterParameter<float> m_ParticleAngle;

		float m_ParticleAnimFPS;

		unsigned char m_ParticleAnimColumn;

		unsigned char m_ParticleAnimRow;

	public:
		Emitter(void)
			: m_ParticleLifeTime(10)
			, m_ParticleColorAlpha(255)
			, m_ParticleColorRed(255)
			, m_ParticleColorGreen(255)
			, m_ParticleColorBlue(255)
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

		void Draw(IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;

	class AutoSpawnEmitter
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
		AutoSpawnEmitter(void)
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

	typedef boost::shared_ptr<AutoSpawnEmitter> AutoSpawnEmitterPtr;

	class EmitterInstance
		: public SingleInstance<EmitterInstance>
	{
	public:
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

		virtual void DrawInstance(IDirect3DDevice9 * pd3dDevice, DWORD NumInstances);
	};
}
