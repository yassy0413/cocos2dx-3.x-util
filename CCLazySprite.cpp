/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCLazySprite.h"
#include "network/HttpClient.h"

NS_CC_EXT_BEGIN

typedef std::unordered_map<std::string, std::list<LazySprite*>> DownloadingTaskList;
static DownloadingTaskList* _downloadingTasks = nullptr;

void LazySprite::requestDownload(const std::string& url, const std::string& cacheFilePath, const std::string& cacheFileDir, LazySprite* target){
    
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
        req->setResponseCallback([cacheFilePath, cacheFileDir](network::HttpClient* client, network::HttpResponse* response){
            
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
            
            if( response->isSucceed() ){
                // ダウンロードしたファイルをローカルへ保存する
                if( FileUtils::getInstance()->createDirectory( cacheFileDir ) ){
                    FILE* file = fopen( cacheFilePath.c_str(), "wb" );
                    fwrite( response->getResponseData()->data(), response->getResponseData()->size(), 1, file );
                    fclose( file );
                }
                for( auto it : targets ){
                    // 自分以外からの参照があれば処理
                    if( it->getReferenceCount() > 1 ){
                        it->addImageAsync(cacheFilePath, std::move(*response->getResponseData()));
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
        req->release();
    }
}


#pragma mark -- LazySprite

LazySprite::LazySprite()
: _finishedCallback(nullptr)
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
    delete sprite;
    return nullptr;
}

Sprite* LazySprite::createWithURL(const std::string& url, const ccLazySpriteCallback& callback, const std::string& cachePath){
    LazySprite *sprite = new (std::nothrow) LazySprite();
    if (sprite && sprite->initWithURL(url, callback, cachePath))
    {
        sprite->autorelease();
        return sprite;
    }
    delete sprite;
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
        _finishedCallback = callback;
        
        std::string cacheFileDir = FileUtils::getInstance()->getWritablePath() + cachePath;
        if( *cacheFileDir.rbegin() != '/' ){
            cacheFileDir.push_back('/');
        }
        
        std::string filename;
        const auto separator = url.rfind('/');
        if( separator != std::string::npos ){
            const std::string domain = url.substr(0, separator);
            if( !domain.empty() ){
                char* buf;
                cocos2d::base64Encode((const unsigned char*)domain.c_str(), (unsigned int)domain.length(), &buf);
                cacheFileDir += buf;
                cacheFileDir.push_back('/');
                free(buf);
            }
            filename = url.substr(separator+1, std::string::npos);
            if( !filename.empty() ){
                char* buf;
                cocos2d::base64Encode((const unsigned char*)filename.c_str(), (unsigned int)filename.length(), &buf);
                filename = buf;
                free(buf);
            }
        }
        if( filename.empty() ){
            filename = cocos2d::StringUtils::toString( std::hash<std::string>()(url) );
        }
        const std::string cacheFilePath = cacheFileDir + filename;
        
        if( FileUtils::getInstance()->isFileExist( cacheFilePath ) ){
            addImageAsync(cacheFilePath);
            CCLOG("cache hit [%s]", url.c_str());
        }else{
            requestDownload(url, cacheFilePath, cacheFileDir, this);
        }
        return true;
    }
    return false;
}

void LazySprite::addImageAsync(const std::string& filename){
    retain();
    Director::getInstance()->getTextureCache()->addImageAsync(filename, [this](Texture2D* texture){
        
        // 自分以外からの参照があれば処理
        if( getReferenceCount() > 1 ){
            setTexture(texture);
            setTextureRect(Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height ));
            if( _finishedCallback != nullptr ){
                _finishedCallback(this);
            }
        }
        
        release();
    });
}

void LazySprite::addImageAsync(std::string filename, std::vector<char>&& data){
    retain();
    auto it = _work.emplace(_work.end(), std::move(filename), std::move(data), nullptr);
    //
    auto task = [this, it](){
        const std::vector<char>& data = std::get<1>(*it);
        Image* image = new (std::nothrow) Image();
        image->initWithImageData(reinterpret_cast<const unsigned char*>(data.data()), data.size());
        std::get<2>(*it) = image;
    };
    auto finished = [this, it](void*){
        // 自分以外からの参照があれば処理
        if( getReferenceCount() > 1 ){
            Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(std::get<2>(*it), std::get<0>(*it));
            setTexture(texture);
            setTextureRect(Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height ));
            if( _finishedCallback != nullptr ){
                _finishedCallback( this );
            }
        }
        _work.erase(it);
    };
    AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_OTHER, finished, nullptr, task);
}

NS_CC_EXT_END
