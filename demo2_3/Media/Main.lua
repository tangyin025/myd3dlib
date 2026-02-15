require "Hud.lua"
require "Settings.lua"
require "Player.lua"
require "Action.lua"

-- 创建一个物理球
actor2=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,1,-5),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local lambert2=Material()
lambert2.Shader="shader/mtl_BlinnPhong.fx"
lambert2.PassMask=Material.PassMaskShadowNormalOpaque
lambert2:SetParameter("g_DiffuseTexture", "texture/Checker.bmp")
lambert2:SetParameter("g_NormalTexture", "texture/Normal.dds")
lambert2:SetParameter("g_SpecularTexture", "texture/White.dds")
local cmp2=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp2.MeshPath="mesh/Sphere.mesh.xml"
cmp2.Material=lambert2
actor2:InsertComponent(cmp2)
actor2:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp2:CreateSphereShape(Vector3(0,0,0),Quaternion.Identity(),1,0.5,0.5,0.5)
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
local actor2_behavior=Actor2Behavior(NamedObject.MakeUniqueName("actor_behavior"))
actor2:InsertComponentAdopt(actor2_behavior)

-- 在角色手部绑定物体
actor3=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(0.1,0.25,0.1),AABB(-1,1))
local cmp3=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp3.MeshPath="mesh/Cylinder.mesh.xml"
cmp3.Material=lambert2:Clone()
actor3:InsertComponent(cmp3)
actor3:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp3:CreateCapsuleShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(-90)),0.1,0.25,0.5,0.5,0.5)
cmp3.SimulationFilterWord0=1
cmp3.QueryFilterWord0=1

-- 在角色手部绑定物体
actor4=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(0.1,0.25,0.1),AABB(-1,1))
local cmp4=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp4.MeshPath="mesh/Cylinder.mesh.xml"
cmp4.Material=lambert2:Clone()
actor4:InsertComponent(cmp4)
actor4:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp4:CreateCapsuleShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(-90)),0.1,0.25,0.5,0.5,0.5)
cmp4.SimulationFilterWord0=1
cmp4.QueryFilterWord0=1

-- 搞一个trigger
actor5=Actor(NamedObject.MakeUniqueName("actor"),Vector3(3,1,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp5=StaticEmitter(NamedObject.MakeUniqueName("mesh_cmp"),actor5.aabb,3,EmitterComponent.FaceTypeCamera,EmitterComponent.SpaceTypeLocal)
actor5:InsertComponent(cmp5)
actor5:CreateRigidActor(Actor.eRIGID_STATIC)
cmp5:CreateBoxShape(Vector3(0,0,0),Quaternion(0,0,0,1),1,1,1,0.5,0.5,0.5)
cmp5.SimulationFilterWord0=2
cmp5.QueryFilterWord0=2
cmp5.ShapeFlags=bit.bor(Component.eSCENE_QUERY_SHAPE,Component.eTRIGGER_SHAPE,Component.eVISUALIZATION)

-- 加载场景资源
scene01=nil
client:LoadSceneAsync("scene01.xml", "scene01_", function(res)
	scene01=res
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
	
	-- 之前Settings.dlg.Visible=false，导致hud被focused
	Control.SetFocusControl(nil)

	SPlayer.player:SetPose(Vector3(0,3,0),Quaternion.Identity())
	client:AddEntity(SPlayer.player)

	local actor1 = client:GetNamedObject("scene01_actor1")
	class 'actor1Behavior'(Component)
	function actor1Behavior:__init(name)
		Component.__init(self,name)
	end
	function actor1Behavior:RequestResource()
		Component.RequestResource(self)
		-- self.Actor:PlayAction(SAction.act_pose)
	end
	function actor1Behavior:OnPxThreadSubstep(dtime)
		self.Actor:SetPxPoseOrbyPxThread(Vector3(-3,1,math.sin(client.TotalTime)*3-5),Quaternion.Identity(),nil)
	end
	local actor1_behavior=actor1Behavior(NamedObject.MakeUniqueName('actor_behavior'))
	actor1:InsertComponentAdopt(actor1_behavior)

	actor3:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor3.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	client:AddEntity(actor3)
	SPlayer.player:Attach(actor3, SAction.skel:GetBoneIndex("joint13"))
	actor3.Position=Vector3(2.5,0,0)
	actor3.Rotation=Quaternion.RotationYawPitchRoll(0,0,math.rad(90))

	actor4:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor4.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	client:AddEntity(actor4)
	SPlayer.player:Attach(actor4, SAction.skel:GetBoneIndex("joint59"))
	actor4.Position=Vector3(2.5,0,0)
	actor4.Rotation=Quaternion.RotationYawPitchRoll(0,0,math.rad(90))

	actor2:SetPose(Vector3(0,1,-5),Quaternion.Identity())
	client:AddEntity(actor2)

	class 'actor5Behavior'(Component)
	function actor5Behavior:__init(name)
		Component.__init(self,name)
	end
	function actor5Behavior:OnTrigger(arg)
		print("enter trigger: "..arg.other.Name, arg.events)
	end
	local actor5_behavior=actor5Behavior(NamedObject.MakeUniqueName('actor_behavior'))
	actor5:InsertComponentAdopt(actor5_behavior)
	actor5:UpdateWorld()
	client:AddEntity(actor5)

	-- SPlayer.player:Detach(actor3);actor3:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor3.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end;SPlayer.player:Detach(actor4);actor4:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor4.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end

end, 0)
	
-- 特殊渲染选项
client.SsaoEnable=true
client:SetVisualizationParameter(PhysxScene.eSCALE,1)
client:SetVisualizationParameter(PhysxScene.eCOLLISION_SHAPES,1)
client:SetVisualizationParameter(PhysxScene.eCOLLISION_FNORMALS,1)
client:SetVisualizationParameter(PhysxScene.eCOLLISION_AABBS,1)
client:SetControllerDebugRenderingFlags(PhysxScene.eALL)
