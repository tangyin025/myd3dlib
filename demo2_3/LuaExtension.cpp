#include "stdafx.h"
#include "LuaExtension.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/iterator_policy.hpp>
#include "LuaExtension.inl"
#include "Console.h"
#include "Material.h"
#include "RenderPipeline.h"
#include "Character.h"
#include "Animation.h"
#include "Terrain.h"
#include "ActionTrack.h"

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
		class_<my::Vector2>("Vector2")
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

		, class_<my::Vector3>("Vector3")
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
			.def("transform", (my::Vector4(my::Vector3::*)(const my::Matrix4 &) const)&my::Vector3::transform)
			.def("transformTranspose", &my::Vector3::transformTranspose)
			.def("transformCoord", &my::Vector3::transformCoord)
			.def("transformCoordTranspose", &my::Vector3::transformCoordTranspose)
			.def("transformNormal", &my::Vector3::transformNormal)
			.def("transformNormalTranspose", &my::Vector3::transformNormalTranspose)
			.def("transform", (my::Vector3(my::Vector3::*)(const my::Quaternion &) const)&my::Vector3::transform)

		, class_<my::Vector4>("Vector4")
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

		, class_<my::Rectangle>("Rectangle")
			.def(constructor<float, float, float, float>())
			.def_readwrite("l", &my::Rectangle::l)
			.def_readwrite("t", &my::Rectangle::t)
			.def_readwrite("r", &my::Rectangle::r)
			.def_readwrite("b", &my::Rectangle::b)
			.def("intersect", &my::Rectangle::intersect)
			.def("intersectSelf", &my::Rectangle::intersectSelf)
			.def("Union", &my::Rectangle::Union)
			.def("unionSelf", &my::Rectangle::unionSelf)
			.def("offset", (my::Rectangle(my::Rectangle::*)(float, float) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::offsetSelf)
			.def("offset", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::offsetSelf)
			.def("shrink", (my::Rectangle(my::Rectangle::*)(float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle(my::Rectangle::*)(float, float, float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float, float, float))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle(my::Rectangle::*)(const my::Vector4 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector4 &))&my::Rectangle::shrinkSelf)
			.def("LeftTop", (my::Vector2(my::Rectangle::*)(void) const)&my::Rectangle::LeftTop)
			.def("RightBottom", (my::Vector2(my::Rectangle::*)(void) const)&my::Rectangle::RightBottom)
			.def("Center", &my::Rectangle::Center)
			.def("Width", &my::Rectangle::Width)
			.def("Height", &my::Rectangle::Height)
			.def("Extent", &my::Rectangle::Extent)
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

		, class_<my::Matrix4>("Matrix4")
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
			.def("scale", (my::Matrix4(my::Matrix4::*)(float, float, float) const)&my::Matrix4::scale)
			.def("scale", (my::Matrix4(my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::scale)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::scaleSelf)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::scaleSelf)
			.def("rotateX", &my::Matrix4::rotateX)
			.def("rotateY", &my::Matrix4::rotateY)
			.def("rotateZ", &my::Matrix4::rotateZ)
			.def("rotate", &my::Matrix4::rotate)
			.def("translate", (my::Matrix4(my::Matrix4::*)(float, float, float) const)&my::Matrix4::translate)
			.def("translate", (my::Matrix4(my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::translate)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::translateSelf)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::translateSelf)
			.def("lerp", &my::Matrix4::lerp)
			.def("lerpSelf", &my::Matrix4::lerpSelf)
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

		, class_<my::AABB>("AABB")
			.def(constructor<float, float>())
			.def(constructor<float, float, float, float, float, float>())
			.def(constructor<const my::Vector3 &, const my::Vector3 &>())
			.def_readwrite("min", &my::AABB::m_min)
			.def_readwrite("max", &my::AABB::m_max)
			.def("transform", &my::AABB::transform)

		, class_<my::Spline>("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("Interpolate", (float (my::Spline::*)(float, float) const)&my::Spline::Interpolate)

		, class_<my::Emitter>("Emitter")
			.def(constructor<unsigned int>())
			.def("RemoveAllParticle", &my::Emitter::RemoveAllParticle)

		, class_<my::BaseCamera, boost::shared_ptr<my::BaseCamera> >("BaseCamera")
			.def_readonly("View", &my::BaseCamera::m_View)
			.def_readonly("Proj", &my::BaseCamera::m_Proj)
			.def_readonly("ViewProj", &my::BaseCamera::m_ViewProj)
			.def_readonly("InverseViewProj", &my::BaseCamera::m_InverseViewProj)

		, class_<my::Camera, my::BaseCamera, boost::shared_ptr<my::Camera> >("Camera")
			.def_readwrite("Aspect", &my::Camera::m_Aspect)
			.def_readwrite("Eye", &my::Camera::m_Eye)
			.def_readwrite("Eular", &my::Camera::m_Eular)
			.def_readwrite("Nz", &my::Camera::m_Nz)
			.def_readwrite("Fz", &my::Camera::m_Fz)

		, class_<my::OrthoCamera, my::Camera, boost::shared_ptr<my::Camera> >("OrthoCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Diagonal", &my::OrthoCamera::m_Diagonal)

		, class_<my::PerspectiveCamera, my::Camera, boost::shared_ptr<my::Camera> >("PerspectiveCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Fov", &my::PerspectiveCamera::m_Fov)

		, class_<my::ModelViewerCamera, my::Camera, boost::shared_ptr<my::Camera> >("ModelViewerCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("LookAt", &my::ModelViewerCamera::m_LookAt)
			.def_readwrite("Distance", &my::ModelViewerCamera::m_Distance)

		, class_<my::FirstPersonCamera, my::Camera, boost::shared_ptr<my::Camera> >("FirstPersonCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("LocalVel", &my::FirstPersonCamera::m_LocalVel)

		, class_<my::OctEntity>("OctEntity")
	];

	module(m_State)[
		class_<my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("BaseTexture")

		, class_<my::Texture2D, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("Texture2D")

		, class_<my::CubeTexture, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("CubeTexture")

		, class_<my::Mesh, boost::shared_ptr<my::Mesh> >("Mesh")
			.def("GetNumFaces", &my::Mesh::GetNumFaces)
			.def("GetNumVertices", &my::Mesh::GetNumVertices)

		, class_<my::OgreMesh, my::Mesh, boost::shared_ptr<my::OgreMesh> >("OgreMesh")
			.def("SaveOgreMesh", &my::OgreMesh::SaveOgreMesh)
			.def("SaveSimplifiedOgreMesh", &my::OgreMesh::SaveSimplifiedOgreMesh)
			.def("Transform", &my::OgreMesh::Transform)
			.def("GetMaterialNum", &my::OgreMesh::GetMaterialNum)
			.def("GetMaterialName", &my::OgreMesh::GetMaterialName)

		, class_<my::OgreSkeletonAnimation, boost::shared_ptr<my::OgreSkeletonAnimation> >("OgreSkeletonAnimation")
			.def("AddOgreSkeletonAnimationFromFile", &my::OgreSkeletonAnimation::AddOgreSkeletonAnimationFromFile)
			.def("SaveOgreSkeletonAnimation", &my::OgreSkeletonAnimation::SaveOgreSkeletonAnimation)
			.def("Transform", &my::OgreSkeletonAnimation::Transform)

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

		//, def("res2texture", &boost::dynamic_pointer_cast<my::BaseTexture, my::DeviceResourceBase>)
		//, def("res2mesh", &boost::dynamic_pointer_cast<my::OgreMesh, my::DeviceResourceBase>)
		//, def("res2skeleton", &boost::dynamic_pointer_cast<my::OgreSkeletonAnimation, my::DeviceResourceBase>)
		//, def("res2effect", &boost::dynamic_pointer_cast<my::Effect, my::DeviceResourceBase>)
		//, def("res2font", &boost::dynamic_pointer_cast<my::Font, my::DeviceResourceBase>)
	];

	module(m_State)[
		def("ARGB", &ARGB)

		, class_<my::EventArg>("EventArg")

		, class_<my::EventFunction>("EventFunction")

		, class_<my::ControlEventArg, my::EventArg>("ControlEventArg")
			.def_readonly("sender", &my::ControlEventArg::sender)

		, class_<my::MouseEventArg, my::ControlEventArg>("MouseEventArg")
			.def_readonly("pt", &my::MouseEventArg::pt)

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
			.def_readwrite("EventMouseEnter", &my::Control::m_EventMouseEnter)
			.def_readwrite("EventMouseLeave", &my::Control::m_EventMouseLeave)
			.def_readwrite("EventMouseClick", &my::Control::m_EventMouseClick)

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
			.def_readwrite("EventChange", &my::EditBox::m_EventChange)
			.def_readwrite("EventEnter", &my::EditBox::m_EventEnter)

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
			.def_readwrite("EventSelectionChanged", &my::ComboBox::m_EventSelectionChanged)

		, class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(constructor<>())
			.def_readwrite("World", &my::Dialog::m_World)
			.def("Refresh", &my::Dialog::Refresh)
			.def_readwrite("EventAlign", &my::Dialog::m_EventAlign)
			.def_readwrite("EventRefresh", &my::Dialog::m_EventRefresh)
			.scope
			[
				def("LoadFromFile", &my::Dialog::LoadFromFile)
			]
			.def("SaveToFile", &my::Dialog::SaveToFile)

		, class_<Console, my::Dialog, boost::shared_ptr<Console> >("Console")
	];

	module(m_State)[
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

		, class_<my::DxutWindow, boost::shared_ptr<my::DxutWindow> >("DxutWindow")
			.def("PostMessage", &my::DxutWindow::PostMessage)

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

		, class_<my::TimerEventArg, my::EventArg>("TimerEventArg")
			.def_readonly("Interval", &my::TimerEventArg::m_Interval)

		, class_<my::Timer, boost::shared_ptr<my::Timer> >("Timer")
			.def(constructor<float, float>())
			.def_readonly("Interval", &my::Timer::m_Interval)
			.def_readonly("RemainingTime", &my::Timer::m_RemainingTime)
			.def_readwrite("EventTimer", &my::Timer::m_EventTimer)

		, class_<my::InputEventArg, my::EventArg>("InputEventArg")
			.def_readwrite("handled", &my::InputEventArg::handled)

		, class_<my::KeyboardEventArg, my::InputEventArg>("KeyboardEventArg")
			.def_readonly("kc", &my::KeyboardEventArg::kc)

		, class_<my::MouseMoveEventArg, my::InputEventArg>("MouseMoveEventArg")
			.def_readonly("x", &my::MouseMoveEventArg::x)
			.def_readonly("y", &my::MouseMoveEventArg::y)
			.def_readonly("z", &my::MouseMoveEventArg::z)

		, class_<my::MouseBtnEventArg, my::InputEventArg>("MouseBtnEventArg")
			.def_readonly("index", &my::MouseBtnEventArg::index)

		, class_<my::JoystickAxisEventArg, my::InputEventArg>("JoystickAxisEventArg")
			.def_readonly("axis", &my::JoystickAxisEventArg::axis)
			.def_readonly("value", &my::JoystickAxisEventArg::value)

		, class_<my::JoystickPovEventArg, my::InputEventArg>("JoystickPovEventArg")
			.def_readonly("index", &my::JoystickPovEventArg::index)
			.def_readonly("dir", &my::JoystickPovEventArg::dir)

		, class_<my::JoystickBtnEventArg, my::InputEventArg>("JoystickBtnEventArg")
			.def_readonly("index", &my::JoystickBtnEventArg::index)

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
			.def("AddParameterFloat", &Material::AddParameterFloat)
			.def("AddParameterFloat2", &Material::AddParameterFloat2)
			.def("AddParameterFloat3", &Material::AddParameterFloat3)
			.def("AddParameterFloat4", &Material::AddParameterFloat4)
			.def("AddParameterTexture", &Material::AddParameterTexture)
			.def("SetParameterTexture", &Material::SetParameterTexture)
			.scope
			[
				def("LoadFromFile", &Material::LoadFromFile)
			]
			.def("SaveToFile", &Material::SaveToFile)

		, class_<Component, boost::shared_ptr<Component> >("Component")
			.enum_("ComponentType")
			[
				value("ComponentTypeComponent", Component::ComponentTypeComponent),
				value("ComponentTypeActor", Component::ComponentTypeActor),
				value("ComponentTypeCharacter", Component::ComponentTypeCharacter),
				value("ComponentTypeMesh", Component::ComponentTypeMesh),
				value("ComponentTypeCloth", Component::ComponentTypeCloth),
				value("ComponentTypeEmitter", Component::ComponentTypeEmitter),
				value("ComponentTypeStaticEmitter", Component::ComponentTypeStaticEmitter),
				value("ComponentTypeSphericalEmitter", Component::ComponentTypeSphericalEmitter),
				value("ComponentTypeTerrain", Component::ComponentTypeTerrain)
			]
			.def_readonly("Type", &Component::m_Type)
			.def_readonly("Actor", &Component::m_Actor)
			.def("Clone", &Component::Clone)
			.def("RequestResource", &Component::RequestResource)
			.def("ReleaseResource", &Component::ReleaseResource)
			.def("EnterPhysxScene", &Component::EnterPhysxScene)
			.def("LeavePhysxScene", &Component::LeavePhysxScene)
			.def("CalculateAABB", &Component::CalculateAABB)
			.def("CreateBoxShape", &Component::CreateBoxShape)
			.def("CreateCapsuleShape", &Component::CreateCapsuleShape)
			.def("CreatePlaneShape", &Component::CreatePlaneShape)
			.def("CreateSphereShape", &Component::CreateSphereShape)
			.def("ClearShape", &Component::ClearShape)

		, class_<MeshComponent, Component, boost::shared_ptr<Component> >("MeshComponent")
			.def(constructor<>())
			.def_readwrite("MeshPath", &MeshComponent::m_MeshPath)
			.def_readwrite("MeshSubMeshName", &MeshComponent::m_MeshSubMeshName)
			.def_readonly("Mesh", &MeshComponent::m_Mesh)
			.def_readwrite("MeshEventReady", &MeshComponent::m_MeshEventReady)
			.def_readwrite("bInstance", &MeshComponent::m_bInstance)
			.def_readwrite("bUseAnimation", &MeshComponent::m_bUseAnimation)
			.def("CreateTriangleMeshShape", &MeshComponent::CreateTriangleMeshShape)
			.def_readonly("MaterialList", &MeshComponent::m_MaterialList, luabind::return_stl_iterator)
			.def("AddMaterial", &MeshComponent::AddMaterial)

		, class_<ClothComponent, Component, boost::shared_ptr<Component> >("ClothComponent")
			.def(constructor<>())
			.def("CreateClothFromMesh", &ClothComponent::CreateClothFromMesh)
			.def_readonly("MaterialList", &ClothComponent::m_MaterialList, luabind::return_stl_iterator)
			.def("AddMaterial", &ClothComponent::AddMaterial)

		, class_<EmitterComponent, Component, boost::shared_ptr<Component> >("EmitterComponent")
			.def_readwrite("Material", &EmitterComponent::m_Material)
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
			.def("Spawn", &EmitterComponent::Spawn)

		, class_<StaticEmitterComponent, EmitterComponent, boost::shared_ptr<Component> >("StaticEmitterComponent")
			.def(constructor<unsigned int>())

		, class_<SphericalEmitterComponent, EmitterComponent, boost::shared_ptr<Component> >("SphericalEmitterComponent")
			.def(constructor<unsigned int>())
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
			.def_readwrite("SpawnCycle", &SphericalEmitterComponent::m_SpawnCycle)

		, class_<TerrainChunk, my::Emitter>("TerrainChunk")
			.def_readonly("aabb", &TerrainChunk::m_aabb)
			.def_readonly("Row", &TerrainChunk::m_Row)
			.def_readonly("Col", &TerrainChunk::m_Col)

		, class_<Terrain, Component, boost::shared_ptr<Component> >("Terrain")
			.def(constructor<int, int, int, float>())
			.def_readonly("RowChunks", &Terrain::m_RowChunks)
			.def_readonly("ColChunks", &Terrain::m_ColChunks)
			.def_readonly("ChunkSize", &Terrain::m_ChunkSize)
			.def_readwrite("Material", &Terrain::m_Material)
			//.def_readonly("Chunks", &Terrain::m_Chunks, luabind::return_stl_iterator)
			.def("GetChunk", &Terrain::GetChunk)

		, class_<Actor, my::OctEntity, boost::shared_ptr<Actor> >("Actor")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, const my::Vector3 &, const my::AABB &>())
			.def_readwrite("aabb", &Actor::m_aabb)
			.def_readwrite("Position", &Actor::m_Position)
			.def_readwrite("Rotation", &Actor::m_Rotation)
			.def_readwrite("Scale", &Actor::m_Scale)
			.def_readwrite("World", &Actor::m_World)
			.def_readwrite("LodDist", &Actor::m_LodDist)
			.def_readwrite("LodFactor", &Actor::m_LodFactor)
			.def_readwrite("Animation", &Actor::m_Animation)
			.def_readonly("Cmps", &Actor::m_Cmps, luabind::return_stl_iterator)
			.def("Clone", &Actor::Clone)
			.def("RequestResource", &Actor::RequestResource)
			.def("ReleaseResource", &Actor::ReleaseResource)
			.def("EnterPhysxScene", &Actor::EnterPhysxScene)
			.def("LeavePhysxScene", &Actor::LeavePhysxScene)
			.def("SetPxPose", &Actor::SetPxPose)
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
			.def("AddComponent", &Actor::AddComponent)
			.def("RemoveComponent", &Actor::RemoveComponent)
			.def("ClearAllComponent", &Actor::ClearAllComponent)
			.scope
			[
				def("LoadFromFile", &Actor::LoadFromFile)
			]
			.def("SaveToFile", &Actor::SaveToFile)
			.def("Attach", &Actor::Attach)
			.def("Dettach", &Actor::Dettach)
			.def("PlayAction", &Actor::PlayAction)

		, def("actor2ent", (boost::shared_ptr<my::OctEntity>(*)(const boost::shared_ptr<Actor> &))&boost::static_pointer_cast<my::OctEntity, Actor>)

		, class_<Character, Actor, boost::shared_ptr<Actor> >("Character")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, const my::Vector3 &, const my::AABB &, float, float, float>())
			.def_readwrite("Velocity", &Character::m_Velocity)
			.def("SetPxPose", &Character::SetPxPose)

		, class_<AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNode")
			.property("Child0", &AnimationNode::GetChild<0>, &AnimationNode::SetChild<0>)
			.property("Child1", &AnimationNode::GetChild<1>, &AnimationNode::SetChild<1>)

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
			.def_readwrite("ActiveChild", &AnimationNodeBlend::m_ActiveChild)

		, class_<AnimationNodeBlendBySpeed, AnimationNodeBlend, boost::shared_ptr<AnimationNode> >("AnimationNodeBlendBySpeed")
			.def(constructor<>())
			.def_readwrite("Speed0", &AnimationNodeBlendBySpeed::m_Speed0)
			.def_readwrite("BlendInTime", &AnimationNodeBlendBySpeed::m_BlendInTime)

		, class_<AnimationNodeRateBySpeed, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeRateBySpeed")
			.def(constructor<>())
			.def_readwrite("Speed0", &AnimationNodeRateBySpeed::m_Speed0)

		, class_<AnimationRoot, AnimationNodeSlot, boost::shared_ptr<AnimationRoot> >("AnimationRoot")
			.def(constructor<Actor *>())
			.def_readwrite("SkeletonPath", &AnimationRoot::m_SkeletonPath)
			.def_readonly("Skeleton", &AnimationRoot::m_Skeleton)
			.def_readwrite("SkeletonEventReady", &AnimationRoot::m_SkeletonEventReady)
			.def("ReloadSequenceGroup", &AnimationRoot::ReloadSequenceGroup)
			.def("AddJiggleBone", (void (AnimationRoot::*)(const std::string &, float, float, float))&AnimationRoot::AddJiggleBone)
			.def("AddIK", &AnimationRoot::AddIK)

		, class_<Action, boost::intrusive_ptr<Action> >("Action")
			.def(constructor<>())
			.def_readwrite("Length", &Action::m_Length)
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
			.def_readwrite("ParticlePosX", &ActionTrackEmitter::m_ParticlePosX)
			.def_readwrite("ParticlePosY", &ActionTrackEmitter::m_ParticlePosY)
			.def_readwrite("ParticlePosZ", &ActionTrackEmitter::m_ParticlePosZ)
			.def_readwrite("ParticleColorR", &ActionTrackEmitter::m_ParticleColorR)
			.def_readwrite("ParticleColorG", &ActionTrackEmitter::m_ParticleColorG)
			.def_readwrite("ParticleColorB", &ActionTrackEmitter::m_ParticleColorB)
			.def_readwrite("ParticleColorA", &ActionTrackEmitter::m_ParticleColorA)
			.def_readwrite("ParticleSizeX", &ActionTrackEmitter::m_ParticleSizeX)
			.def_readwrite("ParticleSizeY", &ActionTrackEmitter::m_ParticleSizeY)
			.def_readwrite("ParticleAngle", &ActionTrackEmitter::m_ParticleAngle)
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
