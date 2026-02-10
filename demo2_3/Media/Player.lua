module("SPlayer", package.seeall)
require "Action.lua"

-- -- 修正不规范资源
-- local mesh=client:LoadMesh("character/casual19_m_highpoly.mesh.xml","")
-- mesh:Transform(Matrix4.Compose(
	-- Vector3(1,1,1),Quaternion.Identity(),Vector3(0,-95,0)))
-- mesh:SaveOgreMesh("Media/character/casual19_m_highpoly.mesh.xml")
-- local mesh2=client:LoadMesh("mesh/Cylinder.mesh.xml","")
-- mesh2:Transform(Matrix4.Compose(
	-- Vector3(0.1,0.25,0.1), Quaternion.RotationYawPitchRoll(0,0,math.rad(90)),Vector3(0.25,0,0)))
-- mesh2:SaveOgreMesh("Media/mesh/Cylinder.mesh.xml")

-- 创建Player主体
player=Actor("local_player",Vector3(0,3,0),Quaternion.Identity(),Vector3(0.1),AABB(-100,100))
local controller_cmp=Controller(NamedObject.MakeUniqueName("controller_cmp"),1.5,0.1,0.1,0.5,0)
player:InsertComponent(controller_cmp)

-- 模型材质
for i=0,3 do
	local cmp=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
	cmp.MeshPath="character/jack.mesh.xml"
	cmp.MeshSubMeshId=i
	cmp.Material=Material()
	cmp.Material.Shader="shader/mtl_BlinnPhong.fx"
	cmp.Material.PassMask=Material.PassMaskShadowNormalOpaque
	cmp.Material:SetParameter("g_DiffuseTexture", "character/jack.dds")
	cmp.Material:SetParameter("g_NormalTexture", "texture/Normal.dds")
	cmp.Material:SetParameter("g_SpecularTexture", "character/jack_s.dds")
	player:InsertComponent(cmp)
end

-- 构建动画树
local rate_walk=AnimationNodeSequence("clip_run",1.0,true,"move")
local node_walk=AnimationNodeBlendList("node_walk",2)
node_walk:SetChild(0,AnimationNodeSequence("clip_stand"))
node_walk:SetChild(1,rate_walk)
local rate_run=AnimationNodeSequence("clip_run",1.0,true,"move")
local node_run=AnimationNodeBlendList("node_run",2)
node_run:SetChildAdopt(0,node_walk)
node_run:SetChild(1,rate_run)
local node_run_slot=AnimationNodeSlot("node_run_slot")
node_run_slot:SetChildAdopt(0,node_run)

-- 加载动画资源
local animator_cmp=Animator(NamedObject.MakeUniqueName("anim_cmp"))
animator_cmp:SetChild(0,node_run_slot)
animator_cmp:ReloadSequenceGroup()
animator_cmp.SkeletonPath="character/jack.skeleton.xml"
animator_cmp:AddIK(SAction.skel:GetBoneIndex("joint1"), 0.1, 1)
animator_cmp:AddIK(SAction.skel:GetBoneIndex("joint82"), 0.1, 1)
-- player:InsertComponent(animator_cmp)

-- 角色行为
class 'PlayerBehavior'(Component)
function PlayerBehavior:__init(name)
	Component.__init(self,name)
	self.velocity=Vector3(0,0,0)
	self.LookDist=3
end
function PlayerBehavior:RequestResource()
	Component.RequestResource(self)
	self.Actor:PlayAction(SAction.act_tuowei)
	client.Camera.Euler=Vector3(0,0,0)
end
function PlayerBehavior:ReleaseResource()
	Component.ReleaseResource(self)
end
function PlayerBehavior:Update(elapsedTime)
	-- 更新角色位置
	self.Actor:SetPose(controller_cmp.FootPosition)
	self.velocity.y=self.velocity.y-9.81*elapsedTime
	local speed=2
	local direction=Vector3(0,0,0)
	if not Control.GetFocusControl() then
		-- 键盘按下事件
		if client.keyboard:IsKeyPress(57) then
			self.velocity.y=5.0
			self.Actor:PlayAction(SAction.act_jump)
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
	
	-- 更新相机
	local LookMatrix=Matrix4.RotationYawPitchRoll(client.Camera.Euler.y,client.Camera.Euler.x,client.Camera.Euler.z)
	client.Camera.Eye=self.Actor.Position+Vector3(0,controller_cmp.Height,0)+LookMatrix.row2.xyz*self.LookDist
	client.Camera:UpdateViewProj()
	client.SkyLightCam.Eye=self.Actor.Position
	client.ViewedCenter=self.Actor.Position

	-- 更新音效
	client.listener:SetPosition(self.Actor.Position,Sound3DBuffer.DS3D_DEFERRED)
	client.listener:SetVelocity(self.velocity,Sound3DBuffer.DS3D_DEFERRED)
	client.listener:SetOrientation(LookMatrix.row2.xyz,LookMatrix.row1.xyz,Sound3DBuffer.DS3D_DEFERRED)
	client.listener:CommitDeferredSettings()
end
function PlayerBehavior:OnPxThreadSubstep(dtime)
	local disp=self.velocity*dtime
	local moveFlag=controller_cmp:Move(disp,0.001,dtime,1)
	if ret or bit.band(moveFlag,Controller.eCOLLISION_DOWN) ~= 0 then
		self.velocity.y=0
	end
	self.Actor:SetPxPoseOrbyPxThread(controller_cmp.FootPosition,self.Actor.Rotation,controller_cmp)
end
function PlayerBehavior:OnPxThreadShapeHit(arg)
	-- print("shape hit: "..arg.other.Name.."pos("..arg.worldPos.x..","..arg.worldPos.y..","..arg.worldPos.z..") nol("..arg.worldNormal.x..","..arg.worldNormal.y..","..arg.worldNormal.z..") dir("..arg.dir.x..","..arg.dir.y..","..arg.dir.z..")")
end
function PlayerBehavior:OnPxThreadControllerHit(arg)
	print("controller hit: "..arg.other.Name)
end
function PlayerBehavior:OnPxThreadObstacleHit(arg)
	print("obscale hit:..")
end
function PlayerBehavior:OnGUI(ui_render,elapsedTime,dim)
	-- print("BehaviorComponent:OnGUI",viewport.x,viewport.y)
	if client.joystick then
		ui_render:PushString(Rectangle.LeftTop(10,10,10,10),
			string.format("joystick0:\nX: %d\nY: %d\nZ: %d\nRX: %d\nRY: %d\nRZ: %d\nPov: %d\nBtn: ",
			client.joystick.X,
			client.joystick.Y,
			client.joystick.Z,
			client.joystick.Rx,
			client.joystick.Ry,
			client.joystick.Rz,
			bit.tobit(client.joystick:GetPov(0)))..table.concat(
				(function()
					local t={}
					for i=0,31 do
						if client.joystick:IsButtonDown(i) then
							t[#t+1]=tostring(i)
						end
					end
					return t
				end)(), ", "
			),0xffffffff,Font.AlignLeftTop,0xff000000,1,client.Font)
	end
end
local player_behavior=PlayerBehavior(NamedObject.MakeUniqueName('player_behavior'))
player:InsertComponentAdopt(player_behavior)

-- 动画在behavior之后插入是为了同步Update顺序
player:InsertComponent(animator_cmp)
