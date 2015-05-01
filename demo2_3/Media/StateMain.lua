-- dofile "LoadingUI.lua"
-- dofile "Hud.lua"

-- 设置相机
local k=math.cos(math.rad(45))
local d=20
game.Camera.Eye=Vector3(d*k*k,d*k+1,d*k*k)
game.Camera.Eular=Vector3(math.rad(-45),math.rad(45),0)
game.Camera.EventAlign=function(args)
	local desc=game.BackBufferSurfaceDesc
	game.Camera.Aspect=desc.Width/desc.Height
end

-- 设置环境光
game.SkyLight.Eye=Vector3(0,0,0)
game.SkyLight.Eular=Vector3(math.rad(-45),math.rad(-45),0)
game.SkyLight.Width=50
game.SkyLight.Height=50
game.SkyLight.Nz=-50
game.SkyLight.Fz=50

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
game:AddAnimatorFromFile(actor,"mesh/cloth.skeleton.xml")
game:AddMeshComponentFromFile(actor,"mesh/tube.mesh.xml",true)
game:AddEmitterComponentFromFile(actor,"emitter/emitter_01.xml")
game:AddClothComponentFromFile(actor,"mesh/cloth.mesh.xml","mesh/cloth.skeleton.xml","joint5")
game:LoadMeshSetAsync("mesh/scene.mesh.xml", function (res)
	game:AddMeshComponentList(actor, res2mesh_set(res))
end)

-- local actor = Actor()
-- game:AddActor(actor)
-- game:AddAnimatorFromFile(actor,"mesh/casual19_m_highpoly.skeleton.xml")
-- game:AddSkeletonMeshComponentFromFile(actor,"mesh/casual19_m_highpoly.mesh.xml",false)
