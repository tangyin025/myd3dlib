
#include "myd3dlib.h"
#include <DXUTCamera.h>

// ------------------------------------------------------------------------------------------
// MyDemo
// ------------------------------------------------------------------------------------------

class MyDemo : public my::DxutSample
{
protected:
	CModelViewerCamera m_camera;

	my::EffectPtr m_effect;

	my::MeshPtr m_mesh;

	my::TexturePtr m_texture;

	my::OgreSkeletonAnimationPtr m_skeleton;

	my::BoneList m_animPose;

	my::BoneList m_incrementedPose;

	my::BoneList m_hierarchyBoneList;

	my::BoneList m_hierarchyBoneList2;

	my::TransformList m_inverseTransformList;

	my::TransformList m_TransformList;

	my::TransformList m_dualQuaternionList;

	class AnimationTimeMgr
	{
	public:
		float m_time;

		std::string m_anim;

		typedef std::map<std::string, std::pair<float, std::string> > AnimTimeMap;

		AnimTimeMap m_animTime;

		AnimationTimeMgr(void)
			: m_time(0)
		{
		}

		void SetAnimationTime(const std::string & anim, float time, const std::string & next_anim)
		{
			//_ASSERT(m_animTime.end() == m_animTime.find(anim));

			m_animTime[anim] = std::make_pair(time, next_anim);

			if(m_anim.empty())
			{
				m_anim = anim;
			}
		}

		void AddAnimationTime(float fElapsedTime)
		{
			AnimTimeMap::const_iterator anim_time_iter = m_animTime.find(m_anim);
			_ASSERT(m_animTime.end() != anim_time_iter);

			float time = m_time + fElapsedTime;
			if(time < anim_time_iter->second.first)
			{
				m_time = time;
			}
			else
			{
				m_anim = anim_time_iter->second.second;
				m_time = 0;
				AddAnimationTime(time - anim_time_iter->second.first);
			}
		}
	};

	AnimationTimeMgr m_animTimeMgr;

	HRESULT OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutSample::OnD3D9CreateDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 初始化相机
		D3DXVECTOR3 vecEye(0.0f, 0.0f, -50.0f);
		D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
		m_camera.SetViewParams(&vecEye, &vecAt);
		m_camera.SetModelCenter(D3DXVECTOR3(0.0f, 15.0f, 0.0f));

