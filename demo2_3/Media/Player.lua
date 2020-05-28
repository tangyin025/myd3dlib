require "Action.lua"
module("SPlayer", package.seeall)

-- 修正不规范资源
mesh=game:LoadMesh("character/casual19_m_highpoly.mesh.xml","")
mesh:Transform(Matrix4.Compose(
	Vector3(0.01,0.01,0.01),Quaternion.Identity(),Vector3(0,-0.95,0)))
-- mesh:SaveOgreMesh("Media/character/casual19_m_highpoly.mesh.xml")
skel=game:LoadSkeleton("character/casual19_m_highpoly.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
skel:Transform(Matrix4.Compose(
	Vector3(0.01,0.01,0.01),Quaternion.Identity(),Vector3(0,-0.95,0)))
-- skel:SaveOgreSkeletonAnimation("Media/character/casual19_m_highpoly.skeleton.xml")
mesh2=game:LoadMesh("mesh/Cylinder.mesh.xml","")
mesh2:Transform(Matrix4.Compose(
	Vector3(0.1,0.25,0.1), Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.25,0,0)))
-- mesh2:SaveOgreMesh("Media/mesh/Cylinder.mesh.xml")

-- 创建Player主体
player=Player("local_player",Vector3(0,3,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1), 1.5, 0.1, 0.1, 1)

-- 模型材质
local lambert1=Material()
lambert1.Shader="shader/mtl_lambert1.fx"
lambert1.PassMask=Material.PassMaskShadowNormalOpaque
lambert1:AddParameterTexture("g_DiffuseTexture", "character/casual19_m_35.jpg")
lambert1:AddParameterTexture("g_NormalTexture", "character/casual19_m_35_normal.png")
lambert1:AddParameterTexture("g_SpecularTexture", "character/casual19_m_35_spec.png")

-- 模型
local cmp=MeshComponent()
cmp.MeshPath="character/casual19_m_highpoly.mesh.xml"
cmp.MeshSubMeshName=""
cmp:AddMaterial(lambert1)
cmp.bUseAnimation=true
player:AddComponent(cmp)

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
	-- anim:AddJiggleBone("Bip01_R_Forearm",0.01,0.01,-10)
	anim:AddIK("Bip01_L_Thigh", 0.1, 1)
	anim:AddIK("Bip01_R_Thigh", 0.1, 1)
end
player.Animation=anim

-- 鼠标移动事件
player.EventMouseMove=function(arg)
	if Control.GetFocusControl() then
		return
	end
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

-- 键盘按下事件
player.EventKeyDown=function(arg)
	if Control.GetFocusControl() then
		return
	end
	if arg.kc == 57 then
		player.Velocity.y=5.0
		player.State=Character.CharacterState.StateHang
		-- player.Animation:Play("jumpforward","",2,0.3,0.3,false,1,0,"")
		player:PlayAction(SAction.act_jump)
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

-- 键盘抬起事件
player.EventKeyUp=function(arg)
	if Control.GetFocusControl() then
		return
	end
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
