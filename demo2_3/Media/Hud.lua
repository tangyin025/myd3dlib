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

local btn_toggle_fs=CommonUI.ComButton(Vector2(35,10),Vector2(125,22),"Toggle full screen")
btn_toggle_fs.EventClick=function(args)
	game:ToggleFullScreen()
end
dlg:InsertControl(btn_toggle_fs)

local btn_toggle_ref=CommonUI.ComButton(Vector2(35,35),Vector2(125,22),"Toggle REF (F3)")
btn_toggle_ref:SetHotkey(114) -- VK_F3
btn_toggle_ref.EventClick=function(args)
	game:ToggleREF()
end
dlg:InsertControl(btn_toggle_ref)

btn_change_device=CommonUI.ComButton(Vector2(35,60),Vector2(125,22),"Change device (F2)")
btn_change_device:SetHotkey(113) -- VK_F2
btn_change_device.EventClick=function(args)
	-- game:ChangeDevice()
end
dlg:InsertControl(btn_change_device)

game:InsertDlg(dlg)