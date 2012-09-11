#include "stdafx.h"
#include "mySingleton.h"
#include "myDxutApp.h"

using namespace my;

DeviceRelatedObjectBase::DeviceRelatedObjectBase(void)
{
	DxutApplication::getSingleton().RegisterDeviceRelatedObject(this);
}

DeviceRelatedObjectBase::~DeviceRelatedObjectBase(void)
{
	DxutApplication::getSingleton().UnregisterDeviceRelatedObject(this);
}
