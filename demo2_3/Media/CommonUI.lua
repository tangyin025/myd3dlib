module("CommonUI", package.seeall)

com_lbl_skin=ControlSkin()
com_lbl_skin.Font=_Font.font1
com_lbl_skin.TextColor=ARGB(255,255,255,255)
com_lbl_skin.TextAlign=Font.AlignLeftMiddle

com_btn_skin=ButtonSkin()
com_btn_skin.Image=ControlImage(Loader.LoadTexture("com_btn_normal.png"),Vector4(5,5,5,5))
com_btn_skin.Font=_Font.font1
com_btn_skin.TextColor=ARGB(255,255,255,255)
com_btn_skin.TextAlign=Font.AlignCenterMiddle
com_btn_skin.PressedOffset=Vector2(1,2)
com_btn_skin.DisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"),Vector4(5,5,5,5))
com_btn_skin.PressedImage=ControlImage(Loader.LoadTexture("com_btn_down.png"),Vector4(5,5,5,5))
com_btn_skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"),Vector4(5,5,5,5))

com_cbx_skin=ComboBoxSkin()
com_cbx_skin.Image=ControlImage(Loader.LoadTexture("com_btn_normal.png"),Vector4(5,5,5,5))
com_cbx_skin.Font=_Font.font1
com_cbx_skin.TextColor=ARGB(255,255,255,255)
com_cbx_skin.TextAlign=Font.AlignCenterMiddle
com_cbx_skin.PressedOffset=Vector2(1,2)
com_cbx_skin.DisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"),Vector4(5,5,5,5))
com_cbx_skin.PressedImage=ControlImage(Loader.LoadTexture("com_btn_down.png"),Vector4(5,5,5,5))
com_cbx_skin.MouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"),Vector4(5,5,5,5))
com_cbx_skin.DropdownImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.DropdownItemMouseOverImage=ControlImage(Loader.LoadTexture("com_btn_hover.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarUpBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarUpBtnDisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarDownBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarDownBtnDisabledImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarThumbBtnNormalImage=ControlImage(Loader.LoadTexture("com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarImage=ControlImage(Loader.LoadTexture("com_btn_disable.png"), Vector4(7,7,7,7))