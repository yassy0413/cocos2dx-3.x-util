/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCTERM_H__
#define __CCTERM_H__

#include "CCUnixTime.h"

NS_CC_EXT_BEGIN

/**
 * 期間
 */
class Term
{
public:
    Term(){}
    
    // duration(時間単位)の定義
    typedef std::chrono::duration<double>                       Seconds;
    typedef std::chrono::duration<double, std::ratio<60>>       Minutes;
    typedef std::chrono::duration<double, std::ratio<3600>>     Hours;
    typedef std::chrono::duration<double, std::ratio<86400>>    Days;
    
    /**
     * 開始時間と終了時間を設定して初期化
     * @param start,end UNIX時間。UNIXエポックからの経過秒数
     */
    Term(time_t start, time_t end)
    : _start( std::chrono::system_clock::from_time_t(start) )
    , _end( std::chrono::system_clock::from_time_t(end) )
    {
    }
    
    Term(std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end)
    : _start(start)
    , _end(end)
    {}
    
    /**
     * 開始時間と終了時間を設定
     * @param start,end UNIX時間。UNIXエポックからの経過秒数
     */
    inline void set(time_t start, time_t end){
        _start = std::chrono::system_clock::from_time_t(start);
        _end = std::chrono::system_clock::from_time_t(end);
    }
    
    inline void set(std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end){
        _start = start;
        _end = end;
    }
    
    /**
     * 期間開始までの残り時間を取得
     */
    template <class DurationUnit>
    double getTimeToStart() const {
        const auto now = std::chrono::system_clock::from_time_t( UnixTime::getInstance()->getUnixTime() );
        return std::chrono::duration_cast<DurationUnit>( _start - now ).count();
    }
    
    inline double getSecondsToStart() const {
        return getTimeToStart<Term::Seconds>();
    }
    
    /**
     * 期間終了までの残り時間を取得
     */
    template <class DurationUnit>
    double getTimeToEnd() const {
        const auto now = std::chrono::system_clock::from_time_t( UnixTime::getInstance()->getUnixTime() );
        return std::chrono::duration_cast<DurationUnit>( _end - now ).count();
    }
    
    inline double getSecondsToEnd() const {
        return getTimeToEnd<Term::Seconds>();
    }
    
    /**
     * 現在が期間中か判定
     */
    bool isValid() const {
        return (getSecondsToStart() <= 0.0f) && (getSecondsToEnd() >= 0.0f);
    }
    
private:
    std::chrono::system_clock::time_point _start;
    std::chrono::system_clock::time_point _end;
};

NS_CC_EXT_END
#endif
