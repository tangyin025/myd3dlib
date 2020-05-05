require "Settings.lua"
require "Hud.lua"

-- 关闭控制台
game.Console.Visible=false

-- 设置相机
local k=math.cos(math.rad(45))
local d=20
game.Camera.Eye=Vector3(d*k*k,d*k+1,d*k*k)
game.Camera.Eular=Vector3(math.rad(-45),math.rad(45),0)
game.Camera.EventAlign=function(arg)
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
-- cmp.MeshEventReady=function(arg)
	-- cmp.Mesh:Transform(Matrix4.Scaling(256,1,256))
-- end
-- actor:AddComponent(cmp)
-- actor:CreateRigidActor(Actor.eRIGID_STATIC)
-- cmp:CreatePlaneShape(Vector3(0,0,0),Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),1)
-- actor:UpdateWorld()
-- game:AddEntity(actor2ent(actor),actor.aabb:transform(actor.World))

-- ActionTrack
act_jump=Action()
act_jump.Length=5.0
local track=ActionTrackAnimation()
track:AddKeyFrame(0,"jumpforward","",2.0,0.3,0.3,false,1,0,"")
act_jump:AddTrack(track)

act_env=Action()
act_env.Length=5.0
track=ActionTrackSound()
track:AddKeyFrame(0,"demo2_3/untitled/drumloop")
act_env:AddTrack(track)

act_tuowei=Action()
act_tuowei.Length=99999
local particle1=Material()
particle1.Shader="shader/mtl_particle1.fx"
particle1.PassMask=Material.PassMaskTransparent
particle1.ZWriteEnable=0
particle1.BlendMode=Material.BlendModeAdditive
particle1:AddParameterTexture("g_Texture", "texture/flare.dds")
local track=ActionTrackEmitter()
track.ParticleMaterial=particle1
track.ParticleFaceType=EmitterComponent.FaceTypeCamera
track.ParticleLifeTime=5
track.ParticleColorR:AddNode(0,1,0,0)
track.ParticleColorR:AddNode(3,0,0,0)
track.ParticleColorG:AddNode(0,1,0,0)
track.ParticleColorG:AddNode(3,0,0,0)
track.ParticleColorB:AddNode(0,1,0,0)
track.ParticleColorB:AddNode(3,0,0,0)
track.ParticleColorA:AddNode(0,1,0,0)
track.ParticleColorA:AddNode(3,0,0,0)
track.SpawnInterval=0.1
track.SpawnLength=99999
track:AddKeyFrame(0)
act_tuowei:AddTrack(track)

-- 创建玩家Actor
--[[local--]] player=Player(Vector3(0,3,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1), 1.5, 0.1, 0.1)
player.EventMouseMove=function(arg)
	if arg.x ~= 0 then
		player.LookAngle.y=player.LookAngle.y-math.rad(arg.x)
	end
	if arg.y ~= 0 then
		player.LookAngle.x=player.LookAngle.x-math.rad(arg.y)
	end
	if arg.z ~= 0 then
		player.LookDist=player.LookDist-arg.z/480.0
	end
end
player.EventKeyDown=function(arg)
	if arg.kc == 57 then
		player.Velocity.y=5.0
		-- player.Animation:Play("jumpforward","",2,0.3,0.3,false,1,0,"")
		player:PlayAction(act_jump)
	end
	if arg.kc == 17 then
		player.MoveAxis.y=player.MoveAxis.y+1
	end
	if arg.kc == 30 then
		-- player.Animation:Play("jumpforward","",2,0.3,0.3,false,1,1,"move")
		player.MoveAxis.x=player.MoveAxis.x+1
	end
	if arg.kc == 31 then
		player.MoveAxis.y=player.MoveAxis.y-1
	end
	if arg.kc == 32 then
		player.MoveAxis.x=player.MoveAxis.x-1
	end
end
player.EventKeyUp=function(arg)
	if arg.kc == 17 then
		player.MoveAxis.y=player.MoveAxis.y-1
	end
	if arg.kc == 30 then
		player.MoveAxis.x=player.MoveAxis.x-1
	end
	if arg.kc == 31 then
		player.MoveAxis.y=player.MoveAxis.y+1
	end
	if arg.kc == 32 then
		player.MoveAxis.x=player.MoveAxis.x+1
	end
end

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
cmp.MeshEventReady=function(arg)
	cmp.Mesh:Transform(local_trans)
end
cmp:AddMaterial(lambert2)
cmp.bUseAnimation=true
player:AddComponent(cmp)
player:UpdateWorld()
game:AddEntity(actor2ent(player),player.aabb:transform(player.World))

-- 构建动画树
local seq_idle=AnimationNodeSequence()
seq_idle.Name="idle1"
local seq_walk=AnimationNodeSequence()
seq_walk.Name="walk"
seq_walk.Group="move"
local rate_walk=AnimationNodeRateBySpeed()
rate_walk.Speed0=1.2
rate_walk.Child0=seq_walk
local node_walk=AnimationNodeBlendBySpeed()
node_walk.Speed0=1.0
node_walk.Child0=seq_idle
node_walk.Child1=rate_walk
local seq_run=AnimationNodeSequence()
seq_run.Name="run"
seq_run.Group="move"
local rate_run=AnimationNodeRateBySpeed()
rate_run.Speed0=7
rate_run.Child0=seq_run
local node_run=AnimationNodeBlendBySpeed()
node_run.Speed0=5.0
node_run.Child0=node_walk
node_run.Child1=rate_run

-- 加载动画资源
local anim=AnimationRoot(player)
anim.Child0=node_run
anim:ReloadSequenceGroup()
anim.SkeletonPath="character/casual19_m_highpoly.skeleton.xml"
anim.SkeletonEventReady=function(arg)
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
	anim.Skeleton:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
	anim.Skeleton:Transform(local_trans)
	-- anim:AddJiggleBone("Bip01_R_Forearm",0.01,0.01,-10)
	anim:AddIK("Bip01_L_Thigh", 0.1, 1)
	anim:AddIK("Bip01_R_Thigh", 0.1, 1)
end
player.Animation=anim

-- 创建一个物理球
--[[local--]] actor4=Actor(Vector3(0,1,-5),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
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
game:AddEntity(actor2ent(actor4),actor4.aabb:transform(actor4.World))

-- 球体发声
actor4:PlayAction(act_env)

player:PlayAction(act_tuowei)

-- -- 在角色手部绑定物体
-- local actor2=Actor(Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
-- local cmp3=MeshComponent()
-- cmp3.MeshPath="mesh/Cylinder.mesh.xml"
-- cmp3.MeshEventReady=function(arg)
	-- cmp3.Mesh:Transform(Matrix4.Compose(Vector3(0.1,0.25,0.1),
		-- Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.25,0,0)))
-- end
-- cmp3:AddMaterial(lambert3)
-- actor2:AddComponent(cmp3)
-- actor2:CreateRigidActor(Actor.eRIGID_DYNAMIC)
-- actor2:SetRigidBodyFlag(Actor.eKINEMATIC,true)
-- cmp3:CreateCapsuleShape(Vector3(0.25,0,0),Quaternion.Identity(),0.1,0.25,1)
-- game:AddEntity(actor2ent(actor2),actor2.aabb:transform(actor2.World))
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
-- game:AddEntity(actor2ent(actor3),actor3.aabb:transform(actor2.World))
-- player:Attach(actor3, 29)

-- -- 特殊渲染选项
-- game.SsaoEnable=true
-- game.VisualizationParameter=1