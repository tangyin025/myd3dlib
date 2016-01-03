#include "stdafx.h"
#include "mySingleton.h"
#include "myDxutApp.h"

using namespace my;

DeviceRelatedObjectBase::DeviceRelatedObjectBase(void)
{
	D3DContext::getSingleton().m_EventDeviceReset.connect(boost::bind(&DeviceRelatedObjectBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.connect(boost::bind(&DeviceRelatedObjectBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.connect(boost::bind(&DeviceRelatedObjectBase::OnDestroyDevice, this));
}

DeviceRelatedObjectBase::~DeviceRelatedObjectBase(void)
{
	D3DContext::getSingleton().m_EventDeviceReset.disconnect(boost::bind(&DeviceRelatedObjectBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.disconnect(boost::bind(&DeviceRelatedObjectBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.disconnect(boost::bind(&DeviceRelatedObjectBase::OnDestroyDevice, this));
}
