require "Settings.lua"
require "Hud.lua"

-- 设置相机
local k=math.cos(math.rad(45))
local d=20
game.Camera.Eye=Vector3(d*k*k,d*k+1,d*k*k)
game.Camera.Eular=Vector3(math.rad(-45),math.rad(45),0)
game.Camera.EventAlign=function(args)
	local desc=game.BackBufferSurfaceDesc
	game.Camera.Aspect=desc.Width/desc.Height
end

-- 设置环境光
game.SkyLightCam.Eye=Vector3(0,0,0)
game.SkyLightCam.Eular=Vector3(math.rad(-30),math.rad(0),0)
game.SkyLightCam.Width=50
game.SkyLightCam.Height=50
game.SkyLightCam.Nz=-50
game.SkyLightCam.Fz=50
game.SkyLightDiffuse=Vector4(0.7,0.7,0.7,0.7)
game.SkyLightAmbient=Vector4(0.5,0.5,0.5,0.0)

-- 加载场景资源
game:LoadScene("scene01.xml")

-- -- 创建地面
-- local actor=Actor(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-512,512))
-- local cmp=MeshComponent()
-- local lambert1=Material()
-- lambert1.Shader="shader/mtl_lambert1.fx"
-- lambert1.PassMask=Material.PassMaskShadowNormalOpaque
-- -- lambert1.RepeatUV.x=64
-- -- lambert1.RepeatUV.y=64
-- lambert1:AddParameterTexture("g_DiffuseTexture", "texture/Checker.bmp")
-- lambert1:AddParameterTexture("g_NormalTexture", "texture/Normal.dds")
-- lambert1:AddParameterTexture("g_SpecularTexture", "texture/White.dds")
-- cmp:AddMaterial(lambert1)
-- cmp.MeshPath="mesh/plane.mesh.xml"
-- cmp.MeshEventReady=function(args)
	-- cmp.Mesh:Transform(Matrix4.Scaling(256,1,256))
-- end
-- actor:AddComponent(cmp)
-- actor:CreateRigidActor(Actor.eRIGID_STATIC)
-- cmp:CreatePlaneShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),1)
-- actor:UpdateWorld()
-- game:AddEntity(actor2oct(actor),actor.aabb:transform(actor.World))

-- 创建玩家Actor
local player=Character(Vector3(0,3,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1), 1.5, 0.1, 0.1)

-- 加载皮肤
local local_trans=Matrix4.Compose(Vector3(0.01,0.01,0.01),Quaternion.Identity(),Vector3(0,-0.95,0))
local lambert2=Material()
lambert2.Shader="shader/mtl_lambert1.fx"
lambert2.PassMask=Material.PassMaskShadowNormalOpaque
lambert2:AddParameterTexture("g_DiffuseTexture", "character/casual19_m_35.jpg")
lambert2:AddParameterTexture("g_NormalTexture", "character/casual19_m_35_normal.png")
lambert2:AddParameterTexture("g_SpecularTexture", "character/casual19_m_35_spec.png")
local cmp=MeshComponent()
cmp.MeshPath="character/casual19_m_highpoly.mesh.xml"
cmp.MeshEventReady=function(args)
	cmp.Mesh:Transform(local_trans)
end
cmp:AddMaterial(lambert2)
cmp.bUseAnimation=true
player:AddComponent(cmp)
player:UpdateWorld()
game:AddEntity(actor2oct(player),player.aabb:transform(player.World))

-- 加载动画资源
local anim=Animator(player)
anim.SkeletonPath="character/casual19_m_highpoly.skeleton.xml"
anim.SkeletonEventReady=function(args)
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
	anim.Skeleton:Transform(local_trans)
	-- anim:AddJiggleBone("Bip01_R_Forearm",0.01,0.01,-10)
	anim:AddIK("Bip01_L_Thigh", 0.1, 1)
	anim:AddIK("Bip01_R_Thigh", 0.1, 1)
end

-- 构建动画树
local seq_idle=AnimationNodeSequence(anim)
seq_idle.Name="idle1"
seq_idle.Root="Bip01"
local seq_walk=AnimationNodeSequence(anim)
seq_walk.Name="walk"
seq_walk.Root="Bip01"
seq_walk.Group="move"
local rate_walk=AnimationNodeRateBySpeed(anim)
rate_walk.Speed0=1.2
rate_walk.Child0=seq_walk
local node_walk=AnimationNodeBlendBySpeed(anim)
node_walk.Speed0=1.0
node_walk.Child0=seq_idle
node_walk.Child1=rate_walk
local seq_run=AnimationNodeSequence(anim)
seq_run.Name="run"
seq_run.Root="Bip01"
seq_run.Group="move"
local rate_run=AnimationNodeRateBySpeed(anim)
rate_run.Speed0=7
rate_run.Child0=seq_run
local node_run=AnimationNodeBlendBySpeed(anim)
node_run.Speed0=5.0
node_run.Child0=node_walk
node_run.Child1=rate_run
local node_slot=AnimationNodeSlot(anim)
node_slot.Child0=node_run
anim.Node=node_slot
anim.Node:OnSetOwner()
player.Animator=anim

-- 创建控制器
player.Controller=PlayerController(player)

-- 创建一个物理球
local actor4=Actor(Vector3(0,1,-5),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local lambert3=Material()
lambert3.Shader="shader/mtl_lambert1.fx"
lambert3.PassMask=Material.PassMaskShadowNormalOpaque
lambert3:AddParameterTexture("g_DiffuseTexture", "texture/Checker.bmp")
lambert3:AddParameterTexture("g_NormalTexture", "texture/Normal.dds")
lambert3:AddParameterTexture("g_SpecularTexture", "texture/White.dds")
local cmp2=MeshComponent()
cmp2.MeshPath="mesh/Sphere.mesh.xml"
cmp2:AddMaterial(lambert3)
actor4:AddComponent(cmp2)
actor4:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp2:CreateSphereShape(Vector3(0,0,0),Quaternion.Identity(),1,1)
actor4:UpdateWorld()
game:AddEntity(actor2oct(actor4),actor4.aabb:transform(actor4.World))

-- -- 在角色手部绑定物体
-- local actor2=Actor(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
-- local cmp3=MeshComponent()
-- cmp3.MeshPath="mesh/Cylinder.mesh.xml"
-- cmp3.MeshEventReady=function(args)
	-- cmp3.Mesh:Transform(Matrix4.Compose(Vector3(0.1,0.25,0.1),
		-- Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.25,0,0)))
-- end
-- cmp3:AddMaterial(lambert3)
-- actor2:AddComponent(cmp3)
-- actor2:CreateRigidActor(Actor.eRIGID_DYNAMIC)
-- actor2:SetRigidBodyFlag(Actor.eKINEMATIC,true)
-- cmp3:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,1)
-- game:AddEntity(actor2oct(actor2),actor2.aabb:transform(actor2.World))
-- player:Attach(actor2, 10)

-- -- 在角色手部绑定物体
-- local actor3=Actor(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
-- local cmp4=MeshComponent()
-- cmp4.MeshPath="mesh/Cylinder.mesh.xml"
-- cmp4:AddMaterial(lambert3)
-- actor3:AddComponent(cmp4)
-- actor3:CreateRigidActor(Actor.eRIGID_DYNAMIC)
-- actor3:SetRigidBodyFlag(Actor.eKINEMATIC,true)
-- cmp4:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,1)
-- game:AddEntity(actor2oct(actor3),actor3.aabb:transform(actor2.World))
-- player:Attach(actor3, 29)

-- -- 特殊渲染选项
-- game.SsaoEnable=true
-- game.VisualizationParameter=1