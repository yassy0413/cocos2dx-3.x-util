#ifndef __CCB_ZIP_DOWNLOADER_H__
#define __CCB_ZIP_DOWNLOADER_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include <thread>
#include <queue>

NS_CC_BEGIN
namespace network {
	class HttpClient;
	class HttpResponse;
}
NS_CC_END

NS_CC_EXT_BEGIN

class ZipDownloader;
typedef std::function<void(const std::string& url, const void* data)> ccZipDownloaderCallback;

/*
 * zipファイルのダウンロードと展開
 */
class ZipDownloader
{
public:
	/** Return the shared instance **/
	static ZipDownloader *getInstance();
	
	/** Relase the shared instance **/
	static void destroyInstance();
	
	/**
	 * urlで指定されたzipファイルをダウンロードし、WritablePathへ展開する
	 */
	void download(const std::string& url, const ccZipDownloaderCallback& callback);
	
	/**
	 * dataで指定されたzipのバイナリデータを、WritablePathへ展開する
	 */
	void unzip(const std::string& url, const void* data, ssize_t datasize, const ccZipDownloaderCallback& callback);
	
	/**
	 * 実行中のタスク数を取得
	 */
	inline size_t numWorkingTask() const { return _numWorkingTask; }
	
private:
	ZipDownloader();
	virtual ~ZipDownloader();
	
	void httpRequestCallback(network::HttpClient* client, network::HttpResponse* response);
	void unzipThread();
	void dispatchResponseCallbacks();
	
	struct UnzipUnit {
		ccZipDownloaderCallback* _pCallback;
		std::string _url;
		const void* _data;
		ssize_t _datasize;
		network::HttpResponse* _pHttpResponse;
		
		~UnzipUnit();
	};
	
	std::thread _unzipThread;
	
	std::queue<UnzipUnit*> _unzipQueue;
	std::mutex _unzipQueueMutex;
	std::condition_variable_any _SleepCondition;
	
	std::queue<UnzipUnit*> _responseQueue;
	std::mutex _responseQueueMutex;
	
	size_t _numWorkingTask;
};

NS_CC_EXT_END

#endif