#pragma once

#include "mySingleton.h"
#include "myPhysics.h"
#include "myMesh.h"

namespace my
{
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

		void Update(double fTime, float fElapsedTime);
	};

	typedef boost::shared_ptr<Emitter> EmitterPtr;

	class EmitterInstance
		: public SingleInstance<EmitterInstance>
	{
	public:
		CComPtr<IDirect3DDevice9> m_Device;

		D3DVERTEXELEMENT9Set m_VertexElemSet;

		CComPtr<IDirect3DVertexBuffer9> m_VertexBuffer;

		CComPtr<IDirect3DVertexBuffer9> m_InstanceData;

	public:
		EmitterInstance(void)
		{
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, 0, 0));
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreatePositionElement(1, 0, 0));
			WORD offset = sizeof(D3DVERTEXELEMENT9Set::PositionType);
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateColorElement(1, offset, 0));
			offset += sizeof(D3DVERTEXELEMENT9Set::ColorType);
			m_VertexElemSet.insert(D3DVERTEXELEMENT9Set::CreateCustomElement(1, D3DDECLUSAGE_TEXCOORD, 1, offset, D3DDECLTYPE_SHORT2));
			offset += sizeof(short) * 2;
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
