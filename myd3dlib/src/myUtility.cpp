#include "StdAfx.h"
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"

using namespace my;

LoaderMgr::LoaderMgr(void)
{
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

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		Timer * const timer = timer_iter->first.get();
		timer->m_RemainingTime = Max(timer->m_RemainingTime - fElapsedTime, m_MinRemainingTime);
		while(timer->m_RemainingTime <= 0 && timer_iter->second)
		{
			if(timer->m_EventTimer)
				timer->m_EventTimer(m_DefaultArgs);

			timer->m_RemainingTime += timer->m_Interval;
		}

		if(timer_iter->second)
			timer_iter++;
		else
			m_timerSet.erase(timer_iter++);
	}
}

void DialogMgr::SetDlgViewport(const Vector2 & vp)
{
	m_View = UIRender::PerspectiveView(D3DXToRadian(75), vp.x, vp.y);

	m_Proj = UIRender::PerspectiveProj(D3DXToRadian(75), vp.x, vp.y);

	DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
	for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
	{
		DialogPtrSet::iterator dlg_iter = dlg_layer_iter->second.begin();
		for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
		{
			if((*dlg_iter)->EventAlign)
				(*dlg_iter)->EventAlign(EventArgsPtr(new EventArgs()));
		}
	}
}

void DialogMgr::Draw(
	UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
	for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
	{
		DialogPtrSet::iterator dlg_iter = dlg_layer_iter->second.begin();
		for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
		{
			ui_render->SetTransform((*dlg_iter)->m_World, m_View, m_Proj);

			(*dlg_iter)->Draw(ui_render, fElapsedTime);
		}
	}
}

bool DialogMgr::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	ControlPtr ControlFocus = Dialog::s_ControlFocus.lock();
	if(ControlFocus)
	{
		_ASSERT(!boost::dynamic_pointer_cast<Dialog>(ControlFocus));
		if(ControlFocus->MsgProc(hWnd, uMsg, wParam, lParam))
			return true;
	}

	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if(ControlFocus)
		{
			if(ControlFocus->HandleKeyboard(uMsg, wParam, lParam))
				return true;
		}

		if(uMsg == WM_KEYDOWN
			&& (!ControlFocus || !boost::dynamic_pointer_cast<EditBox>(ControlFocus)))
		{
			DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
			for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
			{
				DialogPtrSet::iterator dlg_iter = dlg_layer_iter->second.begin();
				for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
				{
					if((*dlg_iter)->HandleKeyboard(uMsg, wParam, lParam))
						return true;
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
		{
			Matrix4 invViewMatrix = m_View.inverse();
			const Vector3 & viewX = invViewMatrix[0].xyz;
			const Vector3 & viewY = invViewMatrix[1].xyz;
			const Vector3 & viewZ = invViewMatrix[2].xyz;
			const Vector3 & ptEye = invViewMatrix[3].xyz;

			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			Vector2 ptScreen((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f);
			Vector2 ptProj(Lerp(-1.0f, 1.0f, ptScreen.x / ClientRect.right) / m_Proj._11, Lerp(1.0f, -1.0f, ptScreen.y / ClientRect.bottom) / m_Proj._22);
			Vector3 dir = (viewX * ptProj.x + viewY * ptProj.y - viewZ).normalize();

			DialogPtrSetMap::reverse_iterator dlg_layer_iter = m_dlgSetMap.rbegin();
			for(; dlg_layer_iter != m_dlgSetMap.rend(); dlg_layer_iter++)
			{
				DialogPtrSet::reverse_iterator dlg_iter = dlg_layer_iter->second.rbegin();
				for(; dlg_iter != dlg_layer_iter->second.rend(); dlg_iter++)
				{
					// ! 只处理看得见的 Dialog
					if((*dlg_iter)->GetEnabled() && (*dlg_iter)->GetVisible())
					{
						Vector3 dialogNormal = Vector3(0, 0, 1).transformNormal((*dlg_iter)->m_World);
						float dialogDistance = ((Vector3 &)(*dlg_iter)->m_World[3]).dot(dialogNormal);
						IntersectionTests::TestResult result = IntersectionTests::rayAndHalfSpace(ptEye, dir, dialogNormal, dialogDistance);

						if(result.first)
						{
							Vector3 ptInt(ptEye + dir * result.second);
							Vector3 pt = ptInt.transformCoord((*dlg_iter)->m_World.inverse());
							Vector2 ptLocal = pt.xy - (*dlg_iter)->m_Location;
							if(ControlFocus && (*dlg_iter)->ContainsControl(ControlFocus))
							{
								// ! 只处理自己的 FocusControl
								if(ControlFocus->HandleMouse(uMsg, ptLocal, wParam, lParam))
									return true;
							}

							ControlPtr ControlPtd = (*dlg_iter)->GetControlAtPoint(ptLocal);
							if(ControlPtd && ControlPtd->GetEnabled())
							{
								if(ControlPtd->HandleMouse(uMsg, ptLocal, wParam, lParam))
								{
									(*dlg_iter)->RequestFocus(ControlPtd);
									return true;
								}
							}

							// ! 用以解决对话框控件丢失焦点
							if(uMsg == WM_LBUTTONDOWN && (*dlg_iter)->ContainsControl(ControlFocus) && !(*dlg_iter)->ContainsPoint(pt.xy))
							{
								ControlFocus->OnFocusOut();
								Dialog::s_ControlFocus.reset();
							}

							if((*dlg_iter)->HandleMouse(uMsg, pt.xy, wParam, lParam))
							{
								// ! 强制让自己具有 FocusControl
								(*dlg_iter)->ForceFocusControl();
								return true;
							}

							if(ControlPtd != (*dlg_iter)->m_ControlMouseOver)
							{
								if((*dlg_iter)->m_ControlMouseOver)
									(*dlg_iter)->m_ControlMouseOver->OnMouseLeave();

								(*dlg_iter)->m_ControlMouseOver = ControlPtd;
								if(ControlPtd)
									ControlPtd->OnMouseEnter();
							}
						}
					}
				}
			}
		}
		break;
	}

	return false;
}

void ModelViewerCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position = Vector3(0,0,-m_Distance).transform(m_Orientation) + m_LookAt;

	m_View = Matrix4::Translation(-m_LookAt)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z)
		* Matrix4::Translation(Vector3(0,0,-m_Distance));

	m_Proj = Matrix4::PerspectiveAovRH(m_Fov, m_Aspect, m_Nz, m_Fz);
}

