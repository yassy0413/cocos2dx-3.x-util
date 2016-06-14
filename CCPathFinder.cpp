/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCPathFinder.h"
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>


NS_CC_EXT_BEGIN

PathFinder* PathFinder::createWithDirectory(const std::string& path){
    PathFinder* pRet = new (std::nothrow) PathFinder();
    if( pRet && pRet->init() ){
        pRet->autorelease();
        pRet->setCurrentDirectory(path);
        return pRet;
    }
    delete pRet;
    return nullptr;
}

PathFinder::PathFinder()
: onFileSelectCallback(nullptr)
{
    onStringSelectCallback = [this](StringFinder* sender, const char* text){
        for( const auto& stat : _statList ){
            if( stat.path == text ){
                if( stat.isDir ){
                    setCurrentDirectory( getCurrentDirectory() + stat.path );
                }else{
                    if( onFileSelectCallback ){
                        onFileSelectCallback( this, getCurrentDirectory() + stat.path );
                    }
                }
                break;
            }
        }
    };
}

PathFinder::~PathFinder(){
}

PathFinder::Stat::Stat(bool d, const char* p)
: isDir(d)
, path(p)
{}

#pragma mark --

void PathFinder::setCurrentDirectory(const std::string& path){
    
    CC_ASSERT(!path.empty());
    
    auto rit = path.rbegin();
    
    if( *rit == '.' ){
        ++rit;
        if( rit != path.rend() && *rit == '.' ){
            _currentDirectory = _currentDirectory.substr( 0, _currentDirectory.rfind('/') );
            _currentDirectory = _currentDirectory.substr( 0, _currentDirectory.rfind('/') );
        }
    }else{
        _currentDirectory = path;
    }
    
    if( *_currentDirectory.rbegin() != '/' ){
        _currentDirectory.push_back('/');
    }
    
    //
    readDirectory( _currentDirectory, _statList );
    
    //
    std::vector<std::string> strings;
    for( const auto& stat : _statList ){
        strings.push_back( stat.path );
    }
    setStrings( std::move(strings) );
    
    //
    setFilter( "" );
}

void PathFinder::readDirectory(std::string path, PathFinder::StatList& out){
    
    CC_ASSERT(!path.empty());
    
    std::string path_( path );
    if( *path_.rbegin() == '/' ){
        path_.resize( path_.size() - 1 );
    }
    cocos2d::FileUtils::getInstance()->purgeCachedEntries();
    const std::string rootpath( cocos2d::FileUtils::getInstance()->fullPathForFilename(path_) );
    CCLOG("PathFinder::readDirectory: [%s]", rootpath.c_str());
    
    out.clear();
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
    WIN32_FIND_DATAA win32fd;
    HANDLE hFind = FindFirstFileA((rootpath + "\\*.*").c_str(), &win32fd);
    if( hFind != INVALID_HANDLE_VALUE ){
        do {
            out.emplace_back( (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, win32fd.cFileName );
        }
        while( FindNextFileA(hFind, &win32fd) );
        
        FindClose(hFind);
    }
#else
    if( DIR* dir = opendir( rootpath.c_str() ) ){
        struct stat statbuf;
        for( dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir) ){
            const std::string fullpath_( rootpath + "/" + entry->d_name );
            lstat( fullpath_.c_str(), &statbuf );
            out.emplace_back( S_ISDIR(statbuf.st_mode), entry->d_name );
        }
        closedir( dir );
    }
#endif
}


NS_CC_EXT_END
