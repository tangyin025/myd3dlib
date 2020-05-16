require "CommonUI.lua"
module("Settings",package.seeall)

dlg=Dialog()
dlg.Name="Settings"
dlg.Color=ARGB(150,0,0,0)
dlg.Size=Vector2(640,480)
dlg.Skin=CommonUI.com_dlg_skin

local lbl_title=Control()
lbl_title.Location=Vector2(17,13)
lbl_title.Size=Vector2(256,42)
lbl_title.Color=ARGB(255,255,255,255)
lbl_title.Skin=ControlSkin()
lbl_title.Skin.Image=ControlImage()
lbl_title.Skin.Image.Texture=game:LoadTexture("texture/CommonUI.png")
lbl_title.Skin.Image.Rect=Rectangle(0,0,256,42)
lbl_title.Skin.Image.Border=Vector4(0,0,0,0)
dlg:InsertControl(lbl_title)

local btn_ok=Button()
btn_ok.Name="btn_ok"
btn_ok.Location=Vector2(230,439)
btn_ok.Size=Vector2(80,32)
btn_ok.Text="OK"
btn_ok.Skin=CommonUI.com_btn_skin
dlg:InsertControl(btn_ok)

local btn_cancel=Button()
btn_cancel.Name="btn_cancel"
btn_cancel.Location=Vector2(315,439)
btn_cancel.Size=Vector2(80,32)
btn_cancel.Text="Cancel"
btn_cancel.Skin=CommonUI.com_btn_skin
dlg:InsertControl(btn_cancel)

local item_y=390
local item_height=30
local lbl_vertical_sync=Static()
lbl_vertical_sync.Location=Vector2(0,item_y)
lbl_vertical_sync.Size=Vector2(190,22)
lbl_vertical_sync.Text="Vertical Sync"
lbl_vertical_sync.Skin=CommonUI.com_lbl_skin
lbl_vertical_sync.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_vertical_sync)

local cbx_vertical_sync=ComboBox()
cbx_vertical_sync.Name="cbx_vertical_sync"
cbx_vertical_sync.Location=Vector2(201,item_y)
cbx_vertical_sync.Size=Vector2(304,22)
cbx_vertical_sync.ScrollbarWidth=20
cbx_vertical_sync.DropdownSize=Vector2(304-20,130)
cbx_vertical_sync.Border=Vector4(20,0,20,0)
cbx_vertical_sync.ItemHeight=22
cbx_vertical_sync.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_vertical_sync)

item_y=item_y-item_height
local lbl_vertex_processing=Static()
lbl_vertex_processing.Location=Vector2(0,item_y)
lbl_vertex_processing.Size=Vector2(190,22)
lbl_vertex_processing.Text="Vertex Processing"
lbl_vertex_processing.Skin=CommonUI.com_lbl_skin
lbl_vertex_processing.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_vertex_processing)

local cbx_vertex_processing=ComboBox()
cbx_vertex_processing.Name="cbx_vertex_processing"
cbx_vertex_processing.Location=Vector2(201,item_y)
cbx_vertex_processing.Size=Vector2(304,22)
cbx_vertex_processing.ScrollbarWidth=20
cbx_vertex_processing.DropdownSize=Vector2(304-20,130)
cbx_vertex_processing.Border=Vector4(20,0,20,0)
cbx_vertex_processing.ItemHeight=22
cbx_vertex_processing.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_vertex_processing)

item_y=item_y-item_height
local lbl_multisample_quality=Static()
lbl_multisample_quality.Location=Vector2(0,item_y)
lbl_multisample_quality.Size=Vector2(190,22)
lbl_multisample_quality.Text="Multisample Quality"
lbl_multisample_quality.Skin=CommonUI.com_lbl_skin
lbl_multisample_quality.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_multisample_quality)

local cbx_multisample_quality=ComboBox()
cbx_multisample_quality.Name="cbx_multisample_quality"
cbx_multisample_quality.Location=Vector2(201,item_y)
cbx_multisample_quality.Size=Vector2(304,22)
cbx_multisample_quality.ScrollbarWidth=20
cbx_multisample_quality.DropdownSize=Vector2(304-20,130)
cbx_multisample_quality.Border=Vector4(20,0,20,0)
cbx_multisample_quality.ItemHeight=22
cbx_multisample_quality.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_multisample_quality)

item_y=item_y-item_height
local lbl_multisample_type=Static()
lbl_multisample_type.Location=Vector2(0,item_y)
lbl_multisample_type.Size=Vector2(190,22)
lbl_multisample_type.Text="Multisample Type"
lbl_multisample_type.Skin=CommonUI.com_lbl_skin
lbl_multisample_type.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_multisample_type)

