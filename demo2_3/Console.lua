console=Dialog()
console.Color=ARGB(197,0,0,0)
-- console.Location=Vector2(50,95)
console.Size=Vector2(700,410)
console.Skin.Font=font
console.Skin.TextColor=ARGB(255,255,255,255)
console.Skin.TextAlign=Font.AlignLeftTop
console.EventAlign=function(args)
	console.Location=Vector2(50,95)
end

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
local edit=ConsoleEditBox()
edit.Color=ARGB(15,255,255,255)
edit.Location=Vector2(5, 410-5-20)
edit.Size=Vector2(700-5-5,20)
edit.Border=Vector4(0,0,0,0)
edit.Text="在这里输入命令"
edit.Skin.Font=font
edit.Skin.TextColor=ARGB(255,63,188,239)
edit.EventEnter=function()
	panel:AddLine(edit.Text, edit.Skin.TextColor)
	table.insert(e_texts, edit.Text)
	if #e_texts > 16 then
		table.remove(e_texts,1)
	end
	e_texts_idx=#e_texts+1
	game:ExecuteCode(edit.Text)
	edit.Text=""
end
edit.EventPrevLine=function()
	e_texts_idx=math.max(1,e_texts_idx-1)
	edit.Text=e_texts[e_texts_idx]
end
edit.EventNextLine=function()
	e_texts_idx=math.min(#e_texts,e_texts_idx+1)
	edit.Text=e_texts[e_texts_idx]
end
console:InsertControl(edit)

game:InsertDlg(1, console)

game.EventToggleConsole=function(args)
	console.Visible=not console.Visible
end

game.Panel=panel