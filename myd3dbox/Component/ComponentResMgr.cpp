#include "StdAfx.h"
#include "ComponentResMgr.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <fstream>

using namespace my;

void ComponentResMgr::OnMaterialDiffuseTextureLoaded(
	boost::weak_ptr<Material> weak_mat_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	MaterialPtr mat_ptr = weak_mat_ptr.lock();
	if (mat_ptr)
	{
		mat_ptr->m_DiffuseTexture.second = boost::dynamic_pointer_cast<BaseTexture>(res);
	}
}

void ComponentResMgr::OnMeshComponentMaterialLoaded(
	boost::weak_ptr<MeshComponent> weak_lod_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId,
	bool bInstance)
{
	MeshComponentPtr cmp_ptr = weak_lod_ptr.lock();
	if (cmp_ptr)
	{
		if (cmp_ptr->m_MaterialList.size() <= AttribId)
		{
			cmp_ptr->m_MaterialList.resize(AttribId + 1);
		}
		cmp_ptr->m_MaterialList[AttribId] = boost::dynamic_pointer_cast<Material>(res);

		if (!cmp_ptr->m_MaterialList[AttribId]->m_DiffuseTexture.first.empty())
		{
			LoadTextureAsync(cmp_ptr->m_MaterialList[AttribId]->m_DiffuseTexture.first,
				boost::bind(&ComponentResMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_MaterialList[AttribId], _1));
		}
	}
}

void ComponentResMgr::OnMeshComponentMeshLoaded(
	boost::weak_ptr<MeshComponent> weak_lod_ptr,
	my::DeviceRelatedObjectBasePtr res,
	bool bInstance)
{
	MeshComponentPtr cmp_ptr = weak_lod_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
		cmp_ptr->m_MaterialList.resize(cmp_ptr->m_Mesh->m_MaterialNameList.size());
		cmp_ptr->m_bInstance = bInstance;
		for (unsigned int i = 0; i < cmp_ptr->m_Mesh->m_MaterialNameList.size(); i++)
		{
			LoadMaterialAsync(str_printf("material/%s.xml", cmp_ptr->m_Mesh->m_MaterialNameList[i].c_str()),
				boost::bind(&ComponentResMgr::OnMeshComponentMaterialLoaded, this, cmp_ptr, _1, i, bInstance));
		}
	}
}

void ComponentResMgr::OnEmitterComponentEmitterLoaded(
	boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	EmitterMeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Emitter = boost::dynamic_pointer_cast<Emitter>(res);
	}
}

void ComponentResMgr::OnEmitterComponentMaterialLoaded(
	boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	EmitterMeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Material = boost::dynamic_pointer_cast<Material>(res);

		if (!cmp_ptr->m_Material->m_DiffuseTexture.first.empty())
		{
			LoadTextureAsync(cmp_ptr->m_Material->m_DiffuseTexture.first,
				boost::bind(&ComponentResMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_Material, _1));
		}
	}
}

void ComponentResMgr::OnClothComponentMaterialLoaded(
	boost::weak_ptr<ClothComponent> weak_lod_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId)
{
	ClothComponentPtr cmp_ptr = weak_lod_ptr.lock();
	if (cmp_ptr)
	{
		if (cmp_ptr->m_MaterialList.size() <= AttribId)
		{
			cmp_ptr->m_MaterialList.resize(AttribId + 1);
		}
		cmp_ptr->m_MaterialList[AttribId] = boost::dynamic_pointer_cast<Material>(res);

		if (!cmp_ptr->m_MaterialList[AttribId]->m_DiffuseTexture.first.empty())
		{
			LoadTextureAsync(cmp_ptr->m_MaterialList[AttribId]->m_DiffuseTexture.first,
				boost::bind(&ComponentResMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_MaterialList[AttribId], _1));
		}
	}
}

