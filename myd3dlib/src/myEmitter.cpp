#include "StdAfx.h"
#include "myEmitter.h"

using namespace my;

void Emitter::Reset(void)
{
	m_ParticleList.clear();

	m_Time = 0;
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
		ParticlePtr particle(new Particle());
		particle->setVelocity(Vector3(Random(0.0f,5.0f), Random(0.0f,5.0f), Random(0.0f,5.0f)));
		m_ParticleList.push_back(particle);
	}
}

EmitterInstance::SingleInstance * SingleInstance<EmitterInstance>::s_ptr = NULL;

HRESULT EmitterInstance::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer);
	_ASSERT(!m_InstanceData);

	m_Device = pd3dDevice;

	return S_OK;
}

HRESULT EmitterInstance::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer);
	_ASSERT(!m_InstanceData);

	_ASSERT(m_Device);

	HRESULT hr;
	if(FAILED(hr = m_Device->CreateVertexBuffer(
		m_VertexElemSet.CalculateVertexStride(0) * 6, 0, 0, D3DPOOL_DEFAULT, &m_VertexBuffer, 0)))
	{
		return hr;
	}

	if(FAILED(hr = m_Device->CreateVertexBuffer(
		m_VertexElemSet.CalculateVertexStride(1) * 1024, 0, 0, D3DPOOL_DEFAULT, &m_InstanceData, 0)))
	{
		return hr;
	}

	return S_OK;
}

void EmitterInstance::OnLostDevice(void)
{
	m_VertexBuffer.Release();
	m_InstanceData.Release();
}

void EmitterInstance::OnDestroyDevice(void)
{
	_ASSERT(!m_VertexBuffer);
	_ASSERT(!m_InstanceData);

	m_Device.Release();
}
