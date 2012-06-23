font=LoadFont("wqy-microhei-lite.ttc", 13)
game.font=font
dofile "Console.lua"
dofile "Hud.lua"

camera=Camera(math.rad(75),4/3,0.1,3000)
scene=Scene()
scene.Camera=camera
game:InsertScene(1, scene)