require "ui/zhCN.lua"
require "ui/Hud.lua"
require "ui/Settings.lua"
require "Player.lua"
require "Action.lua"

collectionObjs=CollectionObjMap()

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
cmp2:CreateSphereShape(Vector3(0,0,0),Quaternion.Identity(),1,collectionObjs)
cmp2.SimulationFilterWord0=1
cmp2.QueryFilterWord0=1
class 'Actor2Behavior'(Component)
function Actor2Behavior:__init(name)
	Component.__init(self,name)
end
function Actor2Behavior:RequestResource()
	Component.RequestResource(self)
	self.Actor:PlayAction(SAction.act_sound,9999)
end
actor2_behavior=Actor2Behavior(NamedObject.MakeUniqueName("actor_behavior"))
actor2:AddComponent(actor2_behavior)

-- 在角色手部绑定物体
actor3=Actor(NamedObject.MakeUniqueName("actor"),Vector3(25,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.1,0.25,0.1),AABB(-1,1))
local cmp3=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp3.MeshPath="mesh/Cylinder.mesh.xml"
cmp3.Material=lambert2:Clone()
actor3:AddComponent(cmp3)
actor3:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp3:CreateCapsuleShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(-90)),0.1,0.25,collectionObjs)
cmp3.SimulationFilterWord0=1
cmp3.QueryFilterWord0=1

-- 在角色手部绑定物体
actor4=Actor(NamedObject.MakeUniqueName("actor"),Vector3(25,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.1,0.25,0.1),AABB(-1,1))
local cmp4=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp4.MeshPath="mesh/Cylinder.mesh.xml"
cmp4.Material=lambert2:Clone()
actor4:AddComponent(cmp4)
actor4:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp4:CreateCapsuleShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(-90)),0.1,0.25,collectionObjs)
cmp4.SimulationFilterWord0=1
cmp4.QueryFilterWord0=1

-- 搞一个trigger
actor5=Actor(NamedObject.MakeUniqueName("actor"),Vector3(3,1,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp5=StaticEmitter(NamedObject.MakeUniqueName("mesh_cmp"),actor5.aabb,3,EmitterComponent.FaceTypeCamera,EmitterComponent.SpaceTypeLocal,EmitterComponent.VelocityTypeNone,EmitterComponent.PrimitiveTypeQuad)
actor5:AddComponent(cmp5)
actor5:CreateRigidActor(Actor.eRIGID_STATIC)
cmp5:CreateBoxShape(Vector3(0,0,0),Quaternion(0,0,0,1),1,1,1,collectionObjs)
cmp5.SimulationFilterWord0=2
cmp5.QueryFilterWord0=2
cmp5:SetShapeFlag(Component.eSIMULATION_SHAPE,false)
cmp5:SetShapeFlag(Component.eTRIGGER_SHAPE,true)

-- 加载场景资源
scene01=nil
client:LoadSceneAsync("scene01.xml", "scene01_", function(res)
	scene01=res2scene(res)
	client.SkyLightCam.Euler = scene01.SkyLightCamEuler;
	client.SkyLightColor = scene01.SkyLightColor;
	client.AmbientColor = scene01.AmbientColor;
	client.DofParams = scene01.DofParams;
	client.SsaoBias = scene01.SsaoBias;
	client.SsaoIntensity = scene01.SsaoIntensity;
	client.SsaoRadius = scene01.SsaoRadius;
	client.SsaoScale = scene01.SsaoScale;
	client.FogColor = scene01.FogColor;
	client.FogStartDistance = scene01.FogStartDistance;
	client.FogHeight = scene01.FogHeight;
	client.FogFalloff = scene01.FogFalloff;
	for act in scene01.ActorList do
		client:AddEntity(act)
	end

	SPlayer.player:SetPose(Vector3(0,3,0),Quaternion.Identity())
	client:AddEntity(SPlayer.player)

	actor3:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor3.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	client:AddEntity(actor3)
	SPlayer.player:Attach(actor3, 10)

	actor4:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor4.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	client:AddEntity(actor4)
	SPlayer.player:Attach(actor4, 29)

	actor2:SetPose(Vector3(0,1,-5),Quaternion.Identity())
	client:AddEntity(actor2)

	actor5:UpdateWorld()
	client:AddEntity(actor5)

	SAction.act_moving_track.ParamStartPos=Vector3(-3,1,0)
	SAction.act_moving_track.ParamEndPos=Vector3(-3,1,-5)
	local actor6 = client:GetNamedObject("scene01_actor1")
	class 'Actor6Behavior'(Component)
	function Actor6Behavior:__init(name)
		Component.__init(self,name)
	end
	function Actor6Behavior:RequestResource()
		Component.RequestResource(self)
		self.Actor:PlayAction(SAction.act_moving,9999)
	end
	actor6_behavior=Actor6Behavior(NamedObject.MakeUniqueName('actor_behavior'))
	actor6:AddComponent(actor6_behavior)	

	-- SPlayer.player:Detach(actor3);actor3:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor3.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end;SPlayer.player:Detach(actor4);actor4:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor4.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end
	-- SAction.act_pose_track.ParamStartPos=Vector3(0,3,0);SAction.act_pose_track.ParamEndPos=Vector3(-3,3,0);SPlayer.player:PlayAction(SAction.act_pose)

end, 0)
	
-- -- 特殊渲染选项
-- client.SsaoEnable=true
-- client:SetVisualizationParameter(PhysxScene.eSCALE,1)
-- client:SetVisualizationParameter(PhysxScene.eCOLLISION_SHAPES,1)
-- client:SetVisualizationParameter(PhysxScene.eCOLLISION_FNORMALS,1)
-- client:SetVisualizationParameter(PhysxScene.eCOLLISION_AABBS,1)
-- client:SetControllerDebugRenderingFlags(PhysxScene.eALL)
