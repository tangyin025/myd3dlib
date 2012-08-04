#include "stdafx.h"
#include "mySingleton.h"
#include "myDxutApp.h"

using namespace my;

DeviceRelatedObjectBase::DeviceRelatedObjectBase(void)
{
	DxutApp::getSingleton().RegisterDeviceRelatedObject(this);
}

DeviceRelatedObjectBase::~DeviceRelatedObjectBase(void)
{
	DxutApp::getSingleton().UnregisterDeviceRelatedObject(this);
}
