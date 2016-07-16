/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_PROGRESS_VALUE_H__
#define __CC_PROGRESS_VALUE_H__

#include "cocos2d.h"


NS_CC_EXT_BEGIN

/**
 * 進行値
 */
template <class T>
class ProgressNumber
: public Ref
{
public:
    typedef ProgressNumber<T> SelfType;
    typedef T ValueType;
    
    ProgressNumber(Scheduler* scheduler = nullptr)
    : onValueUpdated(nullptr)
    , _scheduler(scheduler)
    , _scheduling(false)
    {}
    
    virtual ~ProgressNumber(){
        unscheduleUpdate();
    }
    
    /**
     * 進行値幅を設定し、１秒間あたりの指定速度で推移するオブジェクトを作成
     */
    static SelfType* createWithSpeed(T from, T to, float speed, Scheduler* scheduler = nullptr){
        SelfType *value = new (std::nothrow) SelfType(scheduler);
        if (value){
            value->autorelease();
            value->setWithSpeed(from, to, speed);
            return value;
        }
        return nullptr;
    }
    
    /**
     * 進行値幅を設定し、指定秒数で推移するオブジェクトを作成
     */
    static SelfType* createWithSeconds(T from, T to, float seconds, Scheduler* scheduler = nullptr){
        SelfType *value = new (std::nothrow) SelfType(scheduler);
        if (value){
            value->autorelease();
            value->setWithSeconds(from, to, seconds);
            return value;
        }
        return nullptr;
    }
    
    /**
     * 進行値幅を設定し、１秒間あたりの指定速度で推移する
     */
    void setWithSpeed(T from, T to, float speedPerSecond){
        _from = from;
        _to = to;
        _rate = 0.0f;
        _speed = speedPerSecond / (to - from);
        scheduleUpdate();
    }
    
    /**
     * 進行値幅を設定し、指定秒数で推移する
     */
    void setWithSeconds(T from, T to, float seconds){
        _from = from;
        _to = to;
        _rate = 0.0f;
        _speed = 1.0f / seconds;
        scheduleUpdate();
    }
    
    /**
     * 固定値を設定
     */
    void reset(T var){
        _from = var;
        _to = var;
        _rate = 1.0f;
        _speed = 0.0f;
        unscheduleUpdate();
    }
    
    /**
     * 値を進行させる
     */
    void update( float delta ){
        if( _rate < 1.0f ){
            _rate = std::min( 1.0f, _rate + _speed * delta );
            if( onValueUpdated ){
                onValueUpdated(this);
            }
        }else{
            unscheduleUpdate();
        }
    }
    
    /**
     * 強制終了
     */
    void finish(){
        reset( _to );
    }
    
    /**
     * 現在値を取得
     */
    T get() const {
        return T( _from + (_to - _from) * _rate );
    }
    
    /**
     * 目標値を取得
     */
    T getTargetValue() const {
        return _to;
    }
    
    /**
     * 進行中か判定
     */
    bool isBusy() const {
        return _rate < 1.0f;
    }
    
    /**
     * 進行率の取得
     */
    float getRate() const {
        return _rate;
    }
    
    /**
     * 値更新コールバック
     @code
     onValueUpdated = [](cocos2d::Ref* sender){
        auto value = static_cast<ProgressNumber<int>*>(sender);
        if(!value->isBusy()){ CCLOG("Finished!"); }
     };
     @endcode
     */
    typedef std::function<void(Ref* sender)> ccValueUpdated;
    ccValueUpdated onValueUpdated;
    
private:
    T _from;
    T _to;
    float _rate;
    float _speed;
    Scheduler* _scheduler;
    bool _scheduling;
    
    void scheduleUpdate(){
        if( _scheduler && !_scheduling ){
            _scheduler->scheduleUpdate(this, 0.0f, false);
            _scheduling = true;
            retain();
        }
    }
    void unscheduleUpdate(){
        if( _scheduler && _scheduling ){
            _scheduler->unscheduleUpdate(this);
            _scheduling = false;
            release();
        }
    }
};

