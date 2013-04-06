#pragma once

#include "mySingleton.h"
#include "myPhysics.h"
#include "myMesh.h"

namespace my
{
	class EmitterInstance;

	class Emitter
	{
	public:
		float m_Time;

		float m_Rate;

		ParticlePtrList m_ParticleList;

	public:
		Emitter(void)
			: m_Time(0)
			, m_Rate(100)
		{
		}

		virtual ~Emitter(void)
		{
		}

		void Reset(void);

		void Spawn(void);

		void Update(double fTime, float fElapsedTime);

		DWORD BuildInstance(EmitterInstance * pEmitterInstance);

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
	};
}
