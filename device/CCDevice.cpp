/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDevice.h"

#pragma mark -- Common Methods

NS_CC_EXT_BEGIN

void Device::dump(){
    CCLOG("BundleVersion %s", getBundleVersion().c_str());
    CCLOG("BundleShortVersion %s", getBundleShortVersion().c_str());
    CCLOG("SystemVersion %s", getSystemVersion().c_str());
    CCLOG("SystemName %s", getSystemName().c_str());
    CCLOG("Orientation %s", isPortrait()?"PORTRAIT":"LANDSCAPE");
    CCLOG("DiskFree %f MB", getDiskFreeByates()/1024.0/1024);
}

NS_CC_EXT_END


#pragma mark -- Depend on platform

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <platform/android/jni/JniHelper.h>
static const char* CLASS_NAME = "com/yassyapp/Device";

NS_CC_EXT_BEGIN

static std::string callStaticStringMethod(const char* methodName){
    std::string result;
    
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, methodName, "()Ljava/lang/String;") ){
        jstring ret = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
        const char* str = t.env->GetStringUTFChars(ret, nullptr);
        result = str;
        t.env->ReleaseStringUTFChars(ret, str);
        t.env->DeleteLocalRef(t.classID);
    }
    
    return result;
}

std::string Device::getBundleVersion(){
    int64_t result = 0;
    
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getBundleVersion", "()J") ){
        result = t.env->CallStaticLongMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
    
    return cocos2d::StringUtils::toString(result);
}

std::string Device::getBundleShortVersion(){
    return callStaticStringMethod("getBundleShortVersion");
}

std::string Device::getSystemVersion(){
    return callStaticStringMethod("getSystemVersion");
}

std::string Device::getSystemName(){
    return callStaticStringMethod("getSystemName");
}

bool Device::isPortrait(){
    bool result = true;
    
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "isPortrait", "()Z") ){
        result = t.env->CallStaticBooleanMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
    
    return result;
}

uint64_t Device::getDiskFreeByates(){
    uint64_t result = 0;
    
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getDiskFreeByates", "()J") ){
        result = t.env->CallStaticLongMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
    
    return result;
}

void Device::clearUserDefaults(){
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "clearUserDefaults", "()Z") ){
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}

NS_CC_EXT_END
#endif
