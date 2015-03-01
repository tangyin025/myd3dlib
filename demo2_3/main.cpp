#include "stdafx.h"
#include "Game.h"
//#include "Logic/MeshComponent.h"
#include "Logic/Logic.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
{
public:
	//// ========================================================================================================
	//// 骨骼动画
	//// ========================================================================================================
	//SkeletonMeshComponentPtr m_skel_mesh;
	//OgreSkeletonAnimationPtr m_skel_anim;
	//BoneList m_skel_pose;
	//BoneList m_skel_pose_heir1;
	//BoneList m_skel_pose_heir2;

	//MeshInstancePtr m_mesh_ins;
	MeshComponentPtr m_mesh_ins;
	EmitterMeshComponentPtr m_emitter;

	//// ========================================================================================================
	//// 大场景
	//// ========================================================================================================
	//OgreMeshSetPtr m_meshSet;

	// ========================================================================================================
	// 布料系统
	// ========================================================================================================
	std::vector<PxClothParticle> m_cloth_particles;
	MeshComponentPtr m_cloth_mesh;
	MeshAnimatorPtr m_cloth_mesh_anim;
	//OgreSkeletonAnimationPtr m_cloth_anim;
	//CachePtr m_cloth_mesh_vertices;
	//BoneList m_cloth_pose;
	//BoneList m_cloth_pose_heir1;
	//BoneList m_cloth_pose_heir2;
	//TransformList m_cloth_duals;
	//PxCloth * m_cloth;

	//DeformationMeshComponentPtr m_deform_mesh;

	//// ========================================================================================================
	//// 逻辑系统
	//// ========================================================================================================
	//LogicPtr m_Logic;

	void OnKeyDown(my::InputEventArg * arg)
	{
		KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
		Vector3 scale, pos; Quaternion rot;
		m_cloth_mesh->m_World.Decompose(scale, rot, pos);
		switch (karg.kc)
		{
		case VK_UP:
			pos.x += 1;
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		case VK_DOWN:
			pos.x -= 1;
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		case VK_LEFT:
			rot *= Quaternion::RotationAxis(Vector3(1,0,0), D3DXToRadian(30));
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		case VK_RIGHT:
			rot *= Quaternion::RotationAxis(Vector3(1,0,0), D3DXToRadian(-30));
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		}
	}

	Demo::Demo(void)
		//: m_Logic(new Logic)
	{
	}

	void DrawTextAtWorld(
		const Vector3 & pos,
		LPCWSTR lpszText,
		D3DCOLOR Color,
		Font::Align align = Font::AlignCenterMiddle)
	{
		const Vector3 ptProj = pos.transformCoord(m_Camera->m_ViewProj);
		if(ptProj.z > 0.0f && ptProj.z < 1.0f)
		{
			const Vector2 vp = DialogMgr::GetDlgViewport();
			const Vector2 ptVp(Lerp(0.0f, vp.x, (ptProj.x + 1) / 2), Lerp(0.0f, vp.y, (1 - ptProj.y) / 2));
			m_Font->DrawString(m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);
		}
	}

	MeshComponent::LODPtr CreateMeshComponentLOD(MeshComponent * owner, OgreMeshPtr mesh, bool cloth)
	{
		MeshComponent::LODPtr lod;
		if (cloth)
		{
			lod.reset(new ClothMeshComponentLOD(owner));
		}
		else
		{
			lod.reset(new MeshComponent::LOD(owner));
		}
		lod->m_Mesh = mesh;
		std::vector<std::string>::const_iterator mat_name_iter = lod->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != lod->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			lod->m_Materials.push_back(LoadMaterial(str_printf("material/%s.xml", mat_name_iter->c_str())));
		}
		return lod;
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

		ExecuteCode("dofile \"Hud.lua\"");

		// ========================================================================================================
		// 骨骼动画
		// ========================================================================================================
		//SaveSimplyMesh("aaa.mesh.xml", LoadMesh("mesh/sportive03_f.mesh.xml"), 400);
		//SaveSimplyMesh("bbb.mesh.xml", LoadMesh("mesh/sportive03_f.mesh.xml"), 40);

		//m_skel_mesh.reset(new SkeletonMeshComponent(AABB(-1,-1,-1,1,1,1)));
		//MeshComponent::LODPtr lod(new MeshComponent::LOD);
		//lod->m_Mesh = LoadMesh("mesh/sportive03_f.mesh.xml");
		//std::vector<std::string>::const_iterator mat_name_iter = lod->m_Mesh->m_MaterialNameList.begin();
		//for(; mat_name_iter != lod->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		//{
		//	lod->m_Materials.push_back(LoadMaterial(str_printf("material/%s.xml", mat_name_iter->c_str())));
		//}
		//m_skel_mesh->m_lods.push_back(lod);
		//m_skel_mesh->m_World = Matrix4::Scaling(0.05f,0.05f,0.05f);
		//m_skel_mesh->m_Animator.reset(new MeshAnimator());
		//m_skel_anim = LoadSkeleton("mesh/sportive03_f.skeleton.xml");

		//m_mesh_ins = LoadMesh("mesh/tube.mesh.xml");
		//m_mesh_ins->CreateInstance(pd3dDevice);
		m_mesh_ins.reset(new MeshComponent(AABB(-1,1)));
		m_mesh_ins->m_lods.push_back(CreateMeshComponentLOD(m_mesh_ins.get(), LoadMesh("mesh/tube.mesh.xml"), false));

		m_emitter.reset(new EmitterMeshComponent(AABB(-1,1)));
		m_emitter->m_Emitter = LoadEmitter("emitter/emitter_01.xml");
		m_emitter->m_Material = LoadMaterial("material/lambert1.xml");

		//// ========================================================================================================
		//// 大场景
		//// ========================================================================================================
		//m_meshSet = LoadMeshSet("mesh/scene.mesh.xml");
		//OgreMeshSet::iterator mesh_iter = m_meshSet->begin();
		//for(; mesh_iter != m_meshSet->end(); mesh_iter++)
		//{
		//	m_OctScene->PushComponent(CreateMeshComponent(*mesh_iter), 0.1f);
		//}

		// ========================================================================================================
		// 布料系统
		// ========================================================================================================
		PxClothCollisionSphere spheres[2] =
		{
			{PxVec3(-5.0f, 0.0f, 0.0f), 3.f},
			{PxVec3( 5.0f, 0.0f, 0.0f), 3.f}
		};
		// A tapered capsule
		PxU32 capsulePairs[] = { 0, 1 };
		PxClothCollisionData collisionData;
		collisionData.spheres = spheres;
		collisionData.numSpheres = 0;
		collisionData.pairIndexBuffer = capsulePairs;
		collisionData.numPairs = 0;

		PxClothCollisionPlane p;
		p.normal = PxVec3(0.0f, 1.0f, 0.0f);
		p.distance = 0.0f;

		//m_cloth_anim = LoadSkeleton("mesh/cloth.skeleton.xml");
		m_cloth_mesh_anim.reset(new SimpleMeshAnimator());
		m_cloth_mesh_anim->m_Animation = LoadSkeleton("mesh/cloth.skeleton.xml");

		m_cloth_mesh.reset(new MeshComponent(AABB(-1,1)));
		m_cloth_mesh->m_lods.push_back(CreateMeshComponentLOD(m_cloth_mesh.get(), LoadMesh("mesh/cloth.mesh.xml"), true));
		dynamic_pointer_cast<ClothMeshComponentLOD>(m_cloth_mesh->m_lods[0])->CreateCloth(this,
			m_cloth_mesh_anim->m_Animation->m_boneHierarchy,
			m_cloth_mesh_anim->m_Animation->GetBoneIndex("joint5"),
			PxClothCollisionData());
		//m_cloth_mesh_vertices.reset(new Cache(
		//	m_cloth_mesh->m_lods[0]->m_Mesh->GetNumVertices() * m_cloth_mesh->m_lods[0]->m_Mesh->GetNumBytesPerVertex()));
		//memcpy(&(*m_cloth_mesh_vertices)[0], m_cloth_mesh->m_lods[0]->m_Mesh->LockVertexBuffer(), m_cloth_mesh_vertices->size());
		//m_cloth_mesh->m_lods[0]->m_Mesh->UnlockVertexBuffer();
		//InitClothParticles(
		//	m_cloth_particles,
		//	m_cloth_mesh->m_lods[0]->m_Mesh,
		//	m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset,
		//	m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Offset,
		//	m_cloth_anim->m_boneHierarchy,
		//	m_cloth_anim->GetBoneIndex("joint5"));
		//my::MemoryOStreamPtr ofs(new my::MemoryOStream);
		//CookClothFabric(ofs, m_cloth_mesh->m_lods[0]->m_Mesh, m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset);
		//IStreamPtr ifs(new MemoryIStream(&(*ofs->m_cache)[0], ofs->m_cache->size()));
		//physx_ptr<PxClothFabric> clothFabric(CreateClothFabric(ifs));
		//m_cloth = m_sdk->createCloth(PxTransform(PxVec3(0,0,0), PxQuat(0,0,0,1)), *clothFabric, &m_cloth_particles[0], collisionData, PxClothFlags());
		//m_cloth->addCollisionPlane(p);
		//m_cloth->addCollisionConvex(0x01);
		//m_cloth->setFrictionCoefficient(1.0f);
		m_PxScene->addActor(*dynamic_pointer_cast<ClothMeshComponentLOD>(m_cloth_mesh->m_lods[0])->m_cloth);

		//// 创建物理地面，但是布料不参与碰撞
		//m_PxScene->addActor(*PxCreateStatic(*m_sdk, PxTransform(PxQuat(PxHalfPi, PxVec3(0,0,1))), PxPlaneGeometry(), *m_PxMaterial));

		//m_deform_mesh.reset(new DeformationMeshComponent(AABB(-1,-1,-1,1,1,1)));
		//OgreMeshPtr tmp = LoadMesh("mesh/sportive03_f.mesh.xml");
		//m_deform_mesh->CreateFromOgreMeshWithoutMaterials(pd3dDevice, tmp);
		//std::vector<std::string>::const_iterator mat_name_iter = tmp->m_MaterialNameList.begin();
		//for(; mat_name_iter != tmp->m_MaterialNameList.end(); mat_name_iter++)
		//{
		//	m_deform_mesh->m_Materials.push_back(LoadMaterial(str_printf("material/%s.xml", mat_name_iter->c_str())));
		//}
		//m_deform_mesh->m_World = Matrix4::Scaling(0.05f,0.05f,0.05f);
		//AddResource("___trwrwr342423", m_deform_mesh);

		//// ========================================================================================================
		//// 逻辑系统
		//// ========================================================================================================
		//m_Logic->Create();

		ExecuteCode("dofile \"StateMain.lua\"");

		Game::getSingleton().m_KeyPressedEvent = boost::bind(&Demo::OnKeyDown, this, _1);

		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}
		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		Game::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		//// 注意顺序
		//m_Logic->Destroy();
		m_cloth_mesh.reset();

		Game::OnDestroyDevice();
	}

	virtual void OnPxThreadSubstep(float fElapsedTime)
	{
		// ========================================================================================================
		// 布料系统
		// ========================================================================================================
		//static float anim_time = 0.5f;
		////anim_time = fmod(anim_time + fElapsedTime, m_cloth_anim->GetAnimation("clip1").GetTime());
		//m_cloth_pose.resize(m_cloth_anim->m_boneBindPose.size());
		//m_cloth_anim->BuildAnimationPose(
		//	m_cloth_pose,
		//	m_cloth_anim->m_boneHierarchy,
		//	m_cloth_anim->GetBoneIndex("joint1"),
		//	"clip1",
		//	anim_time);
		//m_cloth_pose_heir1.clear();
		//m_cloth_pose_heir1.resize(m_cloth_anim->m_boneBindPose.size());
		//m_cloth_anim->m_boneBindPose.BuildHierarchyBoneList(
		//	m_cloth_pose_heir1,
		//	m_cloth_anim->m_boneHierarchy,
		//	m_cloth_anim->GetBoneIndex("joint1"));
		//m_cloth_pose_heir2.clear();
		//m_cloth_pose_heir2.resize(m_cloth_anim->m_boneBindPose.size());
		//m_cloth_pose.BuildHierarchyBoneList(
		//	m_cloth_pose_heir2,
		//	m_cloth_anim->m_boneHierarchy,
		//	m_cloth_anim->GetBoneIndex("joint1"));
		//m_cloth_duals.clear();
		//m_cloth_duals.resize(m_cloth_anim->m_boneBindPose.size());
		//m_cloth_pose_heir1.BuildDualQuaternionList(m_cloth_duals, m_cloth_pose_heir2);
		m_cloth_mesh_anim->Update(fElapsedTime);

		//static std::vector<Vector3> vertices;
		//vertices.resize(m_cloth_mesh->m_lods[0]->m_Mesh->GetNumVertices());
		//unsigned char * pVertices = (unsigned char *)&(*m_cloth_mesh_vertices)[0];
		//for (unsigned int i = 0; i < vertices.size(); i++)
		//{
		//	void * pVertex = pVertices + i * m_cloth_mesh->m_lods[0]->m_Mesh->GetNumBytesPerVertex();
		//	BoneList::TransformVertexWithDualQuaternionList(
		//		vertices[i],
		//		m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.GetPosition(pVertex),
		//		m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.GetBlendIndices(pVertex),
		//		m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.GetBlendWeight(pVertex),
		//		m_cloth_duals);
		//}
		//UpdateClothParticles(m_cloth, (unsigned char *)&vertices[0], 0, sizeof(vertices[0]));

		//ReadClothParticles(m_cloth_mesh->m_lods[0]->m_Mesh,
		//	m_cloth_mesh->m_lods[0]->m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset, m_cloth);
		//PxTransform Trans = m_cloth->getGlobalPose();
		//m_cloth_mesh->m_World = Matrix4::Compose(Vector3(1,1,1),(Quaternion&)Trans.q, (Vector3&)Trans.p);
		dynamic_pointer_cast<ClothMeshComponentLOD>(m_cloth_mesh->m_lods[0])->UpdateCloth(m_cloth_mesh_anim->m_DualQuats);
		//PxTransform pose = pose = dynamic_pointer_cast<ClothMeshComponentLOD>(m_cloth_mesh->m_lods[0])->m_cloth->getGlobalPose();
		//m_cloth_mesh->m_World = Matrix4::Compose(Vector3(1,1,1), (Quaternion &)pose.q, (Vector3 &)pose.p);
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		if (m_Camera)
		{
			m_Camera->OnFrameMove(fTime, fElapsedTime);
		}

		m_ScrInfos[0] = str_printf(L"%.2f", m_fFps);

		//// ========================================================================================================
		//// 骨骼动画
		//// ========================================================================================================
		//static float anim_time = 0;
		//anim_time = fmod(anim_time + fElapsedTime, m_skel_anim->GetAnimation("walk").GetTime());
		//m_skel_pose.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_anim->BuildAnimationPose(
		//	m_skel_pose,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("Bip01"),
		//	"walk",
		//	anim_time);
		//m_skel_pose[m_skel_anim->GetBoneIndex("Bip01")].m_position.z = 0; // 固定根节点的z轴移动
		//m_skel_pose_heir1.clear();
		//m_skel_pose_heir1.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_anim->m_boneBindPose.BuildHierarchyBoneList(
		//	m_skel_pose_heir1,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("Bip01"));
		//m_skel_pose_heir2.clear();
		//m_skel_pose_heir2.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_pose.BuildHierarchyBoneList(
		//	m_skel_pose_heir2,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("Bip01"));
		//m_skel_mesh->m_Animator->m_DualQuats.clear();
		//m_skel_mesh->m_Animator->m_DualQuats.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_pose_heir1.BuildDualQuaternionList(m_skel_mesh->m_Animator->m_DualQuats, m_skel_pose_heir2);

		if (m_emitter->m_Emitter)
			m_emitter->m_Emitter->Update(fTime, fElapsedTime);

		//// ========================================================================================================
		//// 逻辑系统
		//// ========================================================================================================
		//m_Logic->Update(fElapsedTime);
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		PushGrid();

		//// ========================================================================================================
		//// 骨骼动画
		//// ========================================================================================================
		//m_skel_mesh->QueryMesh(this, RenderPipeline::DrawStageCBuffer);

		//Matrix4 * mat = m_mesh_ins->LockInstanceData(2);
		//mat[0] = Matrix4::Translation(10,0,0);
		//mat[1] = Matrix4::Translation(-10,0,0);
		//m_mesh_ins->UnlockInstanceData();

		//m_SimpleSampleInst->SetTexture("g_MeshTexture", m_TexChecker);
		//m_SimpleSampleInst->SetMatrix("g_World", Matrix4::identity);
		//m_SimpleSampleInst->SetTechnique("RenderScene");
		//UINT passes = m_SimpleSampleInst->Begin(0);
		//for (UINT p = 0; p < passes; p++)
		//{
		//	m_SimpleSampleInst->BeginPass(p);
		//	m_mesh_ins->DrawSubsetInstance(0, 2);
		//	m_SimpleSampleInst->EndPass();
		//}
		//m_SimpleSampleInst->End();
		m_mesh_ins->QueryMesh(this, RenderPipeline::DrawStageCBuffer);
		m_emitter->QueryMesh(this,  RenderPipeline::DrawStageCBuffer);

		// ========================================================================================================
		// 布料系统
		// ========================================================================================================
		m_cloth_mesh->QueryMesh(this, RenderPipeline::DrawStageCBuffer);
		//m_SimpleSampleSkel->SetTechnique("RenderScene");
		//UINT passes = m_SimpleSampleSkel->Begin(0);
		//m_SimpleSampleSkel->SetTexture("g_MeshTexture", m_TexChecker);
		//m_SimpleSampleSkel->SetMatrixArray("g_dualquat", &m_cloth_duals[0], m_cloth_duals.size());
		//for (UINT p = 0; p < passes; p++)
		//{
		//	m_SimpleSampleSkel->BeginPass(p);
		//	m_cloth_mesh->m_lods[0]->m_Mesh->DrawSubset(0);
		//	m_SimpleSampleSkel->EndPass();
		//}
		//m_SimpleSampleSkel->End();

		//m_deform_mesh->QueryMesh(this, RenderPipeline::DrawStageCBuffer);

		Game::OnFrameRender(pd3dDevice, fTime, fElapsedTime);
	}

	virtual void OnUIRender(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime)
	{
		// 绘制坐标
		DrawTextAtWorld(Vector3(12,0,0), L"x", D3DCOLOR_ARGB(255,255,255,0));
		DrawTextAtWorld(Vector3(0,0,12), L"z", D3DCOLOR_ARGB(255,255,255,0));

		Game::OnUIRender(ui_render, fTime, fElapsedTime);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT lr;
		if (m_Camera && (lr = m_Camera->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)))
		{
			return lr;
		}

		if(lr = Game::MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
		{
			return lr;
		}

		switch(uMsg)
		{
		case WM_RBUTTONUP:
			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			std::pair<Vector3, Vector3> ray = m_Camera->CalculateRay(
				Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());
			break;
		}
		return 0;
	}
};

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	// 设置crtdbg监视内存泄漏
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}
