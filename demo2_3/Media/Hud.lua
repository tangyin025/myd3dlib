module("Hud", package.seeall)

dlg=Dialog()
dlg.Color=ARGB(0,0,0,0)
-- dlg.Location=Vector2(800-170,0)
dlg.Size=Vector2(170,170)
dlg.Skin.Font=_Font.font1
dlg.Skin.TextColor=ARGB(255,255,255,255)
dlg.Skin.TextAlign=Font.AlignLeftTop
dlg.EventAlign=function(args)
	dlg.Location=Vector2(args.vp.x-170,0)
end

local btn=CommonUI.ComButton(Vector2(35,10),Vector2(125,22),"Toggle full screen")
btn.EventClick=function(args)
	game:ToggleFullScreen()
end
dlg:InsertControl(btn)

local btn=CommonUI.ComButton(Vector2(35,35),Vector2(125,22),"Toggle REF (F3)")
btn:SetHotkey(114) -- VK_F3
btn.EventClick=function(args)
	game:ToggleRef()
end
dlg:InsertControl(btn)

local btn=CommonUI.ComButton(Vector2(35,60),Vector2(125,22),"Change device (F2)")
btn:SetHotkey(113) -- VK_F2
btn.EventClick=function(args)
	game:ChangeDevice()
end
dlg:InsertControl(btn)

game:InsertDlg(dlg)