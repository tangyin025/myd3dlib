#include <myD3dLib.h>
#include <boost/bind.hpp>
#include <Opcode.h>

using namespace my;

class Demo
	: public DxutApp
	, public ResourceMgr
{
protected:
	CComPtr<ID3DXFont> m_font;

	CComPtr<ID3DXSprite> m_sprite;

	std::vector<DeviceResourceBasePtr> m_reses;

	void foo(DeviceResourceBasePtr res)
	{
		//Texture2DPtr tex = boost::dynamic_pointer_cast<Texture2D>(res);

		//OgreMeshPtr mesh = boost::dynamic_pointer_cast<OgreMesh>(res);

		//OgreSkeletonAnimationPtr skel = boost::dynamic_pointer_cast<OgreSkeletonAnimation>(res);

		//EffectPtr eff = boost::dynamic_pointer_cast<Effect>(res);

		//FontPtr fnt = boost::dynamic_pointer_cast<Font>(res);

		m_reses.push_back(res);
	}

public:
	Demo(void)
	{
	}

	virtual bool IsDeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		return true;
	}

	virtual bool ModifyDeviceSettings(
		DXUTD3D9DeviceSettings * pDeviceSettings)
	{
		return true;
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = D3DXCreateFont(
			pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"), &m_font)))
		{
			return hr;
		}

		ResourceMgr::RegisterFileDir("../demo2_3/Media");

		ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

		//LoadTextureAsync("texture/galileo_cross.dds", boost::bind(&Demo::foo, this, _1));

		//LoadTextureAsync("texture/galileo_cross.dds", boost::bind(&Demo::foo, this, _1));

		//BaseTexturePtr tex = LoadTexture("texture/galileo_cross.dds");

		class AAA : public my::IResourceCallback
		{
		public:
			my::DeviceResourceBasePtr m_res;
			virtual void OnReady(my::DeviceResourceBasePtr res) {
				m_res = res;
			}
		};
		AAA aaa;
		LoadTextureAsync("texture/galileo_cross.dds", &aaa);
		RemoveIORequestCallback("texture/galileo_cross.dds", &aaa);

		//LoadMeshAsync("mesh/sportive03_f.mesh.xml", boost::bind(&Demo::foo, this, _1));

		//LoadSkeletonAsync("mesh/sportive03_f.skeleton.xml", boost::bind(&Demo::foo, this, _1));

		////EffectPtr eff = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());

		//LoadFontAsync("font/wqy-microhei.ttc", 13, boost::bind(&Demo::foo, this, _1));

		////MaterialPtr mat = LoadMaterial("material/lambert1.xml");

		//LoadMaterialAsync("material/casual19_m_highpolyPhong.txt", boost::bind(&Demo::foo, this, _1));

		//LoadMeshSetAsync("mesh/scene1.mesh.xml", boost::bind(&Demo::foo, this, _1));

		struct BBB
		{
			my::OgreMesh * mesh;
			DWORD VertexStride;
			void * pVertices;
			void * pIndices;
			static void RequestCallback(udword triangle_index, Opcode::VertexPointers& triangle, void* user_data)
			{
				BBB * b = (BBB *)user_data;
				unsigned short i[3] = {
					*((unsigned short *)b->pIndices + triangle_index * 3 + 0),
					*((unsigned short *)b->pIndices + triangle_index * 3 + 1),
					*((unsigned short *)b->pIndices + triangle_index * 3 + 2)};
				void * pVertex[3] = {
					(unsigned char *)b->pVertices + i[0] * b->VertexStride,
					(unsigned char *)b->pVertices + i[1] * b->VertexStride,
					(unsigned char *)b->pVertices + i[2] * b->VertexStride};
				triangle.Vertex[0] = (Point *)&b->mesh->m_VertexElems.GetPosition(pVertex[0]);
				triangle.Vertex[1] = (Point *)&b->mesh->m_VertexElems.GetPosition(pVertex[1]);
				triangle.Vertex[2] = (Point *)&b->mesh->m_VertexElems.GetPosition(pVertex[2]);
			}
		};

		my::OgreMeshPtr mesh = LoadMesh("mesh/Nyra.mesh.xml");
		Opcode::MeshInterface mi;
		mi.SetNbTriangles(mesh->GetNumFaces());
		mi.SetNbVertices(mesh->GetNumVertices());
		BBB b;
		b.mesh = mesh.get();
		b.VertexStride = mesh->GetNumBytesPerVertex();
		b.pVertices = mesh->LockVertexBuffer();
		b.pIndices = mesh->LockIndexBuffer();
		mi.SetCallback(&BBB::RequestCallback, &b);
		Opcode::OPCODECREATE opcc;
		opcc.mIMesh = &mi;
		Opcode::Model mdl;
		mdl.Build(opcc);
		mesh->UnlockVertexBuffer();
		mesh->UnlockIndexBuffer();

		struct CCC
		{
			static void HitCallback(const Opcode::CollisionFace& hit, void* user_data)
			{
				float k = hit.mDistance;
			}
		};

		Opcode::RayCollider rc;
		rc.SetFirstContact(true);
		rc.SetCulling(true);
		rc.SetHitCallback(&CCC::HitCallback);
		CCC c;
		rc.SetUserData(&c);
		rc.Collide(IceMaths::Ray(IceMaths::Point(0,5,10), IceMaths::Point(0,0,-1)), mdl, NULL, NULL);

		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		m_font->OnResetDevice();

		if(FAILED(hr = D3DXCreateSprite(pd3dDevice, &m_sprite)))
		{
			return hr;
		}

		ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		m_font->OnLostDevice();

		m_sprite.Release();

		ResourceMgr::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		m_font.Release();

		ResourceMgr::OnDestroyDevice();
	}

	void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		// Clear the render target and the zbuffer 
		V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );

		// Render the scene
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			V(m_sprite->Begin(D3DXSPRITE_ALPHABLEND));
			wchar_t buff[256];
			int len = swprintf_s(buff, _countof(buff), L"%.2f", m_fFps);
			V(m_font->DrawTextW(m_sprite, buff, len, CRect(5,5,100,100), DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(1.0f,1.0f,0.0f,1.0f)));
			V(m_sprite->End());
			V( pd3dDevice->EndScene() );
		}
	}

	virtual void OnFrameTick(double fTime, float fElapsedTime)
	{
		CheckIORequests();

		OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

		Present(0,0,0,0);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		return 0;
	}

	virtual void OnResourceFailed(const std::string & error_str)
	{
	}
};

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	return Demo().Run();
}
