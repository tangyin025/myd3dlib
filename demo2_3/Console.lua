module("Console", package.seeall)

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
local e=ConsoleEditBox()
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

game.EventToggleConsole=function(args)
	console.Visible=not console.Visible
end

game.Panel=panel