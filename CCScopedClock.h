/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_SCOPED_CLOCK_H__
#define __CC_SCOPED_CLOCK_H__

#include "cocos2d.h"
#include "cocos-ext.h"

#ifndef CC_ENABLE_SCOPED_CLODK
#define CC_ENABLE_SCOPED_CLODK (COCOS2D_DEBUG > 0)
#endif

#define CC_SCOPED_CLODK cocos2d::extension::ScopedClock _

NS_CC_EXT_BEGIN

/**
 * スコープ内の処理時間を計測する
 * @exsample cocos2d::extension::ScopedClock _(__PRETTY_FUNCTION__);
 */
class ScopedClock final
{
public:
    
    inline ScopedClock(const char* name = "", const char* description = "")
#if CC_ENABLE_SCOPED_CLODK
    : _lastTime( std::chrono::system_clock::now() )
    , _name( name )
    , _description(description)
#endif
    {}
    
    inline ~ScopedClock(){
#if CC_ENABLE_SCOPED_CLODK
        const auto elapsed = std::chrono::system_clock::now() - _lastTime;
        const double duration = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0;
        CCLOG( "%s: [%.5f sec] %s", _name.c_str(), duration, _description.c_str() );
#endif
    }
    
private:
#if CC_ENABLE_SCOPED_CLODK
    const std::chrono::system_clock::time_point _lastTime;
    const std::string _name;
    const std::string _description;
#endif
};

NS_CC_EXT_END
#endif
