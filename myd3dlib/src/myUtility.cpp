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
			Vector3 dir = (viewX * ptProj.x + viewY * ptProj.y + viewZ).normalize();

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
