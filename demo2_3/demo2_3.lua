local font=LoadFont("wqy-microhei-lite.ttc", 13)

local console=Dialog()
console.Color=ARGB(197,0,0,0)
console.Location=Vector2(50,95)
console.Size=Vector2(700,410)
console.Skin.Font=font
console.Skin.TextColor=ARGB(255,255,255,255)
console.Skin.TextAlign=Font.AlignLeftTop

local panel=MessagePanel()
panel.Color=ARGB(0,0,0,0)
panel.Location=Vector2(5,5)
panel.Size=Vector2(700-5-5,410-5-5-20)
panel.Skin.Font=font
panel.scrollbar.Color=ARGB(15,255,255,255)
panel.scrollbar.Location=Vector2(panel.Size.x - 20, 0)
panel.scrollbar.Size=Vector2(20, panel.Size.y)
panel.scrollbar.nPageSize=3
panel.scrollbar.Skin=ScrollBarSkin()
console:InsertControl(panel)

local e_texts={}
local e_texts_idx=0
local e=ConsoleImeEditBox()
e.Color=ARGB(15,255,255,255)
e.Location=Vector2(5, 410-5-20)
e.Size=Vector2(700-5-5,20)
e.Border=Vector4(0,0,0,0)
e.Text="在这里输入命令"
e.Skin.Font=font
e.Skin.TextColor=ARGB(255,63,188,239)
e.EventEnter=function()
	panel:AddLine(e.Text, e.Skin.TextColor)
	table.insert(e_texts, e.Text)
	if #e_texts > 16 then
		table.remove(e_texts,1)
	end
	e_texts_idx=#e_texts+1
	game:ExecuteCode(e.Text)
	e.Text=""
end
e.EventPrevLine=function()
	e_texts_idx=math.max(1,e_texts_idx-1)
	e.Text=e_texts[e_texts_idx]
end
e.EventNextLine=function()
	e_texts_idx=math.min(#e_texts,e_texts_idx+1)
	e.Text=e_texts[e_texts_idx]
end
console:InsertControl(e)

game:InsertDlg(1, console)

local hud=Dialog()
hud.Color=ARGB(0,0,0,0)
hud.Location=Vector2(800-170,0)
hud.Size=Vector2(170,170)
hud.Skin.Font=font
hud.Skin.TextColor=ARGB(255,255,255,255)
hud.Skin.TextAlign=Font.AlignLeftTop

local btn_skin=ButtonSkin()
btn_skin.Image=ControlImage(LoadTexture("button_normal.png"), Vector4(7,7,7,7))
btn_skin.Font=font
btn_skin.TextColor=ARGB(255,255,255,255)
btn_skin.TextAlign=Font.AlignCenterMiddle
btn_skin.DisabledImage=ControlImage(LoadTexture("button_disable.png"), Vector4(7,7,7,7))
btn_skin.PressedImage=ControlImage(LoadTexture("button_down.png"), Vector4(7,7,7,7))
btn_skin.MouseOverImage=ControlImage(LoadTexture("button_hover.png"), Vector4(7,7,7,7))
btn_skin.PressedOffset=Vector2(1,2)

local btn=Button()
btn.Text="Toggle full screen"
btn.Location=Vector2(35,10)
btn.Size=Vector2(125,22)
btn.Skin=btn_skin
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
btn.EventClick=function(args)
	game:ChangeDevice()
end
hud:InsertControl(btn)

game:InsertDlg(2, hud)

game.EventConsole=function(args)
	console.Visible=not console.Visible
end

game.EventAlign=function(args)
	local vp=args.vp
	UIRender.BuildPerspectiveMatrices(math.rad(75), vp.x, vp.y, console.ViewMatrix, console.ProjMatrix)
	UIRender.BuildPerspectiveMatrices(math.rad(75), vp.x, vp.y, hud.ViewMatrix, hud.ProjMatrix)
	hud.Location=Vector2(args.vp.x-170,0)
end

game.font=font
game.Panel=panel