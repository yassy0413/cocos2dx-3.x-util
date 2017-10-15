/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceCamera.h"
#include "CCDevice.h"

#pragma mark -- Common Methods

NS_CC_EXT_BEGIN

static DeviceCamera *s_Instance = nullptr;

DeviceCamera* DeviceCamera::getInstance(){
    if (s_Instance == nullptr) {
        s_Instance = new (std::nothrow) DeviceCamera();
    }
    return s_Instance;
}

void DeviceCamera::destroyInstance(){
    CC_SAFE_DELETE(s_Instance);
}

DeviceCamera::DeviceCamera()
: _internal(nullptr)
, _renderTarget(nullptr)
{}

DeviceCamera::~DeviceCamera(){
    stop();
    clearSprites();
}

void DeviceCamera::setPosition(CaptureDevicePosition pos){
    if( pos == CaptureDevicePosition::Default ){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
        _position = CaptureDevicePosition::Front;
#else
        _position = CaptureDevicePosition::Back;
#endif
    }else{
        _position = pos;
    }
}

Texture2D* DeviceCamera::getTexture(){
    return _renderTarget? _renderTarget->getTexture() : nullptr ;
}

Sprite* DeviceCamera::createSprite(const std::function<void(Sprite* sender)>& onCreated){
    Sprite* p;
    if( _renderTarget ){
        p = Sprite::createWithTexture(_renderTarget->getTexture());
        applySprite(p);
        if( onCreated ){
            onCreated(p);
        }
    }else{
        if( !_internal ){
            start();
        }
        p = Sprite::create();
        p->retain();
        _sprites.emplace_back(p, onCreated);
    }
    return p;
}

void DeviceCamera::applyImage(const void* data, int32_t width, int32_t height){
    if( !_renderTarget ){
        _renderTarget = experimental::RenderTarget::create(width, height, Texture2D::PixelFormat::RGBA8888);
        _renderTarget->retain();
        
        const double scaleFactor = 1.0 / Director::getInstance()->getContentScaleFactor();
        width *= scaleFactor;
        height *= scaleFactor;
        
        for( auto& p : _sprites ){
            if( p.first->getReferenceCount() > 1 ){
                p.first->setTexture(_renderTarget->getTexture());
                p.first->setTextureRect(Rect(0, 0, width, height));
                applySprite(p.first);
                if( p.second ){
                    p.second(p.first);
                }
            }
        }
        clearSprites();
    }
    _renderTarget->getTexture()->updateWithData(data, 0, 0, width, height);
}

void DeviceCamera::applySprite(Sprite* sprite){
    sprite->setBlendFunc(BlendFunc::DISABLE);
    sprite->setPosition(Director::getInstance()->getWinSize() * 0.5f);
    
    if( Device::isPortrait() ){
        sprite->setRotation(90);
        sprite->setScale(Director::getInstance()->getWinSize().height / sprite->getContentSize().height);
    }else{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        sprite->setRotation(180);
#endif
        sprite->setScale(Director::getInstance()->getWinSize().width / sprite->getContentSize().width);
    }
}

void DeviceCamera::clearSprites(){
    for( auto& p : _sprites ){
        p.first->release();
    }
    _sprites.clear();
}

NS_CC_EXT_END


#pragma mark -- Depend on platform

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <platform/android/jni/JniHelper.h>
static const char* CLASS_NAME = "com/yassyapp/DeviceCamera";

struct ImageBuffer final {
    
    ImageBuffer()
    : data(nullptr)
    , dirty(false)
    {}
    
    ~ImageBuffer(){
        std::lock_guard<std::mutex> _(mutex);
        free(data);
    }
    
    void updateFrameBuffer(const uint8_t* yuv, int width, int height){
        
        if( !data ){
            data = malloc(width * height * 4);
            this->width = width;
            this->height = height;
        }
        
        std::lock_guard<std::mutex> _(mutex);
        if( dirty )
            return;
        dirty = true;
        
        //
        uint32_t* rgba = reinterpret_cast<uint32_t*>(data);
        
        // YCbCr_420_SP(NV21) -> RGBA
        //@see https://issuetracker.google.com/issues/36905470
        const int frameSize = width * height;
        
        for( int j = 0, yp = 0; j < height; ++j ){
            int uvp = frameSize + (j >> 1) * width;
            int u = 0;
            int v = 0;
            for( int i = 0; i < width; ++i, ++yp ){
                int y = yuv[yp] - 16;
                if (y < 0) y = 0;
                if ((i & 1) == 0) {
                    v = yuv[uvp++] - 128;
                    u = yuv[uvp++] - 128;
                }
                
                const int y1192 = 1192 * y;
                int r = (y1192 + 1634 * v);
                int g = (y1192 - 833 * v - 400 * u);
                int b = (y1192 + 2066 * u);
                
                if (r < 0) r = 0; else if (r > 262143) r = 262143;
                if (g < 0) g = 0; else if (g > 262143) g = 262143;
                if (b < 0) b = 0; else if (b > 262143) b = 262143;
                
                rgba[yp] = 0xff000000 | ((b << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((r >> 10) & 0xff);
            }
        }
    }
    
    void* data;
    int32_t width;
    int32_t height;
    bool dirty;
    std::mutex mutex;
};

extern "C" {
    JNIEXPORT void JNICALL Java_com_yassyapp_DeviceCamera_updateFrameBuffer(JNIEnv* env, jclass, jbyteArray data, jint width, jint height){
        if( auto internal = reinterpret_cast<ImageBuffer*>(cocos2d::extension::DeviceCamera::getInstance()->getInternal()) ){
            jbyte* yuv = env->GetByteArrayElements(data, nullptr);
            internal->updateFrameBuffer(reinterpret_cast<uint8_t*>(yuv), width, height);
            env->ReleaseByteArrayElements(data, yuv, JNI_ABORT);
        }
    }
}

NS_CC_EXT_BEGIN

void DeviceCamera::start(CaptureDevicePosition pos, Quality quality){
    if( !_internal ){
        setPosition(pos);
        Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
        _internal = new (std::nothrow) ImageBuffer();
        
        float scale = 1.0f;
        if( quality == Quality::Medium ){
            scale = 0.5f;
        }else if( quality == Quality::Low ){
            scale = 0.25f;
        }
        const auto frameSize = Director::getInstance()->getWinSize() * Director::getInstance()->getContentScaleFactor();
        const int width = frameSize.width * scale;
        const int height = frameSize.height * scale;
        
        cocos2d::JniMethodInfo t;
        if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "start", "(IIZ)V") ){
            t.env->CallStaticVoidMethod(t.classID, t.methodID, width, height, _position == CaptureDevicePosition::Back);
            t.env->DeleteLocalRef(t.classID);
        }
    }
}

void DeviceCamera::stop(){
    if( ImageBuffer* internal = (ImageBuffer*)_internal ){
        cocos2d::JniMethodInfo t;
        if( cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "stop", "()V") ){
            t.env->CallStaticVoidMethod(t.classID, t.methodID);
            t.env->DeleteLocalRef(t.classID);
        }
        
        delete internal;
        _internal = nullptr;
        Director::getInstance()->getScheduler()->unscheduleUpdate(this);
    }
}

void DeviceCamera::update(float delta){
    ImageBuffer* internal = (ImageBuffer*)_internal;
    
    std::lock_guard<std::mutex> _(internal->mutex);
    if( internal->dirty ){
        internal->dirty = false;
        applyImage(internal->data, internal->width, internal->height);
    }
}

NS_CC_EXT_END
#endif
