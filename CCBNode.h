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
{
public:
	CCBNode();
	virtual ~CCBNode();
	
	/*
	 * Custom CREATE_FUNC
	 */
	static CCBNode* create(cocosbuilder::CCBReader * ccbReader);
	
	/**
	 * cocosbuilder::CCBMemberVariableAssigner
	 */
	virtual bool onAssignCCBMemberVariable(Ref* target, const char* memberVariableName, Node* node);
	virtual bool onAssignCCBCustomProperty(Ref* target, const char* memberVariableName, const Value& value);
	
	/**
	 * cocosbuilder::CCBSelectorResolver
	 */
	virtual SEL_MenuHandler onResolveCCBCCMenuItemSelector(Ref * pTarget, const char* pSelectorName);
	virtual SEL_CallFuncN onResolveCCBCCCallFuncSelector(Ref * pTarget, const char* pSelectorName);
	virtual Control::Handler onResolveCCBCCControlSelector(Ref * pTarget, const char* pSelectorName);
	
	/**
	 * cocosbuilder::CCBAnimationManagerDelegate
	 */
	virtual void completedAnimationSequenceNamed(const char *name);
	
public:
	
	/**
	 *
	 */
	inline Node* getVariable(const char* name) const {
		return _variableNodes.at(name);
	}
	
	/**
	 *
	 */
	template <class T>
	inline T* getVariableAs(const char* name) const {
		CC_ASSERT( dynamic_cast<T*>( _variableNodes.at(name) ) );
		return reinterpret_cast<T*>( _variableNodes.at(name) );
	}
	
	/**
	 *
	 */
	template <class T>
	inline T* getSafeVariableAs(const char* name) const {
		return dynamic_cast<T*>( _variableNodes.at(name) );
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
	 * @exsample
	 *	runAnimation("TimeLine01");
	 *	reserveAnimation("TimeLine02");
	 *	reserveAnimation("TimeLine03");
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
	
private:
	Map<std::string, Node*> _variableNodes;
	cocosbuilder::CCBAnimationManager* _animationManager;
	const char* _lastCompletedAnimationSequenceNamed;
	const char* _runningAnimationName;
	std::queue<const char*> _reservedAnimationNames;
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