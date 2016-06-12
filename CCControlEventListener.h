/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCControlEventListener_H__
#define __CCControlEventListener_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include <map>

NS_CC_EXT_BEGIN

/**
 * Convert ControlEvent to Callback Functor
 */
class ControlEventListener
: public Ref
{
public:
    CREATE_FUNC(ControlEventListener);
    ControlEventListener();
    virtual ~ControlEventListener();
    virtual bool init();
    
    typedef std::function<void(Control*, Control::EventType)> ccControlEventCallback;
    
    /**
     *
     */
    void add(Control* target, ccControlEventCallback callback, Control::EventType controlEvents = Control::EventType::TOUCH_UP_INSIDE);
    
    /**
     *
     */
    void remove(Control* target);
    
private:
    std::unordered_map<Control*, ccControlEventCallback> _callback;
    
    void onControlEvent(Ref* sender, Control::EventType controlEvent);
};

NS_CC_EXT_END

#endif