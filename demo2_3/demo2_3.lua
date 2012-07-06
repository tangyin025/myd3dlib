font=LoadFont("wqy-microhei.ttc", 13)
game.font=font
dofile "Console.lua"
dofile "Hud.lua"

camera=ModuleViewCamera(math.rad(75),4/3,0.1,3000)
camera.Rotation=Vector3(math.rad(-45),math.rad(45),0)
camera.Distance=10
scene=Scene()
scene.Camera=camera
game:InsertScene(1, scene)

-- 利用EventAlign调整相机的Aspect
local d=Dialog();d.Visible=false;d.EventAlign=function(args) camera.Aspect=args.vp.x/args.vp.y end;game:InsertDlg(3,d)