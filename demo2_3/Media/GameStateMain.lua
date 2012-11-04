
-- 获取当前state
local state=game:GetCurrentState()

-- -- 创建相机
-- local camera=ModelViewerCamera(math.rad(75),4/3.0,0.1,3000)
-- camera.Rotation=Vector3(math.rad(45),math.rad(-45),0)
-- camera.LookAt=Vector3(0,1,0)
-- camera.Distance=3
-- camera.EventAlign=function(args)
	-- camera.Aspect=args.vp.x/args.vp.y
-- end
-- state.Camera=camera

local camera=FirstPersonCamera(math.rad(75),4/3.0,0.1,3000)
local k=math.cos(math.rad(45))
local d=3
camera.Position=Vector3(d*k*k,d*k+1,-d*k*k)
camera.Rotation=Vector3(math.rad(45),math.rad(-45),0)
camera.EventAlign=function(args)
	camera.Aspect=args.vp.x/args.vp.y
end
state.Camera=camera

-- 读取材质
local function LoadMaterial(name)
	dofile("material/"..name..".lua")
	local mat = Material()
	SetupMaterial(mat)
	return mat
end

-- 读取mesh
local function LoadEffectMesh(name)
	local effectMesh = EffectMesh()
	effectMesh.Mesh = game:LoadMesh(name)
	for i = 0,effectMesh.Mesh:GetMaterialNum()-1 do
		effectMesh:InsertMaterial(LoadMaterial(effectMesh.Mesh:GetMaterialName(i)))
	end
	return effectMesh
end

-- 创建场景
local function CreateScene(n)
	state:InsertStaticMesh(LoadEffectMesh(n..".mesh.xml"))
end

-- 创建角色
local function CreateRole(n,p,t,s)
	local character=Character()
	character:InsertMeshLOD(LoadEffectMesh(n..".mesh.xml"))
	character:InsertSkeletonLOD(game:LoadSkeleton(n..".skeleton.xml"))
	character.Scale=Vector3(s,s,s)
	character.Position=p
	character.StateTime=t
	state:InsertCharacter(character)
end

-- CreateScene("plane")

CreateScene("water")

-- CreateRole("tube", Vector3(0,0,0), 0, 1)

CreateRole("casual19_m_highpoly", Vector3(0,0,0), 0, 0.01)

-- CreateRole("sportive03_f", Vector3(0,0,0), 0, 0.01)

-- for i=-5,5 do
	-- for j=-5,5 do
		-- CreateRole("casual19_m_highpoly", Vector3(i,0,j), math.random(0,1), 0.01)
	-- end
-- end