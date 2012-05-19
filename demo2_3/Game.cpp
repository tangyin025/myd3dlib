#include "Game.h"

Game::Game(void)
{
}

Game::~Game(void)
{
}

bool Game::IsD3D9DeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}

bool Game::ModifyDeviceSettings(
	DXUTDeviceSettings * pDeviceSettings)
{
	assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

	HRESULT hr;
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	D3DCAPS9 caps;

	V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps ) );

	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if( caps.MaxVertexBlendMatrices < 2 )
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}

void Game::OnInit(void)
{
	DxutApp::OnInit();

	m_settingsDlg.Init(&m_dlgResourceMgr);
}

HRESULT Game::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9CreateDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	V(m_dlgResourceMgr.OnD3D9CreateDevice(pd3dDevice));

	V(m_settingsDlg.OnD3D9CreateDevice(pd3dDevice));

	my::ResourceMgr::getSingleton().RegisterFileDir(".");
	my::ResourceMgr::getSingleton().RegisterZipArchive("data.zip", "");
	my::ResourceMgr::getSingleton().RegisterFileDir("..\\demo2_3");
	my::ResourceMgr::getSingleton().RegisterZipArchive("..\\demo2_3\\data.zip");
	my::ResourceMgr::getSingleton().RegisterFileDir("..\\..\\Common\\medias");

	my::CachePtr cache = my::ResourceMgr::getSingleton().OpenArchiveStream("Untitled-1.png")->GetWholeCache();
	m_uiTex = my::Texture::CreateTextureFromFileInMemory(pd3dDevice, &(*cache)[0], cache->size());
	D3DSURFACE_DESC uiTexDesc = m_uiTex->GetLevelDesc(0);

	cache = my::ResourceMgr::getSingleton().OpenArchiveStream("wqy-microhei-lite.ttc")->GetWholeCache();
	m_uiFnt = my::Font::CreateFontFromFileInCache(pd3dDevice, cache, 13, 1);

	m_defDlgSkin = my::ControlSkinPtr(new my::ControlSkin());
	m_defDlgSkin->m_Font = m_uiFnt;
	m_defDlgSkin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_defDlgSkin->m_TextAlign = my::Font::AlignLeftTop;

	m_hudDlg = my::DialogPtr(new my::Dialog());
	m_hudDlg->m_Color = D3DCOLOR_ARGB(0,255,0,0);
	m_hudDlg->m_Skin = m_defDlgSkin;
	m_dlgSet.insert(m_hudDlg);

	my::ButtonSkinPtr defBtnSkin(new my::ButtonSkin());
	defBtnSkin->m_Texture = m_uiTex;
	defBtnSkin->m_TextureRect = CRect(CPoint(10,10), CSize(125,22));
	defBtnSkin->m_Font = m_uiFnt;
	defBtnSkin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
	defBtnSkin->m_TextAlign = my::Font::AlignCenterMiddle;
	defBtnSkin->m_DisabledTexRect = CRect(CPoint(10,100), CSize(125,22));
	defBtnSkin->m_PressedTexRect = CRect(CPoint(10,70), CSize(125,22));
	defBtnSkin->m_MouseOverTexRect = CRect(CPoint(10,40), CSize(125,22));
	defBtnSkin->m_PressedOffset = my::Vector2(1,1);

	my::ButtonPtr btn = my::ButtonPtr(new my::Button());
	btn->m_Text = L"Toggle full screen";
	btn->m_Location = my::Vector2(35,10);
	btn->m_Size = my::Vector2(125,22);
	btn->m_Skin = defBtnSkin;
	btn->EventClick = fastdelegate::MakeDelegate(this, &Game::OnToggleFullScreen);
	m_hudDlg->m_Controls.insert(btn);

	btn = my::ButtonPtr(new my::Button());
	btn->m_Text = L"Toggle REF (F3)";
	btn->SetHotkey(VK_F3);
	btn->m_Location = my::Vector2(35,35);
	btn->m_Size = my::Vector2(125,22);
	btn->m_Skin = defBtnSkin;
	btn->EventClick = fastdelegate::MakeDelegate(this, &Game::OnToggleRef);
	m_hudDlg->m_Controls.insert(btn);

	btn = my::ButtonPtr(new my::Button());
	btn->m_Text = L"Change device (F2)";
	btn->SetHotkey(VK_F2);
	btn->m_Location = my::Vector2(35,60);
	btn->m_Size = my::Vector2(125,22);
	btn->m_Skin = defBtnSkin;
	btn->EventClick = fastdelegate::MakeDelegate(this, &Game::OnChangeDevice);
	m_hudDlg->m_Controls.insert(btn);

	m_console = ConsolePtr(new Console());
	m_dlgSet.insert(m_console);

	m_input = my::Input::CreateInput(GetModuleHandle(NULL));
	m_keyboard = my::Keyboard::CreateKeyboard(m_input->m_ptr);
	m_mouse = my::Mouse::CreateMouse(m_input->m_ptr);

	m_sound = my::Sound::CreateSound();
	m_sound->SetCooperativeLevel(GetHWND(), DSSCL_PRIORITY);

	initiate();

	return S_OK;
}

HRESULT Game::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Console::getSingleton().AddLine(L"Game::OnD3D9ResetDevice");

	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	V(m_dlgResourceMgr.OnD3D9ResetDevice());

	V(m_settingsDlg.OnD3D9ResetDevice());

	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		my::UIRender::BuildPerspectiveMatrices(
			D3DXToRadian(75.0f),
			pBackBufferSurfaceDesc->Width,
			pBackBufferSurfaceDesc->Height,
			(*dlg_iter)->m_ViewMatrix,
			(*dlg_iter)->m_ProjMatrix);
	}

	m_hudDlg->m_Location = my::Vector2((float)pBackBufferSurfaceDesc->Width - 170, 0);

	m_hudDlg->m_Size = my::Vector2(170, 170);

	CurrentState()->OnD3D9ResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

	return S_OK;
}

