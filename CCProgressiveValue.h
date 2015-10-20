/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_PROGRESSIVE_VALUE_H__
#define __CC_PROGRESSIVE_VALUE_H__

#include "cocos2d.h"
#include "ExtensionMacros.h"
#include <codecvt>


NS_CC_EXT_BEGIN

/*
 * 進行値
 */
template <class T>
class ProgressiveNumber
: public Ref
{
public:
	typedef ProgressiveNumber<T> SelfType;
	typedef T ValueType;
	
	ProgressiveNumber(Scheduler* scheduler = nullptr)
	: onValueUpdated(nullptr)
	, _scheduler(scheduler)
	, _scheduling(false)
	{}
	
	virtual ~ProgressiveNumber(){
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
	 * 値更新コールバック
	 @code
	 onValueUpdated = [](cocos2d::Ref* sender){
	 	auto value = static_cast<ProgressiveNumber<int>*>(sender);
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
class ProgressiveText
: public ProgressiveNumber<int>
{
public:
	
	ProgressiveText(Scheduler* scheduler = nullptr)
	: ProgressiveNumber(scheduler)
	{}
	
	/**
	 * 文字列を設定し、１秒間あたりの指定速度で少しずつ増えていき完成するオブジェクトを作成
	 */
	static ProgressiveText* createTextWithSpeed(const std::string& text, float speedPerSecond, Scheduler* scheduler = nullptr){
		ProgressiveText *value = new (std::nothrow) ProgressiveText(scheduler);
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
	static ProgressiveText* createTextWithSeconds(const std::string& text, float seconds, Scheduler* scheduler = nullptr){
		ProgressiveText *value = new (std::nothrow) ProgressiveText(scheduler);
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
class ProgressiveTextW
: public ProgressiveNumber<int>
{
public:
	
	ProgressiveTextW(Scheduler* scheduler = nullptr)
	: ProgressiveNumber(scheduler)
	{}
	
	/**
	 * 文字列を設定し、１秒間あたりの指定速度で少しずつ増えていき完成するオブジェクトを作成
	 */
	static ProgressiveTextW* createTextWithSpeed(const std::string& text, float speedPerSecond, Scheduler* scheduler = nullptr){
		ProgressiveTextW *value = new (std::nothrow) ProgressiveTextW(scheduler);
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
	static ProgressiveTextW* createTextWithSeconds(const std::string& text, float seconds, Scheduler* scheduler = nullptr){
		ProgressiveTextW *value = new (std::nothrow) ProgressiveTextW(scheduler);
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
		_text = ConvType().from_bytes( text );
		setWithSpeed(0, _text.size(), speedPerSecond);
	}
	
	/**
	 * 文字列を設定し、指定秒数まで少しずつ増えていき完成する
	 */
	void setTextWithSeconds(const std::string& text, float seconds){
		_text = ConvType().from_bytes( text );
		setWithSeconds(0, _text.size(), seconds);
	}
	
	/**
	 * 現在の文字列を取得
	 */
	std::string getText() const {
		return ConvType().to_bytes( _text.substr(0, get()) );
	}
	
private:
	typedef std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ConvType;
	ConvType::wide_string _text;
};

NS_CC_EXT_END

#endif