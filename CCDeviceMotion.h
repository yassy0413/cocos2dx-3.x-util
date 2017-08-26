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
: public Ref
{
public:
    CREATE_FUNC(DeviceMotion);
    
    /// 回転姿勢の取得
    Quaternion getQuat() const;
    
    virtual void update(float delta);
    
CC_CONSTRUCTOR_ACCESS:
    
    DeviceMotion();
    virtual ~DeviceMotion();
    
    virtual bool init();
    
private:
    void* _internal;
    Quaternion _quat;
};

NS_CC_EXT_END

#endif
