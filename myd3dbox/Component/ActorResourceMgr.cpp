#include "StdAfx.h"
#include "ActorResourceMgr.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

using namespace my;

void ActorResourceMgr::OnMaterialDiffuseTextureLoaded(
	boost::weak_ptr<Material> weak_mat_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	MaterialPtr mat_ptr = weak_mat_ptr.lock();
	if (mat_ptr)
	{
		mat_ptr->m_DiffuseTexture.second = boost::dynamic_pointer_cast<BaseTexture>(res);
	}
}

void ActorResourceMgr::OnMeshComponentMaterialLoaded(
	boost::weak_ptr<MeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId,
	bool bInstance)
{
	MeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
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
				boost::bind(&ActorResourceMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_MaterialList[AttribId], _1));
		}
	}
}

void ActorResourceMgr::OnMeshComponentMeshLoaded(
	boost::weak_ptr<MeshComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	bool bInstance)
{
	MeshComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
		cmp_ptr->m_MaterialList.resize(cmp_ptr->m_Mesh->m_MaterialNameList.size());
		cmp_ptr->m_bInstance = bInstance;
		for (unsigned int i = 0; i < cmp_ptr->m_Mesh->m_MaterialNameList.size(); i++)
		{
			MaterialPtr mat = CreateMaterial(str_printf("material/%s.xml", cmp_ptr->m_Mesh->m_MaterialNameList[i].c_str()));
			OnMeshComponentMaterialLoaded(cmp_ptr, mat, i, bInstance);
		}
	}
}

void ActorResourceMgr::OnEmitterComponentEmitterLoaded(
	boost::weak_ptr<EmitterComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	EmitterComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Emitter = boost::dynamic_pointer_cast<Emitter>(res);
		MaterialPtr mat = CreateMaterial(str_printf("material/%s.xml", cmp_ptr->m_Emitter->m_MaterialName.c_str()));
		OnEmitterComponentMaterialLoaded(cmp_ptr, mat);
	}
}

void ActorResourceMgr::OnEmitterComponentMaterialLoaded(
	boost::weak_ptr<EmitterComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res)
{
	EmitterComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		cmp_ptr->m_Material = boost::dynamic_pointer_cast<Material>(res);

		if (!cmp_ptr->m_Material->m_DiffuseTexture.first.empty())
		{
			LoadTextureAsync(cmp_ptr->m_Material->m_DiffuseTexture.first,
				boost::bind(&ActorResourceMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_Material, _1));
		}
	}
}

void ActorResourceMgr::OnClothComponentMaterialLoaded(
	boost::weak_ptr<ClothComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	DWORD AttribId)
{
	ClothComponentPtr cmp_ptr = weak_cmp_ptr.lock();
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
				boost::bind(&ActorResourceMgr::OnMaterialDiffuseTextureLoaded, this, cmp_ptr->m_MaterialList[AttribId], _1));
		}
	}
}

