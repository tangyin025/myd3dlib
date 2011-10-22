
#include "stdafx.h"
#include "myd3dlib.h"

namespace my
{
	DeviceRelatedObjectBase::DeviceRelatedObjectBase(void)
	{
		DeviceRelatedObjectBaseSet & set = DeviceRelatedObjectBaseSet::getSingleton();
		_ASSERT(set.end() == set.find(this));
		set.insert(this);
	}

	DeviceRelatedObjectBase::~DeviceRelatedObjectBase(void)
	{
		DeviceRelatedObjectBaseSet & set = DeviceRelatedObjectBaseSet::getSingleton();
		DeviceRelatedObjectBaseSet::iterator this_iter = set.find(this);
		_ASSERT(set.end() != this_iter);
		set.erase(this_iter);
	}

	void DeviceRelatedObjectBaseSet::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		iterator obj_iter = begin();
		for(; obj_iter != end(); obj_iter++)
		{
			(*obj_iter)->OnD3D9ResetDevice(pd3dDevice, pBackBufferSurfaceDesc);
		}
	}

	Singleton<DeviceRelatedObjectBaseSet>::DrivedClassPtr DeviceRelatedObjectBaseSet::s_ptr;

	void DeviceRelatedObjectBaseSet::OnD3D9LostDevice(void)
	{
		iterator obj_iter = begin();
		for(; obj_iter != end(); obj_iter++)
		{
			(*obj_iter)->OnD3D9LostDevice();
		}
	}

	void DeviceRelatedObjectBaseSet::OnD3D9DestroyDevice(void)
	{
		iterator obj_iter = begin();
		for(; obj_iter != end(); obj_iter++)
		{
			(*obj_iter)->OnD3D9DestroyDevice();
		}
	}

	bool CALLBACK DxutAppBase::IsD3D9DeviceAcceptable_s(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed,
		void * pUserContext)
	{
		try
		{
			IDirect3D9 * pD3D = DXUTGetD3D9Object();
			if(FAILED((pD3D->CheckDeviceFormat(
				pCaps->AdapterOrdinal,
				pCaps->DeviceType,
				AdapterFormat,
				D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
				D3DRTYPE_TEXTURE,
				BackBufferFormat))))
			{
				return false;
			}

			return reinterpret_cast<DxutAppBase *>(pUserContext)->IsD3D9DeviceAcceptable(
				pCaps, AdapterFormat, BackBufferFormat, bWindowed);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return false;
	}

	bool CALLBACK DxutAppBase::ModifyDeviceSettings_s(
		DXUTDeviceSettings * pDeviceSettings,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->ModifyDeviceSettings(
				pDeviceSettings);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return false;
	}

	HRESULT CALLBACK DxutAppBase::OnD3D9CreateDevice_s(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9CreateDevice(
				pd3dDevice, pBackBufferSurfaceDesc);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return D3DERR_INVALIDCALL;
	}

	HRESULT CALLBACK DxutAppBase::OnD3D9ResetDevice_s(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9ResetDevice(
				pd3dDevice, pBackBufferSurfaceDesc);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return D3DERR_INVALIDCALL;
	}

	void CALLBACK DxutAppBase::OnD3D9LostDevice_s(
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9LostDevice();
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void CALLBACK DxutAppBase::OnD3D9DestroyDevice_s(
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9DestroyDevice();
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void CALLBACK DxutAppBase::OnFrameMove_s(
		double fTime,
		float fElapsedTime,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnFrameMove(
				fTime, fElapsedTime);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void CALLBACK DxutAppBase::OnD3D9FrameRender_s(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9FrameRender(
				pd3dDevice, fTime, fElapsedTime);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	LRESULT CALLBACK DxutAppBase::MsgProc_s(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->MsgProc(
				hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return 0;
	}

	void CALLBACK DxutAppBase::OnKeyboard_s(
		UINT nChar,
		bool bKeyDown,
		bool bAltDown,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnKeyboard(
				nChar, bKeyDown, bAltDown);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	DxutAppBase::DxutAppBase(void)
	{
		DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable_s, this);
		DXUTSetCallbackDeviceChanging(ModifyDeviceSettings_s, this);
		DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice_s, this);
		DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice_s, this);
		DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice_s, this);
		DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice_s, this);
		DXUTSetCallbackFrameMove(OnFrameMove_s, this);
		DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender_s, this);
		DXUTSetCallbackMsgProc(MsgProc_s, this);
		DXUTSetCallbackKeyboard(OnKeyboard_s, this);
	}

	DxutAppBase::~DxutAppBase(void)
	{
	}

	int DxutAppBase::Run(
		bool bWindowed /*= true*/,
		int nSuggestedWidth /*= 800*/,
		int nSuggestedHeight /*= 600*/)
	{
		DXUTInit(true, true, NULL);
		DXUTSetCursorSettings(true, true);
		WCHAR szPath[MAX_PATH];
		GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
		DXUTCreateWindow(szPath);
		DXUTCreateDevice(bWindowed, nSuggestedWidth, nSuggestedHeight);
		DXUTMainLoop();
		int nExitCode = DXUTGetExitCode();
		DXUTDestroyState();
		return nExitCode;
	}

	SingleInstance<DxutApp> * DxutApp::s_ptr = NULL;

	bool DxutApp::IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		return true;
	}

	bool DxutApp::ModifyDeviceSettings(
		DXUTDeviceSettings * pDeviceSettings)
	{
		if(DXUT_D3D9_DEVICE == pDeviceSettings->ver
			&& D3DDEVTYPE_REF == pDeviceSettings->d3d9.DeviceType)
		{
			DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
		}
		return true;
	}

	HRESULT DxutApp::OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		return S_OK;
	}

	HRESULT DxutApp::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		DeviceRelatedObjectBaseSet::getSingleton().OnD3D9ResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

		return S_OK;
	}

	void DxutApp::OnD3D9LostDevice(void)
	{
		DeviceRelatedObjectBaseSet::getSingleton().OnD3D9LostDevice();
	}

	void DxutApp::OnD3D9DestroyDevice(void)
	{
		DeviceRelatedObjectBaseSet::getSingleton().OnD3D9DestroyDevice();
	}

	void DxutApp::OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
	}

	void DxutApp::OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
	}

	LRESULT DxutApp::MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		return 0;
	}

	void DxutApp::OnKeyboard(
		UINT nChar,
		bool bKeyDown,
		bool bAltDown)
	{
	}

	void DxutApp::OnInit(void)
	{
	}

	int DxutApp::Run(
		bool bWindowed /*= true*/,
		int nSuggestedWidth /*= 800*/,
		int nSuggestedHeight /*= 600*/)
	{
		try
		{
			OnInit();
		}
		catch(const my::Exception & e)
		{
			MessageBox(GetDesktopWindow(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
			return 0;
		}

		return DxutAppBase::Run(bWindowed, nSuggestedWidth, nSuggestedHeight);
	}

	bool DxutSample::IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		{
			return false;
		}

		return true;
	}

	void DxutSample::OnInit(void)
	{
		DxutApp::OnInit();

		m_settingsDlg.Init(&m_dlgResourceMgr);
		m_hudDlg.Init(&m_dlgResourceMgr);
		m_hudDlg.SetCallback(OnGUIEvent_s, this);

		int nY = 10;
		m_hudDlg.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, nY, 125, 22);
		m_hudDlg.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, nY += 24, 125, 22, VK_F3);
		m_hudDlg.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 35, nY += 24, 125, 22, VK_F2);
	}

	HRESULT DxutSample::OnD3D9CreateDevice(
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

		FAILED_THROW_D3DEXCEPTION(D3DXCreateFont(
			pd3dDevice, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_font));

		V(D3DXCreateSprite(pd3dDevice, &m_sprite));

		return S_OK;
	}

	HRESULT DxutSample::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		V(m_dlgResourceMgr.OnD3D9ResetDevice());

		V(m_settingsDlg.OnD3D9ResetDevice());

		m_hudDlg.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);

		m_hudDlg.SetSize(170, 170);

		V(m_font->OnResetDevice());

		V(m_sprite->OnResetDevice());

		return S_OK;
	}

	void DxutSample::OnD3D9LostDevice(void)
	{
		DxutApp::OnD3D9LostDevice();

		m_dlgResourceMgr.OnD3D9LostDevice();

		m_settingsDlg.OnD3D9LostDevice();

		m_font->OnLostDevice();

		m_sprite->OnLostDevice();
	}

	void DxutSample::OnD3D9DestroyDevice(void)
	{
		DxutApp::OnD3D9DestroyDevice();

		m_dlgResourceMgr.OnD3D9DestroyDevice();

		m_settingsDlg.OnD3D9DestroyDevice();

		m_font.Release();

		m_sprite.Release();
	}

	void DxutSample::OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		DxutApp::OnFrameMove(fTime, fElapsedTime);
	}

	void DxutSample::OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		if(m_settingsDlg.IsActive())
		{
			m_settingsDlg.OnRender(fElapsedTime);
			return;
		}

		OnRender(pd3dDevice, fTime, fElapsedTime);

		HRESULT hr;
		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			CDXUTTextHelper txtHelper(m_font, m_sprite, 15);
			txtHelper.Begin();
			txtHelper.SetInsertionPos(5, 5);
			txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
			txtHelper.DrawTextLine(DXUTGetFrameStats( DXUTIsVsyncEnabled()));
			txtHelper.DrawTextLine(DXUTGetDeviceStats());
			txtHelper.End();

			V(m_hudDlg.OnRender(fElapsedTime));

			V(pd3dDevice->EndScene());
		}
	}

	void DxutSample::OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		HRESULT hr;
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));
	}

	void CALLBACK DxutSample::OnGUIEvent_s(
		UINT nEvent,
		int nControlID,
		CDXUTControl * pControl,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutSample *>(pUserContext)->OnGUIEvent(
				nEvent, nControlID, pControl);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void DxutSample::OnGUIEvent(
		UINT nEvent,
		int nControlID,
		CDXUTControl * pControl)
	{
		switch(nControlID)
		{
		case IDC_TOGGLEFULLSCREEN:
			DXUTToggleFullScreen();
			break;

		case IDC_TOGGLEREF:
			DXUTToggleREF();
			break;

		case IDC_CHANGEDEVICE:
			m_settingsDlg.SetActive(!m_settingsDlg.IsActive());
			break;
		}
	}

	LRESULT DxutSample::MsgProc(
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

		*pbNoFurtherProcessing = m_dlgResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam);
		if(*pbNoFurtherProcessing)
		{
			return 0;
		}

		if(m_settingsDlg.IsActive())
		{
			m_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
			return 0;
		}

		*pbNoFurtherProcessing = m_hudDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		if(*pbNoFurtherProcessing)
		{
			return 0;
		}

		return 0;
	}
};
