dofile "CommonUI.lua"
dofile "Hud.lua"

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
