/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_ZIP_DOWNLOADER_H__
#define __CC_ZIP_DOWNLOADER_H__

#include "cocos2d.h"
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

typedef std::function<void(bool succeeded)> ccZipDownloaderCallback;

/**
 * zipファイルのダウンロードと展開
 *
 @code
 ZipDownloader::getInstance()->download("http://localhost/test.zip", myCallback);
 @endcode
 */
class ZipDownloader
{
public:
    CC_DISALLOW_COPY_AND_ASSIGN(ZipDownloader);
    
    /** Return the shared instance **/
    static ZipDownloader *getInstance();
    
    /** Relase the shared instance **/
    static void destroyInstance();
    
    /**
     * urlで指定されたzipファイルをダウンロードし、outdirへ展開する
     */
    void download(const std::string& url, const std::string& outdir, const ccZipDownloaderCallback& callback);
    
private:
    ZipDownloader();
    virtual ~ZipDownloader();
};

NS_CC_EXT_END

#endif