void ComponentResMgr::OnClothComponentMeshLoaded(
	boost::weak_ptr<ClothComponent> weak_lod_ptr,
	my::DeviceRelatedObjectBasePtr res,
	boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
	boost::shared_ptr<my::BoneHierarchy> hierarchy,
	DWORD root_i,
	boost::shared_ptr<PxClothCollisionData> collData)
{
	ClothComponentPtr cmp_ptr = weak_lod_ptr.lock();
	if (cmp_ptr)
	{
		OgreMeshPtr mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
		cmp_ptr->m_MaterialList.resize(mesh->m_MaterialNameList.size());
		for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
		{
			LoadMaterialAsync(str_printf("material/%s.xml", mesh->m_MaterialNameList[i].c_str()),
				boost::bind(&ComponentResMgr::OnClothComponentMaterialLoaded, this, cmp_ptr, _1, i));
		}

		if (cmp_ptr->m_VertexData.empty())
		{
			cmp_ptr->m_VertexStride = mesh->GetNumBytesPerVertex();
			cmp_ptr->m_VertexData.resize(mesh->GetNumVertices() * cmp_ptr->m_VertexStride);
			memcpy(&cmp_ptr->m_VertexData[0], mesh->LockVertexBuffer(), cmp_ptr->m_VertexData.size());
			mesh->UnlockVertexBuffer();

			cmp_ptr->m_IndexData.resize(mesh->GetNumFaces() * 3);
			if (cmp_ptr->m_IndexData.size() > USHRT_MAX)
			{
				THROW_CUSEXCEPTION(str_printf("create deformation mesh with overflow index size %u", cmp_ptr->m_IndexData.size()));
			}
			VOID * pIndices = mesh->LockIndexBuffer();
			for (unsigned int face_i = 0; face_i < mesh->GetNumFaces(); face_i++)
			{
				if(mesh->GetOptions() & D3DXMESH_32BIT)
				{
					cmp_ptr->m_IndexData[face_i * 3 + 0] = (WORD)*((DWORD *)pIndices + face_i * 3 + 0);
					cmp_ptr->m_IndexData[face_i * 3 + 1] = (WORD)*((DWORD *)pIndices + face_i * 3 + 1);
					cmp_ptr->m_IndexData[face_i * 3 + 2] = (WORD)*((DWORD *)pIndices + face_i * 3 + 2);
				}
				else
				{
					cmp_ptr->m_IndexData[face_i * 3 + 0] = *((WORD *)pIndices + face_i * 3 + 0);
					cmp_ptr->m_IndexData[face_i * 3 + 1] = *((WORD *)pIndices + face_i * 3 + 1);
					cmp_ptr->m_IndexData[face_i * 3 + 2] = *((WORD *)pIndices + face_i * 3 + 2);
				}
			}
			mesh->UnlockIndexBuffer();

			cmp_ptr->m_AttribTable = mesh->m_AttribTable;
			std::vector<D3DVERTEXELEMENT9> velist(MAX_FVF_DECL_SIZE);
			mesh->GetDeclaration(&velist[0]);
			HRESULT hr;
			if (FAILED(hr = mesh->GetDevice()->CreateVertexDeclaration(&velist[0], &cmp_ptr->m_Decl)))
			{
				THROW_D3DEXCEPTION(hr);
			}
		}

		cmp_ptr->m_VertexElems = mesh->m_VertexElems;

		cmp_ptr->m_particles.resize(mesh->GetNumVertices());
		unsigned char * pVertices = (unsigned char *)&cmp_ptr->m_VertexData[0];
		for(unsigned int i = 0; i < cmp_ptr->m_particles.size(); i++) {
			unsigned char * pVertex = pVertices + i * cmp_ptr->m_VertexStride;
			cmp_ptr->m_particles[i].pos = (PxVec3 &)cmp_ptr->m_VertexElems.GetPosition(pVertex);
			unsigned char * pIndices = (unsigned char *)&cmp_ptr->m_VertexElems.GetBlendIndices(pVertex);
			BOOST_STATIC_ASSERT(4 == my::D3DVertexElementSet::MAX_BONE_INDICES);
			cmp_ptr->m_particles[i].invWeight = (
				pIndices[0] == root_i || hierarchy->HaveChild(root_i, pIndices[0]) ||
				pIndices[1] == root_i || hierarchy->HaveChild(root_i, pIndices[1]) ||
				pIndices[2] == root_i || hierarchy->HaveChild(root_i, pIndices[2]) ||
				pIndices[3] == root_i || hierarchy->HaveChild(root_i, pIndices[3])) ? 1 / 1.0f : 0.0f;
		}

		my::MemoryOStreamPtr ofs(new my::MemoryOStream);
		CookClothFabric(PxContext.get<0>(), Vector3(0.0f, -9.81f, 0.0f),
			ofs, mesh, cmp_ptr->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset);

		my::IStreamPtr ifs(new my::MemoryIStream(&(*ofs->m_cache)[0], ofs->m_cache->size()));
		PxClothFabric * fabric = CreateClothFabric(PxContext.get<1>(), ifs);
		cmp_ptr->m_Cloth = PxContext.get<1>()->createCloth(
			PxTransform(PxVec3(0,0,0), PxQuat(0,0,0,1)), *fabric, &cmp_ptr->m_particles[0], *collData, PxClothFlags()); // ! fabric->release()

		PxContext.get<2>()->addActor(*cmp_ptr->m_Cloth);
	}
}

