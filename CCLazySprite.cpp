/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCLazySprite.h"
#include "HttpClient.h"

NS_CC_EXT_BEGIN

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
		retain();
		Director::getInstance()->getTextureCache()->addImageAsync( filename, CC_CALLBACK_1(LazySprite::textureLoadCallback, this) );
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
			retain();
			Director::getInstance()->getTextureCache()->addImageAsync( _cacheFilePath, CC_CALLBACK_1(LazySprite::textureLoadCallback, this) );
		}else{
			network::HttpRequest* req = new network::HttpRequest();
			req->setRequestType( network::HttpRequest::Type::GET );
			req->setUrl( url.c_str() );
			req->setResponseCallback( CC_CALLBACK_2(LazySprite::httpRequestCallback, this) );
			
			retain();
			network::HttpClient::getInstance()->sendImmediate( req );
		}
		return true;
	}
	return false;
}

void LazySprite::httpRequestCallback(network::HttpClient* client, network::HttpResponse* response){
	
	if( response->getResponseCode() == 200 ){
		// ダウンロードしたファイルをローカルへ保存する
		if( FileUtils::getInstance()->createDirectory( _cacheFileDir ) ){
			FILE* file = fopen( _cacheFilePath.c_str(), "wb" );
			fwrite( &response->getResponseData()->at(0), response->getResponseData()->size(), 1, file );
			fclose( file );
		}
		// 自分以外からの参照があれば処理
		if( getReferenceCount() > 1 ){
			// TODO: make thread. バイナリを直接渡せるようにする
			retain();
			Director::getInstance()->getTextureCache()->addImageAsync( _cacheFilePath, CC_CALLBACK_1(LazySprite::textureLoadCallback, this) );
		}
	}else{
		CCLOG("%s", _cacheFilePath.c_str());
		CC_ASSERT(0);
	}
	
	release();
}

void LazySprite::textureLoadCallback(Texture2D* texture){
	
	// 自分以外からの参照があれば処理
	if( getReferenceCount() > 1 ){
		setTexture( texture );
		setTextureRect( Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height ) );
		if( _pCallback != nullptr ){
			_pCallback( this );
		}
	}
	
	release();
}

NS_CC_EXT_END
