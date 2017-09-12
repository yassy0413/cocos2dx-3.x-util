/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCDEVICEMOTION_H__
#define __CCDEVICEMOTION_H__

#include <cocos2d.h>
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

/**
 * デバイスセンサーによる回転値の取得
 */
class DeviceMotion
{
public:
    CC_DISALLOW_COPY_AND_ASSIGN(DeviceMotion);
    
    /** Return the shared instance **/
    static DeviceMotion *getInstance();
    
    /** Relase the shared instance **/
    static void destroyInstance();
    
    ///
    void start();
    
    ///
    void stop();
    
    /// 回転姿勢の取得
    Quaternion getQuat() const;
    
    /// 回転姿勢の設定
    void setQuat(const Quaternion& q);
    
    
    virtual void update(float delta);
    
CC_CONSTRUCTOR_ACCESS:
    
    DeviceMotion();
    virtual ~DeviceMotion();
    
private:
    Quaternion _quat;
};

NS_CC_EXT_END

#endif
