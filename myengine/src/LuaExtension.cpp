#include "LuaExtension.h"
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include "libc.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "myInput.h"
#include "LuaExtension.inl"
#include "Material.h"
#include "RenderPipeline.h"
#include "PhysxContext.h"
#include "Animation.h"
#include "Actor.h"
#include "Terrain.h"
#include "StaticEmitter.h"
#include "Controller.h"
#include "NavigationSerialization.h"
#include "ActionTrack.h"
#include "noise.h"
#include <boost/scope_exit.hpp>

static int add_file_and_line(lua_State * L)
{
   lua_Debug d;
   lua_getstack(L, 1, &d);
   lua_getinfo(L, "Sln", &d);
   std::string err = lua_tostring(L, -1);
   lua_pop(L, 1);
   std::stringstream msg;
   msg << d.short_src << ":" << d.currentline;

   if (d.name != 0)
   {
      msg << "(" << d.namewhat << " " << d.name << ")";
   }
   msg << " " << err;
   lua_pushstring(L, msg.str().c_str());
   return 1;
}

static void translate_my_exception(lua_State* L, my::Exception const & e)
{
	std::string s = e.what();
	lua_pushlstring(L, s.c_str(), s.length());
}

static DWORD ARGB(int a, int r, int g, int b)
{
	return D3DCOLOR_ARGB(a,r,g,b);
}

struct ScriptControl : my::Control, luabind::wrap_base
{
	ScriptControl(const char* Name)
		: my::Control(Name)
	{
		// ! make sure the ownership of lua part when using shared_ptr pass to Dialog::InsertControl
	}

	virtual ~ScriptControl(void)
	{
		_ASSERT(!IsRequested());
	}

	virtual DWORD GetControlType(void) const
	{
		return ControlTypeScript;
	}

