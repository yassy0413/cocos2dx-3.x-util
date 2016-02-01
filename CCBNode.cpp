/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCBNode.h"

NS_CC_EXT_BEGIN

const char* CCBNode::DEFAULT_ANIMATION_NAME = "Default Timeline";

CCBNode* CCBNode::create(cocosbuilder::CCBReader * ccbReader){
    CCBNode* pRet = new (std::nothrow) CCBNode();
    if( pRet && pRet->init() ){
        pRet->autorelease();
        pRet->_animationManager = ccbReader->getAnimationManager();
        return pRet;
    }
    delete pRet;
    return nullptr;
}


CCBNode* CCBNode::createFromFile(const char* ccbiFileName){
    CCASSERT( FileUtils::getInstance()->isFileExist(ccbiFileName), ccbiFileName );
    auto reader = new (std::nothrow) cocosbuilder::CCBReader( cocosbuilder::NodeLoaderLibrary::getInstance() );
    auto pRet = dynamic_cast<cocos2d::extension::CCBNode*>( reader->readNodeGraphFromFile(ccbiFileName) );
    CCASSERT( pRet, cocos2d::StringUtils::format("[%s] is not CCBNode", ccbiFileName).c_str() );
    reader->release();
    return pRet;
}

CCBNode::CCBNode()
: onNodeLoaderCompleteCallback(nullptr)
, onAnimationCompleteCallback(nullptr)
, _animationManager(nullptr)
, _lastCompletedAnimationSequenceNamed("")
, _runningAnimationName("")
, _autoReleaseWithAnimation(false)
{}

CCBNode::~CCBNode()
{}

bool CCBNode::onAssignCCBMemberVariable(Ref* target, const char* memberVariableName, Node* node){
    if( target == this ){
        //CCLOG("CCBNode: add variable member [%s]", memberVariableName);
        CCASSERT( _variableNodes.find(memberVariableName) == _variableNodes.end(), memberVariableName ); // duplicated
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
    
    if( onAnimationCompleteCallback ){
        onAnimationCompleteCallback( name );
    }
    
    if( _autoReleaseWithAnimation ){
        removeFromParent();
    }else{
        if( _reservedAnimationNames.empty() ){
            _runningAnimationName = "";
        }else{
            _runningAnimationName = _reservedAnimationNames.front();
            _reservedAnimationNames.pop();
            runAnimation(_runningAnimationName);
        }
    }
}

void CCBNode::onNodeLoaded(cocos2d::Node * pNode, cocosbuilder::NodeLoader * pNodeLoader){
    if( onNodeLoaderCompleteCallback ){
        onNodeLoaderCompleteCallback();
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
