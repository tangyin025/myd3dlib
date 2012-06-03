
console=Dialog()
console.Color=ARGB(197,0,0,0)
console.Location=Vector2(50,95)
console.Size=Vector2(700,410)
console.Skin.Font=game.uiFnt
console.Skin.TextColor=ARGB(255,255,255,255)
console.Skin.TextAlign=Font.AlignLeftTop

local e=ImeEditBox()
e.Color=ARGB(15,255,255,255)
e.Location=Vector2(5, 410-5-20)
e.Size=Vector2(700-5-5,20)
e.Border=Vector4(0,0,0,0)
e.Text="在这里输入命令"
e.Skin.Font=game.uiFnt
e.Skin.TextColor=ARGB(255,63,188,239)
console:InsertControl(e)

panel=MessagePanel()
panel.Color=ARGB(0,0,0,0)
panel.Location=Vector2(5,5)
panel.Size=Vector2(700-5-5,410-5-5-20)
panel.Skin.Font=game.uiFnt
panel.scrollbar.Color=ARGB(15,255,255,255)
panel.scrollbar.Location=Vector2(panel.Size.x - 20, 0)
panel.scrollbar.Size=Vector2(20, panel.Size.y)
panel.scrollbar.nPageSize=3
panel.scrollbar.Skin=ScrollBarSkin()
console:InsertControl(panel)

e.EventEnter=function()
	game:ExecuteCode(e.Text)
	e.Text=""
	e.nCaret=0
	e.nFirstVisible=0
end

game:InsertDlg(console)