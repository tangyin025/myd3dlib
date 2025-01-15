-- MarchingCubes测试，至少需要有地形用来做碰撞
local terrain=theApp:GetNamedObject("actor0_terrain0")
assert(terrain and terrain.ComponentType==Component.ComponentTypeTerrain)

-- 加载cube默认mesh
cube_mesh=theApp:LoadMesh("mesh/MarchingCubes.mesh.xml")

-- id表示为三维空间坐标，由于double有效数为9007199254740992，所以用1024^3足够，哈希值：x*1024^2+y*1024+z
function coordToId(x,y,z)
	return x*1024^2+y*1024+z
end

-- 那么当某个hash反推：x=math.floor(v/1024/1024)、y=math.floor(v/1024%1024)、z=v%1024
function idToCoord(id)
	return math.floor(id/1024/1024),math.floor(id/1024%1024),id%1024
end

-- 可自绘的cmp
class 'EditorBehavior'(Component)
function EditorBehavior:__init(name)
	print("EditorBehavior:__init")
    Component.__init(self,name)
	self._name=name
	self.SignatureFlags=bit.bor(Component.SignatureFlagAddToPipeline)
	self.grids={}
	self.cubes={}
	self.Material=Material()
	self.Material.Shader="shader/mtl_BlinnPhong.fx"
	self.Material.PassMask=Material.PassMaskShadowNormalOpaque
	self.Material.CullMode=Material.D3DCULL_NONE
	self.Material:SetParameter("g_DiffuseTexture","texture/Checker.bmp")
	self.Material:SetParameter("g_NormalTexture","texture/Normal.dds")
	self.Material:SetParameter("g_SpecularTexture","texture/White.dds")
end

function EditorBehavior:__finalize()
    -- called when the an object is collected
	print("EditorBehavior:__finalize",self._name)
end

function EditorBehavior:Update(elapsedTime)
	-- 防止拖动时卡顿
	local view=theApp.MainWnd.ActiveView
	if view.IsCaptured then
		return
	end

	-- 鼠标射线地形碰撞点
	local pt=view.CursorPos
	local desc=view.SwapChainBufferDesc
	local ray=view.Camera:CalculateRay(Vector2(pt.x,pt.y),CSize(desc.Width,desc.Height))
	local terrain_loc_ray=ray:transform(terrain.Actor.World.inverse)
	local res,chunkid=terrain:RayTest(terrain_loc_ray,view.Camera.LookAt)
	if res.first then
		-- 将射线转换为cmp本地坐标
		res.second=(terrain_loc_ray.d*res.second):transformNormal(terrain.Actor.World*self.Actor.World.inverse).magnitude
	end

	-- 鼠标射线和grids碰撞点
	local local_ray=ray:transform(self.Actor.World.inverse)
	local local_grid_id=nil
	for k,grid in pairs(self.grids) do
		local x,y,z=idToCoord(k)
		local local_res=IntersectionTests.rayAndAABB(local_ray.p,local_ray.d,AABB(Vector3(x,y,z),0.5))
		if local_res.first and local_res.second<res.second then
			res.first=true
			res.second=local_res.second
			local_grid_id=k
		end
	end

	-- 合成碰撞点
	if res.first then
		local pos=local_ray.p+local_ray.d*(res.second-0.1)
		pos.x=local_ray.d.x<0 and math.ceil(pos.x-0.5) or math.floor(pos.x+0.5)
		pos.y=local_ray.d.y<0 and math.ceil(pos.y-0.5) or math.floor(pos.y+0.5)
		pos.z=local_ray.d.z<0 and math.ceil(pos.z-0.5) or math.floor(pos.z+0.5)
		self.target_pos=pos

		-- 设置grids，同时更新grid周围cubes
		if theApp.keyboard:IsKeyPress(--[[Keyboard.KC_Z--]]0x2C) then
			self:AddGrid(self.target_pos.x,self.target_pos.y,self.target_pos.z)
		elseif theApp.keyboard:IsKeyPress(--[[Keyboard.KC_X--]]0x2D) and local_grid_id then
			local local_x,local_y,local_z=idToCoord(local_grid_id)
			self:RemoveGrid(local_x,local_y,local_z)
		end
	end
end