/**
 * 進行文字列
 */
class ProgressText
: public ProgressNumber<int>
{
public:
    
    ProgressText(Scheduler* scheduler = nullptr)
    : ProgressNumber(scheduler)
    {}
    
    /**
     * 文字列を設定し、１秒間あたりの指定速度で少しずつ増えていき完成するオブジェクトを作成
     */
    static ProgressText* createTextWithSpeed(const std::string& text, float speedPerSecond, Scheduler* scheduler = nullptr){
        ProgressText *value = new (std::nothrow) ProgressText(scheduler);
        if (value){
            value->autorelease();
            value->setTextWithSpeed(text, speedPerSecond);
            return value;
        }
        return nullptr;
    }
    
    /**
     * 文字列を設定し、指定秒数まで少しずつ増えていき完成するオブジェクトを作成
     */
    static ProgressText* createTextWithSeconds(const std::string& text, float seconds, Scheduler* scheduler = nullptr){
        ProgressText *value = new (std::nothrow) ProgressText(scheduler);
        if (value){
            value->autorelease();
            value->setTextWithSeconds(text, seconds);
            return value;
        }
        return nullptr;
    }
    
    /**
     * 文字列を設定し、１秒間あたりの指定速度で少しずつ増えていき完成する
     */
    void setTextWithSpeed(const std::string& text, float speedPerSecond){
        _text = text;
        setWithSpeed(0, _text.size(), speedPerSecond);
    }
    
    /**
     * 文字列を設定し、指定秒数まで少しずつ増えていき完成する
     */
    void setTextWithSeconds(const std::string& text, float seconds){
        _text = text;
        setWithSeconds(0, _text.size(), seconds);
    }
    
    /**
     * 現在の文字列を取得
     */
    std::string getText() const {
        return _text.substr(0, get());
    }
    
private:
    std::string _text;
};

/**
 * 進行文字列(Wide)
 */
class ProgressTextW
: public ProgressNumber<int>
{
public:
    
    ProgressTextW(Scheduler* scheduler = nullptr)
    : ProgressNumber(scheduler)
    {}
    
    /**
     * 文字列を設定し、１秒間あたりの指定速度で少しずつ増えていき完成するオブジェクトを作成
     */
    static ProgressTextW* createTextWithSpeed(const std::string& text, float speedPerSecond, Scheduler* scheduler = nullptr){
        ProgressTextW *value = new (std::nothrow) ProgressTextW(scheduler);
        if (value){
            value->autorelease();
            value->setTextWithSpeed(text, speedPerSecond);
            return value;
        }
        return nullptr;
    }
    
    /**
     * 文字列を設定し、指定秒数まで少しずつ増えていき完成するオブジェクトを作成
     */
    static ProgressTextW* createTextWithSeconds(const std::string& text, float seconds, Scheduler* scheduler = nullptr){
        ProgressTextW *value = new (std::nothrow) ProgressTextW(scheduler);
        if (value){
            value->autorelease();
            value->setTextWithSeconds(text, seconds);
            return value;
        }
        return nullptr;
    }
    
    /**
     * 文字列を設定し、１秒間あたりの指定速度で少しずつ増えていき完成する
     */
    void setTextWithSpeed(const std::string& text, float speedPerSecond){
        cocos2d::StringUtils::UTF8ToUTF16(text, _text);
        setWithSpeed(0, _text.size(), speedPerSecond);
    }
    
    /**
     * 文字列を設定し、指定秒数まで少しずつ増えていき完成する
     */
    void setTextWithSeconds(const std::string& text, float seconds){
        cocos2d::StringUtils::UTF8ToUTF16(text, _text);
        setWithSeconds(0, _text.size(), seconds);
    }
    
    /**
     * 現在の文字列を取得
     */
    std::string getText() const {
        std::string result;
        cocos2d::StringUtils::UTF16ToUTF8(_text.substr(0, get()), result);
        return result;
    }
    
private:
    std::u16string _text;
};

NS_CC_EXT_END

#endif