void ActorResourceMgr::OnClothComponentMeshLoaded(
	boost::weak_ptr<ClothComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
	boost::shared_ptr<my::BoneHierarchy> hierarchy,
	DWORD root_i,
	boost::shared_ptr<PxClothCollisionData> collData)
{
	ClothComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		OgreMeshPtr mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
		cmp_ptr->m_MaterialList.resize(mesh->m_MaterialNameList.size());
		for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
		{
			MaterialPtr mat = CreateMaterial(str_printf("material/%s.xml", mesh->m_MaterialNameList[i].c_str()));
			OnClothComponentMaterialLoaded(cmp_ptr, mat, i);
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

void ActorResourceMgr::OnClothComponentSkeletonLoaded(
	boost::weak_ptr<ClothComponent> weak_cmp_ptr,
	my::DeviceRelatedObjectBasePtr res,
	boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
	std::string mesh_path,
	std::string root_name,
	boost::shared_ptr<PxClothCollisionData> collData)
{
	ClothComponentPtr cmp_ptr = weak_cmp_ptr.lock();
	if (cmp_ptr)
	{
		OgreSkeletonAnimationPtr skel = boost::dynamic_pointer_cast<OgreSkeletonAnimation>(res);

		LoadMeshAsync(mesh_path, boost::bind(&ActorResourceMgr::OnClothComponentMeshLoaded, this, cmp_ptr, _1, PxContext,
			boost::shared_ptr<my::BoneHierarchy>(new my::BoneHierarchy(skel->m_boneHierarchy)), skel->GetBoneIndex(root_name), collData));
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

void ActorResourceMgr::CookTriangleMesh(PxCooking * Cooking, my::OStreamPtr ostream, my::MeshPtr mesh)
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

void ActorResourceMgr::CookTriangleMeshToFile(PxCooking * Cooking, std::string path, my::MeshPtr mesh)
{
	CookTriangleMesh(Cooking, my::FileOStream::Open(ms2ts(path).c_str()), mesh);
}

PxTriangleMesh * ActorResourceMgr::CreateTriangleMesh(PxPhysics * sdk, my::IStreamPtr istream)
{
	// ! should be call at resource thread
	PxTriangleMesh * ret = sdk->createTriangleMesh(PhysXIStream(istream));
	return ret;
}

void ActorResourceMgr::CookClothFabric(PxCooking * Cooking, const my::Vector3 & Gravity, my::OStreamPtr ostream, my::MeshPtr mesh, WORD PositionOffset)
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

void ActorResourceMgr::CookClothFabricToFile(PxCooking * Cooking, const my::Vector3 & Gravity, std::string path, my::MeshPtr mesh, WORD PositionOffset)
{
	CookClothFabric(Cooking, Gravity, my::FileOStream::Open(ms2ts(path).c_str()), mesh, PositionOffset);
}

PxClothFabric * ActorResourceMgr::CreateClothFabric(PxPhysics * sdk, my::IStreamPtr istream)
{
	PxClothFabric * ret = sdk->createClothFabric(PhysXIStream(istream));
	return ret;
}

boost::shared_ptr<my::Emitter> ActorResourceMgr::CreateEmitter(const std::string & path)
{
	boost::shared_ptr<my::Emitter> ret;
	if (CheckPath(path))
	{
		CachePtr cache = OpenIStream(path)->GetWholeCache();
		membuf mb((char *)&(*cache)[0], cache->size());
		std::istream istr(&mb);
		boost::archive::xml_iarchive ar(istr);
		ar >> boost::serialization::make_nvp("Emitter", ret);
		return ret;
	}
	return ret;
}

void ActorResourceMgr::SaveEmitter(const std::string & path, boost::shared_ptr<Emitter> emitter)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Emitter", emitter);
}

boost::shared_ptr<Material> ActorResourceMgr::CreateMaterial(const std::string & path)
{
	MaterialPtr ret;
	if (CheckPath(path))
	{
		CachePtr cache = OpenIStream(path)->GetWholeCache();
		membuf mb((char *)&(*cache)[0], cache->size());
		std::istream istr(&mb);
		boost::archive::xml_iarchive ar(istr);
		ar >> boost::serialization::make_nvp("Material", ret);
		return ret;
	}
	return ret;
}

void ActorResourceMgr::SaveMaterial(const std::string & path, boost::shared_ptr<Material> material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Material", material);
}

MeshComponentPtr ActorResourceMgr::AddMeshComponent(Actor * owner, boost::shared_ptr<my::Mesh> mesh, bool bInstance)
{
	MeshComponentPtr ret(new MeshComponent(owner));
	OnMeshComponentMeshLoaded(ret, mesh, bInstance);
	owner->AddComponent(ret);
	return ret;
}

MeshComponentPtr ActorResourceMgr::AddMeshComponentFromFile(Actor * owner, const std::string & path, bool bInstance)
{
	MeshComponentPtr ret(new MeshComponent(owner));
	LoadMeshAsync(path, boost::bind(&ActorResourceMgr::OnMeshComponentMeshLoaded, this, ret, _1, bInstance));
	owner->AddComponent(ret);
	return ret;
}

void ActorResourceMgr::AddMeshComponentList(Actor * owner, boost::shared_ptr<my::OgreMeshSet> mesh_set)
{
	OgreMeshSet::iterator mesh_iter = mesh_set->begin();
	for (; mesh_iter != mesh_set->end(); mesh_iter++)
	{
		MeshComponentPtr ret(new MeshComponent(owner));
		OnMeshComponentMeshLoaded(ret, *mesh_iter, false);
		ret->Min = (*mesh_iter)->m_AABB.Min;
		ret->Max = (*mesh_iter)->m_AABB.Max;
		owner->AddComponent(ret);
	}
}

SkeletonMeshComponentPtr ActorResourceMgr::AddSkeletonMeshComponent(Actor * owner, boost::shared_ptr<my::Mesh> mesh, bool bInstance)
{
	SkeletonMeshComponentPtr ret(new SkeletonMeshComponent(owner));
	OnMeshComponentMeshLoaded(ret, mesh, bInstance);
	owner->AddComponent(ret);
	return ret;
}

SkeletonMeshComponentPtr ActorResourceMgr::AddSkeletonMeshComponentFromFile(Actor * owner, const std::string & path, bool bInstance)
{
	SkeletonMeshComponentPtr ret(new SkeletonMeshComponent(owner));
	LoadMeshAsync(path, boost::bind(&ActorResourceMgr::OnMeshComponentMeshLoaded, this, ret, _1, bInstance));
	owner->AddComponent(ret);
	return ret;
}

EmitterComponentPtr ActorResourceMgr::AddEmitterComponentFromFile(Actor * owner, const std::string & path)
{
	EmitterComponentPtr ret(new EmitterComponent(owner));
	my::EmitterPtr emt = CreateEmitter(path);
	OnEmitterComponentEmitterLoaded(ret, emt);
	owner->AddComponent(ret);
	return ret;
}

ClothComponentPtr ActorResourceMgr::AddClothComponentFromFile(
	Actor * owner,
	boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
	const std::string & mesh_path,
	const std::string & skel_path,
	const std::string & root_name,
	const PxClothCollisionData& collData)
{
	ClothComponentPtr ret(new ClothComponent(owner));
	LoadSkeletonAsync(skel_path, boost::bind(&ActorResourceMgr::OnClothComponentSkeletonLoaded,
		this, ret, _1, PxContext, mesh_path, root_name, boost::shared_ptr<PxClothCollisionData>(new PxClothCollisionData(collData))));
	owner->AddComponent(ret);
	owner->m_Clothes.push_back(ret);
	return ret;
}