function EditorBehavior:AddGrid(x,y,z)
	self.grids[coordToId(x,y,z)]=1
	self:UpdateTileRoundTheGrid(x,y,z,true)
end

function EditorBehavior:RemoveGrid(x,y,z)
	self.grids[coordToId(x,y,z)]=nil
	self:UpdateTileRoundTheGrid(x,y,z,false)
end

function EditorBehavior:UpdateTileRoundTheGrid(x,y,z,inside)
	for i=0,1 do for j=0,1 do for k=0,1 do
		local pos=i*4+j*2+k
		local ipos=bit.band(bit.bnot(pos),0x7)
		local mask=bit.lshift(0x1,ipos)
		local cube=self.cubes[coordToId(x+i,y+j,z+k)]
		if cube then
			if inside then
				self.cubes[coordToId(x+i,y+j,z+k)]=bit.bor(cube,mask)
			else
				cube=bit.band(cube,bit.bnot(mask))
				if 0==cube then
					self.cubes[coordToId(x+i,y+j,z+k)]=nil
				else
					self.cubes[coordToId(x+i,y+j,z+k)]=cube
				end
			end
		else
			if inside then
				self.cubes[coordToId(x+i,y+j,z+k)]=mask
			end
		end
	end end end
end

function EditorBehavior:OnSetShader(context,shader,lparam)
	-- print("EditorBehavior:OnSetShader")
	local x,y,z=idToCoord(lparam)
	shader:SetMatrix(shader:GetParameterByName(0,"g_World"),Matrix4.Translation(x-0.5,y-0.5,z-0.5)*self.Actor.World)
	shader:SetVector(shader:GetParameterByName(0,"g_MeshColor"),Vector4(0,1,1,1))
end

function EditorBehavior:AddToPipeline(frustum,pipeline,PassMask,ViewPos,TargetPos)
	-- print("EditorBehavior:AddToPipeline")
	if bit.band(bit.lshift(1,RenderPipeline.PassTypeNormal),PassMask)~=0 then
		-- 绘制调试信息
		local view=theApp.MainWnd.RenderingView
		view:PushLineAABB(self.Actor.OctAabb,ARGB(255,0,255,0))
		if self.target_pos then
			view:PushLineAABB(AABB(self.target_pos,0.5):transform(self.Actor.World),ARGB(255,255,0,255))
		end
	end
	for pass=RenderPipeline.PassTypeShadow,RenderPipeline.PassTypeTransparent do
		if bit.band(bit.lshift(1,pass),self.Material.PassMask,PassMask)~=0 then
			local shader=pipeline:QueryShader({MESH_TYPE="0"},"shader/mtl_BlinnPhong.fx",pass)
			if shader then
				-- 渲染cubes
				for k,cube in pairs(self.cubes) do
					if cube>0 and cube<255 then
						pipeline:PushMesh(pass,cube_mesh,cube-1,shader,self,self.Material,k)
					end
				end
			end
		end
	end
end

function EditorBehavior:OnGUI(ui_render,elapsedTime,dim)
	-- print("EditorBehavior:OnGUI")
	local pt=theApp.MainWnd.RenderingView.Camera:WorldToScreen(self.Actor.Position,dim)
	pt.x=math.floor(pt.x)
	pt.y=math.floor(pt.y)
	ui_render:PushString(Rectangle(pt.xy),"测试",ARGB(255,255,255,255),Font.AlignLeftTop,theApp.Font)

	-- 将所有cube字段输出
	for k,cube in pairs(self.cubes) do
		local x,y,z=idToCoord(k)
		local trans=Matrix4.Translation(x-0.5,y-0.5,z-0.5)*self.Actor.World
		local pt=theApp.MainWnd.RenderingView.Camera:WorldToScreen(trans.row3.xyz,dim)
		ui_render:PushString(Rectangle(pt.xy),tostring(cube),ARGB(255,255,0,255),Font.AlignLeftTop,theApp.Font)
	end
end

-- 创建基本act用来测试自绘
local cmp=EditorBehavior(NamedObject.MakeUniqueName("editor_behavior"))
local act=Actor(NamedObject.MakeUniqueName("actor"),Vector3(-10),Quaternion.Identity(),Vector3(1),AABB(0,20))
act:InsertComponentAdopt(cmp)
theApp.MainWnd:AddEntity(act)
theApp.MainWnd:PushToActorList(act)
