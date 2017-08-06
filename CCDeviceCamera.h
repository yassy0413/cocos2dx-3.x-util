/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCCAMERAVIEW_H__
#define __CCCAMERAVIEW_H__

#include "cocos2d.h"

NS_CC_BEGIN

/**
 * デバイスカメラの映像をスプライトとして扱う
 * @required CoreMedia.framework
 */
class DeviceCameraSprite
: public Sprite
{
public:
    
    /// カメラの位置
    enum class CaptureDevicePosition {
        Front,
        Back,
        Default
    };
    
    /// 映像品質
    enum class Quality {
        Low,
        Medium,
        High
    };
    
    static DeviceCameraSprite* create(CaptureDevicePosition pos = CaptureDevicePosition::Default,
                                      Quality quality = Quality::Medium);
    
    void start();
    void stop();
    
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void update(float delta) override;
    
CC_CONSTRUCTOR_ACCESS:
    
    DeviceCameraSprite();
    virtual ~DeviceCameraSprite();
    
    virtual bool init(CaptureDevicePosition pos, Quality quality);
    
private:
    void* _internal;
    experimental::FrameBuffer* _frameBuffer;
};

NS_CC_END

#endif
