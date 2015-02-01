#include "CCBNode.h"

NS_CC_EXT_BEGIN

CCBNode* CCBNode::create(cocosbuilder::CCBReader * ccbReader){
	CCBNode* pRet = new CCBNode();
	if( pRet && pRet->init() ){
		pRet->autorelease();
		pRet->_animationManager = ccbReader->getAnimationManager();
		return pRet;
	}else{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

CCBNode::CCBNode()
: _animationManager(nullptr)
, _lastCompletedAnimationSequenceNamed("")
, _runningAnimationName("")
{}

CCBNode::~CCBNode()
{}

bool CCBNode::onAssignCCBMemberVariable(Ref* target, const char* memberVariableName, Node* node){
	if( target == this ){
		CCLOG("CCBNode: add variable member [%s]", memberVariableName);
		CC_ASSERT( _variableNodes.find(memberVariableName) == _variableNodes.end() ); // duplicated
		_variableNodes.insert( memberVariableName, node );
		return true;
	}
	return false;
}

bool CCBNode::onAssignCCBCustomProperty(Ref* target, const char* memberVariableName, const Value& value){
	return false;
}

SEL_MenuHandler CCBNode::onResolveCCBCCMenuItemSelector(Ref * pTarget, const char* pSelectorName){
	return nullptr;
}

Control::Handler CCBNode::onResolveCCBCCControlSelector(Ref * pTarget, const char* pSelectorName){
	return nullptr;
}

SEL_CallFuncN CCBNode::onResolveCCBCCCallFuncSelector(Ref * pTarget, const char* pSelectorName){
	return nullptr;
}

void CCBNode::completedAnimationSequenceNamed(const char *name){
	_lastCompletedAnimationSequenceNamed = name;
	
	if( _reservedAnimationNames.empty() ){
		_runningAnimationName = "";
	}else{
		_runningAnimationName = _reservedAnimationNames.front();
		_reservedAnimationNames.pop();
		runAnimation(_runningAnimationName);
	}
}

void CCBNode::runAnimation(const char* pName, float fTweenDuration){
	CC_ASSERT(pName);
	_runningAnimationName = pName;
	CC_ASSERT(_animationManager);
	_animationManager->runAnimationsForSequenceNamedTweenDuration(pName, fTweenDuration);
}

void CCBNode::reserveAnimation(const char* pName){
	if( !isRunningAnimation() ){
		runAnimation(pName);
	}else{
		_reservedAnimationNames.push(pName);
	}
}

NS_CC_EXT_END
