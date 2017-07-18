#pragma once

#include "myResource.h"
#include "mySkeleton.h"
#include "myMesh.h"
#include "myEffect.h"
#include "myEmitter.h"
#include "myUI.h"
#include "myInput.h"
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

			Vertex(const Vector3 & v, D3DCOLOR _color)
				: x(v.x), y(v.y), z(v.z), color(_color)
			{
			}
		};

		std::vector<Vertex> m_vertices;

	public:
		DrawHelper(void)
		{
		}

		void BeginLine(void);

		void EndLine(IDirect3DDevice9 * pd3dDevice, const Matrix4 & Transform = Matrix4::identity);

		void PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR Color);

		void PushWireAABB(const AABB & aabb, D3DCOLOR Color);

		void PushGrid(float length = 12, float linesEvery = 5, unsigned subLines = 5, D3DCOLOR GridColor = D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR AxisColor = D3DCOLOR_ARGB(255,0,0,0), const Matrix4 & Transform = Matrix4::identity);
	};

	typedef boost::function<void (float)> TimerEvent;

	class Timer
	{
	public:
		const float m_Interval;

		float m_RemainingTime;

		TimerEvent m_EventTimer;

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

		TimerPtr AddTimer(float Interval, TimerEvent EventTimer);

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
		Vector3 m_Eye;

		Matrix4 m_View;

		Matrix4 m_Proj;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

	public:
		BaseCamera(void)
			: m_Eye(0,0,0)
			, m_View(Matrix4::Identity())
			, m_Proj(Matrix4::Identity())
			, m_ViewProj(Matrix4::Identity())
			, m_InverseViewProj(Matrix4::Identity())
		{
		}

		virtual ~BaseCamera(void)
		{
		}

		virtual void UpdateViewProj(void)
		{
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

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim)
		{
			return Ray(Vector3::zero, Vector3::unitX);
		}

		virtual Frustum CalculateFrustum(const Rectangle & rc, const CSize & dim)
		{
			return Frustum(Plane(1,0,0,0),Plane(1,0,0,0),Plane(1,0,0,0),Plane(1,0,0,0),Plane(1,0,0,0),Plane(1,0,0,0));
		}

		virtual void OnViewportChanged(const Vector2 & Viewport)
		{
		}

		virtual float CalculateViewportScaler(Vector3 WorldPos) const
		{
			return 1;
		}

		static Vector3 ScreenToWorld(const Matrix4 & InverseViewProj, const Vector2 & pt, const Vector2 & dim, float z);

		static Ray PerspectiveRay(const Matrix4 & InverseViewProj, const Vector3 & pos, const Vector2 & pt, const Vector2 & dim);

		static Ray OrthoRay(const Matrix4 & InverseViewProj, const Vector3 & dir, const Vector2 & pt, const Vector2 & dim);

		static Frustum RectangleToFrustum(const Matrix4 & InverseViewProj, const Rectangle & rc, const Vector2 & dim);
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		float m_Aspect;

		Vector3 m_Eular;

		float m_Nz;

		float m_Fz;

		ControlEvent EventAlign;

	public:
		Camera(float Aspect, float Nz, float Fz)
			: m_Aspect(Aspect)
			, m_Eular(0,0,0)
			, m_Nz(Nz)
			, m_Fz(Fz)
		{
		}
	};

	typedef boost::shared_ptr<Camera> CameraPtr;

	class OrthoCamera
		: public Camera
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

	class ModelViewerCamera
		: public Camera
	{
	public:
		float m_Fov;

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
			: Camera(Aspect, Nz, Fz)
			, m_Fov(Fov)
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

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim);

		virtual Frustum CalculateFrustum(const Rectangle & rc, const CSize & dim);

		virtual void OnViewportChanged(const Vector2 & Viewport);

		virtual float CalculateViewportScaler(Vector3 WorldPos) const;
	};

	class FirstPersonCamera
		: public Camera
	{
	public:
		float m_Fov;

		Vector3 m_LocalVel;

		DWORD m_DragMode;

		CPoint m_DragPt;

	public:
		FirstPersonCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Aspect, Nz, Fz)
			, m_Fov(Fov)
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

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim);

		virtual Frustum CalculateFrustum(const Rectangle & rc, const CSize & dim);

		virtual void OnViewportChanged(const Vector2 & Viewport);

		virtual float CalculateViewportScaler(Vector3 WorldPos) const;
	};

	class InputMgr
		: public SingleInstance<InputMgr>
	{
	public:
		struct JoystickEnumDesc
		{
			LPDIRECTINPUT8 input;
			HWND hwnd;
			LONG min_x;
			LONG max_x;
			LONG min_y;
			LONG max_y;
			LONG min_z;
			LONG max_z;
			float dead_zone;
			JoystickPtr joystick;
		};

		InputPtr m_input;

		JoystickPtr m_joystick;

		CPoint m_MousePos;

		InputEvent m_MouseMovedEvent;

		InputEvent m_MousePressedEvent;

		InputEvent m_MouseReleasedEvent;

		InputEvent m_KeyPressedEvent;

		InputEvent m_KeyReleasedEvent;

	public:
		InputMgr(void)
		{
		}

		void Create(HINSTANCE hinst, HWND hwnd);

		void Destroy(void);

		void Update(double fTime, float fElapsedTime);

		bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static BOOL CALLBACK JoystickFinderCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	};
}
