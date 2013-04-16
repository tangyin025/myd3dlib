#pragma once

#include "myResource.h"
#include "mySkeleton.h"
#include "myMesh.h"
#include "myEffect.h"
#include "myEmitter.h"
#include "myUI.h"

namespace my
{
	class Timer
	{
	public:
		const float m_Interval;

		float m_RemainingTime;

		ControlEvent m_EventTimer;

		bool m_Removed;

	public:
		Timer(float Interval, float RemainingTime = 0)
			: m_Interval(Interval)
			, m_RemainingTime(Interval)
			, m_Removed(true)
		{
		}
	};

	typedef boost::shared_ptr<Timer> TimerPtr;

	class TimerMgr
	{
	protected:
		typedef std::set<TimerPtr> TimerPtrSet;

		TimerPtrSet m_timerSet;

		EventArgsPtr m_DefaultArgs;

		const int m_MaxIterCount;

	public:
		TimerMgr(void)
			: m_DefaultArgs(new EventArgs())
			, m_MaxIterCount(4)
		{
		}

		TimerPtr AddTimer(float Interval, ControlEvent EventTimer);

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
		float m_Fov;

		float m_Aspect;

		float m_Nz;

		float m_Fz;

		Matrix4 m_View;

		Matrix4 m_Proj;

	public:
		BaseCamera(float Fov, float Aspect, float Nz, float Fz)
			: m_Fov(Fov)
			, m_Aspect(Aspect)
			, m_Nz(Nz)
			, m_Fz(Fz)
			, m_View(Matrix4::identity)
			, m_Proj(Matrix4::identity)
		{
		}

		virtual ~BaseCamera(void)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime) = 0;

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing) = 0;
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		Vector3 m_Position;

		Quaternion m_Orientation;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

		ControlEvent EventAlign;

	public:
		Camera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: BaseCamera(Fov, Aspect, Nz, Fz)
			, m_Position(Vector3::zero)
			, m_Orientation(Quaternion::identity)
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
	};

	class ModelViewerCamera
		: public Camera
	{
	public:
		Vector3 m_LookAt;

		Vector3 m_Rotation;

		float m_Distance;

		bool m_bDrag;

		CPoint m_DragPos;

	public:
		ModelViewerCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Fov, Aspect, Nz, Fz)
			, m_LookAt(Vector3::zero)
			, m_Rotation(Vector3::zero)
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
	};

	class FirstPersonCamera
		: public Camera
	{
	public:
		Vector3 m_Velocity;

		Vector3 m_Rotation;

		bool m_bDrag;

		CPoint m_DragPos;

	public:
		FirstPersonCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Fov, Aspect, Nz, Fz)
			, m_Velocity(0,0,0)
			, m_Rotation(0,0,0)
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
	};

	class DialogMgr
	{
	public:
		typedef std::vector<DialogPtr> DialogPtrSet;

		typedef std::map<int, DialogPtrSet> DialogPtrSetMap;

		DialogPtrSetMap m_dlgSetMap;

		FirstPersonCamera m_Camera;

	public:
		DialogMgr(void)
		{
			SetDlgViewport(Vector2(800,600));
		}

		virtual ~DialogMgr(void)
		{
		}

		void SetDlgViewport(const Vector2 & vp);

		Vector2 GetDlgViewport(void) const;

		void Draw(
			UIRender * ui_render,
			double fTime,
			float fElapsedTime);

		bool MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam);

		void InsertDlg(DialogPtr dlg)
		{
			m_dlgSetMap[0].push_back(dlg);

			if(dlg->EventAlign)
				dlg->EventAlign(EventArgsPtr(new EventArgs()));
		}

		void RemoveDlg(DialogPtr dlg)
		{
			DialogPtrSet::iterator dlg_iter = std::find(m_dlgSetMap[0].begin(), m_dlgSetMap[0].end(), dlg);
			if(dlg_iter != m_dlgSetMap[0].end())
			{
				m_dlgSetMap[0].erase(dlg_iter);
			}
		}

		void RemoveAllDlg()
		{
			m_dlgSetMap[0].clear();
		}
	};

	class EmitterMgr
	{
	public:
		typedef std::set<EmitterPtr> EmitterPtrSet;

		EmitterPtrSet m_EmitterSet;

	public:
		EmitterMgr(void)
		{
		}

		virtual ~EmitterMgr(void)
		{
		}

		void Update(
			double fTime,
			float fElapsedTime);

		void Draw(
			EmitterInstance * pInstance,
			double fTime,
			float fElapsedTime);

		void InsertEmitter(EmitterPtr emitter)
		{
			_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

			m_EmitterSet.insert(emitter);
		}

		void RemoveEmitter(EmitterPtr emitter)
		{
			m_EmitterSet.erase(emitter);
		}

		void RemoveAllEmitter(void)
		{
			m_EmitterSet.clear();
		}
	};

	class DrawHelper
	{
	protected:
		HRESULT hr;

	public:
		static void DrawLine(
			IDirect3DDevice9 * pd3dDevice,
			const my::Vector3 & v0,
			const my::Vector3 & v1,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);

		static void DrawSphere(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);

		static void DrawBox(
			IDirect3DDevice9 * pd3dDevice,
			const my::Vector3 & halfSize,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);

		static void DrawTriangle(
			IDirect3DDevice9 * pd3dDevice,
			const my::Vector3 & v0,
			const my::Vector3 & v1,
			const my::Vector3 & v2,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);

		static void DrawSpereStage(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			int VSTAGE_BEGIN,
			int VSTAGE_END,
			float offsetY,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);

		static void DrawCylinderStage(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			float y0,
			float y1,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);

		static void DrawCapsule(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			float height,
			D3DCOLOR Color,
			const my::Matrix4 & world = my::Matrix4::identity);
	};
}
