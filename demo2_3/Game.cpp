#include "stdafx.h"
#include "Game.h"
#include "GameState.h"
#include "LuaExtension.h"
#include <luabind/luabind.hpp>
#include <signal.h>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;
//
//void DrawHelper::DrawLine(
//	IDirect3DDevice9 * pd3dDevice,
//	const Vector3 & v0,
//	const Vector3 & v1,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	Vertex v[2];
//	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
//	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawSphere(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	DrawSpereStage(pd3dDevice, radius, 0, 20, 0, Color, world);
//}
//
//void DrawHelper::DrawBox(
//	IDirect3DDevice9 * pd3dDevice,
//	const Vector3 & halfSize,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	Vertex v[8];
//	v[0].x = -halfSize.x; v[0].y = -halfSize.y; v[0].z = -halfSize.z; v[0].color = Color;
//	v[1].x =  halfSize.x; v[1].y = -halfSize.y; v[1].z = -halfSize.z; v[1].color = Color;
//	v[2].x = -halfSize.x; v[2].y =  halfSize.y; v[2].z = -halfSize.z; v[2].color = Color;
//	v[3].x =  halfSize.x; v[3].y =  halfSize.y; v[3].z = -halfSize.z; v[3].color = Color;
//	v[4].x = -halfSize.x; v[4].y =  halfSize.y; v[4].z =  halfSize.z; v[4].color = Color;
//	v[5].x =  halfSize.x; v[5].y =  halfSize.y; v[5].z =  halfSize.z; v[5].color = Color;
//	v[6].x = -halfSize.x; v[6].y = -halfSize.y; v[6].z =  halfSize.z; v[6].color = Color;
//	v[7].x =  halfSize.x; v[7].y = -halfSize.y; v[7].z =  halfSize.z; v[7].color = Color;
//
//	unsigned short idx[12 * 2];
//	int i = 0;
//	idx[i++] = 0; idx[i++] = 1; idx[i++] = 1; idx[i++] = 3; idx[i++] = 3; idx[i++] = 2; idx[i++] = 2; idx[i++] = 0;
//	idx[i++] = 0; idx[i++] = 6; idx[i++] = 1; idx[i++] = 7; idx[i++] = 3; idx[i++] = 5; idx[i++] = 2; idx[i++] = 4;
//	idx[i++] = 6; idx[i++] = 7; idx[i++] = 7; idx[i++] = 5; idx[i++] = 5; idx[i++] = 4; idx[i++] = 4; idx[i++] = 6;
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, _countof(v), _countof(idx) / 2, idx, D3DFMT_INDEX16, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawTriangle(
//	IDirect3DDevice9 * pd3dDevice,
//	const Vector3 & v0,
//	const Vector3 & v1,
//	const Vector3 & v2,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	Vertex v[4];
//	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
//	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
//	v[2].x = v2.x; v[2].y = v2.y; v[2].z = v2.z; v[2].color = Color;
//	v[3] = v[0];
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, _countof(v) - 1, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawSpereStage(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	int VSTAGE_BEGIN,
//	int VSTAGE_END,
//	float offsetY,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	const int VSTAGE = 20;
//	const int HSTAGE = 20;
//	Vertex v[VSTAGE * HSTAGE * 4];
//	for(int j = VSTAGE_BEGIN; j < VSTAGE_END; j++)
//	{
//		for(int i = 0; i < HSTAGE; i++)
//		{
//			float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
//			float Fi[2] = {D3DX_PI / VSTAGE * j, D3DX_PI / VSTAGE * (j + 1)};
//			Vertex * pv = &v[(j * HSTAGE + i) * 4];
//			pv[0].x = radius * sin(Fi[0]) * cos(Theta[0]);
//			pv[0].y = radius * cos(Fi[0]) + offsetY;
//			pv[0].z = radius * sin(Fi[0]) * sin(Theta[0]);
//			pv[0].color = Color;
//
//			pv[1].x = radius * sin(Fi[0]) * cos(Theta[1]);
//			pv[1].y = radius * cos(Fi[0]) + offsetY;
//			pv[1].z = radius * sin(Fi[0]) * sin(Theta[1]);
//			pv[1].color = Color;
//
//			pv[2] = pv[0];
//
//			pv[3].x = radius * sin(Fi[1]) * cos(Theta[0]);
//			pv[3].y = radius * cos(Fi[1]) + offsetY;
//			pv[3].z = radius * sin(Fi[1]) * sin(Theta[0]);
//			pv[3].color = Color;
//		}
//	}
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawCylinderStage(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	float y0,
//	float y1,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	const int HSTAGE = 20;
//	Vertex v[HSTAGE * 4];
//	for(int i = 0; i < HSTAGE; i++)
//	{
//		float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
//		Vertex * pv = &v[i * 4];
//		pv[0].x = radius * cos(Theta[0]);
//		pv[0].y = y0;
//		pv[0].z = radius * sin(Theta[0]);
//		pv[0].color = Color;
//
//		pv[1].x = radius * cos(Theta[1]);
//		pv[1].y = y0;
//		pv[1].z = radius * sin(Theta[1]);
//		pv[1].color = Color;
//
//		pv[2] = pv[0];
//
//		pv[3].x = radius * cos(Theta[0]);
//		pv[3].y = y1;
//		pv[3].z = radius * sin(Theta[0]);
//		pv[3].color = Color;
//	}
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawCapsule(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	float height,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	float y0 = height * 0.5f;
//	float y1 = -y0;
//	DrawSpereStage(pd3dDevice, radius, 0, 10, y0, Color, world);
//	DrawSpereStage(pd3dDevice, radius, 10, 20, y1, Color, world);
//	DrawCylinderStage(pd3dDevice, radius, y0, y1, Color, world);
//}

LoaderMgr::LoaderMgr(void)
{
	RegisterFileDir("Media");
	RegisterZipArchive("Media.zip");
	RegisterFileDir("..\\demo2_3\\Media");
	RegisterZipArchive("..\\demo2_3\\Media.zip");
}

LoaderMgr::~LoaderMgr(void)
{
}

HRESULT LoaderMgr::Open(
	D3DXINCLUDE_TYPE IncludeType,
	LPCSTR pFileName,
	LPCVOID pParentData,
	LPCVOID * ppData,
	UINT * pBytes)
{
	CachePtr cache;
	std::string loc_path = std::string("shader/") + pFileName;
	switch(IncludeType)
	{
	case D3DXINC_SYSTEM:
	case D3DXINC_LOCAL:
		if(CheckArchivePath(loc_path))
		{
			cache = OpenArchiveStream(loc_path)->GetWholeCache();
			*ppData = &(*cache)[0];
			*pBytes = cache->size();
			_ASSERT(m_cacheSet.end() == m_cacheSet.find(*ppData));
			m_cacheSet[*ppData] = cache;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT LoaderMgr::Close(
	LPCVOID pData)
{
	_ASSERT(m_cacheSet.end() != m_cacheSet.find(pData));
	m_cacheSet.erase(m_cacheSet.find(pData));
	return S_OK;
}

HRESULT LoaderMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnResetDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}

	return S_OK;
}

void LoaderMgr::OnLostDevice(void)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnLostDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}
}

void LoaderMgr::OnDestroyDevice(void)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnDestroyDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}

	m_resourceSet.clear();
}

TexturePtr LoaderMgr::LoadTexture(const std::string & path, bool reload)
{
	TexturePtr ret = GetDeviceRelatedResource<Texture>(path, reload);
	if(!ret->m_ptr)
	{
		std::string loc_path = std::string("texture/") + path;
		std::string full_path = GetFullPath(loc_path);
		if(!full_path.empty())
		{
			ret->CreateTextureFromFile(GetD3D9Device(), ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateTextureFromFileInMemory(GetD3D9Device(), &(*cache)[0], cache->size());
		}
	}
	return ret;
}

CubeTexturePtr LoaderMgr::LoadCubeTexture(const std::string & path, bool reload)
{
	CubeTexturePtr ret = GetDeviceRelatedResource<CubeTexture>(path, reload);
	if(!ret->m_ptr)
	{
		std::string loc_path = std::string("texture/") + path;
		std::string full_path = GetFullPath(loc_path);
		if(!full_path.empty())
		{
			ret->CreateCubeTextureFromFile(GetD3D9Device(), ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateCubeTextureFromFileInMemory(GetD3D9Device(), &(*cache)[0], cache->size());
		}
	}
	return ret;
}

OgreMeshPtr LoaderMgr::LoadMesh(const std::string & path, bool reload)
{
	OgreMeshPtr ret = GetDeviceRelatedResource<OgreMesh>(path, reload);
	if(!ret->m_ptr)
	{
		std::string loc_path = std::string("mesh/") + path;
		std::string full_path = GetFullPath(loc_path);
		if(!full_path.empty())
		{
			ret->CreateMeshFromOgreXml(GetD3D9Device(), full_path.c_str(), true);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateMeshFromOgreXmlInMemory(GetD3D9Device(), (char *)&(*cache)[0], cache->size(), true);
		}
	}
	return ret;
}

OgreSkeletonAnimationPtr LoaderMgr::LoadSkeleton(const std::string & path, bool reload)
{
	OgreSkeletonAnimationSet::const_iterator res_iter = m_skeletonSet.find(path);
	OgreSkeletonAnimationPtr ret;
	if(m_skeletonSet.end() != res_iter)
	{
		ret = res_iter->second.lock();
		if(ret)
		{
			if(reload)
				ret->Clear();
			else
				return ret;
		}
	}
	else
		ret.reset(new OgreSkeletonAnimation());

	std::string loc_path = std::string("mesh/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateOgreSkeletonAnimationFromFile(ms2ts(full_path.c_str()).c_str());
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateOgreSkeletonAnimation((char *)&(*cache)[0], cache->size());
	}
	return ret;
}

EffectPtr LoaderMgr::LoadEffect(const std::string & path, bool reload)
{
	EffectPtr ret = GetDeviceRelatedResource<Effect>(path, reload);
	if(!ret->m_ptr)
	{
		std::string loc_path = std::string("shader/") + path;
		std::string full_path = GetFullPath(loc_path);
		if(!full_path.empty())
		{
			ret->CreateEffectFromFile(GetD3D9Device(), ms2ts(full_path.c_str()).c_str(), NULL, NULL, 0, m_EffectPool);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateEffect(GetD3D9Device(), &(*cache)[0], cache->size(), NULL, this, 0, m_EffectPool);
		}
	}
	return ret;
}

FontPtr LoaderMgr::LoadFont(const std::string & path, int height, bool reload)
{
	FontPtr ret = GetDeviceRelatedResource<Font>(str_printf("%s, %d", path.c_str(), height), reload);
	if(!ret->m_face)
	{
		std::string loc_path = std::string("font/") + path;
		std::string full_path = GetFullPath(loc_path);
		if(!full_path.empty())
		{
			ret->CreateFontFromFile(GetD3D9Device(), full_path.c_str(), height);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateFontFromFileInCache(GetD3D9Device(), cache, height);
		}
	}
	return ret;
}

void Timer::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_RemainingTime += fElapsedTime;
	unsigned int iter = 0;
	while(m_RemainingTime >= 0 && iter++ < m_MaxIter && m_Running)
	{
		m_RemainingTime -= m_Interval;

		if(m_EventTimer)
			m_EventTimer(m_DefaultArgs);
	}
}

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		// ! take care of EventTimer removing self
		(*timer_iter++)->OnFrameMove(fElapsedTime, fElapsedTime);
	}
}

