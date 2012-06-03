print("Hello, world")

d=Dialog()
d.Location=Vector2(100,100)
d.Size=Vector2(300,300)
d.Color=ARGB(255,255,255,255)
d.Skin.Font=game.uiFont

s=Static()
s.Location=Vector2(10,10)
s.Size=Vector2(100,20)
s.Skin.Font=game.uiFont
s.Skin.TextColor=ARGB(255,0,0,255)
s.Text="唐寅是好人"

local count=1

b=Button()
b.Location=Vector2(10, 30)
b.Size=Vector2(100,20)
b.Skin.Font=game.uiFont
b.Skin.TextColor=ARGB(255,255,0,0)
b.Text="我是按钮"
b.EventClick=function()
	count=count+1
	s.Text="不要点我嘛！" .. count
end

d:InsertControl(s)
d:InsertControl(b)
game:InsertDlg(d)