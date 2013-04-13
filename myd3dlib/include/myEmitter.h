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
		Vector3 m_Position;

		Quaternion m_Orientation;

		typedef std::deque<ParticlePtr> ParticlePtrList;

		ParticlePtrList m_ParticleList;

		float m_Time;

		float m_ParticleLifeTime;

		float m_SpawnInterval;

		float m_RemainingSpawnTime;

		Vector3 m_HalfSpawnArea;

		float m_SpawnSpeed;

		EmitterParameter<float> m_SpawnInclination;

		EmitterParameter<float> m_SpawnAzimuth;

		float m_SpawnLoopTime;

		EmitterParameter<int> m_ParticleColorAlpha;

		EmitterParameter<int> m_ParticleColorRed;

		EmitterParameter<int> m_ParticleColorGreen;

		EmitterParameter<int> m_ParticleColorBlue;

	public:
		Emitter(void)
			: m_Position(0,0,0)
			, m_Orientation(Quaternion::Identity())
			, m_Time(0)
			, m_ParticleLifeTime(10)
			, m_SpawnInterval(1/100.0f)
			, m_RemainingSpawnTime(0)
			, m_HalfSpawnArea(0,0,0)
			, m_SpawnSpeed(1)
			, m_SpawnInclination(D3DXToRadian(0))
			, m_SpawnAzimuth(D3DXToRadian(0))
			, m_SpawnLoopTime(10)
			, m_ParticleColorAlpha(255)
			, m_ParticleColorRed(255)
			, m_ParticleColorGreen(255)
			, m_ParticleColorBlue(255)
		{
		}

		virtual ~Emitter(void)
		{
		}

		void Reset(void);

		void Spawn(const Vector3 & Position, const Vector3 & Velocity);

		void Update(double fTime, float fElapsedTime);

		DWORD BuildInstance(
			EmitterInstance * pEmitterInstance,
			double fTime,
			float fElapsedTime);

		void Draw(IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;

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
