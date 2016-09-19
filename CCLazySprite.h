/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_LAZY_SPRITE_H__
#define __CC_LAZY_SPRITE_H__

#include "cocos2d.h"
#include "cocos-ext.h"

NS_CC_BEGIN
namespace network {
    class HttpClient;
    class HttpResponse;
}
NS_CC_END

NS_CC_EXT_BEGIN

typedef std::function<void(Sprite* sprite)> ccLazySpriteCallback;

/**
 * <img>のように扱えるスプライト
 */
class LazySprite
: public Sprite
{
public:
    
    /**
     * urlで指定された画像ファイルをダウンロードし、テクスチャの非同期読み込みが完了したら自身へ適用するスプライトを生成
     * @return  An autoreleased sprite object.
     */
    static Sprite* createWithURL(const std::string& url, const ccLazySpriteCallback& callback = nullptr, const std::string& cachePath = "LazySprite/");
    
    /**
     * filenameで指定されたテクスチャの非同期読み込みが完了したら自身へ適用するスプライトを生成
     * @return  An autoreleased sprite object.
     */
    static Sprite* createAsync(const std::string& filename, const ccLazySpriteCallback& callback = nullptr);
    
CC_CONSTRUCTOR_ACCESS:
    
    LazySprite();
    virtual ~LazySprite();
    
    bool initWithURL(const std::string& url, const ccLazySpriteCallback& callback, const std::string& cachePath);
    bool initAsync(const std::string& filename, const ccLazySpriteCallback& callback);
    
private:
    static void requestDownload(const std::string& url, const std::string& cacheFilePath, const std::string& cacheFileDir, LazySprite* target);
    void addImageAsync(const std::string& filename);
    
    ccLazySpriteCallback _finishedCallback;
};

NS_CC_EXT_END

#endif
