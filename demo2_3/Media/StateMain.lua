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
game.SkyLight.Eular=Vector3(math.rad(-30),math.rad(0),0)
game.SkyLight.Width=50
game.SkyLight.Height=50
game.SkyLight.Nz=-50
game.SkyLight.Fz=50
game.SkyLightColor=Vector4(1,1,1,1)

local actor = Actor()
game:AddActor(actor)
-- game:AddMeshComponentFromFile(actor,"mesh/plane.mesh.xml",false)
game:AddMeshComponentFromFile(actor,"mesh/casual19_m_highpoly.mesh.xml",true).World=Matrix4.Scaling(0.05,0.05,0.05)

-- -- local actor = Actor()
-- -- game:AddActor(actor)
-- -- game:AddAnimatorFromFile(actor,"mesh/cloth.skeleton.xml")
-- -- game:AddMeshComponentFromFile(actor,"mesh/tube.mesh.xml",true)
-- -- game:AddEmitterComponentFromFile(actor,"emitter/emitter_01.xml")
-- -- game:AddClothComponentFromFile(actor,"mesh/cloth.mesh.xml","mesh/cloth.skeleton.xml","joint5")
-- -- game:LoadMeshSetAsync("mesh/scene.mesh.xml", function (res)
	-- -- game:AddMeshComponentList(actor, res2mesh_set(res))
-- -- end)

-- -- local actor = Actor()
-- -- game:AddActor(actor)
-- -- game:AddAnimatorFromFile(actor,"mesh/casual19_m_highpoly.skeleton.xml")
-- -- local cmp=game:AddSkeletonMeshComponentFromFile(actor,"mesh/casual19_m_highpoly.mesh.xml",false)
-- -- cmp.World=Matrix4.Scaling(0.05,0.05,0.05)

-- -- 光源
-- local light={}
-- local cmp={}
-- light[1]=SphericalEmitter()
-- light[1].ParticleSizeX:AddNode(0,30,0,0)
-- light[1].ParticleSizeY:AddNode(0,30,0,0)
-- light[1].ParticleColorR:AddNode(0,255,0,0)
-- light[1].ParticleColorG:AddNode(0,0,0,0)
-- light[1].ParticleColorB:AddNode(0,0,0,0)
-- light[1].ParticleColorA:AddNode(0,255,0,0)
-- light[1].MaterialName="light1"
-- light[2]=SphericalEmitter()
-- light[2].ParticleSizeX:AddNode(0,30,0,0)
-- light[2].ParticleSizeY:AddNode(0,30,0,0)
-- light[2].ParticleColorR:AddNode(0,0,0,0)
-- light[2].ParticleColorG:AddNode(0,255,0,0)
-- light[2].ParticleColorB:AddNode(0,0,0,0)
-- light[2].ParticleColorA:AddNode(0,255,0,0)
-- light[2].MaterialName="light1"
-- light[3]=SphericalEmitter()
-- light[3].ParticleSizeX:AddNode(0,30,0,0)
-- light[3].ParticleSizeY:AddNode(0,30,0,0)
-- light[3].ParticleColorR:AddNode(0,0,0,0)
-- light[3].ParticleColorG:AddNode(0,0,0,0)
-- light[3].ParticleColorB:AddNode(0,255,0,0)
-- light[3].ParticleColorA:AddNode(0,255,0,0)
-- light[3].MaterialName="light1"
-- light[4]=SphericalEmitter()
-- light[4].ParticleSizeX:AddNode(0,30,0,0)
-- light[4].ParticleSizeY:AddNode(0,30,0,0)
-- light[4].ParticleColorR:AddNode(0,255,0,0)
-- light[4].ParticleColorG:AddNode(0,255,0,0)
-- light[4].ParticleColorB:AddNode(0,0,0,0)
-- light[4].ParticleColorA:AddNode(0,255,0,0)
-- light[4].MaterialName="light1"
-- light[5]=SphericalEmitter()
-- light[5].ParticleSizeX:AddNode(0,30,0,0)
-- light[5].ParticleSizeY:AddNode(0,30,0,0)
-- light[5].ParticleColorR:AddNode(0,255,0,0)
-- light[5].ParticleColorG:AddNode(0,0,0,0)
-- light[5].ParticleColorB:AddNode(0,255,0,0)
-- light[5].ParticleColorA:AddNode(0,255,0,0)
-- light[5].MaterialName="light1"
-- light[6]=SphericalEmitter()
-- light[6].ParticleSizeX:AddNode(0,30,0,0)
-- light[6].ParticleSizeY:AddNode(0,30,0,0)
-- light[6].ParticleColorR:AddNode(0,0,0,0)
-- light[6].ParticleColorG:AddNode(0,255,0,0)
-- light[6].ParticleColorB:AddNode(0,255,0,0)
-- light[6].ParticleColorA:AddNode(0,255,0,0)
-- light[6].MaterialName="light1"
-- local actor = Actor()
-- game:AddActor(actor)
-- cmp[1]=game:AddEmitterComponent(actor, light[1])
-- -- cmp[2]=game:AddEmitterComponent(actor, light[2])
-- -- cmp[3]=game:AddEmitterComponent(actor, light[3])
-- -- cmp[4]=game:AddEmitterComponent(actor, light[4])
-- -- cmp[5]=game:AddEmitterComponent(actor, light[5])
-- -- cmp[6]=game:AddEmitterComponent(actor, light[6])

-- -- local t = Timer(0.033,0)
-- local angle = 0;
-- -- t.EventTimer=function(args)
	-- -- angle=angle+1
	-- -- game.SkyLight.Eular=Vector3(math.rad(-30),math.rad(-angle),0)
	-- cmp[1].World=Matrix4.Translation(5,2.5,5)*Matrix4.RotationY(math.rad(angle))
	-- -- cmp[2].World=Matrix4.Translation(5,2.5,5)*Matrix4.RotationY(math.rad(angle+60))
	-- -- cmp[3].World=Matrix4.Translation(5,2.5,5)*Matrix4.RotationY(math.rad(angle+120))
	-- -- cmp[4].World=Matrix4.Translation(5,2.5,5)*Matrix4.RotationY(math.rad(angle+180))
	-- -- cmp[5].World=Matrix4.Translation(5,2.5,5)*Matrix4.RotationY(math.rad(angle+240))
	-- -- cmp[6].World=Matrix4.Translation(5,2.5,5)*Matrix4.RotationY(math.rad(angle+300))
-- -- end
-- -- game:InsertTimer(t)

-- e=SphericalEmitter()
-- game:SaveEmitter("ddd.emt.xml", e)

-- function CreateTextureParam(path)
	-- param=ParameterValueTexture()
	-- param.Path=path
	-- return param
-- end

-- m=Material()
-- m.Shader="lambert.fx"
-- m:AddParameter("g_DiffuseTexture", CreateTextureParam("texture/casual19_m_35.jpg"))
-- m:AddParameter("g_NormalTexture", CreateTextureParam("texture/casual19_m_35_normal.dds"))
-- m:AddParameter("g_SpecularTexture", CreateTextureParam("texture/casual19_m_35_spec.dds"))
-- game:SaveMaterial("material/casual19_m_highpolyPhong.xml", m)
