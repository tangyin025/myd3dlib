hud=Dialog()
hud.Color=ARGB(0,0,0,0)
-- hud.Location=Vector2(800-170,0)
hud.Size=Vector2(170,170)
hud.Skin.Font=game.font
hud.Skin.TextColor=ARGB(255,255,255,255)
hud.Skin.TextAlign=Font.AlignLeftTop
hud.EventAlign=function(args)
	hud.Location=Vector2(args.vp.x-170,0)
end

local btn_skin=ButtonSkin()
btn_skin.Image=ControlImage(Game.LoadTexture("Hud_btn_normal.png"), Vector4(7,7,7,7))
btn_skin.Font=game.font
btn_skin.TextColor=ARGB(255,255,255,255)
btn_skin.TextAlign=Font.AlignCenterMiddle
btn_skin.DisabledImage=ControlImage(Game.LoadTexture("Hud_btn_disable.png"), Vector4(7,7,7,7))
btn_skin.PressedImage=ControlImage(Game.LoadTexture("Hud_btn_down.png"), Vector4(7,7,7,7))
btn_skin.MouseOverImage=ControlImage(Game.LoadTexture("Hud_btn_hover.png"), Vector4(7,7,7,7))

local btn=Button()
btn.Text="Toggle full screen"
btn.Location=Vector2(35,10)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.PressedOffset=Vector2(1,2)
btn.EventClick=function(args)
	game:ToggleFullScreen()
end
hud:InsertControl(btn)

local btn=Button()
btn.Text="Toggle REF (F3)"
btn:SetHotkey(114) -- VK_F3
btn.Location=Vector2(35,35)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.PressedOffset=Vector2(1,2)
btn.EventClick=function(args)
	game:ToggleRef()
end
hud:InsertControl(btn)

local btn=Button()
btn.Text="Change device (F2)"
btn:SetHotkey(113) -- VK_F2
btn.Location=Vector2(35,60)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
btn.PressedOffset=Vector2(1,2)
btn.EventClick=function(args)
	game:ChangeDevice()
end
hud:InsertControl(btn)

game:InsertDlg(hud)