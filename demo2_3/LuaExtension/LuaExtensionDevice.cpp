#include "stdafx.h"
#include "../Game.h"
#include "LuaExtension.h"

using namespace luabind;

void ExportDevice2Lua(lua_State * L)
{
	module(L)
	[
		class_<D3DSURFACE_DESC>("D3DSURFACE_DESC")
			.def_readwrite("Format", &D3DSURFACE_DESC::Format)
			.def_readwrite("Type", &D3DSURFACE_DESC::Type)
			.def_readwrite("Usage", &D3DSURFACE_DESC::Usage)
			.def_readwrite("Pool", &D3DSURFACE_DESC::Pool)
			.def_readwrite("MultiSampleType", &D3DSURFACE_DESC::MultiSampleType)
			.def_readwrite("MultiSampleQuality", &D3DSURFACE_DESC::MultiSampleQuality)
			.def_readwrite("Width", &D3DSURFACE_DESC::Width)
			.def_readwrite("Height", &D3DSURFACE_DESC::Height)

		, class_<D3DPRESENT_PARAMETERS>("D3DPRESENT_PARAMETERS")
			.def_readwrite("BackBufferWidth", &D3DPRESENT_PARAMETERS::BackBufferWidth)
			.def_readwrite("BackBufferHeight", &D3DPRESENT_PARAMETERS::BackBufferHeight)
			.def_readwrite("BackBufferFormat", &D3DPRESENT_PARAMETERS::BackBufferFormat)
			.def_readwrite("BackBufferCount", &D3DPRESENT_PARAMETERS::BackBufferCount)
			.def_readwrite("MultiSampleType", &D3DPRESENT_PARAMETERS::MultiSampleType)
			.def_readwrite("MultiSampleQuality", &D3DPRESENT_PARAMETERS::MultiSampleQuality)
			.def_readwrite("SwapEffect", &D3DPRESENT_PARAMETERS::SwapEffect)
			.def_readwrite("hDeviceWindow", &D3DPRESENT_PARAMETERS::hDeviceWindow)
			.def_readwrite("Windowed", &D3DPRESENT_PARAMETERS::Windowed)
			.def_readwrite("EnableAutoDepthStencil", &D3DPRESENT_PARAMETERS::EnableAutoDepthStencil)
			.def_readwrite("AutoDepthStencilFormat", &D3DPRESENT_PARAMETERS::AutoDepthStencilFormat)
			.def_readwrite("Flags", &D3DPRESENT_PARAMETERS::Flags)
			.def_readwrite("FullScreen_RefreshRateInHz", &D3DPRESENT_PARAMETERS::FullScreen_RefreshRateInHz)
			.def_readwrite("PresentationInterval", &D3DPRESENT_PARAMETERS::PresentationInterval)

		, class_<D3DDISPLAYMODE>("D3DDISPLAYMODE")
			.def_readwrite("Width", &D3DDISPLAYMODE::Width)
			.def_readwrite("Height", &D3DDISPLAYMODE::Height)
			.def_readwrite("RefreshRate", &D3DDISPLAYMODE::RefreshRate)
			.def_readwrite("Format", &D3DDISPLAYMODE::Format)

		, class_<CGrowableArray<D3DDISPLAYMODE> >("CD3D9EnumAdapterInfoArray")
			.def("GetAt", &CGrowableArray<D3DDISPLAYMODE>::GetAt)
			.def("GetSize", &CGrowableArray<D3DDISPLAYMODE>::GetSize)

		, class_<DXUTD3D9DeviceSettings>("DXUTD3D9DeviceSettings")
			.enum_("VertexProcessingType")
			[
				value("D3DCREATE_SOFTWARE_VERTEXPROCESSING", D3DCREATE_SOFTWARE_VERTEXPROCESSING),
				value("D3DCREATE_MIXED_VERTEXPROCESSING", D3DCREATE_MIXED_VERTEXPROCESSING),
				value("D3DCREATE_HARDWARE_VERTEXPROCESSING", D3DCREATE_HARDWARE_VERTEXPROCESSING),
				value("D3DCREATE_PUREDEVICE", D3DCREATE_PUREDEVICE)
			]
			.enum_("PresentIntervalType")
			[
				value("D3DPRESENT_INTERVAL_DEFAULT", D3DPRESENT_INTERVAL_DEFAULT),
				value("D3DPRESENT_INTERVAL_IMMEDIATE", D3DPRESENT_INTERVAL_IMMEDIATE)
			]
			.def_readwrite("AdapterOrdinal", &DXUTD3D9DeviceSettings::AdapterOrdinal)
			.def_readwrite("DeviceType", &DXUTD3D9DeviceSettings::DeviceType)
			.def_readwrite("AdapterFormat", &DXUTD3D9DeviceSettings::AdapterFormat)
			.def_readwrite("BehaviorFlags", &DXUTD3D9DeviceSettings::BehaviorFlags)
			.def_readwrite("pp", &DXUTD3D9DeviceSettings::pp)

		, class_<CD3D9EnumAdapterInfo>("CD3D9EnumAdapterInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumAdapterInfo::AdapterOrdinal)
			.def_readonly("szUniqueDescription", &CD3D9EnumAdapterInfo::szUniqueDescription)
			.def_readonly("displayModeList", &CD3D9EnumAdapterInfo::displayModeList)
			.def_readonly("deviceInfoList", &CD3D9EnumAdapterInfo::deviceInfoList)

		, class_<CGrowableArray<CD3D9EnumAdapterInfo *> >("CD3D9EnumAdapterInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetSize)

		, class_<CD3D9EnumDeviceInfo>("CD3D9EnumDeviceInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceInfo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceInfo::DeviceType)
			.def_readonly("deviceSettingsComboList", &CD3D9EnumDeviceInfo::deviceSettingsComboList)

		, class_<CGrowableArray<CD3D9EnumDeviceInfo *> >("CD3D9EnumDeviceInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetSize)

		, class_<CGrowableArray<D3DFORMAT> >("D3DFORMATArray")
			.def("GetAt", &CGrowableArray<D3DFORMAT>::GetAt)
			.def("GetSize", &CGrowableArray<D3DFORMAT>::GetSize)

		, class_<CGrowableArray<D3DMULTISAMPLE_TYPE> >("D3DMULTISAMPLE_TYPEArray")
			.def("GetAt", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetAt)
			.def("GetSize", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetSize)

		, class_<CGrowableArray<DWORD> >("DWORDArray")
			.def("GetAt", (const DWORD & (CGrowableArray<DWORD>::*)(int))&CGrowableArray<DWORD>::GetAt) // ! forced convert to const ref
			.def("GetSize", &CGrowableArray<DWORD>::GetSize)

		, class_<CD3D9EnumDeviceSettingsCombo>("CD3D9EnumDeviceSettingsCombo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceSettingsCombo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceSettingsCombo::DeviceType)
			.def_readonly("AdapterFormat", &CD3D9EnumDeviceSettingsCombo::AdapterFormat)
			.def_readonly("BackBufferFormat", &CD3D9EnumDeviceSettingsCombo::BackBufferFormat)
			.def_readonly("Windowed", &CD3D9EnumDeviceSettingsCombo::Windowed)
			.def_readonly("depthStencilFormatList", &CD3D9EnumDeviceSettingsCombo::depthStencilFormatList)
			.def_readonly("multiSampleTypeList", &CD3D9EnumDeviceSettingsCombo::multiSampleTypeList)
			.def_readonly("multiSampleQualityList", &CD3D9EnumDeviceSettingsCombo::multiSampleQualityList)
			.def("IsDepthStencilMultiSampleConflict", &CD3D9EnumDeviceSettingsCombo::IsDepthStencilMultiSampleConflict)

		, class_<CGrowableArray<CD3D9EnumDeviceSettingsCombo *> >("CD3D9EnumDeviceSettingsComboArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetSize)

		, class_<CD3D9Enumeration>("CD3D9Enumeration")
			.def("GetAdapterInfoList", &CD3D9Enumeration::GetAdapterInfoList)
			.def("GetAdapterInfo", &CD3D9Enumeration::GetAdapterInfo)
			.def("GetDeviceInfo", &CD3D9Enumeration::GetDeviceInfo)
			.def("GetDeviceSettingsCombo", (CD3D9EnumDeviceSettingsCombo *(CD3D9Enumeration::*)(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL))&CD3D9Enumeration::GetDeviceSettingsCombo)

		, class_<my::DxutApp, CD3D9Enumeration>("DxutApp")
			.scope
			[
				def("DXUTD3DDeviceTypeToString", &my::DxutApp::DXUTD3DDeviceTypeToString),
				def("DXUTD3DFormatToString", &my::DxutApp::DXUTD3DFormatToString),
				def("DXUTMultisampleTypeToString", &my::DxutApp::DXUTMultisampleTypeToString),
				def("DXUTVertexProcessingTypeToString", &my::DxutApp::DXUTVertexProcessingTypeToString)
			]
			.def("GetD3D9BackBufferSurfaceDesc", &my::DxutApp::GetD3D9BackBufferSurfaceDesc)
			.def("GetD3D9DeviceSettings", &my::DxutApp::GetD3D9DeviceSettings)
			.def("ToggleFullScreen", &my::DxutApp::ToggleFullScreen)
			.def("ToggleREF", &my::DxutApp::ToggleREF)
			.property("SoftwareVP", &my::DxutApp::GetSoftwareVP, &my::DxutApp::SetSoftwareVP)
			.property("HardwareVP", &my::DxutApp::GetHardwareVP, &my::DxutApp::SetHardwareVP)
			.property("PureHardwareVP", &my::DxutApp::GetPureHardwareVP, &my::DxutApp::SetPureHardwareVP)
			.property("MixedVP", &my::DxutApp::GetMixedVP, &my::DxutApp::SetMixedVP)
			.def("ChangeDevice", &my::DxutApp::ChangeDevice)

		, class_<my::Timer, boost::shared_ptr<my::Timer> >("Timer")
			.def(constructor<float, float>())
			.def_readonly("Interval", &my::Timer::m_Interval)
			.def_readonly("RemainingTime", &my::Timer::m_RemainingTime)
			.def_readwrite("EventTimer", &my::Timer::m_EventTimer)
	];
}