local cbx_multisample_type=ComboBox()
cbx_multisample_type.Name="cbx_multisample_type"
cbx_multisample_type.Location=Vector2(201,item_y)
cbx_multisample_type.Size=Vector2(304,22)
cbx_multisample_type.ScrollbarWidth=20
cbx_multisample_type.DropdownSize=Vector2(304-20,130)
cbx_multisample_type.Border=Vector4(20,0,20,0)
cbx_multisample_type.ItemHeight=22
cbx_multisample_type.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_multisample_type)

item_y=item_y-item_height
local lbl_depth_stencil_format=Static()
lbl_depth_stencil_format.Location=Vector2(0,item_y)
lbl_depth_stencil_format.Size=Vector2(190,22)
lbl_depth_stencil_format.Text="Depth/Stencil Format"
lbl_depth_stencil_format.Skin=CommonUI.com_lbl_skin
lbl_depth_stencil_format.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_depth_stencil_format)

local cbx_depth_stencil_format=ComboBox()
cbx_depth_stencil_format.Name="cbx_depth_stencil_format"
cbx_depth_stencil_format.Location=Vector2(201,item_y)
cbx_depth_stencil_format.Size=Vector2(304,22)
cbx_depth_stencil_format.ScrollbarWidth=20
cbx_depth_stencil_format.DropdownSize=Vector2(304-20,130)
cbx_depth_stencil_format.Border=Vector4(20,0,20,0)
cbx_depth_stencil_format.ItemHeight=22
cbx_depth_stencil_format.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_depth_stencil_format)

item_y=item_y-item_height
local lbl_back_buffer_format=Static()
lbl_back_buffer_format.Location=Vector2(0,item_y)
lbl_back_buffer_format.Size=Vector2(190,22)
lbl_back_buffer_format.Text="Back Buffer Format"
lbl_back_buffer_format.Skin=CommonUI.com_lbl_skin
lbl_back_buffer_format.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_back_buffer_format)

local cbx_back_buffer_format=ComboBox()
cbx_back_buffer_format.Name="cbx_back_buffer_format"
cbx_back_buffer_format.Location=Vector2(201,item_y)
cbx_back_buffer_format.Size=Vector2(304,22)
cbx_back_buffer_format.ScrollbarWidth=20
cbx_back_buffer_format.DropdownSize=Vector2(304-20,130)
cbx_back_buffer_format.Border=Vector4(20,0,20,0)
cbx_back_buffer_format.ItemHeight=22
cbx_back_buffer_format.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_back_buffer_format)

item_y=item_y-item_height
local lbl_refresh_rate=Static()
lbl_refresh_rate.Location=Vector2(0,item_y)
lbl_refresh_rate.Size=Vector2(190,22)
lbl_refresh_rate.Text="Refresh Rate"
lbl_refresh_rate.Skin=CommonUI.com_lbl_skin
lbl_refresh_rate.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_refresh_rate)

local cbx_refresh_rate=ComboBox()
cbx_refresh_rate.Name="cbx_refresh_rate"
cbx_refresh_rate.Location=Vector2(201,item_y)
cbx_refresh_rate.Size=Vector2(304,22)
cbx_refresh_rate.ScrollbarWidth=20
cbx_refresh_rate.DropdownSize=Vector2(304-20,130)
cbx_refresh_rate.Border=Vector4(20,0,20,0)
cbx_refresh_rate.ItemHeight=22
cbx_refresh_rate.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_refresh_rate)

item_y=item_y-item_height
local lbl_resolution=Static()
lbl_resolution.Location=Vector2(0,item_y)
lbl_resolution.Size=Vector2(190,22)
lbl_resolution.Text="Resolution"
lbl_resolution.Skin=CommonUI.com_lbl_skin
lbl_resolution.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_resolution)

local cbx_resolution=ComboBox()
cbx_resolution.Name="cbx_resolution"
cbx_resolution.Location=Vector2(201,item_y)
cbx_resolution.Size=Vector2(304,22)
cbx_resolution.ScrollbarWidth=20
cbx_resolution.DropdownSize=Vector2(304-20,130)
cbx_resolution.Border=Vector4(20,0,20,0)
cbx_resolution.ItemHeight=22
cbx_resolution.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_resolution)

