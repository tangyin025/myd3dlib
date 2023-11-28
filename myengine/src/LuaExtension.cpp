#include "LuaExtension.h"
extern "C" {
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
#include <luabind/tag_function.hpp>
#include <luabind/discard_result_policy.hpp>
#include "libc.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "myUi.h"
#include "myInput.h"
#include "mySound.h"
#include "myAStar.h"
#include "myEffect.h"
#include "PrintCallStack.h"
#include "LuaExtension.inl"
#include "Material.h"
#include "RenderPipeline.h"
#include "PhysxContext.h"
#include "Animator.h"
#include "Actor.h"
#include "Terrain.h"
#include "StaticMesh.h"
#include "StaticEmitter.h"
#include "Controller.h"
#include "NavigationSerialization.h"
#include "LargeImage.h"
#include "SoundContext.h"
#include "ActionTrack.h"
#include "Steering.h"
#include "SceneContext.h"
#include "rapidxml_print.hpp"
//#include "FastNoiseLite.h"
//#include "clipper.hpp"
#include <boost/scope_exit.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/shared_container_iterator.hpp>
#include <boost/regex.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

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

static void basetexture_generate_mip_sub_levels(my::BaseTexture* self)
{
	// ! https://gamedev.net/forums/topic/633842-idirect3dbasetexture9generatemipsublevels-not-working/4996937/
	HRESULT hr;
	V(D3DXFilterTexture(self->m_ptr, NULL, 0, D3DX_DEFAULT));
}

static void basetexture_save_texture_to_file(my::BaseTexture* self, const char* u8_path, D3DXIMAGE_FILEFORMAT DestFormat)
{
	HRESULT hr;
	V(D3DXSaveTextureToFile(u8tots(u8_path).c_str(), DestFormat, self->m_ptr, NULL));
}

static void texture2d_create_texture_from_file(my::Texture2D* self, const char* u8_path, int Width, int Height, int MipLevels, D3DFORMAT Format)
{
//#define D3DX_DEFAULT            ((UINT) -1)
//#define D3DX_DEFAULT_NONPOW2    ((UINT) -2)
//#define D3DX_DEFAULT_FLOAT      FLT_MAX
//#define D3DX_FROM_FILE          ((UINT) -3)
//#define D3DFMT_FROM_FILE        ((D3DFORMAT) -3)
	self->CreateTextureFromFile(u8tots(u8_path).c_str(), Width, Height, MipLevels, 0, Format, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL);
}

static void texture2d_set_as_render_target(my::Texture2D* self, int target_id)
{
	HRESULT hr;
	CComPtr<IDirect3DSurface9> surf = self->GetSurfaceLevel(0);
	V(my::D3DContext::getSingleton().m_d3dDevice->SetRenderTarget(target_id, surf));
}

static void texture2d_fill_color(my::Texture2D* self, const RECT* pRect, D3DCOLOR color)
{
	HRESULT hr;
	CComPtr<IDirect3DSurface9> surf = self->GetSurfaceLevel(0);
	V(my::D3DContext::getSingleton().m_d3dDevice->ColorFill(surf, pRect, color));
}

static void cubetexture_load_cube_map_surface_from_file(my::CubeTexture* self, D3DCUBEMAP_FACES FaceType, const char* u8_path)
{
	HRESULT hr;
	CComPtr<IDirect3DSurface9> surf = self->GetCubeMapSurface(FaceType, 0);
	V(D3DXLoadSurfaceFromFileW(surf, NULL, NULL, u8tots(u8_path).c_str(), NULL, D3DX_DEFAULT, 0, NULL));
}

static void ogremesh_create_mesh_from_ogre_xml_in_file(my::OgreMesh* self, const char* u8_path)
{
	self->CreateMeshFromOgreXmlInFile(u8tots(u8_path).c_str(), true, D3DXMESH_MANAGED);
}

static void ogremesh_create_mesh_from_obj_in_file(my::OgreMesh* self, const char* u8_path)
{
	self->CreateMeshFromObjInFile(u8tots(u8_path).c_str(), true, D3DXMESH_MANAGED);
}

static int ogremesh_get_material_num(const my::OgreMesh* self)
{
	return (int)self->m_MaterialNameList.size();
}

static const std::string& ogremesh_get_material_name(const my::OgreMesh* self, int i)
{
	return self->m_MaterialNameList[i];
}

static int ogremesh_get_lod_num(const my::OgreMesh* self)
{
	return (int)self->m_LodNameList.size();
}

static const std::string& ogremesh_get_lod_name(const my::OgreMesh* self, int i)
{
	return self->m_LodNameList[i];
}

static unsigned int ogreskeletonanimation_get_bone_num(my::OgreSkeletonAnimation* self)
{
	return (unsigned int)self->m_boneBindPose.size();
}

static const my::Bone & ogreskeletonanimation_get_bind_pose_bone(const my::OgreSkeletonAnimation* self, int i)
{
	return self->m_boneBindPose[i];
}

static DWORD ARGB(int a, int r, int g, int b)
{
	return D3DCOLOR_ARGB(a,r,g,b);
}

static bool Counter(const unsigned int c)
{
	static unsigned int g_c = 0;
	return 0 == (g_c = (g_c + 1) % c);
}

static std::string PrintCallStack(void)
{
	std::ostringstream osstr;
	PrintCallStack(osstr);
	return osstr.str();
}

typedef std::vector<my::NamedObject*> obj_list;

typedef boost::shared_container_iterator<obj_list> shared_obj_list_iter;

static boost::iterator_range<shared_obj_list_iter> d3dcontext_get_named_object_list(my::D3DContext* self)
{
	boost::shared_ptr<obj_list> objs(new obj_list());
	boost::range::transform(self->m_NamedObjects, std::back_inserter(*objs), boost::bind(&my::D3DContext::NamedObjectMap::value_type::second, boost::placeholders::_1));
	return boost::make_iterator_range(shared_obj_list_iter(objs->begin(), objs), shared_obj_list_iter(objs->end(), objs));
}

static void cloth_component_set_stretch(ClothComponent* self, physx::PxClothFabricPhaseType::Enum type, float stiffness, float stiffnessMultiplier, float compressionLimit, float stretchLimit)
{
	self->m_Cloth->setStretchConfig(type, physx::PxClothStretchConfig(stiffness, stiffnessMultiplier, compressionLimit, stretchLimit));
}

static void cloth_component_get_stretch(const ClothComponent* self, physx::PxClothFabricPhaseType::Enum type, float & stiffness, float & stiffnessMultiplier, float & compressionLimit, float & stretchLimit)
{
	physx::PxClothStretchConfig stretchConfig = self->m_Cloth->getStretchConfig(type);
	stiffness = stretchConfig.stiffness;
	stiffnessMultiplier = stretchConfig.stiffnessMultiplier;
	compressionLimit = stretchConfig.compressionLimit;
	stretchLimit = stretchConfig.stretchLimit;
}

static void cloth_component_set_tether(ClothComponent* self, float stiffness, float stretchLimit)
{
	self->m_Cloth->setTetherConfig(physx::PxClothTetherConfig(stiffness, stretchLimit));
}

static void cloth_component_get_tether(const ClothComponent* self, float& stiffness, float& stretchLimit)
{
	physx::PxClothTetherConfig tetherConfig = self->m_Cloth->getTetherConfig();
	stiffness = tetherConfig.stiffness;
	stretchLimit = tetherConfig.stretchLimit;
}

static void cloth_component_add_collision_sphere(ClothComponent* self, const my::Vector3& pos, float radius, int boneid)
{
	physx::PxClothCollisionSphere sphere((physx::PxVec3&)pos, radius);
	self->m_ClothSphereBones.push_back(std::make_pair(sphere, boneid));
	self->m_Cloth->addCollisionSphere(sphere);
	_ASSERT(self->m_ClothSphereBones.size() == self->m_Cloth->getNbCollisionSpheres());
}

static void cloth_component_add_collision_capsule(ClothComponent* self, int first, int second)
{
	self->m_Cloth->addCollisionCapsule(first, second);
}

class StaticEmitterParticleIterator : public std::iterator<std::forward_iterator_tag, my::Emitter::Particle>
{
protected:
	boost::shared_ptr<StaticEmitterStream> emit_str;
	StaticEmitter::ChunkMap::iterator chunk_iter;
	StaticEmitterChunkBuffer::iterator particle_iter;

public:
	explicit StaticEmitterParticleIterator(
		boost::shared_ptr<StaticEmitterStream> _emit_str,
		StaticEmitter::ChunkMap::iterator _chunk_iter)
		: emit_str(_emit_str)
		, chunk_iter(_chunk_iter)
	{
		if (chunk_iter != emit_str->m_emit->m_Chunks.end())
		{
			particle_iter = emit_str->GetBuffer(chunk_iter->first.first, chunk_iter->first.second)->begin();
		}
	}
	// Assignment operator
	StaticEmitterParticleIterator& operator=(const StaticEmitterParticleIterator& src)
	{
		emit_str = src.emit_str;
		chunk_iter = src.chunk_iter;
		particle_iter = src.particle_iter;
	}
	// Dereference an iterator
	my::Emitter::Particle& operator*()
	{
		// When the value is one step more than the last, it's an end iterator
		if (chunk_iter == emit_str->m_emit->m_Chunks.end() || particle_iter == emit_str->GetBuffer(chunk_iter->first.first, chunk_iter->first.second)->end())
		{
			throw std::logic_error("Cannot dereference an end iterator.");
		}
		return *particle_iter;
	}
	// Prefix increment operator
	StaticEmitterParticleIterator& operator++()
	{
		// When the value is one step more than the last, it's an end iterator
		if (chunk_iter == emit_str->m_emit->m_Chunks.end() || particle_iter == emit_str->GetBuffer(chunk_iter->first.first, chunk_iter->first.second)->end())
		{
			throw std::logic_error("Cannot dereference an end iterator.");
		}
		particle_iter++;
		if (particle_iter == emit_str->GetBuffer(chunk_iter->first.first, chunk_iter->first.second)->end())
		{
			chunk_iter++;
			if (chunk_iter != emit_str->m_emit->m_Chunks.end())
			{
				particle_iter = emit_str->GetBuffer(chunk_iter->first.first, chunk_iter->first.second)->begin();
			}
		}
		return *this;
	}
	// Postfix increment operator
	StaticEmitterParticleIterator operator++(int)
	{
		StaticEmitterParticleIterator temp = *this;
		temp++;                                      // Increment the value by the range step
		return temp;                                 // The iterator before it's incremented
	}
	// Comparisons
	bool operator==(const StaticEmitterParticleIterator& iter) const
	{
		return chunk_iter == iter.chunk_iter && (chunk_iter == emit_str->m_emit->m_Chunks.end() || particle_iter == iter.particle_iter);
	}
	bool operator!=(const StaticEmitterParticleIterator& iter) const
	{
		return chunk_iter != iter.chunk_iter || (chunk_iter != emit_str->m_emit->m_Chunks.end() && particle_iter != iter.particle_iter);
	}
};

static boost::iterator_range<StaticEmitterParticleIterator> static_emitter_get_particle_list(StaticEmitter* self)
{
	boost::shared_ptr<StaticEmitterStream> emit_str(new StaticEmitterStream(self));
	return boost::make_iterator_range(StaticEmitterParticleIterator(emit_str, self->m_Chunks.begin()), StaticEmitterParticleIterator(emit_str, self->m_Chunks.end()));
}

static my::Bone animator_get_bone(Animator* self, int i)
{
	return self->anim_pose_hier[i];
}

struct ScriptControl;

static void control_insert_control_adopt(my::Control* self, ScriptControl* ctrl)
{
	self->InsertControl(my::ControlPtr(ctrl));
}

static void control_insert_control_adopt(my::Control* self, unsigned int i, ScriptControl* ctrl)
{
	self->InsertControl(i, my::ControlPtr(ctrl));
}

struct ScriptComponent;

static void actor_insert_component_adopt(Actor* self, ScriptComponent* cmp)
{
	self->InsertComponent(ComponentPtr(cmp));
}

static void actor_insert_component_adopt(Actor* self, unsigned int i, ScriptComponent* cmp)
{
	self->InsertComponent(i, ComponentPtr(cmp));
}

static Actor* actor_get_attacher(const Actor* self, unsigned int i)
{
	return i < self->m_Attaches.size() ? self->m_Attaches[i] : NULL;
}

struct ScriptAnimationNodeBlendList;

template <unsigned int i>
static AnimationNodePtr animation_node_get_child(const AnimationNode* self)
{
	return self->GetChild(i);
}

template <unsigned int i>
static void animation_node_set_child(AnimationNode* self, AnimationNodePtr node)
{
	return self->SetChild(i, node);
}

static void animation_node_set_child_adopt(AnimationNode* self, int i, ScriptAnimationNodeBlendList* node)
{
	self->SetChild(i, AnimationNodePtr(node));
}

static my::Vector3 steering_get_corner(const Steering* self, int i)
{
	if (i < self->m_ncorners)
	{
		const float* verts = &self->m_cornerVerts[i * 3];
		return my::Vector3(verts[0], verts[1], verts[2]);
	}
	return my::Vector3(0, 0, 0);
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

	virtual bool CanHaveFocus(void) const
	{
		return true;
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
		return false;
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
		return false;
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
		return false;
	}

	static bool default_HandleMouse(my::Control* ptr, UINT uMsg, const my::Vector2& pt, WPARAM wParam, LPARAM lParam)
	{
		return ptr->Control::HandleMouse(uMsg, pt, wParam, lParam);
	}
};

struct ScriptComponent : Component, luabind::wrap_base
{
	ScriptComponent(const char* Name)
		: Component(Name)
	{
		// ! make sure the ownership of lua part when using shared_ptr pass to Actor::InsertComponent
	}

	virtual ~ScriptComponent(void)
	{
		_ASSERT(!IsRequested());

		_ASSERT(my::DialogMgr::getSingleton().m_UIPassObjs.end() == std::find(my::DialogMgr::getSingleton().m_UIPassObjs.begin(), my::DialogMgr::getSingleton().m_UIPassObjs.end(),
			boost::bind(&Component::OnGUI, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)));
	}

	enum { TypeID = ComponentTypeScript };

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
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

		ptr->m_Actor->m_EventOnTrigger.connect(boost::bind(&Component::OnTrigger, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventOnContact.connect(boost::bind(&Component::OnContact, ptr, boost::placeholders::_1));

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

		ptr->m_Actor->m_EventOnTrigger.disconnect(boost::bind(&Component::OnTrigger, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventOnContact.disconnect(boost::bind(&Component::OnContact, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadShapeHit.disconnect(boost::bind(&Component::OnPxThreadShapeHit, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadControllerHit.disconnect(boost::bind(&Component::OnPxThreadControllerHit, ptr, boost::placeholders::_1));

		ptr->m_Actor->m_EventPxThreadObstacleHit.disconnect(boost::bind(&Component::OnPxThreadObstacleHit, ptr, boost::placeholders::_1));
	}

	virtual void OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam)
	{
		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		try
		{
			luabind::wrap_base::call<void>("OnSetShader", my::D3DContext::getSingletonPtr(), shader, lparam);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnSetShader(Component* ptr, my::D3DContext* context, my::Effect* shader, LPARAM lparam)
	{
		//ptr->Component::OnSetShader(context->m_d3dDevice, shader, lparam);
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
		//ptr->Component::Update(fElapsedTime);
	}

	virtual void OnPxThreadSubstep(float dtime)
	{
		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

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
		//ptr->Component::OnPxThreadSubstep(dtime);

		my::Vector3 disp;
		ptr->m_Actor->TickActionAndGetDisplacement(dtime, disp);
	}

	virtual void OnTrigger(my::EventArg* arg)
	{
		try
		{
			luabind::wrap_base::call<void>("OnTrigger", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnTrigger(Component* ptr, my::EventArg* arg)
	{
		//ptr->Component::OnTrigger(arg);
	}

	virtual void OnContact(my::EventArg* arg)
	{
		try
		{
			luabind::wrap_base::call<void>("OnContact", arg);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnContact(Component* ptr, my::EventArg* arg)
	{
		//ptr->Component::OnContact(arg);
	}

	virtual void OnPxThreadShapeHit(my::EventArg* arg)
	{
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
		//ptr->Component::OnPxThreadShapeHit(arg);
	}

	virtual void OnPxThreadControllerHit(my::EventArg* arg)
	{
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
		//ptr->Component::OnPxThreadControllerHit(arg);
	}

	virtual void OnPxThreadObstacleHit(my::EventArg* arg)
	{
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
		//ptr->Component::OnPxThreadObstacleHit(arg);
	}

	virtual void OnGUI(my::UIRender* ui_render, float fElapsedTime, const my::Vector2 & Viewport)
	{
		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		try
		{
			luabind::wrap_base::call<void>("OnGUI", ui_render, fElapsedTime, Viewport);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnGUI(Component* ptr, my::UIRender* ui_render, float fElapsedTime, const my::Vector2 & Viewport)
	{
		//ptr->Component::OnGUI(ui_render, fElapsedTime, Viewport);
	}

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
	{
		if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
		{
			my::DialogMgr::getSingleton().m_UIPassObjs.push_back(boost::bind(&Component::OnGUI, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
		}

		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		try
		{
			luabind::wrap_base::call<void>("AddToPipeline", frustum, pipeline, PassMask, ViewPos, TargetPos);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_AddToPipeline(Component* ptr, const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
	{
		//ptr->Component::AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
	}
};

struct ScriptAnimationNodeBlendList : AnimationNodeBlendList, luabind::wrap_base
{
	ScriptAnimationNodeBlendList(const char* Name, unsigned int ChildNum)
		: AnimationNodeBlendList(Name, ChildNum)
	{
	}

	virtual ~ScriptAnimationNodeBlendList(void)
	{
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight)
	{
		try
		{
			luabind::wrap_base::call<void>("Tick", fElapsedTime, fTotalWeight);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_Tick(AnimationNodeBlendList* ptr, float fElapsedTime, float fTotalWeight)
	{
		ptr->AnimationNodeBlendList::Tick(fElapsedTime, fTotalWeight);
	}
};

class ScriptActionTrack : public ActionTrack
{
public:
	luabind::object m_Creator;

	ScriptActionTrack(const luabind::object& Creator)
		: m_Creator(Creator)
	{
	}

	virtual ~ScriptActionTrack(void)
	{
	}

	virtual ActionTrackInstPtr CreateInstance(Actor* _Actor, float Rate) const
	{
		return ActionTrackInstPtr(luabind::call_function<ActionTrackInst*>(m_Creator, _Actor)[luabind::adopt(luabind::result)]);
	}
};

struct ScriptActionTrackInst : ActionTrackInst, luabind::wrap_base
{
public:
	ScriptActionTrackInst(Actor* actor)
		: ActionTrackInst(actor)
	{
	}

	virtual ~ScriptActionTrackInst(void)
	{
	}

	virtual void UpdateTime(float LastTime, float Time)
	{
		try
		{
			luabind::wrap_base::call<void>("UpdateTime", LastTime, Time);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_UpdateTime(ScriptActionTrackInst* ptr, float LastTime, float Time)
	{
		//ptr->ActionTrackInst::UpdateTime(LastTime, Time);
	}

	virtual void Stop(void)
	{
		try
		{
			luabind::wrap_base::call<void>("Stop");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_Stop(ScriptActionTrackInst* ptr)
	{
		//ptr->ActionTrackInst::Stop();
	}

	virtual bool GetDisplacement(float LastTime, float dtime, my::Vector3 & disp)
	{
		try
		{
			return luabind::wrap_base::call<bool>("GetDisplacement", LastTime, dtime, disp);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
		return false;
	}

	static bool default_GetDisplacement(ScriptActionTrackInst* ptr, float LastTime, float dtime, my::Vector3 & disp)
	{
		return ptr->ActionTrackInst::GetDisplacement(LastTime, dtime, disp);
	}
};

typedef std::vector<Component*> cmp_list;

typedef boost::shared_container_iterator<cmp_list> shared_cmp_list_iter;

extern boost::iterator_range<shared_cmp_list_iter> controller_get_geom_stream(const Controller* self);

static my::Effect* renderpipeline_query_shader(RenderPipeline* self, const luabind::object& macro, const char* path, unsigned int PassID)
{
	std::vector<D3DXMACRO> macs;
	std::vector<std::string> strs;
	luabind::iterator iter(macro), end;
	for (; iter != end; iter++)
	{
		strs.push_back(boost::lexical_cast<std::string>(iter.key()));
		strs.push_back(boost::lexical_cast<std::string>(*iter));
		D3DXMACRO m = { (strs.rbegin() + 1)->c_str(), strs.rbegin()->c_str() };
		macs.push_back(m);
	}
	D3DXMACRO m = { 0 };
	macs.push_back(m);
	return self->QueryShader(macs.data(), path, PassID);
}

static bool physxscene_raycast(PhysxScene* self,
	const my::Vector3& origin, const my::Vector3& unitDir, float distance, unsigned int filterWord0, const luabind::object& callback, unsigned int MaxNbTouches)
{
	return self->Raycast(origin, unitDir, distance, filterWord0, boost::bind(&luabind::call_function<bool, my::EventArg*>, boost::ref(callback), boost::placeholders::_1), MaxNbTouches);
}

static bool physxscene_overlap_box(PhysxScene* self,
	float hx, float hy, float hz, const my::Vector3& Position, const my::Quaternion& Rotation, unsigned int filterWord0, const luabind::object& callback, unsigned int MaxNbTouches)
{
	physx::PxBoxGeometry box(hx, hy, hz);
	return self->Overlap(box, Position, Rotation, filterWord0, boost::bind(&luabind::call_function<bool, my::EventArg*>, boost::ref(callback), boost::placeholders::_1), MaxNbTouches);
}

static bool physxscene_overlap_sphere(PhysxScene* self,
	float radius, const my::Vector3& Position, unsigned int filterWord0, const luabind::object& callback, unsigned int MaxNbTouches)
{
	physx::PxSphereGeometry sphere(radius);
	return self->Overlap(sphere, Position, my::Quaternion::Identity(), filterWord0, boost::bind(&luabind::call_function<bool, my::EventArg*>, boost::ref(callback), boost::placeholders::_1), MaxNbTouches);
}

static bool physxscene_sweep_box(PhysxScene* self,
	const my::Vector3& HalfBox, const my::Vector3& Position, const my::Quaternion& Rotation, const my::Vector3& unitDir, float distance, unsigned int filterWord0, const luabind::object& callback, unsigned int MaxNbTouches)
{
	physx::PxBoxGeometry box(HalfBox.x, HalfBox.y, HalfBox.z);
	return self->Sweep(box, Position, Rotation, unitDir, distance, filterWord0, boost::bind(&luabind::call_function<bool, my::EventArg*>, boost::ref(callback), boost::placeholders::_1), MaxNbTouches);
}

static bool physxscene_sweep_sphere(PhysxScene* self,
	float radius, const my::Vector3& Position, const my::Vector3& unitDir, float distance, unsigned int filterWord0, const luabind::object& callback, unsigned int MaxNbTouches)
{
	physx::PxSphereGeometry sphere(radius);
	return self->Sweep(sphere, Position, my::Quaternion::Identity(), unitDir, distance, filterWord0, boost::bind(&luabind::call_function<bool, my::EventArg*>, boost::ref(callback), boost::placeholders::_1), MaxNbTouches);
}

static void indexedbitmap_save_indexed_bitmap(my::IndexedBitmap * self, const char * path, const luabind::object & get_color)
{
	self->SaveIndexedBitmap(path, boost::bind(&luabind::call_function<DWORD, unsigned char>, boost::ref(get_color), boost::placeholders::_1));
}

static bool regex_search(boost::regex* self, const char* s, boost::cmatch & match)
{
	return boost::regex_search(s, match, *self, boost::match_default);
}

typedef std::vector<boost::cmatch> sub_match_list;

typedef boost::shared_container_iterator<sub_match_list> shared_sub_match_list_iter;

static boost::iterator_range<shared_sub_match_list_iter> regex_search_all(boost::regex* self, const char* s)
{
	boost::cmatch match;
	boost::shared_ptr<sub_match_list> matchs(new sub_match_list());
	for (; boost::regex_search(s, match, *self, boost::match_default); s = match[0].second)
	{
		matchs->push_back(match);
	}
	return boost::make_iterator_range(shared_sub_match_list_iter(matchs->begin(), matchs), shared_sub_match_list_iter(matchs->end(), matchs));
}

static bool cmatch_is_matched(boost::cmatch* self, int i)
{
	return self->operator[](i).matched;
}

static std::string cmatch_sub_match(boost::cmatch* self, int i)
{
	return self->operator[](i);
}

static void xml_node_print_to_file(rapidxml::xml_node<char>* self, const char* u8_path)
{
	std::ofstream ofs(u8tows(u8_path).c_str(), std::ios::out, _SH_DENYRW);
	if (ofs.is_open())
	{
		rapidxml::print<char>(ofs, *self, 0);
	}
}

static void xml_document_parse(rapidxml::xml_document<char>* self, const char* u8_path, my::CachePtr& cache)
{
	_ASSERT(!cache);
	cache = my::FileIStream::Open(u8tows(u8_path).c_str())->GetWholeCache();
	cache->push_back(0);
	self->parse<0>((char*)&(*cache)[0]);
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
		def("srand", &srand)

		, def("Lerp", &my::Lerp<float>)

		, def("Clamp", &my::Clamp<float>)

		, def("Wrap", &my::Wrap<float>)

		, def("Terrace", &my::Terrace<float>)

		, class_<my::Vector2>("Vector2")
			.def(constructor<float>())
			.def(constructor<float, float>())
			.def(constructor<const my::Vector2&>())
			.def_readwrite("x", &my::Vector2::x)
			.def_readwrite("y", &my::Vector2::y)
			.def(const_self == other<const my::Vector2 &>())
			.def(-const_self)
			.def(const_self + other<const my::Vector2 &>())
			.def(const_self + float())
			.def(const_self - other<const my::Vector2 &>())
			.def(const_self - float())
			.def(const_self * other<const my::Vector2 &>())
			.def(const_self * float())
			.def(const_self / other<const my::Vector2 &>())
			.def(const_self / float())
			.def("kross", &my::Vector2::kross)
			.def("dot", &my::Vector2::dot)
			.property("magnitude", &my::Vector2::magnitude)
			.property("magnitudeSq", &my::Vector2::magnitudeSq)
			.def("distance", &my::Vector2::distance)
			.def("distanceSq", &my::Vector2::distanceSq)
			.def("lerp", &my::Vector2::lerp)
			.def("lerpSelf", &my::Vector2::lerpSelf, return_reference_to(boost::placeholders::_1))
			.property("normalize", &my::Vector2::normalize)
			.def("normalizeSelf", &my::Vector2::normalizeSelf, return_reference_to(boost::placeholders::_1))
			.def("transform", &my::Vector2::transform)
			.def("transformTranspose", &my::Vector2::transformTranspose)
			.def("transformCoord", &my::Vector2::transformCoord)
			.def("transformCoordTranspose", &my::Vector2::transformCoordTranspose)
			.def("transformNormal", &my::Vector2::transformNormal)
			.def("transformNormalTranspose", &my::Vector2::transformNormalTranspose)
			.def("transform", &my::Vector2::transform)
			.property("Polar", &my::Vector2::cartesianToPolar)
			.scope
			[
				def("PolarToCartesian", &my::Vector2::PolarToCartesian),
				def("RandomUnit", &my::Vector2::RandomUnit),
				def("RandomUnitCircle", &my::Vector2::RandomUnitCircle)
			]

		, class_<my::Vector3>("Vector3")
			.def(constructor<float>())
			.def(constructor<const my::Vector2&, float>())
			.def(constructor<float, float, float>())
			.def(constructor<const my::Vector3&>())
			.def_readwrite("x", &my::Vector3::x)
			.def_readwrite("y", &my::Vector3::y)
			.def_readwrite("z", &my::Vector3::z)
			.def_readwrite("xy", &my::Vector3::xy)
			.def_readwrite("yz", &my::Vector3::yz)
			.def(const_self == other<const my::Vector3 &>())
			.def(-const_self)
			.def(const_self + other<const my::Vector3 &>())
			.def(const_self + float())
			.def(const_self - other<const my::Vector3 &>())
			.def(const_self - float())
			.def(const_self * other<const my::Vector3 &>())
			.def(const_self * float())
			.def(const_self / other<const my::Vector3 &>())
			.def(const_self / float())
			.def("cross", &my::Vector3::cross)
			.def("perpendicular", &my::Vector3::perpendicular)
			.def("dot", &my::Vector3::dot)
			.def("dot2D", &my::Vector3::dot2D)
			.property("magnitude", &my::Vector3::magnitude)
			.property("magnitudeSq", &my::Vector3::magnitudeSq)
			.property("magnitude2D", &my::Vector3::magnitude2D)
			.property("magnitudeSq2D", &my::Vector3::magnitudeSq2D)
			.def("distance", &my::Vector3::distance)
			.def("distanceSq", &my::Vector3::distanceSq)
			.def("distance2D", &my::Vector3::distance2D)
			.def("distanceSq2D", &my::Vector3::distanceSq2D)
			.def("lerp", &my::Vector3::lerp)
			.def("lerpSelf", &my::Vector3::lerpSelf, return_reference_to(boost::placeholders::_1))
			.property("normalize", luabind::tag_function<my::Vector3(const my::Vector3*)>(
				boost::bind((my::Vector3(my::Vector3::*)(const my::Vector3&)const) & my::Vector3::normalize, boost::placeholders::_1, my::Vector3(1, 0, 0))))
			.property("normalize2D", luabind::tag_function<my::Vector3(const my::Vector3*)>(
				boost::bind((my::Vector3(my::Vector3::*)(const my::Vector3&)const) & my::Vector3::normalize2D, boost::placeholders::_1, my::Vector3(1, 0, 0))))
			.def("normalizeSelf", luabind::tag_function<my::Vector3& (my::Vector3*)>(
				boost::bind((my::Vector3 & (my::Vector3::*)(const my::Vector3&)) & my::Vector3::normalizeSelf, boost::placeholders::_1, my::Vector3(1, 0, 0))))
			.def("transform", (my::Vector4(my::Vector3::*)(const my::Matrix4 &) const)&my::Vector3::transform)
			.def("transformTranspose", &my::Vector3::transformTranspose)
			.def("transformCoord", &my::Vector3::transformCoordSafe)
			.def("transformCoordTranspose", &my::Vector3::transformCoordTranspose)
			.def("transformNormal", &my::Vector3::transformNormal)
			.def("transformNormalTranspose", &my::Vector3::transformNormalTranspose)
			.def("transform", (my::Vector3(my::Vector3::*)(const my::Quaternion &) const)&my::Vector3::transform)
			.property("Polar", &my::Vector3::cartesianToPolar)
			.def("cosTheta", &my::Vector3::cosTheta)
			.def("angle", &my::Vector3::angle)
			.def("signedAngle", &my::Vector3::signedAngle)
			.property("xz", &my::Vector3::xz)
			.scope
			[
				def("PolarToCartesian", &my::Vector3::PolarToCartesian),
				def("Cosine", &my::Vector3::Cosine),
				def("RandomUnit", &my::Vector3::RandomUnit),
				def("RandomUnitSphere", &my::Vector3::RandomUnitSphere)
			]

		, class_<my::Vector4>("Vector4")
			.def(constructor<float>())
			.def(constructor<const my::Vector3&, float>())
			.def(constructor<float, float, float, float>())
			.def(constructor<const my::Vector4&>())
			.def_readwrite("x", &my::Vector4::x)
			.def_readwrite("y", &my::Vector4::y)
			.def_readwrite("z", &my::Vector4::z)
			.def_readwrite("w", &my::Vector4::w)
			.def_readwrite("xy", &my::Vector4::xy)
			.def_readwrite("yz", &my::Vector4::yz)
			.def_readwrite("xyz", &my::Vector4::xyz)
			.def_readwrite("r", &my::Vector4::x)
			.def_readwrite("g", &my::Vector4::y)
			.def_readwrite("b", &my::Vector4::z)
			.def_readwrite("a", &my::Vector4::w)
			.def(const_self == other<const my::Vector4 &>())
			.def(-const_self)
			.def(const_self + other<const my::Vector4 &>())
			.def(const_self + float())
			.def(const_self - other<const my::Vector4 &>())
			.def(const_self - float())
			.def(const_self * other<const my::Vector4 &>())
			.def(const_self * float())
			.def(const_self / other<const my::Vector4 &>())
			.def(const_self / float())
			.def("cross", &my::Vector4::cross)
			.def("dot", &my::Vector4::dot)
			.property("magnitude", &my::Vector4::magnitude)
			.property("magnitudeSq", &my::Vector4::magnitudeSq)
			.def("lerp", &my::Vector4::lerp)
			.def("lerpSelf", &my::Vector4::lerpSelf, return_reference_to(boost::placeholders::_1))
			.property("normalize", &my::Vector4::normalize)
			.def("normalizeSelf", &my::Vector4::normalizeSelf, return_reference_to(boost::placeholders::_1))
			.def("transform", &my::Vector4::transform)
			.def("transformTranspose", &my::Vector4::transformTranspose)
			.property("xz", &my::Vector4::xz)
			.scope
			[
				def("FromArgb", &my::Vector4::FromArgb)
			]
			.property("argb", &my::Vector4::toArgb)

		, class_<my::Rectangle>("Rectangle")
			.def(constructor<float, float, float, float>())
			.def(constructor<const my::Vector2&, const my::Vector2&>())
			.def(constructor<const my::Vector2&>())
			.def_readwrite("l", &my::Rectangle::l)
			.def_readwrite("t", &my::Rectangle::t)
			.def_readwrite("r", &my::Rectangle::r)
			.def_readwrite("b", &my::Rectangle::b)
			.def("intersect", &my::Rectangle::intersect)
			.def("intersectSelf", &my::Rectangle::intersectSelf, return_reference_to(boost::placeholders::_1))
			.def("union", (my::Rectangle(my::Rectangle::*)(const my::Rectangle &) const)&my::Rectangle::Union)
			.def("unionSelf", (my::Rectangle & (my::Rectangle::*)(const my::Rectangle &))&my::Rectangle::unionSelf, return_reference_to(boost::placeholders::_1))
			.def("union", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::Union)
			.def("unionSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::unionSelf, return_reference_to(boost::placeholders::_1))
			.def("offset", (my::Rectangle(my::Rectangle::*)(float, float) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::offsetSelf, return_reference_to(boost::placeholders::_1))
			.def("offset", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::offsetSelf, return_reference_to(boost::placeholders::_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::shrinkSelf, return_reference_to(boost::placeholders::_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::shrinkSelf, return_reference_to(boost::placeholders::_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(float, float, float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float, float, float))&my::Rectangle::shrinkSelf, return_reference_to(boost::placeholders::_1))
			.def("shrink", (my::Rectangle(my::Rectangle::*)(const my::Vector4 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector4 &))&my::Rectangle::shrinkSelf, return_reference_to(boost::placeholders::_1))
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
			.def(constructor<const my::Quaternion&>())
			.def_readwrite("x", &my::Quaternion::x)
			.def_readwrite("y", &my::Quaternion::y)
			.def_readwrite("z", &my::Quaternion::z)
			.def_readwrite("w", &my::Quaternion::w)
			.def_readwrite("xyz", &my::Quaternion::xyz)
			.def(-const_self)
			.def(const_self + other<const my::Quaternion &>())
			.def(const_self + float())
			.def(const_self - other<const my::Quaternion &>())
			.def(const_self - float())
			.def(const_self * other<const my::Quaternion &>())
			.def(const_self * other<const my::Vector3 &>())
			.def(const_self * float())
			.def(const_self / other<const my::Quaternion &>())
			.def(const_self / float())
			.property("conjugate", &my::Quaternion::conjugate)
			.def("conjugateSelf", &my::Quaternion::conjugateSelf, return_reference_to(boost::placeholders::_1))
			.def("dot", &my::Quaternion::dot)
			.property("inverse", &my::Quaternion::inverse)
			.property("magnitude", &my::Quaternion::magnitude)
			.property("magnitudeSq", &my::Quaternion::magnitudeSq)
			.property("ln", &my::Quaternion::ln)
			.def("multiply", &my::Quaternion::multiply)
			.property("normalize", &my::Quaternion::normalize)
			.def("normalizeSelf", &my::Quaternion::normalizeSelf, return_reference_to(boost::placeholders::_1))
			.def("lerp", &my::Quaternion::lerp)
			.def("lerpSelf", &my::Quaternion::lerpSelf, return_reference_to(boost::placeholders::_1))
			.def("slerp", &my::Quaternion::slerp)
			.def("slerpSelf", &my::Quaternion::slerpSelf, return_reference_to(boost::placeholders::_1))
			.def("squad", &my::Quaternion::squad)
			.def("squadSelf", &my::Quaternion::squadSelf, return_reference_to(boost::placeholders::_1))
			.def("toAxisAngle", &my::Quaternion::toAxisAngle, pure_out_value(boost::placeholders::_2) + pure_out_value(boost::placeholders::_3))
			.property("EulerAngles", &my::Quaternion::toEulerAngles)
			.scope
			[
				def("Identity", &my::Quaternion::Identity),
				def("RotationAxis", &my::Quaternion::RotationAxis),
				def("RotationMatrix", &my::Quaternion::RotationMatrix),
				def("RotationYawPitchRoll", &my::Quaternion::RotationYawPitchRoll),
				def("RotationFromTo", &my::Quaternion::RotationFromToSafe),
				def("RotationEulerAngles", &my::Quaternion::RotationEulerAngles)
			]

		, class_<my::Matrix4>("Matrix4")
			.def(constructor<float>())
			.property("row0", &my::Matrix4::getRow<0>, &my::Matrix4::setRow<0>)
			.property("row1", &my::Matrix4::getRow<1>, &my::Matrix4::setRow<1>)
			.property("row2", &my::Matrix4::getRow<2>, &my::Matrix4::setRow<2>)
			.property("row3", &my::Matrix4::getRow<3>, &my::Matrix4::setRow<3>)
			.property("column0", &my::Matrix4::getColumn<0>, &my::Matrix4::setColumn<0>)
			.property("column1", &my::Matrix4::getColumn<1>, &my::Matrix4::setColumn<1>)
			.property("column2", &my::Matrix4::getColumn<2>, &my::Matrix4::setColumn<2>)
			.property("column3", &my::Matrix4::getColumn<3>, &my::Matrix4::setColumn<3>)
			.def(const_self == other<const my::Matrix4 &>())
			.def(-const_self)
			.def(const_self + other<const my::Matrix4 &>())
			.def(const_self + float())
			.def(const_self - other<const my::Matrix4 &>())
			.def(const_self - float())
			.def(const_self * other<const my::Matrix4 &>())
			.def(const_self * float())
			.def(const_self / other<const my::Matrix4 &>())
			.def(const_self / float())
			.def("Decompose", &my::Matrix4::Decompose, pure_out_value(boost::placeholders::_2) + pure_out_value(boost::placeholders::_3) + pure_out_value(boost::placeholders::_4))
			.property("inverse", &my::Matrix4::inverse)
			.def("multiply", &my::Matrix4::multiply)
			.def("multiplyTranspose", &my::Matrix4::multiplyTranspose)
			.property("transpose", &my::Matrix4::transpose)
			.def("transformTranspose", &my::Matrix4::transformTranspose)
			.def("scale", (my::Matrix4(my::Matrix4::*)(float, float, float) const)&my::Matrix4::scale)
			.def("scale", (my::Matrix4(my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::scale)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::scaleSelf, return_reference_to(boost::placeholders::_1))
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::scaleSelf, return_reference_to(boost::placeholders::_1))
			.def("rotateX", &my::Matrix4::rotateX)
			.def("rotateY", &my::Matrix4::rotateY)
			.def("rotateZ", &my::Matrix4::rotateZ)
			.def("rotate", &my::Matrix4::rotate)
			.def("translate", (my::Matrix4(my::Matrix4::*)(float, float, float) const)&my::Matrix4::translate)
			.def("translate", (my::Matrix4(my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::translate)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::translateSelf, return_reference_to(boost::placeholders::_1))
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::translateSelf, return_reference_to(boost::placeholders::_1))
			.def("lerp", &my::Matrix4::lerp)
			.def("lerpSelf", &my::Matrix4::lerpSelf, return_reference_to(boost::placeholders::_1))
			.property("EulerAngles", &my::Matrix4::toEulerAngles)
			.property("Rotation", &my::Matrix4::toRotation)
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

		, class_<my::Bone>("Bone")
			.def(constructor<const my::Vector3 &, const my::Quaternion &>())
			.def(constructor<const my::Vector3 &>())
			.def(constructor<const my::Bone &>())
			.def_readwrite("rotation", &my::Bone::m_rotation)
			.def_readwrite("position", &my::Bone::m_position)
			.def("Increment", &my::Bone::Increment)
			.def("IncrementSelf", &my::Bone::IncrementSelf)
			.def("Lerp", &my::Bone::Lerp)
			.def("LerpSelf", &my::Bone::LerpSelf, return_reference_to(boost::placeholders::_1))
			.def("Transform", (my::Bone(my::Bone::*)(const my::Vector3&, const my::Quaternion&) const)& my::Bone::Transform)
			.def("Transform", (my::Bone(my::Bone::*)(const my::Bone&) const)& my::Bone::Transform)
			.def("TransformSelf", (my::Bone &(my::Bone::*)(const my::Vector3&, const my::Quaternion&))& my::Bone::TransformSelf, return_reference_to(boost::placeholders::_1))
			.def("TransformSelf", (my::Bone &(my::Bone::*)(const my::Bone&))& my::Bone::TransformSelf, return_reference_to(boost::placeholders::_1))
			.def("TransformTranspose", (my::Bone(my::Bone::*)(const my::Vector3&, const my::Quaternion&) const)& my::Bone::TransformTranspose)
			.def("TransformTranspose", (my::Bone(my::Bone::*)(const my::Bone&) const)& my::Bone::TransformTranspose)

		, class_<my::Plane>("Plane")
			.def(constructor<float, float, float, float>())
			.def_readwrite("a", &my::Plane::a)
			.def_readwrite("b", &my::Plane::b)
			.def_readwrite("c", &my::Plane::c)
			.def_readwrite("d", &my::Plane::d)

		, class_<my::Ray>("Ray")
			.def(constructor<const my::Vector3 &, const my::Vector3 &>())
			.def_readwrite("p", &my::Ray::p)
			.def_readwrite("d", &my::Ray::d)
			.def("transform", &my::Ray::transform)
			.def("transformSelf", &my::Ray::transformSelf, return_reference_to(boost::placeholders::_1))

		, class_<my::Frustum>("Frustum")
			.def(constructor<const my::Plane&, const my::Plane&, const my::Plane&, const my::Plane&, const my::Plane&, const my::Plane&>())
			.def_readonly("Up", &my::Frustum::Up)
			.def_readonly("Down", &my::Frustum::Down)
			.def_readonly("Left", &my::Frustum::Left)
			.def_readonly("Right", &my::Frustum::Right)
			.def_readonly("Near", &my::Frustum::Near)
			.def_readonly("Far", &my::Frustum::Far)

		, class_<my::AABB>("AABB")
			.def(constructor<float, float>())
			.def(constructor<float, float, float, float, float, float>())
			.def(constructor<const my::Vector3 &, const my::Vector3 &>())
			.def(constructor<const my::Vector3 &, float>())
			.def_readwrite("min", &my::AABB::m_min)
			.def_readwrite("max", &my::AABB::m_max)
			.scope
			[
				def("Invalid", &my::AABB::Invalid)
			]
			.property("IsValid", &my::AABB::IsValid)
			.def("valid", &my::AABB::valid)
			.def("validSelf", &my::AABB::validSelf, return_reference_to(boost::placeholders::_1))
			.def(const_self == other<const my::AABB &>())
			.def(const_self + other<const my::Vector3 &>())
			.def(const_self + float())
			.def(const_self - other<const my::Vector3 &>())
			.def(const_self - float())
			.def(const_self * other<const my::Vector3 &>())
			.def(const_self * float())
			.def(const_self / other<const my::Vector3 &>())
			.def(const_self / float())
			.property("Center", &my::AABB::Center)
			.property("Extent", &my::AABB::Extent)
			.def("SlicePxPyPz", &my::AABB::Slice<my::AABB::QuadrantPxPyPz>)
			.def("Intersect", (bool (my::AABB::*)(const my::Vector3&) const)& my::AABB::Intersect)
			.def("Intersect2D", &my::AABB::Intersect2D)
			.def("Intersect", (my::AABB (my::AABB::*)(const my::AABB&) const)& my::AABB::Intersect)
			.def("intersectSelf", &my::AABB::intersectSelf, return_reference_to(boost::placeholders::_1))
			.def("union", (my::AABB (my::AABB::*)(const my::Vector3&) const)& my::AABB::Union)
			.def("unionSelf", (my::AABB& (my::AABB::*)(const my::Vector3&))&my::AABB::unionSelf, return_reference_to(boost::placeholders::_1))
			.def("union", (my::AABB (my::AABB::*)(const my::AABB&) const)&my::AABB::Union)
			.def("unionSelf", (my::AABB& (my::AABB::*)(const my::AABB&))&my::AABB::unionSelf, return_reference_to(boost::placeholders::_1))
			.def("shrink", &my::AABB::shrink)
			.def("shrinkSelf", &my::AABB::shrinkSelf, return_reference_to(boost::placeholders::_1))
			.def("p", &my::AABB::p)
			.def("n", &my::AABB::n)
			.def("transform", &my::AABB::transform)
			.def("transformSelf", &my::AABB::transformSelf, return_reference_to(boost::placeholders::_1))

		, class_<my::UDim>("UDim")
			.def(constructor<float, float>())
			.def_readwrite("scale", &my::UDim::scale)
			.def_readwrite("offset", &my::UDim::offset)

		, class_<my::Spline>("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("AddNode", luabind::tag_function<void(my::Spline*,float,float)>(
				boost::bind(&my::Spline::AddNode, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, 0.0f, 0.0f)))
			.def("Interpolate", (float (my::Spline::*)(float, float) const)&my::Spline::Interpolate)
			.def("Interpolate", luabind::tag_function<float(my::Spline*, float)>(
				boost::bind(&my::Spline::Interpolate, boost::placeholders::_1, boost::placeholders::_2, 0.0f)))

		, class_<my::Emitter::Particle>("EmitterParticle")
			.def_readwrite("Position", &my::Emitter::Particle::m_Position)

		, class_<my::BaseCamera, boost::shared_ptr<my::BaseCamera> >("BaseCamera")
			.def_readonly("View", &my::BaseCamera::m_View)
			.def_readonly("Proj", &my::BaseCamera::m_Proj)
			.def_readonly("ViewProj", &my::BaseCamera::m_ViewProj)
			.def_readonly("InverseViewProj", &my::BaseCamera::m_InverseViewProj)
			.def("ScreenToWorld", &my::BaseCamera::ScreenToWorld)
			.def("WorldToScreen", &my::BaseCamera::WorldToScreen)

		, class_<my::Camera, my::BaseCamera, boost::shared_ptr<my::Camera> >("Camera")
			.def_readwrite("Eye", &my::Camera::m_Eye)
			.def_readwrite("Euler", &my::Camera::m_Euler)
			.def_readwrite("Nz", &my::Camera::m_Nz)
			.def_readwrite("Fz", &my::Camera::m_Fz)
			.def("CalculateRay", &my::Camera::CalculateRay)
			.def("CalculateFrustum", &my::Camera::CalculateFrustum)
			.def("AlignUnit", &my::Camera::AlignUnit)

		, class_<my::OrthoCamera, my::Camera, boost::shared_ptr<my::Camera> >("OrthoCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Width", &my::OrthoCamera::m_Width)
			.def_readwrite("Height", &my::OrthoCamera::m_Height)

		, class_<my::PerspectiveCamera, my::Camera, boost::shared_ptr<my::Camera> >("PerspectiveCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Fov", &my::PerspectiveCamera::m_Fov)
			.def_readwrite("Aspect", &my::PerspectiveCamera::m_Aspect)

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

		, class_<tagPOINT>("tagPOINT")
			.def_readwrite("x", &tagPOINT::x)
			.def_readwrite("y", &tagPOINT::y)

		, class_<CPoint, tagPOINT>("CPoint")
			.def(constructor<int, int>())

		, class_<tagSIZE>("tagSIZE")
			.def_readwrite("cx", &tagSIZE::cx)
			.def_readwrite("cy", &tagSIZE::cy)

		, class_<CSize, tagSIZE>("CSize")
			.def(constructor<int, int>())

		, class_<tagRECT>("tagRECT")
			.def_readwrite("left", &tagRECT::left)
			.def_readwrite("top", &tagRECT::top)
			.def_readwrite("right", &tagRECT::right)
			.def_readwrite("bottom", &tagRECT::bottom)

		, class_<CRect, tagRECT>("CRect")
			.def(constructor<int, int, int, int>())
			.def(constructor<tagPOINT, tagSIZE>())

		, class_<my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("DeviceResourceBase")
			.def_readonly("Key", &my::DeviceResourceBase::m_Key)

		, class_<my::BaseTexture, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("BaseTexture")
			.enum_("D3DFORMAT")
			[
				value("D3DFMT_UNKNOWN", D3DFMT_UNKNOWN),
				//value("D3DFMT_R8G8B8", D3DFMT_R8G8B8),
				value("D3DFMT_A8R8G8B8", D3DFMT_A8R8G8B8),
				value("D3DFMT_X8R8G8B8", D3DFMT_X8R8G8B8)
			]
			.def("GenerateMipSubLevels", &basetexture_generate_mip_sub_levels)
			.def("GetLevelDesc", &my::BaseTexture::GetLevelDesc)
			.enum_("D3DXIMAGE_FILEFORMAT")
			[
				value("D3DXIFF_BMP", D3DXIFF_BMP),
				value("D3DXIFF_JPG", D3DXIFF_JPG),
				value("D3DXIFF_TGA", D3DXIFF_TGA),
				value("D3DXIFF_PNG", D3DXIFF_PNG),
				value("D3DXIFF_DDS", D3DXIFF_DDS),
				value("D3DXIFF_PPM", D3DXIFF_PPM),
				value("D3DXIFF_DIB", D3DXIFF_DIB),
				value("D3DXIFF_HDR", D3DXIFF_HDR),
				value("D3DXIFF_PFM", D3DXIFF_PFM)
			]
			.def("SaveTextureToFile", &basetexture_save_texture_to_file)
			.enum_("D3DLOCK")
			[
				value("D3DLOCK_READONLY", D3DLOCK_READONLY),
				value("D3DLOCK_DISCARD", D3DLOCK_DISCARD),
				value("D3DLOCK_NOOVERWRITE", D3DLOCK_NOOVERWRITE),
				value("D3DLOCK_NOSYSLOCK", D3DLOCK_NOSYSLOCK),
				value("D3DLOCK_DONOTWAIT", D3DLOCK_DONOTWAIT),
				value("D3DLOCK_NO_DIRTY_UPDATE", D3DLOCK_NO_DIRTY_UPDATE)
			]
			.enum_("D3DUSAGE")
			[
				value("D3DUSAGE_RENDERTARGET", D3DUSAGE_RENDERTARGET),
				value("D3DUSAGE_DEPTHSTENCIL", D3DUSAGE_DEPTHSTENCIL),
				value("D3DUSAGE_DYNAMIC", D3DUSAGE_DYNAMIC)
			]
			.enum_("D3DPOOL")
			[
				value("D3DPOOL_DEFAULT", D3DPOOL_DEFAULT),
				value("D3DPOOL_MANAGED", D3DPOOL_MANAGED),
				value("D3DPOOL_SYSTEMMEM", D3DPOOL_MANAGED),
				value("D3DPOOL_SCRATCH", D3DPOOL_SCRATCH)
			]

		, class_<my::Texture2D, my::BaseTexture, boost::shared_ptr<my::DeviceResourceBase> >("Texture2D")
			.def(constructor<>())
			.def("LockRect", &my::Texture2D::LockRect)
			.def("UnlockRect", &my::Texture2D::UnlockRect)
			.def("CreateTexture", &my::Texture2D::CreateTexture)
			.def("CreateTextureFromFile", &texture2d_create_texture_from_file)
			.def("SetAsRenderTarget", &texture2d_set_as_render_target)
			.def("FillColor", &texture2d_fill_color)

		, class_<my::CubeTexture, my::BaseTexture, boost::shared_ptr<my::DeviceResourceBase> >("CubeTexture")
			.def(constructor<>())
			.def("CreateCubeTexture", luabind::tag_function<void(my::CubeTexture*, UINT, UINT, D3DFORMAT)>(
				boost::bind(&my::CubeTexture::CreateAdjustedCubeTexture, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, 0, boost::placeholders::_4, D3DPOOL_MANAGED)))
			.enum_("D3DCUBEMAP_FACES")
			[
				value("D3DCUBEMAP_FACE_POSITIVE_X", D3DCUBEMAP_FACE_POSITIVE_X),
				value("D3DCUBEMAP_FACE_NEGATIVE_X", D3DCUBEMAP_FACE_NEGATIVE_X),
				value("D3DCUBEMAP_FACE_POSITIVE_Y", D3DCUBEMAP_FACE_POSITIVE_Y),
				value("D3DCUBEMAP_FACE_NEGATIVE_Y", D3DCUBEMAP_FACE_NEGATIVE_Y),
				value("D3DCUBEMAP_FACE_POSITIVE_Z", D3DCUBEMAP_FACE_POSITIVE_Z),
				value("D3DCUBEMAP_FACE_NEGATIVE_Z", D3DCUBEMAP_FACE_NEGATIVE_Z)
			]
			.def("LoadCubeMapSurfaceFromFile", &cubetexture_load_cube_map_surface_from_file)

		, class_<my::Mesh, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("Mesh")
			.property("NumFaces", &my::Mesh::GetNumFaces)
			.property("NumVertices", &my::Mesh::GetNumVertices)

		, class_<my::OgreMesh, my::Mesh, boost::shared_ptr<my::DeviceResourceBase> >("OgreMesh")
			.def(constructor<>())
			.def("CreateMeshFromOgreXmlInFile", &ogremesh_create_mesh_from_ogre_xml_in_file)
			.def("CreateMeshFromObjInFile", &ogremesh_create_mesh_from_obj_in_file)
			.def("CreateMeshFromOther", &my::OgreMesh::CreateMeshFromOther)
			.def("AppendMesh", &my::OgreMesh::AppendMesh)
			.def("CombineMesh", &my::OgreMesh::CombineMesh)
			.def("SaveOgreMesh", &my::OgreMesh::SaveOgreMesh)
			.enum_("D3DXMESHOPT")
			[
				value("D3DXMESH_MANAGED", D3DXMESH_MANAGED),
				value("D3DXMESHOPT_COMPACT", D3DXMESHOPT_COMPACT),
				value("D3DXMESHOPT_ATTRSORT", D3DXMESHOPT_ATTRSORT),
				value("D3DXMESHOPT_VERTEXCACHE", D3DXMESHOPT_VERTEXCACHE),
				value("D3DXMESHOPT_STRIPREORDER", D3DXMESHOPT_STRIPREORDER),
				value("D3DXMESHOPT_IGNOREVERTS", D3DXMESHOPT_IGNOREVERTS),
				value("D3DXMESHOPT_DONOTSPLIT", D3DXMESHOPT_DONOTSPLIT),
				value("D3DXMESHOPT_DEVICEINDEPENDENT", D3DXMESHOPT_DEVICEINDEPENDENT)
			]
			.def("Optimize", &my::OgreMesh::Optimize)
			.def("OptimizeInplace", &my::OgreMesh::OptimizeInplace)
			.enum_("D3DXMESHSIMP")
			[
				value("D3DXMESHSIMP_VERTEX", D3DXMESHSIMP_VERTEX),
				value("D3DXMESHSIMP_FACE", D3DXMESHSIMP_FACE)
			]
			.def("SimplifyMesh", &my::OgreMesh::SimplifyMesh)
			.def("SaveObj", &my::OgreMesh::SaveObj)
			.def("Transform", &my::OgreMesh::Transform)
			.property("MaterialNum", &ogremesh_get_material_num)
			.def("GetMaterialName", &ogremesh_get_material_name)
			.property("LodNum", &ogremesh_get_lod_num)
			.def("GetLodName", &ogremesh_get_lod_name)
			.def("CalculateAABB", &my::OgreMesh::CalculateAABB)

		, class_<my::BoneHierarchyNode>("BoneHierarchyNode")
			.def_readonly("sibling", &my::BoneHierarchyNode::m_sibling)
			.def_readonly("child", &my::BoneHierarchyNode::m_child)

		, class_<my::BoneHierarchy>("BoneHierarchy")

		, class_<my::OgreSkeletonAnimation, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("OgreSkeletonAnimation")
			.def_readonly("boneHierarchy", &my::OgreSkeletonAnimation::m_boneHierarchy)
			.def("GetBoneIndex", &my::OgreSkeletonAnimation::GetBoneIndex)
			.def("GetBoneName", &my::OgreSkeletonAnimation::FindBoneName)
			.property("BoneNum", &ogreskeletonanimation_get_bone_num)
			.def("GetBindPoseBone", &ogreskeletonanimation_get_bind_pose_bone)
			.def("AddOgreSkeletonAnimationFromFile", &my::OgreSkeletonAnimation::AddOgreSkeletonAnimationFromFile)
			.def("SaveOgreSkeletonAnimation", &my::OgreSkeletonAnimation::SaveOgreSkeletonAnimation)
			//.def("AdjustAnimationRoot", &my::OgreSkeletonAnimation::AdjustAnimationRoot)

		// ! many methods of my::BaseEffect, my::Effect cannot be use in lua
		, class_<my::BaseEffect, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("BaseEffect")
			.def("GetAnnotation", &my::BaseEffect::GetAnnotation)
			.def("GetAnnotationByName", &my::BaseEffect::GetAnnotationByName)
			.def("GetBool", &my::BaseEffect::GetBool)
			.def("GetBoolArray", &my::BaseEffect::GetBoolArray)
			.def("GetDesc", &my::BaseEffect::GetDesc)
			.def("GetFloat", &my::BaseEffect::GetFloat)
			.def("GetFloatArray", &my::BaseEffect::GetFloatArray)
			.def("GetFunction", &my::BaseEffect::GetFunction)
			.def("GetFunctionByName", &my::BaseEffect::GetFunctionByName)
			.def("GetFunctionDesc", &my::BaseEffect::GetFunctionDesc)
			.def("GetInt", &my::BaseEffect::GetInt)
			.def("GetIntArray", &my::BaseEffect::GetIntArray)
			.def("GetMatrix", &my::BaseEffect::GetMatrix)
			.def("GetMatrixArray", &my::BaseEffect::GetMatrixArray)
			.def("GetMatrixPointerArray", &my::BaseEffect::GetMatrixPointerArray)
			.def("GetMatrixTranspose", &my::BaseEffect::GetMatrixTranspose)
			.def("GetMatrixTransposeArray", &my::BaseEffect::GetMatrixTransposeArray)
			.def("GetMatrixTransposePointerArray", &my::BaseEffect::GetMatrixTransposePointerArray)
			.def("GetParameter", &my::BaseEffect::GetParameter)
			.def("GetParameterByName", &my::BaseEffect::GetParameterByName)
			.def("GetParameterBySemantic", &my::BaseEffect::GetParameterBySemantic)
			.def("GetParameterDesc", &my::BaseEffect::GetParameterDesc)
			.def("GetParameterElement", &my::BaseEffect::GetParameterElement)
			.def("GetPass", &my::BaseEffect::GetPass)
			.def("GetPassByName", &my::BaseEffect::GetPassByName)
			.def("GetPassDesc", &my::BaseEffect::GetPassDesc)
			.def("GetPixelShader", &my::BaseEffect::GetPixelShader)
			.def("GetString", &my::BaseEffect::GetString)
			.def("GetTechnique", &my::BaseEffect::GetTechnique)
			.def("GetTechniqueByName", &my::BaseEffect::GetTechniqueByName)
			.def("GetTechniqueDesc", &my::BaseEffect::GetTechniqueDesc)
			.def("GetTexture", &my::BaseEffect::GetTexture)
			.def("GetValue", &my::BaseEffect::GetValue)
			.def("GetVector", &my::BaseEffect::GetVector)
			.def("GetVectorArray", &my::BaseEffect::GetVectorArray)
			.def("GetVertexShader", &my::BaseEffect::GetVertexShader)
			.def("SetArrayRange", &my::BaseEffect::SetArrayRange)
			.def("SetBool", &my::BaseEffect::SetBool)
			.def("SetBoolArray", &my::BaseEffect::SetBoolArray)
			.def("SetFloat", &my::BaseEffect::SetFloat)
			.def("SetFloatArray", &my::BaseEffect::SetFloatArray)
			.def("SetInt", &my::BaseEffect::SetInt)
			.def("SetIntArray", &my::BaseEffect::SetIntArray)
			.def("SetMatrix", &my::BaseEffect::SetMatrix)
			.def("SetMatrixArray", &my::BaseEffect::SetMatrixArray)
			.def("SetMatrixPointerArray", &my::BaseEffect::SetMatrixPointerArray)
			.def("SetMatrixTranspose", &my::BaseEffect::SetMatrixTranspose)
			.def("SetMatrixTransposeArray", &my::BaseEffect::SetMatrixTransposeArray)
			.def("SetMatrixTransposePointerArray", &my::BaseEffect::SetMatrixTransposePointerArray)
			.def("SetString", &my::BaseEffect::SetString)
			// ! luabind cannot convert boost::shared_ptr<Derived Class> to base ptr
			.def("SetTexture", &my::BaseEffect::SetTexture)
			.def("SetValue", &my::BaseEffect::SetValue)
			.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector4 &))&my::BaseEffect::SetVector)
			.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector3 &))&my::BaseEffect::SetVector)
			.def("SetVectorArray", &my::BaseEffect::SetVectorArray)

		, class_<my::Effect, my::BaseEffect, boost::shared_ptr<my::DeviceResourceBase> >("Effect")
			.def("ApplyParameterBlock", &my::Effect::ApplyParameterBlock)
			.def("Begin", &my::Effect::Begin)
			.def("BeginParameterBlock", &my::Effect::BeginParameterBlock)
			.def("BeginPass", &my::Effect::BeginPass)
			.def("CloneEffect", &my::Effect::CloneEffect)
			.def("CommitChanges", &my::Effect::CommitChanges)
			.def("DeleteParameterBlock", &my::Effect::DeleteParameterBlock)
			.def("End", &my::Effect::End)
			.def("EndParameterBlock", &my::Effect::EndParameterBlock)
			.def("EndPass", &my::Effect::EndPass)
			.def("FindNextValidTechnique", &my::Effect::FindNextValidTechnique)
			.def("GetCurrentTechnique", &my::Effect::GetCurrentTechnique)
			.def("GetDevice", &my::Effect::GetDevice)
			.def("GetPool", &my::Effect::GetPool)
			.def("GetStateManager", &my::Effect::GetStateManager)
			.def("IsParameterUsed", &my::Effect::IsParameterUsed)
			.def("SetRawValue", &my::Effect::SetRawValue)
			.def("SetStateManager", &my::Effect::SetStateManager)
			.def("SetTechnique", &my::Effect::SetTechnique)
			.def("ValidateTechnique", &my::Effect::ValidateTechnique)

		, class_<my::Font, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("Font")
			.enum_("Align")
			[
				value("AlignLeftTop", my::Font::AlignLeftTop),
				value("AlignLeftTopMultiLine", my::Font::AlignLeftTop | my::Font::AlignMultiLine),
				value("AlignCenterTop", my::Font::AlignCenterTop),
				value("AlignRightTop", my::Font::AlignRightTop),
				value("AlignLeftMiddle", my::Font::AlignLeftMiddle),
				value("AlignCenterMiddle", my::Font::AlignCenterMiddle),
				value("AlignRightMiddle", my::Font::AlignRightMiddle),
				value("AlignLeftBottom", my::Font::AlignLeftBottom),
				value("AlignCenterBottom", my::Font::AlignCenterBottom),
				value("AlignRightBottom", my::Font::AlignRightBottom),
				value("AlignVertical", my::Font::AlignVertical)
			]
			.def_readonly("Height", &my::Font::m_Height)
			.def_readonly("LineHeight", &my::Font::m_LineHeight)
			.property("Scale", &my::Font::GetScale, &my::Font::SetScale)

		, class_<my::Wav, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("Wav")

		, class_<my::Cache, boost::shared_ptr<my::Cache> >("Cache")
			.def("push_back", (void(my::Cache::*)(const unsigned char &))&my::Cache::push_back)

		, class_<my::IStream, boost::shared_ptr<my::IStream> >("IStream")
			.def("GetWholeCache", &my::IStream::GetWholeCache)

		, class_<my::ResourceMgr>("ResourceMgr")
			.def("CheckPath", &my::ResourceMgr::CheckPath)
			.def("GetFullPath", &my::ResourceMgr::GetFullPath)
			.def("GetRelativePath", &my::ResourceMgr::GetRelativePath)
			.def("OpenIStream", &my::ResourceMgr::OpenIStream)
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
			.def("LoadWav", &my::ResourceMgr::LoadWav)
			.def("LoadWavAsync", &my::ResourceMgr::LoadWavAsync<luabind::object>)

		//, def("res2texture", (boost::shared_ptr<my::BaseTexture>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::BaseTexture, my::DeviceResourceBase>)
		//, def("res2mesh", (boost::shared_ptr<my::Mesh>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::Mesh, my::DeviceResourceBase>)
		//, def("res2skeleton", (boost::shared_ptr<my::OgreSkeletonAnimation>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::OgreSkeletonAnimation, my::DeviceResourceBase>)
		//, def("res2effect", (boost::shared_ptr<my::Effect>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::Effect, my::DeviceResourceBase>)
		//, def("res2font", (boost::shared_ptr<my::Font>(*)(const boost::shared_ptr<my::DeviceResourceBase>&))& boost::dynamic_pointer_cast<my::Font, my::DeviceResourceBase>)
	];

	module(m_State)[
		def("ARGB", &ARGB)

		, def("Counter", &Counter)

		, def("PrintCallStack", (std::string (*)(void))& PrintCallStack)

		, def("CopyFile", &CopyFileA)

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
			.def_readonly("self", &my::ControlEventArg::self)

		, class_<my::VisibleEventArg, my::ControlEventArg>("VisibleEventArg")
			.def_readonly("Visible", &my::VisibleEventArg::Visible)

		, class_<my::FocusEventArg, my::ControlEventArg>("FocusEventArg")
			.def_readonly("Focused", &my::FocusEventArg::Focused)

		, class_<my::MouseEventArg, my::ControlEventArg>("MouseEventArg")
			.def_readonly("pt", &my::MouseEventArg::pt)

		, class_<my::UIRender>("UIRender")
			.def_readonly("WhiteTex", &my::UIRender::m_WhiteTex)
			.def("Begin", &my::UIRender::Begin)
			.def("End", &my::UIRender::End)
			.def("SetWorld", &my::UIRender::SetWorld)
			.def("SetViewProj", &my::UIRender::SetViewProj)
			.def("Flush", &my::UIRender::Flush)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, D3DCOLOR, const my::Rectangle&, const my::BaseTexture*))& my::UIRender::PushRectangle)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, D3DCOLOR, const my::Rectangle&, const my::BaseTexture*, const my::Rectangle&))& my::UIRender::PushRectangle)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, D3DCOLOR, const my::Rectangle&, const my::BaseTexture*, const my::Matrix4&))& my::UIRender::PushRectangle)
			.def("PushRectangle", (void (my::UIRender::*)(const my::Rectangle&, D3DCOLOR, const my::Rectangle&, const my::BaseTexture*, const my::Matrix4&, const my::Rectangle&))& my::UIRender::PushRectangle)
			.def("PushWindow", (void (my::UIRender::*)(const my::Rectangle&, DWORD, const my::Rectangle&, const my::Vector4&, const my::BaseTexture*))& my::UIRender::PushWindow)
			.def("PushWindow", (void (my::UIRender::*)(const my::Rectangle&, DWORD, const my::Rectangle&, const my::Vector4&, const my::BaseTexture*, const my::Rectangle&))& my::UIRender::PushWindow)
			.def("PushString", luabind::tag_function<void(my::UIRender*, const my::Rectangle&, const std::wstring&, D3DCOLOR, my::Font::Align, my::Font*)>(
				boost::bind(&my::UIRender::PushString, boost::placeholders::_1, boost::placeholders::_2, boost::bind(&std::wstring::c_str, boost::placeholders::_3), boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6)))
			.def("PushString", luabind::tag_function<void(my::UIRender*, const my::Rectangle&, const std::wstring&, D3DCOLOR, my::Font::Align, D3DCOLOR, float, my::Font*)>(
				boost::bind(&my::UIRender::PushString, boost::placeholders::_1, boost::placeholders::_2, boost::bind(&std::wstring::c_str, boost::placeholders::_3), boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8)))
			.def("PushString", luabind::tag_function<void(my::UIRender*, const my::Rectangle&, const std::wstring&, D3DCOLOR, my::Font::Align, my::Font*, const my::Matrix4&)>(
				boost::bind(&my::UIRender::PushString, boost::placeholders::_1, boost::placeholders::_2, boost::bind(&std::wstring::c_str, boost::placeholders::_3), boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7)))
			.def("PushString", luabind::tag_function<void(my::UIRender*, const my::Rectangle&, const std::wstring&, D3DCOLOR, my::Font::Align, D3DCOLOR, float, my::Font*, const my::Matrix4&)>(
				boost::bind(&my::UIRender::PushString, boost::placeholders::_1, boost::placeholders::_2, boost::bind(&std::wstring::c_str, boost::placeholders::_3), boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8, boost::placeholders::_9)))
			.def("PushLayer", &my::UIRender::GetVertexList, luabind::discard_result)

		, class_<my::ControlImage, boost::shared_ptr<my::ControlImage> >("ControlImage")
			.def(constructor<>())
			.def_readwrite("TexturePath", &my::ControlImage::m_TexturePath)
			.def_readwrite("Texture", &my::ControlImage::m_Texture)
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
			.def_readwrite("VisibleShowSoundPath", &my::ControlSkin::m_VisibleShowSoundPath)
			.def_readwrite("VisibleHideSoundPath", &my::ControlSkin::m_VisibleHideSoundPath)
			.def_readwrite("MouseEnterSoundPath", &my::ControlSkin::m_MouseEnterSoundPath)
			.def_readwrite("MouseLeaveSoundPath", &my::ControlSkin::m_MouseLeaveSoundPath)
			.def_readwrite("MouseClickSoundPath", &my::ControlSkin::m_MouseClickSoundPath)
			.def("DrawImage", (void (my::ControlSkin::*)(my::UIRender*, const my::ControlImagePtr&, const my::Rectangle&, DWORD))& my::ControlSkin::DrawImage)
			.def("DrawImage", (void (my::ControlSkin::*)(my::UIRender*, const my::ControlImagePtr&, const my::Rectangle&, DWORD, const my::Rectangle&))& my::ControlSkin::DrawImage)
			.def("Clone", &my::ControlSkin::Clone)

		, class_<my::Control, my::NamedObject, ScriptControl/*, boost::shared_ptr<my::Control>*/ >("Control")
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
			.def_readonly("Childs", &my::Control::m_Childs, return_stl_iterator)
			.def_readonly("Parent", &my::Control::m_Parent)
			.def_readwrite("x", &my::Control::m_x)
			.def_readwrite("y", &my::Control::m_y)
			.def_readwrite("Width", &my::Control::m_Width)
			.def_readwrite("Height", &my::Control::m_Height)
			.def_readonly("Rect", &my::Control::m_Rect)
			.def_readwrite("Skin", &my::Control::m_Skin)
			.def_readwrite("Pressed", &my::Control::m_bPressed)
			.def_readwrite("EventVisibleChanged", &my::Control::m_EventVisibleChanged)
			.def_readwrite("EventFocusChanged", &my::Control::m_EventFocusChanged)
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
			.def("OnLayout", &my::Control::OnLayout)
			.property("Enabled", &my::Control::GetEnabled, &my::Control::SetEnabled)
			.property("EnabledHierarchy", &my::Control::GetEnabledHierarchy)
			.property("Visible", &my::Control::GetVisible, &my::Control::SetVisible)
			.property("VisibleHierarchy", &my::Control::GetVisibleHierarchy)
			.def_readwrite("Hotkey", &my::Control::m_nHotkey)
			.property("Focused", &my::Control::GetFocused, &my::Control::SetFocused)
			.property("Captured", &my::Control::GetCaptured, &my::Control::SetCaptured)
			.property("MouseOver", &my::Control::GetMouseOver, &my::Control::SetMouseOver)
			.def("InsertControl", (void(my::Control::*)(boost::shared_ptr<my::Control>))& my::Control::InsertControl)
			.def("InsertControl", (void(my::Control::*)(unsigned int, boost::shared_ptr<my::Control>))& my::Control::InsertControl)
			.def("InsertControlAdopt", (void(*)(my::Control*, ScriptControl*))& control_insert_control_adopt, adopt(boost::placeholders::_2))
			.def("InsertControlAdopt", (void(*)(my::Control*, unsigned int i, ScriptControl*))& control_insert_control_adopt, adopt(boost::placeholders::_3))
			.def("RemoveControl", &my::Control::RemoveControl)
			.property("ChildNum", &my::Control::GetChildNum)
			.property("SiblingId", &my::Control::GetSiblingId, &my::Control::SetSiblingId)
			.def("ClearAllControl", &my::Control::ClearAllControl)
			.def("ContainsControl", &my::Control::ContainsControl)
			.property("TopControl", &my::Control::GetTopControl)

		, class_<my::StaticSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("StaticSkin")
			.def(constructor<>())
			.def_readwrite("FontPath", &my::StaticSkin::m_FontPath)
			.def_readwrite("FontHeight", &my::StaticSkin::m_FontHeight)
			.def_readwrite("FontFaceIndex", &my::StaticSkin::m_FontFaceIndex)
			.def_readwrite("TextColor", &my::StaticSkin::m_TextColor)
			.def_readwrite("TextAlign", &my::StaticSkin::m_TextAlign)
			.def_readwrite("TextOutlineColor", &my::StaticSkin::m_TextOutlineColor)
			.def_readwrite("TextOutlineWidth", &my::StaticSkin::m_TextOutlineWidth)
			.def("DrawString", &my::StaticSkin::DrawString)

		, class_<my::Static, my::Control, boost::shared_ptr<my::Control> >("Static")
			.def(constructor<const char *>())
			.def_readwrite("Text", &my::Static::m_Text)

		, class_<my::ProgressBarSkin, my::StaticSkin, boost::shared_ptr<my::ControlSkin> >("ProgressBarSkin")
			.def(constructor<>())
			.def_readwrite("ForegroundImage", &my::ProgressBarSkin::m_ForegroundImage)

		, class_<my::ProgressBar, my::Static, boost::shared_ptr<my::Control> >("ProgressBar")
			.def(constructor<const char *>())
			.def_readwrite("Progress", &my::ProgressBar::m_Progress)

		, class_<my::ButtonSkin, my::StaticSkin, boost::shared_ptr<my::ControlSkin> >("ButtonSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::ButtonSkin::m_DisabledImage)
			.def_readwrite("PressedImage", &my::ButtonSkin::m_PressedImage)
			.def_readwrite("MouseOverImage", &my::ButtonSkin::m_MouseOverImage)
			.def_readwrite("PressedOffset", &my::ButtonSkin::m_PressedOffset)

		, class_<my::Button, my::Static, boost::shared_ptr<my::Control> >("Button")
			.def(constructor<const char *>())
			.def("SetHotkey", &my::Button::SetHotkey)

		, class_<my::EditBoxSkin, my::StaticSkin, boost::shared_ptr<my::ControlSkin> >("EditBoxSkin")
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

		, class_<my::ComboBox, my::Button, boost::shared_ptr<my::Control> >("ComboBox")
			.def(constructor<const char *>())
			.def_readonly("ScrollBar", &my::ComboBox::m_ScrollBar)
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

		, class_<my::ListBox, my::Control, boost::shared_ptr<my::Control> >("ListBox")
			.def(constructor<const char *>())
			.def_readonly("ScrollBar", &my::ListBox::m_ScrollBar)
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
		class_<HWND__>("HWND")

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
				value("WM_CLOSE", WM_CLOSE),
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
			.def_readonly("hWnd", &my::DxutWindow::m_hWnd)
			.def("PostMessage", (BOOL(CWindow::*)(UINT))& my::DxutWindow::PostMessage)
			.def("PostMessage", (BOOL(CWindow::*)(UINT, WPARAM))& my::DxutWindow::PostMessage)
			.def("PostMessage", (BOOL(CWindow::*)(UINT, WPARAM, LPARAM))& my::DxutWindow::PostMessage)

		, class_<my::D3DContext>("D3DContext")
			.def_readonly("AbsoluteTime", &my::D3DContext::m_fAbsoluteTime)
			.def_readwrite("TimeScale", &my::D3DContext::m_fTimeScale)
			.def_readonly("ElapsedTime", &my::D3DContext::m_fElapsedTime)
			.def_readonly("TotalTime", &my::D3DContext::m_fTotalTime)
			.def_readonly("DeviceSettings", &my::D3DContext::m_DeviceSettings, copy(result))
			.def_readonly("BackBufferSurfaceDesc", &my::D3DContext::m_BackBufferSurfaceDesc)
			.property("NamedObjects", &d3dcontext_get_named_object_list, return_stl_iterator)
			.def("GetNamedObject", &my::D3DContext::GetNamedObject)

		, class_<my::DxutApp, bases<my::D3DContext, CD3D9Enumeration> >("DxutApp")
			.def_readonly("wnd", &my::DxutApp::m_wnd)
			.scope
			[
				def("DXUTD3DDeviceTypeToString", &my::DxutApp::DXUTD3DDeviceTypeToString),
				def("DXUTD3DFormatToString", &my::DxutApp::DXUTD3DFormatToString),
				def("DXUTMultisampleTypeToString", &my::DxutApp::DXUTMultisampleTypeToString),
				def("DXUTVertexProcessingTypeToString", &my::DxutApp::DXUTVertexProcessingTypeToString)
			]
			.def("ToggleFullScreen", &my::DxutApp::ToggleFullScreen)
			.def("ToggleREF", &my::DxutApp::ToggleREF)
			.property("SoftwareVP", &my::DxutApp::GetSoftwareVP, &my::DxutApp::SetSoftwareVP)
			.property("HardwareVP", &my::DxutApp::GetHardwareVP, &my::DxutApp::SetHardwareVP)
			.property("PureHardwareVP", &my::DxutApp::GetPureHardwareVP, &my::DxutApp::SetPureHardwareVP)
			.property("MixedVP", &my::DxutApp::GetMixedVP, &my::DxutApp::SetMixedVP)
			.def("ChangeDevice", &my::DxutApp::ChangeDevice)

		, class_<my::InputDevice, boost::shared_ptr<my::InputDevice> >("InputDevice")
			.enum_("CooperativeLevelType")
			[
				value("DISCL_EXCLUSIVE", DISCL_EXCLUSIVE),
				value("DISCL_NONEXCLUSIVE", DISCL_NONEXCLUSIVE),
				value("DISCL_FOREGROUND", DISCL_FOREGROUND),
				value("DISCL_BACKGROUND", DISCL_BACKGROUND),
				value("DISCL_NOWINKEY", DISCL_NOWINKEY)
			]
			.def("SetCooperativeLevel", &my::InputDevice::SetCooperativeLevel)
			.def("Unacquire", &my::InputDevice::Unacquire)

		, class_<my::Keyboard, my::InputDevice, boost::shared_ptr<my::InputDevice> >("Keyboard")
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
			.def("SetKeyState", &my::Keyboard::SetKeyState)
			.def("IsKeyDown", &my::Keyboard::IsKeyDown)
			.def("IsKeyPress", &my::Keyboard::IsKeyPress)
			.def("IsKeyRelease", &my::Keyboard::IsKeyRelease)

		, class_<my::Mouse, my::InputDevice, boost::shared_ptr<my::InputDevice> >("Mouse")
			.property("X", &my::Mouse::GetX)
			.property("Y", &my::Mouse::GetY)
			.property("Z", &my::Mouse::GetZ)
			.def("IsButtonDown", &my::Mouse::IsButtonDown)
			.def("IsButtonPress", &my::Mouse::IsButtonPress)
			.def("IsButtonRelease", &my::Mouse::IsButtonRelease)

		, class_<my::Joystick, my::InputDevice, boost::shared_ptr<my::InputDevice> >("Joystick")
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
			.enum_("Type")
			[
				value("KeyboardButton", my::InputMgr::KeyboardButton),
				value("KeyboardNegativeButton", my::InputMgr::KeyboardNegativeButton),
				value("MouseMove", my::InputMgr::MouseMove),
				value("MouseButton", my::InputMgr::MouseButton),
				value("JoystickAxis", my::InputMgr::JoystickAxis),
				value("JoystickNegativeAxis", my::InputMgr::JoystickNegativeAxis),
				value("JoystickPov", my::InputMgr::JoystickPov),
				value("JoystickNegativePov", my::InputMgr::JoystickNegativePov),
				value("JoystickButton", my::InputMgr::JoystickButton)
			]

		, class_<my::SoundBuffer, boost::shared_ptr<my::SoundBuffer> >("SoundBuffer")
			.def("Play", &my::SoundBuffer::Play)
			.def("Stop", &my::SoundBuffer::Stop)
			.property("Frequency", &my::SoundBuffer::GetFrequency, &my::SoundBuffer::SetFrequency)
			.property("Pan", &my::SoundBuffer::GetPan, &my::SoundBuffer::SetPan)
			.property("Volume", &my::SoundBuffer::GetVolume, &my::SoundBuffer::SetVolume)
			.property("Status", &my::SoundBuffer::GetStatus)

		, class_<my::Sound3DBuffer, boost::shared_ptr<my::Sound3DBuffer> >("Sound3DBuffer")
			.enum_("Apply")
			[
				value("DS3D_IMMEDIATE", DS3D_IMMEDIATE),
				value("DS3D_DEFERRED", DS3D_DEFERRED)
			]
			.def("GetMode", &my::Sound3DBuffer::GetMode)
			.def("SetMode", &my::Sound3DBuffer::SetMode)
			.def("GetMaxDistance", &my::Sound3DBuffer::GetMaxDistance)
			.def("SetMaxDistance", &my::Sound3DBuffer::SetMaxDistance)
			.def("GetMinDistance", &my::Sound3DBuffer::GetMinDistance)
			.def("SetMinDistance", &my::Sound3DBuffer::SetMinDistance)
			.def("GetPosition", &my::Sound3DBuffer::GetPosition)
			.def("SetPosition", &my::Sound3DBuffer::SetPosition)
			.def("GetVelocity", &my::Sound3DBuffer::GetVelocity)
			.def("SetVelocity", &my::Sound3DBuffer::SetVelocity)

		, class_<my::Sound3DListener, boost::shared_ptr<my::Sound3DListener> >("Sound3DListener")
			.def("CommitDeferredSettings", &my::Sound3DListener::CommitDeferredSettings)
			.def("GetDistanceFactor", &my::Sound3DListener::GetDistanceFactor)
			.def("SetDistanceFactor", &my::Sound3DListener::SetDistanceFactor)
			.def("GetDopplerFactor", &my::Sound3DListener::GetDopplerFactor)
			.def("SetDopplerFactor", &my::Sound3DListener::SetDopplerFactor)
			.def("GetRolloffFactor", &my::Sound3DListener::GetRolloffFactor)
			.def("SetRolloffFactor", &my::Sound3DListener::SetRolloffFactor)
			.def("GetOrientation", &my::Sound3DListener::GetOrientation, pure_out_value(boost::placeholders::_2) + pure_out_value(boost::placeholders::_3))
			.def("SetOrientation", &my::Sound3DListener::SetOrientation)
			.def("GetPosition", &my::Sound3DListener::GetPosition)
			.def("SetPosition", &my::Sound3DListener::SetPosition)
			.def("GetVelocity", &my::Sound3DListener::GetVelocity)
			.def("SetVelocity", &my::Sound3DListener::SetVelocity)

		, class_<my::Timer, boost::shared_ptr<my::Timer> >("Timer")
			.def(constructor<float>())
			.def_readonly("Interval", &my::Timer::m_Interval)
			.def_readwrite("RemainingTime", &my::Timer::m_RemainingTime)
	];

	module(m_State)[
		class_<Material, boost::shared_ptr<Material> >("Material")
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
			.enum_("PassMask")
			[
				value("PassMaskNone", Material::PassMaskNone),
				value("PassMaskShadow", Material::PassMaskShadow),
				value("PassMaskLight", Material::PassMaskLight),
				value("PassMaskBackground", Material::PassMaskBackground),
				value("PassMaskOpaque", Material::PassMaskOpaque),
				value("PassMaskNormalOpaque", Material::PassMaskNormalOpaque),
				value("PassMaskShadowNormalOpaque", Material::PassMaskShadowNormalOpaque),
				value("PassMaskTransparent", Material::PassMaskTransparent),
				value("PassMaskNormalTransparent", Material::PassMaskNormalTransparent)
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

		, class_<Component, my::NamedObject, ScriptComponent/*, boost::shared_ptr<Component>*/ >("Component")
			.def(constructor<const char *>())
			.def(const_self == other<const Component&>())
			.enum_("ComponentType")
			[
				value("ComponentTypeComponent", Component::ComponentTypeComponent),
				value("ComponentTypeActor", Component::ComponentTypeActor),
				value("ComponentTypeController", Component::ComponentTypeController),
				value("ComponentTypeMesh", Component::ComponentTypeMesh),
				value("ComponentTypeStaticMesh", Component::ComponentTypeStaticMesh),
				value("ComponentTypeCloth", Component::ComponentTypeCloth),
				value("ComponentTypeStaticEmitter", Component::ComponentTypeStaticEmitter),
				value("ComponentTypeSphericalEmitter", Component::ComponentTypeSphericalEmitter),
				value("ComponentTypeTerrain", Component::ComponentTypeTerrain),
				value("ComponentTypeAnimator", Component::ComponentTypeAnimator),
				value("ComponentTypeNavigation", Component::ComponentTypeNavigation),
				value("ComponentTypeSteering", Component::ComponentTypeSteering),
				value("ComponentTypeScript", Component::ComponentTypeScript)
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
			.def("OnSetShader", &Component::OnSetShader, &ScriptComponent::default_OnSetShader)
			.def("Update", &Component::Update, &ScriptComponent::default_Update)
			.def("OnPxThreadSubstep", &Component::OnPxThreadSubstep, &ScriptComponent::default_OnPxThreadSubstep)
			.enum_("PxPairFlag")
			[
				value("eNOTIFY_TOUCH_FOUND", physx::PxPairFlag::eNOTIFY_TOUCH_FOUND),
				value("eNOTIFY_TOUCH_LOST", physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			]
			.def("OnTrigger", &Component::OnTrigger, &ScriptComponent::default_OnTrigger)
			.def("OnContact", &Component::OnContact, &ScriptComponent::default_OnContact)
			.def("OnPxThreadShapeHit", &Component::OnPxThreadShapeHit, &ScriptComponent::default_OnPxThreadShapeHit)
			.def("OnPxThreadControllerHit", &Component::OnPxThreadControllerHit, &ScriptComponent::default_OnPxThreadControllerHit)
			.def("OnPxThreadObstacleHit", &Component::OnPxThreadObstacleHit, &ScriptComponent::default_OnPxThreadObstacleHit)
			.def("OnGUI", &Component::OnGUI, &ScriptComponent::default_OnGUI)
			.def("CalculateAABB", &Component::CalculateAABB)
			.def("AddToPipeline", &Component::AddToPipeline, &ScriptComponent::default_AddToPipeline)
			.property("Material", &Component::GetMaterial, &Component::SetMaterial)
			.def("CreateBoxShape", &Component::CreateBoxShape)
			.def("CreateCapsuleShape", &Component::CreateCapsuleShape)
			.def("CreatePlaneShape", &Component::CreatePlaneShape)
			.def("CreateSphereShape", &Component::CreateSphereShape)
			.property("SimulationFilterWord0", &Component::GetSimulationFilterWord0, &Component::SetSimulationFilterWord0)
			.property("QueryFilterWord0", &Component::GetQueryFilterWord0, &Component::SetQueryFilterWord0)
			.enum_("PxShapeFlag")
			[
				value("eSIMULATION_SHAPE", physx::PxShapeFlag::eSIMULATION_SHAPE),
				value("eSCENE_QUERY_SHAPE", physx::PxShapeFlag::eSCENE_QUERY_SHAPE),
				value("eTRIGGER_SHAPE", physx::PxShapeFlag::eTRIGGER_SHAPE),
				value("eVISUALIZATION", physx::PxShapeFlag::eVISUALIZATION),
				value("ePARTICLE_DRAIN", physx::PxShapeFlag::ePARTICLE_DRAIN)
			]
			.property("ShapeFlags", &Component::GetShapeFlags, &Component::SetShapeFlags)
			.enum_("PxGeometryType")
			[
				value("eSPHERE", physx::PxGeometryType::eSPHERE),
				value("ePLANE", physx::PxGeometryType::ePLANE),
				value("eCAPSULE", physx::PxGeometryType::eCAPSULE),
				value("eBOX", physx::PxGeometryType::eBOX),
				value("eCONVEXMESH", physx::PxGeometryType::eCONVEXMESH),
				value("eTRIANGLEMESH", physx::PxGeometryType::eTRIANGLEMESH),
				value("eHEIGHTFIELD", physx::PxGeometryType::eHEIGHTFIELD),
				value("eINVALID", physx::PxGeometryType::eINVALID)
			]
			.property("GeometryType", &Component::GetGeometryType)
			.property("ShapeLocalPose", &Component::GetShapeLocalPose, &Component::SetShapeLocalPose)
			.def("ClearShape", &Component::ClearShape)
			.property("SiblingId", &Component::GetSiblingId, &Component::SetSiblingId)

		, class_<MeshComponent, Component, boost::shared_ptr<Component> >("MeshComponent")
			.def(constructor<const char *>())
			.def_readwrite("MeshPath", &MeshComponent::m_MeshPath)
			.def_readwrite("MeshSubMeshId", &MeshComponent::m_MeshSubMeshId)
			.def_readonly("Mesh", &MeshComponent::m_Mesh)
			.def_readwrite("MeshColor", &MeshComponent::m_MeshColor)
			.enum_("InstanceType")
			[
				value("InstanceTypeNone", MeshComponent::InstanceTypeNone),
				value("InstanceTypeInstance", MeshComponent::InstanceTypeInstance),
				value("InstanceTypeBatch", MeshComponent::InstanceTypeBatch)
			]
			.def_readwrite("InstanceType", &MeshComponent::m_InstanceType)
			.def_readonly("PxMeshPath", &MeshComponent::m_PxMeshPath)
			.def("CreateTriangleMeshShape", &MeshComponent::CreateTriangleMeshShape)
			.def("CreateConvexMeshShape", &MeshComponent::CreateConvexMeshShape)

		, class_<StaticMesh, bases<MeshComponent, my::AABB>, boost::shared_ptr<Component> >("StaticMesh")
			.def(constructor<const char *, const my::AABB &, float>())
			.def_readonly("ChunkWidth", &StaticMesh::m_ChunkWidth)
			.def_readwrite("ChunkLodScale", &StaticMesh::m_ChunkLodScale)
			.def("AddChunk", &StaticMesh::AddChunk)

		, class_<ClothComponent, Component, boost::shared_ptr<Component> >("ClothComponent")
			.def(constructor<const char *>())
			.def("CreateClothFromMesh", &ClothComponent::CreateClothFromMesh)
			.def("CreateVirtualParticles", &ClothComponent::CreateVirtualParticles)
			.enum_("PxClothFlag")
			[
				value("eDEFAULT", physx::PxClothFlag::Enum::eDEFAULT),
				value("eCUDA", physx::PxClothFlag::Enum::eCUDA),
				value("eGPU", physx::PxClothFlag::Enum::eGPU),
				value("eSWEPT_CONTACT", physx::PxClothFlag::Enum::eSWEPT_CONTACT),
				value("eSCENE_COLLISION", physx::PxClothFlag::Enum::eSCENE_COLLISION)
			]
			.property("ClothFlags", &ClothComponent::GetClothFlags, &ClothComponent::SetClothFlags)
			.property("ExternalAcceleration", &ClothComponent::GetExternalAcceleration, &ClothComponent::SetExternalAcceleration)
			.enum_("PxClothFabricPhaseType")
			[
				value("eVERTICAL", physx::PxClothFabricPhaseType::eVERTICAL),
				value("eHORIZONTAL", physx::PxClothFabricPhaseType::eHORIZONTAL),
				value("eBENDING", physx::PxClothFabricPhaseType::eBENDING),
				value("eSHEARING", physx::PxClothFabricPhaseType::eSHEARING)
			]
			.def("SetStretch", &cloth_component_set_stretch)
			.def("GetStretch", &cloth_component_get_stretch, pure_out_value(boost::placeholders::_3) + pure_out_value(boost::placeholders::_4) + pure_out_value(boost::placeholders::_5) + pure_out_value(boost::placeholders::_6))
			.def("SetTether", &cloth_component_set_tether)
			.def("GetTether", &cloth_component_get_tether, pure_out_value(boost::placeholders::_3) + pure_out_value(boost::placeholders::_4))
			.def("AddCollisionSphere", &cloth_component_add_collision_sphere)
			.def("AddCollisionCapsule", &cloth_component_add_collision_capsule)

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

		, class_<StaticEmitter, bases<EmitterComponent, my::AABB>, boost::shared_ptr<Component> >("StaticEmitter")
			.def(constructor<const char *, const my::AABB &, float, EmitterComponent::FaceType, EmitterComponent::SpaceType, EmitterComponent::VelocityType, StaticEmitter::PrimitiveType>())
			.enum_("PrimitiveType")
			[
				value("PrimitiveTypeTri", StaticEmitter::PrimitiveTypeTri),
				value("PrimitiveTypeQuad", StaticEmitter::PrimitiveTypeQuad),
				value("PrimitiveTypeMesh", StaticEmitter::PrimitiveTypeMesh)
			]
			.def_readwrite("EmitterPrimitiveType", &StaticEmitter::m_EmitterPrimitiveType)
			.def_readwrite("MeshPath", &StaticEmitter::m_MeshPath)
			.def_readwrite("MeshSubMeshId", &StaticEmitter::m_MeshSubMeshId)
			.def_readonly("ChunkWidth", &StaticEmitter::m_ChunkWidth)
			.def_readwrite("ChunkPath", &StaticEmitter::m_ChunkPath)
			.def_readwrite("ChunkLodScale", &StaticEmitter::m_ChunkLodScale)
			.property("ParticleList", &static_emitter_get_particle_list, return_stl_iterator + dependency(result, boost::placeholders::_1))

		, class_<StaticEmitterStream>("StaticEmitterStream")
			.def(constructor<StaticEmitter *>())
			.def("Flush", &StaticEmitterStream::Flush)
			.def("Spawn", &StaticEmitterStream::Spawn)

		, class_<CircularEmitter, EmitterComponent, boost::shared_ptr<Component> >("CircularEmitter")
			.def(constructor<const char*, unsigned int, EmitterComponent::FaceType, EmitterComponent::SpaceType, EmitterComponent::VelocityType>())
			.def_readonly("ParticleList", &CircularEmitter::m_ParticleList, return_stl_iterator)
			.property("Capacity", &CircularEmitter::GetCapacity, &CircularEmitter::SetCapacity)
			.def("Spawn", &CircularEmitter::Spawn)
			.def("RemoveAllParticle", &CircularEmitter::RemoveAllParticle)

		, class_<SphericalEmitter, CircularEmitter, boost::shared_ptr<Component> >("SphericalEmitter")
			.def(constructor<const char *, unsigned int, EmitterComponent::FaceType, EmitterComponent::SpaceType, EmitterComponent::VelocityType>())
			.def_readwrite("ParticleLifeTime", &SphericalEmitter::m_ParticleLifeTime)
			.def_readwrite("SpawnInterval", &SphericalEmitter::m_SpawnInterval)
			.def_readwrite("HalfSpawnArea", &SphericalEmitter::m_HalfSpawnArea)
			.def_readwrite("SpawnInclination", &SphericalEmitter::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &SphericalEmitter::m_SpawnAzimuth)
			.def_readwrite("SpawnSpeed", &SphericalEmitter::m_SpawnSpeed)
			.def_readwrite("SpawnColorR", &SphericalEmitter::m_SpawnColorR)
			.def_readwrite("SpawnColorG", &SphericalEmitter::m_SpawnColorG)
			.def_readwrite("SpawnColorB", &SphericalEmitter::m_SpawnColorB)
			.def_readwrite("SpawnColorA", &SphericalEmitter::m_SpawnColorA)
			.def_readwrite("SpawnSizeX", &SphericalEmitter::m_SpawnSizeX)
			.def_readwrite("SpawnSizeY", &SphericalEmitter::m_SpawnSizeY)
			.def_readwrite("SpawnAngle", &SphericalEmitter::m_SpawnAngle)
			.def_readwrite("SpawnCycle", &SphericalEmitter::m_SpawnCycle)
			.def_readwrite("SpawnBoneId", &SphericalEmitter::m_SpawnBoneId)
			.def_readwrite("SpawnLocalPose", &SphericalEmitter::m_SpawnLocalPose)

		, class_<TerrainChunk, my::OctEntity>("TerrainChunk")
			.def_readonly("Row", &TerrainChunk::m_Row)
			.def_readonly("Col", &TerrainChunk::m_Col)

		, class_<Terrain, bases<Component, my::AABB>, boost::shared_ptr<Component> >("Terrain")
			.def(constructor<const char *, int, int, int, int>())
			.def_readonly("RowChunks", &Terrain::m_RowChunks)
			.def_readonly("ColChunks", &Terrain::m_ColChunks)
			.def_readonly("ChunkSize", &Terrain::m_ChunkSize)
			.def_readonly("MinChunkLodSize", &Terrain::m_MinChunkLodSize)
			.def_readwrite("ChunkPath", &Terrain::m_ChunkPath)
			.def_readwrite("ChunkLodScale", &Terrain::m_ChunkLodScale)
			//.def_readonly("Chunks", &Terrain::m_Chunks, return_stl_iterator)
			.def("CreateHeightFieldShape", &Terrain::CreateHeightFieldShape)
			.def("RayTest", &Terrain::RayTest, pure_out_value(boost::placeholders::_4))
			.def("RayTest2D", &Terrain::RayTest2D)
			.def("GetIndices", &TerrainStream::GetIndices, pure_out_value(boost::placeholders::_4) + pure_out_value(boost::placeholders::_5) + pure_out_value(boost::placeholders::_6) + pure_out_value(boost::placeholders::_7) + pure_out_value(boost::placeholders::_8) + pure_out_value(boost::placeholders::_9))

		, class_<TerrainStream>("TerrainStream")
			.def(constructor<Terrain *>())
			.def("Flush", &TerrainStream::Flush)
			.def("GetPos", &TerrainStream::GetPos)
			.def("SetPos", (void(TerrainStream::*)(int, int, const my::Vector3&))&TerrainStream::SetPos)
			.def("SetPos", (void(TerrainStream::*)(int, int, float))&TerrainStream::SetPos)
			.def("GetColor", &TerrainStream::GetColor)
			.def("SetColor", (void(TerrainStream::*)(int, int, D3DCOLOR))&TerrainStream::SetColor)
			.def("GetNormal", &TerrainStream::GetNormal)
			.def("SetNormal", (void(TerrainStream::*)(int, int, const my::Vector3&))&TerrainStream::SetNormal)
			//.def("RayTest", &TerrainStream::RayTest)
			.def("RayTest2D", &TerrainStream::RayTest2D)

		, class_<Controller, Component, boost::shared_ptr<Component> >("Controller")
			.enum_("CollisionFlag")
			[
				value("eCOLLISION_SIDES", physx::PxControllerCollisionFlag::eCOLLISION_SIDES),
				value("eCOLLISION_UP", physx::PxControllerCollisionFlag::eCOLLISION_UP),
				value("eCOLLISION_DOWN", physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			]
			.def(constructor<const char*, float, float, float, float, float>())
			.def("Move", &Controller::Move)
			.property("Height", &Controller::GetHeight, &Controller::SetHeight)
			.property("Radius", &Controller::GetRadius, &Controller::SetRadius)
			.property("StepOffset", &Controller::GetStepOffset, &Controller::SetStepOffset)
			.property("ContactOffset", &Controller::GetContactOffset, &Controller::SetContactOffset)
			.property("SlopeLimit", &Controller::GetSlopeLimit, &Controller::SetSlopeLimit)
			.property("UpDirection", &Controller::GetUpDirection, &Controller::SetUpDirection)
			.property("Position", &Controller::GetPosition, &Controller::SetPosition)
			.property("FootOffset", &Controller::GetFootOffset)
			.property("FootPosition", &Controller::GetFootPosition, &Controller::SetFootPosition)
			.property("ContactNormalDownPass", &Controller::GetContactNormalDownPass)
			.property("ContactNormalSidePass", &Controller::GetContactNormalSidePass)
			.property("GeomStream", &controller_get_geom_stream, return_stl_iterator)
			.property("TouchedComponent", &Controller::GetTouchedComponent)
			.property("TouchedPosWorld", &Controller::GetTouchedPosWorld)
			.property("TouchedPosLocal", &Controller::GetTouchedPosLocal)

		, class_<Navigation, Component, boost::shared_ptr<Component> >("Navigation")

		, class_<Steering, Component, boost::shared_ptr<Component> >("Steering")
			.enum_("CrowdAgentState")
			[
				value("DT_CROWDAGENT_STATE_INVALID", Steering::DT_CROWDAGENT_STATE_INVALID),
				value("DT_CROWDAGENT_STATE_WALKING", Steering::DT_CROWDAGENT_STATE_WALKING),
				value("DT_CROWDAGENT_STATE_OFFMESH", Steering::DT_CROWDAGENT_STATE_OFFMESH)
			]
			.def(constructor<const char*, float, float, float, Navigation *>())
			.def_readwrite("Forward", &Steering::m_Forward)
			.def_readwrite("Speed", &Steering::m_Speed)
			.def_readwrite("MaxSpeed", &Steering::m_MaxSpeed)
			.def_readwrite("BrakingSpeed", &Steering::m_BrakingSpeed)
			.def_readwrite("MaxAdjustedSpeed", &Steering::m_MaxAdjustedSpeed)
			.def_readonly("agentPos", &Steering::m_agentPos)
			.def_readonly("targetPos", &Steering::m_targetPos)
			.def_readonly("targetRef", &Steering::m_targetRef)
			.def_readonly("targetRefPos", &Steering::m_targetRefPos)
			.def_readonly("ncorners", &Steering::m_ncorners)
			.def("SeekDir", &Steering::SeekDir)
			.def("SeekTarget", &Steering::SeekTarget, pure_out_value(boost::placeholders::_5) + pure_out_value(boost::placeholders::_6) + pure_out_value(boost::placeholders::_7))
			.def("GetCorner", &steering_get_corner)

		, class_<ActorEventArg, my::EventArg>("ActorEventArg")
			.def_readonly("self", &ActorEventArg::self)
			.def_readonly("self_cmp", &ActorEventArg::self_cmp)

		, class_<TriggerEventArg, ActorEventArg>("TriggerEventArg")
			.def_readonly("other", &TriggerEventArg::other)
			.def_readonly("other_cmp", &TriggerEventArg::other_cmp)
			.def_readonly("events", &TriggerEventArg::events)

		, class_<ContactEventArg, TriggerEventArg>("ContactEventArg")
			.def_readonly("position", &ContactEventArg::position)
			.def_readonly("separation", &ContactEventArg::separation)
			.def_readonly("normal", &ContactEventArg::normal)
			.def_readonly("impulse", &ContactEventArg::impulse)

		, class_<ControllerEventArg, ActorEventArg>("ControllerEventArg")
			.def_readonly("worldPos", &ControllerEventArg::worldPos)
			.def_readonly("worldNormal", &ControllerEventArg::worldNormal)
			.def_readonly("dir", &ControllerEventArg::dir)
			.def_readonly("length", &ControllerEventArg::length)
			.def_readonly("flag", &ControllerEventArg::flag)

		, class_<ShapeHitEventArg, ControllerEventArg>("ShapeHitEventArg")
			.def_readonly("other", &ShapeHitEventArg::other)
			.def_readonly("other_cmp", &ShapeHitEventArg::other_cmp)
			.def_readonly("triangleIndex", &ShapeHitEventArg::triangleIndex)

		, class_<ControllerHitEventArg, ControllerEventArg>("ControllerHitEventArg")
			.def_readonly("other", &ControllerHitEventArg::other)
			.def_readonly("other_cmp", &ControllerHitEventArg::other_cmp)

		, class_<Actor, bases<my::NamedObject, my::OctEntity>, boost::shared_ptr<Actor> >("Actor")
			.def(constructor<const char *, const my::Vector3 &, const my::Quaternion &, const my::Vector3 &, const my::AABB &>())
			.def(const_self == other<const Actor&>())
			.def_readwrite("aabb", &Actor::m_aabb)
			.def_readwrite("Position", &Actor::m_Position)
			.def_readwrite("Rotation", &Actor::m_Rotation)
			.def_readwrite("Scale", &Actor::m_Scale)
			.def_readonly("World", &Actor::m_World)
			.def_readwrite("LodDist", &Actor::m_LodDist)
			.def_readwrite("LodFactor", &Actor::m_LodFactor)
			.def_readwrite("CullingDistSq", &Actor::m_CullingDistSq)
			.def_readonly("Cmps", &Actor::m_Cmps, return_stl_iterator)
			.def_readonly("Base", &Actor::m_Base)
			.def_readwrite("BaseBoneId", &Actor::m_BaseBoneId)
			.property("Requested", &Actor::IsRequested)
			.def("Clone", &Actor::Clone)
			.def("RequestResource", &Actor::RequestResource)
			.def("ReleaseResource", &Actor::ReleaseResource)
			.def("SetPose", (void (Actor::*)(const my::Vector3&))& Actor::SetPose)
			.def("SetPose", (void (Actor::*)(const my::Vector3&, const my::Quaternion&))& Actor::SetPose)
			.def("SetPose", (void (Actor::*)(const my::Bone&))& Actor::SetPose)
			.def("SetPxPoseOrbyPxThread", (void (Actor::*)(const my::Vector3&))& Actor::SetPxPoseOrbyPxThread)
			.def("SetPxPoseOrbyPxThread", (void (Actor::*)(const my::Vector3&, const my::Quaternion&, const Component *))& Actor::SetPxPoseOrbyPxThread)
			.def("SetPxPoseOrbyPxThread", (void (Actor::*)(const my::Bone&))& Actor::SetPxPoseOrbyPxThread)
			.def("UpdateAABB", &Actor::UpdateAABB)
			.def("UpdateWorld", &Actor::UpdateWorld)
			.def("UpdateOctNode", &Actor::UpdateOctNode)
			.def("ClearRigidActor", &Actor::ClearRigidActor)
			.enum_("ActorType")
			[
				value("eRIGID_STATIC", physx::PxActorType::eRIGID_STATIC),
				value("eRIGID_DYNAMIC", physx::PxActorType::eRIGID_DYNAMIC),
				value("eACTOR_COUNT", physx::PxActorType::eACTOR_COUNT)
			]
			.def("CreateRigidActor", &Actor::CreateRigidActor)
			.property("RigidActorType", &Actor::GetRigidActorType)
			.enum_("RigidBodyFlag")
			[
				value("eKINEMATIC", physx::PxRigidBodyFlag::eKINEMATIC),
				value("eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES", physx::PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES),
				value("eENABLE_CCD", physx::PxRigidBodyFlag::eENABLE_CCD),
				value("eENABLE_CCD_FRICTION", physx::PxRigidBodyFlag::eENABLE_CCD_FRICTION)
			]
			.def("SetRigidBodyFlag", &Actor::SetRigidBodyFlag)
			.def("GetRigidBodyFlag", &Actor::GetRigidBodyFlag)
			.property("Mass", &Actor::GetMass, &Actor::SetMass)
			.property("CMassLocalPose", &Actor::GetCMassLocalPose, &Actor::SetCMassLocalPose)
			.property("MassSpaceInertiaTensor", &Actor::GetMassSpaceInertiaTensor, &Actor::SetMassSpaceInertiaTensor)
			.def("UpdateMassAndInertia", &Actor::UpdateMassAndInertia)
			.property("LinearVelocity", &Actor::GetLinearVelocity, &Actor::SetLinearVelocity)
			.property("AngularVelocity", &Actor::GetAngularVelocity, &Actor::SetAngularVelocity)
			.property("Sleeping", &Actor::IsSleeping)
			.def("WakeUp", &Actor::WakeUp)
			.def("InsertComponent", (void (Actor::*)(ComponentPtr))& Actor::InsertComponent)
			.def("InsertComponent", (void (Actor::*)(unsigned int i, ComponentPtr))& Actor::InsertComponent)
			.def("InsertComponentAdopt", (void(*)(Actor*, ScriptComponent*))& actor_insert_component_adopt, adopt(boost::placeholders::_2))
			.def("InsertComponentAdopt", (void(*)(Actor*, unsigned int i, ScriptComponent*))& actor_insert_component_adopt, adopt(boost::placeholders::_3))
			.def("RemoveComponent", &Actor::RemoveComponent)
			.property("ComponentNum", &Actor::GetComponentNum)
			.def("ClearAllComponent", &Actor::ClearAllComponent)
			.def("Attach", &Actor::Attach)
			.def("Detach", &Actor::Detach)
			.property("AttachNum", &Actor::GetAttachNum)
			.def("GetAttacher", &actor_get_attacher)
			.def("GetAttachPose", &Actor::GetAttachPose)
			.def("ClearAllAttach", &Actor::ClearAllAttach)
			.property("TopBase", &Actor::GetTopBase)
			.def("AddRevoluteJoint", &Actor::AddRevoluteJoint)
			.def("AddD6Joint", &Actor::AddD6Joint)
			.def("PlayAction", &Actor::PlayAction)
			.def("PlayAction", luabind::tag_function<boost::shared_ptr<ActionInst>(Actor*, Action*)>(
				boost::bind(&Actor::PlayAction, boost::placeholders::_1, boost::placeholders::_2, 1.0f)))
			.def("StopActionInst", &Actor::StopActionInst)
			.def("StopAllActionInst", &Actor::StopAllActionInst)
			.def("TickActionAndGetDisplacement", &Actor::TickActionAndGetDisplacement, pure_out_value(boost::placeholders::_3))
			.def("GetFirstComponent", (Component * (Actor::*)(DWORD, unsigned int))&Actor::GetFirstComponent)
			.def("GetFirstComponent", luabind::tag_function<Component* (Actor*, DWORD)>(
				boost::bind<Component*>((Component* (Actor::*)(DWORD, unsigned int))&Actor::GetFirstComponent, boost::placeholders::_1, boost::placeholders::_2, 0)))

		//, def("act2entity", (boost::shared_ptr<my::OctEntity>(*)(const boost::shared_ptr<Actor>&))& boost::static_pointer_cast<my::OctEntity, Actor>)

		, class_<AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNode")
			.def_readonly("Parent", &AnimationNode::m_Parent)
			.def_readonly("Name", &AnimationNode::m_Name)
			.property("Child0", &animation_node_get_child<0>, &animation_node_set_child<0>)
			.property("Child1", &animation_node_get_child<1>, &animation_node_set_child<1>)
			.property("Child2", &animation_node_get_child<2>, &animation_node_set_child<2>)
			.property("Child3", &animation_node_get_child<3>, &animation_node_set_child<3>)
			.property("Child4", &animation_node_get_child<4>, &animation_node_set_child<4>)
			.property("Child5", &animation_node_get_child<5>, &animation_node_set_child<5>)
			.def("SetChildAdopt", &animation_node_set_child_adopt, adopt(boost::placeholders::_3))
			.property("ChildNum", luabind::tag_function<size_t (const AnimationNode*)>(
				boost::bind(&AnimationNode::AnimationNodePtrList::size, boost::bind(&AnimationNode::m_Childs, boost::placeholders::_1))))
			.def("RemoveChild", &AnimationNode::RemoveChild)
			.def("FindSubNode", (AnimationNode* (AnimationNode::*)(const std::string&))& AnimationNode::FindSubNode)

		, class_<AnimationNodeSequence, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeSequence")
			.def(constructor<const char*>())
			.def(constructor<const char*, float>())
			.def(constructor<const char*, float, bool>())
			.def(constructor<const char*, float, bool, const char*>())
			.def_readwrite("Name", &AnimationNodeSequence::m_Name)
			.def_readwrite("Rate", &AnimationNodeSequence::m_Rate)
			.def_readwrite("Loop", &AnimationNodeSequence::m_Loop)
			.def_readwrite("Group", &AnimationNodeSequence::m_Group)

		, class_<AnimationNodeSlot, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeSlot")
			.def(constructor<const char*>())
			//.property("IsPlaying", &AnimationNodeSlot::IsPlaying)
			//.def("Stop", &AnimationNodeSlot::Stop)

		, class_<AnimationNodeSubTree, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeSubTree")
			.def(constructor<const char*>())
			.def_readwrite("NodeId", &AnimationNodeSubTree::m_NodeId)

		, class_<AnimationNodeBlendList, AnimationNode, ScriptAnimationNodeBlendList/*, boost::shared_ptr<AnimationNode>*/ >("AnimationNodeBlendList")
			.def(constructor<const char*, unsigned int>())
			.def_readwrite("BlendTime", &AnimationNodeBlendList::m_BlendTime)
			.def("SetTargetWeight", (void (AnimationNodeBlendList::*)(int, float))&AnimationNodeBlendList::SetTargetWeight)
			.def("SetTargetWeight", (void (AnimationNodeBlendList::*)(int, float, bool))&AnimationNodeBlendList::SetTargetWeight)
			.def("GetTargetWeight", &AnimationNodeBlendList::GetTargetWeight)
			.def("GetWeight", luabind::tag_function<float(const AnimationNodeBlendList*, int)>(
				boost::bind((const float& (std::vector<float>::*)(size_t) const)& std::vector<float>::at, boost::bind(&AnimationNodeBlendList::m_Weight, boost::placeholders::_1), boost::placeholders::_2)))
			.def("SetActiveChild", &AnimationNodeBlendList::SetActiveChild)
			.def("GetActiveChild", &AnimationNodeBlendList::GetActiveChild)
			.def("Tick", &AnimationNodeBlendList::Tick, &ScriptAnimationNodeBlendList::default_Tick)

		, class_<AnimationNodeRate, AnimationNode, boost::shared_ptr<AnimationNode> >("AnimationNodeRate")
			.def(constructor<const char*>())
			.def_readwrite("Rate", &AnimationNodeRate::m_Rate)

		, class_<AnimationEventArg, my::EventArg>("AnimationEventArg")
			.def_readonly("self", &AnimationEventArg::self)

		, class_<Animator, bases<Component, AnimationNode>, boost::shared_ptr<Component> >("Animator") // ! luabind::bases for accessing AnimationNodeSlot properties from boost::shared_ptr<Component>
			.def(constructor<const char*>())
			.def_readwrite("SkeletonPath", &Animator::m_SkeletonPath)
			.def_readonly("Skeleton", &Animator::m_Skeleton)
			.def_readonly("anim_pose_hier", &Animator::anim_pose_hier, return_stl_iterator)
			.def("ReloadSequenceGroup", &Animator::ReloadSequenceGroup)
			.def("AddDynamicBone", &Animator::AddDynamicBone)
			.def("AddIK", &Animator::AddIK)
			.def("DrawDebugBone", &Animator::DrawDebugBone)
			.def("GetBone", &animator_get_bone)

		, class_<LargeImage/*, my::ControlImage*/, boost::shared_ptr<LargeImage> >("LargeImage")
			.def(constructor<>())
			.def_readwrite("TexturePath", &LargeImage::m_TexturePath)
			.def("RequestResource", &LargeImage::RequestResource)
			.def("ReleaseResource", &LargeImage::ReleaseResource)
			//.def("Draw", (void (LargeImage::*)(my::UIRender*, const my::Rectangle&, DWORD))& LargeImage::Draw)
			.def("Draw", (void (LargeImage::*)(my::UIRender*, const my::Rectangle&, DWORD, const my::Rectangle&, int))& LargeImage::Draw)

		, class_<Action, boost::shared_ptr<Action> >("Action")
			.def(constructor<float>())
			.def(const_self == other<const Action&>())
			.def_readwrite("Length", &Action::m_Length)
			.def("AddTrack", &Action::AddTrack)
			.def("RemoveTrack", &Action::RemoveTrack)

		, class_<ActionInst, boost::shared_ptr<ActionInst> >("ActionInst")
			.def_readonly("Template", &ActionInst::m_Template)
			.def_readonly("LastTime", &ActionInst::m_LastTime)
			.def_readonly("Time", &ActionInst::m_Time)
			.def_readwrite("Rate", &ActionInst::m_Rate)
			.property("Actor", luabind::tag_function<Actor*(ActionInst*)>(
				boost::bind(&ActionTrackInst::m_Actor, boost::bind(&ActionTrackInstPtr::get,
					boost::bind((ActionTrackInstPtr&(ActionInst::ActionTrackInstPtrList::*)())&ActionInst::ActionTrackInstPtrList::front,
						boost::bind<ActionInst::ActionTrackInstPtrList&>(&ActionInst::m_TrackInstList, boost::placeholders::_1))))))

		, class_<ActionTrack, boost::shared_ptr<ActionTrack> >("ActionTrack")

		, class_<ActionTrackAnimation, ActionTrack, boost::shared_ptr<ActionTrack> >("ActionTrackAnimation")
			.def(constructor<>())
			.def("AddKeyFrame", &ActionTrackAnimation::AddKeyFrame)

		, class_<ActionTrackSound, ActionTrack, boost::shared_ptr<ActionTrack> >("ActionTrackSound")
			.def(constructor<>())
			.def("AddKeyFrame", &ActionTrackSound::AddKeyFrame)

		, class_<ActionTrackEmitter, ActionTrack, boost::shared_ptr<ActionTrack> >("ActionTrackEmitter")
			.def(constructor<>())
			.def("AddKeyFrame", &ActionTrackEmitter::AddKeyFrame)
			.def_readwrite("EmitterMaterial", &ActionTrackEmitter::m_EmitterMaterial)
			.def_readwrite("EmitterCapacity", &ActionTrackEmitter::m_EmitterCapacity)
			.def_readwrite("EmitterFaceType", &ActionTrackEmitter::m_EmitterFaceType)
			.def_readwrite("EmitterSpaceType", &ActionTrackEmitter::m_EmitterSpaceType)
			.def_readwrite("SpawnArea", &ActionTrackEmitter::m_SpawnArea)
			.def_readwrite("SpawnInclination", &ActionTrackEmitter::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &ActionTrackEmitter::m_SpawnAzimuth)
			.def_readwrite("SpawnSpeed", &ActionTrackEmitter::m_SpawnSpeed)
			.def_readwrite("SpawnBoneId", &ActionTrackEmitter::m_SpawnBoneId)
			.def_readwrite("SpawnLocalPose", &ActionTrackEmitter::m_SpawnLocalPose)
			.def_readwrite("ParticleLifeTime", &ActionTrackEmitter::m_ParticleLifeTime)
			.def_readwrite("ParticleDamping", &ActionTrackEmitter::m_ParticleDamping)
			.def_readwrite("ParticleColorR", &ActionTrackEmitter::m_ParticleColorR)
			.def_readwrite("ParticleColorG", &ActionTrackEmitter::m_ParticleColorG)
			.def_readwrite("ParticleColorB", &ActionTrackEmitter::m_ParticleColorB)
			.def_readwrite("ParticleColorA", &ActionTrackEmitter::m_ParticleColorA)
			.def_readwrite("ParticleSizeX", &ActionTrackEmitter::m_ParticleSizeX)
			.def_readwrite("ParticleSizeY", &ActionTrackEmitter::m_ParticleSizeY)
			.def_readwrite("ParticleAngle", &ActionTrackEmitter::m_ParticleAngle)

		, class_<ActionTrackVelocity, ActionTrack, boost::shared_ptr<ActionTrack> >("ActionTrackVelocity")
			.def(constructor<>())
			.def_readwrite("ParamVelocity", &ActionTrackVelocity::m_ParamVelocity)
			.def("AddKeyFrame", &ActionTrackVelocity::AddKeyFrame)

		, class_<ActionTrackPose, ActionTrack, boost::shared_ptr<ActionTrack> >("ActionTrackPose")
			.def(constructor<>())
			.def_readwrite("ParamPose", &ActionTrackPose::m_ParamPose)
			.def_readwrite("Interpolation", &ActionTrackPose::m_Interpolation)
			.def("AddKeyFrame", &ActionTrackPose::AddKeyFrame)

		, class_<ScriptActionTrack, ActionTrack, boost::shared_ptr<ActionTrack> >("ScriptActionTrack")
			.def(constructor<const luabind::object &>())

		, class_<ActionTrackInst, ScriptActionTrackInst>("ActionTrackInst")
			.def(constructor<Actor*>())
			.def_readonly("Actor", &ActionTrackInst::m_Actor)
			.def("UpdateTime", &ActionTrackInst::UpdateTime, &ScriptActionTrackInst::default_UpdateTime)
			.def("Stop", &ActionTrackInst::Stop, &ScriptActionTrackInst::default_Stop)
			.def("GetDisplacement", &ActionTrackInst::GetDisplacement, &ScriptActionTrackInst::default_GetDisplacement)

		, class_<RenderPipeline>("RenderPipeline")
			//.enum_("MeshType")
			//[
			//	luabind::value("MeshTypeMesh", RenderPipeline::MeshTypeMesh),
			//	luabind::value("MeshTypeParticle", RenderPipeline::MeshTypeParticle),
			//	luabind::value("MeshTypeTerrain", RenderPipeline::MeshTypeTerrain),
			//	luabind::value("MeshTypeNum", RenderPipeline::MeshTypeNum)
			//]
			.enum_("PassType")
			[
				luabind::value("PassTypeShadow", RenderPipeline::PassTypeShadow),
				luabind::value("PassTypeNormal", RenderPipeline::PassTypeNormal),
				luabind::value("PassTypeNormalTransparent", RenderPipeline::PassTypeNormalTransparent),
				luabind::value("PassTypeLight", RenderPipeline::PassTypeLight),
				luabind::value("PassTypeBackground", RenderPipeline::PassTypeBackground),
				luabind::value("PassTypeOpaque", RenderPipeline::PassTypeOpaque),
				luabind::value("PassTypeTransparent", RenderPipeline::PassTypeTransparent),
				luabind::value("PassTypeNum", RenderPipeline::PassTypeNum)
			]
			.def_readonly("SHADOW_MAP_SIZE", &RenderPipeline::SHADOW_MAP_SIZE)
			.def_readwrite("SHADOW_BIAS", &RenderPipeline::SHADOW_BIAS)
			.def_readonly("SkyLightCam", &RenderPipeline::m_SkyLightCam)
			.def_readwrite("SkyLightColor", &RenderPipeline::m_SkyLightColor)
			.def_readwrite("AmbientColor", &RenderPipeline::m_AmbientColor)
			.def_readwrite("SsaoBias", &RenderPipeline::m_SsaoBias)
			.def_readwrite("SsaoIntensity", &RenderPipeline::m_SsaoIntensity)
			.def_readwrite("SsaoRadius", &RenderPipeline::m_SsaoRadius)
			.def_readwrite("SsaoScale", &RenderPipeline::m_SsaoScale)
			.def_readwrite("FogColor", &RenderPipeline::m_FogColor)
			.def_readwrite("FogStartDistance", &RenderPipeline::m_FogStartDistance)
			.def_readwrite("FogHeight", &RenderPipeline::m_FogHeight)
			.def_readwrite("FogFalloff", &RenderPipeline::m_FogFalloff)
			.def_readwrite("DofParams", &RenderPipeline::m_DofParams)
			.def_readwrite("LuminanceThreshold", &RenderPipeline::m_LuminanceThreshold)
			.def_readwrite("BloomColor", &RenderPipeline::m_BloomColor)
			.def_readwrite("BloomFactor", &RenderPipeline::m_BloomFactor)
			.enum_("RenderTargetType")
			[
				luabind::value("RenderTargetNormal", RenderPipeline::RenderTargetNormal),
				luabind::value("RenderTargetPosition", RenderPipeline::RenderTargetPosition),
				luabind::value("RenderTargetLight", RenderPipeline::RenderTargetLight),
				luabind::value("RenderTargetOpaque", RenderPipeline::RenderTargetOpaque),
				luabind::value("RenderTargetDownFilter", RenderPipeline::RenderTargetDownFilter)
			]
			.def("QueryShader", &renderpipeline_query_shader)
			.def("PushMesh", &RenderPipeline::PushMesh)

		, class_<HitArg, my::EventArg>("HitArg")
			.def_readonly("actor", &HitArg::actor)
			.def_readonly("cmp", &HitArg::cmp)

		, class_<OverlapHitArg, HitArg>("OverlapHitArg")
			.def_readonly("faceIndex", &OverlapHitArg::faceIndex)

		, class_<SweepHitArg, OverlapHitArg>("SweepHitArg")
			.def_readonly("position", &SweepHitArg::position)
			.def_readonly("normal", &SweepHitArg::normal)
			.def_readonly("distance", &SweepHitArg::distance)

		, class_<RaycastHitArg, SweepHitArg>("RaycastHitArg")
			.def_readonly("u", &RaycastHitArg::u)
			.def_readonly("v", &RaycastHitArg::v)

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
			.property("Gravity", &PhysxScene::GetGravity, &PhysxScene::SetGravity)
			.def("Raycast", &physxscene_raycast)
			.def("OverlapBox", &physxscene_overlap_box)
			.def("OverlapSphere", &physxscene_overlap_sphere)
			.def("SweepBox", &physxscene_sweep_box)
			.def("SweepSphere", &physxscene_sweep_sphere)

		, class_<SoundEvent, boost::shared_ptr<SoundEvent> >("SoundEvent")
			.def_readonly("sbuffer", &SoundEvent::m_sbuffer)
			.def_readonly("3dbuffer", &SoundEvent::m_3dbuffer)

		, class_<Mp3, boost::shared_ptr<Mp3> >("Mp3")
			.def(constructor<>())
			.def("Play", &Mp3::Play)
			.def("StopAsync", &Mp3::StopAsync)
			.def("Stop", &Mp3::Stop)

		, class_<SceneContext, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("SceneContext")
			.def_readonly("SkyLightCamEuler", &SceneContext::m_SkyLightCamEuler)
			.def_readonly("SkyLightColor", &SceneContext::m_SkyLightColor)
			.def_readonly("AmbientColor", &SceneContext::m_AmbientColor)
			.def_readonly("DofParams", &SceneContext::m_DofParams)
			.def_readonly("LuminanceThreshold", &SceneContext::m_LuminanceThreshold)
			.def_readonly("BloomColor", &SceneContext::m_BloomColor)
			.def_readonly("BloomFactor", &SceneContext::m_BloomFactor)
			.def_readonly("SsaoBias", &SceneContext::m_SsaoBias)
			.def_readonly("SsaoIntensity", &SceneContext::m_SsaoIntensity)
			.def_readonly("SsaoRadius", &SceneContext::m_SsaoRadius)
			.def_readonly("SsaoScale", &SceneContext::m_SsaoScale)
			.def_readonly("FogColor", &SceneContext::m_FogColor)
			.def_readonly("FogStartDistance", &SceneContext::m_FogStartDistance)
			.def_readonly("FogHeight", &SceneContext::m_FogHeight)
			.def_readonly("FogFalloff", &SceneContext::m_FogFalloff)
			.def_readonly("ActorList", &SceneContext::m_ActorList, return_stl_iterator)
			.def_readonly("DialogList", &SceneContext::m_DialogList, return_stl_iterator)
	];

	module(m_State)[
		// ! remove /GR- from PhysXExtensions.vcxproj
		class_<physx::PxJoint>("Joint")
			.def("setLocalPose", (void (physx::PxJoint::*)(physx::PxJointActorIndex::Enum actor, const my::Bone & pose)) & physx::PxJoint::setLocalPose)
			.def("getLocalPose", (my::Bone(physx::PxJoint::*)(physx::PxJointActorIndex::Enum)) & physx::PxJoint::getLocalPose)
			.enum_("PxConstraintFlag")
			[
				value("eBROKEN", physx::PxConstraintFlag::eBROKEN),
				value("ePROJECT_TO_ACTOR0", physx::PxConstraintFlag::ePROJECT_TO_ACTOR0),
				value("ePROJECT_TO_ACTOR1", physx::PxConstraintFlag::ePROJECT_TO_ACTOR1),
				value("ePROJECTION", physx::PxConstraintFlag::ePROJECTION),
				value("eCOLLISION_ENABLED", physx::PxConstraintFlag::eCOLLISION_ENABLED),
				value("eVISUALIZATION", physx::PxConstraintFlag::eVISUALIZATION),
				value("eDRIVE_LIMITS_ARE_FORCES", physx::PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES),
				value("eIMPROVED_SLERP", physx::PxConstraintFlag::eIMPROVED_SLERP),
				value("eDISABLE_PREPROCESSING", physx::PxConstraintFlag::eDISABLE_PREPROCESSING),
				value("eGPU_COMPATIBLE", physx::PxConstraintFlag::eGPU_COMPATIBLE)
			]
			.def("setConstraintFlag", &physx::PxJoint::setConstraintFlag)

		, class_<physx::PxRevoluteJoint, physx::PxJoint>("RevoluteJoint")
			.enum_("PxRevoluteJointFlag")
			[
				value("eLIMIT_ENABLED", physx::PxRevoluteJointFlag::eLIMIT_ENABLED),
				value("eDRIVE_ENABLED", physx::PxRevoluteJointFlag::eDRIVE_ENABLED),
				value("eDRIVE_FREESPIN", physx::PxRevoluteJointFlag::eDRIVE_FREESPIN)
			]
			.def("setRevoluteJointFlag", &physx::PxRevoluteJoint::setRevoluteJointFlag)
			.def("setLimit", luabind::tag_function<void(physx::PxRevoluteJoint*, float, float)>(
				boost::bind(&physx::PxRevoluteJoint::setLimit, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxJointAngularLimitPair>(), boost::placeholders::_2, boost::placeholders::_3, -1.0f))))

		, class_<physx::PxD6Joint, physx::PxJoint>("D6Joint")
			.enum_("PxD6Axis")
			[
				value("eX", physx::PxD6Axis::eX),
				value("eY", physx::PxD6Axis::eY),
				value("eZ", physx::PxD6Axis::eZ),
				value("eTWIST", physx::PxD6Axis::eTWIST),
				value("eSWING1", physx::PxD6Axis::eSWING1),
				value("eSWING2", physx::PxD6Axis::eSWING2),
				value("eCOUNT", physx::PxD6Axis::eCOUNT)
			]
			.enum_("PxD6Motion")
			[
				value("eLOCKED", physx::PxD6Motion::eLOCKED),
				value("eLIMITED", physx::PxD6Motion::eLIMITED),
				value("eFREE", physx::PxD6Motion::eFREE)
			]
			.def("setMotion", &physx::PxD6Joint::setMotion)
			.def("getMotion", &physx::PxD6Joint::getMotion)
			.def("setTwistLimit", luabind::tag_function<void(physx::PxD6Joint*,float,float)>(
				boost::bind(&physx::PxD6Joint::setTwistLimit, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxJointAngularLimitPair>(), boost::placeholders::_2, boost::placeholders::_3, -1.0f))))
			.def("setSwingLimit", luabind::tag_function<void(physx::PxD6Joint*,float,float)>(
				boost::bind(&physx::PxD6Joint::setSwingLimit, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxJointLimitCone>(), boost::placeholders::_2, boost::placeholders::_3, -1.0f))))

		, class_<my::BilinearFiltering<unsigned short> >("BilinearFilteringL16")
			.def(constructor<const D3DLOCKED_RECT&, int, int>())
			.def("Get", &my::BilinearFiltering<unsigned short>::Get)
			.def("Set", &my::BilinearFiltering<unsigned short>::Set)
			.def("Sample", &my::BilinearFiltering<unsigned short>::Sample)

		, class_<my::BilinearFiltering<DWORD> >("BilinearFilteringU32")
			.def(constructor<const D3DLOCKED_RECT&, int, int>())
			.def("Get", &my::BilinearFiltering<DWORD>::Get)
			.def("Set", &my::BilinearFiltering<DWORD>::Set)
			.def("Sample", &my::BilinearFiltering<DWORD>::Sample)

		, class_<my::IndexedBitmap>("IndexedBitmap")
			.def(constructor<int, int>())
			.property("Width", &my::IndexedBitmap::GetWidth)
			.property("Height", &my::IndexedBitmap::GetHeight)
			.def("GetPixel", &my::IndexedBitmap::GetPixel)
			.def("SetPixel", &my::IndexedBitmap::SetPixel)
			.def("LoadFromFile", &my::IndexedBitmap::LoadFromFile)
			.def("SaveIndexedBitmap", &indexedbitmap_save_indexed_bitmap)

		, class_<my::RayResult>("RayResult")
			.def(constructor<bool, float>())
			.def_readwrite("first", &my::RayResult::first)
			.def_readwrite("second", &my::RayResult::second)

		, class_<my::CollisionPrimitive>("CollisionPrimitive")
			.property("Offset", &my::CollisionPrimitive::getOffset, &my::CollisionPrimitive::setOffset)
			.property("Friction", &my::CollisionPrimitive::getFriction, &my::CollisionPrimitive::setFriction)
			.property("Restitution", &my::CollisionPrimitive::getRestitution, &my::CollisionPrimitive::setRestitution)
			.property("Transform", &my::CollisionPrimitive::getTransform, &my::CollisionPrimitive::setTransform)
			.enum_("PrimitiveType")
			[
				value("PrimitiveTypeShere", my::CollisionPrimitive::PrimitiveTypeShere),
				value("PrimitiveTypeBox", my::CollisionPrimitive::PrimitiveTypeBox)
			]
			.property("PrimitiveType", &my::CollisionPrimitive::getPrimitiveType)

		, class_<my::CollisionSphere, my::CollisionPrimitive>("CollisionSphere")
			.def(constructor<float, const my::Matrix4&, float, float>())
			.property("Radius", &my::CollisionSphere::getRadius, &my::CollisionSphere::setRadius)

		, class_<my::CollisionBox, my::CollisionPrimitive>("CollisionBox")
			.def(constructor<const my::Vector3&, const my::Matrix4&, float, float>())
			.property("HalfSize", &my::CollisionBox::getHalfSize, &my::CollisionBox::setHalfSize)

		, class_<my::IntersectionTests>("IntersectionTests")
			.scope
			[
				def("sphereAndHalfSpace", &my::IntersectionTests::sphereAndHalfSpace),
				def("sphereAndSphere", &my::IntersectionTests::sphereAndSphere),
				def("boxAndHalfSpace", &my::IntersectionTests::boxAndHalfSpace),
				def("boxAndBox", &my::IntersectionTests::boxAndBox),
				def("rayAndParallelPlane", &my::IntersectionTests::rayAndParallelPlane),
				def("rayAndAABB", &my::IntersectionTests::rayAndAABB),
				def("rayAndHalfSpace", &my::IntersectionTests::rayAndHalfSpace),
				def("rayAndSphere", &my::IntersectionTests::rayAndSphere),
				def("rayAndCylinder", &my::IntersectionTests::rayAndCylinder),
				def("rayAndTriangle", &my::IntersectionTests::rayAndTriangle),
				def("rayAndBox", &my::IntersectionTests::rayAndBox),
				def("IntersectSegments2D", &my::IntersectionTests::IntersectSegments2D, pure_out_value(boost::placeholders::_5))
			]

		, class_<PhysxSpatialIndex>("PhysxSpatialIndex")
			.def(constructor<>())
			.def("AddTriangle", &PhysxSpatialIndex::AddTriangle)
			.def("AddMesh", &PhysxSpatialIndex::AddMesh)
			.def("AddBox", luabind::tag_function<void(PhysxSpatialIndex*, float, float, float, const my::Vector3&, const my::Quaternion&)>(
				boost::bind(&PhysxSpatialIndex::AddGeometry, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxBoxGeometry>(), boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4), boost::placeholders::_5, boost::placeholders::_6)))
			.def("AddSphere", luabind::tag_function<void(PhysxSpatialIndex*, float, const my::Vector3&, const my::Quaternion&)>(
				boost::bind(&PhysxSpatialIndex::AddGeometry, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxSphereGeometry>(), boost::placeholders::_2), boost::placeholders::_3, boost::placeholders::_4)))
			.property("TriangleNum", &PhysxSpatialIndex::GetTriangleNum)
			.property("GeometryNum", &PhysxSpatialIndex::GetGeometryNum)
			.def("GetTriangle", &PhysxSpatialIndex::GetTriangle, pure_out_value(boost::placeholders::_3) + pure_out_value(boost::placeholders::_4) + pure_out_value(boost::placeholders::_5))
			.def("GetGeometryType", &PhysxSpatialIndex::GetGeometryType)
			.def("GetBox", &PhysxSpatialIndex::GetBox, pure_out_value(boost::placeholders::_3) + pure_out_value(boost::placeholders::_4) + pure_out_value(boost::placeholders::_5) + pure_out_value(boost::placeholders::_6) + pure_out_value(boost::placeholders::_7))
			.def("GetGeometryWorldBox", &PhysxSpatialIndex::GetGeometryWorldBox)
			.def("Raycast", &PhysxSpatialIndex::Raycast, pure_out_value(boost::placeholders::_5))
			.def("OverlapBox", luabind::tag_function<bool(PhysxSpatialIndex*, float, float, float, const my::Vector3&, const my::Quaternion&)>(
				boost::bind(&PhysxSpatialIndex::Overlap, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxBoxGeometry>(), boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4), boost::placeholders::_5, boost::placeholders::_6)))
			.def("OverlapSphere", luabind::tag_function<bool(PhysxSpatialIndex*, float, const my::Vector3&, const my::Quaternion&)>(
				boost::bind(&PhysxSpatialIndex::Overlap, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxSphereGeometry>(), boost::placeholders::_2), boost::placeholders::_3, boost::placeholders::_4)))
			.def("OverlapCapsule", luabind::tag_function<bool(PhysxSpatialIndex*, float, float, const my::Vector3&, const my::Quaternion&)>(
				boost::bind(&PhysxSpatialIndex::Overlap, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxCapsuleGeometry>(), boost::placeholders::_2, boost::placeholders::_3), boost::placeholders::_4, boost::placeholders::_5)))
			.def("SweepBox", luabind::tag_function<bool(PhysxSpatialIndex*, float, float, float, const my::Vector3&, const my::Quaternion&, const my::Vector3&, float, float&)>(
				boost::bind(&PhysxSpatialIndex::Sweep, boost::placeholders::_1, boost::bind(boost::value_factory<physx::PxBoxGeometry>(), boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4), boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8, boost::placeholders::_9)), pure_out_value(boost::placeholders::_9))
			.def("CalculateAABB", &PhysxSpatialIndex::CalculateAABB)

		, class_<boost::regex>("regex")
			.def(constructor<const char *>())
			.def("search", &regex_search, pure_out_value(boost::placeholders::_3) /*+ dependency(boost::placeholders::_3, boost::placeholders::_2)*/) // ! dependency not worked
			.def("search_all", &regex_search_all, return_stl_iterator /*+ dependency(result, boost::placeholders::_2)*/)

		, class_<boost::cmatch>("cmatch")
			.def(constructor<>())
			.def("is_matched", &cmatch_is_matched)
			.def("sub_match", &cmatch_sub_match)

		, class_<rapidxml::xml_base<char> >("xml_base")
			.property("name", (char* (rapidxml::xml_base<char>::*)()const)& rapidxml::xml_base<char>::name)
			.property("value", (char* (rapidxml::xml_base<char>::*)()const)& rapidxml::xml_base<char>::value)

		, class_<rapidxml::xml_node<char>, rapidxml::xml_base<char> >("xml_node")
			.property("type", (rapidxml::node_type(rapidxml::xml_node<char>::*)() const)& rapidxml::xml_node<char>::type)
			.def("first_node", luabind::tag_function<rapidxml::xml_node<char>* (const rapidxml::xml_node<char>*)>(
				boost::bind(&rapidxml::xml_node<char>::first_node, boost::placeholders::_1, (char*)0, 0, true)))
			.def("first_node", luabind::tag_function<rapidxml::xml_node<char>* (const rapidxml::xml_node<char>*, const std::string&)>(
				boost::bind(&rapidxml::xml_node<char>::first_node, boost::placeholders::_1, boost::bind(&std::string::c_str, boost::placeholders::_2), boost::bind(&std::string::length, boost::placeholders::_2), true)))
			.def("first_attribute", luabind::tag_function<rapidxml::xml_node<char>* (const rapidxml::xml_node<char>*)>(
				boost::bind(&rapidxml::xml_node<char>::first_attribute, boost::placeholders::_1, (char*)0, 0, true)))
			.def("first_attribute", luabind::tag_function<rapidxml::xml_node<char>* (const rapidxml::xml_node<char>*, const std::string&)>(
				boost::bind(&rapidxml::xml_node<char>::first_attribute, boost::placeholders::_1, boost::bind(&std::string::c_str, boost::placeholders::_2), boost::bind(&std::string::length, boost::placeholders::_2), true)))
			.def("next_sibling", luabind::tag_function<rapidxml::xml_node<char>* (const rapidxml::xml_node<char>*)>(
				boost::bind(&rapidxml::xml_node<char>::next_sibling, boost::placeholders::_1, (char*)0, 0, true)))
			.def("next_sibling", luabind::tag_function<rapidxml::xml_node<char>* (const rapidxml::xml_node<char>*, const std::string&)>(
				boost::bind(&rapidxml::xml_node<char>::next_sibling, boost::placeholders::_1, boost::bind(&std::string::c_str, boost::placeholders::_2), boost::bind(&std::string::length, boost::placeholders::_2), true)))
			.def("print_to_file", &xml_node_print_to_file)

		, class_<rapidxml::xml_attribute<char>, rapidxml::xml_base<char> >("xml_attribute")
			.def("next_attribute", luabind::tag_function<rapidxml::xml_attribute<char>* (const rapidxml::xml_attribute<char>*)>(
				boost::bind(&rapidxml::xml_attribute<char>::next_attribute, boost::placeholders::_1, (char*)0, 0, true)))
			.def("next_attribute", luabind::tag_function<rapidxml::xml_attribute<char>* (const rapidxml::xml_attribute<char>*, const std::string&)>(
				boost::bind(&rapidxml::xml_attribute<char>::next_attribute, boost::placeholders::_1, boost::bind(&std::string::c_str, boost::placeholders::_2), boost::bind(&std::string::length, boost::placeholders::_2), true)))

		, class_<rapidxml::xml_document<char>, rapidxml::xml_node<char> >("xml_document")
			.def(constructor<>())
			.def("parse", &xml_document_parse, pure_out_value(boost::placeholders::_3) + dependency(boost::placeholders::_1, boost::placeholders::_3))

		, class_<boost::property_tree::ptree>("ptree")
			.def(constructor<>())
			.def("read_xml", luabind::tag_function<void(boost::property_tree::ptree&, const std::string&)>(
				boost::bind((void (*)(const std::string&, boost::property_tree::ptree&, int, const std::locale&))boost::property_tree::read_xml, boost::placeholders::_2, boost::placeholders::_1, 0, std::locale())))
			.def("read_info", luabind::tag_function<void(boost::property_tree::ptree&, const std::string&)>(
				boost::bind((void (*)(const std::string&, boost::property_tree::ptree&, const std::locale&))boost::property_tree::read_info, boost::placeholders::_2, boost::placeholders::_1, std::locale())))
			.property("data", (const std::string& (boost::property_tree::ptree::*)()const)& boost::property_tree::ptree::data)
			.def("get", luabind::tag_function<std::string(boost::property_tree::ptree*,const char*,const std::string&)>(
				boost::bind(&boost::property_tree::ptree::get<std::string>, boost::placeholders::_1, boost::bind(boost::value_factory<boost::property_tree::path>(), boost::placeholders::_2, '/'), boost::placeholders::_3)))

		//, class_<FastNoiseLite>("FastNoiseLite")
		//	.def(constructor<int>())
		//	.enum_("NoiseType")
		//	[
		//		value("NoiseType_OpenSimplex2", FastNoiseLite::NoiseType_OpenSimplex2),
		//		value("NoiseType_OpenSimplex2S", FastNoiseLite::NoiseType_OpenSimplex2S),
		//		value("NoiseType_Cellular", FastNoiseLite::NoiseType_Cellular),
		//		value("NoiseType_Perlin", FastNoiseLite::NoiseType_Perlin),
		//		value("NoiseType_ValueCubic", FastNoiseLite::NoiseType_ValueCubic),
		//		value("NoiseType_Value", FastNoiseLite::NoiseType_Value)
		//	]
		//	.enum_("RotationType3D")
		//	[
		//		value("RotationType3D_None", FastNoiseLite::RotationType3D_None),
		//		value("RotationType3D_ImproveXYPlanes", FastNoiseLite::RotationType3D_ImproveXYPlanes),
		//		value("RotationType3D_ImproveXZPlanes", FastNoiseLite::RotationType3D_ImproveXZPlanes)
		//	]
		//	.enum_("FractalType")
		//	[
		//		value("FractalType_None", FastNoiseLite::FractalType_None),
		//		value("FractalType_FBm", FastNoiseLite::FractalType_FBm),
		//		value("FractalType_Ridged", FastNoiseLite::FractalType_Ridged),
		//		value("FractalType_PingPong", FastNoiseLite::FractalType_PingPong),
		//		value("FractalType_DomainWarpProgressive", FastNoiseLite::FractalType_DomainWarpProgressive),
		//		value("FractalType_DomainWarpIndependent", FastNoiseLite::FractalType_DomainWarpIndependent)
		//	]
		//	.enum_("CellularDistanceFunction")
		//	[
		//		value("CellularDistanceFunction_Euclidean", FastNoiseLite::CellularDistanceFunction_Euclidean),
		//		value("CellularDistanceFunction_EuclideanSq", FastNoiseLite::CellularDistanceFunction_EuclideanSq),
		//		value("CellularDistanceFunction_Manhattan", FastNoiseLite::CellularDistanceFunction_Manhattan),
		//		value("CellularDistanceFunction_Hybrid", FastNoiseLite::CellularDistanceFunction_Hybrid)
		//	]
		//	.enum_("CellularReturnType")
		//	[
		//		value("CellularReturnType_CellValue", FastNoiseLite::CellularReturnType_CellValue),
		//		value("CellularReturnType_Distance", FastNoiseLite::CellularReturnType_Distance),
		//		value("CellularReturnType_Distance2", FastNoiseLite::CellularReturnType_Distance2),
		//		value("CellularReturnType_Distance2Add", FastNoiseLite::CellularReturnType_Distance2Add),
		//		value("CellularReturnType_Distance2Sub", FastNoiseLite::CellularReturnType_Distance2Sub),
		//		value("CellularReturnType_Distance2Mul", FastNoiseLite::CellularReturnType_Distance2Mul),
		//		value("CellularReturnType_Distance2Div", FastNoiseLite::CellularReturnType_Distance2Div)
		//	]
		//	.enum_("DomainWarpType")
		//	[
		//		value("DomainWarpType_OpenSimplex2", FastNoiseLite::DomainWarpType_OpenSimplex2),
		//		value("DomainWarpType_OpenSimplex2Reduced", FastNoiseLite::DomainWarpType_OpenSimplex2Reduced),
		//		value("DomainWarpType_BasicGrid", FastNoiseLite::DomainWarpType_BasicGrid)
		//	]
		//	.def("SetSeed", &FastNoiseLite::SetSeed)
		//	.def("SetFrequency", &FastNoiseLite::SetFrequency)
		//	.def("SetNoiseType", &FastNoiseLite::SetNoiseType)
		//	.def("SetRotationType3D", &FastNoiseLite::SetRotationType3D)
		//	.def("SetFractalType", &FastNoiseLite::SetFractalType)
		//	.def("SetFractalOctaves", &FastNoiseLite::SetFractalOctaves)
		//	.def("SetFractalLacunarity", &FastNoiseLite::SetFractalLacunarity)
		//	.def("SetFractalGain", &FastNoiseLite::SetFractalGain)
		//	.def("SetFractalWeightedStrength", &FastNoiseLite::SetFractalWeightedStrength)
		//	.def("SetFractalPingPongStrength", &FastNoiseLite::SetFractalPingPongStrength)
		//	.def("SetCellularDistanceFunction", &FastNoiseLite::SetCellularDistanceFunction)
		//	.def("SetCellularReturnType", &FastNoiseLite::SetCellularReturnType)
		//	.def("SetCellularJitter", &FastNoiseLite::SetCellularJitter)
		//	.def("SetDomainWarpType", &FastNoiseLite::SetDomainWarpType)
		//	.def("SetDomainWarpAmp", &FastNoiseLite::SetDomainWarpAmp)
		//	.def("GetNoise", (float (FastNoiseLite::*)(LUA_NUMBER, LUA_NUMBER))&FastNoiseLite::GetNoise)
		//	.def("GetNoise", (float (FastNoiseLite::*)(LUA_NUMBER, LUA_NUMBER, LUA_NUMBER))&FastNoiseLite::GetNoise)
		//	.def("DomainWarp", (void (FastNoiseLite::*)(LUA_NUMBER&, LUA_NUMBER&))& FastNoiseLite::DomainWarp)
		//	.def("DomainWarp", (void (FastNoiseLite::*)(LUA_NUMBER&, LUA_NUMBER&, LUA_NUMBER&))& FastNoiseLite::DomainWarp)

		//, class_<ClipperLib::IntPoint>("IntPoint")
		//	.def(constructor<int, int>())
		//	.def_readwrite("x", &ClipperLib::IntPoint::X)
		//	.def_readwrite("y", &ClipperLib::IntPoint::Y)
		//	.def(const_self == other<const ClipperLib::IntPoint&>())

		//, class_<ClipperLib::Path>("Path")
		//	.def(constructor<>())
		//	.def("AddPoint", (void (ClipperLib::Path::*)(const ClipperLib::IntPoint&))& ClipperLib::Path::push_back)
		//	.property("PointNum", &ClipperLib::Path::size)
		//	.def("GetPoint", (ClipperLib::IntPoint& (ClipperLib::Path::*)(size_t))& ClipperLib::Path::at)
		//	.property("Orientation", &ClipperLib::Orientation)
		//	.property("Area", &ClipperLib::Area)
		//	.def("PointInPolygon", luabind::tag_function<int(const ClipperLib::Path&, const ClipperLib::IntPoint&)>(
		//		boost::bind(ClipperLib::PointInPolygon, boost::placeholders::_2, boost::placeholders::_1)))
		//	.def("ReversePath", &ClipperLib::ReversePath)

		//, class_<ClipperLib::Paths>("Paths")
		//	.def(constructor<>())
		//	.def("AddPath", (void (ClipperLib::Paths::*)(const ClipperLib::Path&))& ClipperLib::Paths::push_back)
		//	.property("PathNum", &ClipperLib::Paths::size)
		//	.def("GetPath", (ClipperLib::Path& (ClipperLib::Paths::*)(size_t))& ClipperLib::Paths::at)

		//, class_<ClipperLib::Clipper>("Clipper")
		//	.enum_("InitOptions")
		//	[
		//		value("ioReverseSolution", ClipperLib::InitOptions::ioReverseSolution),
		//		value("ioStrictlySimple", ClipperLib::InitOptions::ioStrictlySimple),
		//		value("ioPreserveCollinear", ClipperLib::InitOptions::ioPreserveCollinear)
		//	]
		//	.enum_("ClipType")
		//	[
		//		value("ctIntersection", ClipperLib::ClipType::ctIntersection),
		//		value("ctUnion", ClipperLib::ClipType::ctUnion),
		//		value("ctDifference", ClipperLib::ClipType::ctDifference),
		//		value("ctXor", ClipperLib::ClipType::ctXor)
		//	]
		//	.enum_("PolyType")
		//	[
		//		value("ptSubject", ClipperLib::PolyType::ptSubject),
		//		value("ptClip", ClipperLib::PolyType::ptClip)
		//	]
		//	.enum_("PolyFillType")
		//	[
		//		value("pftEvenOdd", ClipperLib::PolyFillType::pftEvenOdd),
		//		value("pftNonZero", ClipperLib::PolyFillType::pftNonZero),
		//		value("pftPositive", ClipperLib::PolyFillType::pftPositive),
		//		value("pftNegative", ClipperLib::PolyFillType::pftNegative)
		//	]
		//	.def(constructor<int>())
		//	.def("AddPath", &ClipperLib::Clipper::AddPath)
		//	.def("Execute", (bool (ClipperLib::Clipper::*)(ClipperLib::ClipType, ClipperLib::Paths&, ClipperLib::PolyFillType))&ClipperLib::Clipper::Execute, pure_out_value(boost::placeholders::_3))

		//, class_<ClipperLib::ClipperOffset>("ClipperOffset")
		//	.enum_("JoinType")
		//	[
		//		value("jtSquare", ClipperLib::JoinType::jtSquare),
		//		value("jtRound", ClipperLib::JoinType::jtRound),
		//		value("jtMiter", ClipperLib::JoinType::jtMiter)
		//	]
		//	.enum_("EndType")
		//	[
		//		value("etClosedPolygon", ClipperLib::EndType::etClosedPolygon),
		//		value("etClosedLine", ClipperLib::EndType::etClosedLine),
		//		value("etOpenButt", ClipperLib::EndType::etOpenButt),
		//		value("etOpenSquare", ClipperLib::EndType::etOpenSquare),
		//		value("etOpenRound", ClipperLib::EndType::etOpenRound)
		//	]
		//	.def(constructor<double,double>())
		//	.def("AddPath", &ClipperLib::ClipperOffset::AddPath)
		//	.def("AddPaths", &ClipperLib::ClipperOffset::AddPaths)
		//	.def("Execute", (void (ClipperLib::ClipperOffset::*)(ClipperLib::Paths&, double))& ClipperLib::ClipperOffset::Execute, pure_out_value(boost::placeholders::_2))
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

int LuaContext::dostring(const char * s, const char * name)
{
	return luaL_loadbuffer(m_State, s, strlen(s), name) || docall(0, 1);
}

int LuaContext::dogc(int what, int data)
{
	return lua_gc(m_State, what, data);
}
