dofile "CommonUI.lua"
dofile "Hud.lua"

-- -- 获取当前state
-- local state=game:CurrentState()

-- -- 创建相机
-- local camera=ModelViewerCamera(math.rad(75),4/3.0,0.1,3000)
-- camera.Rotation=Vector3(math.rad(-45),math.rad(45),0)
-- camera.LookAt=Vector3(0,1,0)
-- camera.Distance=20
-- camera.EventAlign=function(args)
	-- local desc=game:GetD3D9BackBufferSurfaceDesc()
	-- camera.Aspect=desc.Width/desc.Height
-- end
-- game.Camera=camera

local camera=FirstPersonCamera(math.rad(75),4/3.0,0.1,3000)
local k=math.cos(math.rad(45))
local d=20
camera.Position=Vector3(d*k*k,d*k+1,d*k*k)
camera.Rotation=Vector3(math.rad(-45),math.rad(45),0)
camera.EventAlign=function(args)
	local desc=game:GetD3D9BackBufferSurfaceDesc()
	camera.Aspect=desc.Width/desc.Height
end
game.Camera=camera

-- game:InsertMaterial("waterPhong", game:LoadMaterial("material/waterPhong.xml"))
-- game:InsertMaterial("casual19_m_highpolyPhong", game:LoadMaterial("material/casual19_m_highpolyPhong.xml"))
-- game:InsertMaterial("lambert1", game:LoadMaterial("material/lambert1.xml"))

-- -- 创建场景
-- state:InsertStaticMesh(game:LoadMesh("mesh/water.mesh.xml"))

-- -- 创建角色
-- local character=Character()
-- character.Mesh=game:LoadMesh("mesh/casual19_m_highpoly.mesh.xml")
-- character.Skeleton=game:LoadSkeleton("mesh/casual19_m_highpoly.skeleton.xml")
-- character.Scale=Vector3(0.01,0.01,0.01)
-- state:InsertCharacter(character)

-- game:InsertEmitter(game:LoadEmitter("emitter_01.txt"))