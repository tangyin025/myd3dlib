require "CommonUI.lua"
module("Hud", package.seeall)

dlg=Dialog("Hud")
dlg.x=UDim(1,-170)
dlg.Width=UDim(0,170)
dlg.Height=UDim(0,170)
-- dlg.Skin=CommonUI.com_dlg_skin

local btn_toggle_fs=Button("btn_toggle_fs")
btn_toggle_fs.x=UDim(0,35)
btn_toggle_fs.y=UDim(0,10)
btn_toggle_fs.Width=UDim(0,125)
btn_toggle_fs.Height=UDim(0,22)
btn_toggle_fs.Text="Toggle full screen"
btn_toggle_fs.Skin=CommonUI.com_btn_skin
dlg:InsertControl(btn_toggle_fs)

local btn_toggle_ref=Button("btn_toggle_ref")
btn_toggle_ref.x=UDim(0,35)
btn_toggle_ref.y=UDim(0,35)
btn_toggle_ref.Width=UDim(0,125)
btn_toggle_ref.Height=UDim(0,22)
btn_toggle_ref.Text="Toggle REF (F3)"
btn_toggle_ref.Skin=CommonUI.com_btn_skin
btn_toggle_ref:SetHotkey(114) -- VK_F3
dlg:InsertControl(btn_toggle_ref)

local btn_change_device=Button("btn_change_device")
btn_change_device.x=UDim(0,35)
btn_change_device.y=UDim(0,60)
btn_change_device.Width=UDim(0,125)
btn_change_device.Height=UDim(0,22)
btn_change_device.Text="Change device (F2)"
btn_change_device.Skin=CommonUI.com_btn_skin
btn_change_device:SetHotkey(113) -- VK_F2
dlg:InsertControl(btn_change_device)
-- game:SaveDialog(dlg, "ui/Hud.ui.xml")
-- dlg=game:LoadDialog("ui/Hud.ui.xml")

game:InsertDlg(dlg)
dlg.EventAlign=function(arg)
	-- dlg.Location=Vector2(game.DlgViewport.x-170,0)
end

local btn_toggle_fs=game:GetNamedObject("btn_toggle_fs")
btn_toggle_fs.EventMouseClick=function(arg)
	game:ToggleFullScreen()
end

local btn_toggle_ref=game:GetNamedObject("btn_toggle_ref")
btn_toggle_ref.EventMouseClick=function(arg)
	-- ! do not destroy device within lua context
	game.wnd:PostMessage(40002,0,0)
end

local btn_change_device=game:GetNamedObject("btn_change_device")
btn_change_device.EventMouseClick=function(arg)
	Settings.dlg.Visible=not Settings.dlg.Visible
end
