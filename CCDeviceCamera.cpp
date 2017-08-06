/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceCamera.h"

// TODO: Implement Android
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

NS_CC_BEGIN

DeviceCameraSprite* DeviceCameraSprite::create(CaptureDevicePosition pos, Quality quality){
    auto pRet = new (std::nothrow) DeviceCameraSprite();
    if( pRet && pRet->init(pos, quality) ){
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

DeviceCameraSprite::DeviceCameraSprite()
: _internal(nullptr)
{}

DeviceCameraSprite::~DeviceCameraSprite(){
}

bool DeviceCameraSprite::init(CaptureDevicePosition pos, Quality quality){
    if( Sprite::init() ){
        return true;
    }
    return false;
}

void DeviceCameraSprite::start(){
}

void DeviceCameraSprite::stop(){
}

void DeviceCameraSprite::onEnter(){
    Sprite::onEnter();
    scheduleUpdate();
}

void DeviceCameraSprite::onExit(){
    Sprite::onExit();
    unscheduleUpdate();
}

void DeviceCameraSprite::update(float delta){
}

NS_CC_END
#endif