item_y=item_y-item_height
local lbl_adapter_format=Static()
lbl_adapter_format.Location=Vector2(0,item_y)
lbl_adapter_format.Size=Vector2(190,22)
lbl_adapter_format.Text="Adapter Format"
lbl_adapter_format.Skin=CommonUI.com_lbl_skin
lbl_adapter_format.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_adapter_format)

local cbx_adapter_format=ComboBox()
cbx_adapter_format.Name="cbx_adapter_format"
cbx_adapter_format.Location=Vector2(201,item_y)
cbx_adapter_format.Size=Vector2(304,22)
cbx_adapter_format.ScrollbarWidth=20
cbx_adapter_format.DropdownSize=Vector2(304-20,130)
cbx_adapter_format.Border=Vector4(20,0,20,0)
cbx_adapter_format.ItemHeight=22
cbx_adapter_format.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_adapter_format)

item_y=item_y-item_height
local chx_windowed=CheckBox()
chx_windowed.Name="chx_windowed"
chx_windowed.Location=Vector2(239,item_y)
chx_windowed.Size=Vector2(120,22)
chx_windowed.Text="Windowed"
chx_windowed.Skin=CommonUI.com_chx_skin
dlg:InsertControl(chx_windowed)

local chx_full_screen=CheckBox()
chx_full_screen.Name="chx_full_screen"
chx_full_screen.Location=Vector2(357,item_y)
chx_full_screen.Size=Vector2(120,22)
chx_full_screen.Text="Full Screen"
chx_full_screen.Skin=CommonUI.com_chx_skin
dlg:InsertControl(chx_full_screen)

item_y=item_y-item_height
local lbl_render_device=Static()
lbl_render_device.Location=Vector2(0,item_y)
lbl_render_device.Size=Vector2(190,22)
lbl_render_device.Text="Render Device"
lbl_render_device.Skin=CommonUI.com_lbl_skin
lbl_render_device.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_render_device)

local cbx_render_device=ComboBox()
cbx_render_device.Name="cbx_render_device"
cbx_render_device.Location=Vector2(201,item_y)
cbx_render_device.Size=Vector2(304,22)
cbx_render_device.ScrollbarWidth=20
cbx_render_device.DropdownSize=Vector2(304-20,130)
cbx_render_device.Border=Vector4(20,0,20,0)
cbx_render_device.ItemHeight=22
cbx_render_device.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_render_device)

item_y=item_y-item_height
local lbl_display_adapter=Static()
lbl_display_adapter.Location=Vector2(0,item_y)
lbl_display_adapter.Size=Vector2(190,22)
lbl_display_adapter.Text="Display Adapter"
lbl_display_adapter.Skin=CommonUI.com_lbl_skin
lbl_display_adapter.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_display_adapter)

local cbx_display_adapter=ComboBox()
cbx_display_adapter.Name="cbx_display_adapter"
cbx_display_adapter.Location=Vector2(201,item_y)
cbx_display_adapter.Size=Vector2(304,22)
cbx_display_adapter.ScrollbarWidth=20
cbx_display_adapter.DropdownSize=Vector2(304-20,130)
cbx_display_adapter.Border=Vector4(20,0,20,0)
cbx_display_adapter.ItemHeight=22
cbx_display_adapter.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_display_adapter)
-- game:SaveDialog(dlg, "ui/Settings.ui.xml")
-- dlg=game:LoadDialog("ui/Settings.ui.xml")

game:InsertDlg(dlg)
dlg.Visible=false
dlg.EventAlign=function(arg)
	dlg.Location=(game.DlgViewport-dlg.Size)*0.5
end
dlg.EventVisibleChanged=function(arg)
	if arg.Visible then
		RefreshDisplayAdapter()
		game:PlaySound("demo2_3/untitled/15")
	end
end

local local_device_settings=nil

local btn_ok=dlg:FindControl("btn_ok")
btn_ok.EventMouseClick=function(arg)
	-- print("Settings.btn_ok.EventMouseClick")
	assert(local_device_settings)
	game:ChangeDevice(local_device_settings)
	dlg.Visible=false
end

local btn_cancel=dlg:FindControl("btn_cancel")
btn_cancel.EventMouseClick=function(arg)
	-- print("Settings.OnCancelBtnClicked")
	assert(local_device_settings)
	dlg.Visible=false
end

local cbx_vertical_sync=dlg:FindControl("cbx_vertical_sync")
cbx_vertical_sync.EventSelectionChanged=function(arg)
	OnPresentIntervalChanged()
end

local cbx_vertex_processing=dlg:FindControl("cbx_vertex_processing")
cbx_vertex_processing.EventSelectionChanged=function(arg)
	OnVertexProcessingChanged()
