module("Hud", package.seeall)

dlg=Dialog("Hud")
dlg.x=UDim(1,-170)
dlg.Width=UDim(0,170)
dlg.Height=UDim(0,170)

local btn_toggle_fs=Button("btn_toggle_fs")
btn_toggle_fs.x=UDim(0,35)
btn_toggle_fs.y=UDim(0,10)
btn_toggle_fs.Width=UDim(0,125)
btn_toggle_fs.Height=UDim(0,22)
btn_toggle_fs.Text="Toggle full screen"
btn_toggle_fs.Skin=ButtonSkin()
btn_toggle_fs.Skin.Image=ControlImage()
btn_toggle_fs.Skin.Image.TexturePath="texture/CommonUI.png"
btn_toggle_fs.Skin.Image.Rect=Rectangle.LeftTop(52,43,16,16)
btn_toggle_fs.Skin.Image.Border=Vector4(7,7,7,7)
btn_toggle_fs.Skin.FontPath="font/wqy-microhei.ttc"
btn_toggle_fs.Skin.FontHeight=13
btn_toggle_fs.Skin.FontFaceIndex=0
btn_toggle_fs.Skin.TextColor=ARGB(255,255,255,255)
btn_toggle_fs.Skin.TextAlign=Font.AlignCenterMiddle
btn_toggle_fs.Skin.PressedOffset=Vector2(1,2)
btn_toggle_fs.Skin.DisabledImage=ControlImage()
btn_toggle_fs.Skin.DisabledImage.TexturePath="texture/CommonUI.png"
btn_toggle_fs.Skin.DisabledImage.Rect=Rectangle.LeftTop(1,43,16,16)
btn_toggle_fs.Skin.DisabledImage.Border=Vector4(7,7,7,7)
btn_toggle_fs.Skin.PressedImage=ControlImage()
btn_toggle_fs.Skin.PressedImage.TexturePath="texture/CommonUI.png"
btn_toggle_fs.Skin.PressedImage.Rect=Rectangle.LeftTop(18,43,16,16)
btn_toggle_fs.Skin.PressedImage.Border=Vector4(7,7,7,7)
btn_toggle_fs.Skin.MouseOverImage=ControlImage()
btn_toggle_fs.Skin.MouseOverImage.TexturePath="texture/CommonUI.png"
btn_toggle_fs.Skin.MouseOverImage.Rect=Rectangle.LeftTop(35,43,16,16)
btn_toggle_fs.Skin.MouseOverImage.Border=Vector4(7,7,7,7)
dlg:InsertControl(btn_toggle_fs)

local btn_toggle_ref=Button("btn_toggle_ref")
btn_toggle_ref.x=UDim(0,35)
btn_toggle_ref.y=UDim(0,35)
btn_toggle_ref.Width=UDim(0,125)
btn_toggle_ref.Height=UDim(0,22)
btn_toggle_ref.Text="Toggle REF (F3)"
btn_toggle_ref.Skin=btn_toggle_fs.Skin:Clone()
btn_toggle_ref:SetHotkey(114) -- VK_F3
dlg:InsertControl(btn_toggle_ref)

local btn_change_device=Button("btn_change_device")
btn_change_device.x=UDim(0,35)
btn_change_device.y=UDim(0,60)
btn_change_device.Width=UDim(0,125)
btn_change_device.Height=UDim(0,22)
btn_change_device.Text="Change device (F2)"
btn_change_device.Skin=btn_toggle_fs.Skin:Clone()
btn_change_device:SetHotkey(113) -- VK_F2
dlg:InsertControl(btn_change_device)

dlg.EventAlign=function(arg)
	-- dlg.Location=Vector2(client.DlgViewport.x-170,0)
end

local btn_toggle_fs=client:GetNamedObject("btn_toggle_fs")
btn_toggle_fs.EventMouseClick=function(arg)
	client:ToggleFullScreen()
end

local btn_toggle_ref=client:GetNamedObject("btn_toggle_ref")
btn_toggle_ref.EventMouseClick=function(arg)
	-- ! do not destroy device within lua context
	client.wnd:PostMessage(40002,0,0)
end

local btn_change_device=client:GetNamedObject("btn_change_device")
btn_change_device.EventMouseClick=function(arg)
	Settings.dlg.Visible=not Settings.dlg.Visible
end

client:InsertDlg(dlg)
