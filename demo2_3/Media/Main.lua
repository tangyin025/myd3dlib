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
game.SkyLightDiffuse=Vector4(0.5,0.5,0.5,0.5)
game.SkyLightAmbient=Vector4(0.5,0.5,0.5,0.0)

-- 创建玩家Actor
local player=Character(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1), 1, 0.3)

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

-- 加载动画树
local anim=Animator(player)
anim.SkeletonRes.Path="character/casual19_m_highpoly.skeleton.xml"
anim.SkeletonRes.EventReady=function(args)
	anim.SkeletonRes.Res:Transform(local_trans)
end
local node_walk=AnimationNodeSequence(anim)
node_walk.Name="walk"
node_walk.Root="Bip01"
local node_idle=AnimationNodeSequence(anim)
node_idle.Name="idle"
node_idle.Root="Bip01"
local node_speed=AnimationNodeBlendBySpeed(anim)
node_speed.Child0=node_idle
node_speed.Child1=node_walk
anim.Node=node_speed
player.Animator=anim

-- 创建控制器
player.Controller=PlayerController(player)

-- 加入场景
player:UpdateWorld()
game.Root:AddActor(player,player.aabb:transform(player.World),0.1)

-- 特殊渲染选项
game.SsaoEnable=true
-- game.VisualizationParameter=1