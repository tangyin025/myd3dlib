#pragma once

#include "myMath.h"
#include "mySingleton.h"
#include <atlbase.h>
#include <atltypes.h>
#include <set>

namespace my
{
	class DrawHelper
	{
	protected:
		struct Vertex
		{
			float x, y, z;

			D3DCOLOR color;

			Vertex(float _x, float _y, float _z, D3DCOLOR _color)
				: x(_x), y(_y), z(_z), color(_color)
			{
			}
		};

		std::vector<Vertex> m_lineVerts;

		std::vector<Vertex> m_triVerts;

	public:
		DrawHelper(void)
		{
		}

		void BeginLine(void);

		void EndLine(IDirect3DDevice9 * pd3dDevice);

		void PushLineVertex(float x, float y, float z, D3DCOLOR color);

		void PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR color);

		void PushLineAABB(const AABB & aabb, D3DCOLOR color);

		void PushLineGrid(float length = 12, float linesEvery = 5, unsigned subLines = 5, D3DCOLOR GridColor = D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR AxisColor = D3DCOLOR_ARGB(255,0,0,0), const Matrix4 & Transform = Matrix4::identity);

		void PushTriangleVertex(float x, float y, float z, D3DCOLOR color);

		void PushTriangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2, D3DCOLOR color);
	};

	class TimerEventArg : public EventArg
	{
	public:
		float m_Interval;

	public:
		TimerEventArg(float Interval)
			: m_Interval(Interval)
		{
		}
	};

	class Timer
	{
	public:
		const float m_Interval;

		float m_RemainingTime;

		EventFunction m_EventTimer;

		bool m_Managed;

	public:
		Timer(float Interval, float RemainingTime = 0)
			: m_Interval(Interval)
			, m_RemainingTime(RemainingTime)
			, m_Managed(false)
		{
		}

		void Step(float fElapsedTime, int MaxIter);
	};

	typedef boost::shared_ptr<Timer> TimerPtr;

	class TimerMgr
	{
	protected:
		typedef std::set<TimerPtr> TimerPtrSet;

		TimerPtrSet m_timerSet;

		const int m_MaxIterCount;

	public:
		TimerMgr(void)
			: m_MaxIterCount(4)
		{
		}

		void InsertTimer(TimerPtr timer);

		void RemoveTimer(TimerPtr timer);

		void RemoveAllTimer(void);

		void Update(
			double fTime,
			float fElapsedTime);
	};

	class BaseCamera
	{
	public:
		Matrix4 m_View;

		Matrix4 m_Proj;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

	public:
		BaseCamera(void)
			: m_View(Matrix4::Identity())
			, m_Proj(Matrix4::Identity())
			, m_ViewProj(Matrix4::Identity())
			, m_InverseViewProj(Matrix4::Identity())
		{
		}

		virtual ~BaseCamera(void)
		{
		}

		Vector3 ScreenToWorld(const Vector2 & pt, const Vector2 & dim, float z);

		Vector3 WorldToScreen(const Vector3 & pos, const Vector2 & dim);

		Frustum RectangleToFrustum(const Rectangle & rc, const Vector2 & dim);
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		float m_Aspect;

		Vector3 m_Eye;

		Vector3 m_Euler;

		float m_Nz;

		float m_Fz;

	public:
		Camera(float Aspect, float Nz, float Fz)
			: m_Aspect(Aspect)
			, m_Eye(0,0,0)
			, m_Euler(0,0,0)
			, m_Nz(Nz)
			, m_Fz(Fz)
		{
		}

		virtual void UpdateViewProj(void) = 0;

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing) = 0;

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim) = 0;

		virtual Frustum CalculateFrustum(const Rectangle & rc, const CSize & dim) = 0;

		virtual void OnViewportChanged(const Vector2 & Viewport) = 0;

		virtual float CalculateViewportScaler(Vector3 WorldPos) const = 0;
	};

	typedef boost::shared_ptr<Camera> CameraPtr;

	class OrthoCamera : public Camera
	{
	public:
		float m_Diagonal;

	public:
		OrthoCamera(float Diagonal, float Aspect, float Nz, float Fz)
			: Camera(Aspect, Nz, Fz)
			, m_Diagonal(Diagonal)
		{
			_ASSERT(m_Aspect != 0);
		}

		virtual void UpdateViewProj(void);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim);

		virtual Frustum CalculateFrustum(const Rectangle & rc, const CSize & dim);

		virtual void OnViewportChanged(const Vector2 & Viewport);

		virtual float CalculateViewportScaler(Vector3 WorldPos) const;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		float m_Fov;

	public:
		PerspectiveCamera(float Fov, float Aspect, float Nz, float Fz)
			: Camera(Aspect, Nz, Fz)
			, m_Fov(Fov)
		{
		}

		virtual void UpdateViewProj(void);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim);

		virtual Frustum CalculateFrustum(const Rectangle & rc, const CSize & dim);

		virtual void OnViewportChanged(const Vector2 & Viewport);

		virtual float CalculateViewportScaler(Vector3 WorldPos) const;
	};

	class ModelViewerCamera : public PerspectiveCamera
	{
	public:
		Vector3 m_LookAt;

		float m_Distance;

		enum DragMode
		{
			DragModeNone = 0,
			DragModeRotate,
			DragModeTrake,
			DragModeMove,
			DragModeZoom,
		};

		DWORD m_DragMode;

		CPoint m_DragPt;

	public:
		ModelViewerCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: PerspectiveCamera(Fov, Aspect, Nz, Fz)
			, m_LookAt(0,0,0)
			, m_Distance(0)
			, m_DragMode(DragModeNone)
		{
		}

		virtual void UpdateViewProj(void);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);
	};

	class FirstPersonCamera : public PerspectiveCamera
	{
	public:
		Vector3 m_LocalVel;

		DWORD m_DragMode;

		CPoint m_DragPt;

	public:
		FirstPersonCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: PerspectiveCamera(Fov, Aspect, Nz, Fz)
			, m_LocalVel(0,0,0)
			, m_DragMode(0)
		{
		}

		virtual void UpdateViewProj(void);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);
	};
}
