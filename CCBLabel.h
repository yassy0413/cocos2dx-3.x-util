/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCBLABEL_H__
#define __CCBLABEL_H__

#include "cocos2d.h"
#include "editor-support/cocosbuilder/CCLabelTTFLoader.h"

NS_CC_EXT_BEGIN

/**
 * CocosBuilder直接編集用Label
 */
class CCBLabel
: public Label
, public cocosbuilder::NodeLoaderListener
{
public:
    CREATE_FUNC(CCBLabel);
    
    virtual void onEnter() override {
        Label::onEnter();
        
        // CCBでの配置は必ずSystemFontになるので、ttfファイル指定時はTTFConfigを設定する
        if( getSystemFontName().rfind(".ttf") != std::string::npos ){
            setTTFConfig(TTFConfig(getSystemFontName(), getSystemFontSize()));
        }
        
        if( _outlineSize > 0.0f ){
            enableOutline( _outlineColor, _outlineSize );
        }
    }
    
    virtual void onNodeLoaded(Node* pNode, cocosbuilder::NodeLoader* pNodeLoader) override {
        const auto& customProperties = pNodeLoader->getCustomProperties();
        
        // set outline
        const auto outlineSize = customProperties.find("outlineSize");
        if( outlineSize != customProperties.end() ){
            _outlineSize = outlineSize->second.asFloat();
            
            const auto outlineColor = customProperties.find("outlineColor");
            if( outlineColor != customProperties.end() ){
                int r, g, b;
                sscanf(outlineColor->second.asString().c_str(), "%d,%d,%d", &r, &g, &b);
                _outlineColor.set(r, g, b, 255);
            }
        }
    }
    
CC_CONSTRUCTOR_ACCESS:
    
    CCBLabel()
    : _outlineSize(0.0f)
    , _outlineColor(cocos2d::Color4B::BLACK)
    {}
    
    virtual ~CCBLabel(){
    }
    
private:
    float _outlineSize;
    cocos2d::Color4B _outlineColor;
};

class CCBLabelLoader : public cocosbuilder::LabelTTFLoader {
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(CCBLabelLoader, loader);
private:
    CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(CCBLabel);
};

NS_CC_EXT_END

#endif
