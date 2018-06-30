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

-- 创建玩家Actor
local player=Character(Vector3(0,3,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1), 1, 0.3)

-- 加载皮肤
local local_trans=Matrix4.Compose(Vector3(0.01,0.01,0.01),Quaternion.Identity(),Vector3(0,-0.45,0))
local lambert1=Material()
lambert1.Shader="shader/lambert1.fx"
lambert1.PassMask=Material.PassMaskOpaque
lambert1.MeshTexture.Path="character/casual19_m_35.jpg"
lambert1.NormalTexture.Path="character/casual19_m_35_normal.png"
lambert1.SpecularTexture.Path="character/casual19_m_35_spec.png"
local cmp=MeshComponent()
cmp.MeshRes.Path="character/casual19_m_highpoly.mesh.xml"
cmp.MeshRes.EventReady=function(args)
	cmp.MeshRes.Res:Transform(local_trans)
end
cmp:AddMaterial(lambert1)
cmp.bUseAnimation=true
player:AddComponent(cmp)
player:UpdateWorld()
game.Root:AddActor(actor2oct(player),player.aabb:transform(player.World))

-- 加载动画资源
local anim=Animator(player)
anim.SkeletonRes.Path="character/casual19_m_highpoly.skeleton.xml"
anim.SkeletonRes.EventReady=function(args)
	anim.SkeletonRes.Res:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
	anim.SkeletonRes.Res:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
	anim.SkeletonRes.Res:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
	anim.SkeletonRes.Res:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
	anim.SkeletonRes.Res:Transform(local_trans)
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
node_slot.Root="Bip01"
anim.Node=node_slot
anim.Node:OnSetOwner()
player.Animator=anim

-- 创建控制器
player.Controller=PlayerController(player)

-- 在角色手部绑定物体
local actor2=Actor(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local lambert2=Material()
lambert2.Shader="shader/lambert1.fx"
lambert2.PassMask=Material.PassMaskOpaque
lambert2.MeshTexture.Path="texture/Checker.bmp"
lambert2.NormalTexture.Path="texture/Normal.dds"
lambert2.SpecularTexture.Path="texture/White.dds"
local cmp=MeshComponent()
cmp.MeshRes.Path="mesh/Cube.mesh.xml"
cmp:AddMaterial(lambert2)
actor2:AddComponent(cmp)
actor2:CreateRigidActor(Actor.eRIGID_DYNAMIC)
actor2:SetRigidBodyFlag(Actor.eKINEMATIC,true)
cmp:CreateBoxShape(Vector3(0,0,0),Quaternion.Identity(),0.5,0.5,0.5)
game.Root:AddActor(actor2oct(actor2),actor2.aabb:transform(actor2.World))

-- 创建一个物理球
local actor3=Actor(Vector3(0,5,-5),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local cmp=MeshComponent()
cmp.MeshRes.Path="mesh/Sphere.mesh.xml"
cmp:AddMaterial(lambert2)
actor3:AddComponent(cmp)
actor3:CreateRigidActor(Actor.eRIGID_DYNAMIC)
cmp:CreateSphereShape(Vector3(0,0,0),Quaternion.Identity(),1)
actor3:UpdateWorld()
game.Root:AddActor(actor2oct(actor3),actor3.aabb:transform(actor3.World))

-- 将物体连接到角色手里
player:Attach(actor2, 18)

-- 特殊渲染选项
game.SsaoEnable=true
game.VisualizationParameter=1