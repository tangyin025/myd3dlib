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

-- -- game.Player.MaxVelocity=10.0
-- -- game.Player.Resistance=50.0
-- -- toCharacterController(game.Player.Controller).RotationSpeed=3.14*3--crash client
-- local mesh=game:LoadMesh("mesh/cloth.mesh.xml")
-- local lambert1=Material()
-- lambert1.Shader="shader/lambert1.fx"
-- lambert1.PassMask=Material.PassMaskOpaque
-- lambert1.MeshTexture.Path="texture/Checker.bmp"
-- lambert1.NormalTexture.Path="texture/Normal.dds"
-- lambert1.SpecularTexture.Path="texture/White.dds"
-- local cmp=ClothComponent()
-- cmp:CreateClothFromMesh(mesh,1)
-- cmp:AddMaterial(lambert1)
-- game.Player:AddComponent(cmp)

local lambert1=Material()
lambert1.Shader="shader/lambert1.fx"
lambert1.PassMask=Material.PassMaskOpaque
lambert1.MeshTexture.Path="texture/Checker.bmp"
lambert1.NormalTexture.Path="texture/Normal.dds"
lambert1.SpecularTexture.Path="texture/White.dds"
local cmp=MeshComponent()
cmp.MeshRes.Path="mesh/casual19_m_highpoly.mesh.xml"
cmp:AddMaterial(lambert1)
cmp.bUseAnimation=true
game.Player:AddComponent(cmp)
game.Player.Scale=Vector3(0.05,0.05,0.05)

local anim=Animator(game.Player)
anim.SkeletonRes.Path="mesh/casual19_m_highpoly.skeleton.xml"
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
game.Player.Animator=anim

game.Player.Controller=PlayerController(game.Player)

local cmp2=MeshComponent()
cmp2.MeshRes.Path="mesh/Helix.mesh.xml"
cmp2.MeshRes.EventReady=function(args)
	cmp2.MeshRes.Res:Transform(Matrix4.Scaling(5,5,5))
end
cmp2:AddMaterial(lambert1)
local act2=Actor()
act2:AddComponent(cmp2)
game.Root:AddActor(actor2oct(act2),AABB(-1,1),0.1)
