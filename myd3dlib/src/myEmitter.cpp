#include "StdAfx.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include "myEmitter.h"

using namespace my;

#define PARTICLE_MAX 4096u

BOOST_CLASS_EXPORT(Emitter)

Emitter::~Emitter(void)
{
}

void Emitter::OnResetDevice(void)
{
}

void Emitter::OnLostDevice(void)
{
}

void Emitter::OnDestroyDevice(void)
{
}

void Emitter::Reset(void)
{
	m_ParticleList.clear();
}

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity)
{
	if(m_ParticleList.size() < PARTICLE_MAX)
	{
		ParticlePtr particle(new Particle());
		particle->setPosition(Position);
		particle->setVelocity(Velocity);
		m_ParticleList.push_back(std::make_pair(particle, 0.0f));
	}
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
	_ASSERT(m_ParticleList.size() <= PARTICLE_MAX);

	unsigned char * pInstances =
		(unsigned char *)pEmitterInstance->m_InstanceData.Lock(0, pEmitterInstance->m_InstanceStride * m_ParticleList.size());
	_ASSERT(pInstances);
	for(DWORD i = 0; i < m_ParticleList.size(); i++)
	{
		// ! Can optimize, because all point offset are constant
		unsigned char * pInstance = pInstances + pEmitterInstance->m_InstanceStride * i;
		pEmitterInstance->m_InstanceElems.SetPosition(pInstance, m_ParticleList[i].first->getPosition());

		pEmitterInstance->m_InstanceElems.SetColor(pInstance, D3DCOLOR_ARGB(
			m_ParticleColorA.Interpolate(m_ParticleList[i].second),
			m_ParticleColorR.Interpolate(m_ParticleList[i].second),
			m_ParticleColorG.Interpolate(m_ParticleList[i].second),
			m_ParticleColorB.Interpolate(m_ParticleList[i].second)));

		pEmitterInstance->m_InstanceElems.SetVertexValue(pInstance, D3DDECLUSAGE_TEXCOORD, 1, Vector4(
			m_ParticleSizeX.Interpolate(m_ParticleList[i].second),
			m_ParticleSizeY.Interpolate(m_ParticleList[i].second),
			m_ParticleAngle.Interpolate(m_ParticleList[i].second), 1));

		unsigned int AnimFrame = (unsigned int)(m_ParticleList[i].second * m_ParticleAnimFPS) % ((unsigned int)m_ParticleAnimColumn * m_ParticleAnimRow);
		pEmitterInstance->m_InstanceElems.SetVertexValue(pInstance, D3DDECLUSAGE_TEXCOORD, 2, (DWORD)D3DCOLOR_ARGB(
			0, 0, AnimFrame / m_ParticleAnimRow, AnimFrame % m_ParticleAnimColumn));
	}
	pEmitterInstance->m_InstanceData.Unlock();

	return m_ParticleList.size();
}

void Emitter::Draw(
	EmitterInstance * pEmitterInstance,
	double fTime,
	float fElapsedTime)
{
	DWORD ParticleCount = BuildInstance(pEmitterInstance, fTime, fElapsedTime);

	pEmitterInstance->SetTexture(m_Texture);

	pEmitterInstance->SetAnimationColumnRow(m_ParticleAnimColumn, m_ParticleAnimRow);

	pEmitterInstance->DrawInstance(ParticleCount);
}

BOOST_CLASS_EXPORT(SphericalEmitter)

void SphericalEmitter::Update(double fTime, float fElapsedTime)
{
	Emitter::Update(fTime, fElapsedTime);

	m_Time += fElapsedTime;

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	Vector3 SpawnPos;
	Quaternion SpawnOri;
	switch(m_WorldType)
	{
	case WorldTypeWorld:
		SpawnPos = m_Position;
		SpawnOri = m_Orientation;
		break;

	default:
		SpawnPos = Vector3::zero;
		SpawnOri = Quaternion::identity;
		break;
	}

	while(m_RemainingSpawnTime >= 0)
	{
		Spawn(
			Vector3(
				Random(SpawnPos.x - m_HalfSpawnArea.x, SpawnPos.x + m_HalfSpawnArea.x),
				Random(SpawnPos.y - m_HalfSpawnArea.y, SpawnPos.y + m_HalfSpawnArea.y),
				Random(SpawnPos.z - m_HalfSpawnArea.z, SpawnPos.z + m_HalfSpawnArea.z)),
			Vector3::SphericalToCartesian(Vector3(
				m_SpawnSpeed,
				m_SpawnInclination.Interpolate(fmod(m_Time, m_SpawnLoopTime)),
				m_SpawnAzimuth.Interpolate(fmod(m_Time, m_SpawnLoopTime)))).transform(SpawnOri));

		m_RemainingSpawnTime -= m_SpawnInterval;
	}
}

