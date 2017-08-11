#include "CCTouchGuard.h"

NS_CC_EXT_BEGIN

TouchGuard::TouchGuard()
: _eventListener(nullptr)
{}

TouchGuard::~TouchGuard(){
}

void TouchGuard::onEnter(){
    Node::onEnter();
    
    _eventListener = cocos2d::EventListenerTouchOneByOne::create();
    _eventListener->setSwallowTouches(true);
    _eventListener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event*){
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_eventListener, this);
}

void TouchGuard::onExit(){
    Node::onExit();
    
    _eventDispatcher->removeEventListener(_eventListener);
    _eventDispatcher = nullptr;
}

NS_CC_EXT_END
