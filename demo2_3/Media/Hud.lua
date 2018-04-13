require "CommonUI.lua"
module("Hud", package.seeall)

-- dlg=Dialog()
-- dlg.Name="Hud"
-- dlg.Color=ARGB(0,0,0,0)
-- dlg.Size=Vector2(170,170)

-- local btn_toggle_fs=Button()
-- btn_toggle_fs.Name="btn_toggle_fs"
-- btn_toggle_fs.Location=Vector2(35,10)
-- btn_toggle_fs.Size=Vector2(125,22)
-- btn_toggle_fs.Text="Toggle full screen"
-- btn_toggle_fs.Skin=CommonUI.com_btn_skin
-- dlg:InsertControl(btn_toggle_fs)

-- local btn_toggle_ref=Button()
-- btn_toggle_ref.Name="btn_toggle_ref"
-- btn_toggle_ref.Location=Vector2(35,35)
-- btn_toggle_ref.Size=Vector2(125,22)
-- btn_toggle_ref.Text="Toggle REF (F3)"
-- btn_toggle_ref.Skin=CommonUI.com_btn_skin
-- btn_toggle_ref:SetHotkey(114) -- VK_F3
-- dlg:InsertControl(btn_toggle_ref)

-- local btn_change_device=Button()
-- btn_change_device.Name="btn_change_device"
-- btn_change_device.Location=Vector2(35,60)
-- btn_change_device.Size=Vector2(125,22)
-- btn_change_device.Text="Change device (F2)"
-- btn_change_device.Skin=CommonUI.com_btn_skin
-- btn_change_device:SetHotkey(113) -- VK_F2
-- dlg:InsertControl(btn_change_device)
-- game:SaveDialog(dlg, "ui/Hud.ui.xml")

dlg=game:LoadDialog("ui/Hud.ui.xml")
game:InsertDlg(dlg)
dlg.EventAlign=function(args)
	dlg.Location=Vector2(game.DlgViewport.x-170,0)
end

local btn_toggle_fs=dlg:FindControl("btn_toggle_fs")
btn_toggle_fs.EventClick=function(args)
	game:ToggleFullScreen()
end

local btn_toggle_ref=dlg:FindControl("btn_toggle_ref")
btn_toggle_ref.EventClick=function(args)
	-- ! do not destroy device within lua context
	-- game:ToggleREF()
end

local btn_change_device=dlg:FindControl("btn_change_device")
btn_change_device.EventClick=function(args)
	Settings.dlg.Visible=not Settings.dlg.Visible
end
