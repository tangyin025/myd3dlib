// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "myMath.h"
#include "mySingleton.h"
#include <atlbase.h>
#include <atltypes.h>
#include <set>

namespace my
{
	class BoxPrimitive
	{
	public:
		union
		{
			struct
			{
				const Vector3 _1, _2, _3, _4, _5, _6, _7, _8;
			};

			const Vector3 v[8];
		};

		static const int i[36];

		BoxPrimitive(const AABB& aabb);

		BoxPrimitive(float hx, float hy, float hz);

		BoxPrimitive(float hx, float hy, float hz, const Matrix4& world);
	};

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

		void FlushLine(IDirect3DDevice9 * pd3dDevice);

		void PushLineVertex(float x, float y, float z, D3DCOLOR color);

		void PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR color);

		void PushLineAABB(const AABB & aabb, D3DCOLOR color);

		void PushLineBox(float hx, float hy, float hz, const Matrix4 & world, D3DCOLOR color);

		void PushLineGrid(float length = 12, float linesEvery = 5, unsigned subLines = 5, D3DCOLOR GridColor = D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR AxisColor = D3DCOLOR_ARGB(255,0,0,0), const Matrix4 & Transform = Matrix4::identity);

		void PushTriangleVertex(float x, float y, float z, D3DCOLOR color);

		void PushTriangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2, D3DCOLOR color);
	};

	class Timer
	{
	public:
		float m_RemainingTime;

	public:
		Timer(void)
			: m_RemainingTime(0)
		{
		}

		bool Step(const float interval);
	};

	typedef boost::shared_ptr<Timer> TimerPtr;

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

		Vector3 ScreenToWorld(const Vector2 & pt, const Vector2 & dim, float z) const;

		Vector3 WorldToScreen(const Vector3 & pos, const Vector2 & dim) const;

		Frustum RectangleToFrustum(const Rectangle & rc, const Vector2 & dim) const;
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		Vector3 m_Eye;

		Vector3 m_Euler;

		float m_Nz;

		float m_Fz;

	public:
		Camera(float Nz, float Fz)
			: m_Eye(0,0,0)
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

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim) const = 0;

		virtual void OnDimensionChanged(const CSize & dim) = 0;

		virtual float CalculateDimensionScaler(const Vector3 & WorldPos) const = 0;
	};

	typedef boost::shared_ptr<Camera> CameraPtr;

	class OrthoCamera : public Camera
	{
	public:
		float m_Width;

		float m_Height;

	public:
		OrthoCamera(float Width, float Height, float Nz, float Fz)
			: Camera(Nz, Fz)
			, m_Width(Width)
			, m_Height(Height)
		{
			_ASSERT(m_Width > 0 && m_Height > 0);
		}

		virtual void UpdateViewProj(void);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim) const;

		virtual void OnDimensionChanged(const CSize & dim);

		virtual float CalculateDimensionScaler(const Vector3 & WorldPos) const;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		float m_Fov;

		float m_Aspect;

	public:
		PerspectiveCamera(float Fov, float Aspect, float Nz, float Fz)
			: Camera(Nz, Fz)
			, m_Fov(Fov)
			, m_Aspect(Aspect)
		{
			_ASSERT(m_Aspect > 0);
		}

		virtual void UpdateViewProj(void);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual Ray CalculateRay(const Vector2 & pt, const CSize & dim) const;

		virtual void OnDimensionChanged(const CSize & dim);

		virtual float CalculateDimensionScaler(const Vector3 & WorldPos) const;
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
		ModelViewerCamera(float Fov, float Aspect, float Nz, float Fz)
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
		FirstPersonCamera(float Fov, float Aspect, float Nz, float Fz)
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