void Game::OnD3D9LostDevice(void)
{
	Console::getSingleton().AddLine(L"Game::OnD3D9LostDevice");

	// 当状态切换时发生异常会导致新状态没有被创建
	// 然而 DXUTDestroyState 依然会尝试 OnD3D9LostDevice，所以有必要判断之
	if(!terminated())
		CurrentState()->OnD3D9LostDevice();

	m_dlgResourceMgr.OnD3D9LostDevice();

	m_settingsDlg.OnD3D9LostDevice();

	DxutApp::OnD3D9LostDevice();
}

void Game::OnD3D9DestroyDevice(void)
{
	terminate();

	m_dlgResourceMgr.OnD3D9DestroyDevice();

	m_settingsDlg.OnD3D9DestroyDevice();

	m_dlgSet.clear();

	m_console.reset();

	m_keyboard.reset();

	m_mouse.reset();

	m_input.reset();

	m_sound.reset();

	DxutApp::OnD3D9DestroyDevice();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	DxutApp::OnFrameMove(fTime, fElapsedTime);

	m_keyboard->Capture();

	m_mouse->Capture();

	CurrentState()->OnFrameMove(fTime, fElapsedTime);
}

void Game::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	if(m_settingsDlg.IsActive())
	{
		m_settingsDlg.OnRender(fElapsedTime);
		return;
	}

	CurrentState()->OnD3D9FrameRender(pd3dDevice, fTime, fElapsedTime);

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		my::UIRender::Begin(pd3dDevice);

		DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
		for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
		{
			(*dlg_iter)->OnRender(pd3dDevice, fElapsedTime);
		}

		my::Matrix4 View, Proj;
		D3DVIEWPORT9 vp;
		pd3dDevice->GetViewport(&vp);
		my::UIRender::BuildPerspectiveMatrices(
			D3DXToRadian(75.0f), vp.Width, vp.Height, View, Proj);
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Proj));
		m_uiFnt->DrawString(DXUTGetFrameStats(DXUTIsVsyncEnabled()),
			my::Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0), my::Font::AlignLeftTop);
		m_uiFnt->DrawString(DXUTGetDeviceStats(),
			my::Rectangle::LeftTop(5,5 + (float)m_uiFnt->m_LineHeight,500,10), D3DCOLOR_ARGB(255,255,255,0), my::Font::AlignLeftTop);

		my::UIRender::End(pd3dDevice);

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
	LRESULT hres;
	if(FAILED(hres = DxutApp::MsgProc(
		hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing)
	{
		return hres;
	}

	if((*pbNoFurtherProcessing = m_dlgResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if(m_settingsDlg.IsActive())
	{
		m_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	if(m_console && uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_console->SetEnabled(!m_console->GetEnabled());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	DialogPtrSet::reverse_iterator dlg_iter = m_dlgSet.rbegin();
	for(; dlg_iter != m_dlgSet.rend(); dlg_iter++)
	{
		if((*pbNoFurtherProcessing = (*dlg_iter)->MsgProc(hWnd, uMsg, wParam, lParam)))
			return 0;
	}

	if(!terminated() &&
		(FAILED(hres = CurrentState()->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing))
	{
		return hres;
	}

	return 0;
}

void Game::OnToggleFullScreen(my::ControlPtr ctrl)
{
	DXUTToggleFullScreen();
}

void Game::OnToggleRef(my::ControlPtr ctrl)
{
	DXUTToggleREF();
}

void Game::OnChangeDevice(my::ControlPtr ctrl)
{
	m_settingsDlg.SetActive(!m_settingsDlg.IsActive());
	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		(*dlg_iter)->Refresh();
	}
}

GameLoad::GameLoad(void)
{
	Console::getSingleton().AddLine(L"GameLoad::GameLoad");
}

GameLoad::~GameLoad(void)
{
	Console::getSingleton().AddLine(L"GameLoad::~GameLoad");
}

HRESULT GameLoad::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void GameLoad::OnD3D9LostDevice(void)
{
}

void GameLoad::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	double fAbsTime = Game::getSingleton().GetAbsoluteTime();
	wchar_t buff[256];
	swprintf_s(buff, _countof(buff), L"%f, %f, %f", fTime, fAbsTime, fElapsedTime);
	Console::getSingleton().AddLine(buff);

	if(fTime > 2.0f)
	{
		Game::getSingleton().process_event(EvLoadOver());
	}
}

void GameLoad::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 255, 72), 1, 0));
}

LRESULT GameLoad::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

GamePlay::GamePlay(void)
{
	Console::getSingleton().AddLine(L"GamePlay::GamePlay");

	// 雷人的环境球构造方式！将来还是要扩展成使用 6个 jpg来创建比较省资源空间
	IDirect3DDevice9 * pd3dDevice = Game::getSingleton().GetD3D9Device();
	m_skyBox = SkyBox::CreateSkyBox(pd3dDevice);

	//THROW_CUSEXCEPTION("aaa");
}

GamePlay::~GamePlay(void)
{
	Console::getSingleton().AddLine(L"GamePlay::~GamePlay");
}

HRESULT GamePlay::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void GamePlay::OnD3D9LostDevice(void)
{
}

void GamePlay::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
}

void GamePlay::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 255), 1, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		m_skyBox->Render(fElapsedTime, my::Matrix4::Identity());

		V(pd3dDevice->EndScene());
	}
}

LRESULT GamePlay::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}
