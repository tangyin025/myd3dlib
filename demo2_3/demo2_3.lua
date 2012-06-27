font=LoadFont("wqy-microhei-lite.ttc", 13)
game.font=font
dofile "Console.lua"
dofile "Hud.lua"

camera=Camera(math.rad(75),4/3,0.1,3000)
camera.pos=Vector3(5,5,-5)
camera.ori=Quaternion.RotationAxis(Vector3(1,0,0),math.atan(5/math.sqrt(50)))*Quaternion.RotationAxis(Vector3(0,1,0),math.rad(-45))
scene=Scene()
scene.Camera=camera
game:InsertScene(1, scene)