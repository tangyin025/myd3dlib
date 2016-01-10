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
game.SkyLightDiffuse=Vector4(1.0,1.0,1.0,1.0)
game.SkyLightAmbient=Vector4(0.0,0.0,0.0,0.0)

-- -- 添加模型
-- local cmp = MeshComponent(AABB(-10,10),Matrix4.Scaling(0.05,0.05,0.05),false)
-- cmp.MeshRes.ResPath = "mesh/casual19_m_highpoly.mesh.xml"
-- local material=Material()
-- material.Shader="lambert1.fx"
-- material.PassMask=Material.PassMaskOpaque
-- material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/casual19_m_35.jpg"))
-- material:AddParameter("g_NormalTexture", ParameterValueTexture("texture/casual19_m_35_normal.png"))
-- material:AddParameter("g_SpecularTexture", ParameterValueTexture("texture/casual19_m_35_spec.png"))
-- cmp:AddMaterial(material)
-- local animator = SimpleAnimator()
-- animator.SkeletonRes.ResPath = "mesh/casual19_m_highpoly.skeleton.xml"
-- cmp.Animator = animator
-- cmp:RequestResource()
-- game.Root:AddComponent(cmp2raw(cmp),cmp.aabb:transform(cmp.World),0.1)

-- -- 添加特效
-- local cmp2 = EmitterComponent(AABB(-10,10),Matrix4.Identity())
-- local emitter=SphericalEmitter()
-- emitter.SpawnInterval=1/100
-- emitter.ParticleLifeTime=10
-- emitter.SpawnSpeed=5
-- emitter.SpawnInclination:AddNode(0,math.rad(45),0,0)
-- local Azimuth=math.rad(360)*8
-- emitter.SpawnAzimuth:AddNode(0,0,Azimuth/10,Azimuth/10)
-- emitter.SpawnAzimuth:AddNode(10,Azimuth,Azimuth/10,Azimuth/10)
-- emitter.SpawnColorA:AddNode(0,255,0,0)
-- emitter.SpawnColorA:AddNode(10,0,0,0)
-- emitter.SpawnColorR:AddNode(0,255,0,0)
-- emitter.SpawnColorR:AddNode(10,0,0,0)
-- emitter.SpawnColorG:AddNode(0,255,0,0)
-- emitter.SpawnColorG:AddNode(10,0,0,0)
-- emitter.SpawnColorB:AddNode(0,255,0,0)
-- emitter.SpawnColorB:AddNode(10,0,0,0)
-- emitter.SpawnSizeX:AddNode(0,1,0,0)
-- emitter.SpawnSizeX:AddNode(10,10,0,0)
-- emitter.SpawnSizeY:AddNode(0,1,0,0)
-- emitter.SpawnSizeY:AddNode(10,10,0,0)
-- cmp2.Emitter = emitter
-- local material=Material()
-- material.Shader="particle1.fx"
-- material.PassMask=Material.PassMaskTransparent
-- material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/flare.dds"))
-- cmp2.Material = material
-- cmp2:RequestResource()
-- game.Root:AddComponent(cmp2raw(cmp2),cmp2.aabb:transform(cmp2.World),0.1)

-- -- 添加地形
-- local cmp3 = TerrainComponent(Vector3(-10,0,-10),Vector3(10,0,10),Vector2(0,0),Vector2(1,1),5,5,Matrix4.Identity())
-- local material=Material()
-- material.Shader="lambert1.fx"
-- material.PassMask=Material.PassMaskOpaque
-- material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/wall.jpg"))
-- material:AddParameter("g_NormalTexture", ParameterValueTexture("texture/wall_NM_height.DDS"))
-- material:AddParameter("g_SpecularTexture", ParameterValueTexture("texture/White.dds"))
-- cmp3.Material = material
-- cmp3:RequestResource()
-- game.Root:AddComponent(cmp2raw(cmp3),cmp3.aabb:transform(cmp3.World),0.1)

-- -- 添加光源
-- local l=Emitter()
-- local e=Emitter()
-- for i=0,359,60 do
	-- local radius=10
	-- l:Spawn(Vector3(radius*math.cos(math.rad(i)),0,radius*math.sin(math.rad(i))),Vector3(0,0,0),ARGB(255,255,255,255),Vector2(30,30),0)
	-- e:Spawn(Vector3(radius*math.cos(math.rad(i)),0,radius*math.sin(math.rad(i))),Vector3(0,0,0),ARGB(255,255,255,255),Vector2(5,5),0)
-- end
-- local lcmp=EmitterComponent(AABB(-10,10),Matrix4.Identity())
-- lcmp.Emitter=l
-- local material=Material()
-- material.Shader="light1.fx"
-- material.PassMask=Material.PassMaskLight
-- material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/White.dds"))
-- lcmp.Material=material
-- lcmp:RequestResource()
-- game.Root:AddComponent(cmp2raw(lcmp),lcmp.aabb:transform(lcmp.World),0.1)
-- local ecmp=EmitterComponent(AABB(-10,10),Matrix4.Identity())
-- ecmp.Emitter=e
-- local material=Material()
-- material.Shader="particle1.fx"
-- material.PassMask=Material.PassMaskTransparent
-- material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/flare.dds"))
-- ecmp.Material=material
-- ecmp:RequestResource()
-- game.Root:AddComponent(cmp2raw(ecmp),ecmp.aabb:transform(ecmp.World),0.1)

-- local t = Timer(0.033,0)
-- local angle = 0;
-- t.EventTimer=function(args)
	-- angle=angle+0.5
	-- game.SkyLightCam.Eular=Vector3(math.rad(-30),math.rad(-angle),0)
	-- local trans=Matrix4.Translation(0,2.5,0)*Matrix4.RotationY(math.rad(angle))
	-- lcmp.World=trans
	-- ecmp.World=trans
-- end
-- game:InsertTimer(t)