LRESULT ModelViewerCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Rotation.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Rotation.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		m_Distance -= (short)HIWORD(wParam) / WHEEL_DELTA;
		*pbNoFurtherProcessing = true;
		return 0;
	}
	return 0;
}

void FirstPersonCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position += (m_Velocity * 5.0f * fElapsedTime).transform(m_Orientation);

	m_View = Matrix4::Translation(-m_Position)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z);

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);
}

LRESULT FirstPersonCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Rotation.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Rotation.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'W':
			m_Velocity.z = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			m_Velocity.z = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			m_Velocity.x = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			m_Velocity.x = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case VK_PRIOR:
			m_Velocity.y = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case VK_NEXT:
			m_Velocity.y = -1;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'W':
			if(m_Velocity.z < 0)
				m_Velocity.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			if(m_Velocity.z > 0)
				m_Velocity.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			if(m_Velocity.x < 0)
				m_Velocity.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			if(m_Velocity.x > 0)
				m_Velocity.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case VK_PRIOR:
			if(m_Velocity.y > 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case VK_NEXT:
			if(m_Velocity.y < 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;
	}
	return 0;
}

void DrawHelper::DrawLine(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & v0,
	const Vector3 & v1,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[2];
	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawSphere(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	DrawSpereStage(pd3dDevice, radius, 0, 20, 0, Color, world);
}

void DrawHelper::DrawBox(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & halfSize,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[8];
	v[0].x = -halfSize.x; v[0].y = -halfSize.y; v[0].z = -halfSize.z; v[0].color = Color;
	v[1].x =  halfSize.x; v[1].y = -halfSize.y; v[1].z = -halfSize.z; v[1].color = Color;
	v[2].x = -halfSize.x; v[2].y =  halfSize.y; v[2].z = -halfSize.z; v[2].color = Color;
	v[3].x =  halfSize.x; v[3].y =  halfSize.y; v[3].z = -halfSize.z; v[3].color = Color;
	v[4].x = -halfSize.x; v[4].y =  halfSize.y; v[4].z =  halfSize.z; v[4].color = Color;
	v[5].x =  halfSize.x; v[5].y =  halfSize.y; v[5].z =  halfSize.z; v[5].color = Color;
	v[6].x = -halfSize.x; v[6].y = -halfSize.y; v[6].z =  halfSize.z; v[6].color = Color;
	v[7].x =  halfSize.x; v[7].y = -halfSize.y; v[7].z =  halfSize.z; v[7].color = Color;

	unsigned short idx[12 * 2];
	int i = 0;
	idx[i++] = 0; idx[i++] = 1; idx[i++] = 1; idx[i++] = 3; idx[i++] = 3; idx[i++] = 2; idx[i++] = 2; idx[i++] = 0;
	idx[i++] = 0; idx[i++] = 6; idx[i++] = 1; idx[i++] = 7; idx[i++] = 3; idx[i++] = 5; idx[i++] = 2; idx[i++] = 4;
	idx[i++] = 6; idx[i++] = 7; idx[i++] = 7; idx[i++] = 5; idx[i++] = 5; idx[i++] = 4; idx[i++] = 4; idx[i++] = 6;

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, _countof(v), _countof(idx) / 2, idx, D3DFMT_INDEX16, v, sizeof(v[0]));
}

