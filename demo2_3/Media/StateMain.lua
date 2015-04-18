-- dofile "LoadingUI.lua"
-- dofile "Hud.lua"

-- -- 创建相机
-- local camera=ModelViewerCamera(math.rad(75),4/3.0,0.1,3000)
-- camera.LookAt=Vector3(0,1,0)
-- camera.Eular=Vector3(math.rad(-45),math.rad(45),0)
-- camera.Distance=20
-- camera.EventAlign=function(args)
	-- local desc=game.BackBufferSurfaceDesc
	-- camera.Aspect=desc.Width/desc.Height
-- end
-- game.Camera=camera

local camera=FirstPersonCamera(math.rad(75),4/3.0,0.1,3000)
local k=math.cos(math.rad(45))
local d=20
camera.Eye=Vector3(d*k*k,d*k+1,d*k*k)
camera.Eular=Vector3(math.rad(-45),math.rad(45),0)
camera.EventAlign=function(args)
	local desc=game.BackBufferSurfaceDesc
	camera.Aspect=desc.Width/desc.Height
end
game.Camera=camera

-- local t = Timer(1,0)
-- t.EventTimer=function(args)
	-- LoadingUI.pgs.Progress=LoadingUI.pgs.Progress+0.1
	-- if LoadingUI.pgs.Progress >= 1.09 then
		-- os.exit()
	-- end
-- end
-- game:InsertTimer(t)

local actor = Actor()
game:AddActor(actor)
game:CreateAnimatorFromFile(actor,"mesh/cloth.skeleton.xml")
game:AddMeshComponentFromFile(actor,"mesh/tube.mesh.xml",true)
game:AddEmitterComponentFromFile(actor,"emitter/emitter_01.xml")
game:AddClothComponentFromFile(actor,"mesh/cloth.mesh.xml","mesh/cloth.skeleton.xml","joint5")
game:LoadMeshSetAsync("mesh/scene.mesh.xml", function (res)
	game:AddMeshComponentList(actor, res2mesh_set(res))
end)
