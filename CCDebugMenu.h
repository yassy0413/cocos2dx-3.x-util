/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCDEBUGMENU_H__
#define __CCDEBUGMENU_H__

#include "cocos2d.h"
#include "cocos-ext.h"

#if COCOS2D_DEBUG > 0
NS_CC_EXT_BEGIN

/**
 * デバッグメニュー
 */
class DebugMenu
: public cocos2d::Ref
{
public:
    class Container;
    class Component;
    virtual void update(float delta);
    
    DebugMenu();
    virtual ~DebugMenu();
    virtual bool init();
    
    /**
     * Returns a shared instance of the director.
     * @js _getInstance
     */
    static DebugMenu* getInstance();
    
    /**
     * ロングタップでメニューを開くようにする
     * @param seconds 必要なホールド秒数
     */
    void enableLongTap(float seconds = 1.0f);
    
    /**
     * キー入力でメニューを開くようにする
     */
    void enableKeyboard(EventKeyboard::KeyCode keycode);
    
    /**
     * 加速度センサーでメニューを開くようにする
     * @param threshold 加速量の閾値
     */
    void enableAccele(float threshold = 4.0f);
    
    /**
     * デバッグメニュー表示を開始
     */
    virtual void open();
    
    /**
     * デバッグメニュー表示を終了
     */
    virtual void close();
    
    /**
     * Return a root container.
     */
    Container* getRootContainer() const { return _rootContainer; }
    
private:
    Container* _rootContainer;
    LayerColor* _background;
    float _thresholdAccele;
    float _thresholdLongTap;
    bool _touching;
    std::chrono::system_clock::time_point _touchBeganTime;
    cocos2d::Vec2 _touchBeganPoint;
    cocos2d::Vec2 _touchCurrentPoint;
    
public:
    
    /**
     * デバッグメニュー項目
     */
    class Component
    : public cocos2d::Node
    {
    public:
        friend class Container;
        class Preset;
        
        Component(const std::string& key);
        virtual ~Component();
        
        const std::string& getKey() const { return _key; }
        
        typedef std::function<void(Component* sender)> OnValueChenged;
        OnValueChenged onValueChenged;
        
    protected:
        Container* _container;
        std::string _key;
        
    private:
        virtual void setContainer(Container* container);
    };
    
    /**
     * 項目のリスト管理とイベント処理
     */
    class Container
    : public cocos2d::Node
    {
    public:
        Container();
        virtual ~Container();
        
        // cocos2d::Node
        virtual void onEnter() override;
        virtual void onExit() override;
        
        /// Add component.
        virtual void add(Component* target, Component::OnValueChenged onValueChanged = nullptr);
        
    private:
        cocos2d::extension::ScrollView* _scrollView;
        std::vector<Component*> _components;
        float _offsetY;
    };
    
};

/**
 * コンポーネントのプリセット
 */
class DebugMenu::Component::Preset
{
public:
    
    /**
     * スイッチ式のフラグ
     */
    class Flag
    : public Component
    {
    public:
        Flag(const std::string& key) : Component(key){}
    private:
        virtual void setContainer(Container* container) override;
        virtual void onControlEvent(cocos2d::Ref* sender, cocos2d::extension::Control::EventType controlEvent);
    };
    
    /**
     * ボタン
     */
    class Button
    : public Component
    {
    public:
        Button(const std::string& key) : Component(key){}
        virtual void onEnter() override;
    private:
        virtual void setContainer(Container* container) override;
        virtual void onControlEvent(cocos2d::Ref* sender, cocos2d::extension::Control::EventType controlEvent);
    };
    
    /**
     * スライダー
     */
    class Slider
    : public Component
    {
    public:
        Slider(const std::string& key, float min, float max)
        : Component(key)
        , _minValue(min)
        , _maxValue(max)
        {}
    private:
        virtual void setContainer(Container* container) override;
        virtual void onControlEvent(cocos2d::Ref* sender, cocos2d::extension::Control::EventType controlEvent);
        virtual void updateLabel();
    protected:
        Label* _label;
        float _minValue;
        float _maxValue;
    };
    
    class SliderI
    : public Slider
    {
    public:
        SliderI(const std::string& key, float min, float max)
        : Slider(key, min, max)
        {}
    private:
        virtual void updateLabel() override;
    };
};

NS_CC_EXT_END
#endif
#endif
