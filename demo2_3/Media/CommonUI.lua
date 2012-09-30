module("CommonUI", package.seeall)

function ComLabel(Location, Size, Text)
	local lbl=Static()
	lbl.Location=Location
	lbl.Size=Size
	lbl.Text=Text
	lbl.Skin.Font=_Font.font1
	lbl.Skin.TextColor=ARGB(255,255,255,255)
	lbl.Skin.TextAlign=Font.AlignCenterMiddle
	return lbl
end

function ComButton(Location, Size, Text)
	local btn=Button()
	btn.Location=Location
	btn.Size=Size
	btn.Text=Text
	btn.PressedOffset=Vector2(1,2)
	btn.Skin.Image=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(5,5,5,5))
	btn.Skin.Font=_Font.font1
	btn.Skin.TextColor=ARGB(255,255,255,255)
	btn.Skin.TextAlign=Font.AlignCenterMiddle
	btn.Skin.DisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(5,5,5,5))
	btn.Skin.PressedImage=ControlImage(Loader.LoadTexture("com_btn_down.png"), Vector4(5,5,5,5))
	btn.Skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"), Vector4(5,5,5,5))
	return btn
end

function ComComboBox(Location, Size, DropdownHeight)
	local cbx=ComboBox()
	cbx.Location=Location
	cbx.Size=Size
	cbx.PressedOffset=Vector2(1,2)
	cbx.DropdownSize=Vector2(Size.x, DropdownHeight)
	cbx.ItemHeight=22
	cbx.Border=Vector4(5,0,0,0)
	cbx.Skin.Image=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
	cbx.Skin.Font=_Font.font1
	cbx.Skin.TextColor=ARGB(255,255,255,255)
	cbx.Skin.TextAlign=Font.AlignLeftMiddle
	cbx.Skin.DisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
	cbx.Skin.PressedImage=ControlImage(Loader.LoadTexture("com_btn_down.png"), Vector4(7,7,7,7))
	cbx.Skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"), Vector4(7,7,7,7))
	cbx.Skin.DropdownImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
	cbx.Skin.DropdownItemMouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"), Vector4(7,7,7,7))
	cbx.Skin.ScrollBarUpBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
	cbx.Skin.ScrollBarUpBtnDisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
	cbx.Skin.ScrollBarDownBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
	cbx.Skin.ScrollBarDownBtnDisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
	cbx.Skin.ScrollBarThumbBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
	cbx.Skin.ScrollBarImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
	return cbx
end