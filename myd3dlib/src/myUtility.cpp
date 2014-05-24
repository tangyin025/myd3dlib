#include "StdAfx.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "rapidxml.hpp"
#include <boost/bind.hpp>
#include <fstream>

using namespace my;

void DrawHelper::BeginLine(void)
{
	m_vertices.clear();
}

void DrawHelper::EndLine(IDirect3DDevice9 * pd3dDevice, const Matrix4 & Transform)
{
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Transform);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_vertices.size() / 2, &m_vertices[0], sizeof(m_vertices[0]));
}

void DrawHelper::PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR Color)
{
	m_vertices.push_back(Vertex(v0, Color));
	m_vertices.push_back(Vertex(v1, Color));
}

void DrawHelper::PushWireAABB(const AABB & aabb, D3DCOLOR Color)
{
	Vector3 v[8] = {
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Min.z),
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Max.z),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Max.z),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Min.z),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Min.z),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Max.z),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Max.z),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Min.z),
	};
	PushLine(v[0], v[1], Color); PushLine(v[1], v[2], Color); PushLine(v[2], v[3], Color); PushLine(v[3], v[0], Color);
	PushLine(v[4], v[5], Color); PushLine(v[5], v[6], Color); PushLine(v[6], v[7], Color); PushLine(v[7], v[4], Color);
	PushLine(v[0], v[4], Color); PushLine(v[1], v[5], Color); PushLine(v[2], v[6], Color); PushLine(v[3], v[7], Color);
}

void DrawHelper::PushWireAABB(const AABB & aabb, D3DCOLOR Color, const Matrix4 & Transform)
{
	Vector3 v[8] = {
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Min.z).transformCoord(Transform),
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Min.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Min.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Min.z).transformCoord(Transform),
	};
	PushLine(v[0], v[1], Color); PushLine(v[1], v[2], Color); PushLine(v[2], v[3], Color); PushLine(v[3], v[0], Color);
	PushLine(v[4], v[5], Color); PushLine(v[5], v[6], Color); PushLine(v[6], v[7], Color); PushLine(v[7], v[4], Color);
	PushLine(v[0], v[4], Color); PushLine(v[1], v[5], Color); PushLine(v[2], v[6], Color); PushLine(v[3], v[7], Color);
}

void DrawHelper::PushGrid(float length, float linesEvery, unsigned subLines, D3DCOLOR GridColor, D3DCOLOR AxisColor)
{
	PushLine(Vector3(-length, 0, 0), Vector3( length, 0, 0), AxisColor);
	PushLine(Vector3(0, 0, -length), Vector3(0, 0,  length), AxisColor);

	float stage = linesEvery / subLines;
	for (float incre = stage; incre < length; incre += stage)
	{
		PushLine(Vector3(-length, 0,  incre), Vector3( length, 0,  incre), GridColor);
		PushLine(Vector3(-length, 0, -incre), Vector3( length, 0, -incre), GridColor);
		PushLine(Vector3( incre, 0, -length), Vector3( incre, 0,  length), GridColor);
		PushLine(Vector3(-incre, 0, -length), Vector3(-incre, 0,  length), GridColor);
	}

	PushLine(Vector3(-length, 0,  length), Vector3( length, 0,  length), GridColor);
	PushLine(Vector3(-length, 0, -length), Vector3( length, 0, -length), GridColor);
	PushLine(Vector3( length, 0, -length), Vector3( length, 0,  length), GridColor);
	PushLine(Vector3(-length, 0, -length), Vector3(-length, 0,  length), GridColor);
}

TimerPtr TimerMgr::AddTimer(float Interval, ControlEvent EventTimer)
{
	TimerPtr timer(new Timer(Interval, Interval));
	timer->m_EventTimer = EventTimer;
	InsertTimer(timer);
	return timer;
}

void TimerMgr::InsertTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(timer->m_Removed);

		m_timerSet.insert(timer);

		timer->m_Removed = false;
	}
}

void TimerMgr::RemoveTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(!timer->m_Removed);

		m_timerSet.erase(timer);

		timer->m_Removed = true;
	}
}

