/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCAR.h"
#include "../device/CCDevice.h"

#pragma mark -- Common Methods

NS_CC_EXT_BEGIN

static AR *s_Instance = nullptr;

AR* AR::getInstance(){
    if (s_Instance == nullptr) {
        s_Instance = new (std::nothrow) AR();
    }
    return s_Instance;
}

void AR::destroyInstance(){
    CC_SAFE_DELETE(s_Instance);
}

AR::AR()
: _internal(nullptr)
, _renderTarget(nullptr)
{}

AR::~AR(){
    stop();
    clearSprites();
}

Texture2D* AR::getTexture(){
    return _renderTarget? _renderTarget->getTexture() : nullptr ;
}

Sprite* AR::createSprite(const std::function<void(Sprite* sender)>& onCreated){
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

void AR::applyImage(const void* data, int32_t width, int32_t height){
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

void AR::applySprite(Sprite* sprite){
    sprite->setBlendFunc(BlendFunc::DISABLE);
    sprite->setPosition(Director::getInstance()->getWinSize() * 0.5f);
    
    if( Device::isPortrait() ){
        sprite->setRotation(90);
        sprite->setScale(Director::getInstance()->getWinSize().height / sprite->getContentSize().width);
    }else{
        sprite->setScale(Director::getInstance()->getWinSize().height / sprite->getContentSize().height);
    }
}

void AR::clearSprites(){
    for( auto& p : _sprites ){
        p.first->release();
    }
    _sprites.clear();
}

NS_CC_EXT_END


#pragma mark -- Depend on platform

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

NS_CC_EXT_BEGIN

bool AR::isSupported() const {
    return false;
}

void AR::pause(){
    // Not implemented.
}

void AR::resume(){
    // Not implemented.
}

void AR::start(){
    // Not implemented.
}

void AR::stop(){
    // Not implemented.
}

void AR::update(float delta){
}

NS_CC_EXT_END
#endif
