/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCLazySprite.h"
#include "network/HttpClient.h"

NS_CC_EXT_BEGIN

typedef std::unordered_map<std::string, std::list<LazySprite*>> DownloadingTaskList;
static DownloadingTaskList* _downloadingTasks = nullptr;

void LazySprite::requestDownload(const std::string& url, LazySprite* target){
    
    bool needHttpRequest = true;
    
    if( _downloadingTasks ){
        auto it = _downloadingTasks->find(url);
        if( it != _downloadingTasks->end() ){
            it->second.push_back(target);
            needHttpRequest = false;
        }
    }else{
        _downloadingTasks = new (std::nothrow) DownloadingTaskList();
    }
    
    target->retain();
    
    if( needHttpRequest ){
        _downloadingTasks->emplace(url, std::list<LazySprite*>{target});
        
        network::HttpRequest* req = new network::HttpRequest();
        req->setRequestType( network::HttpRequest::Type::GET );
        req->setUrl( url.c_str() );
        req->setResponseCallback([](network::HttpClient* client, network::HttpResponse* response){
            
            std::list<LazySprite*> targets;
            if( _downloadingTasks ){
                auto it = _downloadingTasks->find( response->getHttpRequest()->getUrl() );
                if( it != _downloadingTasks->end() ){
                    targets = std::move(it->second);
                }
                _downloadingTasks->erase(it);
                
                if( _downloadingTasks->empty() ){
                    CC_SAFE_DELETE(_downloadingTasks);
                }
            }
            CC_ASSERT( !targets.empty() );
            
            const std::string& cacheFilePath = targets.front()->_cacheFilePath;
            const std::string& cacheFileDir = targets.front()->_cacheFileDir;
            
            if( response->isSucceed() ){
                // ダウンロードしたファイルをローカルへ保存する
                if( FileUtils::getInstance()->createDirectory( cacheFileDir ) ){
                    FILE* file = fopen( cacheFilePath.c_str(), "wb" );
                    fwrite( &response->getResponseData()->at(0), response->getResponseData()->size(), 1, file );
                    fclose( file );
                }
                for( auto it : targets ){
                    // 自分以外からの参照があれば処理
                    if( it->getReferenceCount() > 1 ){
                        // TODO: make thread. バイナリを直接渡せるようにする
                        it->addImageAsync(cacheFilePath);
                    }
                }
            }else{
                CCASSERT(0, cacheFilePath.c_str());
            }
            
            for( auto it : targets ){
                it->release();
            }
        });
        network::HttpClient::getInstance()->sendImmediate( req );
    }
}


#pragma mark -- LazySprite

LazySprite::LazySprite()
: _pCallback(nullptr)
{}

LazySprite::~LazySprite(){
}

Sprite* LazySprite::createAsync(const std::string& filename, const ccLazySpriteCallback& callback){
    LazySprite *sprite = new (std::nothrow) LazySprite();
    if (sprite && sprite->initAsync(filename, callback))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* LazySprite::createWithURL(const std::string& url, const ccLazySpriteCallback& callback, const std::string& cachePath){
    LazySprite *sprite = new (std::nothrow) LazySprite();
    if (sprite && sprite->initWithURL(url, callback, cachePath))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool LazySprite::initAsync(const std::string& filename, const ccLazySpriteCallback& callback){
    if( Sprite::init() ){
        addImageAsync(filename);
        return true;
    }
    return false;
}

bool LazySprite::initWithURL(const std::string& url, const ccLazySpriteCallback& callback, const std::string& cachePath){
    if( Sprite::init() ){
        _cacheFileDir = FileUtils::getInstance()->getWritablePath() + cachePath;
        if( *_cacheFileDir.rbegin() != '/' ){
            _cacheFileDir.push_back('/');
        }
        _cacheFilePath = _cacheFileDir + url.substr( url.rfind('/')+1, std::string::npos );
        _pCallback = callback;
        
        if( !cachePath.empty() && FileUtils::getInstance()->isFileExist( _cacheFilePath ) ){
            addImageAsync(_cacheFilePath);
        }else{
            requestDownload(url, this);
        }
        return true;
    }
    return false;
}

void LazySprite::addImageAsync(const std::string& filename){
    retain();
    Director::getInstance()->getTextureCache()->addImageAsync( filename, [this](Texture2D* texture){
        
        // 自分以外からの参照があれば処理
        if( getReferenceCount() > 1 ){
            setTexture( texture );
            setTextureRect( Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height ) );
            if( _pCallback != nullptr ){
                _pCallback( this );
            }
        }
        
        release();
    });
}

NS_CC_EXT_END