end

local cbx_multisample_quality=dlg:FindControl("cbx_multisample_quality")
cbx_multisample_quality.EventSelectionChanged=function(arg)
	OnMultisampleQualityChanged()
end

local cbx_multisample_type=dlg:FindControl("cbx_multisample_type")
cbx_multisample_type.EventSelectionChanged=function(arg)
	OnMultisampleTypeChanged()
end

local cbx_depth_stencil_format=dlg:FindControl("cbx_depth_stencil_format")
cbx_depth_stencil_format.EventSelectionChanged=function(arg)
	OnDepthStencilBufferFormatChanged()
end

local cbx_back_buffer_format=dlg:FindControl("cbx_back_buffer_format")
cbx_back_buffer_format.EventSelectionChanged=function(arg)
	OnBackBufferFormatChanged()
end

local cbx_refresh_rate=dlg:FindControl("cbx_refresh_rate")
cbx_refresh_rate.EventSelectionChanged=function(arg)
	OnRefreshRateChanged()
end

local cbx_resolution=dlg:FindControl("cbx_resolution")
cbx_resolution.EventSelectionChanged=function(arg)
	OnResolutionChanged()
end

local cbx_adapter_format=dlg:FindControl("cbx_adapter_format")
cbx_adapter_format.EventSelectionChanged=function(arg)
	OnAdapterFormatChanged()
end

local chx_windowed=dlg:FindControl("chx_windowed")
local chx_full_screen=dlg:FindControl("chx_full_screen")
chx_windowed.EventMouseClick=function(arg)
	chx_full_screen.Checked=not chx_windowed.Checked
	OnWindowedFullScreenChanged()
end
chx_full_screen.EventMouseClick=function(arg)
	chx_windowed.Checked=not chx_full_screen.Checked
	OnWindowedFullScreenChanged()
end

local cbx_render_device=dlg:FindControl("cbx_render_device")
cbx_render_device.EventSelectionChanged=function(arg)
	OnDeviceTypeChanged()
end

local cbx_display_adapter=dlg:FindControl("cbx_display_adapter")
cbx_display_adapter.EventSelectionChanged=function(arg)
	OnAdapterChanged()
end

function RefreshDisplayAdapter()
	local_device_settings=game.DeviceSettings
	local adapter_info_list=game:GetAdapterInfoList()
	cbx_display_adapter:RemoveAllItems()
	cbx_display_adapter.Selected=0
	for i=0,adapter_info_list:GetSize()-1 do
		local adapter_info=adapter_info_list:GetAt(i)
		cbx_display_adapter:AddItem(adapter_info.szUniqueDescription)
		cbx_display_adapter:SetItemData(i,adapter_info.AdapterOrdinal)
		if local_device_settings.AdapterOrdinal == adapter_info.AdapterOrdinal then
			cbx_display_adapter.Selected=i
		end
	end
	OnAdapterChanged()
	
	local vpt_table={}
	if game.SoftwareVP then
		table.insert(vpt_table, DXUTD3D9DeviceSettings.D3DCREATE_SOFTWARE_VERTEXPROCESSING)
	end
	if game.HardwareVP then
		table.insert(vpt_table, DXUTD3D9DeviceSettings.D3DCREATE_HARDWARE_VERTEXPROCESSING)
	end
	if game.PureHardwareVP then
		table.insert(vpt_table, DXUTD3D9DeviceSettings.D3DCREATE_PUREDEVICE)
	end
	if game.MixedVP then
		table.insert(vpt_table, DXUTD3D9DeviceSettings.D3DCREATE_MIXED_VERTEXPROCESSING)
	end
	
	cbx_vertex_processing:RemoveAllItems()
	cbx_vertex_processing.Selected=0
	for i, vertex_processing_type in ipairs(vpt_table) do
		local item_idx=i-1
		cbx_vertex_processing:AddItem(DxutApp.DXUTVertexProcessingTypeToString(vertex_processing_type))
		cbx_vertex_processing:SetItemData(item_idx,vertex_processing_type)
		if bit.band(local_device_settings.BehaviorFlags, vertex_processing_type) ~= 0 then
			cbx_vertex_processing.Selected=item_idx
		end
	end
	OnVertexProcessingChanged()
	
	cbx_vertical_sync:RemoveAllItems()
	cbx_vertical_sync.Selected=0
	cbx_vertical_sync:AddItem("On")
	cbx_vertical_sync:SetItemData(cbx_vertical_sync:GetNumItems()-1,DXUTD3D9DeviceSettings.D3DPRESENT_INTERVAL_DEFAULT)
	if bit.tobit(local_device_settings.pp.PresentationInterval) == DXUTD3D9DeviceSettings.D3DPRESENT_INTERVAL_DEFAULT then
		cbx_vertical_sync.Selected=cbx_vertical_sync:GetNumItems()-1
	end
	cbx_vertical_sync:AddItem("Off")
	cbx_vertical_sync:SetItemData(cbx_vertical_sync:GetNumItems()-1,DXUTD3D9DeviceSettings.D3DPRESENT_INTERVAL_IMMEDIATE)
	if bit.tobit(local_device_settings.pp.PresentationInterval) == DXUTD3D9DeviceSettings.D3DPRESENT_INTERVAL_IMMEDIATE then
		cbx_vertical_sync.Selected=cbx_vertical_sync:GetNumItems()-1
	end
	OnPresentIntervalChanged()
