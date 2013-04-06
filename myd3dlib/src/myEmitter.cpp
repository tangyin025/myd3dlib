#include "StdAfx.h"
#include "myEmitter.h"

using namespace my;

void Emitter::Reset(void)
{
	m_ParticleList.clear();
}

void Emitter::Spawn(void)
{
	ParticlePtr particle(new Particle());
	particle->setPosition(Vector3(0,0,0));
	particle->setVelocity(Vector3(Random(-5.0f,5.0f),Random(-5.0f,5.0f),Random(-5.0f,5.0f)));
	m_ParticleList.push_back(particle);
}

void Emitter::Update(double fTime, float fElapsedTime)
{
	m_RemainingSpawnTime += fElapsedTime;

	ParticlePtrList::iterator part_iter = m_ParticleList.begin();
	for(; part_iter != m_ParticleList.end(); part_iter++)
	{
		(*part_iter)->integrate(fElapsedTime);
	}

	_ASSERT(m_InverseRate > 0);
	while(m_RemainingSpawnTime >= m_InverseRate)
	{
		Spawn();
		m_RemainingSpawnTime -= m_InverseRate;
	}

	float TotalTime = m_ParticleList.size() * m_InverseRate + m_RemainingSpawnTime;
	if(TotalTime >= m_ParticleLifeTime + m_InverseRate)
	{
		float OverTime = TotalTime - m_ParticleLifeTime;
		size_t remove_count = Min((size_t)(OverTime / m_InverseRate), m_ParticleList.size());
		m_ParticleList.erase(m_ParticleList.begin(), m_ParticleList.begin() + remove_count);
	}
}

DWORD Emitter::BuildInstance(
	EmitterInstance * pEmitterInstance,
	double fTime,
	float fElapsedTime)
{
	DWORD ParticleCount = Min(4096u, m_ParticleList.size());

	unsigned char * pInstances =
		(unsigned char *)pEmitterInstance->m_InstanceData.Lock(0, pEmitterInstance->m_InstanceStride * ParticleCount);
	_ASSERT(pInstances);
	for(DWORD i = 0; i < ParticleCount; i++)
	{
		unsigned char * pInstance = pInstances + pEmitterInstance->m_InstanceStride * i;
		pEmitterInstance->m_VertexElemSet.SetPosition(pInstance, m_ParticleList[i]->getPosition(), 1, 0);
		pEmitterInstance->m_VertexElemSet.SetColor(pInstance, D3DCOLOR_ARGB(255,255,255,255), 1, 0);
	}
	pEmitterInstance->m_InstanceData.Unlock();

	return ParticleCount;
}

void Emitter::RenderInstance(
	IDirect3DDevice9 * pd3dDevice,
	EmitterInstance * pEmitterInstance,
	DWORD ParticleCount)
{
	V(pd3dDevice->SetStreamSource(0, pEmitterInstance->m_VertexBuffer.m_ptr, 0, pEmitterInstance->m_VertexStride));
	V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | ParticleCount));

	V(pd3dDevice->SetStreamSource(1, pEmitterInstance->m_InstanceData.m_ptr, 0, pEmitterInstance->m_InstanceStride));
	V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

	V(pd3dDevice->SetVertexDeclaration(pEmitterInstance->m_Decl));
	V(pd3dDevice->SetIndices(pEmitterInstance->m_IndexData.m_ptr));
	V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));

	V(pd3dDevice->SetStreamSourceFreq(0,1));
	V(pd3dDevice->SetStreamSourceFreq(1,1));
}

void Emitter::Draw(IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	EmitterInstance * pEmitterInstance = EmitterInstance::getSingletonPtr();
	_ASSERT(pEmitterInstance);

	DWORD ParticleCount = BuildInstance(pEmitterInstance, fTime, fElapsedTime);

	RenderInstance(pd3dDevice, pEmitterInstance, ParticleCount);
}

EmitterInstance::SingleInstance * SingleInstance<EmitterInstance>::s_ptr = NULL;

HRESULT EmitterInstance::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_IndexData.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);

	m_Decl = m_VertexElemSet.CreateVertexDeclaration(pd3dDevice);

	return S_OK;
}

HRESULT EmitterInstance::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);

	m_VertexBuffer.CreateVertexBuffer(pd3dDevice, m_VertexStride * 4);
	unsigned char * pVertices = (unsigned char *)m_VertexBuffer.Lock(0, m_VertexStride * 4);
	m_VertexElemSet.SetTexcoord(pVertices + m_VertexStride * 0, Vector2(0,0), 0, 0);
	m_VertexElemSet.SetTexcoord(pVertices + m_VertexStride * 1, Vector2(1,0), 0, 0);
	m_VertexElemSet.SetTexcoord(pVertices + m_VertexStride * 2, Vector2(1,1), 0, 0);
	m_VertexElemSet.SetTexcoord(pVertices + m_VertexStride * 3, Vector2(0,1), 0, 0);
	m_VertexBuffer.Unlock();

	m_IndexData.CreateIndexBuffer(pd3dDevice, sizeof(DWORD) * 4);
	DWORD * pIndices = (DWORD *)m_IndexData.Lock(0, sizeof(DWORD) * 4);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 3;
	m_IndexData.Unlock();

	m_InstanceData.CreateVertexBuffer(pd3dDevice, m_InstanceStride * 4096u);

	return S_OK;
}

void EmitterInstance::OnLostDevice(void)
{
	m_VertexBuffer.OnDestroyDevice();
	m_IndexData.OnDestroyDevice();
	m_InstanceData.OnDestroyDevice();
}

void EmitterInstance::OnDestroyDevice(void)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_IndexData.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);

	m_Decl.Release();
}
