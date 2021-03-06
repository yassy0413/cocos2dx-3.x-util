/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCUNIXTIME_H__
#define __CCUNIXTIME_H__

#include <cocos2d.h>
#include <chrono>
#include <memory>
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

/**
 * For credible UNIX Time (Seconds)
 * 端末の時間操作による影響を避け、安定した時間判定を援助する
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
     * 現在の西暦をUTCで取得する
     */
    std::unique_ptr<tm> getGmTime() const;
    
    /**
     * 指定秒数後の西暦をUTCで取得する
     */
    std::unique_ptr<tm> getGmTimeAfter(int64_t seconds) const;
    
    /**
     * 現在の西暦をLocalTimezoneで取得する
     */
    std::unique_ptr<tm> getLocalTime() const;
    
    /**
     * 指定秒数後の西暦をLocalTimezoneで取得する
     */
    std::unique_ptr<tm> getLocalTimeAfter(int64_t seconds) const;
    
private:
    UnixTime();
    ~UnixTime();
    
    std::chrono::steady_clock::time_point _capturedTimePoint;
    int64_t _capturedUnixTime;
};

#if COCOS2D_DEBUG > 0
/**
 * サーバから時間を取得して初期化するサンプル。
 * @warning For only test.
 */
void setupUnixTimeSample(std::function<void(UnixTime* sender)> callback = nullptr);
#endif

NS_CC_EXT_END
#endif