void DialogMgr::OnAlign(void)
{
	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		UpdateDlgViewProj(*dlg_iter);
	}
}

void DialogMgr::UpdateDlgViewProj(DialogPtr dlg)
{
	const D3DSURFACE_DESC & desc = GetD3D9BackBufferSurfaceDesc();

	if(dlg->EventAlign)
		dlg->EventAlign(EventArgsPtr(new AlignEventArgs(Vector2((float)desc.Width, (float)desc.Height))));
}

void DialogMgr::Draw(
	UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		ui_render->SetWorld((*dlg_iter)->m_Transform);
		ui_render->SetView((*dlg_iter)->m_View);
		ui_render->SetProj((*dlg_iter)->m_Proj);

		(*dlg_iter)->Draw(ui_render, fElapsedTime);
	}
}

bool DialogMgr::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	DialogPtrSet::reverse_iterator dlg_iter = m_dlgSet.rbegin();
	for(; dlg_iter != m_dlgSet.rend(); dlg_iter++)
	{
		if((*dlg_iter)->MsgProc(hWnd, uMsg, wParam, lParam))
			return true;
	}

	return false;
}

Game::Game(void)
{
	m_lua.reset(new LuaContext());

	Export2Lua(m_lua->_state);

	m_CurrentStateIter = m_stateMap.end();
}

