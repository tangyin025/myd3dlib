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

-- -- 利用 EventAlign调整相机的 Aspect
-- local d=Dialog();d.Visible=false;d.EventAlign=function(args) camera.Aspect=args.vp.x/args.vp.y end;game:InsertDlg(d)

local state=game:CurrentState()

local effectMesh = EffectMesh()
effectMesh.Mesh = game:LoadMesh("plane.mesh.xml")
local effect = game:LoadEffect("SimpleSample.fx")
local material = Material()
material.Effect = effect
local texture = game:LoadTexture("Checker.bmp")
effect:SetTechnique("RenderScene")
material:BeginParameterBlock()
effect:SetVector("g_MaterialAmbientColor", Vector4(1,1,1,1))
effect:SetVector("g_MaterialDiffuseColor", Vector4(1,1,1,1))
effect:SetTexture("g_MeshTexture", texture)
material:EndParameterBlock()
effectMesh:InsertMaterial(material)

state:InsertStaticMesh(effectMesh)