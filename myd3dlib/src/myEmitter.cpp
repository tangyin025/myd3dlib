#include "StdAfx.h"
#include "myEmitter.h"

using namespace my;

void Emitter::Reset(void)
{
	m_ParticleList.clear();
}

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity)
{
	ParticlePtr particle(new Particle());
	particle->setPosition(Position);
	particle->setVelocity(Velocity);
	m_ParticleList.push_back(std::make_pair(particle, 0.0f));
}

void Emitter::Update(double fTime, float fElapsedTime)
{
	ParticlePtrPairList::reverse_iterator part_iter = m_ParticleList.rbegin();
	for(; part_iter != m_ParticleList.rend(); part_iter++)
	{
		if((part_iter->second += fElapsedTime) < m_ParticleLifeTime)
		{
			part_iter->first->integrate(fElapsedTime);
		}
		else
		{
			m_ParticleList.erase(m_ParticleList.begin(), m_ParticleList.begin() + (m_ParticleList.rend() - part_iter));
			break;
		}
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
		// ! Can optimize, because all point offset are constant
		unsigned char * pInstance = pInstances + pEmitterInstance->m_InstanceStride * i;
		pEmitterInstance->m_VertexElemSet.SetPosition(pInstance, m_ParticleList[i].first->getPosition(), 1, 0);

		pEmitterInstance->m_VertexElemSet.SetColor(pInstance, D3DCOLOR_ARGB(
			m_ParticleColorAlpha.Interpolate(m_ParticleList[i].second),
			m_ParticleColorRed.Interpolate(m_ParticleList[i].second),
			m_ParticleColorGreen.Interpolate(m_ParticleList[i].second),
			m_ParticleColorBlue.Interpolate(m_ParticleList[i].second)), 1, 0);

		pEmitterInstance->m_VertexElemSet.SetCustomType(pInstance, 1, D3DDECLUSAGE_TEXCOORD, 1, Vector4(
			m_ParticleSizeX.Interpolate(m_ParticleList[i].second),
			m_ParticleSizeY.Interpolate(m_ParticleList[i].second),
			m_ParticleAngle.Interpolate(m_ParticleList[i].second), 1));

		unsigned int AnimFrame = (unsigned int)(m_ParticleList[i].second * m_ParticleAnimFPS) % ((unsigned int)m_ParticleAnimColumn * m_ParticleAnimRow);
		pEmitterInstance->m_VertexElemSet.SetCustomType(pInstance, 1, D3DDECLUSAGE_TEXCOORD, 2, (unsigned int)D3DCOLOR_ARGB(
			0, 0, AnimFrame / m_ParticleAnimRow, AnimFrame % m_ParticleAnimColumn));
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

	DWORD ParticleCount = BuildInstance(pEmitterInstance, fTime, fElapsedTime);

	pEmitterInstance->DrawInstance(pd3dDevice, ParticleCount);
}

void AutoSpawnEmitter::Update(double fTime, float fElapsedTime)
{
	Emitter::Update(fTime, fElapsedTime);

	m_Time += fElapsedTime;

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	while(m_RemainingSpawnTime >= m_SpawnInterval)
	{
		Spawn(
			Vector3(
				Random(m_Position.x - m_HalfSpawnArea.x, m_Position.x + m_HalfSpawnArea.x),
				Random(m_Position.y - m_HalfSpawnArea.y, m_Position.y + m_HalfSpawnArea.y),
				Random(m_Position.z - m_HalfSpawnArea.z, m_Position.z + m_HalfSpawnArea.z)),

			Vector3::SphericalToCartesian(Vector3(
				m_SpawnSpeed,
				m_SpawnInclination.Interpolate(fmod(m_Time, m_SpawnLoopTime)),
				m_SpawnAzimuth.Interpolate(fmod(m_Time, m_SpawnLoopTime)))).transform(m_Orientation));

		m_RemainingSpawnTime -= m_SpawnInterval;

		if((m_ParticleList.back().second += m_RemainingSpawnTime) < m_ParticleLifeTime)
		{
			m_ParticleList.back().first->integrate(m_RemainingSpawnTime);
		}
		else
		{
			m_ParticleList.pop_back();
		}
	}
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

void EmitterInstance::DrawInstance(IDirect3DDevice9 * pd3dDevice, DWORD NumInstances)
{
	HRESULT hr;
	V(pd3dDevice->SetStreamSource(0, m_VertexBuffer.m_ptr, 0, m_VertexStride));
	V(pd3dDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));

	V(pd3dDevice->SetStreamSource(1, m_InstanceData.m_ptr, 0, m_InstanceStride));
	V(pd3dDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

	V(pd3dDevice->SetVertexDeclaration(m_Decl));
	V(pd3dDevice->SetIndices(m_IndexData.m_ptr));
	V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));

	V(pd3dDevice->SetStreamSourceFreq(0,1));
	V(pd3dDevice->SetStreamSourceFreq(1,1));
}
