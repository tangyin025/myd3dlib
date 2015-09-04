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
game.SkyLightCam.Eular=Vector3(math.rad(-45),math.rad(0),0)

-- 设置环境光
game.SkyLightCam.Eye=Vector3(0,0,0)
game.SkyLightCam.Eular=Vector3(math.rad(-30),math.rad(0),0)
game.SkyLightCam.Width=50
game.SkyLightCam.Height=50
game.SkyLightCam.Nz=-50
game.SkyLightCam.Fz=50
game.SkyLightDiffuse=Vector4(0.5,0.5,0.5,0.5)
game.SkyLightAmbient=Vector4(0.0,0.0,0.0,0.0)

-- 添加模型
local actor=Actor(AABB(-1000,1000),1)
game:AddActor(actor)
local cmp=game:CreateMeshComponentFromFile(actor,"mesh/casual19_m_highpoly.mesh.xml",actor.aabb,Matrix4.Scaling(0.05,0.05,0.05),false)
cmp.Animator=game:CreateSimpleAnimatorFromFile(actor,"mesh/casual19_m_highpoly.skeleton.xml")
local cmp=game:CreateClothComponentFromFile(actor,"mesh/cloth.mesh.xml","mesh/cloth.skeleton.xml","joint5",actor.aabb,Matrix4.Identity())
cmp.Animator=game:CreateSimpleAnimatorFromFile(actor,"mesh/cloth.skeleton.xml")
game:LoadMeshSetAsync("mesh/scene.mesh.xml", function (res)
	game:CreateMeshComponentList(actor, res2mesh_set(res))
end)

-- local actor = Actor()
-- game:AddActor(actor)
-- -- game:AddAnimatorFromFile(actor,"mesh/cloth.skeleton.xml")
-- -- game:AddMeshComponentFromFile(actor,"mesh/tube.mesh.xml",true)
-- -- game:AddEmitterComponentFromFile(actor,"emitter/emitter_01.xml")
-- -- game:AddClothComponentFromFile(actor,"mesh/cloth.mesh.xml","mesh/cloth.skeleton.xml","joint5")
-- game:LoadMeshSetAsync("mesh/scene.mesh.xml", function (res)
	-- game:AddMeshComponentList(actor, res2mesh_set(res))
-- end)

-- -- local actor = Actor()
-- -- game:AddActor(actor)
-- -- game:AddAnimatorFromFile(actor,"mesh/casual19_m_highpoly.skeleton.xml")
-- -- local cmp=game:AddSkeletonMeshComponentFromFile(actor,"mesh/casual19_m_highpoly.mesh.xml",false)
-- -- cmp.World=Matrix4.Scaling(0.05,0.05,0.05)

-- 添加光源
local l=Emitter()
local e=Emitter()
l.MaterialName="light1"
l.PassMask=Material.PassMaskLight
e.MaterialName="particle1"
e.PassMask=Material.PassMaskTransparent
for i=0,359,60 do
	local radius=10
	l:Spawn(Vector3(radius*math.cos(math.rad(i)),0,radius*math.sin(math.rad(i))),Vector3(0,0,0),ARGB(255,255,255,255),Vector2(30,30),0)
	e:Spawn(Vector3(radius*math.cos(math.rad(i)),0,radius*math.sin(math.rad(i))),Vector3(0,0,0),ARGB(255,255,255,255),Vector2(5,5),0)
end
lcmp=game:CreateEmitterComponent(actor,l,actor.aabb,Matrix4.Identity())
ecmp=game:CreateEmitterComponent(actor,e,actor.aabb,Matrix4.Identity())

local t = Timer(0.033,0)
local angle = 0;
t.EventTimer=function(args)
	angle=angle+0.5
	game.SkyLightCam.Eular=Vector3(math.rad(-30),math.rad(-angle),0)
	local trans=Matrix4.Translation(0,2.5,0)*Matrix4.RotationY(math.rad(angle))
	lcmp.World=trans
	ecmp.World=trans
end
game:InsertTimer(t)
