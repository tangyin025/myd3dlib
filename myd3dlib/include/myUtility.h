#pragma once

#include "myResource.h"
#include "mySkeleton.h"
#include "myMesh.h"
#include "myEffect.h"
#include "myUI.h"

namespace my
{
	class BaseParameter
	{
	public:
		BaseParameter(void)
		{
		}

		virtual ~BaseParameter(void)
		{
		}

		virtual void SetParameter(my::Effect * effect, const std::string & name) const = 0;
	};

	template <class ParameterType>
	class Parameter : public BaseParameter
	{
	public:
		ParameterType value;

		Parameter(const ParameterType & _value)
			: value(_value)
		{
		}

		virtual void SetParameter(my::Effect * effect, const std::string & name) const;
	};

	typedef boost::shared_ptr<BaseParameter> BaseParameterPtr;

	class ParameterMap : public std::map<std::string, BaseParameterPtr>
	{
	public:
		void SetBool(const std::string & name, bool value);

		void SetFloat(const std::string & name, float value);

		void SetInt(const std::string & name, int value);

		void SetVector(const std::string & name, const my::Vector4 & value);

		void SetMatrix(const std::string & name, const my::Matrix4 & value);

		void SetString(const std::string & name, const std::string & value);

		void SetTexture(const std::string & name, my::BaseTexturePtr value);
	};

	class Material : public ParameterMap
	{
	public:
		my::EffectPtr m_Effect;

	public:
		Material(void)
		{
		}

		void CreateMaterialFromFile(
			LPCSTR pFilename);

		void CreateMaterialFromFileInMemory(
			LPCVOID Memory,
			DWORD SizeOfMemory);

		void ApplyParameterBlock(void);
	};

	typedef boost::shared_ptr<Material> MaterialPtr;

	class LoaderMgr
		: public ResourceMgr
		, public ID3DXInclude
	{
	protected:
		std::map<LPCVOID, CachePtr> m_cacheSet;

		CComPtr<ID3DXEffectPool> m_EffectPool;

		typedef std::map<std::string, boost::weak_ptr<DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

		DeviceRelatedResourceSet m_resourceSet;

		typedef std::map<std::string, boost::weak_ptr<OgreSkeletonAnimation> > OgreSkeletonAnimationSet;

		OgreSkeletonAnimationSet m_skeletonSet;

	public:
		LoaderMgr(void)
		{
		}

		virtual IDirect3DDevice9 * GetD3D9Device(void) = 0;

		virtual __declspec(nothrow) HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID * ppData,
			UINT * pBytes);

		virtual __declspec(nothrow) HRESULT __stdcall Close(
			LPCVOID pData);

		virtual HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		template <class ResourceType>
		boost::shared_ptr<ResourceType> GetDeviceRelatedResource(const std::string & key, bool reload)
		{
			DeviceRelatedResourceSet::const_iterator res_iter = m_resourceSet.find(key);
			if(m_resourceSet.end() != res_iter)
			{
				boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
				if(res)
				{
					if(reload)
						res->OnDestroyDevice();

					return boost::dynamic_pointer_cast<ResourceType>(res);
				}
			}

			boost::shared_ptr<ResourceType> res(new ResourceType());
			m_resourceSet[key] = res;
			return res;
		}

		TexturePtr LoadTexture(const std::string & path, bool reload = false);

		CubeTexturePtr LoadCubeTexture(const std::string & path, bool reload = false);

		OgreMeshPtr LoadMesh(const std::string & path, bool reload = false);

		OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path, bool reload = false);

		EffectPtr LoadEffect(const std::string & path, bool reload = false);

		FontPtr LoadFont(const std::string & path, int height, bool reload = false);
	};

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

	class DialogMgr
	{
	public:
		typedef std::vector<DialogPtr> DialogPtrSet;

		typedef std::map<int, DialogPtrSet> DialogPtrSetMap;

		DialogPtrSetMap m_dlgSetMap;

		Matrix4 m_View;

		Matrix4 m_Proj;

	public:
		DialogMgr(void)
		{
			SetDlgViewport(Vector2(800,600));
		}

		void SetDlgViewport(const Vector2 & vp);

		Vector2 GetDlgViewport(void) const
		{
			return Vector2(-m_View._41*2, m_View._42*2);
		}

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

	class BaseCamera
	{
	public:
		float m_Fov;

		float m_Aspect;

		float m_Nz;

		float m_Fz;

		Matrix4 m_View;

		Matrix4 m_Proj;

		ControlEvent EventAlign;

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

	public:
		Camera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: BaseCamera(Fov, Aspect, Nz, Fz)
			, m_Position(Vector3::zero)
			, m_Orientation(Quaternion::identity)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime)
		{
			m_View = Matrix4::Translation(-m_Position) * Matrix4::RotationQuaternion(m_Orientation.inverse());

			m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);
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