end

local function IsComboBoxSelectedValid(cbx)
	return cbx.Selected >= 0 and cbx.Selected < cbx:GetNumItems()
end

local function GetComboBoxSelectedData(cbx)
	assert(IsComboBoxSelectedValid(cbx))
	return cbx:GetItemData(cbx.Selected)
end

function OnAdapterChanged()
	if IsComboBoxSelectedValid(cbx_display_adapter) then
		-- print("Settings.OnAdapterChanged")
		assert(local_device_settings)
		local_device_settings.AdapterOrdinal=GetComboBoxSelectedData(cbx_display_adapter)
		
		local adapter_info=game:GetAdapterInfo(GetComboBoxSelectedData(cbx_display_adapter))
		cbx_render_device:RemoveAllItems()
		cbx_render_device.Selected=0
		for i=0,adapter_info.deviceInfoList:GetSize()-1 do
			local device_info=adapter_info.deviceInfoList:GetAt(i)
			cbx_render_device:AddItem(DxutApp.DXUTD3DDeviceTypeToString(device_info.DeviceType))
			cbx_render_device:SetItemData(i,device_info.DeviceType)
			if local_device_settings.DeviceType == device_info.DeviceType then
				cbx_render_device.Selected=i
			end
		end
		OnDeviceTypeChanged()
	end
end

function OnDeviceTypeChanged()
	if IsComboBoxSelectedValid(cbx_render_device) then
		-- print("Settings.OnDeviceTypeChanged")
		assert(local_device_settings)
		local_device_settings.DeviceType=GetComboBoxSelectedData(cbx_render_device)
		
		local adapter_original=GetComboBoxSelectedData(cbx_display_adapter)
		local device_type=GetComboBoxSelectedData(cbx_render_device)
		local device_info=game:GetDeviceInfo(adapter_original,device_type)
		chx_windowed.Enable=false
		chx_full_screen.Enable=false
		for i=0,device_info.deviceSettingsComboList:GetSize()-1 do
			local device_settings_combo = device_info.deviceSettingsComboList:GetAt(i)
			if device_settings_combo.AdapterOrdinal == adapter_original
				and device_settings_combo.DeviceType == device_type then
				if device_settings_combo.Windowed then
					chx_windowed.Enable=true
				else
					chx_full_screen.Enable=true
				end
			end
		end
		if local_device_settings.pp.Windowed ~= 0 then
			chx_windowed.Checked=true
			chx_full_screen.Checked=false
		else
			chx_windowed.Checked=false
			chx_full_screen.Checked=true
		end
		OnWindowedFullScreenChanged()
	end
end

local function SameBool(lhs, rhs)
	return (lhs and rhs) or (not lhs and not rhs)
end

