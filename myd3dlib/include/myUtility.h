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

		void PushWireAABB(const AABB & aabb, D3DCOLOR Color, const Matrix4 & Transform);

		void PushGrid(float length = 12, float linesEvery = 5, unsigned subLines = 5, D3DCOLOR GridColor = D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR AxisColor = D3DCOLOR_ARGB(255,0,0,0));
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

		void OnFrameMove(
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
		{
		}

		virtual ~BaseCamera(void)
		{
		}
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		float m_Fov;

		float m_Aspect;

		float m_Nz;

		float m_Fz;

		ControlEvent EventAlign;

	public:
		Camera(float Fov, float Aspect, float Nz, float Fz)
			: m_Fov(Fov)
			, m_Aspect(Aspect)
			, m_Nz(Nz)
			, m_Fz(Fz)
		{
		}

		virtual void Camera::OnFrameMove(
			double fTime,
			float fElapsedTime)
		{
		}
		
		virtual LRESULT Camera::MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing)
		{
			return 0;
		}

		virtual std::pair<Vector3, Vector3> CalculateRay(const Vector2 & pt, const CSize & dim)
		{
			return std::make_pair(Vector3::zero, Vector3::zero);
		}
	};

	typedef boost::shared_ptr<Camera> CameraPtr;

	class ModelViewerCamera
		: public Camera
	{
	public:
		Vector3 m_LookAt;

		Vector3 m_Eular;

		float m_Distance;

		bool m_bDrag;

		CPoint m_DragPos;

		Vector3 m_Eye;

	public:
		ModelViewerCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Fov, Aspect, Nz, Fz)
			, m_LookAt(0,0,0)
			, m_Eular(0,0,0)
			, m_Distance(0)
			, m_bDrag(false)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual std::pair<Vector3, Vector3> CalculateRay(const Vector2 & pt, const CSize & dim);
	};

	class FirstPersonCamera
		: public Camera
	{
	public:
		Vector3 m_Eye;

		Vector3 m_Eular;

		Vector3 m_LocalVel;

		bool m_bDrag;

		CPoint m_DragPos;

	public:
		FirstPersonCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Fov, Aspect, Nz, Fz)
			, m_Eye(0,0,0)
			, m_Eular(0,0,0)
			, m_LocalVel(0,0,0)
			, m_bDrag(false)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual std::pair<Vector3, Vector3> CalculateRay(const Vector2 & pt, const CSize & dim);
	};

	//class EmitterMgr
	//{
	//public:
	//	typedef std::set<EmitterPtr> EmitterPtrSet;

	//	EmitterPtrSet m_EmitterSet;

	//public:
	//	EmitterMgr(void)
	//	{
	//	}

	//	void Update(
	//		double fTime,
	//		float fElapsedTime);

	//	void Draw(
	//		ParticleInstance * pInstance,
	//		const Matrix4 & ViewProj,
	//		const Matrix4 & View,
	//		double fTime,
	//		float fElapsedTime);

	//	void InsertEmitter(EmitterPtr emitter);

	//	void RemoveEmitter(EmitterPtr emitter);

	//	void RemoveAllEmitter(void);
	//};

	//class EffectParameterBase
	//{
	//public:
	//	EffectParameterBase(void)
	//	{
	//	}

	//	virtual ~EffectParameterBase(void);

	//	virtual void SetParameter(Effect * pEffect, const std::string & Name) const = 0;
	//};

	//typedef boost::shared_ptr<EffectParameterBase> EffectParameterBasePtr;

	//template <class T>
	//class EffectParameter : public EffectParameterBase
	//{
	//public:
	//	T m_Value;

	//	EffectParameter(const T & Value)
	//		: m_Value(Value)
	//	{
	//	}

	//	virtual void SetParameter(Effect * pEffect, const std::string & Name) const;
	//};

	class InputMgr
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
