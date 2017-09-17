/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCDevice_H__
#define __CCDevice_H__

#include <cocos2d.h>
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

/**
 * 端末操作
 */
class Device final
{
public:
    static void dump();
    
    static std::string getBundleVersion();
    static std::string getBundleShortVersion();
    static std::string getSystemVersion();
    static std::string getSystemName();
    
    static bool isPortrait();
    
    static uint64_t getDiskFreeBytes();
    
    static void clearUserDefaults();
    
public:
    inline static void setKeepScreenOn(bool keepScreenOn){
        cocos2d::Device::setKeepScreenOn(keepScreenOn);
    }
    
    inline static void vibrate(float duration){
        cocos2d::Device::vibrate(duration);
    }
};

NS_CC_EXT_END

#endif
