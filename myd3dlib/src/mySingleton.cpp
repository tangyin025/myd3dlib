#include "mySingleton.h"
#include "myDxutApp.h"
#include "myResource.h"

using namespace my;

DeviceResourceBase::DeviceResourceBase(void)
{
	// ! boost signals is thread-safety, ref: https://www.boost.org/doc/libs/1_63_0/doc/html/signals2/thread-safety.html
	D3DContext::getSingleton().m_EventDeviceReset.connect(boost::bind(&DeviceResourceBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.connect(boost::bind(&DeviceResourceBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.connect(boost::bind(&DeviceResourceBase::OnDestroyDevice, this));
}

DeviceResourceBase::~DeviceResourceBase(void)
{
	D3DContext::getSingleton().m_EventDeviceReset.disconnect(boost::bind(&DeviceResourceBase::OnResetDevice, this));

	D3DContext::getSingleton().m_EventDeviceLost.disconnect(boost::bind(&DeviceResourceBase::OnLostDevice, this));

	D3DContext::getSingleton().m_EventDeviceDestroy.disconnect(boost::bind(&DeviceResourceBase::OnDestroyDevice, this));
}

IResourceCallback::~IResourceCallback(void)
{
	_ASSERT(!ResourceMgr::getSingleton().FindIORequestCallback(this));
}