void TimerMgr::RemoveAllTimer(void)
{
	m_timerSet.clear();
}

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		TimerPtr timer = (*timer_iter++);
		timer->m_RemainingTime = Min(m_MaxIterCount * timer->m_Interval, timer->m_RemainingTime + fElapsedTime);
		for(int i = 0; timer->m_RemainingTime >= timer->m_Interval && !timer->m_Removed; i++)
		{
			if(timer->m_EventTimer)
				timer->m_EventTimer(m_DefaultArgs);

			timer->m_RemainingTime -= timer->m_Interval;
		}
	}
}

BaseCamera::~BaseCamera(void)
{
}

std::pair<Vector3, Vector3> BaseCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return IntersectionTests::CalculateRay(m_InverseViewProj, m_Position, pt, Vector2((float)dim.cx, (float)dim.cy));
}

void OrthoCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = Matrix4::Translation(-m_Position) * Matrix4::RotationQuaternion(m_Orientation.inverse());

	m_Proj = Matrix4::OrthoRH(m_Width, m_Height, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT OrthoCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

void Camera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = Matrix4::Translation(-m_Position) * Matrix4::RotationQuaternion(m_Orientation.inverse());

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT Camera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

void ModelViewerCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = Matrix4::Translation(-m_LookAt)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z)
		* Matrix4::Translation(Vector3(0,0,-m_Distance));

	m_Proj = Matrix4::PerspectiveAovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position = Vector3(0,0,m_Distance).transform(m_Orientation) + m_LookAt;

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
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

	Camera::OnFrameMove(fTime, fElapsedTime);
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

		case 'E':
			m_Velocity.y = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
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

		case 'E':
			if(m_Velocity.y > 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			if(m_Velocity.y < 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;
	}
	return 0;
}

void EmitterMgr::Update(
	double fTime,
	float fElapsedTime)
{
	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Update(fTime, fElapsedTime);
	}
}

void EmitterMgr::Draw(
	EmitterInstance * pInstance,
	const Matrix4 & ViewProj,
	const Quaternion & ViewOrientation,
	double fTime,
	float fElapsedTime)
{
	pInstance->SetViewProj(ViewProj);

	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Draw(pInstance, ViewOrientation, fTime, fElapsedTime);
	}
}

void EmitterMgr::InsertEmitter(EmitterPtr emitter)
{
	_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

	m_EmitterSet.insert(emitter);
}

void EmitterMgr::RemoveEmitter(EmitterPtr emitter)
{
	m_EmitterSet.erase(emitter);
}

void EmitterMgr::RemoveAllEmitter(void)
{
	m_EmitterSet.clear();
}

EffectParameterBase::~EffectParameterBase(void)
{
}

template <>
void EffectParameter<bool>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetBool(Name.c_str(), m_Value);
}

template <>
void EffectParameter<float>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetFloat(Name.c_str(), m_Value);
}

template <>
void EffectParameter<int>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetInt(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Vector4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetVector(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Matrix4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetMatrix(Name.c_str(), m_Value);
}

template <>
void EffectParameter<std::string>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetString(Name.c_str(), m_Value.c_str());
}

template <>
void EffectParameter<BaseTexturePtr>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetTexture(Name.c_str(), m_Value);
}

void EffectParameterMap::SetBool(const std::string & Name, bool Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<bool>(Value));
}

void EffectParameterMap::SetFloat(const std::string & Name, float Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<float>(Value));
}

void EffectParameterMap::SetInt(const std::string & Name, int Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<int>(Value));
}

void EffectParameterMap::SetVector(const std::string & Name, const Vector4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Vector4>(Value));
}

void EffectParameterMap::SetMatrix(const std::string & Name, const Matrix4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Matrix4>(Value));
}

void EffectParameterMap::SetString(const std::string & Name, const std::string & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<std::string>(Value));
}

void EffectParameterMap::SetTexture(const std::string & Name, BaseTexturePtr Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<BaseTexturePtr>(Value));
}

void Material::OnResetDevice(void)
{
}

void Material::OnLostDevice(void)
{
}

