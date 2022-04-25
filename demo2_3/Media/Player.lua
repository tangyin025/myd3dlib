module("SPlayer", package.seeall)
require "Action.lua"

-- 修正不规范资源
-- mesh=client:LoadMesh("character/casual19_m_highpoly.mesh.xml","")
-- mesh:Transform(Matrix4.Compose(
-- 	Vector3(1,1,1),Quaternion.Identity(),Vector3(0,-95,0)))
-- mesh:SaveOgreMesh("Media/character/casual19_m_highpoly.mesh.xml")
skel=client:LoadSkeleton("character/casual19_m_highpoly.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
skel:Transform(Matrix4.Compose(
	Vector3(1,1,1),Quaternion.Identity(),Vector3(0,-95,0)))
-- skel:SaveOgreSkeletonAnimation("Media/character/casual19_m_highpoly_full.skeleton.xml")
-- mesh2=client:LoadMesh("mesh/Cylinder.mesh.xml","")
-- mesh2:Transform(Matrix4.Compose(
	-- Vector3(0.1,0.25,0.1), Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.25,0,0)))
-- mesh2:SaveOgreMesh("Media/mesh/Cylinder.mesh.xml")

-- 创建Player主体
player=Actor("local_player",Vector3(0,3,0),Quaternion.Identity(),Vector3(0.01,0.01,0.01),AABB(-1,1))
local controller_cmp=Controller(NamedObject.MakeUniqueName("controller_cmp"),1.5,0.1,0.1)
player:InsertComponent(controller_cmp)

-- 模型材质
local lambert1=Material()
lambert1.Shader="shader/mtl_BlinnPhong.fx"
lambert1.PassMask=RenderPipeline.PassMaskShadowNormalOpaque
lambert1:AddParameter("g_DiffuseTexture", "character/casual19_m_35.jpg")
lambert1:AddParameter("g_NormalTexture", "character/casual19_m_35_normal.png")
lambert1:AddParameter("g_SpecularTexture", "character/casual19_m_35_spec.png")

-- 模型
local cmp=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
cmp.MeshPath="character/casual19_m_highpoly.mesh.xml"
cmp.MeshSubMeshName=""
cmp.Material=lambert1
player:InsertComponent(cmp)

-- 构建动画树
local rate_walk=AnimationNodeRate()
rate_walk.Speed0=1.2
rate_walk.Child0=AnimationNodeSequence("walk",1.0,true,"move")
local node_walk=AnimationNodeBlendList(2)
node_walk.Child0=AnimationNodeSequence("idle1")
node_walk.Child1=rate_walk
local rate_run=AnimationNodeRate()
rate_run.Speed0=7
rate_run.Child0=AnimationNodeSequence("run",1.0,true,"move")
local node_run=AnimationNodeBlendList(2)
node_run:SetChildAdopt(0,node_walk)
node_run.Child1=rate_run

-- 加载动画资源
local animator_cmp=Animator(NamedObject.MakeUniqueName("anim_cmp"))
animator_cmp:SetChildAdopt(0,node_run)
animator_cmp:ReloadSequenceGroup()
animator_cmp.SkeletonPath="character/casual19_m_highpoly.skeleton.xml"
client:LoadSkeletonAsync(animator_cmp.SkeletonPath, function(res)
	-- arg.self:AddJiggleBone("Bip01_R_Forearm",0.01,0.01,-10)
	animator_cmp:AddIK(res:GetBoneIndex("Bip01_L_Thigh"), res.boneHierarchy, 0.1, 1)
	animator_cmp:AddIK(res:GetBoneIndex("Bip01_R_Thigh"), res.boneHierarchy, 0.1, 1)
end, 0)
player:InsertComponent(animator_cmp)

-- 角色行为
class 'PlayerBehavior'(Component)
function PlayerBehavior:__init(name)
	Component.__init(self,name)
	self.velocity=Vector3(0,0,0)
	self.LookDist=3
end
function PlayerBehavior:RequestResource()
	Component.RequestResource(self)
	self.Actor:PlayAction(SAction.act_tuowei,9999)
	client.Camera.Euler=Vector3(0,0,0)
end
function PlayerBehavior:ReleaseResource()
	Component.ReleaseResource(self)
