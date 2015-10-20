/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCBBUILDER_H__
#define __CCBBUILDER_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include "cocosbuilder/CocosBuilder.h"

NS_CC_EXT_BEGIN

/*
 *
 */
class CCBBuilder
{
public:
    
    /**
     *
     */
    template < class T >
    CCBBuilder& registerNodeLoader( const char* pClassName )
    {
        cocosbuilder::NodeLoaderLibrary::getInstance()->registerNodeLoader( pClassName, T::loader() );
        return *this;
    }
    
    /**
     *
     */
    inline Node* buildNode( const char* pCCBFileName )
    {
        auto reader = new cocosbuilder::CCBReader( cocosbuilder::NodeLoaderLibrary::getInstance() );
        Node* p = reader->readNodeGraphFromFile( pCCBFileName );
        reader->release();
        return p;
    }
    
    template < class T >
    static Node* buildNode( const char* pCCBFileName, const char* pClassName )
    {
        return CCBBuilder()
        .registerNodeLoader<T>( pClassName )
        .buildNode( pCCBFileName );
    }
    
    /**
     *
     */
    inline Scene* buildScene( const char* pCCBFileName )
    {
        auto scene = Scene::create();
        scene->addChild( buildNode( pCCBFileName ) );
        return scene;
    }
    
    template < class T >
    static Scene* buildScene( const char* pCCBFileName, const char* pClassName )
    {
        return CCBBuilder()
        .registerNodeLoader<T>( pClassName )
        .buildScene( pCCBFileName );
    }
};

NS_CC_EXT_END

#endif