EmitterInstance::EmitterInstance(void)
{
	m_VertexElems.InsertTexcoordElement(0);

	m_InstanceElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_InstanceElems.InsertColorElement(offset);
	offset += sizeof(D3DCOLOR);
	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 1);
	offset += sizeof(Vector4);
	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_TEXCOORD, 2);
	offset += sizeof(DWORD);

	m_velist = m_VertexElems.BuildVertexElementList(0);
	std::vector<D3DVERTEXELEMENT9> ielist = m_InstanceElems.BuildVertexElementList(1);
	m_velist.insert(m_velist.end(), ielist.begin(), ielist.end());
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	m_velist.push_back(ve_end);

	m_VertexStride = D3DXGetDeclVertexSize(&m_velist[0], 0);
	m_InstanceStride = D3DXGetDeclVertexSize(&m_velist[0], 1);
}

EmitterInstance::~EmitterInstance(void)
{
}

HRESULT EmitterInstance::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	_ASSERT(!m_Device);

	m_Device = pd3dDevice;

	_ASSERT(!m_VertexBuffer.m_ptr);
	_ASSERT(!m_IndexData.m_ptr);
	_ASSERT(!m_InstanceData.m_ptr);

	HRESULT hr;
	if(FAILED(hr = pd3dDevice->CreateVertexDeclaration(&m_velist[0], &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

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
	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 0, Vector2(0,0));
	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 1, Vector2(1,0));
	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 2, Vector2(1,1));
	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 3, Vector2(0,1));
	m_VertexBuffer.Unlock();

	m_IndexData.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * 4);
	WORD * pIndices = (WORD *)m_IndexData.Lock(0, sizeof(WORD) * 4);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 3;
	m_IndexData.Unlock();

	m_InstanceData.CreateVertexBuffer(pd3dDevice, m_InstanceStride * PARTICLE_MAX);

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

	m_Device.Release();
}

void EmitterInstance::DrawInstance(DWORD NumInstances)
{
	HRESULT hr;
	V(m_Device->SetStreamSource(0, m_VertexBuffer.m_ptr, 0, m_VertexStride));
	V(m_Device->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));

	V(m_Device->SetStreamSource(1, m_InstanceData.m_ptr, 0, m_InstanceStride));
	V(m_Device->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

	V(m_Device->SetVertexDeclaration(m_Decl));
	V(m_Device->SetIndices(m_IndexData.m_ptr));
	V(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));

	V(m_Device->SetStreamSourceFreq(0,1));
	V(m_Device->SetStreamSourceFreq(1,1));
}

void EmitterMgr::Update(
	double fTime,
	float fElapsedTime)
{
	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Update(fTime, fElapsedTime);
	}
}

void EmitterMgr::Draw(
	EmitterInstance * pInstance,
	const Matrix4 & ViewProj,
	const Quaternion & ViewOrientation,
	double fTime,
	float fElapsedTime)
{
	pInstance->SetViewProj(ViewProj);

	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		switch((*emitter_iter)->m_WorldType)
		{
		case Emitter::WorldTypeWorld:
			pInstance->SetWorld(Matrix4::Identity());
			break;

		default:
			pInstance->SetWorld(Matrix4::RotationQuaternion((*emitter_iter)->m_Orientation) * Matrix4::Translation((*emitter_iter)->m_Position));
			break;
		}

		switch((*emitter_iter)->m_DirectionType)
		{
		case Emitter::DirectionTypeCamera:
			pInstance->SetDirection(
				Vector3(0,0,1).transform(ViewOrientation),
				Vector3(0,1,0).transform(ViewOrientation),
				Vector3(1,0,0).transform(ViewOrientation));
			break;

		case Emitter::DirectionTypeVertical:
			{
				Vector3 Up(0,1,0);
				Vector3 Right = Vector3(0,0,-1).transform(ViewOrientation).cross(Up);
				Vector3 Dir = Right.cross(Up);
				pInstance->SetDirection(Dir, Up, Right);
			}
			break;

		case Emitter::DirectionTypeHorizontal:
			pInstance->SetDirection(Vector3(0,1,0), Vector3(0,0,1), Vector3(-1,0,0));
			break;
		}

		(*emitter_iter)->Draw(pInstance, fTime, fElapsedTime);
	}
}

void EmitterMgr::InsertEmitter(EmitterPtr emitter)
{
	_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

	m_EmitterSet.insert(emitter);
}

void EmitterMgr::RemoveEmitter(EmitterPtr emitter)
{
	m_EmitterSet.erase(emitter);
}

void EmitterMgr::RemoveAllEmitter(void)
{
	m_EmitterSet.clear();
}
