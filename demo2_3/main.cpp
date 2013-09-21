
#include "stdafx.h"
#include "Game.h"
//
//// ------------------------------------------------------------------------------------------
//// MyDemo
//// ------------------------------------------------------------------------------------------
//
//class MyDemo
//	: public my::DxutApp
//	, public my::ResourceMgr
//{
//	class AnimationMgr
//	{
//	public:
//		float m_currentTime;
//
//		std::string m_currentAnim;
//
//		typedef std::map<std::string, std::pair<float, std::string> > AnimTimeMap;
//
//		AnimTimeMap m_animTimeMap;
//
//		my::OgreSkeletonAnimationPtr m_skeleton;
//
//		my::BoneList m_animPose;
//
//		my::BoneList m_incrementedPose;
//
//		my::BoneList m_hierarchyBoneList;
//
//		my::BoneList m_hierarchyBoneList2;
//
//		my::TransformList m_dualQuaternionList;
//
//		AnimationMgr(my::OgreSkeletonAnimationPtr skeleton)
//			: m_currentTime(0)
//			, m_skeleton(skeleton)
//		{
//		}
//
//		void SetAnimationTime(const std::string & anim, float time, const std::string & next_anim)
//		{
//			//_ASSERT(m_animTimeMap.end() == m_animTimeMap.find(anim));
//
//			m_animTimeMap[anim] = std::make_pair(time, next_anim);
//
//			if(m_currentAnim.empty())
//			{
//				m_currentAnim = anim;
//			}
//		}
//
//		void AddAnimationTime(float fElapsedTime)
//		{
//			AnimTimeMap::const_iterator anim_time_iter = m_animTimeMap.find(m_currentAnim);
//
//			_ASSERT(m_animTimeMap.end() != anim_time_iter);
//
//			float time = m_currentTime + fElapsedTime;
//			if(time < anim_time_iter->second.first)
//			{
//				m_currentTime = time;
//			}
//			else
//			{
//				m_currentAnim = anim_time_iter->second.second;
//				m_currentTime = 0;
//				AddAnimationTime(time - anim_time_iter->second.first);
//			}
//		}
//
//		void OnFrameMove(double fTime, float fElapsedTime)
//		{
//			AddAnimationTime(fElapsedTime);
//
//			// ��ȡҶ�ӽ��
//			int root_i = m_skeleton->GetBoneIndex("jack_loBackA");
//			my::BoneHierarchy boneHierarchy;
//			boneHierarchy.resize(m_skeleton->m_boneHierarchy.size());
//			m_skeleton->BuildLeafedHierarchy(boneHierarchy, root_i);
//
//			// ��ȡ��ǰ����
//			m_animPose.clear();
//			m_animPose.resize(m_skeleton->m_boneBindPose.size());
//			m_skeleton->BuildAnimationPose(
//				m_animPose, boneHierarchy, root_i, m_currentAnim, m_currentTime);
//
//			// ����ǰ�����Ͱ󶨶�������
//			m_incrementedPose.clear();
//			m_incrementedPose.resize(m_skeleton->m_boneBindPose.size());
//			m_animPose.Increment(
//				m_incrementedPose, m_skeleton->m_boneBindPose, m_skeleton->m_boneHierarchy, root_i);
//
//			// Ϊ�󶨶������ɲ�λ��Ĺ����б��б����ӹ��������ݽ������������ı任��
//			m_hierarchyBoneList.clear();
//			m_hierarchyBoneList.resize(m_skeleton->m_boneBindPose.size());
//			m_skeleton->m_boneBindPose.BuildHierarchyBoneList(
//				m_hierarchyBoneList, m_skeleton->m_boneHierarchy, root_i);
//
//			// ΪĿ�궯�����ɲ�λ��Ĺ����б�
//			m_hierarchyBoneList2.clear();
//			m_hierarchyBoneList2.resize(m_skeleton->m_boneBindPose.size());
//			m_incrementedPose.BuildHierarchyBoneList(
//				m_hierarchyBoneList2, m_skeleton->m_boneHierarchy, root_i);
//
//			// ���󶨶�����Ŀ�궯���Ĺ����б�����˫��Ԫʽ���󶨶���������Ҫ��任��˫��Ԫʽ����д���
//			m_dualQuaternionList.clear();
//			m_dualQuaternionList.resize(m_skeleton->m_boneBindPose.size());
//			m_hierarchyBoneList.BuildDualQuaternionList(
//				m_dualQuaternionList, m_hierarchyBoneList2);
//		}
//	};
//
//	typedef boost::shared_ptr<AnimationMgr> AnimationMgrPtr;
//
//protected:
//	my::ModelViewerCamera m_camera;
//
//	my::OgreMeshPtr m_characterMesh;
//
//	my::Texture2DPtr m_characterTexture;
//
//	my::EffectPtr m_characterEffect;
//
//	AnimationMgrPtr m_characterAnimMgr;
//
//	my::OgreMeshPtr m_sceneMesh;
//
//	my::Texture2DPtr m_sceneTexture;
//
//	static const unsigned int SHADOWMAP_SIZE = 1024;
//
//	my::Texture2DPtr m_shadowMapRT;
//
//	my::SurfacePtr m_shadowMapDS;
//
//	boost::shared_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
//
//	boost::shared_ptr<btCollisionDispatcher> m_dispatcher;
//
//	boost::shared_ptr<btBroadphaseInterface> m_overlappingPairCache;
//
//	boost::shared_ptr<btSequentialImpulseConstraintSolver> m_solver;
//
//	boost::shared_ptr<btCollisionShape> m_groundShape;
//
//	boost::shared_ptr<btMotionState> m_groundMotionState;
//
//	boost::shared_ptr<btRigidBody> m_groundBody;
//
//	boost::shared_ptr<btCollisionShape> m_sphereShape;
//
//	boost::shared_ptr<btMotionState> m_sphereMotionState;
//
//	boost::shared_ptr<btRigidBody> m_sphereBody;
//
//	// dynamic worldӦ�������������������֮ǰ���٣������������⴦��һ��
//	boost::shared_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
//
//	HRESULT OnCreateDevice(
//		IDirect3DDevice9 * pd3dDevice,
//		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
//	{
//		// ������Դ��ȡ·��
//		RegisterFileDir(".");
//		RegisterZipArchive("data.zip");
//		RegisterFileDir("..\\demo2_3");
//		RegisterZipArchive("..\\demo2_3\\data.zip");
//
//		if(FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
//		{
//			return hr;
//		}
//
//		// ��ʼ�����
//		m_camera.m_Rotation = my::Vector3(0,D3DXToRadian(180),0);
//		m_camera.m_LookAt = my::Vector3(0,0,0);
//		m_camera.m_Distance = 30;
//
//		// ��ʼ����ɫ��Դ
//		m_characterMesh = LoadMesh("jack_hres_all.mesh.xml");
//		m_characterTexture = LoadTexture("jack_texture.jpg");
//		m_characterEffect = LoadEffect("SkinedMesh+ShadowMap.fx");
//		m_characterAnimMgr.reset(new AnimationMgr(LoadSkeleton("jack_anim_stand.skeleton.xml")));
//		m_characterAnimMgr->SetAnimationTime("clip1", m_characterAnimMgr->m_skeleton->GetAnimation("clip1").m_time, "clip2");
//		m_characterAnimMgr->SetAnimationTime("clip2", m_characterAnimMgr->m_skeleton->GetAnimation("clip2").m_time, "clip1");
//		m_characterAnimMgr->SetAnimationTime("clip3", m_characterAnimMgr->m_skeleton->GetAnimation("clip3").m_time, "clip4");
//		m_characterAnimMgr->SetAnimationTime("clip4", m_characterAnimMgr->m_skeleton->GetAnimation("clip4").m_time, "clip3");
//		m_characterAnimMgr->m_currentAnim = "clip1";
//		m_characterAnimMgr->m_currentTime = 0.0f;
//
//		// ��ʼ��������Դ
//		m_sceneMesh = LoadMesh("scene.mesh.xml");
//		m_sceneTexture = LoadTexture("scene.texture.jpg");
//
//		// ��ʼ���������漰�����Դ
//		m_dynamicsWorld.reset();
//		m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
//
//		m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration.get()));
//
//		m_overlappingPairCache.reset(new btDbvtBroadphase());
//
//		m_solver.reset(new btSequentialImpulseConstraintSolver());
//
//		m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(
//			m_dispatcher.get(), m_overlappingPairCache.get(), m_solver.get(), m_collisionConfiguration.get()));
//
//		m_dynamicsWorld->setGravity(btVector3(0,-10,0));
//
//		// �����������
//		const my::Vector3 boxHalfExtents(50.0f, 50.0f, 50.0f);
//		m_groundShape.reset(new btBoxShape(btVector3(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z)));
//
//		btTransform transform;
//		transform.setIdentity();
//		transform.setOrigin(btVector3(0, -boxHalfExtents.y, 0));
//		m_groundMotionState.reset(new btDefaultMotionState(transform));
//
//		btVector3 localInertia(0, 0, 0);
//		m_groundBody.reset(new btRigidBody(
//			btRigidBody::btRigidBodyConstructionInfo(0.0f, m_groundMotionState.get(), m_groundShape.get(), localInertia)));
//		m_groundBody->setRestitution(1.0f);
//
//		// �����������壨���ڽ�ɫ��ײ��
//		const float sphereRadius = 5.0f;
//		m_sphereShape.reset(new btSphereShape(sphereRadius));
//
//		transform.setOrigin(btVector3(0, 10, 0));
//		m_sphereMotionState.reset(new btDefaultMotionState(transform));
//
//		const float sphereMass = 3.0f / 4.0f * 3.14159f * 1.0f;
//		m_sphereShape->calculateLocalInertia(sphereMass, localInertia);
//		m_sphereBody.reset(new btRigidBody(
//			btRigidBody::btRigidBodyConstructionInfo(sphereMass, m_sphereMotionState.get(), m_sphereShape.get(), localInertia)));
//		m_sphereBody->setRestitution(0.0f);
//
//		// ������ͽ�ɫ�����������
//		m_dynamicsWorld->addRigidBody(m_groundBody.get());
//		m_dynamicsWorld->addRigidBody(m_sphereBody.get());
//
//		//THROW_CUSEXCEPTION(_T("aaa"));
//
//		return S_OK;
//	}
//
//	HRESULT OnResetDevice(
//		IDirect3DDevice9 * pd3dDevice,
//		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
//	{
//		if(FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
//		{
//			return hr;
//		}
//
//		// �������������ͶӰ
//		float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
//		m_camera.m_Aspect = fAspectRatio;
//
//		// ��������shadow map��render target��ʹ��D3DXCreateTexture����Ϊ��֧���豸����������ͼ
//		m_shadowMapRT.reset(new my::Texture2D());
//		m_shadowMapRT->CreateAdjustedTexture(
//			pd3dDevice,
//			SHADOWMAP_SIZE,
//			SHADOWMAP_SIZE,
//			1,
//			D3DUSAGE_RENDERTARGET,
//			D3DFMT_R32F,
//			D3DPOOL_DEFAULT);
//
//		// ��������shadow map��depth scentil
//		m_shadowMapDS.reset(new my::Surface());
//		m_shadowMapDS->CreateDepthStencilSurface(
//			pd3dDevice,
//			SHADOWMAP_SIZE,
//			SHADOWMAP_SIZE,
//			D3DFMT_D24X8);
//
//		return S_OK;
//	}
//
//	void OnLostDevice(void)
//	{
//		// �����ﴦ����reset�д�������Դ
//		m_shadowMapRT.reset();
//		m_shadowMapDS.reset();
//
//		ResourceMgr::OnLostDevice();
//	}
//
//	void OnDestroyDevice(void)
//	{
//		ResourceMgr::OnDestroyDevice();
//	}
//
//	void OnFrameMove(
//		double fTime,
//		float fElapsedTime)
//	{
//		my::DxutApp::OnFrameMove(fTime, fElapsedTime);
//
//		// ��������³���
//		btTransform transform;
//		m_sphereMotionState->getWorldTransform(transform);
//		const btVector3 & origin = transform.getOrigin();
//		m_camera.m_LookAt = my::Vector3(origin[0], origin[1] + 20.0f, origin[2]);
//		m_camera.OnFrameMove(fTime, fElapsedTime);
//
//		// ���¹�������
//		m_characterAnimMgr->OnFrameMove(fTime, fElapsedTime);
//
//		// ������������
//		m_dynamicsWorld->stepSimulation(fElapsedTime, 10);
//	}
//
//	void OnFrameRender(
//		IDirect3DDevice9 * pd3dDevice,
//		double fTime,
//		float fElapsedTime)
//	{
//		V(pd3dDevice->Clear(
//			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));
//
//		// ������ͶӰ����
//		my::Matrix4 mWorld = my::Matrix4::Identity();
//		my::Matrix4 mViewProj = m_camera.m_ViewProj;
//
//		// �����������ı任
//		btTransform transform;
//		m_groundMotionState->getWorldTransform(transform);
//		const btVector3 & ground_origin = transform.getOrigin();
//		const btMatrix3x3 & ground_basis = transform.getBasis();
//
//		my::Matrix4 mGroundLocal(
//			ground_basis[0][0], ground_basis[0][1], ground_basis[0][2], 0,
//			ground_basis[1][0], ground_basis[1][1], ground_basis[1][2], 0,
//			ground_basis[2][0], ground_basis[2][1], ground_basis[2][2], 0,
//			ground_origin[0], ground_origin[1], ground_origin[2], 1);
//
//		// �����ɫ��ı任
//		m_sphereMotionState->getWorldTransform(transform);
//		const btVector3 & sphere_origin = transform.getOrigin();
//		const btMatrix3x3 & sphere_basis = transform.getBasis();
//
//		my::Vector4 vSpherePos(sphere_origin[0], sphere_origin[1], sphere_origin[2], 1);
//
//		my::Matrix4 mSphereLocal(
//			sphere_basis[0][0], sphere_basis[0][1], sphere_basis[0][2], 0,
//			sphere_basis[1][0], sphere_basis[1][1], sphere_basis[1][2], 0,
//			sphere_basis[2][0], sphere_basis[2][1], sphere_basis[2][2], 0,
//			sphere_origin[0], sphere_origin[1], sphere_origin[2], 1);
//
//		my::Matrix4 mSphereWorld(mSphereLocal * mWorld);
//
//		my::Matrix4 mCharacterLocal = my::Matrix4::Translation(my::Vector3(0.0f, -5.0f, 0.0f));
//
//		// �����Դλ�ã�������������λ����
//		my::Vector3 vLightPosWS = my::Vector3(30, 30, -30);
//
//		// ������յ�͸�ӱ任
//		my::Matrix4 mViewLight(my::Matrix4::LookAtRH(
//			my::Vector3(vLightPosWS.x, vLightPosWS.y, vLightPosWS.z),
//			my::Vector3(mSphereWorld._41, mSphereWorld._42, mSphereWorld._43),
//			my::Vector3(0.0f, 1.0f, 0.0f)));
//		my::Matrix4 mProjLight(my::Matrix4::PerspectiveFovRH(D3DXToRadian(75.0f), 1.0f, 1.0f, 200.0f));
//		my::Matrix4 mViewProjLight = mViewLight * mProjLight;
//
//		// ��shadow map��Ϊrender target��ע�Ᵽ��ָ�ԭ����render target
//		HRESULT hr;
//		CComPtr<IDirect3DSurface9> oldRt;
//		V(pd3dDevice->GetRenderTarget(0, &oldRt));
//		V(pd3dDevice->SetRenderTarget(0, m_shadowMapRT->GetSurfaceLevel(0)));
//		CComPtr<IDirect3DSurface9> oldDs = NULL;
//		V(pd3dDevice->GetDepthStencilSurface(&oldDs));
//		V(pd3dDevice->SetDepthStencilSurface(m_shadowMapDS->m_ptr));
//		V(pd3dDevice->Clear(
//			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
//		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
//		{
//			// ����d3dx effect����
//			m_characterEffect->SetMatrix("g_mWorldViewProjectionLight", mCharacterLocal * mSphereLocal * mWorld * mViewProjLight);
//			m_characterEffect->SetTechnique("RenderShadow");
//
//			// ��ɫ����Ҫ��ʹ��˫��Ԫʽ�б�
//			m_characterEffect->SetMatrixArray("g_dualquat", &m_characterAnimMgr->m_dualQuaternionList[0], m_characterAnimMgr->m_dualQuaternionList.size());
//
//			// ��Ⱦģ�͵��������֣�ע�⣬ͷ���Ĳ��ֲ�Ҫ�����޳�
//			UINT cPasses = m_characterEffect->Begin();
//			for(UINT p = 0; p < cPasses; ++p)
//			{
//				m_characterEffect->BeginPass(p);
//				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
//				m_characterMesh->DrawSubset(1);
//				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
//				m_characterMesh->DrawSubset(0);
//				m_characterEffect->EndPass();
//			}
//			m_characterEffect->End();
//
//			V(pd3dDevice->EndScene());
//		}
//		V(pd3dDevice->SetRenderTarget(0, oldRt));
//		V(pd3dDevice->SetDepthStencilSurface(oldDs));
//		oldRt.Release();
//		oldDs.Release();
//
//		// �����汳����depth stencil
//		V(pd3dDevice->Clear(
//			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));
//
//		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
//		{
//			UINT cPasses;
//			//// ��Ⱦ�������
//			//m_wireEffect->SetMatrix("g_mWorldViewProjection", mGroundLocal * mWorld * mViewProj);
//			//m_wireEffect->SetMatrix("g_mWorld", mGroundLocal * mWorld);
//			//m_wireEffect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//
//			//m_characterEffect->SetTechnique("RenderScene");
//			//cPasses = m_wireEffect->Begin();
//			//for(UINT p = 0; p < cPasses; ++p)
//			//{
//			//	m_wireEffect->BeginPass(p);
//			//	m_groundMesh->DrawSubset(0);
//			//	m_wireEffect->EndPass();
//			//}
//			//m_wireEffect->End();
//
//			//// ��Ⱦ��ɫ��
//			//m_wireEffect->SetMatrix("g_mWorldViewProjection", mSphereLocal * mWorld * mViewProj);
//			//m_wireEffect->SetMatrix("g_mWorld", mSphereLocal * mWorld);
//			//m_wireEffect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//
//			//m_characterEffect->SetTechnique("RenderScene");
//			//cPasses = m_wireEffect->Begin();
//			//for(UINT p = 0; p < cPasses; ++p)
//			//{
//			//	m_wireEffect->BeginPass(p);
//			//	m_sphereMesh->DrawSubset(0);
//			//	m_wireEffect->EndPass();
//			//}
//			//m_wireEffect->End();
//
//			// ��Ⱦ��ɫģ��
//			m_characterEffect->SetMatrix("g_mWorldViewProjection", mCharacterLocal * mSphereLocal * mWorld * mViewProj);
//			m_characterEffect->SetMatrix("g_mWorld", mCharacterLocal * mSphereLocal * mWorld);
//			m_characterEffect->SetFloat("g_fTime", (float)fTime);
//
//			m_characterEffect->SetVector("g_MaterialAmbientColor", my::Vector4(0.27f, 0.27f, 0.27f, 1.0f));
//			m_characterEffect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//			m_characterEffect->SetTexture("g_MeshTexture", m_characterTexture->m_ptr);
//			m_characterEffect->SetVector("g_LightPos", vLightPosWS);
//			m_characterEffect->SetVector("g_LightDiffuse", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//
//			// ��ɫ����Ҫ��ʹ��˫��Ԫʽ�б�
//			m_characterEffect->SetMatrixArray("g_dualquat", &m_characterAnimMgr->m_dualQuaternionList[0], m_characterAnimMgr->m_dualQuaternionList.size());
//
//			m_characterEffect->SetTexture("g_ShadowTexture", m_shadowMapRT->m_ptr);
//			m_characterEffect->SetMatrix("g_mWorldViewProjectionLight", mCharacterLocal * mSphereLocal * mWorld * mViewProjLight);
//			m_characterEffect->SetTechnique("RenderCharacter");
//			cPasses = m_characterEffect->Begin();
//			for(UINT p = 0; p < cPasses; ++p)
//			{
//				// ��Ⱦģ�͵��������֣�ע�⣬ͷ���Ĳ��ֲ�Ҫ�����޳�
//				m_characterEffect->BeginPass(p);
//				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
//				m_characterMesh->DrawSubset(1);
//				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
//				m_characterMesh->DrawSubset(0);
//				m_characterEffect->EndPass();
//			}
//			m_characterEffect->End();
//
//			// ��Ⱦ����
//			m_characterEffect->SetMatrix("g_mWorldViewProjection", mWorld * mViewProj);
//			m_characterEffect->SetMatrix("g_mWorld", mWorld);
//			m_characterEffect->SetFloat("g_fTime", (float)fTime);
//
//			m_characterEffect->SetVector("g_MaterialAmbientColor", my::Vector4(0.27f, 0.27f, 0.27f, 1.0f));
//			m_characterEffect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//			m_characterEffect->SetTexture("g_MeshTexture", m_sceneTexture->m_ptr);
//			m_characterEffect->SetVector("g_LightPos", vLightPosWS);
//			m_characterEffect->SetVector("g_LightDiffuse", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//
//			m_characterEffect->SetTexture("g_ShadowTexture", m_shadowMapRT->m_ptr);
//			m_characterEffect->SetMatrix("g_mWorldViewProjectionLight", mWorld * mViewProjLight);
//			m_characterEffect->SetTechnique("RenderScene");
//			cPasses = m_characterEffect->Begin();
//			for(UINT p = 0; p < cPasses; ++p)
//			{
//				m_characterEffect->BeginPass(p);
//				m_sceneMesh->DrawSubset(0);
//				m_characterEffect->EndPass();
//			}
//			m_characterEffect->End();
//
//			V(pd3dDevice->EndScene());
//		}
//	}
//
//	LRESULT MsgProc(
//		HWND hWnd,
//		UINT uMsg,
//		WPARAM wParam,
//		LPARAM lParam,
//		bool * pbNoFurtherProcessing)
//	{
//		// �����Ϣ����
//		LRESULT lr;
//		if(lr = m_camera.MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
//		{
//			return lr;
//		}
//
//		return 0;
//	}
//};

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	// ����crtdbg�����ڴ�й©
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Game().Run();
}
