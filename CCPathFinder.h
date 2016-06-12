/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_PATH_FINDER_H__
#define __CC_PATH_FINDER_H__

#include "CCStringFinder.h"


NS_CC_EXT_BEGIN

/**
 * ファイル検索
 */
class PathFinder
: public StringFinder
{
public:
    CREATE_FUNC(PathFinder);
    PathFinder();
    virtual ~PathFinder();
    
    /**
     * 検索ディレクトリを設定してオブジェクトを作成
     */
    static PathFinder* createWithDirectory(const std::string& path);
    
public:
    
    typedef std::function<void(PathFinder* sender, const std::string& path)> ccFileSelectCallback;
    ccFileSelectCallback onFileSelectCallback;
    
    /**
     * ファイル情報
     */
    struct Stat {
        bool isDir;
        std::string path;
    };
    typedef std::vector<PathFinder::Stat> StatList;
    
    /**
     * 検索ディレクトリの設定
     */
    void setCurrentDirectory(const std::string& path);
    const std::string& getCurrentDirectory() const { return _currentDirectory; }
    
    /**
     * 現在のディレクトリから検出されたファイル情報リストを取得
     */
    const PathFinder::StatList& getStatList() const { return _statList; }
    
    /**
     * 指定パスにあるファイル一覧を取得する
     */
    static void readDirectory(std::string path, PathFinder::StatList& out);
    
private:
    StatList _statList;
    std::string _currentDirectory;
};

NS_CC_EXT_END

#endif