		// 读取D3DX Effect文件
		my::CachePtr cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream("SkinedMesh.fx"));
		m_effect = my::Effect::CreateEffect(pd3dDevice, &(*cache)[0], cache->size());

		// 读取模型文件
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream("jack_hres_all.mesh.xml"));
		m_mesh = my::OgreMesh::CreateOgreMesh(pd3dDevice, (char *)&(*cache)[0], cache->size(), D3DXMESH_MANAGED);

		// 创建贴图
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream("jack_texture.jpg"));
		m_texture = my::Texture::CreateTextureFromFileInMemory(pd3dDevice, &(*cache)[0], cache->size());

		// 读取骨骼动画
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream("jack_anim_stand.skeleton.xml"));
		m_skeleton = my::OgreSkeletonAnimation::CreateOgreSkeletonAnimation((char *)&(*cache)[0], cache->size());

		// 初始化动画控制器
		m_animTimeMgr.SetAnimationTime("clip1", m_skeleton->GetAnimation("clip1").m_time, "clip2");
		m_animTimeMgr.SetAnimationTime("clip2", m_skeleton->GetAnimation("clip2").m_time, "clip1");
		m_animTimeMgr.SetAnimationTime("clip3", m_skeleton->GetAnimation("clip3").m_time, "clip4");
		m_animTimeMgr.SetAnimationTime("clip4", m_skeleton->GetAnimation("clip4").m_time, "clip3");
		m_animTimeMgr.m_anim = "clip1";
		m_animTimeMgr.m_time = 0.0f;

		return S_OK;
	}

	HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutSample::OnD3D9ResetDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 重新设置相机的投影
		float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
		m_camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
		m_camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

		return S_OK;
	}

	void OnD3D9LostDevice(void)
	{
		DxutSample::OnD3D9LostDevice();
	}

	void OnD3D9DestroyDevice(void)
	{
		DxutSample::OnD3D9DestroyDevice();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		DxutSample::OnFrameMove(fTime, fElapsedTime);

		// 在这里更新场景
		m_camera.FrameMove(fElapsedTime);

		// 获取骨骼动画的某一帧
		//float time = (float)DXUTGetTime();
		//time = fmod(time, m_skeleton->GetAnimation("clip1").m_time);
		m_animTimeMgr.AddAnimationTime(fElapsedTime);
		int root_i = m_skeleton->GetBoneIndex("jack_loBackA");
		m_animPose.clear();
		m_animPose.resize(m_skeleton->m_boneBindPose.size());
		m_skeleton->BuildAnimationPose(m_animPose, root_i, m_animTimeMgr.m_anim, m_animTimeMgr.m_time);

		// 将动画和绑定动作叠加
		m_incrementedPose.clear();
		m_incrementedPose.resize(m_skeleton->m_boneBindPose.size());
		m_animPose.Increment(
			m_incrementedPose, m_skeleton->m_boneBindPose, m_skeleton->m_boneHierarchy, root_i);

		// 计算绑定动作的逆变换
		m_hierarchyBoneList.clear();
		m_hierarchyBoneList.resize(m_skeleton->m_boneBindPose.size());
		m_inverseTransformList.clear();
		m_inverseTransformList.resize(m_skeleton->m_boneBindPose.size());
		//m_skeleton->m_boneBindPose.BuildInverseHierarchyTransformList(
		//	m_inverseTransformList, m_skeleton->m_boneHierarchy, root_i, my::Matrix4::identity);
		m_skeleton->m_boneBindPose.BuildHierarchyBoneList(
			m_hierarchyBoneList, m_skeleton->m_boneHierarchy, root_i);
		//m_hierarchyBoneList.BuildInverseTransformList(m_inverseTransformList);

		// 计算目标动画的正变换，注意，还要加上world空间变换
		my::Matrix4 mWorld = *(my::Matrix4 *)m_camera.GetWorldMatrix();
		my::Vector3 camPos, camScale; my::Quaternion camRot;
		mWorld.Decompose(camScale, camRot, camPos);
		m_hierarchyBoneList2.clear();
		m_hierarchyBoneList2.resize(m_skeleton->m_boneBindPose.size());
		m_TransformList.clear();
		m_TransformList.resize(m_skeleton->m_boneBindPose.size());
		//m_incrementedPose.BuildHierarchyTransformList(
		//	m_TransformList, m_skeleton->m_boneHierarchy, root_i, mWorld);
		m_incrementedPose.BuildHierarchyBoneList(
			m_hierarchyBoneList2, m_skeleton->m_boneHierarchy, root_i, camRot, camPos);
		//m_hierarchyBoneList2.BuildTransformList(m_TransformList);

		// 合并所有的顶点变换，把结果保存到 m_inverseTransformList
		//m_inverseTransformList.TransformSelf(
		//	m_TransformList, m_skeleton->m_boneHierarchy, root_i);
		//m_hierarchyBoneList.IncrementSelf(
		//	m_hierarchyBoneList2, m_skeleton->m_boneHierarchy, root_i).BuildTransformList(m_inverseTransformList);
		m_dualQuaternionList.clear();
		m_dualQuaternionList.resize(m_skeleton->m_boneBindPose.size());
		m_hierarchyBoneList.BuildDualQuaternionList(
			m_dualQuaternionList, m_hierarchyBoneList2);
	}

	void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		// 获得相机投影矩阵
		my::Matrix4 mWorld = *(my::Matrix4 *)m_camera.GetWorldMatrix();
		my::Matrix4 mProj = *(my::Matrix4 *)m_camera.GetProjMatrix();
		my::Matrix4 mView = *(my::Matrix4 *)m_camera.GetViewMatrix();
		my::Matrix4 mViewProj = mView * mProj;
		my::Matrix4 mWorldViewProjection = mWorld * mViewProj;

		// 清理缓存背景及depth stencil
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新D3DX Effect值
			m_effect->SetMatrix("g_mWorldViewProjection", mWorldViewProjection);
			m_effect->SetMatrix("g_mWorld", mWorld);
			m_effect->SetFloat("g_fTime", (float)fTime);

			// 所有的mesh使用同一种材质，同一张贴图
			m_effect->SetVector("g_MaterialAmbientColor", my::Vector4(0.27f, 0.27f, 0.27f, 1.0f));
			m_effect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			m_effect->SetTexture("g_MeshTexture", m_texture->m_ptr);
			m_effect->SetFloatArray("g_LightDir", (float *)&my::Vector3(0.0f, 0.0f, 1.0f), 3);
			m_effect->SetVector("g_LightDiffuse", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

			// 初始化骨骼变换列表
			//m_effect->SetMatrixArray("mWorldMatrixArray", &m_inverseTransformList[0], m_inverseTransformList.size());
			m_effect->SetMatrixArray("g_dualquat", &m_dualQuaternionList[0], m_dualQuaternionList.size());
			m_effect->SetMatrix("mViewProj", mViewProj);

			// 渲染模型的两个部分，注意，头发的部分不要背面剔除
			m_effect->SetTechnique("RenderScene");
			UINT cPasses = m_effect->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_effect->BeginPass(p);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
				m_mesh->DrawSubset(1);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
				m_mesh->DrawSubset(0);
				m_effect->EndPass();
			}
			m_effect->End();

			V(pd3dDevice->EndScene());
		}
	}

	LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT hres;
		if(FAILED(hres = DxutSample::MsgProc(
			hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing)
		{
			return hres;
		}

		// 相机消息处理
		return m_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
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

	// 初始化资源管理器收索路径
	my::ResourceMgr::getSingleton().RegisterFileDir(".");
	my::ResourceMgr::getSingleton().RegisterFileDir("..\\demo2_1");
	my::ResourceMgr::getSingleton().RegisterFileDir("..\\demo2_2");
	my::ResourceMgr::getSingleton().RegisterFileDir("..\\demo2_3");
	my::ResourceMgr::getSingleton().RegisterFileDir("..\\..\\Common\\medias");
	my::ResourceMgr::getSingleton().RegisterZipArchive("data.zip");

	return MyDemo().Run(true, 800, 600);
}
