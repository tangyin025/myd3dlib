#include "stdafx.h"
#include "mySingleton.h"
#include "myDxutApp.h"

using namespace my;

DeviceResourceBase::DeviceResourceBase(void)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	D3DContext::getSingleton().m_EventDeviceReset.connect(boost::bind(&DeviceResourceBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.connect(boost::bind(&DeviceResourceBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.connect(boost::bind(&DeviceResourceBase::OnDestroyDevice, this));
}

DeviceResourceBase::~DeviceResourceBase(void)
{
	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	D3DContext::getSingleton().m_EventDeviceReset.disconnect(boost::bind(&DeviceResourceBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.disconnect(boost::bind(&DeviceResourceBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.disconnect(boost::bind(&DeviceResourceBase::OnDestroyDevice, this));
}
