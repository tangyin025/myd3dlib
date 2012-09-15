dofile "Loader.lua"
dofile "Hud.lua"

-- 获取当前state
local state=game:CurrentState()

-- 设置相机
local camera = state.Camera
camera.LookAt=Vector3(0,1,0)
camera.Distance=3

-- ! 利用EventAlign调整相机的Aspect
local d=Dialog();d.Visible=false;d.EventAlign=function(args) camera.Aspect=args.vp.x/args.vp.y end;game:InsertDlg(d)

-- 创建场景
local function CreateScene(n)
	state:InsertStaticMesh(LoadEffectMesh(n..".mesh.xml"))
end

-- 创建角色
local function CreateRole(n,p,t)
	local character = Character()
	character:InsertMeshLOD(LoadEffectMesh(n..".mesh.xml"))
	character:InsertSkeletonLOD(LoadSkeleton(n..".skeleton.xml"))
	character.Scale = Vector3(0.01,0.01,0.01)
	character.Position = p
	character.StateTime = t
	state:InsertCharacter(character)
end

-- -- CreateScene("plane")

CreateScene("water")

CreateRole("casual19_m_highpoly", Vector3(0,0,0), 0)

-- for i=-5,5 do
	-- for j = -5,5 do
		-- CreateRole("casual19_m_highpoly", Vector3(i,0,j), math.random(0,1))
	-- end
-- end