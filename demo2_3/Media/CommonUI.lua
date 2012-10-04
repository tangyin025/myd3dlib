require "Loader.lua"

module("CommonUI", package.seeall)

com_lbl_skin=ControlSkin()
com_lbl_skin.Font=_Font.font1
com_lbl_skin.TextColor=ARGB(255,255,255,255)
com_lbl_skin.TextAlign=Font.AlignLeftMiddle

com_btn_skin=ButtonSkin()
com_btn_skin.Image=ControlImage(Loader.LoadTexture("com_btn_normal.png"),Vector4(7,7,7,7))
com_btn_skin.Font=_Font.font1
com_btn_skin.TextColor=ARGB(255,255,255,255)
com_btn_skin.TextAlign=Font.AlignCenterMiddle
com_btn_skin.PressedOffset=Vector2(1,2)
com_btn_skin.DisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"),Vector4(7,7,7,7))
com_btn_skin.PressedImage=ControlImage(Loader.LoadTexture("com_btn_down.png"),Vector4(7,7,7,7))
com_btn_skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"),Vector4(7,7,7,7))

com_chx_skin=ButtonSkin()
com_chx_skin.Image=ControlImage(Loader.LoadTexture("com_chx_normal.png"),Vector4(0,0,0,0))
com_chx_skin.Font=_Font.font1
com_chx_skin.TextColor=ARGB(255,255,255,255)
com_chx_skin.TextAlign=Font.AlignLeftMiddle
com_chx_skin.DisabledImage=ControlImage(Loader.LoadTexture("com_chx_disable.png"),Vector4(0,0,0,0))
com_chx_skin.PressedImage=ControlImage(Loader.LoadTexture("com_chx_down.png"),Vector4(0,0,0,0))
com_chx_skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_chx_hover.png"),Vector4(0,0,0,0))

com_cbx_skin=ComboBoxSkin()
com_cbx_skin.Image=ControlImage(Loader.LoadTexture("com_btn_normal.png"),Vector4(7,7,7,7))
com_cbx_skin.Font=_Font.font1
com_cbx_skin.TextColor=ARGB(255,255,255,255)
com_cbx_skin.TextAlign=Font.AlignCenterMiddle
com_cbx_skin.PressedOffset=Vector2(1,2)
com_cbx_skin.DisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"),Vector4(7,7,7,7))
com_cbx_skin.PressedImage=ControlImage(Loader.LoadTexture("com_btn_down.png"),Vector4(7,7,7,7))
com_cbx_skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"),Vector4(7,7,7,7))
com_cbx_skin.DropdownImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.DropdownItemMouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarUpBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarUpBtnDisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarDownBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarDownBtnDisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarThumbBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))

function UpdateDlgViewProj(dlg, width, height)
	local fovy=1.308996938995747
	local half_width=width*0.5
	local half_height=height*0.5
	local dist=half_height*math.tan(math.pi*0.5-fovy*0.5)
	dlg.View=Matrix4.LookAtLH(Vector3(half_width,half_height,dist),Vector3(half_width,half_height,0),Vector3(0,-1,0))
	dlg.Proj=Matrix4.PerspectiveFovLH(fovy,width/height,0.1,3000)
end