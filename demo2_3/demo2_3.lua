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

ui_tex=LoadTexture("Untitled-1.png")

hud=Dialog()
hud.Color=ARGB(0,0,0,0)
hud.Location=Vector2(800-170,0)
hud.Size=Vector2(170,170)
hud.Skin.Font=ui_fnt
hud.Skin.TextColor=ARGB(255,255,255,255)
hud.Skin.TextAlign=Font.AlignLeftTop

btn_skin=ButtonSkin()
btn_skin.Texture=ui_tex
btn_skin.TextureRect=CRect(CPoint(10,10),CSize(125,22))
btn_skin.Font=ui_fnt
btn_skin.TextColor=ARGB(255,255,255,255)
btn_skin.TextAlign=Font.AlignCenterMiddle
btn_skin.DisabledTexRect=CRect(CPoint(10,100),CSize(125,22))
btn_skin.PressedTexRect=CRect(CPoint(10,70),CSize(125,22))
btn_skin.MouseOverTexRect=CRect(CPoint(10,40),CSize(125,22))

local btn=Button()
btn.Text="Toggle full screen"
btn.Location=Vector2(35,10)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.EventClick=function()
	game:ToggleFullScreen()
end
hud:InsertControl(btn)

toggle_ref_btn=Button()
toggle_ref_btn.Text="Toggle REF (F3)"
toggle_ref_btn:SetHotkey(114) -- VK_F3
toggle_ref_btn.Location=Vector2(35,35)
toggle_ref_btn.Size=Vector2(125,22)
toggle_ref_btn.Skin=btn_skin
-- toggle_ref_btn.EventClick=function()
	-- 不能在这里调用ToggleRef，会导致lua被中途销毁，
	-- 参见Game::OnD3D9DestroyDevice
	-- game:ToggleRef()
-- end
hud:InsertControl(toggle_ref_btn)

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