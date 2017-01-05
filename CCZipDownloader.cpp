/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCZipDownloader.h"
#include "network/HttpClient.h"

NS_CC_EXT_BEGIN

static void pushToWriteFile(const ccZipDownloaderCallback& callback, const std::string& path, unsigned char* data, ssize_t size){
    CCLOG("pushToWriteFile: %s", path.c_str());
    // 別スレッドで実行されるタスク
    auto task = [path, data, size](){
        if( data ){
            FILE* file = fopen(path.c_str(), "wb");
            fwrite(data, size, 1, file);
            fclose(file);
            CCLOG("endOfWriteFile: %s", path.c_str());
            
            // zipfile->getFileData で確保されたメモリを開放する
            free(data);
        }
    };
    // 最後にUIスレッドで実行されるタスク
    auto finished = [callback](void*){
        if(callback){ callback(true); }
    };
    // 非同期タスクの開始 (TASK_IOのスレッドキューへ積まれる)
    AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, finished, nullptr, task);
}

static void pushToUnzip(const ccZipDownloaderCallback& callback, const std::string& outdir, network::HttpResponse* response){
    // 別スレッドで実行されるタスク
    auto task = [callback, outdir, response](){
        
        // zipに含まれるファイル情報リストを取得
        ZipFile* zipfile = ZipFile::createWithBuffer(response->getResponseData()->data(),
                                                     response->getResponseData()->size());
        for( std::string filename = zipfile->getFirstFilename(); !filename.empty(); filename = zipfile->getNextFilename() ){
            if( *filename.rbegin() == '/' ){
                
                // It's a directory.
                FileUtils::getInstance()->createDirectory( outdir + filename );
                
            }else{
                
                // It's a file.
                ssize_t filesize;
                unsigned char* filedata = zipfile->getFileData( filename, &filesize );
                // ファイルデータが取得できたので、書き込みを行うスレッドへ送る
                pushToWriteFile(nullptr, outdir + filename, filedata, filesize);
                
            }
        }
        delete zipfile;
        
        // 終了判定用に空データを送る
        pushToWriteFile(callback, "", nullptr, 0);
    };
    // 最後にUIスレッドで実行されるタスク
    auto finished = [response](void*){
        response->release();
    };
    // unzipタスク実行中に破棄されないよう保護する
    response->retain();
    // 非同期タスクの開始 (TASK_OTHERのスレッドキューへ積まれる)
    AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_OTHER, finished, nullptr, task);
}

#pragma mark - ZipDownloader Class

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
{}

ZipDownloader::~ZipDownloader(){
}

void ZipDownloader::download(const std::string& url, const std::string& outdir, const ccZipDownloaderCallback& callback){
    auto req = new (std::nothrow) network::HttpRequest();
    req->setRequestType(network::HttpRequest::Type::GET);
    req->setUrl(url);
    req->setResponseCallback([callback, outdir](network::HttpClient* client, network::HttpResponse* response){
        if( response->isSucceed() ){
            // ダウンロードしたzipファイルを展開スレッドへ送る
            pushToUnzip(callback, outdir, response);
        }else{
            callback(false);
        }
    });
    network::HttpClient::getInstance()->send( req );
    req->release();
}

NS_CC_EXT_END