	virtual void RequestResource(void)
	{
		try
		{
			luabind::wrap_base::call<void>("RequestResource");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_RequestResource(my::Control* ptr)
	{
		ptr->Control::RequestResource();
	}

	virtual void ReleaseResource(void)
	{
		try
		{
			luabind::wrap_base::call<void>("ReleaseResource");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_ReleaseResource(my::Control* ptr)
	{
		ptr->Control::ReleaseResource();
	}

	virtual void Draw(my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size)
	{
		m_Rect = my::Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		try
		{
			luabind::wrap_base::call<void>("Draw", ui_render, fElapsedTime, Offset, Size);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_Draw(my::Control* ptr, my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size)
	{
		ptr->Control::Draw(ui_render, fElapsedTime, Offset, Size);
	}

	virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		try
		{
			return luabind::wrap_base::call<bool>("MsgProc", hWnd, uMsg, wParam, lParam);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
		return 0;
	}

	static bool default_MsgProc(my::Control* ptr, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return ptr->Control::MsgProc(hWnd, uMsg, wParam, lParam);
	}

	virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		try
		{
			return luabind::wrap_base::call<bool>("HandleKeyboard", uMsg, wParam, lParam);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
		return 0;
	}

	static bool default_HandleKeyboard(my::Control* ptr, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return ptr->Control::HandleKeyboard(uMsg, wParam, lParam);
	}

	virtual bool HandleMouse(UINT uMsg, const my::Vector2& pt, WPARAM wParam, LPARAM lParam)
	{
		try
		{
			return luabind::wrap_base::call<bool>("HandleMouse", uMsg, pt, wParam, lParam);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
		return 0;
	}

	static bool default_HandleMouse(my::Control* ptr, UINT uMsg, const my::Vector2& pt, WPARAM wParam, LPARAM lParam)
	{
		return ptr->Control::HandleMouse(uMsg, pt, wParam, lParam);
	}
};

struct ScriptComponent : Component, luabind::wrap_base
{
#ifdef _DEBUG
	bool m_OnPxThreadSubstepMuted;
#endif

	ScriptComponent(const char* Name)
		: Component(Name)
#ifdef _DEBUG
		, m_OnPxThreadSubstepMuted(false)
#endif
	{
		// ! make sure the ownership of lua part when using shared_ptr pass to Actor::AddComponent
	}

	virtual ~ScriptComponent(void)
	{
		_ASSERT(!IsRequested());
	}

	virtual ComponentType GetComponentType(void) const
	{
		return ComponentTypeScript;
	}

	virtual void RequestResource(void)
	{
		try
		{
			luabind::wrap_base::call<void>("RequestResource");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_RequestResource(Component * ptr)
	{
		ptr->Component::RequestResource();

		_ASSERT(ptr->m_Actor);

		PhysxScene* scene = dynamic_cast<PhysxScene*>(ptr->m_Actor->m_Node->GetTopNode());

		scene->m_EventPxThreadSubstep.connect(boost::bind(&Component::OnPxThreadSubstep, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventEnterTrigger.connect(boost::bind(&Component::OnEnterTrigger, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventLeaveTrigger.connect(boost::bind(&Component::OnLeaveTrigger, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadShapeHit.connect(boost::bind(&Component::OnPxThreadShapeHit, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadControllerHit.connect(boost::bind(&Component::OnPxThreadControllerHit, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadObstacleHit.connect(boost::bind(&Component::OnPxThreadObstacleHit, ptr, boost::placeholders::_1));
	}

	virtual void ReleaseResource(void)
	{
		try
		{
			luabind::wrap_base::call<void>("ReleaseResource");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_ReleaseResource(Component* ptr)
	{
		ptr->Component::ReleaseResource();

		_ASSERT(ptr->m_Actor);

		PhysxScene* scene = dynamic_cast<PhysxScene*>(ptr->m_Actor->m_Node->GetTopNode());

		scene->m_EventPxThreadSubstep.disconnect(boost::bind(&Component::OnPxThreadSubstep, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventEnterTrigger.disconnect(boost::bind(&Component::OnEnterTrigger, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventLeaveTrigger.disconnect(boost::bind(&Component::OnLeaveTrigger, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadShapeHit.disconnect(boost::bind(&Component::OnPxThreadShapeHit, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadControllerHit.disconnect(boost::bind(&Component::OnPxThreadControllerHit, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadObstacleHit.disconnect(boost::bind(&Component::OnPxThreadObstacleHit, ptr, boost::placeholders::_1));
	}

	virtual void Update(float fElapsedTime)
	{
		try
		{
			luabind::wrap_base::call<void>("Update", fElapsedTime);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_Update(Component* ptr, float fElapsedTime)
	{
		ptr->Component::Update(fElapsedTime);
	}

	virtual void OnPxThreadSubstep(float dtime)
	{
		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

#ifdef _DEBUG
		m_OnPxThreadSubstepMuted = true;
		BOOST_SCOPE_EXIT(&m_OnPxThreadSubstepMuted)
		{
			m_OnPxThreadSubstepMuted = false;
		}
		BOOST_SCOPE_EXIT_END
#endif

		try
		{
			luabind::wrap_base::call<void>("OnPxThreadSubstep", dtime);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnPxThreadSubstep(Component* ptr, float dtime)
	{
		ptr->Component::OnPxThreadSubstep(dtime);
	}

	virtual void OnEnterTrigger(my::EventArg* arg)
	{
		try
		{
			luabind::wrap_base::call<void>("OnEnterTrigger", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnEnterTrigger(Component* ptr, my::EventArg* arg)
	{
		ptr->OnEnterTrigger(arg);
	}

	virtual void OnLeaveTrigger(my::EventArg* arg)
	{
		try
		{
			luabind::wrap_base::call<void>("OnLeaveTrigger", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnLeaveTrigger(Component* ptr, my::EventArg* arg)
	{
		ptr->OnLeaveTrigger(arg);
	}

	virtual void OnPxThreadShapeHit(my::EventArg* arg)
	{
		//my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		_ASSERT(m_OnPxThreadSubstepMuted);

		try
		{
			luabind::wrap_base::call<void>("OnPxThreadShapeHit", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnPxThreadShapeHit(Component* ptr, my::EventArg* arg)
	{
		ptr->OnPxThreadShapeHit(arg);
	}

	virtual void OnPxThreadControllerHit(my::EventArg* arg)
	{
		//my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		_ASSERT(m_OnPxThreadSubstepMuted);

		try
		{
			luabind::wrap_base::call<void>("OnPxThreadControllerHit", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnPxThreadControllerHit(Component* ptr, my::EventArg* arg)
	{
		ptr->OnPxThreadControllerHit(arg);
	}

	virtual void OnPxThreadObstacleHit(my::EventArg* arg)
	{
		//my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		_ASSERT(m_OnPxThreadSubstepMuted);

		try
		{
			luabind::wrap_base::call<void>("OnPxThreadObstacleHit", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnPxThreadObstacleHit(Component* ptr, my::EventArg* arg)
	{
		ptr->OnPxThreadObstacleHit(arg);
	}
};

LuaContext::LuaContext(void)
	: m_State(NULL)
{
}

void LuaContext::Init(void)
{
	m_State = luaL_newstate();
	luaL_openlibs(m_State);
	luabind::open(m_State);

	// ! 会导致内存泄漏，但可以重写 handle_exception_aux，加入 my::Exception的支持
	luabind::register_exception_handler<my::Exception>(&translate_my_exception);

	//// ! 为什么不起作用
	//set_pcall_callback(add_file_and_line);

	using namespace luabind;

	module(m_State)[
		def("Lerp", (float(*)(float, float, float))&my::Lerp<float>)

		, def("Clamp", (float(*)(float, float, float))&my::Clamp<float>)

		, def("Wrap", (float(*)(float, float, float))&my::Wrap<float>)

		, class_<my::Vector2>("Vector2")
			.def(constructor<float, float>())
			.def_readwrite("x", &my::Vector2::x)
			.def_readwrite("y", &my::Vector2::y)
			.def(self + other<const my::Vector2 &>())
			.def(self + float())
			.def(self - other<const my::Vector2 &>())
			.def(self - float())
			.def(self * other<const my::Vector2 &>())
			.def(self * float())
			.def(self / other<const my::Vector2 &>())
			.def(self / float())
			.def("cross", &my::Vector2::cross)
			.def("dot", &my::Vector2::dot)
			.property("magnitude", &my::Vector2::magnitude)
			.property("magnitudeSq", &my::Vector2::magnitudeSq)
			.def("lerp", &my::Vector2::lerp)
			.def("lerpSelf", &my::Vector2::lerpSelf, luabind::return_reference_to(_1))
			.property("normalize", &my::Vector2::normalize)
			.def("normalizeSelf", &my::Vector2::normalizeSelf, luabind::return_reference_to(_1))
			.def("transform", &my::Vector2::transform)
			.def("transformTranspose", &my::Vector2::transformTranspose)
			.def("transformCoord", &my::Vector2::transformCoord)
			.def("transformCoordTranspose", &my::Vector2::transformCoordTranspose)
			.def("transformNormal", &my::Vector2::transformNormal)
			.def("transformNormalTranspose", &my::Vector2::transformNormalTranspose)
			.def("transform", &my::Vector2::transform)
			.scope
			[
				def("RandomUnit", &my::Vector2::RandomUnit),
				def("RandomUnitCircle", &my::Vector2::RandomUnitCircle)
			]

		, class_<my::Vector3>("Vector3")
			.def(constructor<float, float, float>())
			.def_readwrite("x", &my::Vector3::x)
			.def_readwrite("y", &my::Vector3::y)
			.def_readwrite("z", &my::Vector3::z)
			.def_readwrite("xy", &my::Vector3::xy)
			.def(self + other<const my::Vector3 &>())
			.def(self + float())
			.def(self - other<const my::Vector3 &>())
			.def(self - float())
			.def(self * other<const my::Vector3 &>())
			.def(self * float())
			.def(self / other<const my::Vector3 &>())
			.def(self / float())
			.def("cross", &my::Vector3::cross)
			.def("dot", &my::Vector3::dot)
			.def("dot2D", &my::Vector3::dot2D)
			.property("magnitude", &my::Vector3::magnitude)
			.property("magnitudeSq", &my::Vector3::magnitudeSq)
			.property("magnitude2D", &my::Vector3::magnitude2D)
			.property("magnitudeSq2D", &my::Vector3::magnitudeSq2D)
			.def("lerp", &my::Vector3::lerp)
			.def("lerpSelf", &my::Vector3::lerpSelf, luabind::return_reference_to(_1))
			.property("normalize", &my::Vector3::normalize)
			.def("normalizeSelf", &my::Vector3::normalizeSelf, luabind::return_reference_to(_1))
			.def("transform", (my::Vector4(my::Vector3::*)(const my::Matrix4 &) const)&my::Vector3::transform)
			.def("transformTranspose", &my::Vector3::transformTranspose)
			.def("transformCoord", &my::Vector3::transformCoord)
			.def("transformCoordTranspose", &my::Vector3::transformCoordTranspose)
			.def("transformNormal", &my::Vector3::transformNormal)
			.def("transformNormalTranspose", &my::Vector3::transformNormalTranspose)
			.def("transform", (my::Vector3(my::Vector3::*)(const my::Quaternion &) const)&my::Vector3::transform)
			.def("cartesianToSpherical", &my::Vector3::cartesianToSpherical)
			.scope
			[
				def("SphericalToCartesian", &my::Vector3::SphericalToCartesian),
				def("Angle", &my::Vector3::Angle),
				def("CosTheta", &my::Vector3::CosTheta),
				def("Cosine", &my::Vector3::Cosine),
				def("RandomUnit", &my::Vector3::RandomUnit),
				def("RandomUnitCircle", &my::Vector3::RandomUnitSphere)
			]

		, class_<my::Vector4>("Vector4")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Vector4::x)
			.def_readwrite("y", &my::Vector4::y)
			.def_readwrite("z", &my::Vector4::z)
			.def_readwrite("w", &my::Vector4::w)
			.def_readwrite("xy", &my::Vector4::xy)
			.def_readwrite("xyz", &my::Vector4::xyz)
			.def(self + other<const my::Vector4 &>())
			.def(self + float())
			.def(self - other<const my::Vector4 &>())
			.def(self - float())
			.def(self * other<const my::Vector4 &>())
			.def(self * float())
			.def(self / other<const my::Vector4 &>())
			.def(self / float())
			.def("cross", &my::Vector4::cross)
			.def("dot", &my::Vector4::dot)
			.property("magnitude", &my::Vector4::magnitude)
			.property("magnitudeSq", &my::Vector4::magnitudeSq)
			.def("lerp", &my::Vector4::lerp)
			.def("lerpSelf", &my::Vector4::lerpSelf, luabind::return_reference_to(_1))
			.property("normalize", &my::Vector4::normalize)
			.def("normalizeSelf", &my::Vector4::normalizeSelf, luabind::return_reference_to(_1))
			.def("transform", &my::Vector4::transform)
			.def("transformTranspose", &my::Vector4::transformTranspose)

		, class_<my::Rectangle>("Rectangle")
			.def(constructor<float, float, float, float>())
			.def_readwrite("l", &my::Rectangle::l)
			.def_readwrite("t", &my::Rectangle::t)
			.def_readwrite("r", &my::Rectangle::r)
			.def_readwrite("b", &my::Rectangle::b)
			.def("intersect", &my::Rectangle::intersect)
			.def("intersectSelf", &my::Rectangle::intersectSelf, luabind::return_reference_to(_1))
			.def("union", &my::Rectangle::Union)
			.def("unionSelf", &my::Rectangle::unionSelf, luabind::return_reference_to(_1))
			.def("offset", (my::Rectangle(my::Rectangle::*)(float, float) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::offsetSelf, luabind::return_reference_to(_1))
			.def("offset", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::offsetSelf, luabind::return_reference_to(_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::shrinkSelf, luabind::return_reference_to(_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::shrinkSelf, luabind::return_reference_to(_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(float, float, float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float, float, float))&my::Rectangle::shrinkSelf, luabind::return_reference_to(_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(const my::Vector4 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector4 &))&my::Rectangle::shrinkSelf, luabind::return_reference_to(_1))
			.property("LeftTop", (my::Vector2(my::Rectangle::*)(void) const)&my::Rectangle::LeftTop)
			.property("RightBottom", (my::Vector2(my::Rectangle::*)(void) const)&my::Rectangle::RightBottom)
			.property("Center", &my::Rectangle::Center)
			.property("Width", &my::Rectangle::Width)
			.property("Height", &my::Rectangle::Height)
			.property("Extent", &my::Rectangle::Extent)
			.def("PtInRect", &my::Rectangle::PtInRect)
			.scope
			[
				def("LeftTop", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::LeftTop),
				def("LeftTop", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftTop),
				def("LeftMiddle", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::LeftMiddle),
				def("LeftMiddle", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftMiddle),
				def("LeftBottom", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::LeftBottom),
				def("LeftBottom", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftBottom),
				def("CenterTop", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::CenterTop),
				def("CenterTop", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterTop),
				def("CenterMiddle", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::CenterMiddle),
				def("CenterMiddle", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterMiddle),
				def("CenterBottom", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::CenterBottom),
				def("CenterBottom", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterBottom),
				def("RightTop", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::RightTop),
				def("RightTop", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightTop),
				def("RightMiddle", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::RightMiddle),
				def("RightMiddle", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightMiddle),
				def("RightBottom", (my::Rectangle(*)(float, float, float, float))&my::Rectangle::RightBottom),
				def("RightBottom", (my::Rectangle(*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightBottom)
			]

		, class_<my::Quaternion>("Quaternion")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Quaternion::x)
			.def_readwrite("y", &my::Quaternion::y)
			.def_readwrite("z", &my::Quaternion::z)
			.def_readwrite("w", &my::Quaternion::w)
			.def_readwrite("xyz", &my::Quaternion::xyz)
			.def(self + other<const my::Quaternion &>())
			.def(self + float())
			.def(self - other<const my::Quaternion &>())
			.def(self - float())
			.def(self * other<const my::Quaternion &>())
			.def(self * other<const my::Vector3 &>())
			.def(self * float())
			.def(self / other<const my::Quaternion &>())
			.def(self / float())
			.def("conjugate", &my::Quaternion::conjugate)
			.def("conjugateSelf", &my::Quaternion::conjugateSelf, luabind::return_reference_to(_1))
			.def("dot", &my::Quaternion::dot)
			.property("inverse", &my::Quaternion::inverse)
			.property("magnitude", &my::Quaternion::magnitude)
			.property("magnitudeSq", &my::Quaternion::magnitudeSq)
			.property("ln", &my::Quaternion::ln)
			.def("multiply", &my::Quaternion::multiply)
			.property("normalize", &my::Quaternion::normalize)
			.def("normalizeSelf", &my::Quaternion::normalizeSelf, luabind::return_reference_to(_1))
			.def("lerp", &my::Quaternion::lerp)
			.def("lerpSelf", &my::Quaternion::lerpSelf, luabind::return_reference_to(_1))
			.def("slerp", &my::Quaternion::slerp)
			.def("slerpSelf", &my::Quaternion::slerpSelf, luabind::return_reference_to(_1))
			.def("squad", &my::Quaternion::squad)
			.def("squadSelf", &my::Quaternion::squadSelf, luabind::return_reference_to(_1))
			.def("toAxisAngle", &my::Quaternion::toAxisAngle, luabind::pure_out_value(_2) + luabind::pure_out_value(_3))
			.property("EulerAngles", &my::Quaternion::toEulerAngles)
			.scope
			[
				def("Identity", &my::Quaternion::Identity),
				def("RotationAxis", &my::Quaternion::RotationAxis),
				def("RotationMatrix", &my::Quaternion::RotationMatrix),
				def("RotationYawPitchRoll", &my::Quaternion::RotationYawPitchRoll),
				def("RotationFromTo", &my::Quaternion::RotationFromTo),
				def("RotationEulerAngles", &my::Quaternion::RotationEulerAngles)
			]

		, class_<my::Matrix4>("Matrix4")
			.def(self + other<const my::Matrix4 &>())
			.property("row0", &my::Matrix4::getRow<0>, &my::Matrix4::setRow<0>/*, luabind::return_reference_to(_1)*/)
			.property("row1", &my::Matrix4::getRow<1>, &my::Matrix4::setRow<1>/*, luabind::return_reference_to(_1)*/)
			.property("row2", &my::Matrix4::getRow<2>, &my::Matrix4::setRow<2>/*, luabind::return_reference_to(_1)*/)
			.property("row3", &my::Matrix4::getRow<3>, &my::Matrix4::setRow<3>/*, luabind::return_reference_to(_1)*/)
			.property("column0", &my::Matrix4::getColumn<0>, &my::Matrix4::setColumn<0>)
			.property("column1", &my::Matrix4::getColumn<1>, &my::Matrix4::setColumn<1>)
			.property("column2", &my::Matrix4::getColumn<2>, &my::Matrix4::setColumn<2>)
			.property("column3", &my::Matrix4::getColumn<3>, &my::Matrix4::setColumn<3>)
			.def(self + float())
			.def(self - other<const my::Matrix4 &>())
			.def(self - float())
			.def(self * other<const my::Matrix4 &>())
			.def(self * float())
			.def(self / other<const my::Matrix4 &>())
			.def(self / float())
			.property("inverse", &my::Matrix4::inverse)
			.def("multiply", &my::Matrix4::multiply)
			.def("multiplyTranspose", &my::Matrix4::multiplyTranspose)
			.property("transpose", &my::Matrix4::transpose)
			.def("transformTranspose", &my::Matrix4::transformTranspose)
			.def("scale", (my::Matrix4(my::Matrix4::*)(float, float, float) const)&my::Matrix4::scale)
			.def("scale", (my::Matrix4(my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::scale)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::scaleSelf, luabind::return_reference_to(_1))
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::scaleSelf, luabind::return_reference_to(_1))
			.def("rotateX", &my::Matrix4::rotateX)
			.def("rotateY", &my::Matrix4::rotateY)
			.def("rotateZ", &my::Matrix4::rotateZ)
			.def("rotate", &my::Matrix4::rotate)
			.def("translate", (my::Matrix4(my::Matrix4::*)(float, float, float) const)&my::Matrix4::translate)
			.def("translate", (my::Matrix4(my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::translate)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::translateSelf, luabind::return_reference_to(_1))
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::translateSelf, luabind::return_reference_to(_1))
			.def("lerp", &my::Matrix4::lerp)
			.def("lerpSelf", &my::Matrix4::lerpSelf, luabind::return_reference_to(_1))
			.property("EulerAngles", &my::Matrix4::toEulerAngles)
			.scope
			[
				def("Compose", (my::Matrix4(*)(const my::Vector3 &, const my::Quaternion &, const my::Vector3 &))&my::Matrix4::Compose),
				def("Identity", &my::Matrix4::Identity),
				def("LookAtLH", &my::Matrix4::LookAtLH),
				def("LookAtRH", &my::Matrix4::LookAtRH),
				def("OrthoLH", &my::Matrix4::OrthoLH),
				def("OrthoRH", &my::Matrix4::OrthoRH),
				def("OrthoOffCenterLH", &my::Matrix4::OrthoOffCenterLH),
				def("OrthoOffCenterRH", &my::Matrix4::OrthoOffCenterRH),
				def("PerspectiveFovLH", &my::Matrix4::PerspectiveFovLH),
				def("PerspectiveFovRH", &my::Matrix4::PerspectiveFovRH),
				def("PerspectiveLH", &my::Matrix4::PerspectiveLH),
				def("PerspectiveRH", &my::Matrix4::PerspectiveRH),
				def("PerspectiveOffCenterLH", &my::Matrix4::PerspectiveOffCenterLH),
				def("PerspectiveOffCenterRH", &my::Matrix4::PerspectiveOffCenterRH),
				def("RotationAxis", &my::Matrix4::RotationAxis),
				def("RotationQuaternion", &my::Matrix4::RotationQuaternion),
				def("RotationX", &my::Matrix4::RotationX),
				def("RotationY", &my::Matrix4::RotationY),
				def("RotationZ", &my::Matrix4::RotationZ),
				def("RotationYawPitchRoll", &my::Matrix4::RotationYawPitchRoll),
				def("Scaling", (my::Matrix4(*)(float, float, float))&my::Matrix4::Scaling),
				def("Scaling", (my::Matrix4(*)(const my::Vector3 &))&my::Matrix4::Scaling),
				def("Transformation", &my::Matrix4::Transformation),
				def("Transformation2D", &my::Matrix4::Transformation2D),
				def("Translation", (my::Matrix4(*)(float, float, float))&my::Matrix4::Translation),
				def("Translation", (my::Matrix4(*)(const my::Vector3 &))&my::Matrix4::Translation)
			]

		, class_<my::Ray>("Ray")
			.def(constructor<const my::Vector3 &, const my::Vector3 &>())
			.def_readwrite("p", &my::Ray::p)
			.def_readwrite("d", &my::Ray::d)
			.def("transform", &my::Ray::transform)
			.def("transformSelf", &my::Ray::transformSelf)

		, class_<my::RayResult>("RayResult")
			.def(constructor<bool, float>())
			.def_readonly("first", &my::RayResult::first)
			.def_readonly("second", &my::RayResult::second)

		, class_<my::AABB>("AABB")
			.def(constructor<float, float>())
			.def(constructor<float, float, float, float, float, float>())
			.def(constructor<const my::Vector3 &, const my::Vector3 &>())
			.def_readwrite("min", &my::AABB::m_min)
			.def_readwrite("max", &my::AABB::m_max)
			.def("IsValid", &my::AABB::IsValid)
			.def("valid", &my::AABB::valid)
			.def("validSelf", &my::AABB::validSelf, luabind::return_reference_to(_1))
			.def("Center", &my::AABB::Center)
			.def("Extent", &my::AABB::Extent)
			.def("PtInRect", &my::AABB::PtInRect)
			.def("SlicePxPyPz", &my::AABB::Slice<my::AABB::QuadrantPxPyPz>)
			.def("Intersect", (bool (my::AABB::*)(const my::Vector3&) const)& my::AABB::Intersect)
			.def("Intersect2D", &my::AABB::Intersect2D)
			.def("Intersect", (my::AABB (my::AABB::*)(const my::AABB&) const)& my::AABB::Intersect)
			.def("intersectSelf", &my::AABB::intersectSelf)
			.def("union", (my::AABB (my::AABB::*)(const my::Vector3&) const)& my::AABB::Union)
			.def("unionSelf", (my::AABB& (my::AABB::*)(const my::Vector3&))&my::AABB::unionSelf, luabind::return_reference_to(_1))
			.def("union", (my::AABB (my::AABB::*)(const my::AABB&) const)&my::AABB::Union)
			.def("unionSelf", (my::AABB& (my::AABB::*)(const my::AABB&))&my::AABB::unionSelf, luabind::return_reference_to(_1))
			.def("shrink", &my::AABB::shrink)
			.def("shrinkSelf", &my::AABB::shrinkSelf, luabind::return_reference_to(_1))
			.def("p", &my::AABB::p)
			.def("n", &my::AABB::n)
			.def("transform", &my::AABB::transform)
			.def("transformSelf", &my::AABB::transformSelf, luabind::return_reference_to(_1))

		, class_<my::UDim>("UDim")
			.def(constructor<float, float>())
			.def_readwrite("scale", &my::UDim::scale)
			.def_readwrite("offset", &my::UDim::offset)

		, class_<my::Spline>("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("Interpolate", (float (my::Spline::*)(float, float) const)&my::Spline::Interpolate)

		, class_<my::Emitter::Particle>("EmitterParticle")
			.def_readwrite("Position", &my::Emitter::Particle::m_Position)

		, class_<my::Emitter>("Emitter")
			.def(constructor<unsigned int>())
			.def_readonly("ParticleList", &my::Emitter::m_ParticleList, luabind::return_stl_iterator)
			.property("Capacity", &my::Emitter::GetCapacity, &my::Emitter::SetCapacity)
			.def("Spawn", &my::Emitter::Spawn)
			.def("RemoveAllParticle", &my::Emitter::RemoveAllParticle)

		, class_<my::BaseCamera, boost::shared_ptr<my::BaseCamera> >("BaseCamera")
			.def_readonly("View", &my::BaseCamera::m_View)
			.def_readonly("Proj", &my::BaseCamera::m_Proj)
			.def_readonly("ViewProj", &my::BaseCamera::m_ViewProj)
			.def_readonly("InverseViewProj", &my::BaseCamera::m_InverseViewProj)

		, class_<my::Camera, my::BaseCamera, boost::shared_ptr<my::Camera> >("Camera")
			.def_readwrite("Aspect", &my::Camera::m_Aspect)
			.def_readwrite("Eye", &my::Camera::m_Eye)
			.def_readwrite("Euler", &my::Camera::m_Euler)
			.def_readwrite("Nz", &my::Camera::m_Nz)
			.def_readwrite("Fz", &my::Camera::m_Fz)

		, class_<my::OrthoCamera, my::Camera, boost::shared_ptr<my::Camera> >("OrthoCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Diagonal", &my::OrthoCamera::m_Diagonal)

		, class_<my::PerspectiveCamera, my::Camera, boost::shared_ptr<my::Camera> >("PerspectiveCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Fov", &my::PerspectiveCamera::m_Fov)

		, class_<my::ModelViewerCamera, my::PerspectiveCamera, boost::shared_ptr<my::Camera> >("ModelViewerCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("LookAt", &my::ModelViewerCamera::m_LookAt)
			.def_readwrite("Distance", &my::ModelViewerCamera::m_Distance)

		, class_<my::FirstPersonCamera, my::PerspectiveCamera, boost::shared_ptr<my::Camera> >("FirstPersonCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("LocalVel", &my::FirstPersonCamera::m_LocalVel)

		, class_<my::OctEntity>("OctEntity")
			.def_readonly("OctAabb", &my::OctEntity::m_OctAabb)
	];

	module(m_State)[
		class_<D3DLOCKED_RECT>("D3DLOCKED_RECT")
			.def_readonly("Pitch", &D3DLOCKED_RECT::Pitch)
			.def_readonly("pBits", &D3DLOCKED_RECT::pBits)

		, class_<my::DeviceResourceBase, boost::intrusive_ptr<my::DeviceResourceBase> >("DeviceResourceBase")
			.def_readonly("Key", &my::DeviceResourceBase::m_Key)

		, class_<my::BaseTexture, my::DeviceResourceBase, boost::intrusive_ptr<my::DeviceResourceBase> >("BaseTexture")

		, class_<my::Texture2D, my::BaseTexture, boost::intrusive_ptr<my::DeviceResourceBase> >("Texture2D")
			.def("GetLevelDesc", &my::Texture2D::GetLevelDesc)
			.def("LockRect", &my::Texture2D::LockRect)
			.def("UnlockRect", &my::Texture2D::UnlockRect)

		, class_<my::CubeTexture, my::BaseTexture, boost::intrusive_ptr<my::DeviceResourceBase> >("CubeTexture")

		, class_<my::Mesh, my::DeviceResourceBase, boost::intrusive_ptr<my::DeviceResourceBase> >("Mesh")
			.property("NumFaces", &my::Mesh::GetNumFaces)
			.property("NumVertices", &my::Mesh::GetNumVertices)

		, class_<my::OgreMesh, my::Mesh, boost::intrusive_ptr<my::DeviceResourceBase> >("OgreMesh")
			.def("SaveOgreMesh", &my::OgreMesh::SaveOgreMesh)
			.def("SaveSimplifiedOgreMesh", &my::OgreMesh::SaveSimplifiedOgreMesh)
			.def("Transform", &my::OgreMesh::Transform)
			.property("MaterialNum", &my::OgreMesh::GetMaterialNum)
			.def("GetMaterialName", &my::OgreMesh::GetMaterialName)

		, class_<my::BoneHierarchy>("BoneHierarchy")

		, class_<my::OgreSkeletonAnimation, my::DeviceResourceBase, boost::intrusive_ptr<my::DeviceResourceBase> >("OgreSkeletonAnimation")
			.def_readonly("boneHierarchy", &my::OgreSkeletonAnimation::m_boneHierarchy)
			.def("GetBoneIndex", &my::OgreSkeletonAnimation::GetBoneIndex)
			.def("AddOgreSkeletonAnimationFromFile", &my::OgreSkeletonAnimation::AddOgreSkeletonAnimationFromFile)
			.def("SaveOgreSkeletonAnimation", &my::OgreSkeletonAnimation::SaveOgreSkeletonAnimation)
			.def("Transform", &my::OgreSkeletonAnimation::Transform)

		// ! many methods of my::BaseEffect, my::Effect cannot be use in lua
		, class_<my::BaseEffect, my::DeviceResourceBase, boost::intrusive_ptr<my::DeviceResourceBase> >("BaseEffect")
			//.def("GetAnnotation", &my::BaseEffect::GetAnnotation)
			//.def("GetAnnotationByName", &my::BaseEffect::GetAnnotationByName)
			//.def("GetBool", &my::BaseEffect::GetBool)
			//.def("GetBoolArray", &my::BaseEffect::GetBoolArray)
			//.def("GetDesc", &my::BaseEffect::GetDesc)
			//.def("GetFloat", &my::BaseEffect::GetFloat)
			//.def("GetFloatArray", &my::BaseEffect::GetFloatArray)
			//.def("GetFunction", &my::BaseEffect::GetFunction)
			//.def("GetFunctionByName", &my::BaseEffect::GetFunctionByName)
			//.def("GetFunctionDesc", &my::BaseEffect::GetFunctionDesc)
			//.def("GetInt", &my::BaseEffect::GetInt)
			//.def("GetIntArray", &my::BaseEffect::GetIntArray)
			//.def("GetMatrix", &my::BaseEffect::GetMatrix)
			//.def("GetMatrixArray", &my::BaseEffect::GetMatrixArray)
			//.def("GetMatrixPointerArray", &my::BaseEffect::GetMatrixPointerArray)
			//.def("GetMatrixTranspose", &my::BaseEffect::GetMatrixTranspose)
			//.def("GetMatrixTransposeArray", &my::BaseEffect::GetMatrixTransposeArray)
			//.def("GetMatrixTransposePointerArray", &my::BaseEffect::GetMatrixTransposePointerArray)
			//.def("GetParameter", &my::BaseEffect::GetParameter)
			//.def("GetParameterByName", &my::BaseEffect::GetParameterByName)
			//.def("GetParameterBySemantic", &my::BaseEffect::GetParameterBySemantic)
			//.def("GetParameterDesc", &my::BaseEffect::GetParameterDesc)
			//.def("GetParameterElement", &my::BaseEffect::GetParameterElement)
			//.def("GetPass", &my::BaseEffect::GetPass)
			//.def("GetPassByName", &my::BaseEffect::GetPassByName)
			//.def("GetPassDesc", &my::BaseEffect::GetPassDesc)
			//.def("GetPixelShader", &my::BaseEffect::GetPixelShader)
			//.def("GetString", &my::BaseEffect::GetString)
			//.def("GetTechnique", &my::BaseEffect::GetTechnique)
			//.def("GetTechniqueByName", &my::BaseEffect::GetTechniqueByName)
			//.def("GetTechniqueDesc", &my::BaseEffect::GetTechniqueDesc)
			//.def("GetTexture", &my::BaseEffect::GetTexture)
			//.def("GetValue", &my::BaseEffect::GetValue)
			//.def("GetVector", &my::BaseEffect::GetVector)
			//.def("GetVectorArray", &my::BaseEffect::GetVectorArray)
			//.def("GetVertexShader", &my::BaseEffect::GetVertexShader)
			//.def("SetArrayRange", &my::BaseEffect::SetArrayRange)
			//.def("SetBool", &my::BaseEffect::SetBool)
			//.def("SetBoolArray", &my::BaseEffect::SetBoolArray)
			//.def("SetFloat", &my::BaseEffect::SetFloat)
			//.def("SetFloatArray", &my::BaseEffect::SetFloatArray)
			//.def("SetInt", &my::BaseEffect::SetInt)
			//.def("SetIntArray", &my::BaseEffect::SetIntArray)
			//.def("SetMatrix", &my::BaseEffect::SetMatrix)
			//.def("SetMatrixArray", &my::BaseEffect::SetMatrixArray)
			//.def("SetMatrixPointerArray", &my::BaseEffect::SetMatrixPointerArray)
			//.def("SetMatrixTranspose", &my::BaseEffect::SetMatrixTranspose)
			//.def("SetMatrixTransposeArray", &my::BaseEffect::SetMatrixTransposeArray)
			//.def("SetMatrixTransposePointerArray", &my::BaseEffect::SetMatrixTransposePointerArray)
			//.def("SetString", &my::BaseEffect::SetString)
			//// ! luabind cannot convert boost::shared_ptr<Derived Class> to base ptr
			//.def("SetTexture", &my::BaseEffect::SetTexture)
			//.def("SetValue", &my::BaseEffect::SetValue)
			//.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector4 &))&my::BaseEffect::SetVector)
			//.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector3 &))&my::BaseEffect::SetVector)
			//.def("SetVectorArray", &my::BaseEffect::SetVectorArray)

		, class_<my::Effect, my::BaseEffect, boost::intrusive_ptr<my::DeviceResourceBase> >("Effect")
			//.def("ApplyParameterBlock", &my::Effect::ApplyParameterBlock)
			//.def("Begin", &my::Effect::Begin)
			//.def("BeginParameterBlock", &my::Effect::BeginParameterBlock)
			//.def("BeginPass", &my::Effect::BeginPass)
			//.def("CloneEffect", &my::Effect::CloneEffect)
			//.def("CommitChanges", &my::Effect::CommitChanges)
			//.def("DeleteParameterBlock", &my::Effect::DeleteParameterBlock)
			//.def("End", &my::Effect::End)
			//.def("EndParameterBlock", &my::Effect::EndParameterBlock)
			//.def("EndPass", &my::Effect::EndPass)
			//.def("FindNextValidTechnique", &my::Effect::FindNextValidTechnique)
			//.def("GetCurrentTechnique", &my::Effect::GetCurrentTechnique)
			//.def("GetDevice", &my::Effect::GetDevice)
			//.def("GetPool", &my::Effect::GetPool)
			//.def("GetStateManager", &my::Effect::GetStateManager)
			//.def("IsParameterUsed", &my::Effect::IsParameterUsed)
			//.def("SetRawValue", &my::Effect::SetRawValue)
			//.def("SetStateManager", &my::Effect::SetStateManager)
			//.def("SetTechnique", &my::Effect::SetTechnique)
			//.def("ValidateTechnique", &my::Effect::ValidateTechnique)

		, class_<my::Font, my::DeviceResourceBase, boost::intrusive_ptr<my::DeviceResourceBase> >("Font")
			.enum_("Align")
			[
				value("AlignLeft", my::Font::AlignLeft),
				value("AlignCenter", my::Font::AlignCenter),
				value("AlignRight", my::Font::AlignRight),
				value("AlignTop", my::Font::AlignTop),
				value("AlignMiddle", my::Font::AlignMiddle),
				value("AlignBottom", my::Font::AlignBottom),
				value("AlignLeftTop", my::Font::AlignLeftTop),
				value("AlignCenterTop", my::Font::AlignCenterTop),
				value("AlignRightTop", my::Font::AlignRightTop),
				value("AlignLeftMiddle", my::Font::AlignLeftMiddle),
				value("AlignCenterMiddle", my::Font::AlignCenterMiddle),
				value("AlignRightMiddle", my::Font::AlignRightMiddle),
				value("AlignLeftBottom", my::Font::AlignLeftBottom),
				value("AlignCenterBottom", my::Font::AlignCenterBottom),
				value("AlignRightBottom", my::Font::AlignRightBottom)
			]
			.def_readonly("Height", &my::Font::m_Height)
			.def_readonly("LineHeight", &my::Font::m_LineHeight)

		, class_<my::ResourceMgr>("ResourceMgr")
			.def("CheckIORequests", &my::ResourceMgr::CheckIORequests)
			.def("LoadTexture", &my::ResourceMgr::LoadTexture)
			.def("LoadTextureAsync", &my::ResourceMgr::LoadTextureAsync<luabind::object>)
			.def("LoadMesh", &my::ResourceMgr::LoadMesh)
			.def("LoadMeshAsync", &my::ResourceMgr::LoadMeshAsync<luabind::object>)
			.def("LoadSkeleton", &my::ResourceMgr::LoadSkeleton)
			.def("LoadSkeletonAsync", &my::ResourceMgr::LoadSkeletonAsync<luabind::object>)
			.def("LoadEffect", &my::ResourceMgr::LoadEffect)
			.def("LoadEffectAsync", &my::ResourceMgr::LoadEffectAsync<luabind::object>)
			.def("LoadFont", &my::ResourceMgr::LoadFont)
			.def("LoadFontAsync", &my::ResourceMgr::LoadFontAsync<luabind::object>)

		//, def("res2texture", (boost::shared_ptr<my::BaseTexture>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::BaseTexture, my::DeviceResourceBase>)
		//, def("res2mesh", (boost::shared_ptr<my::Mesh>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::Mesh, my::DeviceResourceBase>)
		//, def("res2skeleton", (boost::shared_ptr<my::OgreSkeletonAnimation>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::OgreSkeletonAnimation, my::DeviceResourceBase>)
		//, def("res2effect", (boost::shared_ptr<my::Effect>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::Effect, my::DeviceResourceBase>)
		//, def("res2font", (boost::shared_ptr<my::Font>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::Font, my::DeviceResourceBase>)
	];

	module(m_State)[
		def("ARGB", &ARGB)

		, class_<WPARAM>("WPARAM")

		, class_<LPARAM>("LPARAM")

		, class_<my::NamedObject>("NamedObject")
			.scope
			[
				def("MakeUniqueName", &my::NamedObject::MakeUniqueName)
			]
			.property("Name", &my::NamedObject::GetName, &my::NamedObject::SetName)

		, class_<my::EventArg>("EventArg")

		, class_<my::EventFunction>("EventFunction")
			.def("clear", &my::EventFunction::clear)

		, class_<my::ControlEventArg, my::EventArg>("ControlEventArg")
			.def_readonly("sender", &my::ControlEventArg::sender)

		, class_<my::VisibleEventArg, my::ControlEventArg>("VisibleEventArg")
			.def_readonly("Visible", &my::VisibleEventArg::Visible)

		, class_<my::MouseEventArg, my::ControlEventArg>("MouseEventArg")
			.def_readonly("pt", &my::MouseEventArg::pt)

		, class_<my::UIRender>("UIRender")
			.enum_("UILayerType")
			[
				value("UILayerTexture", my::UIRender::UILayerTexture),
				value("UILayerFont", my::UIRender::UILayerFont)
			]
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, const my::Rectangle&, D3DCOLOR, my::BaseTexture*, my::UIRender::UILayerType))& my::UIRender::PushRectangle)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, const my::Rectangle&, D3DCOLOR, my::BaseTexture*, const my::Rectangle&, my::UIRender::UILayerType))& my::UIRender::PushRectangle)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, const my::Rectangle&, D3DCOLOR, my::BaseTexture*, const my::Matrix4&, my::UIRender::UILayerType))& my::UIRender::PushRectangle)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, const my::Rectangle&, D3DCOLOR, my::BaseTexture*, const my::Matrix4&, const my::Rectangle&, my::UIRender::UILayerType))& my::UIRender::PushRectangle)
			.def("PushWindow", (void (my::UIRender::*)(const my::Rectangle&, DWORD, const my::Rectangle&, const my::Vector4&, const CSize&, my::BaseTexture*, my::UIRender::UILayerType))& my::UIRender::PushWindow)
			.def("PushWindow", (void (my::UIRender::*)(const my::Rectangle&, DWORD, const my::Rectangle&, const my::Vector4&, const CSize&, my::BaseTexture*, const my::Rectangle&, my::UIRender::UILayerType))& my::UIRender::PushWindow)

		, class_<my::ControlImage, boost::shared_ptr<my::ControlImage> >("ControlImage")
			.def(constructor<>())
			.def_readwrite("TexturePath", &my::ControlImage::m_TexturePath)
			.def_readwrite("Rect", &my::ControlImage::m_Rect)
			.def_readwrite("Border", &my::ControlImage::m_Border)
			.def("Clone", &my::ControlImage::Clone)
			.def("RequestResource", &my::ControlImage::RequestResource)
			.def("ReleaseResource", &my::ControlImage::ReleaseResource)
			.def("Draw", (void (my::ControlImage::*)(my::UIRender*, const my::Rectangle&, DWORD))&my::ControlImage::Draw)
			.def("Draw", (void (my::ControlImage::*)(my::UIRender*, const my::Rectangle&, DWORD, const my::Rectangle&))& my::ControlImage::Draw)

		, class_<my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ControlSkin")
			.def(constructor<>())
			.def_readwrite("Color", &my::ControlSkin::m_Color)
			.def_readwrite("Image", &my::ControlSkin::m_Image)
			.def_readwrite("FontPath", &my::ControlSkin::m_FontPath)
			.def_readwrite("FontHeight", &my::ControlSkin::m_FontHeight)
			.def_readwrite("FontFaceIndex", &my::ControlSkin::m_FontFaceIndex)
			.def_readwrite("TextColor", &my::ControlSkin::m_TextColor)
			.def_readwrite("TextAlign", &my::ControlSkin::m_TextAlign)
			.def_readwrite("VisibleShowSound", &my::ControlSkin::m_VisibleShowSound)
			.def_readwrite("VisibleHideSound", &my::ControlSkin::m_VisibleHideSound)
			.def_readwrite("MouseEnterSound", &my::ControlSkin::m_MouseEnterSound)
			.def_readwrite("MouseLeaveSound", &my::ControlSkin::m_MouseLeaveSound)
			.def_readwrite("MouseClickSound", &my::ControlSkin::m_MouseClickSound)
			.def("Clone", &my::ControlSkin::Clone)

		, class_<my::Control, ScriptControl, my::NamedObject, boost::shared_ptr<my::Control> >("Control")
			.def(constructor<const char *>())
			.enum_("ControlType")
			[
				value("ControlTypeControl", my::Control::ControlTypeControl),
				value("ControlTypeStatic", my::Control::ControlTypeStatic),
				value("ControlTypeProgressBar", my::Control::ControlTypeProgressBar),
				value("ControlTypeButton", my::Control::ControlTypeButton),
				value("ControlTypeEditBox", my::Control::ControlTypeEditBox),
				value("ControlTypeImeEditBox", my::Control::ControlTypeImeEditBox),
				value("ControlTypeScrollBar", my::Control::ControlTypeScrollBar),
				value("ControlTypeCheckBox", my::Control::ControlTypeCheckBox),
				value("ControlTypeComboBox", my::Control::ControlTypeComboBox),
				value("ControlTypeListBox", my::Control::ControlTypeListBox),
				value("ControlTypeDialog", my::Control::ControlTypeDialog),
				value("ControlTypeScript", my::Control::ControlTypeScript)
			]
			.property("ControlType", &my::Control::GetControlType)
			.def_readonly("Childs", &my::Control::m_Childs, luabind::return_stl_iterator)
			.def_readonly("Parent", &my::Control::m_Parent)
			.def_readwrite("x", &my::Control::m_x)
			.def_readwrite("y", &my::Control::m_y)
			.def_readwrite("Width", &my::Control::m_Width)
			.def_readwrite("Height", &my::Control::m_Height)
			.def_readonly("Rect", &my::Control::m_Rect)
			.def_readwrite("Skin", &my::Control::m_Skin)
			.def_readwrite("Pressed", &my::Control::m_bPressed)
			.def_readwrite("EventVisibleChanged", &my::Control::m_EventVisibleChanged)
			.def_readwrite("EventMouseEnter", &my::Control::m_EventMouseEnter)
			.def_readwrite("EventMouseLeave", &my::Control::m_EventMouseLeave)
			.def_readwrite("EventMouseClick", &my::Control::m_EventMouseClick)
			.property("Requested", &my::Control::IsRequested)
			.scope
			[
				def("GetFocusControl", &my::Control::GetFocusControl),
				def("SetFocusControl", &my::Control::SetFocusControl),
				def("GetCaptureControl", &my::Control::GetCaptureControl),
				def("SetCaptureControl", &my::Control::SetCaptureControl),
				def("GetMouseOverControl", &my::Control::GetMouseOverControl),
				def("SetMouseOverControl", &my::Control::SetMouseOverControl)
			]
			.def("RequestResource", &my::Control::RequestResource, &ScriptControl::default_RequestResource)
			.def("ReleaseResource", &my::Control::ReleaseResource, &ScriptControl::default_ReleaseResource)
			.def("Draw", &my::Control::Draw, &ScriptControl::default_Draw)
			.def("MsgProc", &my::Control::MsgProc, &ScriptControl::default_MsgProc)
			.def("HandleKeyboard", &my::Control::HandleKeyboard, &ScriptControl::default_HandleKeyboard)
			.def("HandleMouse", &my::Control::HandleMouse, &ScriptControl::default_HandleMouse)
			.def("HitTest", &my::Control::HitTest)
			.property("Enabled", &my::Control::GetEnabled, &my::Control::SetEnabled)
			.property("Visible", &my::Control::GetVisible, &my::Control::SetVisible)
			.property("Focused", &my::Control::GetFocused, &my::Control::SetFocused)
			.property("Captured", &my::Control::GetCaptured, &my::Control::SetCaptured)
			.property("MouseOver", &my::Control::GetMouseOver, &my::Control::SetMouseOver)
			.def("InsertControl", &my::Control::InsertControl)
			.def("RemoveControl", &my::Control::RemoveControl)
			.def("ClearAllControl", &my::Control::ClearAllControl)
			.def("ContainsControl", &my::Control::ContainsControl)

		, class_<my::Static, my::Control, boost::shared_ptr<my::Control> >("Static")
			.def(constructor<const char *>())
			.def_readwrite("Text", &my::Static::m_Text)

		, class_<my::ProgressBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ProgressBarSkin")
			.def(constructor<>())
			.def_readwrite("ForegroundImage", &my::ProgressBarSkin::m_ForegroundImage)

		, class_<my::ProgressBar, my::Static, boost::shared_ptr<my::Control> >("ProgressBar")
			.def(constructor<const char *>())
			.def_readwrite("Progress", &my::ProgressBar::m_Progress)

		, class_<my::ButtonSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ButtonSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::ButtonSkin::m_DisabledImage)
			.def_readwrite("PressedImage", &my::ButtonSkin::m_PressedImage)
			.def_readwrite("MouseOverImage", &my::ButtonSkin::m_MouseOverImage)
			.def_readwrite("PressedOffset", &my::ButtonSkin::m_PressedOffset)

		, class_<my::Button, my::Static, boost::shared_ptr<my::Control> >("Button")
			.def(constructor<const char *>())
			.def("SetHotkey", &my::Button::SetHotkey)

		, class_<my::EditBoxSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("EditBoxSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::EditBoxSkin::m_DisabledImage)
			.def_readwrite("FocusedImage", &my::EditBoxSkin::m_FocusedImage)
			.def_readwrite("SelBkColor", &my::EditBoxSkin::m_SelBkColor)
			.def_readwrite("CaretColor", &my::EditBoxSkin::m_CaretColor)

		, class_<my::EditBox, my::Static, boost::shared_ptr<my::Control> >("EditBox")
			.def(constructor<const char *>())
			.property("Text", &my::EditBox::GetText, &my::EditBox::SetText)
			.def_readwrite("Border", &my::EditBox::m_Border)
			.def_readwrite("EventChange", &my::EditBox::m_EventChange)
			.def_readwrite("EventEnter", &my::EditBox::m_EventEnter)

		, class_<my::ImeEditBox, my::EditBox, boost::shared_ptr<my::Control> >("ImeEditBox")
			.def(constructor<const char *>())

		, class_<my::ScrollBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ScrollBarSkin")
			.def(constructor<>())
			.def_readwrite("UpBtnNormalImage", &my::ScrollBarSkin::m_UpBtnNormalImage)
			.def_readwrite("UpBtnDisabledImage", &my::ScrollBarSkin::m_UpBtnDisabledImage)
			.def_readwrite("DownBtnNormalImage", &my::ScrollBarSkin::m_DownBtnNormalImage)
			.def_readwrite("DownBtnDisabledImage", &my::ScrollBarSkin::m_DownBtnDisabledImage)
			.def_readwrite("ThumbBtnNormalImage", &my::ScrollBarSkin::m_ThumbBtnNormalImage)

		, class_<my::ScrollBar, my::Control, boost::shared_ptr<my::Control> >("ScrollBar")
			.def(constructor<const char *>())
			.def_readwrite("nPosition", &my::ScrollBar::m_nPosition) // ! should use property
			.def_readwrite("nPageSize", &my::ScrollBar::m_nPageSize) // ! should use property
			.def_readwrite("nStart", &my::ScrollBar::m_nStart) // ! should use property
			.def_readwrite("nEnd", &my::ScrollBar::m_nEnd) // ! should use property

		, class_<my::CheckBox, my::Button, boost::shared_ptr<my::Control> >("CheckBox")
			.def(constructor<const char *>())
			.def_readwrite("Checked", &my::CheckBox::m_Checked)

		, class_<my::ComboBoxSkin, my::ButtonSkin, boost::shared_ptr<my::ControlSkin> >("ComboBoxSkin")
			.def(constructor<>())
			.def_readwrite("DropdownImage", &my::ComboBoxSkin::m_DropdownImage)
			.def_readwrite("DropdownItemTextColor", &my::ComboBoxSkin::m_DropdownItemTextColor)
			.def_readwrite("DropdownItemTextAlign", &my::ComboBoxSkin::m_DropdownItemTextAlign)
			.def_readwrite("DropdownItemMouseOverImage", &my::ComboBoxSkin::m_DropdownItemMouseOverImage)
			.def_readwrite("ScrollBarUpBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarUpBtnNormalImage)
			.def_readwrite("ScrollBarUpBtnDisabledImage", &my::ComboBoxSkin::m_ScrollBarUpBtnDisabledImage)
			.def_readwrite("ScrollBarDownBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarDownBtnNormalImage)
			.def_readwrite("ScrollBarDownBtnDisabledImage", &my::ComboBoxSkin::m_ScrollBarDownBtnDisabledImage)
			.def_readwrite("ScrollBarThumbBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarThumbBtnNormalImage)
			.def_readwrite("ScrollBarImage", &my::ComboBoxSkin::m_ScrollBarImage)

		, class_<my::ComboBox, my::Button, boost::shared_ptr<my::Control> >("ComboBox")
			.def(constructor<const char *>())
			.property("DropdownSize", &my::ComboBox::GetDropdownSize, &my::ComboBox::SetDropdownSize)
			.property("ScrollbarWidth", &my::ComboBox::GetScrollbarWidth, &my::ComboBox::SetScrollbarWidth)
			.property("ScrollbarUpDownBtnHeight", &my::ComboBox::GetScrollbarUpDownBtnHeight, &my::ComboBox::SetScrollbarUpDownBtnHeight)
			.property("Border", &my::ComboBox::GetBorder, &my::ComboBox::SetBorder)
			.property("ItemHeight", &my::ComboBox::GetItemHeight, &my::ComboBox::SetItemHeight)
			.property("Selected", &my::ComboBox::GetSelected, &my::ComboBox::SetSelected)
			.def("AddItem", &my::ComboBox::AddItem)
			.def("RemoveAllItems", &my::ComboBox::RemoveAllItems)
			.def("ContainsItem", &my::ComboBox::ContainsItem)
			.def("FindItem", &my::ComboBox::FindItem)
			.def("GetItemData", &my::ComboBox::GetItemDataUInt)
			.def("SetItemData", (void (my::ComboBox::*)(int, unsigned int))&my::ComboBox::SetItemData)
			.property("NumItems", &my::ComboBox::GetNumItems)
			.def_readwrite("EventSelectionChanged", &my::ComboBox::m_EventSelectionChanged)

		, class_<my::ListBoxSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ListBoxSkin")
			.def(constructor<>())
			.def_readwrite("ScrollBarUpBtnNormalImage", &my::ListBoxSkin::m_ScrollBarUpBtnNormalImage)
			.def_readwrite("ScrollBarUpBtnDisabledImage", &my::ListBoxSkin::m_ScrollBarUpBtnDisabledImage)
			.def_readwrite("ScrollBarDownBtnNormalImage", &my::ListBoxSkin::m_ScrollBarDownBtnNormalImage)
			.def_readwrite("ScrollBarDownBtnDisabledImage", &my::ListBoxSkin::m_ScrollBarDownBtnDisabledImage)
			.def_readwrite("ScrollBarThumbBtnNormalImage", &my::ListBoxSkin::m_ScrollBarThumbBtnNormalImage)
			.def_readwrite("ScrollBarImage", &my::ListBoxSkin::m_ScrollBarImage)

		, class_<my::ListBox, my::Control, boost::shared_ptr<my::Control> >("ListBox")
			.def(constructor<const char *>())
			.property("ScrollbarWidth", &my::ListBox::GetScrollbarWidth, &my::ListBox::SetScrollbarWidth)
			.property("ScrollbarUpDownBtnHeight", &my::ListBox::GetScrollbarUpDownBtnHeight, &my::ListBox::SetScrollbarUpDownBtnHeight)
			.property("ItemSize", &my::ListBox::GetItemSize, &my::ListBox::SetItemSize)

		, class_<my::DialogSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("DialogSkin")
			.def(constructor<>())

		, class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(constructor<const char *>())
			.def_readwrite("World", &my::Dialog::m_World)
			.def_readwrite("EnableDrag", &my::Dialog::m_EnableDrag)
			.def_readwrite("EventAlign", &my::Dialog::m_EventAlign)
			.def("MoveToFront", &my::Dialog::MoveToFront)
	];

	module(m_State)[
		class_<HWND>("HWND")

		, class_<D3DSURFACE_DESC>("D3DSURFACE_DESC")
			.def_readwrite("Format", &D3DSURFACE_DESC::Format)
			.def_readwrite("Type", &D3DSURFACE_DESC::Type)
			.def_readwrite("Usage", &D3DSURFACE_DESC::Usage)
			.def_readwrite("Pool", &D3DSURFACE_DESC::Pool)
			.def_readwrite("MultiSampleType", &D3DSURFACE_DESC::MultiSampleType)
			.def_readwrite("MultiSampleQuality", &D3DSURFACE_DESC::MultiSampleQuality)
			.def_readwrite("Width", &D3DSURFACE_DESC::Width)
			.def_readwrite("Height", &D3DSURFACE_DESC::Height)

		, class_<D3DPRESENT_PARAMETERS>("D3DPRESENT_PARAMETERS")
			.def_readwrite("BackBufferWidth", &D3DPRESENT_PARAMETERS::BackBufferWidth)
			.def_readwrite("BackBufferHeight", &D3DPRESENT_PARAMETERS::BackBufferHeight)
			.def_readwrite("BackBufferFormat", &D3DPRESENT_PARAMETERS::BackBufferFormat)
			.def_readwrite("BackBufferCount", &D3DPRESENT_PARAMETERS::BackBufferCount)
			.def_readwrite("MultiSampleType", &D3DPRESENT_PARAMETERS::MultiSampleType)
			.def_readwrite("MultiSampleQuality", &D3DPRESENT_PARAMETERS::MultiSampleQuality)
			.def_readwrite("SwapEffect", &D3DPRESENT_PARAMETERS::SwapEffect)
			.def_readwrite("hDeviceWindow", &D3DPRESENT_PARAMETERS::hDeviceWindow)
			.def_readwrite("Windowed", &D3DPRESENT_PARAMETERS::Windowed)
			.def_readwrite("EnableAutoDepthStencil", &D3DPRESENT_PARAMETERS::EnableAutoDepthStencil)
			.def_readwrite("AutoDepthStencilFormat", &D3DPRESENT_PARAMETERS::AutoDepthStencilFormat)
			.def_readwrite("Flags", &D3DPRESENT_PARAMETERS::Flags)
			.def_readwrite("FullScreen_RefreshRateInHz", &D3DPRESENT_PARAMETERS::FullScreen_RefreshRateInHz)
			.def_readwrite("PresentationInterval", &D3DPRESENT_PARAMETERS::PresentationInterval)

		, class_<D3DDISPLAYMODE>("D3DDISPLAYMODE")
			.def_readwrite("Width", &D3DDISPLAYMODE::Width)
			.def_readwrite("Height", &D3DDISPLAYMODE::Height)
			.def_readwrite("RefreshRate", &D3DDISPLAYMODE::RefreshRate)
			.def_readwrite("Format", &D3DDISPLAYMODE::Format)

		, class_<CGrowableArray<D3DDISPLAYMODE> >("CD3D9EnumAdapterInfoArray")
			.def("GetAt", &CGrowableArray<D3DDISPLAYMODE>::GetAt)
			.property("Size", &CGrowableArray<D3DDISPLAYMODE>::GetSize)

		, class_<DXUTD3D9DeviceSettings>("DXUTD3D9DeviceSettings")
			.enum_("VertexProcessingType")
			[
				value("D3DCREATE_SOFTWARE_VERTEXPROCESSING", D3DCREATE_SOFTWARE_VERTEXPROCESSING),
				value("D3DCREATE_MIXED_VERTEXPROCESSING", D3DCREATE_MIXED_VERTEXPROCESSING),
				value("D3DCREATE_HARDWARE_VERTEXPROCESSING", D3DCREATE_HARDWARE_VERTEXPROCESSING),
				value("D3DCREATE_PUREDEVICE", D3DCREATE_PUREDEVICE)
			]
			.enum_("PresentIntervalType")
			[
				value("D3DPRESENT_INTERVAL_DEFAULT", D3DPRESENT_INTERVAL_DEFAULT),
				value("D3DPRESENT_INTERVAL_IMMEDIATE", D3DPRESENT_INTERVAL_IMMEDIATE)
			]
			.def_readwrite("AdapterOrdinal", &DXUTD3D9DeviceSettings::AdapterOrdinal)
			.def_readwrite("DeviceType", &DXUTD3D9DeviceSettings::DeviceType)
			.def_readwrite("AdapterFormat", &DXUTD3D9DeviceSettings::AdapterFormat)
			.def_readwrite("BehaviorFlags", &DXUTD3D9DeviceSettings::BehaviorFlags)
			.def_readwrite("pp", &DXUTD3D9DeviceSettings::pp)

		, class_<CD3D9EnumAdapterInfo>("CD3D9EnumAdapterInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumAdapterInfo::AdapterOrdinal)
			.def_readonly("szUniqueDescription", &CD3D9EnumAdapterInfo::szUniqueDescription)
			.def_readonly("displayModeList", &CD3D9EnumAdapterInfo::displayModeList)
			.def_readonly("deviceInfoList", &CD3D9EnumAdapterInfo::deviceInfoList)

		, class_<CGrowableArray<CD3D9EnumAdapterInfo *> >("CD3D9EnumAdapterInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetAt)
			.property("Size", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetSize)

		, class_<CD3D9EnumDeviceInfo>("CD3D9EnumDeviceInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceInfo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceInfo::DeviceType)
			.def_readonly("deviceSettingsComboList", &CD3D9EnumDeviceInfo::deviceSettingsComboList)

		, class_<CGrowableArray<CD3D9EnumDeviceInfo *> >("CD3D9EnumDeviceInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetAt)
			.property("Size", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetSize)

		, class_<CGrowableArray<D3DFORMAT> >("D3DFORMATArray")
			.def("GetAt", &CGrowableArray<D3DFORMAT>::GetAt)
			.property("Size", &CGrowableArray<D3DFORMAT>::GetSize)

		, class_<CGrowableArray<D3DMULTISAMPLE_TYPE> >("D3DMULTISAMPLE_TYPEArray")
			.def("GetAt", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetAt)
			.property("Size", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetSize)

		, class_<CGrowableArray<DWORD> >("DWORDArray")
			.def("GetAt", (const DWORD & (CGrowableArray<DWORD>::*)(int))&CGrowableArray<DWORD>::GetAt) // ! forced convert to const ref
			.property("Size", &CGrowableArray<DWORD>::GetSize)

		, class_<CD3D9EnumDeviceSettingsCombo>("CD3D9EnumDeviceSettingsCombo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceSettingsCombo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceSettingsCombo::DeviceType)
			.def_readonly("AdapterFormat", &CD3D9EnumDeviceSettingsCombo::AdapterFormat)
			.def_readonly("BackBufferFormat", &CD3D9EnumDeviceSettingsCombo::BackBufferFormat)
			.def_readonly("Windowed", &CD3D9EnumDeviceSettingsCombo::Windowed)
			.def_readonly("depthStencilFormatList", &CD3D9EnumDeviceSettingsCombo::depthStencilFormatList)
			.def_readonly("multiSampleTypeList", &CD3D9EnumDeviceSettingsCombo::multiSampleTypeList)
			.def_readonly("multiSampleQualityList", &CD3D9EnumDeviceSettingsCombo::multiSampleQualityList)
			.def("IsDepthStencilMultiSampleConflict", &CD3D9EnumDeviceSettingsCombo::IsDepthStencilMultiSampleConflict)

		, class_<CGrowableArray<CD3D9EnumDeviceSettingsCombo *> >("CD3D9EnumDeviceSettingsComboArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetAt)
			.property("Size", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetSize)

		, class_<CD3D9Enumeration>("CD3D9Enumeration")
			.property("AdapterInfoList", &CD3D9Enumeration::GetAdapterInfoList)
			.def("GetAdapterInfo", &CD3D9Enumeration::GetAdapterInfo)
			.def("GetDeviceInfo", &CD3D9Enumeration::GetDeviceInfo)
			.def("GetDeviceSettingsCombo", (CD3D9EnumDeviceSettingsCombo *(CD3D9Enumeration::*)(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL))&CD3D9Enumeration::GetDeviceSettingsCombo)

		, class_<my::DxutWindow, boost::shared_ptr<my::DxutWindow> >("DxutWindow")
			.enum_("MessageCode")
			[
				value("WM_MOUSEFIRST", WM_MOUSEFIRST),
				value("WM_MOUSEMOVE", WM_MOUSEMOVE),
				value("WM_LBUTTONDOWN", WM_LBUTTONDOWN),
				value("WM_LBUTTONUP", WM_LBUTTONUP),
				value("WM_LBUTTONDBLCLK", WM_LBUTTONDBLCLK),
				value("WM_RBUTTONDOWN", WM_RBUTTONDOWN),
				value("WM_RBUTTONUP", WM_RBUTTONUP),
				value("WM_RBUTTONDBLCLK", WM_RBUTTONDBLCLK),
				value("WM_MBUTTONDOWN", WM_MBUTTONDOWN),
				value("WM_MBUTTONUP", WM_MBUTTONUP),
				value("WM_MBUTTONDBLCLK", WM_MBUTTONDBLCLK),
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
				value("WM_MOUSEWHEEL", WM_MOUSEWHEEL),
#endif
#if (_WIN32_WINNT >= 0x0500)
				value("WM_XBUTTONDOWN", WM_XBUTTONDOWN),
				value("WM_XBUTTONUP", WM_XBUTTONUP),
				value("WM_XBUTTONDBLCLK", WM_XBUTTONDBLCLK),
#endif
#if (_WIN32_WINNT >= 0x0600)
				value("WM_MOUSEHWHEEL", WM_MOUSEHWHEEL),
#endif
				value("WM_USER", WM_USER)
			]
			.def("PostMessage", &my::DxutWindow::PostMessage)

		, class_<my::Clock>("Clock")
			.def_readonly("AbsoluteTime", &my::Clock::m_fAbsoluteTime)
			.def_readonly("AbsoluteElapsedTime", &my::Clock::m_fAbsoluteElapsedTime)
			.def_readwrite("TimeScale", &my::Clock::m_fTimeScale)
			.def_readonly("ElapsedTime", &my::Clock::m_fElapsedTime)
			.def_readonly("TotalTime", &my::Clock::m_fTotalTime)

		, class_<my::D3DContext, my::Clock>("D3DContext")
			.def("GetNamedObject", &my::D3DContext::GetNamedObject)

		, class_<my::DxutApp, bases<my::D3DContext, CD3D9Enumeration> >("DxutApp")
			.scope
			[
				def("DXUTD3DDeviceTypeToString", &my::DxutApp::DXUTD3DDeviceTypeToString),
				def("DXUTD3DFormatToString", &my::DxutApp::DXUTD3DFormatToString),
				def("DXUTMultisampleTypeToString", &my::DxutApp::DXUTMultisampleTypeToString),
				def("DXUTVertexProcessingTypeToString", &my::DxutApp::DXUTVertexProcessingTypeToString)
			]
			.def_readonly("BackBufferSurfaceDesc", &my::DxutApp::m_BackBufferSurfaceDesc)
			.def_readonly("DeviceSettings", &my::DxutApp::m_DeviceSettings, luabind::copy(result))
			.def("ToggleFullScreen", &my::DxutApp::ToggleFullScreen)
			.def("ToggleREF", &my::DxutApp::ToggleREF)
			.property("SoftwareVP", &my::DxutApp::GetSoftwareVP, &my::DxutApp::SetSoftwareVP)
			.property("HardwareVP", &my::DxutApp::GetHardwareVP, &my::DxutApp::SetHardwareVP)
			.property("PureHardwareVP", &my::DxutApp::GetPureHardwareVP, &my::DxutApp::SetPureHardwareVP)
			.property("MixedVP", &my::DxutApp::GetMixedVP, &my::DxutApp::SetMixedVP)
			.def("ChangeDevice", &my::DxutApp::ChangeDevice)

		, class_<my::Keyboard, boost::shared_ptr<my::Keyboard> >("Keyboard")
			.enum_("KeyCode")
			[
				value("KC_UNASSIGNED", my::KC_UNASSIGNED),
				value("KC_ESCAPE", my::KC_ESCAPE),
				value("KC_1", my::KC_1),
				value("KC_2", my::KC_2),
				value("KC_3", my::KC_3),
				value("KC_4", my::KC_4),
				value("KC_5", my::KC_5),
				value("KC_6", my::KC_6),
				value("KC_7", my::KC_7),
				value("KC_8", my::KC_8),
				value("KC_9", my::KC_9),
				value("KC_0", my::KC_0),
				value("KC_MINUS", my::KC_MINUS),
				value("KC_EQUALS", my::KC_EQUALS),
				value("KC_BACK", my::KC_BACK),
				value("KC_TAB", my::KC_TAB),
				value("KC_Q", my::KC_Q),
				value("KC_W", my::KC_W),
				value("KC_E", my::KC_E),
				value("KC_R", my::KC_R),
				value("KC_T", my::KC_T),
				value("KC_Y", my::KC_Y),
				value("KC_U", my::KC_U),
				value("KC_I", my::KC_I),
				value("KC_O", my::KC_O),
				value("KC_P", my::KC_P),
				value("KC_LBRACKET", my::KC_LBRACKET),
				value("KC_RBRACKET", my::KC_RBRACKET),
				value("KC_RETURN", my::KC_RETURN),
				value("KC_LCONTROL", my::KC_LCONTROL),
				value("KC_A", my::KC_A),
				value("KC_S", my::KC_S),
				value("KC_D", my::KC_D),
				value("KC_F", my::KC_F),
				value("KC_G", my::KC_G),
				value("KC_H", my::KC_H),
				value("KC_J", my::KC_J),
				value("KC_K", my::KC_K),
				value("KC_L", my::KC_L),
				value("KC_SEMICOLON", my::KC_SEMICOLON),
				value("KC_APOSTROPHE", my::KC_APOSTROPHE),
				value("KC_GRAVE", my::KC_GRAVE),
				value("KC_LSHIFT", my::KC_LSHIFT),
				value("KC_BACKSLASH", my::KC_BACKSLASH),
				value("KC_Z", my::KC_Z),
				value("KC_X", my::KC_X),
				value("KC_C", my::KC_C),
				value("KC_V", my::KC_V),
				value("KC_B", my::KC_B),
				value("KC_N", my::KC_N),
				value("KC_M", my::KC_M),
				value("KC_COMMA", my::KC_COMMA),
				value("KC_PERIOD", my::KC_PERIOD),
				value("KC_SLASH", my::KC_SLASH),
				value("KC_RSHIFT", my::KC_RSHIFT),
				value("KC_MULTIPLY", my::KC_MULTIPLY),
				value("KC_LMENU", my::KC_LMENU),
				value("KC_SPACE", my::KC_SPACE),
				value("KC_CAPITAL", my::KC_CAPITAL),
				value("KC_F1", my::KC_F1),
				value("KC_F2", my::KC_F2),
				value("KC_F3", my::KC_F3),
				value("KC_F4", my::KC_F4),
				value("KC_F5", my::KC_F5),
				value("KC_F6", my::KC_F6),
				value("KC_F7", my::KC_F7),
				value("KC_F8", my::KC_F8),
				value("KC_F9", my::KC_F9),
				value("KC_F10", my::KC_F10),
				value("KC_NUMLOCK", my::KC_NUMLOCK),
				value("KC_SCROLL", my::KC_SCROLL),
				value("KC_NUMPAD7", my::KC_NUMPAD7),
				value("KC_NUMPAD8", my::KC_NUMPAD8),
				value("KC_NUMPAD9", my::KC_NUMPAD9),
				value("KC_SUBTRACT", my::KC_SUBTRACT),
				value("KC_NUMPAD4", my::KC_NUMPAD4),
				value("KC_NUMPAD5", my::KC_NUMPAD5),
				value("KC_NUMPAD6", my::KC_NUMPAD6),
				value("KC_ADD", my::KC_ADD),
				value("KC_NUMPAD1", my::KC_NUMPAD1),
				value("KC_NUMPAD2", my::KC_NUMPAD2),
				value("KC_NUMPAD3", my::KC_NUMPAD3),
				value("KC_NUMPAD0", my::KC_NUMPAD0),
				value("KC_DECIMAL", my::KC_DECIMAL),
				value("KC_OEM_102", my::KC_OEM_102),
				value("KC_F11", my::KC_F11),
				value("KC_F12", my::KC_F12),
				value("KC_F13", my::KC_F13),
				value("KC_F14", my::KC_F14),
				value("KC_F15", my::KC_F15),
				value("KC_KANA", my::KC_KANA),
				value("KC_ABNT_C1", my::KC_ABNT_C1),
				value("KC_CONVERT", my::KC_CONVERT),
				value("KC_NOCONVERT", my::KC_NOCONVERT),
				value("KC_YEN", my::KC_YEN),
				value("KC_ABNT_C2", my::KC_ABNT_C2),
				value("KC_NUMPADEQUALS", my::KC_NUMPADEQUALS),
				value("KC_PREVTRACK", my::KC_PREVTRACK),
				value("KC_AT", my::KC_AT),
				value("KC_COLON", my::KC_COLON),
				value("KC_UNDERLINE", my::KC_UNDERLINE),
				value("KC_KANJI", my::KC_KANJI),
				value("KC_STOP", my::KC_STOP),
				value("KC_AX", my::KC_AX),
				value("KC_UNLABELED", my::KC_UNLABELED),
				value("KC_NEXTTRACK", my::KC_NEXTTRACK),
				value("KC_NUMPADENTER", my::KC_NUMPADENTER),
				value("KC_RCONTROL", my::KC_RCONTROL),
				value("KC_MUTE", my::KC_MUTE),
				value("KC_CALCULATOR", my::KC_CALCULATOR),
				value("KC_PLAYPAUSE", my::KC_PLAYPAUSE),
				value("KC_MEDIASTOP", my::KC_MEDIASTOP),
				value("KC_VOLUMEDOWN", my::KC_VOLUMEDOWN),
				value("KC_VOLUMEUP", my::KC_VOLUMEUP),
				value("KC_WEBHOME", my::KC_WEBHOME),
				value("KC_NUMPADCOMMA", my::KC_NUMPADCOMMA),
				value("KC_DIVIDE", my::KC_DIVIDE),
				value("KC_SYSRQ", my::KC_SYSRQ),
				value("KC_RMENU", my::KC_RMENU),
				value("KC_PAUSE", my::KC_PAUSE),
				value("KC_HOME", my::KC_HOME),
				value("KC_UP", my::KC_UP),
				value("KC_PGUP", my::KC_PGUP),
				value("KC_LEFT", my::KC_LEFT),
				value("KC_RIGHT", my::KC_RIGHT),
				value("KC_END", my::KC_END),
				value("KC_DOWN", my::KC_DOWN),
				value("KC_PGDOWN", my::KC_PGDOWN),
				value("KC_INSERT", my::KC_INSERT),
				value("KC_DELETE", my::KC_DELETE),
				value("KC_LWIN", my::KC_LWIN),
				value("KC_RWIN", my::KC_RWIN),
				value("KC_APPS", my::KC_APPS),
				value("KC_POWER", my::KC_POWER),
				value("KC_SLEEP", my::KC_SLEEP),
				value("KC_WAKE", my::KC_WAKE),
				value("KC_WEBSEARCH", my::KC_WEBSEARCH),
				value("KC_WEBFAVORITES", my::KC_WEBFAVORITES),
				value("KC_WEBREFRESH", my::KC_WEBREFRESH),
				value("KC_WEBSTOP", my::KC_WEBSTOP),
				value("KC_WEBFORWARD", my::KC_WEBFORWARD),
				value("KC_WEBBACK", my::KC_WEBBACK),
				value("KC_MYCOMPUTER", my::KC_MYCOMPUTER),
				value("KC_MAIL", my::KC_MAIL),
				value("KC_MEDIASELECT", my::KC_MEDIASELECT)
			]
			.def("IsKeyDown", &my::Keyboard::IsKeyDown)
			.def("IsKeyPress", &my::Keyboard::IsKeyPress)
			.def("IsKeyRelease", &my::Keyboard::IsKeyRelease)

		, class_<my::Mouse, boost::shared_ptr<my::Mouse> >("Mouse")
			.property("X", &my::Mouse::GetX)
			.property("Y", &my::Mouse::GetY)
			.property("Z", &my::Mouse::GetZ)
			.def("IsButtonDown", &my::Mouse::IsButtonDown)
			.def("IsButtonPress", &my::Mouse::IsButtonPress)
			.def("IsButtonRelease", &my::Mouse::IsButtonRelease)

		, class_<my::Joystick, boost::shared_ptr<my::Joystick> >("Joystick")
			.enum_("JoystickPov")
			[
				value("JP_None", my::JP_None),
				value("JP_North", my::JP_North),
				value("JP_NorthEast", my::JP_NorthEast),
				value("JP_East", my::JP_East),
				value("JP_SouthEast", my::JP_SouthEast),
				value("JP_South", my::JP_South),
				value("JP_SouthWest", my::JP_SouthWest),
				value("JP_West", my::JP_West),
				value("JP_NorthWest", my::JP_NorthWest)
			]
			.property("X", &my::Joystick::GetX)
			.property("Y", &my::Joystick::GetY)
			.property("Z", &my::Joystick::GetZ)
			.property("Rx", &my::Joystick::GetRx)
			.property("Ry", &my::Joystick::GetRy)
			.property("Rz", &my::Joystick::GetRz)
			.def("GetSlider", &my::Joystick::GetSlider)
			.def("GetPov", &my::Joystick::GetPov)
			.def("IsButtonDown", &my::Joystick::IsButtonDown)
			.def("IsButtonPress", &my::Joystick::IsButtonPress)
			.def("IsButtonRelease", &my::Joystick::IsButtonRelease)

		, class_<my::InputMgr>("InputMgr")
			.def_readonly("keyboard", &my::InputMgr::m_keyboard)
			.def_readonly("mouse", &my::InputMgr::m_mouse)
			.def_readonly("joystick", &my::InputMgr::m_joystick)
			.enum_("Type")
			[
				value("KeyboardButton", my::InputMgr::KeyboardButton),
				value("MouseMove", my::InputMgr::MouseMove),
				value("MouseButton", my::InputMgr::MouseButton),
				value("JoystickAxis", my::InputMgr::JoystickAxis),
				value("JoystickPov", my::InputMgr::JoystickPov),
				value("JoystickButton", my::InputMgr::JoystickButton)
			]
			.def("BindKey", &my::InputMgr::BindKey)
			.def("UnbindKey", &my::InputMgr::UnbindKey)
			.def("GetKeyAxisRaw", &my::InputMgr::GetKeyAxisRaw)
			.def("IsKeyDown", &my::InputMgr::IsKeyDown)
			.def("IsKeyPressRaw", &my::InputMgr::IsKeyPressRaw)
			.def("IsKeyPress", &my::InputMgr::IsKeyPress)
			.def("IsKeyRelease", &my::InputMgr::IsKeyRelease)

		, class_<my::TimerEventArg, my::EventArg>("TimerEventArg")
			.def_readonly("Interval", &my::TimerEventArg::m_Interval)

		, class_<my::Timer, boost::shared_ptr<my::Timer> >("Timer")
			.def(constructor<float, float>())
			.def_readonly("Interval", &my::Timer::m_Interval)
			.def_readonly("RemainingTime", &my::Timer::m_RemainingTime)
			.def_readwrite("EventTimer", &my::Timer::m_EventTimer)

		, class_<CPoint>("CPoint")
			.def_readwrite("x", &CPoint::x)
			.def_readwrite("y", &CPoint::y)
	];

	module(m_State)[
		class_<Material, boost::shared_ptr<Material> >("Material")
			.enum_("PassMask")
			[
				value("PassMaskNone", RenderPipeline::PassMaskNone),
				value("PassMaskLight", RenderPipeline::PassMaskLight),
				value("PassMaskBackground", RenderPipeline::PassMaskBackground),
				value("PassMaskOpaque", RenderPipeline::PassMaskOpaque),
				value("PassMaskNormalOpaque", RenderPipeline::PassMaskNormalOpaque),
				value("PassMaskShadowNormalOpaque", RenderPipeline::PassMaskShadowNormalOpaque),
				value("PassMaskTransparent", RenderPipeline::PassMaskTransparent)
			]
			.enum_("CullMode")
			[
				value("CullModeNone", D3DCULL_NONE),
				value("CullModeCW", D3DCULL_CW),
				value("CullModeCCW", D3DCULL_CCW)
			]
			.enum_("BlendMode")
			[
				value("BlendModeNone", Material::BlendModeNone),
				value("BlendModeAlpha", Material::BlendModeAlpha),
				value("BlendModeAdditive", Material::BlendModeAdditive)
			]
			.def(constructor<>())
			.def_readwrite("Shader", &Material::m_Shader)
			.def_readwrite("PassMask", &Material::m_PassMask)
			.def_readwrite("CullMode", &Material::m_CullMode)
			.def_readwrite("ZEnable", &Material::m_ZEnable)
			.def_readwrite("ZWriteEnable", &Material::m_ZWriteEnable)
			.def_readwrite("BlendMode", &Material::m_BlendMode)
			.def("Clone", &Material::Clone)
			.def("ParseShaderParameters", &Material::ParseShaderParameters)
			.def("AddParameter", &Material::AddParameter<float>)
			.def("AddParameter", &Material::AddParameter<my::Vector2>)
			.def("AddParameter", &Material::AddParameter<my::Vector3>)
			.def("AddParameter", &Material::AddParameter<my::Vector4>)
			.def("AddParameter", &Material::AddParameter<std::string>)
			.def("SetParameter", &Material::SetParameter<float>)
			.def("SetParameter", &Material::SetParameter<my::Vector2>)
			.def("SetParameter", &Material::SetParameter<my::Vector3>)
			.def("SetParameter", &Material::SetParameter<my::Vector4>)
			.def("SetParameter", &Material::SetParameter<std::string>)

		, class_<CollectionObjMap>("CollectionObjMap")
			.def(constructor<>())

		, class_<Component, ScriptComponent, my::NamedObject, boost::shared_ptr<Component> >("Component")
			.def(constructor<const char *>())
			.enum_("ComponentType")
			[
				value("ComponentTypeComponent", Component::ComponentTypeComponent),
				value("ComponentTypeActor", Component::ComponentTypeActor),
				value("ComponentTypeController", Component::ComponentTypeController),
				value("ComponentTypeMesh", Component::ComponentTypeMesh),
				value("ComponentTypeCloth", Component::ComponentTypeCloth),
				value("ComponentTypeStaticEmitter", Component::ComponentTypeStaticEmitter),
				value("ComponentTypeSphericalEmitter", Component::ComponentTypeSphericalEmitter),
				value("ComponentTypeTerrain", Component::ComponentTypeTerrain),
				value("ComponentTypeScript", Component::ComponentTypeScript),
				value("ComponentTypeAnimator", Component::ComponentTypeAnimator),
				value("ComponentTypeNavigation", Component::ComponentTypeNavigation)
			]
			.property("ComponentType", &Component::GetComponentType)
			.enum_("LODMask")
			[
				value("LOD0", Component::LOD0),
				value("LOD1", Component::LOD1),
				value("LOD2", Component::LOD2),
				value("LOD0_1", Component::LOD0_1),
				value("LOD1_2", Component::LOD1_2),
				value("LOD0_1_2", Component::LOD0_1_2)
			]
			.def_readwrite("LodMask", &Component::m_LodMask)
			.def_readonly("Actor", &Component::m_Actor)
			.property("Requested", &Component::IsRequested)
			.def("Clone", &Component::Clone)
			.def("RequestResource", &Component::RequestResource, &ScriptComponent::default_RequestResource)
			.def("ReleaseResource", &Component::ReleaseResource, &ScriptComponent::default_ReleaseResource)
			.def("Update", &Component::Update, &ScriptComponent::default_Update)
			.def("OnPxThreadSubstep", &Component::OnPxThreadSubstep, &ScriptComponent::default_OnPxThreadSubstep)
			.def("OnEnterTrigger", &Component::OnEnterTrigger, &ScriptComponent::default_OnEnterTrigger)
			.def("OnLeaveTrigger", &Component::OnLeaveTrigger, &ScriptComponent::default_OnLeaveTrigger)
			.def("OnPxThreadShapeHit", &Component::OnPxThreadShapeHit, &ScriptComponent::default_OnPxThreadShapeHit)
			.def("OnPxThreadControllerHit", &Component::OnPxThreadControllerHit, &ScriptComponent::default_OnPxThreadControllerHit)
			.def("OnPxThreadObstacleHit", &Component::OnPxThreadObstacleHit, &ScriptComponent::default_OnPxThreadObstacleHit)
			.def("CalculateAABB", &Component::CalculateAABB)
			.property("Material", &Component::GetMaterial, &Component::SetMaterial)
			.def("CreateBoxShape", &Component::CreateBoxShape)
			.def("CreateCapsuleShape", &Component::CreateCapsuleShape)
			.def("CreatePlaneShape", &Component::CreatePlaneShape)
			.def("CreateSphereShape", &Component::CreateSphereShape)
			.property("SimulationFilterWord0", &Component::GetSimulationFilterWord0, &Component::SetSimulationFilterWord0)
			.property("QueryFilterWord0", &Component::GetQueryFilterWord0, &Component::SetQueryFilterWord0)
			.enum_("ShapeFlag")
			[
				value("eSIMULATION_SHAPE", physx::PxShapeFlag::eSIMULATION_SHAPE),
				value("eSCENE_QUERY_SHAPE", physx::PxShapeFlag::eSCENE_QUERY_SHAPE),
				value("eTRIGGER_SHAPE", physx::PxShapeFlag::eTRIGGER_SHAPE),
				value("eVISUALIZATION", physx::PxShapeFlag::eVISUALIZATION),
				value("ePARTICLE_DRAIN", physx::PxShapeFlag::ePARTICLE_DRAIN)
			]
			.def("SetShapeFlag", &Component::SetShapeFlag)
			.def("GetShapeFlag", &Component::GetShapeFlag)
			.def("ClearShape", &Component::ClearShape)

		, class_<MeshComponent, Component, boost::shared_ptr<Component> >("MeshComponent")
			.def(constructor<const char *>())
			.def_readwrite("MeshPath", &MeshComponent::m_MeshPath)
			.def_readwrite("MeshSubMeshName", &MeshComponent::m_MeshSubMeshName)
			.def_readwrite("MeshSubMeshId", &MeshComponent::m_MeshSubMeshId)
			.def_readonly("Mesh", &MeshComponent::m_Mesh)
			.def_readwrite("bInstance", &MeshComponent::m_bInstance)
			.def("CreateTriangleMeshShape", &MeshComponent::CreateTriangleMeshShape)

		, class_<ClothComponent, Component, boost::shared_ptr<Component> >("ClothComponent")
			.def(constructor<const char *>())
			.def("CreateClothFromMesh", &ClothComponent::CreateClothFromMesh)

		, class_<EmitterComponent, Component, boost::shared_ptr<Component> >("EmitterComponent")
			.enum_("FaceType")
			[
				value("FaceTypeX", EmitterComponent::FaceTypeX),
				value("FaceTypeY", EmitterComponent::FaceTypeY),
				value("FaceTypeZ", EmitterComponent::FaceTypeZ),
				value("FaceTypeCamera", EmitterComponent::FaceTypeCamera),
				value("FaceTypeAngle", EmitterComponent::FaceTypeAngle),
				value("FaceTypeAngleCamera", EmitterComponent::FaceTypeAngleCamera)
			]
			.def_readwrite("EmitterFaceType", &EmitterComponent::m_EmitterFaceType)
			.enum_("SpaceType")
			[
				value("SpaceTypeWorld", EmitterComponent::SpaceTypeWorld),
				value("SpaceTypeLocal", EmitterComponent::SpaceTypeLocal)
			]
			.def_readwrite("EmitterSpaceType", &EmitterComponent::m_EmitterSpaceType)
			.enum_("VelocityType")
			[
				value("VelocityTypeNone", EmitterComponent::VelocityTypeNone),
				value("VelocityTypeVel", EmitterComponent::VelocityTypeVel)
			]
			.def_readwrite("EmitterVelType", &EmitterComponent::m_EmitterVelType)
			.enum_("PrimitiveType")
			[
				value("PrimitiveTypeTri", EmitterComponent::PrimitiveTypeTri),
				value("PrimitiveTypeQuad", EmitterComponent::PrimitiveTypeQuad)
			]
			.def_readwrite("EmitterPrimitiveType", &EmitterComponent::m_EmitterPrimitiveType)

		, class_<StaticEmitter, EmitterComponent, boost::shared_ptr<Component> >("StaticEmitter")
			.def(constructor<const char *, const my::AABB &, float, EmitterComponent::FaceType, EmitterComponent::SpaceType, EmitterComponent::VelocityType, EmitterComponent::PrimitiveType>())
			.def_readonly("ChunkWidth", &StaticEmitter::m_ChunkWidth)
			.def_readwrite("EmitterChunkPath", &StaticEmitter::m_EmitterChunkPath)

		, class_<StaticEmitterStream>("StaticEmitterStream")
			.def(constructor<StaticEmitter *>())
			.def("Release", &StaticEmitterStream::Release)
			.def("Spawn", &StaticEmitterStream::Spawn)

		, class_<SphericalEmitter, EmitterComponent, boost::shared_ptr<Component> >("SphericalEmitter")
			.def(constructor<const char *, unsigned int, EmitterComponent::FaceType, EmitterComponent::SpaceType, EmitterComponent::VelocityType, EmitterComponent::PrimitiveType>())
			.def_readwrite("ParticleLifeTime", &SphericalEmitter::m_ParticleLifeTime)
			.def_readwrite("SpawnInterval", &SphericalEmitter::m_SpawnInterval)
			.def_readwrite("HalfSpawnArea", &SphericalEmitter::m_HalfSpawnArea)
			.def_readwrite("SpawnSpeed", &SphericalEmitter::m_SpawnSpeed)
			.def_readwrite("SpawnInclination", &SphericalEmitter::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &SphericalEmitter::m_SpawnAzimuth)
			.def_readwrite("SpawnColorR", &SphericalEmitter::m_SpawnColorR)
			.def_readwrite("SpawnColorG", &SphericalEmitter::m_SpawnColorG)
			.def_readwrite("SpawnColorB", &SphericalEmitter::m_SpawnColorB)
			.def_readwrite("SpawnColorA", &SphericalEmitter::m_SpawnColorA)
			.def_readwrite("SpawnSizeX", &SphericalEmitter::m_SpawnSizeX)
			.def_readwrite("SpawnSizeY", &SphericalEmitter::m_SpawnSizeY)
			.def_readwrite("SpawnAngle", &SphericalEmitter::m_SpawnAngle)
			.def_readwrite("SpawnCycle", &SphericalEmitter::m_SpawnCycle)
			.def("Spawn", &SphericalEmitter::Spawn)

		, class_<TerrainChunk, my::OctEntity>("TerrainChunk")
			.def_readonly("Row", &TerrainChunk::m_Row)
			.def_readonly("Col", &TerrainChunk::m_Col)

		, class_<Terrain, Component, boost::shared_ptr<Component> >("Terrain")
			.def(constructor<const char *, int, int, int, int>())
			.def_readonly("RowChunks", &Terrain::m_RowChunks)
			.def_readonly("ColChunks", &Terrain::m_ColChunks)
			.def_readonly("ChunkSize", &Terrain::m_ChunkSize)
			.def_readwrite("ChunkPath", &Terrain::m_ChunkPath)
			//.def_readonly("Chunks", &Terrain::m_Chunks, luabind::return_stl_iterator)
			.def("GetChunk", &Terrain::GetChunk)
			.def("CreateHeightFieldShape", &Terrain::CreateHeightFieldShape)

		, class_<TerrainStream>("TerrainStream")
			.def(constructor<Terrain *>())
			.def("Release", &TerrainStream::Release)
			.def("GetIndices", &TerrainStream::GetIndices, luabind::pure_out_value(_4) + luabind::pure_out_value(_5) + luabind::pure_out_value(_6) + luabind::pure_out_value(_7) + luabind::pure_out_value(_8) + luabind::pure_out_value(_9))
			.def("GetPos", &TerrainStream::GetPos, copy(result))
			.def("SetPos", (void(TerrainStream::*)(const my::Vector3 &, int, int, bool))&TerrainStream::SetPos)
			.def("GetColor", &TerrainStream::GetColor, copy(result))
			.def("SetColor", (void(TerrainStream::*)(D3DCOLOR, int, int))&TerrainStream::SetColor)
			.def("GetNormal", &TerrainStream::GetNormal, copy(result))
			.def("SetNormal", (void(TerrainStream::*)(const my::Vector3&, int, int))&TerrainStream::SetNormal)
			.def("RayTest", &TerrainStream::RayTest)

		, class_<Controller, Component, boost::shared_ptr<Component> >("Controller")
			.enum_("CollisionFlag")
			[
				value("eCOLLISION_SIDES", physx::PxControllerCollisionFlag::eCOLLISION_SIDES),
				value("eCOLLISION_UP", physx::PxControllerCollisionFlag::eCOLLISION_UP),
				value("eCOLLISION_DOWN", physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			]
			.def(constructor<const char*, float, float, float, unsigned int>())
			.def_readwrite("filterWord0", &Controller::m_filterWord0)
			.def("Move", &Controller::Move)

		, class_<Navigation, Component, boost::shared_ptr<Navigation> >("Navigation")
			.def(constructor<const char*>())

		, class_<ActorEventArg, my::EventArg>("ActorEventArg")
			.def_readonly("self", &ActorEventArg::self)

		, class_<TriggerEventArg, ActorEventArg>("TriggerEventArg")
			.def_readonly("self_cmp", &TriggerEventArg::self_cmp)
			.def_readonly("other", &TriggerEventArg::other)
			.def_readonly("other_cmp", &TriggerEventArg::other_cmp)

		, class_<ControllerEventArg, ActorEventArg>("ControllerEventArg")
			.def_readonly("self_cmp", &ControllerEventArg::self_cmp)
			.def_readonly("worldPos", &ControllerEventArg::worldPos)
			.def_readonly("worldNormal", &ControllerEventArg::worldNormal)
			.def_readonly("dir", &ControllerEventArg::dir)
			.def_readonly("length", &ControllerEventArg::length)

		, class_<ShapeHitEventArg, ControllerEventArg>("ShapeHitEventArg")
			.def_readonly("other", &ShapeHitEventArg::other)
			.def_readonly("other_cmp", &ShapeHitEventArg::other_cmp)
			.def_readonly("triangleIndex", &ShapeHitEventArg::triangleIndex)

		, class_<ControllerHitEventArg, ControllerEventArg>("ControllerHitEventArg")
			.def_readonly("other", &ControllerHitEventArg::other)
			.def_readonly("other_cmp", &ControllerHitEventArg::other_cmp)

		, class_<Actor, bases<my::NamedObject, my::OctEntity>, boost::shared_ptr<Actor> >("Actor")
			.def(constructor<const char *, const my::Vector3 &, const my::Quaternion &, const my::Vector3 &, const my::AABB &>())
			.def_readwrite("aabb", &Actor::m_aabb)
			.def_readwrite("Position", &Actor::m_Position)
			.def_readwrite("Rotation", &Actor::m_Rotation)
			.def_readwrite("Scale", &Actor::m_Scale)
			.def_readonly("World", &Actor::m_World)
			.def_readwrite("LodDist", &Actor::m_LodDist)
			.def_readwrite("LodFactor", &Actor::m_LodFactor)
			.def_readonly("Cmps", &Actor::m_Cmps, luabind::return_stl_iterator)
			//.def_readwrite("EventEnterTrigger", &Actor::m_EventEnterTrigger)
			//.def_readwrite("EventLeaveTrigger", &Actor::m_EventLeaveTrigger)
			//.def_readwrite("EventPxThreadShapeHit", &Actor::m_EventPxThreadShapeHit)
			.def(self == other<const Actor&>())
			.property("Requested", &Actor::IsRequested)
			.def("Clone", &Actor::Clone)
			.def("RequestResource", &Actor::RequestResource)
			.def("ReleaseResource", &Actor::ReleaseResource)
			.def("SetPose", &Actor::SetPose)
			.def("SetPxPoseOrbyPxThread", (void (Actor::*)(const my::Vector3&))& Actor::SetPxPoseOrbyPxThread)
			.def("SetPxPoseOrbyPxThread", (void (Actor::*)(const my::Vector3&, const my::Quaternion&))& Actor::SetPxPoseOrbyPxThread)
			.def("UpdateAABB", &Actor::UpdateAABB)
			.def("UpdateWorld", &Actor::UpdateWorld)
			.def("UpdateOctNode", &Actor::UpdateOctNode)
			.def("ClearRigidActor", &Actor::ClearRigidActor)
			.enum_("ActorType")
			[
				value("eRIGID_STATIC", physx::PxActorType::eRIGID_STATIC),
				value("eRIGID_DYNAMIC", physx::PxActorType::eRIGID_DYNAMIC)
			]
			.def("CreateRigidActor", &Actor::CreateRigidActor)
			.enum_("RigidBodyFlag")
			[
				value("eKINEMATIC", physx::PxRigidBodyFlag::eKINEMATIC),
				value("eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES", physx::PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES),
				value("eENABLE_CCD", physx::PxRigidBodyFlag::eENABLE_CCD),
				value("eENABLE_CCD_FRICTION", physx::PxRigidBodyFlag::eENABLE_CCD_FRICTION)
			]
			.def("SetRigidBodyFlag", &Actor::SetRigidBodyFlag)
			.def("GetRigidBodyFlag", &Actor::GetRigidBodyFlag)
			.def("AddComponent", &Actor::AddComponent/*, luabind::adopt(_1)*/)
			.def("RemoveComponent", &Actor::RemoveComponent)
			.def("ClearAllComponent", &Actor::ClearAllComponent)
			.def("Attach", &Actor::Attach)
			.def("Detach", &Actor::Detach)
			.def("PlayAction", &Actor::PlayAction)
			.def("GetFirstComponent", (Component * (Actor::*)(Component::ComponentType))&Actor::GetFirstComponent)

		//, def("act2entity", (boost::shared_ptr<my::OctEntity>(*)(const boost::shared_ptr<Actor>&))& boost::static_pointer_cast<my::OctEntity, Actor>)

		, class_<AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNode")
			.property("Child0", &AnimationNode::GetChild<0>, &AnimationNode::SetChild<0>)
			.property("Child1", &AnimationNode::GetChild<1>, &AnimationNode::SetChild<1>)
			.def("RemoveChild", &AnimationNode::RemoveChild)

		, class_<AnimationNodeSequence, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeSequence")
			.def(constructor<>())
			.def_readwrite("Name", &AnimationNodeSequence::m_Name)
			.property("RootList", &AnimationNodeSequence::GetRootList, &AnimationNodeSequence::SetRootList)
			.def_readwrite("Loop", &AnimationNodeSequence::m_Loop)
			.def_readwrite("Group", &AnimationNodeSequence::m_Group)

		, class_<AnimationNodeSlot, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeSlot")
			.def(constructor<>())
			.def("Play", &AnimationNodeSlot::Play)
			.def("Stop", &AnimationNodeSlot::Stop)

		, class_<AnimationNodeBlend, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeBlend")
			.def(constructor<>())
			.def_readwrite("TargetWeight", &AnimationNodeBlend::m_TargetWeight)
			.def_readwrite("BlendTime", &AnimationNodeBlend::m_BlendTime)
			.def("SetActiveChild", &AnimationNodeBlend::SetActiveChild)

		, class_<AnimationNodeRate, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeRate")
			.def(constructor<>())
			.def_readwrite("Rate", &AnimationNodeRate::m_Rate)

		, class_<AnimationEventArg, my::EventArg>("AnimationEventArg")
			.def_readonly("self", &AnimationEventArg::self)

		, class_<Animator, luabind::bases<Component, AnimationNodeSlot>, boost::shared_ptr<Component> >("Animator") // ! luabind::bases for accessing AnimationNodeSlot properties from boost::shared_ptr<Component>
			.def(constructor<const char*>())
			.def_readwrite("SkeletonPath", &Animator::m_SkeletonPath)
			.def_readonly("Skeleton", &Animator::m_Skeleton)
			.def("ReloadSequenceGroup", &Animator::ReloadSequenceGroup)
			.def("AddJiggleBone", (void (Animator::*)(int, const my::BoneHierarchy &, float, float, float))&Animator::AddJiggleBone)
			.def("AddIK", &Animator::AddIK)

		, class_<Action, boost::intrusive_ptr<Action> >("Action")
			.def(constructor<>())
			.def("AddTrack", &Action::AddTrack)
			.def("RemoveTrack", &Action::RemoveTrack)

		, class_<ActionTrack, boost::intrusive_ptr<ActionTrack> >("ActionTrack")

		, class_<ActionTrackAnimation, ActionTrack, boost::intrusive_ptr<ActionTrack> >("ActionTrackAnimation")
			.def(constructor<>())
			.def("AddKeyFrame", &ActionTrackAnimation::AddKeyFrame)

		, class_<ActionTrackSound, ActionTrack, boost::intrusive_ptr<ActionTrack> >("ActionTrackSound")
			.def(constructor<>())
			.def("AddKeyFrame", &ActionTrackSound::AddKeyFrame)

		, class_<ActionTrackEmitter, ActionTrack, boost::intrusive_ptr<ActionTrack> >("ActionTrackEmitter")
			.def(constructor<>())
			.def("AddKeyFrame", &ActionTrackEmitter::AddKeyFrame)
			.def_readwrite("EmitterMaterial", &ActionTrackEmitter::m_EmitterMaterial)
			.def_readwrite("EmitterCapacity", &ActionTrackEmitter::m_EmitterCapacity)
			.def_readwrite("EmitterFaceType", &ActionTrackEmitter::m_EmitterFaceType)
			.def_readwrite("ParticleLifeTime", &ActionTrackEmitter::m_ParticleLifeTime)
			.def_readwrite("ParticleVelocityX", &ActionTrackEmitter::m_ParticleVelocityX)
			.def_readwrite("ParticleVelocityY", &ActionTrackEmitter::m_ParticleVelocityY)
			.def_readwrite("ParticleVelocityZ", &ActionTrackEmitter::m_ParticleVelocityZ)
			.def_readwrite("ParticleColorR", &ActionTrackEmitter::m_ParticleColorR)
			.def_readwrite("ParticleColorG", &ActionTrackEmitter::m_ParticleColorG)
			.def_readwrite("ParticleColorB", &ActionTrackEmitter::m_ParticleColorB)
			.def_readwrite("ParticleColorA", &ActionTrackEmitter::m_ParticleColorA)
			.def_readwrite("ParticleSizeX", &ActionTrackEmitter::m_ParticleSizeX)
			.def_readwrite("ParticleSizeY", &ActionTrackEmitter::m_ParticleSizeY)
			.def_readwrite("ParticleAngle", &ActionTrackEmitter::m_ParticleAngle)
			.def_readwrite("AttachBoneName", &ActionTrackEmitter::m_AttachBoneName)

		, class_<ActionTrackPose, ActionTrack, boost::intrusive_ptr<ActionTrack> >("ActionTrackPose")
			.def(constructor<float>())
			.def("AddKeyFrame", &ActionTrackPose::AddKeyFrame)
			.def_readwrite("Length", &ActionTrackPose::m_Length)
			.def_readwrite("InterpolateX", &ActionTrackPose::m_InterpolateX)
			.def_readwrite("InterpolateY", &ActionTrackPose::m_InterpolateY)
			.def_readwrite("InterpolateZ", &ActionTrackPose::m_InterpolateZ)
			.def_readwrite("ParamStartPos", &ActionTrackPose::m_ParamStartPos)
			.def_readwrite("ParamEndPos", &ActionTrackPose::m_ParamEndPos)

		, class_<PhysxScene>("PhysxScene")
			.enum_("PxVisualizationParameter")
			[
				value("eSCALE", physx::PxVisualizationParameter::eSCALE),
				value("eWORLD_AXES", physx::PxVisualizationParameter::eWORLD_AXES),
				value("eBODY_AXES", physx::PxVisualizationParameter::eBODY_AXES),
				value("eBODY_MASS_AXES", physx::PxVisualizationParameter::eBODY_MASS_AXES),
				value("eBODY_LIN_VELOCITY", physx::PxVisualizationParameter::eBODY_LIN_VELOCITY),
				value("eBODY_ANG_VELOCITY", physx::PxVisualizationParameter::eBODY_ANG_VELOCITY),
				value("eDEPRECATED_BODY_JOINT_GROUPS", physx::PxVisualizationParameter::eDEPRECATED_BODY_JOINT_GROUPS),
				value("eCONTACT_POINT", physx::PxVisualizationParameter::eCONTACT_POINT),
				value("eCONTACT_NORMAL", physx::PxVisualizationParameter::eCONTACT_NORMAL),
				value("eCONTACT_ERROR", physx::PxVisualizationParameter::eCONTACT_ERROR),
				value("eACTOR_AXES", physx::PxVisualizationParameter::eACTOR_AXES),
				value("eCOLLISION_AABBS", physx::PxVisualizationParameter::eCOLLISION_AABBS),
				value("eCOLLISION_SHAPES", physx::PxVisualizationParameter::eCOLLISION_SHAPES),
				value("eCOLLISION_AXES", physx::PxVisualizationParameter::eCOLLISION_AXES),
				value("eCOLLISION_COMPOUNDS", physx::PxVisualizationParameter::eCOLLISION_COMPOUNDS),
				value("eCOLLISION_FNORMALS", physx::PxVisualizationParameter::eCOLLISION_FNORMALS),
				value("eCOLLISION_EDGES", physx::PxVisualizationParameter::eCOLLISION_EDGES),
				value("eCOLLISION_STATIC", physx::PxVisualizationParameter::eCOLLISION_STATIC),
				value("eCOLLISION_DYNAMIC", physx::PxVisualizationParameter::eCOLLISION_DYNAMIC),
				value("eDEPRECATED_COLLISION_PAIRS", physx::PxVisualizationParameter::eDEPRECATED_COLLISION_PAIRS),
				value("eJOINT_LOCAL_FRAMES", physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES),
				value("eJOINT_LIMITS", physx::PxVisualizationParameter::eJOINT_LIMITS),
				value("ePARTICLE_SYSTEM_POSITION", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_POSITION),
				value("ePARTICLE_SYSTEM_VELOCITY", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_VELOCITY),
				value("ePARTICLE_SYSTEM_COLLISION_NORMAL", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_COLLISION_NORMAL),
				value("ePARTICLE_SYSTEM_BOUNDS", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_BOUNDS),
				value("ePARTICLE_SYSTEM_GRID", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_GRID),
				value("ePARTICLE_SYSTEM_BROADPHASE_BOUNDS", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_BROADPHASE_BOUNDS),
				value("ePARTICLE_SYSTEM_MAX_MOTION_DISTANCE", physx::PxVisualizationParameter::ePARTICLE_SYSTEM_MAX_MOTION_DISTANCE),
				value("eCULL_BOX", physx::PxVisualizationParameter::eCULL_BOX),
				value("eCLOTH_VERTICAL", physx::PxVisualizationParameter::eCLOTH_VERTICAL),
				value("eCLOTH_HORIZONTAL", physx::PxVisualizationParameter::eCLOTH_HORIZONTAL),
				value("eCLOTH_BENDING", physx::PxVisualizationParameter::eCLOTH_BENDING),
				value("eCLOTH_SHEARING", physx::PxVisualizationParameter::eCLOTH_SHEARING),
				value("eCLOTH_VIRTUAL_PARTICLES", physx::PxVisualizationParameter::eCLOTH_VIRTUAL_PARTICLES),
				value("eMBP_REGIONS", physx::PxVisualizationParameter::eMBP_REGIONS),
				value("eNUM_VALUES", physx::PxVisualizationParameter::eNUM_VALUES)
			]
			.def("GetVisualizationParameter", &PhysxScene::GetVisualizationParameter)
			.def("SetVisualizationParameter", &PhysxScene::SetVisualizationParameter)
			.enum_("PxControllerDebugRenderFlag")
			[
				value("eTEMPORAL_BV", physx::PxControllerDebugRenderFlag::eTEMPORAL_BV),
				value("eCACHED_BV", physx::PxControllerDebugRenderFlag::eCACHED_BV),
				value("eOBSTACLES", physx::PxControllerDebugRenderFlag::eOBSTACLES),
				value("eNONE", physx::PxControllerDebugRenderFlag::eNONE),
				value("eALL", physx::PxControllerDebugRenderFlag::eALL)
			]
			.def("SetControllerDebugRenderingFlags", &PhysxScene::SetControllerDebugRenderingFlags)
	];

	module(m_State)[
		class_<noise::module::Perlin>("Perlin")
			.def(constructor<>())
			.property("Frequency", &noise::module::Perlin::GetFrequency, &noise::module::Perlin::SetFrequency)
			.property("Lacunarity", &noise::module::Perlin::GetLacunarity, &noise::module::Perlin::SetLacunarity)
			.property("NoiseQuality", &noise::module::Perlin::GetNoiseQuality, &noise::module::Perlin::SetNoiseQuality)
			.property("OctaveCount", &noise::module::Perlin::GetOctaveCount, &noise::module::Perlin::SetOctaveCount)
			.property("Persistence", &noise::module::Perlin::GetPersistence, &noise::module::Perlin::SetPersistence)
			.property("Seed", &noise::module::Perlin::GetSeed, &noise::module::Perlin::SetSeed)
			.def("GetValue", &noise::module::Perlin::GetValue)
	];
}

void LuaContext::Shutdown(void)
{
	if (m_State)
	{
		lua_close(m_State);
		m_State = NULL;
	}
}

LuaContext::~LuaContext(void)
{
	_ASSERT(!m_State);
}

static int traceback (lua_State *L)
{
	if (!lua_isstring(L, 1))  /* 'message' not a string? */
		return 1;  /* keep it intact */
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */
	return 1;
}

int LuaContext::docall(int narg, int clear)
{
	int status;
	int base = lua_gettop(m_State) - narg;  /* function index */
	lua_pushcfunction(m_State, traceback);  /* push traceback function */
	lua_insert(m_State, base);  /* put it under chunk and args */
	//signal(SIGINT, laction);
	status = lua_pcall(m_State, narg, (clear ? 0 : LUA_MULTRET), base);
	//signal(SIGINT, SIG_DFL);
	lua_remove(m_State, base);  /* remove traceback function */
	/* force a complete garbage collection in case of errors */
	if (status != 0) lua_gc(m_State, LUA_GCCOLLECT, 0);
	return status;
}

int LuaContext::dostring(const char *s, const char *name)
{
	return luaL_loadbuffer(m_State, s, strlen(s), name) || docall(0, 1);
}

int LuaContext::dogcstep(int step)
{
	return lua_gc(m_State, LUA_GCSTEP, step);
}
