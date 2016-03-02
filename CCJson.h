/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCJSON_H__
#define __CCJSON_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include <external/json/document.h>
#include <external/json/stringbuffer.h>


NS_CC_EXT_BEGIN

/**
 * JSON文字列とCocosオブジェクトとの相互互換
 */
class Json
: public Ref
{
public:
    
    /**
     * 文字列からJSONオブジェクトを生成する
     */
    static Json* createFromStr(const char* str);
    
    /**
     * ValueからJSONオブジェクトを生成する
     * @doc 文字列のコピーは行わない
     */
    static Json* createFromStrInsitu(const char* str);
    
    /**
     * ValueからJSONオブジェクトを生成する
     */
    static Json* createFromValue(const Value& value);
    
    /**
     * JSON形式の文字列を取得
     */
    const char* getString();
    
    /**
     * JSON形式の整形された文字列を取得
     */
    const char* getPrettyString(char indentChar = ' ', uint32_t indentCharCount = 2);
    
    /**
     * Valueを生成
     */
    Value getValue() const;
    
    
CC_CONSTRUCTOR_ACCESS:
    Json();
    virtual ~Json();
    
    virtual bool initFromValue(const Value& value);
    virtual bool initFromStr(const char* str, bool insitu);
    
private:
    rapidjson::Document* _document;
    rapidjson::StringBuffer* _stringBuffer;
};

NS_CC_EXT_END

#endif