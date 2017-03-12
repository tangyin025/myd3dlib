
----------------------------------------------------------------------------------------------------
-- dlg_settings
----------------------------------------------------------------------------------------------------

local dlg_settings=game:LoadDialog("ui/Settings.ui.xml")
game:InsertDlg(dlg_settings)
dlg_settings.Visible=false
dlg_settings.EventAlign=function(args)
	dlg_settings.Location=(game.DlgViewport-dlg_settings.Size)*0.5
end
dlg_settings.EventRefresh=function(args)
	RefreshDisplayAdapter()
	game:PlaySound("aaa/untitled/15")
end

local local_device_settings=nil

local btn_ok=dlg_settings:FindControl("btn_ok")
btn_ok.EventClick=function(args)
	-- print("Settings.btn_ok.EventClick")
	assert(local_device_settings)
	game:ChangeDevice(local_device_settings)
	dlg_settings.Visible=false
end

local btn_cancel=dlg_settings:FindControl("btn_cancel")
btn_cancel.EventClick=function(args)
	-- print("Settings.OnCancelBtnClicked")
	assert(local_device_settings)
	dlg_settings.Visible=false
end

local cbx_vertical_sync=dlg_settings:FindControl("cbx_vertical_sync")
cbx_vertical_sync.EventSelectionChanged=function(args)
	OnPresentIntervalChanged()
end

local cbx_vertex_processing=dlg_settings:FindControl("cbx_vertex_processing")
cbx_vertex_processing.EventSelectionChanged=function(args)
	OnVertexProcessingChanged()
end

local cbx_multisample_quality=dlg_settings:FindControl("cbx_multisample_quality")
cbx_multisample_quality.EventSelectionChanged=function(args)
	OnMultisampleQualityChanged()
end

local cbx_multisample_type=dlg_settings:FindControl("cbx_multisample_type")
cbx_multisample_type.EventSelectionChanged=function(args)
	OnMultisampleTypeChanged()
end

local cbx_depth_stencil_format=dlg_settings:FindControl("cbx_depth_stencil_format")
cbx_depth_stencil_format.EventSelectionChanged=function(args)
	OnDepthStencilBufferFormatChanged()
end

local cbx_back_buffer_format=dlg_settings:FindControl("cbx_back_buffer_format")
cbx_back_buffer_format.EventSelectionChanged=function(args)
	OnBackBufferFormatChanged()
end

local cbx_refresh_rate=dlg_settings:FindControl("cbx_refresh_rate")
cbx_refresh_rate.EventSelectionChanged=function(args)
	OnRefreshRateChanged()
end

local cbx_resolution=dlg_settings:FindControl("cbx_resolution")
cbx_resolution.EventSelectionChanged=function(args)
	OnResolutionChanged()
end

local cbx_adapter_format=dlg_settings:FindControl("cbx_adapter_format")
cbx_adapter_format.EventSelectionChanged=function(args)
	OnAdapterFormatChanged()
end

local chx_windowed=dlg_settings:FindControl("chx_windowed")
local chx_full_screen=dlg_settings:FindControl("chx_full_screen")
chx_windowed.EventClick=function(args)
	chx_full_screen.Checked=not chx_windowed.Checked
	OnWindowedFullScreenChanged()
end
chx_full_screen.EventClick=function(args)
	chx_windowed.Checked=not chx_full_screen.Checked
	OnWindowedFullScreenChanged()
end

local cbx_render_device=dlg_settings:FindControl("cbx_render_device")
cbx_render_device.EventSelectionChanged=function(args)
	OnDeviceTypeChanged()
end

local cbx_display_adapter=dlg_settings:FindControl("cbx_display_adapter")
cbx_display_adapter.EventSelectionChanged=function(args)
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

----------------------------------------------------------------------------------------------------
-- dlg_hud
----------------------------------------------------------------------------------------------------

local dlg_hud=game:LoadDialog("ui/Hud.ui.xml")
game:InsertDlg(dlg_hud)
dlg_hud.EventAlign=function(args)
	dlg_hud.Location=Vector2(game.DlgViewport.x-170,0)
end

local btn_toggle_fs=dlg_hud:FindControl("btn_toggle_fs")
btn_toggle_fs.EventClick=function(args)
	game:ToggleFullScreen()
end

local btn_toggle_ref=dlg_hud:FindControl("btn_toggle_ref")
btn_toggle_ref.EventClick=function(args)
	game:ToggleREF()
end

local btn_change_device=dlg_hud:FindControl("btn_change_device")
btn_change_device.EventClick=function(args)
	dlg_settings.Visible=not dlg_settings.Visible
end

-- 设置相机
local k=math.cos(math.rad(45))
local d=20
game.Camera.Eye=Vector3(d*k*k,d*k+1,d*k*k)
game.Camera.Eular=Vector3(math.rad(-45),math.rad(45),0)
game.Camera.EventAlign=function(args)
	local desc=game.BackBufferSurfaceDesc
	game.Camera.Aspect=desc.Width/desc.Height
end
game.SkyLightCam.Eular=Vector3(math.rad(-45),math.rad(0),0)

-- 设置环境光
game.SkyLightCam.Eye=Vector3(0,0,0)
game.SkyLightCam.Eular=Vector3(math.rad(-30),math.rad(0),0)
game.SkyLightCam.Width=50
game.SkyLightCam.Height=50
game.SkyLightCam.Nz=-50
game.SkyLightCam.Fz=50
game.SkyLightDiffuse=Vector4(1.0,1.0,1.0,1.0)
game.SkyLightAmbient=Vector4(0.0,0.0,0.0,0.0)

-- 添加特效
local cmp = SphericalEmitterComponent(AABB(-10,10),Vector3(0,0,0),Quaternion(0,0,0,1),Vector3(1,1,1))
cmp.Emitter = Emitter()
cmp.SpawnInterval=1/100
cmp.ParticleLifeTime=10
cmp.SpawnSpeed=5
cmp.SpawnInclination:AddNode(0,math.rad(45),0,0)
local Azimuth=math.rad(360)*8
cmp.SpawnAzimuth:AddNode(0,0,Azimuth/10,Azimuth/10)
cmp.SpawnAzimuth:AddNode(10,Azimuth,Azimuth/10,Azimuth/10)
cmp.SpawnColorA:AddNode(0,1,0,0)
cmp.SpawnColorA:AddNode(10,0,0,0)
cmp.SpawnColorR:AddNode(0,1,0,0)
cmp.SpawnColorR:AddNode(10,0,0,0)
cmp.SpawnColorG:AddNode(0,1,0,0)
cmp.SpawnColorG:AddNode(10,0,0,0)
cmp.SpawnColorB:AddNode(0,1,0,0)
cmp.SpawnColorB:AddNode(10,0,0,0)
cmp.SpawnSizeX:AddNode(0,1,0,0)
cmp.SpawnSizeX:AddNode(10,10,0,0)
cmp.SpawnSizeY:AddNode(0,1,0,0)
cmp.SpawnSizeY:AddNode(10,10,0,0)
local material=Material()
material.Shader="particle1.fx"
material.PassMask=Material.PassMaskTransparent
material.MeshTexture.Path="texture/flare.dds"
cmp.Material = material
cmp:RequestResource()
local actor = Actor()
actor:AddComponent(cmp)
game.Root:AddActor(cmp2oct(actor),actor.aabb:transform(actor.World),0.1)
