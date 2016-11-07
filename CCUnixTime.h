/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCUNIXTIME_H__
#define __CCUNIXTIME_H__

#include "ExtensionMacros.h"
#include <chrono>
#include <memory>

NS_CC_EXT_BEGIN

/**
 * UNIX Time (Seconds)
 */
class UnixTime final
: public Ref
{
public:
    CC_DISALLOW_COPY_AND_ASSIGN(UnixTime);
    
    /**
     * Returns a shared instance of the UNIXTime.
     */
    static UnixTime* getInstance();
    
    /**
     * 初期化
     * @param unixTime (since epoch).
     *   端末時間の操作を避ける為、サーバーから取得したデータを推奨
     */
    void setUnixTime(int64_t unixTime);
    
    /**
     * UNIX時間の取得
     */
    int64_t getUnixTime() const;
    
    /**
     * 指定のUNIX時間までの残り秒数を取得
     */
    int64_t getRemainingSecondsFrom(int64_t unixTime) const;
    
    /**
     * 現在の西暦を取得する
     */
    const tm& getLocalTime() const;
    
private:
    UnixTime();
    ~UnixTime();
    
    mutable std::unique_ptr<tm> _tm;
    std::chrono::steady_clock::time_point _capturedTimePoint;
    int64_t _capturedUnixTime;
};

#if COCOS2D_DEBUG > 0
/**
 * サーバから時間を取得して初期化するサンプル。
 * @warning For only test.
 */
void SetupUnixTimeSample(std::function<void(UnixTime* sender)> callback = nullptr);
#endif

NS_CC_EXT_END
#endif