end
function PlayerBehavior:Update(elapsedTime)
	-- 更新角色位置
	self.Actor:SetPose(controller_cmp.Position)
	self.velocity.y=self.velocity.y-9.81*elapsedTime
	local speed=2
	local direction=Vector3(0,0,0)
	if not Control.GetFocusControl() then
		-- 键盘按下事件
		if client.keyboard:IsKeyPress(57) then
			self.velocity.y=5.0
			self.Actor:PlayAction(SAction.act_jump,5)
		end
		
		if client.keyboard:IsKeyDown(17) then
			direction.z=1
		elseif client.keyboard:IsKeyDown(31) then
			direction.z=-1
		else
			direction.z=0
		end
		
		if client.keyboard:IsKeyDown(30) then
			direction.x=1
		elseif client.keyboard:IsKeyDown(32) then
			direction.x=-1
		else
			direction.x=0
		end
		
		if client.keyboard:IsKeyDown(42) then
			speed=10
			node_run:SetActiveChild(1,0.1)
			rate_run.Rate=speed/7
		else
			node_run:SetActiveChild(0,0.1)
		end
		
		-- 鼠标移动事件
		client.Camera.Euler.y=client.Camera.Euler.y-math.rad(client.mouse.X)
		client.Camera.Euler.x=client.Camera.Euler.x-math.rad(client.mouse.Y)
		self.LookDist=self.LookDist-client.mouse.Z/480.0
	end
	
	local lengthsq=direction.magnitudeSq
	if lengthsq > 0.000001 then
		direction=direction*(1/math.sqrt(lengthsq))
		local angle=math.atan2(direction.x,direction.z)+client.Camera.Euler.y+math.pi
		local localAngle=self.Actor.Rotation.EulerAngles.y
		local delta=Wrap(angle-localAngle,-math.pi,math.pi)
		localAngle=localAngle+delta*(1.0-math.pow(0.8,30*elapsedTime))
		self.Actor.Rotation=Quaternion.RotationEulerAngles(0,localAngle,0)
		local moveDir=Quaternion.RotationEulerAngles(0,angle,0)*Vector3(0,0,1)
		self.velocity.x=moveDir.x*speed
		self.velocity.z=moveDir.z*speed
		node_walk:SetActiveChild(1,0.1)
		rate_walk.Rate=speed/1.2
	else
		self.velocity.x=0
		self.velocity.z=0
		node_walk:SetActiveChild(0,0.1)
	end
	
	-- 播放动画
	animator_cmp:Tick(elapsedTime,1.0)
	local LookMatrix=Matrix4.RotationYawPitchRoll(client.Camera.Euler.y,client.Camera.Euler.x,client.Camera.Euler.z)
	client.Camera.Eye=self.Actor.Position+Vector3(0,0.75,0)+LookMatrix.row2.xyz*self.LookDist
	client.SkyLightCam.Eye=self.Actor.Position
	client.ViewedCenter=self.Actor.Position

	-- 更新音效
	client.listener:SetPosition(self.Actor.Position,Sound3DBuffer.DS3D_DEFERRED)
	client.listener:SetVelocity(self.velocity,Sound3DBuffer.DS3D_DEFERRED)
	client.listener:SetOrientation(LookMatrix.row2.xyz,LookMatrix.row1.xyz,Sound3DBuffer.DS3D_DEFERRED)
	client.listener:CommitDeferredSettings()
end
function PlayerBehavior:OnPxThreadSubstep(dtime)
	local moveFlag=controller_cmp:Move(self.velocity*dtime,0.001,dtime,1)
	if bit.band(moveFlag,Controller.eCOLLISION_DOWN) ~= 0 then
		self.velocity.y=0
	end
end
function PlayerBehavior:OnPxThreadShapeHit(arg)
	-- print("shape hit: "..arg.other.Name.."pos("..arg.worldPos.x..","..arg.worldPos.y..","..arg.worldPos.z..") nol("..arg.worldNormal.x..","..arg.worldNormal.y..","..arg.worldNormal.z..") dir("..arg.dir.x..","..arg.dir.y..","..arg.dir.z..")")
end
function PlayerBehavior:OnGUI(ui_render,elapsedTime,viewport)
	-- print("BehaviorComponent:OnGUI",viewport.x,viewport.y)
end
local player_behavior=PlayerBehavior(NamedObject.MakeUniqueName('player_behavior'))
player:InsertComponentAdopt(player_behavior)
