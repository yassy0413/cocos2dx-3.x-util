/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCBNODE_H__
#define __CCBNODE_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include "cocosbuilder/CocosBuilder.h"
#include <queue>

NS_CC_EXT_BEGIN

/*
 * 汎用CCBNodeアクセッサ
 */
class CCBNode
: public Node
, public cocosbuilder::CCBMemberVariableAssigner
, public cocosbuilder::CCBSelectorResolver
, public cocosbuilder::CCBAnimationManagerDelegate
, public cocosbuilder::NodeLoaderListener
{
public:
    CCBNode();
    virtual ~CCBNode();
    
    /*
     * Custom CREATE_FUNC
     */
    static CCBNode* create(cocosbuilder::CCBReader* ccbReader);
    
    /**
     * CCBReaderから作成されたインスタンスを作成する
     */
    static CCBNode* createFromFile(const char* ccbiFileName);
    
    /**
     * CCBのデフォルトアニメーション名称
     */
    static const char* DEFAULT_ANIMATION_NAME;
    
    /**
     * 初期化完了時のコールバック
     */
    typedef std::function<void()> nodeLoaderCompleteCallback;
    nodeLoaderCompleteCallback onNodeLoaderCompleteCallback;
    
public:
    
    // cocosbuilder::CCBMemberVariableAssigner
    virtual bool onAssignCCBMemberVariable(Ref* target, const char* memberVariableName, Node* node) override;
    virtual bool onAssignCCBCustomProperty(Ref* target, const char* memberVariableName, const Value& value) override;
    
    // cocosbuilder::CCBSelectorResolver
    virtual SEL_MenuHandler onResolveCCBCCMenuItemSelector(Ref * pTarget, const char* pSelectorName) override;
    virtual SEL_CallFuncN onResolveCCBCCCallFuncSelector(Ref * pTarget, const char* pSelectorName) override;
    virtual Control::Handler onResolveCCBCCControlSelector(Ref * pTarget, const char* pSelectorName) override;
    
    // cocosbuilder::CCBAnimationManagerDelegate
    virtual void completedAnimationSequenceNamed(const char *name) override;
    
    // cocosbuilder::NodeLoaderListener
    virtual void onNodeLoaded(cocos2d::Node * pNode, cocosbuilder::NodeLoader * pNodeLoader) override;
    
public:
    
    /**
     * ノードを検索
     */
    inline Node* getVariable(const char* name) const noexcept {
        return _variableNodes.at(name);
    }
    
    /**
     * ノードを指定の型として検索
     */
    template <class T>
    inline T* getVariableAs(const char* name) const noexcept {
        CC_ASSERT( dynamic_cast<T*>( _variableNodes.at(name) ) );
        return reinterpret_cast<T*>( _variableNodes.at(name) );
    }
    
    /**
     * ノードを指定の型として検索。対象の型が違うか、ノードが見つからない場合はnullを返す
     */
    template <class T>
    inline T* getSafeVariableAs(const char* name) const noexcept {
        auto it = _variableNodes.find(name);
        if( it != _variableNodes.end() ){
            return dynamic_cast<T*>( it->second );
        }
        return nullptr;
    }
    
public:
    
    /**
     * アニメーションの取得
     */
    inline cocosbuilder::CCBAnimationManager* getAnimationManager() const {
        return _animationManager;
    }
    
    /**
     * アニメーションの再生
     */
    void runAnimation(const char* pName, float fTweenDuration = 0.0f);
    
    /**
     * アニメーションの再生予約
     @code
     runAnimation("TimeLine01");
     reserveAnimation("TimeLine02");
     reserveAnimation("TimeLine03");
     @endcode
     */
    void reserveAnimation(const char* pName);
    
    /**
     * アニメーションが再生中か確認
     */
    inline bool isRunningAnimation() const { return *_runningAnimationName != '\0'; }
    
    /**
     * 再生中のアニメーション名を取得
     */
    inline const char* getRunningAnimationName() const { return _runningAnimationName; }
    
    /**
     * 最後に再生が完了したアニメーション名を取得
     */
    inline const char* getLastCompletedAnimationSequenceNamed() const { return _lastCompletedAnimationSequenceNamed; }
    
    /**
     * 最後に再生したアニメーション名を取得
     */
    inline const char* getLastRunAnimationName() const {
        return isRunningAnimation()? _runningAnimationName : _lastCompletedAnimationSequenceNamed ;
    }
    
    /**
     * 再生後自動破棄設定
     */
    void autoReleaseWithAnimation();
    
    /**
     * アニメーション終了時のコールバック
     */
    typedef std::function<void(const char*)> ccbAnimationCompleteCallback;
    ccbAnimationCompleteCallback onAnimationCompleteCallback;
    
private:
    Map<std::string, Node*> _variableNodes;
    cocosbuilder::CCBAnimationManager* _animationManager;
    const char* _lastCompletedAnimationSequenceNamed;
    const char* _runningAnimationName;
    std::queue<const char*> _reservedAnimationNames;
    bool _autoReleaseWithAnimation;
    
    /**
     * アニメーション再生終了コールバックの受け取り代理
     */
    class AnimationCallbackProxy
    : public Ref
    , public cocosbuilder::CCBAnimationManagerDelegate
    {
    public:
        virtual void completedAnimationSequenceNamed(const char *name) override;
        cocosbuilder::CCBAnimationManagerDelegate* _delegate;
    };
    AnimationCallbackProxy _animationCallbackProxy;
};


class CCBNodeLoader
: public cocosbuilder::NodeLoader
{
public:
    virtual ~CCBNodeLoader() {};
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(CCBNodeLoader, loader);
protected:
    /**
     * Custom CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD
     */
    virtual CCBNode* createNode(cocos2d::Node * pParent, cocosbuilder::CCBReader * ccbReader) { return CCBNode::create(ccbReader); }
};

NS_CC_EXT_END

#endif