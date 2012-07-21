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
	-- 使用临时变量可以减少 wstou8的转换次数
	local code=tostring(edit.Text)
	if string.len(code) > 0 then
		edit.Text=""
		table.insert(e_texts, code)
		if #e_texts > 16 then
			table.remove(e_texts,1)
		end
		e_texts_idx=#e_texts+1
		panel:AddLine(code, edit.Skin.TextColor)
		game:ExecuteCode(code)
	end
end
edit.EventPrevLine=function()
	if e_texts_idx > 1 then
		e_texts_idx=e_texts_idx-1
		edit.Text=e_texts[e_texts_idx]
	end
end
edit.EventNextLine=function()
	if e_texts_idx < #e_texts then
		e_texts_idx=e_texts_idx+1
		edit.Text=e_texts[e_texts_idx]
	end
end
console:InsertControl(edit)

game:InsertDlg(console)

game.EventToggleConsole=function(args)
	console.Visible=not console.Visible
end

game.Panel=panel