class PhysXOStream : public PxOutputStream
{
public:
	my::OStreamPtr ostream;

	PhysXOStream(my::OStreamPtr _ostream)
		: ostream(_ostream)
	{
	}

	virtual PxU32 write(const void* src, PxU32 count)
	{
		return ostream->write(src, count);
	}
};

class PhysXIStream : public PxInputStream
{
public:
	my::IStreamPtr istream;

	PhysXIStream(my::IStreamPtr _istream)
		: istream(_istream)
	{
	}

	virtual PxU32 read(void* dest, PxU32 count)
	{
		return istream->read(dest, count);
	}
};

void ComponentResMgr::CookTriangleMesh(PxCooking * Cooking, my::OStreamPtr ostream, my::MeshPtr mesh)
{
	PxTriangleMeshDesc desc;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();
	desc.points.data = mesh->LockVertexBuffer();
	desc.triangles.count = mesh->GetNumFaces();
	if (mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= PxMeshFlag::e16_BIT_INDICES;
	}
	desc.triangles.data = mesh->LockIndexBuffer();
	desc.materialIndices.stride = sizeof(DWORD);
	desc.materialIndices.data = (PxMaterialTableIndex *)mesh->LockAttributeBuffer();
	Cooking->cookTriangleMesh(desc, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
	mesh->UnlockAttributeBuffer();
}

void ComponentResMgr::CookTriangleMeshToFile(PxCooking * Cooking, std::string path, my::MeshPtr mesh)
{
	CookTriangleMesh(Cooking, my::FileOStream::Open(ms2ts(path).c_str()), mesh);
}

PxTriangleMesh * ComponentResMgr::CreateTriangleMesh(PxPhysics * sdk, my::IStreamPtr istream)
{
	// ! should be call at resource thread
	PxTriangleMesh * ret = sdk->createTriangleMesh(PhysXIStream(istream));
	return ret;
}

void ComponentResMgr::CookClothFabric(PxCooking * Cooking, const my::Vector3 & Gravity, my::OStreamPtr ostream, my::MeshPtr mesh, WORD PositionOffset)
{
	PxClothMeshDesc desc;
	desc.points.data = (unsigned char *)mesh->LockVertexBuffer() + PositionOffset;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();

	desc.triangles.data = mesh->LockIndexBuffer();
	desc.triangles.count = mesh->GetNumFaces();
	if (mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= PxMeshFlag::e16_BIT_INDICES;
	}

	Cooking->cookClothFabric(desc, (PxVec3&)Gravity, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
}

void ComponentResMgr::CookClothFabricToFile(PxCooking * Cooking, const my::Vector3 & Gravity, std::string path, my::MeshPtr mesh, WORD PositionOffset)
{
	CookClothFabric(Cooking, Gravity, my::FileOStream::Open(ms2ts(path).c_str()), mesh, PositionOffset);
}

PxClothFabric * ComponentResMgr::CreateClothFabric(PxPhysics * sdk, my::IStreamPtr istream)
{
	PxClothFabric * ret = sdk->createClothFabric(PhysXIStream(istream));
	return ret;
}

class MaterialIORequest : public IORequest
{
public:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	MaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if (callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if (m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
			try
			{
				MaterialPtr res;
				membuf mb((char *)&(*m_cache)[0], m_cache->size());
				std::istream ims(&mb);
				boost::archive::xml_iarchive ia(ims);
				ia >> boost::serialization::make_nvp("Material", res);
				m_res = res;
			}
			catch (...)
			{
			}
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_res)
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
	}
};

void ComponentResMgr::LoadMaterialAsync(const std::string & path, const my::ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MaterialIORequest(callback, path, this)), false);
}

