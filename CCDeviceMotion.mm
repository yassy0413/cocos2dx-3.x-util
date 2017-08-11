/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceMotion.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import <AVFoundation/AVFoundation.h>
#import <CoreMotion/CoreMotion.h>

@interface CCDeviceMotion : NSObject
@property (assign, nonatomic) cocos2d::Quaternion quat;
@property (retain, nonatomic) CMMotionManager* motionManager;
@end

@implementation CCDeviceMotion

-(id) init {
    if( self = [super init] ){
        [self startMotionWithReferenceFrame:CMAttitudeReferenceFrameXTrueNorthZVertical];
    }
    return self;
}

- (void)dealloc {
    [self stopMotion];
    [super dealloc];
}

-(void) startMotionWithReferenceFrame:(CMAttitudeReferenceFrame)frame {
    if( !_motionManager ){
        _motionManager = [[CMMotionManager alloc] init];
    }
    if( _motionManager.deviceMotionAvailable ){
        _motionManager.showsDeviceMovementDisplay = YES;
        _motionManager.deviceMotionUpdateInterval = cocos2d::Director::getInstance()->getAnimationInterval();
        [_motionManager startDeviceMotionUpdatesUsingReferenceFrame:frame];
    }
}

-(void) stopMotion {
    if( _motionManager ){
        [_motionManager stopDeviceMotionUpdates];
        [_motionManager release];
        _motionManager = nil;
    }
}

@end
#endif

NS_CC_BEGIN

DeviceMotion::DeviceMotion(){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    _internal = [[CCDeviceMotion alloc] init];
    cocos2d::Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
    update(0);
#endif
}

DeviceMotion::~DeviceMotion(){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    cocos2d::Director::getInstance()->getScheduler()->unscheduleUpdate(this);
    CCDeviceMotion* internal = (CCDeviceMotion*)_internal;
    [internal release];
#endif
}

bool DeviceMotion::init(){
    return true;
}

void DeviceMotion::update(float delta){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    CCDeviceMotion* internal = (CCDeviceMotion*)_internal;
    if( internal.motionManager ){
        const CMQuaternion& q(internal.motionManager.deviceMotion.attitude.quaternion);
        _quat.set(q.x, q.y, q.z, q.w);
    }
#endif
}

NS_CC_END