function OnWindowedFullScreenChanged()
	if chx_windowed.Checked or chx_full_screen.Checked then
		-- print("Settings.OnWindowedFullScreenChanged")
		assert(local_device_settings)
		local_device_settings.pp.Windowed=chx_windowed.Checked and 1 or 0
		
		local adapter_original=GetComboBoxSelectedData(cbx_display_adapter)
		local device_type=GetComboBoxSelectedData(cbx_render_device)
		local device_info=game:GetDeviceInfo(adapter_original,device_type)
		cbx_adapter_format:RemoveAllItems()
		cbx_adapter_format.Selected=0
		for i=0,device_info.deviceSettingsComboList:GetSize()-1 do
			local device_settings_combo=device_info.deviceSettingsComboList:GetAt(i)
			assert(device_settings_combo.AdapterOrdinal == adapter_original)
			assert(device_settings_combo.DeviceType == device_type)
			if SameBool(device_settings_combo.Windowed ~= 0,chx_windowed.Checked) then
				local item_idx=cbx_adapter_format:GetNumItems()
				local adapter_format_desc=DxutApp.DXUTD3DFormatToString(device_settings_combo.AdapterFormat)
				if not cbx_adapter_format:ContainsItem(adapter_format_desc,0) then
					cbx_adapter_format:AddItem(adapter_format_desc)
					cbx_adapter_format:SetItemData(item_idx,device_settings_combo.AdapterFormat)
					if local_device_settings.AdapterFormat == device_settings_combo.AdapterFormat then
						cbx_adapter_format.Selected=item_idx
					end
				end
			end
		end
		OnAdapterFormatChanged()
	end
end

local function MAKELONG(low, high)
	return bit.tobit(bit.bor(bit.band(low,0xffff),bit.lshift(bit.band(high,0xffff),16)))
end

local function LOWORD(value)
	return bit.band(value,0xffff)
end

local function HIWORD(value)
	return bit.rshift(bit.band(bit.tobit(value),0xffff0000),16)
end

function OnAdapterFormatChanged()
	if IsComboBoxSelectedValid(cbx_adapter_format) then
		-- print("Settings.OnAdapterFormatChanged")
		assert(local_device_settings)
		local_device_settings.AdapterFormat=GetComboBoxSelectedData(cbx_adapter_format)
		
		local adapter_info=game:GetAdapterInfo(GetComboBoxSelectedData(cbx_display_adapter))
		cbx_resolution:RemoveAllItems()
		cbx_resolution.Selected=0
		for i=0,adapter_info.displayModeList:GetSize()-1 do
			local display_mode=adapter_info.displayModeList:GetAt(i)
			if display_mode.Format == GetComboBoxSelectedData(cbx_adapter_format) then
				local resolution_desc=string.format("%d by %d",display_mode.Width,display_mode.Height)
				if not cbx_resolution:ContainsItem(resolution_desc,0) then
					local item_idx=cbx_resolution:GetNumItems()
					cbx_resolution:AddItem(resolution_desc)
					cbx_resolution:SetItemData(item_idx,MAKELONG(display_mode.Width,display_mode.Height))
					if local_device_settings.pp.BackBufferWidth == display_mode.Width
						and local_device_settings.pp.BackBufferHeight == display_mode.Height then
						cbx_resolution.Selected=item_idx
					end
				end
			end
		end
		OnResolutionChanged()
		
		local adapter_original=GetComboBoxSelectedData(cbx_display_adapter)
		local device_type=GetComboBoxSelectedData(cbx_render_device)
		local device_info=game:GetDeviceInfo(adapter_original,device_type)
		cbx_back_buffer_format:RemoveAllItems()
		cbx_back_buffer_format.Selected=0
		for i=0,device_info.deviceSettingsComboList:GetSize()-1 do
			local device_settings_combo=device_info.deviceSettingsComboList:GetAt(i)
			assert(device_settings_combo.AdapterOrdinal == adapter_original)
			assert(device_settings_combo.DeviceType == device_type)
			if SameBool(device_settings_combo.Windowed ~= 0,chx_windowed.Checked)
				and device_settings_combo.AdapterFormat == GetComboBoxSelectedData(cbx_adapter_format) then
				local item_idx=cbx_back_buffer_format:GetNumItems()
				cbx_back_buffer_format:AddItem(DxutApp.DXUTD3DFormatToString(device_settings_combo.BackBufferFormat))
				cbx_back_buffer_format:SetItemData(item_idx,device_settings_combo.BackBufferFormat)
				if local_device_settings.pp.BackBufferFormat == device_settings_combo.BackBufferFormat then
					cbx_back_buffer_format.Selected=item_idx
				end
			end
		end
		OnBackBufferFormatChanged()
	end
end

