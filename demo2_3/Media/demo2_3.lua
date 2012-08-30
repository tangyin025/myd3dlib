-- font=Game.LoadFont("wqy-microhei.ttc", 13)
-- game.font=font
-- dofile "Console.lua"
dofile "Hud.lua"

-- camera=ModuleViewCamera(math.rad(75),4/3,0.1,3000)
-- camera.Rotation=Vector3(math.rad(-45),math.rad(45),0)
-- camera.Distance=10
-- scene=Scene()
-- scene.Camera=camera
-- game:InsertScene(scene)

local state=game:CurrentState()

local camera = state.Camera
camera.LookAt=Vector3(0,1,0)
-- camera.Distance=1

-- 利用 EventAlign调整相机的 Aspect
local d=Dialog();d.Visible=false;d.EventAlign=function(args) camera.Aspect=args.vp.x/args.vp.y end;game:InsertDlg(d)

-- 防止garbage collect时清理掉还在使用的资源
texture_pool = {}

local function CreateScene()
	local effectMesh = game:LoadEffectMesh("plane.mesh.xml")
	state:InsertStaticMesh(effectMesh)
end

local effectMesh = game:LoadEffectMesh("casual19_m_highpoly.mesh.xml")
local skeleton = game:LoadSkeleton("casual19_m_highpoly.skeleton.xml")
local function CreateRole(p)
	local character = Character()
	character:InsertMeshLOD(effectMesh)
	character:InsertSkeletonLOD(skeleton)
	character.Scale = Vector3(0.01,0.01,0.01)
	character.Position = p
	state:InsertCharacter(character)
end

-- CreateScene()

-- CreateRole(Vector3(0,0,0))

for i=-5,5 do
	for j = -5,5 do
		CreateRole(Vector3(i,0,j))
	end
end