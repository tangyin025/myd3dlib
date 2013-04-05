#include "StdAfx.h"
#include "myEmitter.h"

using namespace my;

void Emitter::Reset(void)
{
	m_ParticleList.clear();

	m_Time = 0;
}

void Emitter::Spawn(void)
{
	ParticlePtr particle(new Particle());
	particle->setVelocity(Vector3(Random(0.0f,5.0f), Random(0.0f,5.0f), Random(0.0f,5.0f)));
	m_ParticleList.push_back(particle);
}

void Emitter::Update(double fTime, float fElapsedTime)
{
	m_Time += fElapsedTime;

	int Count = (int)(m_Time * m_Rate);

	ParticlePtrList::iterator part_iter = m_ParticleList.begin();
	for(; part_iter != m_ParticleList.end(); part_iter++)
	{
		(*part_iter)->integrate(fElapsedTime);
	}

	for(int i = m_ParticleList.size(); i < Count; i++)
	{
		Spawn();
	}
}

EmitterInstance::SingleInstance * SingleInstance<EmitterInstance>::s_ptr = NULL;

HRESULT EmitterInstance::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);

	return S_OK;
}

HRESULT EmitterInstance::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);

	m_VertexBuffer.CreateVertexBuffer(pd3dDevice, m_VertexElemSet.CalculateVertexStride(0) * 6);

	m_InstanceData.CreateVertexBuffer(pd3dDevice, m_VertexElemSet.CalculateVertexStride(1) * 1024);

	return S_OK;
}

void EmitterInstance::OnLostDevice(void)
{
	m_VertexBuffer.OnDestroyDevice();
	m_InstanceData.OnDestroyDevice();
}

void EmitterInstance::OnDestroyDevice(void)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);
}