function OnResolutionChanged()
	if IsComboBoxSelectedValid(cbx_resolution) then
		-- print("Settings.OnResolutionChanged")
		assert(local_device_settings)
		local_device_settings.pp.BackBufferWidth=LOWORD(GetComboBoxSelectedData(cbx_resolution))
		local_device_settings.pp.BackBufferHeight=HIWORD(GetComboBoxSelectedData(cbx_resolution))
		
		cbx_refresh_rate:RemoveAllItems()
		cbx_refresh_rate.Selected=0
		-- Only full screen mode, the refresh_rate is valid
		if chx_windowed.Checked then
			cbx_refresh_rate:AddItem("Default Rate")
			cbx_refresh_rate:SetItemData(0,0)
			cbx_refresh_rate.Enabled=false
		else
			local adapter_info=game:GetAdapterInfo(GetComboBoxSelectedData(cbx_display_adapter))
			local width=LOWORD(GetComboBoxSelectedData(cbx_resolution))
			local height=HIWORD(GetComboBoxSelectedData(cbx_resolution))
			for i=0,adapter_info.displayModeList:GetSize()-1 do
				local display_mode=adapter_info.displayModeList:GetAt(i)
				if display_mode.Format == GetComboBoxSelectedData(cbx_adapter_format)
					and display_mode.Width == width and display_mode.Height == height then
					local refresh_rate=display_mode.RefreshRate
					local item_idx=cbx_refresh_rate:GetNumItems()
					if 0 == refresh_rate then
						cbx_refresh_rate:AddItem("Default Rate")
					else
						cbx_refresh_rate:AddItem(string.format("%d Hz", refresh_rate))
					end
					cbx_refresh_rate:SetItemData(item_idx,refresh_rate)
					if local_device_settings.pp.FullScreen_RefreshRateInHz == refresh_rate then
						cbx_refresh_rate.Selected=item_idx
					end
				end
			end
			cbx_refresh_rate.Enabled=true
		end
		OnRefreshRateChanged()
	end
end

function OnRefreshRateChanged()
	if IsComboBoxSelectedValid(cbx_refresh_rate) then
		-- print("Settings.OnRefreshRateChanged")
		assert(local_device_settings)
		local_device_settings.pp.FullScreen_RefreshRateInHz=GetComboBoxSelectedData(cbx_refresh_rate)
	end
end

function OnBackBufferFormatChanged()
	if IsComboBoxSelectedValid(cbx_back_buffer_format) then
		-- print("Settings.OnBackBufferFormatChanged")
		assert(local_device_settings)
		local_device_settings.pp.BackBufferFormat=GetComboBoxSelectedData(cbx_back_buffer_format)
		
		local device_settings_combo = game:GetDeviceSettingsCombo(
			GetComboBoxSelectedData(cbx_display_adapter),
			GetComboBoxSelectedData(cbx_render_device),
			GetComboBoxSelectedData(cbx_adapter_format),
			GetComboBoxSelectedData(cbx_back_buffer_format),
			chx_windowed.Checked and 1 or 0)
		cbx_depth_stencil_format:RemoveAllItems()
		cbx_depth_stencil_format.Selected=0
		-- Only EnableAutoDepthStencil can select Depth/Stencil format
		if local_device_settings.pp.EnableAutoDepthStencil ~= 0 then
			for i=0,device_settings_combo.depthStencilFormatList:GetSize()-1 do
				local depth_stencil_format=device_settings_combo.depthStencilFormatList:GetAt(i)
				cbx_depth_stencil_format:AddItem(DxutApp.DXUTD3DFormatToString(depth_stencil_format))
				cbx_depth_stencil_format:SetItemData(i,depth_stencil_format)
				if local_device_settings.pp.AutoDepthStencilFormat == depth_stencil_format then
					cbx_depth_stencil_format.Selected=i
				end
			end
			cbx_depth_stencil_format.Enabled=true
		else
			cbx_depth_stencil_format:AddItem("(not used)")
			cbx_depth_stencil_format:SetItemData(0,0)
			cbx_depth_stencil_format.Enabled=false
		end
		OnDepthStencilBufferFormatChanged()
	end
end

function OnDepthStencilBufferFormatChanged()
	if IsComboBoxSelectedValid(cbx_depth_stencil_format) then
		-- print("Settings.OnDepthStencilBufferFormatChanged")
		assert(local_device_settings)
		local_device_settings.pp.AutoDepthStencilFormat=GetComboBoxSelectedData(cbx_depth_stencil_format)
		
		local device_settings_combo = game:GetDeviceSettingsCombo(
			GetComboBoxSelectedData(cbx_display_adapter),
			GetComboBoxSelectedData(cbx_render_device),
			GetComboBoxSelectedData(cbx_adapter_format),
			GetComboBoxSelectedData(cbx_back_buffer_format),
			chx_windowed.Checked and 1 or 0)
		local depth_stencil_format=GetComboBoxSelectedData(cbx_depth_stencil_format)
		cbx_multisample_type:RemoveAllItems()
		cbx_multisample_type.Selected=0
		for i=0,device_settings_combo.multiSampleTypeList:GetSize()-1 do
			local multi_sample_type=device_settings_combo.multiSampleTypeList:GetAt(i)
			if not device_settings_combo:IsDepthStencilMultiSampleConflict(depth_stencil_format,multi_sample_type) then
				local item_idx=cbx_multisample_type:GetNumItems()
				cbx_multisample_type:AddItem(DxutApp.DXUTMultisampleTypeToString(multi_sample_type))
				cbx_multisample_type:SetItemData(item_idx,multi_sample_type)
				if local_device_settings.pp.MultiSampleType == multi_sample_type then
					cbx_multisample_type.Selected=item_idx
				end
			end
		end
		OnMultisampleTypeChanged()
	end