void DrawHelper::DrawTriangle(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & v0,
	const Vector3 & v1,
	const Vector3 & v2,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[4];
	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
	v[2].x = v2.x; v[2].y = v2.y; v[2].z = v2.z; v[2].color = Color;
	v[3] = v[0];

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, _countof(v) - 1, v, sizeof(v[0]));
}

void DrawHelper::DrawSpereStage(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	int VSTAGE_BEGIN,
	int VSTAGE_END,
	float offsetY,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	const int VSTAGE = 20;
	const int HSTAGE = 20;
	Vertex v[VSTAGE * HSTAGE * 4];
	for(int j = VSTAGE_BEGIN; j < VSTAGE_END; j++)
	{
		for(int i = 0; i < HSTAGE; i++)
		{
			float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
			float Fi[2] = {D3DX_PI / VSTAGE * j, D3DX_PI / VSTAGE * (j + 1)};
			Vertex * pv = &v[(j * HSTAGE + i) * 4];
			pv[0].x = radius * sin(Fi[0]) * cos(Theta[0]);
			pv[0].y = radius * cos(Fi[0]) + offsetY;
			pv[0].z = radius * sin(Fi[0]) * sin(Theta[0]);
			pv[0].color = Color;

			pv[1].x = radius * sin(Fi[0]) * cos(Theta[1]);
			pv[1].y = radius * cos(Fi[0]) + offsetY;
			pv[1].z = radius * sin(Fi[0]) * sin(Theta[1]);
			pv[1].color = Color;

			pv[2] = pv[0];

			pv[3].x = radius * sin(Fi[1]) * cos(Theta[0]);
			pv[3].y = radius * cos(Fi[1]) + offsetY;
			pv[3].z = radius * sin(Fi[1]) * sin(Theta[0]);
			pv[3].color = Color;
		}
	}

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawCylinderStage(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	float y0,
	float y1,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	const int HSTAGE = 20;
	Vertex v[HSTAGE * 4];
	for(int i = 0; i < HSTAGE; i++)
	{
		float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
		Vertex * pv = &v[i * 4];
		pv[0].x = radius * cos(Theta[0]);
		pv[0].y = y0;
		pv[0].z = radius * sin(Theta[0]);
		pv[0].color = Color;

		pv[1].x = radius * cos(Theta[1]);
		pv[1].y = y0;
		pv[1].z = radius * sin(Theta[1]);
		pv[1].color = Color;

		pv[2] = pv[0];

		pv[3].x = radius * cos(Theta[0]);
		pv[3].y = y1;
		pv[3].z = radius * sin(Theta[0]);
		pv[3].color = Color;
	}

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawCapsule(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	float height,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	float y0 = height * 0.5f;
	float y1 = -y0;
	DrawSpereStage(pd3dDevice, radius, 0, 10, y0, Color, world);
	DrawSpereStage(pd3dDevice, radius, 10, 20, y1, Color, world);
	DrawCylinderStage(pd3dDevice, radius, y0, y1, Color, world);
}
