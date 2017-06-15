#include "stdafx.h"
#include "LuaExtension.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/iterator_policy.hpp>
#include "Actor.h"

namespace luabind
{
	template <>
	struct default_converter<std::wstring>
		: native_converter_base<std::wstring>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
		}

		std::wstring from(lua_State* L, int index)
		{
			return u8tows(lua_tostring(L, index));
		}

		void to(lua_State* L, std::wstring const& value)
		{
			std::string str = wstou8(value);
			lua_pushlstring(L, str.data(), str.size());
		}
	};

	template <>
	struct default_converter<std::wstring const>
		: default_converter<std::wstring>
	{
	};

	template <>
	struct default_converter<std::wstring const&>
		: default_converter<std::wstring>
	{
	};

	template <>
	struct default_converter<my::ControlEvent>
		: native_converter_base<my::ControlEvent>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::ControlEvent from(lua_State * L, int index)
		{
			struct InternalExceptionHandler
			{
				luabind::object obj;
				InternalExceptionHandler(const luabind::object & _obj)
					: obj(_obj)
				{
				}
				void operator()(my::ControlEventArgs * args)
				{
					obj(args);
				}
			};
			return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
		}

		void to(lua_State * L, my::ControlEvent const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::ControlEvent const &>
		: default_converter<my::ControlEvent>
	{
	};

	template <>
	struct default_converter<my::TimerEvent>
		: native_converter_base<my::TimerEvent>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::TimerEvent from(lua_State * L, int index)
		{
			struct InternalExceptionHandler
			{
				luabind::object obj;
				InternalExceptionHandler(const luabind::object & _obj)
					: obj(_obj)
				{
				}
				void operator()(float interval)
				{
					obj(interval);
				}
			};
			return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
		}

		void to(lua_State * L, my::TimerEvent const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::TimerEvent const &>
		: default_converter<my::TimerEvent>
	{
	};
}

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

static void ExportMath(lua_State * L)
{
	using namespace luabind;
	module(L)
	[
		class_<my::Vector2, boost::shared_ptr<my::Vector2> >("Vector2")
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
			.def("magnitude", &my::Vector2::magnitude)
			.def("magnitudeSq", &my::Vector2::magnitudeSq)
			.def("lerp", &my::Vector2::lerp)
			.def("lerpSelf", &my::Vector2::lerpSelf)
			.def("normalize", &my::Vector2::normalize)
			.def("normalizeSelf", &my::Vector2::normalizeSelf)
			.def("transform", &my::Vector2::transform)
			.def("transformTranspose", &my::Vector2::transformTranspose)
			.def("transformCoord", &my::Vector2::transformCoord)
			.def("transformCoordTranspose", &my::Vector2::transformCoordTranspose)
			.def("transformNormal", &my::Vector2::transformNormal)
			.def("transformNormalTranspose", &my::Vector2::transformNormalTranspose)
			.def("transform", &my::Vector2::transform)

		, class_<my::Vector3, boost::shared_ptr<my::Vector3> >("Vector3")
			.def(constructor<float, float, float>())
			.def_readwrite("x", &my::Vector3::x)
			.def_readwrite("y", &my::Vector3::y)
			.def_readwrite("z", &my::Vector3::z)
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
			.def("magnitude", &my::Vector3::magnitude)
			.def("magnitudeSq", &my::Vector3::magnitudeSq)
			.def("lerp", &my::Vector3::lerp)
			.def("lerpSelf", &my::Vector3::lerpSelf)
			.def("normalize", &my::Vector3::normalize)
			.def("normalizeSelf", &my::Vector3::normalizeSelf)
			.def("transform", (my::Vector4 (my::Vector3::*)(const my::Matrix4 &) const)&my::Vector3::transform)
			.def("transformTranspose", &my::Vector3::transformTranspose)
			.def("transformCoord", &my::Vector3::transformCoord)
			.def("transformCoordTranspose", &my::Vector3::transformCoordTranspose)
			.def("transformNormal", &my::Vector3::transformNormal)
			.def("transformNormalTranspose", &my::Vector3::transformNormalTranspose)
			.def("transform", (my::Vector3 (my::Vector3::*)(const my::Quaternion &) const)&my::Vector3::transform)

		, class_<my::Vector4, boost::shared_ptr<my::Vector4> >("Vector4")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Vector4::x)
			.def_readwrite("y", &my::Vector4::y)
			.def_readwrite("z", &my::Vector4::z)
			.def_readwrite("w", &my::Vector4::w)
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
			.def("magnitude", &my::Vector4::magnitude)
			.def("magnitudeSq", &my::Vector4::magnitudeSq)
			.def("lerp", &my::Vector4::lerp)
			.def("lerpSelf", &my::Vector4::lerpSelf)
			.def("normalize", &my::Vector4::normalize)
			.def("normalizeSelf", &my::Vector4::normalizeSelf)
			.def("transform", &my::Vector4::transform)
			.def("transformTranspose", &my::Vector4::transformTranspose)

		, class_<my::Rectangle, boost::shared_ptr<my::Rectangle> >("Rectangle")
			.def(constructor<float, float, float, float>())
			.def_readwrite("l", &my::Rectangle::l)
			.def_readwrite("t", &my::Rectangle::t)
			.def_readwrite("r", &my::Rectangle::r)
			.def_readwrite("b", &my::Rectangle::b)
			.def("intersect", &my::Rectangle::intersect)
			.def("intersectSelf", &my::Rectangle::intersectSelf)
			.def("Union", &my::Rectangle::Union)
			.def("unionSelf", &my::Rectangle::unionSelf)
			.def("offset", (my::Rectangle (my::Rectangle::*)(float, float) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::offsetSelf)
			.def("offset", (my::Rectangle (my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::offsetSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(float, float, float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float, float, float))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(const my::Vector4 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector4 &))&my::Rectangle::shrinkSelf)
			.def("LeftTop", (my::Vector2 (my::Rectangle::*)(void) const)&my::Rectangle::LeftTop)
			.def("RightBottom", (my::Vector2 (my::Rectangle::*)(void) const)&my::Rectangle::RightBottom)
			.def("Center", &my::Rectangle::Center)
			.def("Width", &my::Rectangle::Width)
			.def("Height", &my::Rectangle::Height)
			.def("Extent", &my::Rectangle::Extent)
			.def("PtInRect", &my::Rectangle::PtInRect)
			.scope
			[
				def("LeftTop", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::LeftTop),
				def("LeftTop", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftTop),
				def("LeftMiddle", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::LeftMiddle),
				def("LeftMiddle", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftMiddle),
				def("LeftBottom", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::LeftBottom),
				def("LeftBottom", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftBottom),
				def("CenterTop", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::CenterTop),
				def("CenterTop", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterTop),
				def("CenterMiddle", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::CenterMiddle),
				def("CenterMiddle", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterMiddle),
				def("CenterBottom", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::CenterBottom),
				def("CenterBottom", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterBottom),
				def("RightTop", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::RightTop),
				def("RightTop", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightTop),
				def("RightMiddle", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::RightMiddle),
				def("RightMiddle", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightMiddle),
				def("RightBottom", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::RightBottom),
				def("RightBottom", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightBottom)
			]

		, class_<my::Quaternion, boost::shared_ptr<my::Quaternion> >("Quaternion")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Quaternion::x)
			.def_readwrite("y", &my::Quaternion::y)
			.def_readwrite("z", &my::Quaternion::z)
			.def_readwrite("w", &my::Quaternion::w)
			.def(self + other<const my::Quaternion &>())
			.def(self + float())
			.def(self - other<const my::Quaternion &>())
			.def(self - float())
			.def(self * other<const my::Quaternion &>())
			.def(self * float())
			.def(self / other<const my::Quaternion &>())
			.def(self / float())
			.def("conjugate", &my::Quaternion::conjugate)
			.def("conjugateSelf", &my::Quaternion::conjugateSelf)
			.def("dot", &my::Quaternion::dot)
			.def("inverse", &my::Quaternion::inverse)
			.def("magnitude", &my::Quaternion::magnitude)
			.def("magnitudeSq", &my::Quaternion::magnitudeSq)
			.def("ln", &my::Quaternion::ln)
			.def("multiply", &my::Quaternion::multiply)
			.def("normalize", &my::Quaternion::normalize)
			.def("normalizeSelf", &my::Quaternion::normalizeSelf)
			.def("lerp", &my::Quaternion::lerp)
			.def("lerpSelf", &my::Quaternion::lerpSelf)
			.def("slerp", &my::Quaternion::slerp)
			.def("slerpSelf", &my::Quaternion::slerpSelf)
			.def("squad", &my::Quaternion::squad)
			.def("squadSelf", &my::Quaternion::squadSelf)
			.scope
			[
				def("Identity", &my::Quaternion::Identity),
				def("RotationAxis", &my::Quaternion::RotationAxis),
				def("RotationMatrix", &my::Quaternion::RotationMatrix),
				def("RotationYawPitchRoll", &my::Quaternion::RotationYawPitchRoll),
				def("RotationFromTo", &my::Quaternion::RotationFromTo)
			]

		, class_<my::Matrix4, boost::shared_ptr<my::Matrix4> >("Matrix4")
			.def(self + other<const my::Matrix4 &>())
			.def(self + float())
			.def(self - other<const my::Matrix4 &>())
			.def(self - float())
			.def(self * other<const my::Matrix4 &>())
			.def(self * float())
			.def(self / other<const my::Matrix4 &>())
			.def(self / float())
			.def("inverse", &my::Matrix4::inverse)
			.def("multiply", &my::Matrix4::multiply)
			.def("multiplyTranspose", &my::Matrix4::multiplyTranspose)
			.def("transpose", &my::Matrix4::transpose)
			.def("transformTranspose", &my::Matrix4::transformTranspose)
			.def("scale", (my::Matrix4 (my::Matrix4::*)(float, float, float) const)&my::Matrix4::scale)
			.def("scale", (my::Matrix4 (my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::scale)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::scaleSelf)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::scaleSelf)
			.def("rotateX", &my::Matrix4::rotateX)
			.def("rotateY", &my::Matrix4::rotateY)
			.def("rotateZ", &my::Matrix4::rotateZ)
			.def("rotate", &my::Matrix4::rotate)
			.def("translate", (my::Matrix4 (my::Matrix4::*)(float, float, float) const)&my::Matrix4::translate)
			.def("translate", (my::Matrix4 (my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::translate)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::translateSelf)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::translateSelf)
			.def("lerp", &my::Matrix4::lerp)
			.def("lerpSelf", &my::Matrix4::lerpSelf)
			.scope
			[
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
				def("Scaling", (my::Matrix4 (*)(float, float, float))&my::Matrix4::Scaling),
				def("Scaling", (my::Matrix4 (*)(const my::Vector3 &))&my::Matrix4::Scaling),
				def("Transformation", &my::Matrix4::Transformation),
				def("Transformation2D", &my::Matrix4::Transformation2D),
				def("Translation", (my::Matrix4 (*)(float, float, float))&my::Matrix4::Translation),
				def("Translation", (my::Matrix4 (*)(const my::Vector3 &))&my::Matrix4::Translation)
			]

		, class_<my::AABB, boost::shared_ptr<my::AABB> >("AABB")
			.def(constructor<float, float>())
			.def(constructor<float, float, float, float, float, float>())
			.def(constructor<const my::Vector3 &, const my::Vector3 &>())
			.def_readwrite("min", &my::AABB::m_min)
			.def_readwrite("max", &my::AABB::m_max)
			.def("transform", &my::AABB::transform)

		, class_<my::OctActor, boost::shared_ptr<my::OctActor> >("OctActor")

		//, class_<my::Octree, boost::shared_ptr<my::Octree> >("Octree")
		//	.def("AddActor", &my::Octree::AddActor)
		//	.def("RemoveActor", &my::Octree::RemoveActor)
		//	.def("ClearAllActor", &my::Octree::ClearAllActor)

		, class_<my::BaseCamera, boost::shared_ptr<my::BaseCamera> >("BaseCamera")
			.def_readonly("View", &my::BaseCamera::m_View)
			.def_readonly("Proj", &my::BaseCamera::m_Proj)
			.def_readonly("ViewProj", &my::BaseCamera::m_ViewProj)
			.def_readonly("InverseViewProj", &my::BaseCamera::m_InverseViewProj)

			, class_<my::Camera, my::BaseCamera, boost::shared_ptr<my::Camera> >("Camera")
			.def_readwrite("Aspect", &my::Camera::m_Aspect)
			.def_readwrite("Nz", &my::Camera::m_Nz)
			.def_readwrite("Fz", &my::Camera::m_Fz)
			.def_readwrite("EventAlign", &my::Camera::EventAlign)

		, class_<my::OrthoCamera, my::BaseCamera, boost::shared_ptr<my::OrthoCamera> >("OrthoCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Eye", &my::OrthoCamera::m_Eye)
			.def_readwrite("Eular", &my::OrthoCamera::m_Eular)
			.def_readwrite("Diagonal", &my::OrthoCamera::m_Diagonal)

		, class_<my::ModelViewerCamera, my::Camera, boost::shared_ptr<my::Camera> >("ModelViewerCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("LookAt", &my::ModelViewerCamera::m_LookAt)
			.def_readwrite("Eular", &my::ModelViewerCamera::m_Eular)
			.def_readwrite("Distance", &my::ModelViewerCamera::m_Distance)

		, class_<my::FirstPersonCamera, my::Camera, boost::shared_ptr<my::Camera> >("FirstPersonCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Eye", &my::FirstPersonCamera::m_Eye)
			.def_readwrite("Eular", &my::FirstPersonCamera::m_Eular)
	];
}

static DWORD ARGB(int a, int r, int g, int b)
{
	return D3DCOLOR_ARGB(a,r,g,b);
}

static void ExportUI(lua_State * L)
{
	using namespace luabind;
	module(L)
	[
		def("ARGB", &ARGB)

		, class_<my::ControlEventArgs, boost::shared_ptr<my::ControlEventArgs> >("ControlEventArgs")

		, class_<my::ControlEvent>("ControlEvent")

		, class_<my::ControlImage, boost::shared_ptr<my::ControlImage> >("ControlImage")
			.def(constructor<>())
			.def_readwrite("Texture", &my::ControlImage::m_Texture)
			.def_readwrite("Rect", &my::ControlImage::m_Rect)
			.def_readwrite("Border", &my::ControlImage::m_Border)

		, class_<my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ControlSkin")
			.def(constructor<>())
			.def_readwrite("Image", &my::ControlSkin::m_Image)
			.def_readwrite("Font", &my::ControlSkin::m_Font)
			.def_readwrite("TextColor", &my::ControlSkin::m_TextColor)
			.def_readwrite("TextAlign", &my::ControlSkin::m_TextAlign)

		, class_<my::Control, boost::shared_ptr<my::Control> >("Control")
			.def(constructor<>())
			.def_readwrite("Name", &my::Control::m_Name)
			.property("Enabled", &my::Control::GetEnabled, &my::Control::SetEnabled)
			.property("Visible", &my::Control::GetVisible, &my::Control::SetVisible)
			.def_readwrite("Location", &my::Control::m_Location)
			.def_readwrite("Size", &my::Control::m_Size)
			.def_readwrite("Color", &my::Control::m_Color)
			.def_readwrite("Skin", &my::Control::m_Skin)
			.def("InsertControl", &my::Control::InsertControl)
			.def("RemoveControl", &my::Control::RemoveControl)
			.def("ClearAllControl", &my::Control::ClearAllControl)
			.def("FindControl", &my::Control::FindControl)
			.def("FindControlRecurse", &my::Control::FindControlRecurse)
			.def_readwrite("EventMouseEnter", &my::Control::EventMouseEnter)
			.def_readwrite("EventMouseLeave", &my::Control::EventMouseLeave)

		, class_<my::Static, my::Control, boost::shared_ptr<my::Control> >("Static")
			.def(constructor<>())
			.def_readwrite("Text", &my::Static::m_Text)

		, class_<my::ProgressBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ProgressBarSkin")
			.def(constructor<>())
			.def_readwrite("ForegroundImage", &my::ProgressBarSkin::m_ForegroundImage)

		, class_<my::ProgressBar, my::Static, boost::shared_ptr<my::Control> >("ProgressBar")
			.def(constructor<>())
			.def_readwrite("Progress", &my::ProgressBar::m_Progress)

		, class_<my::ButtonSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ButtonSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::ButtonSkin::m_DisabledImage)
			.def_readwrite("PressedImage", &my::ButtonSkin::m_PressedImage)
			.def_readwrite("MouseOverImage", &my::ButtonSkin::m_MouseOverImage)
			.def_readwrite("PressedOffset", &my::ButtonSkin::m_PressedOffset)

		, class_<my::Button, my::Static, boost::shared_ptr<my::Control> >("Button")
			.def(constructor<>())
			.def_readwrite("EventClick", &my::Button::EventClick)
			.def("SetHotkey", &my::Button::SetHotkey)

		, class_<my::EditBoxSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("EditBoxSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::EditBoxSkin::m_DisabledImage)
			.def_readwrite("FocusedImage", &my::EditBoxSkin::m_FocusedImage)
			.def_readwrite("SelBkColor", &my::EditBoxSkin::m_SelBkColor)
			.def_readwrite("CaretColor", &my::EditBoxSkin::m_CaretColor)

		, class_<my::EditBox, my::Static, boost::shared_ptr<my::Control> >("EditBox")
			.def(constructor<>())
			.property("Text", &my::EditBox::GetText, &my::EditBox::SetText)
			.def_readwrite("Border", &my::EditBox::m_Border)
			.def_readwrite("EventChange", &my::EditBox::EventChange)
			.def_readwrite("EventEnter", &my::EditBox::EventEnter)

		, class_<my::ImeEditBox, my::EditBox, boost::shared_ptr<my::Control> >("ImeEditBox")
			.def(constructor<>())

		, class_<my::ScrollBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ScrollBarSkin")
			.def(constructor<>())
			.def_readwrite("UpBtnNormalImage", &my::ScrollBarSkin::m_UpBtnNormalImage)
			.def_readwrite("UpBtnDisabledImage", &my::ScrollBarSkin::m_UpBtnDisabledImage)
			.def_readwrite("DownBtnNormalImage", &my::ScrollBarSkin::m_DownBtnNormalImage)
			.def_readwrite("DownBtnDisabledImage", &my::ScrollBarSkin::m_DownBtnDisabledImage)
			.def_readwrite("ThumbBtnNormalImage", &my::ScrollBarSkin::m_ThumbBtnNormalImage)

		, class_<my::ScrollBar, my::Control, boost::shared_ptr<my::Control> >("ScrollBar")
			.def(constructor<>())
			.def_readwrite("nPosition", &my::ScrollBar::m_nPosition) // ! should use property
			.def_readwrite("nPageSize", &my::ScrollBar::m_nPageSize) // ! should use property
			.def_readwrite("nStart", &my::ScrollBar::m_nStart) // ! should use property
			.def_readwrite("nEnd", &my::ScrollBar::m_nEnd) // ! should use property

		, class_<my::CheckBox, my::Button, boost::shared_ptr<my::Control> >("CheckBox")
			.def(constructor<>())
			.def_readwrite("Checked", &my::CheckBox::m_Checked)

		, class_<my::ComboBoxSkin, my::ButtonSkin, boost::shared_ptr<my::ControlSkin> >("ComboBoxSkin")
			.def(constructor<>())
			.def_readwrite("DropdownImage", &my::ComboBoxSkin::m_DropdownImage)
			.def_readwrite("DropdownItemMouseOverImage", &my::ComboBoxSkin::m_DropdownItemMouseOverImage)
			.def_readwrite("ScrollBarUpBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarUpBtnNormalImage)
			.def_readwrite("ScrollBarUpBtnDisabledImage", &my::ComboBoxSkin::m_ScrollBarUpBtnDisabledImage)
			.def_readwrite("ScrollBarDownBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarDownBtnNormalImage)
			.def_readwrite("ScrollBarDownBtnDisabledImage", &my::ComboBoxSkin::m_ScrollBarDownBtnDisabledImage)
			.def_readwrite("ScrollBarThumbBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarThumbBtnNormalImage)
			.def_readwrite("ScrollBarImage", &my::ComboBoxSkin::m_ScrollBarImage)

		, class_<my::ComboBox, my::Button, boost::shared_ptr<my::Control> >("ComboBox")
			.def(constructor<>())
			.property("DropdownSize", &my::ComboBox::GetDropdownSize, &my::ComboBox::SetDropdownSize)
			.property("Border", &my::ComboBox::GetBorder, &my::ComboBox::SetBorder)
			.property("ItemHeight", &my::ComboBox::GetItemHeight, &my::ComboBox::SetItemHeight)
			.property("Selected", &my::ComboBox::GetSelected, &my::ComboBox::SetSelected)
			.def_readwrite("ScrollbarWidth", &my::ComboBox::m_ScrollbarWidth)
			.def("AddItem", &my::ComboBox::AddItem)
			.def("RemoveAllItems", &my::ComboBox::RemoveAllItems)
			.def("ContainsItem", &my::ComboBox::ContainsItem)
			.def("FindItem", &my::ComboBox::FindItem)
			.def("GetItemData", &my::ComboBox::GetItemDataUInt)
			.def("SetItemData", (void (my::ComboBox::*)(int, unsigned int))&my::ComboBox::SetItemData)
			.def("GetNumItems", &my::ComboBox::GetNumItems)
			.def_readwrite("EventSelectionChanged", &my::ComboBox::EventSelectionChanged)

		, class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(constructor<>())
			.def_readwrite("World", &my::Dialog::m_World)
			.def("Refresh", &my::Dialog::Refresh)
			.def_readwrite("EventAlign", &my::Dialog::EventAlign)
			.def_readwrite("EventRefresh", &my::Dialog::EventRefresh)

		, class_<my::DialogMgr>("DialogMgr")
			.property("DlgViewport", &my::DialogMgr::GetDlgViewport, &my::DialogMgr::SetDlgViewport)
			.def("InsertDlg", &my::DialogMgr::InsertDlg)
			.def("RemoveDlg", &my::DialogMgr::RemoveDlg)
			.def("RemoveAllDlg", &my::DialogMgr::RemoveAllDlg)
	];
}

static void ExportEmitter(lua_State * L)
{
	using namespace luabind;
	module(L)
	[
		class_<my::Spline, boost::shared_ptr<my::Spline> >("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("Interpolate", (float (my::Spline::*)(float, float) const)&my::Spline::Interpolate)

		, class_<my::Emitter, boost::shared_ptr<my::Emitter> >("Emitter")
			.def(constructor<>())
			.def("Spawn", &my::Emitter::Spawn)
	];
}

static void ExportResource(lua_State * L)
{
	using namespace luabind;
	module(L)
	[
		class_<my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("BaseTexture")

		, class_<my::Texture2D, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("Texture2D")

		, class_<my::CubeTexture, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("CubeTexture")

		, class_<my::Mesh, boost::shared_ptr<my::Mesh> >("Mesh")
			.def("GetNumFaces", &my::Mesh::GetNumFaces)
			.def("GetNumVertices", &my::Mesh::GetNumVertices)

		, class_<my::OgreMesh, my::Mesh, boost::shared_ptr<my::OgreMesh> >("OgreMesh")
			.def("SaveOgreMesh", &my::OgreMesh::SaveOgreMesh)
			.def("SaveSimplifiedOgreMesh", &my::OgreMesh::SaveSimplifiedOgreMesh)
			.def("GetMaterialNum", &my::OgreMesh::GetMaterialNum)
			.def("GetMaterialName", &my::OgreMesh::GetMaterialName)

		, class_<my::OgreSkeletonAnimation, boost::shared_ptr<my::OgreSkeletonAnimation> >("OgreSkeletonAnimation")

		// ! many methods of my::BaseEffect, my::Effect cannot be use in lua
		, class_<my::BaseEffect, boost::shared_ptr<my::BaseEffect> >("BaseEffect")
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

		, class_<my::Effect, my::BaseEffect, boost::shared_ptr<my::Effect> >("Effect")
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

		, class_<my::Font, boost::shared_ptr<my::Font> >("Font")
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
			.property("Scale", &my::Font::GetScale, &my::Font::SetScale)
			.def_readonly("LineHeight", &my::Font::m_LineHeight)

		, class_<my::ResourceMgr>("ResourceMgr")
			.def("LoadTexture", &my::ResourceMgr::LoadTexture)
			.def("LoadMesh", &my::ResourceMgr::LoadMesh)
			.def("LoadSkeleton", &my::ResourceMgr::LoadSkeleton)
			.def("LoadEffect", &my::ResourceMgr::LoadEffect)
			.def("LoadFont", &my::ResourceMgr::LoadFont)

		, def("res2texture", &boost::dynamic_pointer_cast<my::BaseTexture, my::DeviceResourceBase>)
		, def("res2mesh", &boost::dynamic_pointer_cast<my::OgreMesh, my::DeviceResourceBase>)
		, def("res2skeleton", &boost::dynamic_pointer_cast<my::OgreSkeletonAnimation, my::DeviceResourceBase>)
		, def("res2effect", &boost::dynamic_pointer_cast<my::Effect, my::DeviceResourceBase>)
		, def("res2font", &boost::dynamic_pointer_cast<my::Font, my::DeviceResourceBase>)
	];
}

static void ExportDevice(lua_State * L)
{
	using namespace luabind;
	module(L)
	[
		class_<D3DSURFACE_DESC>("D3DSURFACE_DESC")
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
			.def("GetSize", &CGrowableArray<D3DDISPLAYMODE>::GetSize)

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
			.def("GetSize", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetSize)

		, class_<CD3D9EnumDeviceInfo>("CD3D9EnumDeviceInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceInfo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceInfo::DeviceType)
			.def_readonly("deviceSettingsComboList", &CD3D9EnumDeviceInfo::deviceSettingsComboList)

		, class_<CGrowableArray<CD3D9EnumDeviceInfo *> >("CD3D9EnumDeviceInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetSize)

		, class_<CGrowableArray<D3DFORMAT> >("D3DFORMATArray")
			.def("GetAt", &CGrowableArray<D3DFORMAT>::GetAt)
			.def("GetSize", &CGrowableArray<D3DFORMAT>::GetSize)

		, class_<CGrowableArray<D3DMULTISAMPLE_TYPE> >("D3DMULTISAMPLE_TYPEArray")
			.def("GetAt", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetAt)
			.def("GetSize", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetSize)

		, class_<CGrowableArray<DWORD> >("DWORDArray")
			.def("GetAt", (const DWORD & (CGrowableArray<DWORD>::*)(int))&CGrowableArray<DWORD>::GetAt) // ! forced convert to const ref
			.def("GetSize", &CGrowableArray<DWORD>::GetSize)

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
			.def("GetSize", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetSize)

		, class_<CD3D9Enumeration>("CD3D9Enumeration")
			.def("GetAdapterInfoList", &CD3D9Enumeration::GetAdapterInfoList)
			.def("GetAdapterInfo", &CD3D9Enumeration::GetAdapterInfo)
			.def("GetDeviceInfo", &CD3D9Enumeration::GetDeviceInfo)
			.def("GetDeviceSettingsCombo", (CD3D9EnumDeviceSettingsCombo *(CD3D9Enumeration::*)(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL))&CD3D9Enumeration::GetDeviceSettingsCombo)

		, class_<my::DxutApp, CD3D9Enumeration>("DxutApp")
			.scope
			[
				def("DXUTD3DDeviceTypeToString", &my::DxutApp::DXUTD3DDeviceTypeToString),
				def("DXUTD3DFormatToString", &my::DxutApp::DXUTD3DFormatToString),
				def("DXUTMultisampleTypeToString", &my::DxutApp::DXUTMultisampleTypeToString),
				def("DXUTVertexProcessingTypeToString", &my::DxutApp::DXUTVertexProcessingTypeToString)
			]
			.def_readonly("BackBufferSurfaceDesc", &my::DxutApp::m_BackBufferSurfaceDesc)
			.def_readonly("DeviceSettings", &my::DxutApp::m_DeviceSettings)
			.def("ToggleFullScreen", &my::DxutApp::ToggleFullScreen)
			.def("ToggleREF", &my::DxutApp::ToggleREF)
			.property("SoftwareVP", &my::DxutApp::GetSoftwareVP, &my::DxutApp::SetSoftwareVP)
			.property("HardwareVP", &my::DxutApp::GetHardwareVP, &my::DxutApp::SetHardwareVP)
			.property("PureHardwareVP", &my::DxutApp::GetPureHardwareVP, &my::DxutApp::SetPureHardwareVP)
			.property("MixedVP", &my::DxutApp::GetMixedVP, &my::DxutApp::SetMixedVP)
			.def("ChangeDevice", &my::DxutApp::ChangeDevice)

		, class_<my::Timer, boost::shared_ptr<my::Timer> >("Timer")
			.def(constructor<float, float>())
			.def_readonly("Interval", &my::Timer::m_Interval)
			.def_readonly("RemainingTime", &my::Timer::m_RemainingTime)
			.def_readwrite("EventTimer", &my::Timer::m_EventTimer)
	];
}

static void ExportComponent(lua_State * L)
{
	using namespace luabind;
	module(L)
	[
		class_<ResourceBundle<my::BaseTexture> >("ResourceTextureBundle")
			.def(constructor<const char *>())
			.def(constructor<>())
			.def_readwrite("Path", &ResourceBundle<my::BaseTexture>::m_Path)
			.def_readwrite("Res", &ResourceBundle<my::BaseTexture>::m_Res)

		, class_<ResourceBundle<my::Mesh> >("ResourceMeshBundle")
			.def(constructor<const char *>())
			.def(constructor<>())
			.def_readwrite("Path", &ResourceBundle<my::Mesh>::m_Path)
			.def_readwrite("Res", &ResourceBundle<my::Mesh>::m_Res)

		, class_<ResourceBundle<my::OgreSkeletonAnimation> >("ResourceSkeletonBundle")
			.def(constructor<const char *>())
			.def(constructor<>())
			.def_readwrite("Path", &ResourceBundle<my::OgreSkeletonAnimation>::m_Path)
			.def_readwrite("Res", &ResourceBundle<my::OgreSkeletonAnimation>::m_Res)

		, class_<Animator, boost::shared_ptr<Animator> >("Animator")
			.def_readonly("SkeletonRes", &Animator::m_SkeletonRes)

		, class_<Material, boost::shared_ptr<Material> >("Material")
			.enum_("PassMask")
			[
				value("PassMaskNone", RenderPipeline::PassMaskNone),
				value("PassMaskLight", RenderPipeline::PassMaskLight),
				value("PassMaskOpaque", RenderPipeline::PassMaskOpaque),
				value("PassMaskTransparent", RenderPipeline::PassMaskTransparent)
			]
			.def(constructor<>())
			.def_readwrite("Shader", &Material::m_Shader)
			.def_readwrite("PassMask", &Material::m_PassMask)
			.def_readwrite("MeshColor", &Material::m_MeshColor)
			.def_readonly("MeshTexture", &Material::m_MeshTexture)
			.def_readonly("NormalTexture", &Material::m_NormalTexture)
			.def_readonly("SpecularTexture", &Material::m_SpecularTexture)

		, class_<Component, boost::shared_ptr<Component> >("Component")
			.def_readwrite("Position", &Component::m_Position)
			.def_readwrite("Rotation", &Component::m_Rotation)
			.def_readwrite("Scale", &Component::m_Scale)
			.def_readwrite("World", &Component::m_World)
			.def("RequestResource", &Component::RequestResource)
			.def("ReleaseResource", &Component::ReleaseResource)

		, class_<RenderComponent, Component, boost::shared_ptr<Component> >("RenderComponent")

		, class_<MeshComponent, RenderComponent, boost::shared_ptr<Component> >("MeshComponent")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, const my::Vector3 &>())
			.def(constructor<>())
			.def_readonly("MeshRes", &MeshComponent::m_MeshRes)
			.def_readonly("bInstance", &MeshComponent::m_bInstance)
			.def_readonly("UseAnimation", &MeshComponent::m_bUseAnimation)

		, class_<EmitterComponent, RenderComponent, boost::shared_ptr<Component> >("EmitterComponent")
			.def_readwrite("Emitter", &EmitterComponent::m_Emitter)
			.def_readwrite("Material", &EmitterComponent::m_Material)

		, class_<StaticEmitterComponent, EmitterComponent, boost::shared_ptr<Component> >("StaticEmitterComponent")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, const my::Vector3 &>())
			.def(constructor<>())

		, class_<SphericalEmitterComponent, EmitterComponent, boost::shared_ptr<Component> >("SphericalEmitterComponent")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, const my::Vector3 &>())
			.def(constructor<>())
			.def_readwrite("ParticleLifeTime", &SphericalEmitterComponent::m_ParticleLifeTime)
			.def_readwrite("SpawnInterval", &SphericalEmitterComponent::m_SpawnInterval)
			.def_readwrite("HalfSpawnArea", &SphericalEmitterComponent::m_HalfSpawnArea)
			.def_readwrite("SpawnSpeed", &SphericalEmitterComponent::m_SpawnSpeed)
			.def_readwrite("SpawnInclination", &SphericalEmitterComponent::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &SphericalEmitterComponent::m_SpawnAzimuth)
			.def_readwrite("SpawnColorR", &SphericalEmitterComponent::m_SpawnColorR)
			.def_readwrite("SpawnColorG", &SphericalEmitterComponent::m_SpawnColorG)
			.def_readwrite("SpawnColorB", &SphericalEmitterComponent::m_SpawnColorB)
			.def_readwrite("SpawnColorA", &SphericalEmitterComponent::m_SpawnColorA)
			.def_readwrite("SpawnSizeX", &SphericalEmitterComponent::m_SpawnSizeX)
			.def_readwrite("SpawnSizeY", &SphericalEmitterComponent::m_SpawnSizeY)
			.def_readwrite("SpawnAngle", &SphericalEmitterComponent::m_SpawnAngle)
			.def_readwrite("SpawnLoopTime", &SphericalEmitterComponent::m_SpawnLoopTime)

		, class_<Actor, Component, boost::shared_ptr<Component> >("Actor")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, const my::Vector3 &, const my::AABB &>())
			.def(constructor<>())
			.def_readwrite("aabb", &Actor::m_aabb)
			.def_readwrite("Animator", &Actor::m_Animator)
			.def("AddComponent", &Actor::AddComponent)
			.def("RemoveComponent", &Actor::RemoveComponent)
			.def("ClearAllComponent", &Actor::ClearAllComponent)

		//, def("cmp2oct", &boost::static_pointer_cast<my::OctActor, Component>)
	];
}

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

	ExportMath(m_State);
	ExportResource(m_State);
	ExportUI(m_State);
	ExportEmitter(m_State);
	ExportDevice(m_State);
	ExportComponent(m_State);
}

LuaContext::~LuaContext(void)
{
	if (m_State)
	{
		lua_close(m_State);
	}
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