Game::~Game(void)
{
	RemoveAllDlg();

	RemoveAllTimer();

	ImeEditBox::Uninitialize();
}

bool Game::IsDeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	if( FAILED( m_d3d9->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}

bool Game::ModifyDeviceSettings(
	DXUTD3D9DeviceSettings * pDeviceSettings)
{
	D3DCAPS9 caps;
	V( m_d3d9->GetDeviceCaps( pDeviceSettings->AdapterOrdinal,
		pDeviceSettings->DeviceType,
		&caps ) );

	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if( caps.MaxVertexBlendMatrices < 2 )
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// ! Fix lua print(0xffffffff) issue, ref: http://www.lua.org/bugs.html#5.1-3
	pDeviceSettings->BehaviorFlags |= D3DCREATE_FPU_PRESERVE;

	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		//if( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
		//	DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}

HRESULT Game::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	if(FAILED(hr = D3DXCreateEffectPool(&m_EffectPool)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	ImeEditBox::Initialize(GetHWND());

	ImeEditBox::EnableImeSystem(false);

	ExecuteCode("dofile \"Console.lua\"");

	ExecuteCode("dofile \"Hud.lua\"");

	if(!m_font || !m_console || !m_panel)
	{
		THROW_CUSEXCEPTION("m_font, m_console, m_panel must be created");
	}

	m_console->SetVisible(false);

	UpdateDlgViewProj(m_console);

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(!m_input)
	{
		m_input.reset(new Input());
		m_input->CreateInput(GetModuleHandle(NULL));

		m_keyboard.reset(new Keyboard());
		m_keyboard->CreateKeyboard(m_input->m_ptr);
		m_keyboard->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		m_mouse.reset(new Mouse());
		m_mouse->CreateMouse(m_input->m_ptr);
		m_mouse->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}

	if(!m_sound)
	{
		m_sound.reset(new Sound());
		m_sound->CreateSound();
		m_sound->SetCooperativeLevel(GetHWND(), DSSCL_PRIORITY);
	}

	SetState("GameStateLoad", GameStateBasePtr(new GameStateLoad()));

	SetState("GameStateMain", GameStateBasePtr(new GameStateMain()));

	ChangeState("GameStateLoad");

	//THROW_CUSEXCEPTION("aaa");

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	HRESULT hres;
	if(FAILED(hres = LoaderMgr::OnResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	UpdateDlgViewProj(m_console);

	OnAlign();

	SafeResetState(GetCurrentState());

	return S_OK;
}

void Game::OnLostDevice(void)
{
	AddLine(L"Game::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	SafeLostState(GetCurrentState());

	LoaderMgr::OnLostDevice();
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	SafeDestroyState(GetCurrentState());

	RemoveAllState();

	m_EffectPool.Release();

	ExecuteCode("collectgarbage(\"collect\")");

	m_console.reset();

	RemoveAllDlg();

	ImeEditBox::Uninitialize();

	RemoveAllTimer();

	LoaderMgr::OnDestroyDevice();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_keyboard->Capture();

	m_mouse->Capture();

	TimerMgr::OnFrameMove(fTime, fElapsedTime);

	if(m_stateMap.end() != m_CurrentStateIter)
		m_CurrentStateIter->second->OnFrameMove(fTime, fElapsedTime);
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	if(m_stateMap.end() != m_CurrentStateIter)
		m_CurrentStateIter->second->OnFrameRender(pd3dDevice, fTime, fElapsedTime);
	else
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		UIRender ui_render(pd3dDevice);

		ui_render.Begin();

		DialogMgr::Draw(&ui_render, fTime, fElapsedTime);

		ui_render.SetWorld(m_console->m_Transform);
		ui_render.SetView(m_console->m_View);
		ui_render.SetProj(m_console->m_Proj);
		m_console->Draw(&ui_render, fElapsedTime);

		_ASSERT(m_font);

		Matrix4 View, Proj;
		D3DVIEWPORT9 vp;
		pd3dDevice->GetViewport(&vp);
		UIRender::BuildPerspectiveMatrices(
			D3DXToRadian(75.0f), (float)vp.Width, (float)vp.Height, View, Proj);
		V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity));
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Proj));
		m_font->DrawString(m_strFPS, Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0));

		ui_render.End();

		V(pd3dDevice->EndScene());
	}
}

