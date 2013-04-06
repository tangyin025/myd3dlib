#include "StdAfx.h"
#include "myEmitter.h"

using namespace my;

void Emitter::Reset(void)
{
	m_ParticleList.clear();

	m_Time = 0;
}

void Emitter::Spawn(const Vector3 & pos)
{
	ParticlePtr particle(new Particle());
	particle->setPosition(pos);
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
		Spawn(Vector3(0,0,0));
	}
}

DWORD Emitter::BuildInstance(EmitterInstance * pEmitterInstance)
{
	DWORD ParticleCount = Min(1024u, m_ParticleList.size());
	unsigned char * pInstances = (unsigned char *)pEmitterInstance->m_InstanceData.Lock(0, pEmitterInstance->m_InstanceStride * ParticleCount);
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

void Emitter::Draw(IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	EmitterInstance * pEmitterInstance = EmitterInstance::getSingletonPtr();
	_ASSERT(pEmitterInstance);

	DWORD ParticleCount = BuildInstance(pEmitterInstance);

	pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | ParticleCount);
	pd3dDevice->SetStreamSource(0, pEmitterInstance->m_VertexBuffer.m_ptr, 0, pEmitterInstance->m_VertexStride);

	pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);
	pd3dDevice->SetStreamSource(1, pEmitterInstance->m_InstanceData.m_ptr, 0, pEmitterInstance->m_InstanceStride);

	pd3dDevice->SetVertexDeclaration(pEmitterInstance->m_Decl);
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

	pd3dDevice->SetStreamSourceFreq(0,1);
	pd3dDevice->SetStreamSourceFreq(1,1);
}

EmitterInstance::SingleInstance * SingleInstance<EmitterInstance>::s_ptr = NULL;

HRESULT EmitterInstance::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_VertexBuffer.m_ptr);
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

	m_InstanceData.CreateVertexBuffer(pd3dDevice, m_InstanceStride * 1024);

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

	m_Decl.Release();
}
