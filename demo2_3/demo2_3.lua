font=LoadFont("wqy-microhei-lite.ttc", 13)
game.font=font
dofile "Console.lua"
dofile "Hud.lua"

camera=ModuleViewCamera(math.rad(75),4/3,0.1,3000)
camera.Rotation=Vector3(math.rad(-45),math.rad(45),0)
camera.Distance=10
scene=Scene()
scene.Camera=camera
game:InsertScene(1, scene)

-- 补丁，利用EventAlign调整相机的Aspect
local d=Dialog()
d.Visible=false
d.EventAlign=function(args)
	camera.Aspect=args.vp.x/args.vp.y
end
game:InsertDlg(3,d)

for x = -5,5,2 do
	for z = -5,5,2 do
		local box = CollisionBox(Vector3(0.5,0.5,0.5),Matrix4.Identity(),0.9,0.6)
		local body = RigidBody()
		body.Mass=4
		body.Acceleration=Vector3(0,-9.8*2,0)
		body.Position=Vector3(x,5,z)
		body.Damping=0.95
		body.AngularDamping=0.8
		body.InertialTensor=RigidBody.CalculateBlockInertiaTensor(box.HalfSize, body.Mass)
		body:InsertShape(box)
		scene:InsertBody(body)
	end
end