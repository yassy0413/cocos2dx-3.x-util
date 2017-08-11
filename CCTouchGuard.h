/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCTOUCHGUARD_H__
#define __CCTOUCHGUARD_H__

#include "cocos2d.h"
#include "cocos-ext.h"


NS_CC_EXT_BEGIN

/**
 * タッチの貫通させないノード
 */
class TouchGuard
: public Node
{
public:
    CREATE_FUNC(TouchGuard);
    
    virtual void onEnter() override;
    virtual void onExit() override;
    
CC_CONSTRUCTOR_ACCESS:
    TouchGuard();
    virtual ~TouchGuard();
    
private:
    EventListenerTouchOneByOne* _eventListener;
};


NS_CC_EXT_END

#endif
