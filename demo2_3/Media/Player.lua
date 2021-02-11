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
player=Actor("local_player",Vector3(0,3,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
local character_cmp=Character(NamedObject.MakeUniqueName("character_cmp"),1.5,0.1,0.1,1)
player:AddComponent(character_cmp)

-- 模型材质
local lambert1=Material()
lambert1.Shader="shader/mtl_BlinnPhong.fx"
lambert1.PassMask=Material.PassMaskShadowNormalOpaque
lambert1:AddParameterTexture("g_DiffuseTexture", "character/casual19_m_35.jpg")
lambert1:AddParameterTexture("g_NormalTexture", "character/casual19_m_35_normal.png")
lambert1:AddParameterTexture("g_SpecularTexture", "character/casual19_m_35_spec.png")

-- 模型
local cmp=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp.MeshPath="character/casual19_m_highpoly.mesh.xml"
cmp.MeshSubMeshName=""
cmp.Material=lambert1
cmp.bUseAnimation=true
player:AddComponent(cmp)

-- 构建动画树
local seq_idle=AnimationNodeSequence()
seq_idle.Name="idle1"
local seq_walk=AnimationNodeSequence()
seq_walk.Name="walk"
seq_walk.Group="move"
local rate_walk=AnimationNodeRate()
rate_walk.Speed0=1.2
rate_walk.Child0=seq_walk
local node_walk=AnimationNodeBlend()
node_walk.Speed0=1.0
node_walk.Child0=seq_idle
node_walk.Child1=rate_walk
local seq_run=AnimationNodeSequence()
seq_run.Name="run"
seq_run.Group="move"
local rate_run=AnimationNodeRate()
rate_run.Speed0=7
rate_run.Child0=seq_run
local node_run=AnimationNodeBlend()
node_run.Speed0=5.0
node_run.Child0=node_walk
node_run.Child1=rate_run

-- 加载动画资源
local anim=AnimationRoot(player)
anim.Child0=node_run
anim:ReloadSequenceGroup()
anim.SkeletonPath="character/casual19_m_highpoly.skeleton.xml"
anim.SkeletonEventReady=function(arg)
	-- arg.self:AddJiggleBone("Bip01_R_Forearm",0.01,0.01,-10)
	arg.self:AddIK("Bip01_L_Thigh", 0.1, character_cmp.filterWord0)
	arg.self:AddIK("Bip01_R_Thigh", 0.1, character_cmp.filterWord0)
end
player.Animation=anim

local velocity=Vector3(0,0,0)
local LookDist=3
player.EventUpdate=function(arg)
	velocity.y=velocity.y-9.81*game.ElapsedTime
	local speed=2
	local direction=Vector3(0,0,0)
	if not Control.GetFocusControl() then
		-- 键盘按下事件
		if game.keyboard:IsKeyPress(57) then
			velocity.y=5.0
			arg.self:PlayAction(SAction.act_jump)
		end
		
		if game.keyboard:IsKeyDown(17) then
			direction.z=1
		elseif game.keyboard:IsKeyDown(31) then
			direction.z=-1
		else
			direction.z=0
		end
		
		if game.keyboard:IsKeyDown(30) then
			direction.x=1
		elseif game.keyboard:IsKeyDown(32) then
			direction.x=-1
		else
			direction.x=0
		end
		
		if game.keyboard:IsKeyDown(42) then
			speed=10
			node_run:SetActiveChild(1,0.1)
			rate_run.Rate=speed/7
		else
			node_run:SetActiveChild(0,0.1)
		end
		
		-- 鼠标移动事件
		game.Camera.Euler.y=game.Camera.Euler.y-math.rad(game.mouse:GetX())
		game.Camera.Euler.x=game.Camera.Euler.x-math.rad(game.mouse:GetY())
		LookDist=LookDist-game.mouse:GetZ()/480.0
	end
	
	local lengthsq=direction:magnitudeSq()
	if lengthsq > 0.000001 then
		direction=direction*(1/math.sqrt(lengthsq))
		local angle=math.atan2(direction.x,direction.z)+game.Camera.Euler.y+math.pi
		local localAngle=arg.self.Rotation:toEulerAngles().y
		local delta=Round(angle-localAngle,-math.pi,math.pi)
		localAngle=localAngle+delta*(1.0-math.pow(0.8,30*game.ElapsedTime))
		arg.self.Rotation=Quaternion.RotationEulerAngles(0,localAngle,0)
		local moveDir=Quaternion.RotationEulerAngles(0,angle,0)*Vector3(0,0,1)
		velocity.x=moveDir.x*speed
		velocity.z=moveDir.z*speed
		node_walk:SetActiveChild(1,0.1)
		rate_walk.Rate=speed/1.2
	else
		velocity.x=0
		velocity.z=0
		node_walk:SetActiveChild(0,0.1)
	end
	local moveFlag=character_cmp:Move(velocity*game.ElapsedTime,0.001,game.ElapsedTime)
	if bit.band(moveFlag,Character.eCOLLISION_DOWN) ~= 0 then
		velocity.y=0
	end
	
	local LookMatrix=Matrix4.RotationYawPitchRoll(game.Camera.Euler.y,game.Camera.Euler.x,game.Camera.Euler.z)
	game.Camera.Eye=arg.self.Position+Vector3(0,0.75,0)+LookMatrix.row2.xyz*LookDist
	game.SkyLightCam.Eye=arg.self.Position
	game.ViewedCenter=arg.self.Position
end

-- 处理碰撞事件
player.EventShapeHit=function(arg)
	-- print("shape hit: "..arg.other.Name.."pos("..arg.worldPos.x..","..arg.worldPos.y..","..arg.worldPos.z..") nol("..arg.worldNormal.x..","..arg.worldNormal.y..","..arg.worldNormal.z..") dir("..arg.dir.x..","..arg.dir.y..","..arg.dir.z..")")
end
