/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceMotion.h"

// TODO: Implement Android
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

NS_CC_BEGIN

DeviceMotion::DeviceMotion(){
}

DeviceMotion::~DeviceMotion(){
}

bool DeviceMotion::init(){
    return true;
}

void DeviceMotion::update(float delta){
}

NS_CC_END
#endif