ui_fnt=LoadFont("wqy-microhei-lite.ttc", 13)

console=Dialog()
console.Color=ARGB(197,0,0,0)
console.Location=Vector2(50,95)
console.Size=Vector2(700,410)
console.Skin.Font=ui_fnt
console.Skin.TextColor=ARGB(255,255,255,255)
console.Skin.TextAlign=Font.AlignLeftTop

local e=ImeEditBox()
e.Color=ARGB(15,255,255,255)
e.Location=Vector2(5, 410-5-20)
e.Size=Vector2(700-5-5,20)
e.Border=Vector4(0,0,0,0)
e.Text="在这里输入命令"
e.Skin.Font=ui_fnt
e.Skin.TextColor=ARGB(255,63,188,239)
console:InsertControl(e)

panel=MessagePanel()
panel.Color=ARGB(0,0,0,0)
panel.Location=Vector2(5,5)
panel.Size=Vector2(700-5-5,410-5-5-20)
panel.Skin.Font=ui_fnt
panel.scrollbar.Color=ARGB(15,255,255,255)
panel.scrollbar.Location=Vector2(panel.Size.x - 20, 0)
panel.scrollbar.Size=Vector2(20, panel.Size.y)
panel.scrollbar.nPageSize=3
panel.scrollbar.Skin=ScrollBarSkin()
console:InsertControl(panel)

e.EventEnter=function()
	local t=e.Text
	e.Text=""
	e.nCaret=0
	e.nFirstVisible=0
	panel:AddLine(t, e.Skin.TextColor)
	game:ExecuteCode(t)
end

game:InsertDlg(console)

hud=Dialog()
hud.Color=ARGB(0,0,0,0)
hud.Location=Vector2(800-170,0)
hud.Size=Vector2(170,170)
hud.Skin.Font=ui_fnt
hud.Skin.TextColor=ARGB(255,255,255,255)
hud.Skin.TextAlign=Font.AlignLeftTop

local btn_skin=ButtonSkin()
btn_skin.Image=ControlImage(LoadTexture("button_normal.png"), Vector4(7,7,7,7))
btn_skin.Font=ui_fnt
btn_skin.TextColor=ARGB(255,255,255,255)
btn_skin.TextAlign=Font.AlignCenterMiddle
btn_skin.DisabledImage=ControlImage(LoadTexture("button_disable.png"), Vector4(7,7,7,7))
btn_skin.PressedImage=ControlImage(LoadTexture("button_down.png"), Vector4(7,7,7,7))
btn_skin.MouseOverImage=ControlImage(LoadTexture("button_hover.png"), Vector4(7,7,7,7))
btn_skin.PressedOffset=Vector2(1,2)

local btn=Button()
btn.Text="Toggle full screen"
btn.Location=Vector2(35,10)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.EventClick=function()
	game:ToggleFullScreen()
end
hud:InsertControl(btn)

local btn=Button()
btn.Text="Toggle REF (F3)"
btn:SetHotkey(114) -- VK_F3
btn.Location=Vector2(35,35)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.EventClick=function()
	game:ToggleRef()
end
hud:InsertControl(btn)

local btn=Button()
btn.Text="Change device (F2)"
btn:SetHotkey(113) -- VK_F2
btn.Location=Vector2(35,60)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.EventClick=function()
	game:ChangeDevice()
end
hud:InsertControl(btn)

game:InsertDlg(hud)