LRESULT Game::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	if(m_console && uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_console->SetVisible(!m_console->GetVisible());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if(m_console && (*pbNoFurtherProcessing = m_console->MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if((*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if(m_stateMap.end() != m_CurrentStateIter)
	{
		LRESULT lr;
		if(lr = m_CurrentStateIter->second->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
			return lr;
	}

	return 0;
}

static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(Game::getSingleton().m_lua->_state, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}
//
//static void l_message (const char *pname, const char *msg) {
//  //if (pname) fprintf(stderr, "%s: ", pname);
//  //fprintf(stderr, "%s\n", msg);
//  //fflush(stderr);
//	Game::getSingleton().AddLine(L"");
//	Game::getSingleton().puts(ms2ws(msg).c_str());
//}
//
//static int report (lua_State *L, int status) {
//  if (status && !lua_isnil(L, -1)) {
//    const char *msg = lua_tostring(L, -1);
//    if (msg == NULL) msg = "(error object is not a string)";
//    l_message("aaa", msg);
//    lua_pop(L, 1);
//  }
//  return status;
//}

static int dostring (lua_State *L, const char *s, const char *name) {
  //int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  //return report(L, status);
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  if(status && !lua_isnil(L, -1))
  {
	  std::string msg = lua_tostring(L, -1);
	  if(msg.empty())
		  msg = "(error object is not a string)";
	  lua_pop(L, 1);
	  THROW_CUSEXCEPTION(msg);
  }
  return status;
}

void Game::ExecuteCode(const char * code)
{
	dostring(m_lua->_state, code, "Game::ExecuteCode");
}

void Game::SetState(const std::string & key, GameStateBasePtr state)
{
	_ASSERT(!GetState(key));

	m_stateMap[key] = state;
}

GameStateBasePtr Game::GetState(const std::string & key) const
{
	GameStateBasePtrMap::const_iterator state_iter = m_stateMap.find(key);
	if(m_stateMap.end() != state_iter)
		return state_iter->second;

	return GameStateBasePtr();
}

GameStateBasePtr Game::GetCurrentState(void) const
{
	if(m_stateMap.end() != m_CurrentStateIter)
		return m_CurrentStateIter->second;

	return GameStateBasePtr();
}

std::string Game::GetCurrentStateKey(void) const
{
	if(m_stateMap.end() != m_CurrentStateIter)
		return m_CurrentStateIter->first;

	return "";
}

void Game::SafeCreateState(GameStateBasePtr state)
{
	if(state)
	{
		_ASSERT(!state->m_DeviceObjectsCreated);
		state->OnCreateDevice(GetD3D9Device(), &m_BackBufferSurfaceDesc);
		state->m_DeviceObjectsCreated = true;
	}
}

void Game::SafeResetState(GameStateBasePtr state)
{
	if(state && state->m_DeviceObjectsCreated && m_DeviceObjectsCreated)
	{
		_ASSERT(!state->m_DeviceObjectsReset);
		state->OnResetDevice(GetD3D9Device(), &m_BackBufferSurfaceDesc);
		state->m_DeviceObjectsReset = true;
	}
}

void Game::SafeLostState(GameStateBasePtr state)
{
	if(state && state->m_DeviceObjectsReset)
	{
		state->OnLostDevice();
		state->m_DeviceObjectsReset = false;
	}
}

void Game::SafeDestroyState(GameStateBasePtr state)
{
	if(state && state->m_DeviceObjectsCreated)
	{
		_ASSERT(!state->m_DeviceObjectsReset);
		state->OnDestroyDevice();
		state->m_DeviceObjectsCreated = false;
	}
}

void Game::SafeChangeState(GameStateBasePtr old_state, GameStateBasePtrMap::const_iterator new_state_iter)
{
	SafeLostState(old_state);

	SafeDestroyState(old_state);

	m_CurrentStateIter = new_state_iter;

	if(m_stateMap.end() != new_state_iter)
	{
		try
		{
			SafeCreateState(new_state_iter->second);

			SafeResetState(new_state_iter->second);
		}
		catch(const my::Exception & e)
		{
			SafeLostState(new_state_iter->second);

			SafeDestroyState(new_state_iter->second);

			m_CurrentStateIter = m_stateMap.end();

			// ! 状态切换是可以容错的，只是当状态切换失败后，将没有CurrentState
			AddLine(ms2ws(e.GetDescription().c_str()));
		}
	}
}

void Game::ChangeState(const std::string & key)
{
	SafeChangeState(GetCurrentState(), m_stateMap.find(key));
}

void Game::RemoveAllState(void)
{
	m_stateMap.clear();

	m_CurrentStateIter = m_stateMap.end();
}
