/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCCAMERAVIEW_H__
#define __CCCAMERAVIEW_H__

#include <cocos2d.h>
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

/**
 * デバイスカメラの映像をGLテクスチャとして管理する
 * @required iOS CoreMedia.framework
 * @required Android <uses-permission android:name="android.permission.CAMERA"/>
 */
class DeviceCamera
{
public:
    CC_DISALLOW_COPY_AND_ASSIGN(DeviceCamera);
    
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
    
    /** Return the shared instance **/
    static DeviceCamera *getInstance();
    
    /** Relase the shared instance **/
    static void destroyInstance();
    
    /// 撮影を開始
    void start(CaptureDevicePosition pos = CaptureDevicePosition::Default,
               Quality quality = Quality::Medium);
    
    /// 撮影を終了
    void stop();
    
    /// 撮影中の映像をテクスチャとして取得する
    Texture2D* getTexture();
    
    /// 撮影映像をテクスチャとして持つスプライトを生成する
    Sprite* createSprite(const std::function<void(Sprite* sender)>& onCreated = nullptr);
    
    
    virtual void update(float delta);
    void applyImage(const void* data, int32_t width, int32_t height);
    void* getInternal(){ return _internal; }
    
private:
    DeviceCamera();
    virtual ~DeviceCamera();
    
    void setPosition(CaptureDevicePosition pos);
    void clearSprites();
    
    void* _internal;
    experimental::RenderTarget* _renderTarget;
    CaptureDevicePosition _position;
    std::vector<std::pair<Sprite*, std::function<void(Sprite* sender)>>> _sprites;
};

NS_CC_EXT_END

#endif
