/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceMotion.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import <AVFoundation/AVFoundation.h>
#import <CoreMotion/CoreMotion.h>
static CMMotionManager* motionManager = nil;
#endif

NS_CC_EXT_BEGIN

DeviceMotion::DeviceMotion()
{}

DeviceMotion::~DeviceMotion(){
    stop();
}

void DeviceMotion::update(float delta){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if( motionManager ){
        const CMQuaternion& q(motionManager.deviceMotion.attitude.quaternion);
        _quat.set(q.x, q.y, q.z, q.w);
    }
#endif
}

void DeviceMotion::start(){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if( !motionManager ){
        motionManager = [[CMMotionManager alloc] init];
        
        if( motionManager.deviceMotionAvailable ){
            motionManager.deviceMotionUpdateInterval = cocos2d::Director::getInstance()->getAnimationInterval();
            [motionManager startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryCorrectedZVertical];
            
            cocos2d::Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
            update(0);
        }else{
            [motionManager release];
            motionManager = nil;
        }
    }
#endif
}

void DeviceMotion::stop(){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if( motionManager ){
        [motionManager stopDeviceMotionUpdates];
        [motionManager release];
        motionManager = nil;
        
        cocos2d::Director::getInstance()->getScheduler()->unscheduleUpdate(this);
    }
#endif
}

Quaternion DeviceMotion::getQuat() const {
    // for portrait
    Quaternion q, q2;
    Quaternion::createFromAxisAngle(Vec3::UNIT_Z, M_PI, &q);
    Quaternion::createFromAxisAngle(Vec3::UNIT_X, M_PI_2, &q2);
    return q * q2 * _quat;
}

void DeviceMotion::setQuat(const Quaternion& q){
    _quat = q;
}


NS_CC_EXT_END
