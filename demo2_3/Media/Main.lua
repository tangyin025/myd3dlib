require "ui/zhCN.lua"
require "Hud.lua"
require "Settings.lua"
require "Player.lua"
require "Action.lua"

-- -- 设置环境光
-- game.SkyLightCam.Eye=Vector3(0,0,0)
-- game.SkyLightCam.Eular=Vector3(math.rad(-30),math.rad(0),0)
-- game.SkyLightCam.Width=50
-- game.SkyLightCam.Height=50
-- game.SkyLightCam.Nz=-50
-- game.SkyLightCam.Fz=50
-- game.SkyLightDiffuse=Vector4(0.7,0.7,0.7,0.7)
-- game.SkyLightAmbient=Vector4(0.5,0.5,0.5,0.0)

-- -- 创建地面
-- actor=Actor(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-512,512))
-- local cmp=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
-- local lambert1=Material()
-- lambert1.Shader="shader/mtl_lambert1.fx"
-- lambert1.PassMask=Material.PassMaskShadowNormalOpaque
-- lambert1:AddParameterTexture("g_DiffuseTexture", "texture/Checker.bmp")
-- lambert1:AddParameterTexture("g_NormalTexture", "texture/Normal.dds")
-- lambert1:AddParameterTexture("g_SpecularTexture", "texture/White.dds")
-- cmp:AddMaterial(lambert1)
-- cmp.MeshPath="mesh/plane.mesh.xml"
-- cmp.MeshEventReady=function(arg)
	-- cmp.Mesh:Transform(Matrix4.Scaling(256,1,256))
-- end
-- actor:AddComponent(cmp)
-- actor:CreateRigidActor(Actor.eRIGID_STATIC)
-- cmp:CreatePlaneShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),1)
-- actor:UpdateWorld()
-- game:AddEntity(actor2ent(actor),actor.aabb:transform(actor.World),1.0,0.1)

-- 创建一个物理球
actor2=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,1,-5),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local lambert2=Material()
lambert2.Shader="shader/mtl_lambert1.fx"
lambert2.PassMask=Material.PassMaskShadowNormalOpaque
lambert2:AddParameterTexture("g_DiffuseTexture", "texture/Checker.bmp")
lambert2:AddParameterTexture("g_NormalTexture", "texture/Normal.dds")
lambert2:AddParameterTexture("g_SpecularTexture", "texture/White.dds")
local cmp2=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp2.MeshPath="mesh/Sphere.mesh.xml"
cmp2:AddMaterial(lambert2)
actor2:AddComponent(cmp2)
actor2:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp2:CreateSphereShape(Vector3(0,0,0),Quaternion.Identity(),1,1)

-- 在角色手部绑定物体
actor3=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp3=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp3.MeshPath="mesh/Cylinder.mesh.xml"
cmp3:AddMaterial(lambert2:Clone())
actor3:AddComponent(cmp3)
actor3:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp3:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,1)

-- 在角色手部绑定物体
actor4=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp4=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp4.MeshPath="mesh/Cylinder.mesh.xml"
cmp4:AddMaterial(lambert2:Clone())
actor4:AddComponent(cmp4)
actor4:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp4:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,1)

-- 搞一个trigger
actor5=Actor(NamedObject.MakeUniqueName("actor"),Vector3(3,1,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp5=StaticEmitterComponent(NamedObject.MakeUniqueName("mesh_cmp"),1)
actor5:AddComponent(cmp5)
actor5:CreateRigidActor(Actor.eRIGID_STATIC)
cmp5:CreateBoxShape(Vector3(0,0,0),Quaternion(0,0,0,1),1,1,1,2)
cmp5:SetShapeFlag(Component.eSIMULATION_SHAPE,false)
cmp5:SetShapeFlag(Component.eTRIGGER_SHAPE,true)
actor5.EventEnterTrigger=function(arg)
	print("enter trigger: "..arg.other.Name)
end
actor5.EventLeaveTrigger=function(arg)
	print("leave trigger: "..arg.other.Name)
end

game.EventLoadScene=function(arg)
	SPlayer.player:SetPose(Vector3(0,3,0),Quaternion.Identity())
	game:AddEntity(actor2ent(SPlayer.player),SPlayer.player.aabb:transform(SPlayer.player.World),1.0,0.1)
	SPlayer.player.EventEnterView=function(arg)
		arg.self:PlayAction(SAction.act_tuowei)
	end
	
	actor3:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor3.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	game:AddEntity(actor2ent(actor3),actor3.aabb:transform(actor3.World),1.0,0.1)
	SPlayer.player:Attach(actor3, 10)
	
	actor4:SetRigidBodyFlag(Actor.eKINEMATIC,true)
	for cmp in actor4.Cmps do
		cmp.SimulationFilterWord0 = 2
		cmp.QueryFilterWord0 = 2
	end
	game:AddEntity(actor2ent(actor4),actor4.aabb:transform(actor4.World),1.0,0.1)
	SPlayer.player:Attach(actor4, 29)
	
	actor2:SetPose(Vector3(0,1,-5),Quaternion.Identity())
	game:AddEntity(actor2ent(actor2),actor2.aabb:transform(actor2.World),1.0,0.1)
	actor2.EventEnterView=function(arg)
		arg.self:PlayAction(SAction.act_sound)
	end
	
	actor5:UpdateWorld()
	game:AddEntity(actor2ent(actor5),actor5.aabb:transform(actor5.World),1.0,0.1)
	
	SAction.act_moving_track.ParamStartPos=Vector3(-3,1,0)
	SAction.act_moving_track.ParamEndPos=Vector3(-3,1,-5)
	local actor6 = game:GetNamedObject("editor_actor_1")
	actor6.EventEnterView=function(arg)
		arg.self:PlayAction(SAction.act_moving)
	end
	
	-- SPlayer.player:Detach(actor3);actor3:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor3.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end;SPlayer.player:Detach(actor4);actor4:SetRigidBodyFlag(Actor.eKINEMATIC,false);for cmp in actor4.Cmps do cmp.SimulationFilterWord0=1;cmp.QueryFilterWord0=1 end
	-- SAction.act_pose_track.ParamStartPos=Vector3(0,3,0);SAction.act_pose_track.ParamEndPos=Vector3(-3,3,0);SPlayer.player:PlayAction(SAction.act_pose)
end

-- 加载场景资源
game:LoadScene("scene01.xml")

-- 特殊渲染选项
-- game.SsaoEnable=true
-- game:SetVisualizationParameter(PhysxSceneContext.eSCALE,1)
-- game:SetVisualizationParameter(PhysxSceneContext.eCOLLISION_SHAPES,1)
-- game:SetVisualizationParameter(PhysxSceneContext.eCOLLISION_FNORMALS,1)
-- game:SetVisualizationParameter(PhysxSceneContext.eCOLLISION_AABBS,1)
-- game:SetControllerDebugRenderingFlags(PhysxSceneContext.eALL)
