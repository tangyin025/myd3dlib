require "ui/zhCN.lua"
require "Hud.lua"
require "Settings.lua"
require "Player.lua"
require "Action.lua"

-- 创建一个物理球
actor2=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,1,-5),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local lambert2=Material()
lambert2.Shader="shader/mtl_BlinnPhong.fx"
lambert2.PassMask=Material.PassMaskShadowNormalOpaque
lambert2:AddParameter("g_DiffuseTexture", "texture/Checker.bmp")
lambert2:AddParameter("g_NormalTexture", "texture/Normal.dds")
lambert2:AddParameter("g_SpecularTexture", "texture/White.dds")
local cmp2=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp2.MeshPath="mesh/Sphere.mesh.xml"
cmp2.Material=lambert2
actor2:AddComponent(cmp2)
actor2:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp2:CreateSphereShape(Vector3(0,0,0),Quaternion.Identity(),1,false)
cmp2.SimulationFilterWord0=1
cmp2.QueryFilterWord0=1
class 'Actor2Behavior'(Component)
function Actor2Behavior:__init(name)
	Component.__init(self,name)
end
function Actor2Behavior:RequestResource()
	Component.RequestResource(self)
	self.Actor:PlayAction(SAction.act_sound)
end
actor2_behavior=Actor2Behavior(NamedObject.MakeUniqueName("actor_behavior"))
actor2:AddComponent(actor2_behavior)

-- 在角色手部绑定物体
actor3=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp3=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp3.MeshPath="mesh/Cylinder.mesh.xml"
cmp3.Material=lambert2:Clone()
actor3:AddComponent(cmp3)
actor3:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp3:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,false)
cmp3.SimulationFilterWord0=1
cmp3.QueryFilterWord0=1

-- 在角色手部绑定物体
actor4=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp4=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp4.MeshPath="mesh/Cylinder.mesh.xml"
cmp4.Material=lambert2:Clone()
actor4:AddComponent(cmp4)
actor4:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp4:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,false)
cmp4.SimulationFilterWord0=1
cmp4.QueryFilterWord0=1

-- 搞一个trigger
actor5=Actor(NamedObject.MakeUniqueName("actor"),Vector3(3,1,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp5=StaticEmitterComponent(NamedObject.MakeUniqueName("mesh_cmp"),1,1,EmitterComponent.FaceTypeCamera,EmitterComponent.SpaceTypeLocal,EmitterComponent.VelocityTypeNone,EmitterComponent.PrimitiveTypeQuad)
actor5:AddComponent(cmp5)
actor5:CreateRigidActor(Actor.eRIGID_STATIC)
cmp5:CreateBoxShape(Vector3(0,0,0),Quaternion(0,0,0,1),1,1,1,false)
cmp5.SimulationFilterWord0=2
cmp5.QueryFilterWord0=2
cmp5:SetShapeFlag(Component.eSIMULATION_SHAPE,false)
cmp5:SetShapeFlag(Component.eTRIGGER_SHAPE,true)
actor5.EventEnterTrigger=function(arg)
	print("enter trigger: "..arg.other.Name)
end
actor5.EventLeaveTrigger=function(arg)
	print("leave trigger: "..arg.other.Name)
end

-- 加载场景资源
game:LoadSceneAsync("scene01.xml", function(res)
	game:SetScene(res2scene(res))

	SPlayer.player:SetPose(Vector3(0,3,0),Quaternion.Identity())
	game:AddEntity(act2entity(SPlayer.player),SPlayer.player.aabb:transform(SPlayer.player.World),1.0,0.1)

	actor3:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor3.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	game:AddEntity(act2entity(actor3),actor3.aabb:transform(actor3.World),1.0,0.1)
	SPlayer.player:Attach(actor3, 10)

	actor4:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor4.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	game:AddEntity(act2entity(actor4),actor4.aabb:transform(actor4.World),1.0,0.1)
	SPlayer.player:Attach(actor4, 29)

	actor2:SetPose(Vector3(0,1,-5),Quaternion.Identity())
	game:AddEntity(act2entity(actor2),actor2.aabb:transform(actor2.World),1.0,0.1)

	actor5:UpdateWorld()
	game:AddEntity(act2entity(actor5),actor5.aabb:transform(actor5.World),1.0,0.1)

	SAction.act_moving_track.ParamStartPos=Vector3(-3,1,0)
	SAction.act_moving_track.ParamEndPos=Vector3(-3,1,-5)
	local actor6 = game:GetNamedObject("editor_actor1")
	class 'Actor6Behavior'(Component)
	function Actor6Behavior:__init(name)
		Component.__init(self,name)
	end
	function Actor6Behavior:RequestResource()
		Component.RequestResource(self)
		self.Actor:PlayAction(SAction.act_moving)
	end
	actor6_behavior=Actor6Behavior(NamedObject.MakeUniqueName('actor_behavior'))
	actor6:AddComponent(actor6_behavior)	

	-- SPlayer.player:Detach(actor3);actor3:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor3.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end;SPlayer.player:Detach(actor4);actor4:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor4.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end
	-- SAction.act_pose_track.ParamStartPos=Vector3(0,3,0);SAction.act_pose_track.ParamEndPos=Vector3(-3,3,0);SPlayer.player:PlayAction(SAction.act_pose)

end, 0)
	
-- -- 特殊渲染选项
-- game.SsaoEnable=true
-- game:SetVisualizationParameter(PhysxScene.eSCALE,1)
-- game:SetVisualizationParameter(PhysxScene.eCOLLISION_SHAPES,1)
-- game:SetVisualizationParameter(PhysxScene.eCOLLISION_FNORMALS,1)
-- game:SetVisualizationParameter(PhysxScene.eCOLLISION_AABBS,1)
-- game:SetControllerDebugRenderingFlags(PhysxScene.eALL)
