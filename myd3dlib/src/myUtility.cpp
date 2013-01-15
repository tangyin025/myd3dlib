#include "StdAfx.h"
#include "myUtility.h"
#include "libc.h"

using namespace my;

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
			ret->CreateTextureFromFile(DxutApp::getSingleton().GetD3D9Device(), ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateTextureFromFileInMemory(DxutApp::getSingleton().GetD3D9Device(), &(*cache)[0], cache->size());
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
			ret->CreateCubeTextureFromFile(DxutApp::getSingleton().GetD3D9Device(), ms2ts(full_path.c_str()).c_str());
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateCubeTextureFromFileInMemory(DxutApp::getSingleton().GetD3D9Device(), &(*cache)[0], cache->size());
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
			ret->CreateMeshFromOgreXml(DxutApp::getSingleton().GetD3D9Device(), full_path.c_str(), true);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateMeshFromOgreXmlInMemory(DxutApp::getSingleton().GetD3D9Device(), (char *)&(*cache)[0], cache->size(), true);
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
			ret->CreateEffectFromFile(DxutApp::getSingleton().GetD3D9Device(), ms2ts(full_path.c_str()).c_str(), NULL, NULL, 0, m_EffectPool);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateEffect(DxutApp::getSingleton().GetD3D9Device(), &(*cache)[0], cache->size(), NULL, this, 0, m_EffectPool);
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
			ret->CreateFontFromFile(DxutApp::getSingleton().GetD3D9Device(), full_path.c_str(), height);
		}
		else
		{
			CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
			ret->CreateFontFromFileInCache(DxutApp::getSingleton().GetD3D9Device(), cache, height);
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

void DialogMgr::UpdateDlgViewProj(DialogPtr dlg, const Vector2 & vp)
{
	if(dlg->EventAlign)
		dlg->EventAlign(EventArgsPtr(new AlignEventArgs(vp)));
}

void DialogMgr::Draw(
	UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
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
