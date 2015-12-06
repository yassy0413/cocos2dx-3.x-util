/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCControlEventListener.h"

NS_CC_EXT_BEGIN

ControlEventListener::ControlEventListener()
{}

ControlEventListener::~ControlEventListener()
{}

bool ControlEventListener::init(){
    return true;
}

void ControlEventListener::add(Control* target, ccControlEventCallback callback, Control::EventType controlEvents){
    CC_ASSERT(target != nullptr);
    auto it = _callback.find(target);
    CCASSERT(it == _callback.end(), "Entry duplicated.");
    _callback.emplace(target, callback);
    target->addTargetWithActionForControlEvents(this, cccontrol_selector(ControlEventListener::onControlEvent), controlEvents);
}

void ControlEventListener::remove(Control* target){
    CC_ASSERT(target != nullptr);
    auto it = _callback.find(target);
    if( it != _callback.end() ){
        _callback.erase(it);
    }
}

void ControlEventListener::onControlEvent(Ref* sender, Control::EventType controlEvent){
    auto it = _callback.find((Control*)sender);
    CCASSERT(it != _callback.end(), "Unknown control object.");
    if( it != _callback.end() ){
        it->second((Control*)sender, controlEvent);
    }
}

NS_CC_EXT_END
