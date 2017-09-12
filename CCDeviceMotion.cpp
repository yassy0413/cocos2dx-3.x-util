/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceMotion.h"

#pragma mark -- Common Methods

NS_CC_EXT_BEGIN

static DeviceMotion *s_Instance = nullptr;

DeviceMotion* DeviceMotion::getInstance(){
    if (s_Instance == nullptr) {
        s_Instance = new (std::nothrow) DeviceMotion();
    }
    return s_Instance;
}

void DeviceMotion::destroyInstance(){
    CC_SAFE_DELETE(s_Instance);
}

NS_CC_EXT_END


#pragma mark -- Depend on platform

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <platform/android/jni/JniHelper.h>
static const char* CLASS_NAME = "com/yassyapp/DeviceMotion";

extern "C" {
    JNIEXPORT void JNICALL Java_com_yassyapp_DeviceMotion_updateRotationQuat(JNIEnv* env, jclass, jfloatArray quat){
        jfloat* v = env->GetFloatArrayElements(quat, 0);
        // @see https://goo.gl/tZJvqh
        cocos2d::extension::DeviceMotion::getInstance()->setQuat(cocos2d::Quaternion(v[1], v[2], v[3], v[0]));
        env->ReleaseFloatArrayElements(quat, v, JNI_ABORT);
    }
}


NS_CC_EXT_BEGIN

DeviceMotion::DeviceMotion(){
}

DeviceMotion::~DeviceMotion(){
    stop();
}

void DeviceMotion::update(float delta){
}

void DeviceMotion::start(){
//    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "start");
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "start", "()V") ){
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}

void DeviceMotion::stop(){
    cocos2d::JniMethodInfo t;
    if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "stop", "()V") ){
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}

Quaternion DeviceMotion::getQuat() const {
    return _quat;
}

void DeviceMotion::setQuat(const Quaternion& q){
    _quat = q;
}

NS_CC_EXT_END
#endif