end

function OnMultisampleTypeChanged()
	if IsComboBoxSelectedValid(cbx_multisample_type) then
		-- print("Settings.OnMultisampleTypeChanged")
		assert(local_device_settings)
		local_device_settings.pp.MultiSampleType=GetComboBoxSelectedData(cbx_multisample_type)
		
		local device_settings_combo = game:GetDeviceSettingsCombo(
			GetComboBoxSelectedData(cbx_display_adapter),
			GetComboBoxSelectedData(cbx_render_device),
			GetComboBoxSelectedData(cbx_adapter_format),
			GetComboBoxSelectedData(cbx_back_buffer_format),
			chx_windowed.Checked and 1 or 0)
		local multi_sample_type=GetComboBoxSelectedData(cbx_multisample_type)
		cbx_multisample_quality:RemoveAllItems()
		cbx_multisample_quality.Selected=0
		for i=0,device_settings_combo.multiSampleTypeList:GetSize()-1 do
			if multi_sample_type == device_settings_combo.multiSampleTypeList:GetAt(i) then
				local max_quality = device_settings_combo.multiSampleQualityList:GetAt(i)
				for quality=0,max_quality-1 do
					cbx_multisample_quality:AddItem(string.format("%d", quality))
					cbx_multisample_quality:SetItemData(quality,quality)
					if local_device_settings.pp.MultiSampleQuality == quality then
						cbx_multisample_quality.Selected=quality
					end
				end
				break
			end
		end
		OnMultisampleQualityChanged()
	end
end

function OnMultisampleQualityChanged()
	if IsComboBoxSelectedValid(cbx_multisample_quality) then
		-- print("Settings.OnMultisampleQualityChanged")
		assert(local_device_settings)
		local_device_settings.pp.MultiSampleQuality=GetComboBoxSelectedData(cbx_multisample_quality)
	end
end

function OnVertexProcessingChanged()
	if IsComboBoxSelectedValid(cbx_vertex_processing) then
		-- print("Settings.OnVertexProcessingChanged")
		assert(local_device_settings)
		local_device_settings.BehaviorFlags=bit.band(
			local_device_settings.BehaviorFlags,bit.bnot(DXUTD3D9DeviceSettings.D3DCREATE_SOFTWARE_VERTEXPROCESSING))
		local_device_settings.BehaviorFlags=bit.band(
			local_device_settings.BehaviorFlags,bit.bnot(DXUTD3D9DeviceSettings.D3DCREATE_HARDWARE_VERTEXPROCESSING))
		local_device_settings.BehaviorFlags=bit.band(
			local_device_settings.BehaviorFlags,bit.bnot(DXUTD3D9DeviceSettings.D3DCREATE_PUREDEVICE))
		local_device_settings.BehaviorFlags=bit.band(
			local_device_settings.BehaviorFlags,bit.bnot(DXUTD3D9DeviceSettings.D3DCREATE_MIXED_VERTEXPROCESSING))
		local_device_settings.BehaviorFlags=bit.bor(
			local_device_settings.BehaviorFlags,GetComboBoxSelectedData(cbx_vertex_processing))
		if bit.band(local_device_settings.BehaviorFlags,DXUTD3D9DeviceSettings.D3DCREATE_PUREDEVICE) then
			local_device_settings.BehaviorFlags=bit.bor(
				local_device_settings.BehaviorFlags,DXUTD3D9DeviceSettings.D3DCREATE_HARDWARE_VERTEXPROCESSING)
		end
	end
end

function OnPresentIntervalChanged()
	if IsComboBoxSelectedValid(cbx_vertical_sync) then
		-- print("Settings.OnPresentIntervalChanged")
		assert(local_device_settings)
		local_device_settings.pp.PresentationInterval=GetComboBoxSelectedData(cbx_vertical_sync)
	end
end
