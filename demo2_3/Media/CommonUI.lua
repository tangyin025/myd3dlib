module("CommonUI", package.seeall)

com_lbl_skin=ControlSkin()
com_lbl_skin.Font=game:LoadFont("font/wqy-microhei.ttc", 13)
com_lbl_skin.TextColor=ARGB(255,255,255,255)
com_lbl_skin.TextAlign=Font.AlignLeftMiddle

com_pgs_skin=ProgressBarSkin()
com_pgs_skin.Image=ControlImage(game:LoadTexture("texture/com_btn_disable.png"),Vector4(7,7,7,7))
com_pgs_skin.Font=game:LoadFont("font/wqy-microhei.ttc", 13)
com_pgs_skin.TextColor=ARGB(255,255,255,255)
com_pgs_skin.TextAlign=Font.AlignLeftMiddle
com_pgs_skin.ForegroundImage=ControlImage(game:LoadTexture("texture/com_btn_hover.png"),Vector4(7,7,7,7))

com_btn_skin=ButtonSkin()
com_btn_skin.Image=ControlImage(game:LoadTexture("texture/com_btn_normal.png"),Vector4(7,7,7,7))
com_btn_skin.Font=game:LoadFont("font/wqy-microhei.ttc", 13)
com_btn_skin.TextColor=ARGB(255,255,255,255)
com_btn_skin.TextAlign=Font.AlignCenterMiddle
com_btn_skin.PressedOffset=Vector2(1,2)
com_btn_skin.DisabledImage=ControlImage(game:LoadTexture("texture/com_btn_disable.png"),Vector4(7,7,7,7))
com_btn_skin.PressedImage=ControlImage(game:LoadTexture("texture/com_btn_down.png"),Vector4(7,7,7,7))
com_btn_skin.MouseOverImage=ControlImage(game:LoadTexture("texture/com_btn_hover.png"),Vector4(7,7,7,7))

com_chx_skin=ButtonSkin()
com_chx_skin.Image=ControlImage(game:LoadTexture("texture/com_chx_normal.png"),Vector4(0,0,0,0))
com_chx_skin.Font=game:LoadFont("font/wqy-microhei.ttc", 13)
com_chx_skin.TextColor=ARGB(255,255,255,255)
com_chx_skin.TextAlign=Font.AlignLeftMiddle
com_chx_skin.DisabledImage=ControlImage(game:LoadTexture("texture/com_chx_disable.png"),Vector4(0,0,0,0))
com_chx_skin.PressedImage=ControlImage(game:LoadTexture("texture/com_chx_down.png"),Vector4(0,0,0,0))
com_chx_skin.MouseOverImage=ControlImage(game:LoadTexture("texture/com_chx_hover.png"),Vector4(0,0,0,0))

com_cbx_skin=ComboBoxSkin()
com_cbx_skin.Image=ControlImage(game:LoadTexture("texture/com_btn_normal.png"),Vector4(7,7,7,7))
com_cbx_skin.Font=game:LoadFont("font/wqy-microhei.ttc", 13)
com_cbx_skin.TextColor=ARGB(255,255,255,255)
com_cbx_skin.TextAlign=Font.AlignCenterMiddle
com_cbx_skin.PressedOffset=Vector2(1,2)
com_cbx_skin.DisabledImage=ControlImage(game:LoadTexture("texture/com_btn_disable.png"),Vector4(7,7,7,7))
com_cbx_skin.PressedImage=ControlImage(game:LoadTexture("texture/com_btn_down.png"),Vector4(7,7,7,7))
com_cbx_skin.MouseOverImage=ControlImage(game:LoadTexture("texture/com_btn_hover.png"),Vector4(7,7,7,7))
com_cbx_skin.DropdownImage=ControlImage(game:LoadTexture("texture/com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.DropdownItemMouseOverImage=ControlImage(game:LoadTexture("texture/com_btn_hover.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarUpBtnNormalImage=ControlImage(game:LoadTexture("texture/com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarUpBtnDisabledImage=ControlImage(game:LoadTexture("texture/com_btn_disable.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarDownBtnNormalImage=ControlImage(game:LoadTexture("texture/com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarDownBtnDisabledImage=ControlImage(game:LoadTexture("texture/com_btn_disable.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarThumbBtnNormalImage=ControlImage(game:LoadTexture("texture/com_btn_normal.png"), Vector4(7,7,7,7))
com_cbx_skin.ScrollBarImage=ControlImage(game:LoadTexture("texture/com_btn_disable.png"), Vector4(7,7,7,7))