void Material::OnDestroyDevice(void)
{
}

class ResourceMgr::MaterialIORequest : public IORequest
{
protected:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	MaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	static void OnDiffuseTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_DiffuseTexture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	static void OnNormalTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_NormalTexture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	static void OnSpecularTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_SpecularTexture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	virtual void OnLoadDiffuseTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::OnDiffuseTextureLoaded, boundle, _1));
	}

	virtual void OnLoadNormalTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::OnNormalTextureLoaded, boundle, _1));
	}

	virtual void OnLoadSpecularTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::OnSpecularTextureLoaded, boundle, _1));
	}

	virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
	{
		boundle->m_callbacks = m_callbacks;
		m_callbacks.clear();
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		MaterialPtr res(new Material());
		ResourceCallbackBoundlePtr boundle(new ResourceCallbackBoundle(res));
		membuf mb((char *)&(*m_cache)[0], m_cache->size());
		std::istream ims(&mb);
		boost::archive::text_iarchive ia(ims);
		std::string path;
		ia >> path;
		OnLoadDiffuseTexture(boundle, path);
		ia >> path;
		OnLoadNormalTexture(boundle, path);
		ia >> path;
		OnLoadSpecularTexture(boundle, path);
		m_res = res;
		OnPostBuildResource(boundle);
	}
};

void ResourceMgr::LoadMaterialAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MaterialIORequest(callback, path, this)));
}

MaterialPtr ResourceMgr::LoadMaterial(const std::string & path)
{
	class SyncMaterialIORequest : public MaterialIORequest
	{
	public:
		SyncMaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
			: MaterialIORequest(callback, path, arc)
		{
		}

		virtual void OnLoadDiffuseTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_DiffuseTexture = m_arc->LoadTexture(path);
		}

		virtual void OnLoadNormalTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_NormalTexture = m_arc->LoadTexture(path);
		}

		virtual void OnLoadSpecularTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_SpecularTexture = m_arc->LoadTexture(path);
		}

		virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
		{
		}
	};

	return LoadResource<Material>(path, IORequestPtr(new SyncMaterialIORequest(ResourceCallback(), path, this)));
}

void ResourceMgr::SaveMaterial(const std::string & path, MaterialPtr material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::text_oarchive oa(ofs);
	oa << GetResourceKey(material->m_DiffuseTexture);
	oa << GetResourceKey(material->m_NormalTexture);
	oa << GetResourceKey(material->m_SpecularTexture);
}

class ResourceMgr::EmitterIORequest : public IORequest
{
public:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	EmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	static void OnTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Emitter>(boundle->m_res)->m_Texture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&EmitterIORequest::OnTextureLoaded, boundle, _1));
	}

	virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
	{
		boundle->m_callbacks = m_callbacks;
		m_callbacks.clear();
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		EmitterPtr res;
		ResourceCallbackBoundlePtr boundle;
		membuf mb((char *)&(*m_cache)[0], m_cache->size());
		std::istream ims(&mb);
		boost::archive::text_iarchive ia(ims);
		ia >> res;
		boundle.reset(new ResourceCallbackBoundle(res));
		std::string path;
		ia >> path;
		OnLoadTexture(boundle, path);
		m_res = res;
		OnPostBuildResource(boundle);
	}
};

void ResourceMgr::LoadEmitterAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new EmitterIORequest(callback, path, this)));
}

EmitterPtr ResourceMgr::LoadEmitter(const std::string & path)
{
	class SyncEmitterIORequest : public EmitterIORequest
	{
	public:
		SyncEmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
			: EmitterIORequest(callback, path, arc)
		{
		}

		virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Emitter>(boundle->m_res)->m_Texture = m_arc->LoadTexture(path);
		}

		virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
		{
		}
	};

	return LoadResource<Emitter>(path, IORequestPtr(new SyncEmitterIORequest(ResourceCallback(), path, this)));
}

void ResourceMgr::SaveEmitter(const std::string & path, EmitterPtr emitter)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::text_oarchive oa(ofs);
	oa << emitter;
	oa << GetResourceKey(emitter->m_Texture);
}
