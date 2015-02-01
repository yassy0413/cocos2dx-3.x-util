#include "CCZipDownloader.h"
#include "HttpClient.h"

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
{
	_unzipThread = std::thread(CC_CALLBACK_0(ZipDownloader::unzipThread, this));
}

ZipDownloader::~ZipDownloader(){
	CC_ASSERT( numWorkingTask() == 0 );
	
	_unzipQueueMutex.lock();
	_unzipQueue.push( nullptr );
	_unzipQueueMutex.unlock();
	_SleepCondition.notify_one();
	
	_unzipThread.join();
	
	s_pZipDownloader = nullptr;
}

void ZipDownloader::download(const std::string& url, const ccZipDownloaderCallback& callback){
	
	++_numWorkingTask;
	
	network::HttpRequest* req = new network::HttpRequest();
	req->setRequestType( network::HttpRequest::Type::GET );
	req->setUrl( url.c_str() );
	req->setResponseCallback( CC_CALLBACK_2(ZipDownloader::httpRequestCallback, this) );
	req->setUserData( new ccZipDownloaderCallback( callback ) );
	
	network::HttpClient::getInstance()->send( req );
}

void ZipDownloader::httpRequestCallback(network::HttpClient* client, network::HttpResponse* response){
	//CCLOG("ZipDownloader::httpRequestCallback: %s", response->getHttpRequest()->getUrl());
	
	if( response->getResponseCode() != 200 ){
		CC_ASSERT(0);
		return;
	}
	
	UnzipUnit* pUnit = new UnzipUnit();
	pUnit->_pCallback = (ccZipDownloaderCallback*)response->getHttpRequest()->getUserData();
	pUnit->_url = response->getHttpRequest()->getUrl();
	pUnit->_data = &response->getResponseData()->at(0);
	pUnit->_datasize = response->getResponseData()->size();
	pUnit->_pHttpResponse = response;
	pUnit->_pHttpResponse->retain();
	
	_unzipQueueMutex.lock();
	_unzipQueue.push( pUnit );
	_unzipQueueMutex.unlock();
	
	_SleepCondition.notify_one();
}

void ZipDownloader::unzip(const std::string& url, const void* data, ssize_t datasize, const ccZipDownloaderCallback& callback){
	
	++_numWorkingTask;
	
	UnzipUnit* pUnit = new UnzipUnit();
	pUnit->_pCallback = new ccZipDownloaderCallback( callback );
	pUnit->_url = url;
	pUnit->_data = data;
	pUnit->_datasize = datasize;
	pUnit->_pHttpResponse = nullptr;
	
	_unzipQueueMutex.lock();
	_unzipQueue.push( pUnit );
	_unzipQueueMutex.unlock();
	
	_SleepCondition.notify_one();
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
				_SleepCondition.wait( _unzipQueueMutex );
			}
			pUnit = _unzipQueue.front();
			_unzipQueue.pop();
		}
		
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
		
		//
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
		CC_ASSERT(pUnit->_pCallback);
		(*pUnit->_pCallback)( pUnit->_url, pUnit->_data );
		delete pUnit;
	}
	
	--_numWorkingTask;
}

ZipDownloader::UnzipUnit::~UnzipUnit(){
	delete _pCallback;
	if( _pHttpResponse ){
		_pHttpResponse->release();
	}
}

NS_CC_EXT_END
