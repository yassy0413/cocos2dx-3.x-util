/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_ZIP_DOWNLOADER_H__
#define __CC_ZIP_DOWNLOADER_H__

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
 *
 * @exsample ZipDownloader::getInstance()->download("http://localhost/test.zip", myCallback);
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
	 * @param url 未使用。コールバックで渡されるのみ
	 */
	void unzip(const std::string& url, const void* data, ssize_t datasize, const ccZipDownloaderCallback& callback);
	
	/**
	 * 実行中のタスク数を取得
	 */
	inline size_t numWorkingTask() const { return _numWorkingTask; }
	
	/**
	 * ダウンロードが極端に早い場合のメモリ使用量肥大を防ぐ為、ダウンロード要求に制限をかける閾値
	 *   ダウンロード済みでメモリ上に待機している合計データサイズが
	 *   limitで設定された値を超えている場合は、ダウンロード要求が待機される
	 */
	void setLimitDownloadedDataSize(size_t limit){ _limitDownloadedDataSize = limit; }
	
	/**
	 * ダウンロード同時進行数を設定
	 */
	void setLimitDownloadConcurrency(size_t limit){ _limitDownloadConcurrency = limit; }
	
private:
	ZipDownloader();
	virtual ~ZipDownloader();
	
	void downloadThread();
	void httpRequestCallback(network::HttpClient* client, network::HttpResponse* response);
	void unzipThread();
	void dispatchResponseCallbacks();
	
	struct DownloadUnit {
		ccZipDownloaderCallback* _pCallback;
		std::string _url;
	};
	
	struct UnzipUnit {
		ccZipDownloaderCallback* _pCallback;
		network::HttpResponse* _pHttpResponse;
		std::string _url;
		const void* _data;
		ssize_t _datasize;
		
		~UnzipUnit();
	};
	
	std::thread _downloadThread;
	std::queue<DownloadUnit*> _downloadQueue;
	std::mutex _downloadQueueMutex;
	std::condition_variable_any _downloadSleepCondition;
	
	std::thread _unzipThread;
	std::queue<UnzipUnit*> _unzipQueue;
	std::mutex _unzipQueueMutex;
	std::condition_variable_any _unzipSleepCondition;
	
	std::queue<UnzipUnit*> _responseQueue;
	std::mutex _responseQueueMutex;
	
	size_t _numWorkingTask;
	size_t _numWorkingDownloadTask;
	size_t _downloadedDataSize;
	size_t _limitDownloadedDataSize;
	size_t _limitDownloadConcurrency;
};

NS_CC_EXT_END

#endif