/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCZipDownloader.h"
#include "network/HttpClient.h"

NS_CC_EXT_BEGIN

static ZipDownloader *s_pZipDownloader = nullptr; // pointer to singleton

ZipDownloader* ZipDownloader::getInstance(){
    if (s_pZipDownloader == nullptr) {
        s_pZipDownloader = new (std::nothrow) ZipDownloader();
    }
    return s_pZipDownloader;
}

void ZipDownloader::destroyInstance(){
    CC_SAFE_DELETE(s_pZipDownloader);
}

ZipDownloader::ZipDownloader()
: _numWorkingTask(0)
, _numWorkingDownloadTask(0)
, _downloadedDataSize(0)
, _limitDownloadedDataSize(10*1024*1024)
, _limitDownloadConcurrency(3)
{
    _downloadThread = std::thread(CC_CALLBACK_0(ZipDownloader::downloadThread, this));
    _unzipThread = std::thread(CC_CALLBACK_0(ZipDownloader::unzipThread, this));
}

ZipDownloader::~ZipDownloader(){
    CC_ASSERT( numWorkingTask() == 0 );
    
    _downloadQueueMutex.lock();
    _downloadQueue.push( nullptr );
    _downloadQueueMutex.unlock();
    _downloadSleepCondition.notify_one();
    
    _unzipQueueMutex.lock();
    _unzipQueue.push( nullptr );
    _unzipQueueMutex.unlock();
    _unzipSleepCondition.notify_one();
    
    _downloadThread.join();
    _unzipThread.join();
    
    s_pZipDownloader = nullptr;
}

#pragma mark -- Download Work

void ZipDownloader::download(const std::string& url, ccZipDownloaderCallback callback){
    
    ++_numWorkingTask;
    
    auto pUnit = new DownloadUnit();
    pUnit->_callback = callback;
    pUnit->_url = url;
    
    _downloadQueueMutex.lock();
    _downloadQueue.push( pUnit );
    _downloadQueueMutex.unlock();
    _downloadSleepCondition.notify_one();
}

void ZipDownloader::downloadThread(){
    
    while(1)
    {
        DownloadUnit* pUnit = nullptr;
        
        // タスク発行待ち
        {
            std::lock_guard<std::mutex> lock( _downloadQueueMutex );
            while( _downloadQueue.empty() || (_numWorkingDownloadTask >= _limitDownloadConcurrency) || (_downloadedDataSize > _limitDownloadedDataSize) ){
                _downloadSleepCondition.wait( _downloadQueueMutex );
            }
            pUnit = _downloadQueue.front();
            _downloadQueue.pop();
        }
        
        // 終了検知
        if( pUnit == nullptr ) {
            break;
        }
        
        // HttpRequest の発行
        auto req = new network::HttpRequest();
        req->setRequestType( network::HttpRequest::Type::GET );
        req->setUrl( pUnit->_url.c_str() );
        req->setResponseCallback( CC_CALLBACK_2(ZipDownloader::httpRequestCallback, this) );
        req->setUserData( pUnit );
        
        ++_numWorkingDownloadTask;
        network::HttpClient::getInstance()->sendImmediate( req );
    }
}

void ZipDownloader::httpRequestCallback(network::HttpClient* client, network::HttpResponse* response){
    //CCLOG("ZipDownloader::httpRequestCallback: %s", response->getHttpRequest()->getUrl());
    
    if( !response->isSucceed() ){
        CC_ASSERT(0);
        return;
    }
    
    --_numWorkingDownloadTask;
    
    auto pUnit = new UnzipUnit();
    pUnit->_url = response->getHttpRequest()->getUrl();
    pUnit->_data = &response->getResponseData()->at(0);
    pUnit->_datasize = response->getResponseData()->size();
    pUnit->_pHttpResponse = response;
    pUnit->_pHttpResponse->retain();
    
    auto dlUnit = (DownloadUnit*)response->getHttpRequest()->getUserData();
    pUnit->_callback = dlUnit->_callback;
    delete dlUnit;
    
    _downloadedDataSize += pUnit->_datasize;
    
    _unzipQueueMutex.lock();
    _unzipQueue.push( pUnit );
    _unzipQueueMutex.unlock();
    _unzipSleepCondition.notify_one();
}

#pragma mark -- Unzip Work

void ZipDownloader::unzip(const std::string& url, const void* data, ssize_t datasize, ccZipDownloaderCallback callback){
    
    ++_numWorkingTask;
    
    UnzipUnit* pUnit = new UnzipUnit();
    pUnit->_url = url;
    pUnit->_data = data;
    pUnit->_datasize = datasize;
    pUnit->_pHttpResponse = nullptr;
    pUnit->_callback = callback;
    
    _downloadedDataSize += pUnit->_datasize;
    
    _unzipQueueMutex.lock();
    _unzipQueue.push( pUnit );
    _unzipQueueMutex.unlock();
    _unzipSleepCondition.notify_one();
}

void ZipDownloader::unzipThread(){
    
    auto scheduler = Director::getInstance()->getScheduler();
    
    // 出力先のルートパスを取得
    const std::string writablePath( cocos2d::FileUtils::getInstance()->getWritablePath() );
    
    while(1)
    {
        UnzipUnit* pUnit = nullptr;
        
        // タスク発行待ち
        {
            std::lock_guard<std::mutex> lock( _unzipQueueMutex );
            while( _unzipQueue.empty() ){
                _unzipSleepCondition.wait( _unzipQueueMutex );
            }
            pUnit = _unzipQueue.front();
            _unzipQueue.pop();
        }
        
        _downloadSleepCondition.notify_one();
        
        // 終了検知
        if( pUnit == nullptr ) {
            break;
        }
        
        // zipに含まれるファイル情報リストを取得して展開
        cocos2d::ZipFile* zipfile = cocos2d::ZipFile::createWithBuffer( pUnit->_data, pUnit->_datasize );
        for( std::string filename = zipfile->getFirstFilename(); !filename.empty(); filename = zipfile->getNextFilename() ){
            if( *filename.rbegin() == '/' ){
                
                // It's a directory.
                cocos2d::FileUtils::getInstance()->createDirectory( writablePath + filename );
                
            }else{
                
                // It's a file.
                ssize_t filesize;
                unsigned char* filedata = zipfile->getFileData( filename, &filesize );
                {
                    FILE* file = fopen( (writablePath + filename).c_str(), "wb" );
                    fwrite( filedata, filesize, 1, file );
                    fclose( file );
                }
                free( filedata );
                
            }
        }
        delete zipfile;
        
        _downloadedDataSize -= pUnit->_datasize;
        
        // UIスレッドからのコールバック設定
        _responseQueueMutex.lock();
        _responseQueue.push( pUnit );
        _responseQueueMutex.unlock();
        
        scheduler->performFunctionInCocosThread(CC_CALLBACK_0(ZipDownloader::dispatchResponseCallbacks, this));
    }
}

void ZipDownloader::dispatchResponseCallbacks(){
    
    UnzipUnit* pUnit = nullptr;
    
    _responseQueueMutex.lock();
    if( !_responseQueue.empty() ){
        pUnit = _responseQueue.front();
        _responseQueue.pop();
    }
    _responseQueueMutex.unlock();
    
    if( pUnit ){
        if( pUnit->_callback ){
            pUnit->_callback( pUnit->_url, pUnit->_data );
        }
        delete pUnit;
    }
    
    --_numWorkingTask;
}

ZipDownloader::UnzipUnit::~UnzipUnit(){
    if( _pHttpResponse ){
        _pHttpResponse->release();
    }
}

NS_CC_EXT_END