boost::shared_ptr<Material> ComponentResMgr::LoadMaterial(const std::string & path)
{
	return LoadResource<Material>(path, IORequestPtr(new MaterialIORequest(ResourceCallback(), path, this)));
}

void ComponentResMgr::SaveMaterial(const std::string & path, boost::shared_ptr<Material> material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Material", material);
}

MeshComponentPtr ComponentResMgr::CreateMeshComponentFromFile(const std::string & path, bool bInstance)
{
	MeshComponentPtr ret(new MeshComponent());
	LoadMeshAsync(path, boost::bind(&ComponentResMgr::OnMeshComponentMeshLoaded, this, ret, _1, bInstance));
	return ret;
}

EmitterMeshComponentPtr ComponentResMgr::CreateEmitterComponentFromFile(const std::string & path)
{
	EmitterMeshComponentPtr ret(new EmitterMeshComponent());
	LoadEmitterAsync(path, boost::bind(&ComponentResMgr::OnEmitterComponentEmitterLoaded, this, ret, _1));
	LoadMaterialAsync("material/lambert1.xml", boost::bind(&ComponentResMgr::OnEmitterComponentMaterialLoaded, this, ret, _1));
	return ret;
}

ClothComponentPtr ComponentResMgr::CreateClothComponentFromFile(
	boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
	const std::string & path,
	const my::BoneHierarchy & hierarchy,
	DWORD root_i,
	const PxClothCollisionData& collData)
{
	ClothComponentPtr ret(new ClothComponent());
	LoadMeshAsync(path, boost::bind(&ComponentResMgr::OnClothComponentMeshLoaded, this, ret, _1, PxContext,
		boost::shared_ptr<my::BoneHierarchy>(new my::BoneHierarchy(hierarchy)),
		root_i,
		boost::shared_ptr<PxClothCollisionData>(new PxClothCollisionData(collData))));
	return ret;
}

FMOD::Sound * ComponentResMgr::CreateFModSound(FMOD::System * system, const std::string & path, FMOD_MODE mode)
{
	if (CheckPath(path))
	{
		CachePtr m_cache = OpenIStream(path)->GetWholeCache();
		FMOD_CREATESOUNDEXINFO info = {0};
		info.cbsize = sizeof(info);
		info.length = m_cache->size();
		FMOD_RESULT result;
		FMOD::Sound * sound = NULL;
		result = system->createSound((char *)&(*m_cache)[0], mode | FMOD_OPENMEMORY, &info, &sound);
		FMOD_ERRCHECK(result);
		return sound;
	}
	return NULL;
}
