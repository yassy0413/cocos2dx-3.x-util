/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCComicView.h"
#include "CCScopedClock.h"
#include "network/HttpClient.h"
#include <unistd.h>

NS_CC_BEGIN

#pragma mark -- Attribute

ComicView::Attribute::Attribute()
: direction(Direction::Horizontal)
, pageAdjustment(true)
, pageAdjustmentSpeed(6.0f)
, pageAdjustmentThreshold(0.05f)
, pageAdjustmentLowSpeed(4.0f)
, inertiaDumpingForce(100.0f)
, cacheRange(3)
{}


#pragma mark -- PageData

void ComicView::PageData::clear(){
    CC_SAFE_RELEASE_NULL(texture);
    CC_SAFE_RELEASE_NULL(image);
    CCLOG("*TexDel: %s", url.c_str());
}

void ComicView::PageData::flipData(){
    if( data.size() >= sizeof(int64_t) ){
        int64_t* p = reinterpret_cast<int64_t*>(data.data());
        *p = ~*p;
    }
}


#pragma mark -- ThreadSafeQueue

template <class T>
void ComicView::ThreadSafeQueue<T>::push(T component){
    _mutex.lock();
    _queue.push_back(component);
    _mutex.unlock();
    _sleepCondition.notify_one();
}

template <class T>
T ComicView::ThreadSafeQueue<T>::pop(){
    std::lock_guard<std::mutex> _(_mutex);
    if( !_queue.empty() ){
        auto component = _queue.front();
        _queue.pop_front();
        return component;
    }
    return nullptr;
}

template <class T>
T ComicView::ThreadSafeQueue<T>::pop_wait(){
    while( empty() ){// take measures against spurious wakeup
        _sleepCondition.wait(_sleepMutex);
    }
    std::lock_guard<std::mutex> _(_mutex);
    auto component = _queue.front();
    _queue.pop_front();
    return component;
}

template <class T>
bool ComicView::ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> _(_mutex);
    return _queue.empty();
}

template <class T>
bool ComicView::ThreadSafeQueue<T>::exists(T component) const {
    std::lock_guard<std::mutex> _(_mutex);
    const auto it = std::find(_queue.begin(), _queue.end(), component);
    return it != _queue.end();
}

template <class T>
int32_t ComicView::ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> _(_mutex);
    return  static_cast<int32_t>(_queue.size());
}


#pragma mark -- Class ComicView

ComicView* ComicView::createWithAttribute(std::unique_ptr<Attribute> attribute){
    auto pRet = new (std::nothrow) ComicView();
    if( pRet && pRet->initWithAttribute(std::move(attribute)) ){
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

ComicView::ComicView()
: _pageOffset(0)
, _touchMoved(0)
, _adjustment(false)
, _inertiaEnabled(false)
, _touching(false)
{}

ComicView::~ComicView(){
    if( _pageFileThread.joinable() ){
        _pageFileQueue.push(nullptr);
        _pageFileThread.join();
    }
    if( _pageImageThread.joinable() ){
        _pageImageQueue.push(nullptr);
        _pageImageThread.join();
    }
}

bool ComicView::initWithAttribute(std::unique_ptr<Attribute> attribute){
    
    _attribute = std::move(attribute);
    CC_ASSERT(_attribute->cacheRange > 0);
    
    _attribute->cacheDir = FileUtils::getInstance()->getWritablePath() + _attribute->cacheDir;
    if( *_attribute->cacheDir.rbegin() != '/' ){
        _attribute->cacheDir.push_back('/');
    }
    
    //
    _pageDatas.resize(_attribute->urlList.size());
    for( int lp = 0; lp < _pageDatas.size(); ++lp ){
        auto& pageData = _pageDatas[lp];
        pageData.index = lp;
        pageData.url = _attribute->urlList[lp];
        pageData.storagePath = makePath(pageData.url);
        pageData.loading = false;
        pageData.initializing = false;
        pageData.image = nullptr;
        pageData.texture = nullptr;
    }
    
    //
    _pageViews.resize(_attribute->cacheRange * 2 + 1);
    for( int lp = 0; lp < _pageViews.size(); ++lp ){
        auto& pageView = _pageViews[lp];
        pageView.offsetIndex = lp - (int)_pageViews.size()/2;
        pageView.index = lp;
        pageView.sprite = new (std::nothrow) Sprite();
        pageView.sprite->init();
        pageView.sprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        pageView.loadingNode = new (std::nothrow) Node();
        pageView.loadingNode->init();
        pageView.loadingNode->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        pageView.loadingNode->setVisible(false);
        addChild(pageView.sprite);
        addChild(pageView.loadingNode);
    }
    
    setPage(0);
    
    //
    _pageFileThread = std::thread([this](){
        while( auto p = _pageFileQueue.pop_wait() ){
            if( p->data.empty() ){
                FILE* fp = fopen(p->storagePath.c_str(), "rb");
                fseek(fp, 0, SEEK_END);
                p->data.resize(ftell(fp));
                fseek(fp, 0, SEEK_SET);
                fread(p->data.data(), p->data.size(), 1, fp);
                fclose(fp);
                p->flipData();
            }else{
                p->flipData();
                FILE* fp = fopen(p->storagePath.c_str(), "wb");
                fwrite(p->data.data(), p->data.size(), 1, fp);
                fclose(fp);
                p->flipData();
            }
            p->loading = false;
            p->initializing = true;
            _pageImageQueue.push(p);
        }
    });
    
    //
    _pageImageThread = std::thread([this](){
        while( auto p = _pageImageQueue.pop_wait() ){
            CC_ASSERT(!p->loading);
            CC_SCOPED_CLODK("***** LoadImage");
            
            if( !p->image ){
                p->image = new (std::nothrow) Image();
                p->image->initWithImageData((unsigned char*)p->data.data(), p->data.size());
            }
            p->initializing = false;
            
            _pageImageReadyQueue.push(p);
        }
    });
    
    //
    auto touch = EventListenerTouchOneByOne::create();
    touch->onTouchBegan = [this](Touch* touch, Event*){
        const Vec2 touchLocation = getParent()->convertToNodeSpace(touch->getLocation());
        if( getBoundingBox().containsPoint(touchLocation) ){
            if( _attribute->direction == Direction::Horizontal ){
                _touchLastPoint = touchLocation.x;
            }else{
                _touchLastPoint = touchLocation.y;
            }
            
            _touching = true;
            _touchMoved = 0;
            _inertiaEnabled = false;
            _inertiaSpeed = 0;
            return true;
        }
        return false;
    };
    touch->onTouchMoved = [this](Touch* touch, Event*){
        float point;
        const Vec2 touchLocation = getParent()->convertToNodeSpace(touch->getLocation());
        if( _attribute->direction == Direction::Horizontal ){
            point = touchLocation.x;
        }else{
            point = touchLocation.y;
        }
        
        const float diff = point - _touchLastPoint;
        _touchLastPoint = point;
        
        _touchMoved += diff;
        _pageOffset += diff;
        _inertiaSpeed = diff;
    };
    touch->onTouchEnded = [this](Touch* touch, Event*){
        if( _attribute->pageAdjustment ){
            if( fabsf(_touchMoved) > _pageSize * _attribute->pageAdjustmentThreshold ){
                if( _touchMoved > 0 && (_pageIndex < _pageDatas.size()-1) ){
                    _adjustmentTargetOffset = (_adjustmentTargetOffset < 0)? 0 : _pageSize;
                }else if( _touchMoved < 0 && (_pageIndex > 0) ){
                    _adjustmentTargetOffset = (_adjustmentTargetOffset > 0)? 0 : -_pageSize;
                }
            }else{
                _adjustmentTargetOffset = 0;
            }
            _adjustment = true;
        }else{
            _inertiaEnabled = true;
        }
        
        _touching = false;
    };
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(touch, this);
    _touchEvent = touch;
    
    return true;
}

void ComicView::onEnter(){
    Node::onEnter();
    scheduleUpdate();
    
    if( getContentSize().equals(Size::ZERO) ){
        setContentSize(Director::getInstance()->getWinSize());
    }
}

void ComicView::onExit(){
    Node::onExit();
    unscheduleUpdate();
}

void ComicView::update(float delta){
    
    delta = std::min(delta, Director::getInstance()->getAnimationInterval());
    
    // 別スレッドで読み込んだイメージからテクスチャを作成し、表示対象があれば適用する
    if( auto pageData = _pageImageReadyQueue.pop() ){
        
        auto it = std::find_if(_pageViews.begin(), _pageViews.end(), [pageData](const PageView& pageView){
            return pageData->index == pageView.index;
        });
        if( it == _pageViews.end() ){
            // 対象がいなくなっているので削除
            pageData->clear();
        }else{
            it->sprite->setVisible(true);
            it->loadingNode->setVisible(false);
            updatePage(*it, *pageData);
        }
    }
    
    // 目的のページへとオフセットを進める
    if( _adjustment ){
        if( !_touching ){
            const float diff = _adjustmentTargetOffset - _pageOffset;
            if( fabsf(diff) <= _attribute->pageAdjustmentLowSpeed ){
                _pageOffset = _adjustmentTargetOffset;
                _adjustmentTargetOffset = 0;
                _adjustment = false;
            }else{
                const float add = diff * (delta * _attribute->pageAdjustmentSpeed);
                const float sign = (add < 0.0f)? -1.0f : 1.0f ;
                _pageOffset += std::max(_attribute->pageAdjustmentLowSpeed, fabsf(add)) * sign;
            }
        }
        
    }else if( _inertiaEnabled ){// 慣性を効かせる
        
        const float sign = (_inertiaSpeed>0)? 1.0f : -1.0f ;
        
        static const float limit_speed = 50.0f;
        if( fabsf(_inertiaSpeed) > limit_speed ){
            _inertiaSpeed = limit_speed * sign;
        }
        
        _pageOffset += _inertiaSpeed;
        
        _inertiaSpeed -= std::min(fabsf(_inertiaSpeed), _attribute->inertiaDumpingForce * delta) * sign;
        if( fabsf(_inertiaSpeed) < 1.0f ){
            _inertiaEnabled = false;
        }
    }
    
    // オフセット位置に合わせて、ページを更新
    if( !_attribute->pageAdjustment || !_touching ){
        if( _pageOffset >= _pageSize ){
            _pageOffset -= _pageSize;
            advancePage(+1);
        }else if( _pageOffset <= -_pageSize ){
            _pageOffset += _pageSize;
            advancePage(-1);
        }
    }
    
    // ページの自動位置補正
    if( !_attribute->pageAdjustment ){
        if( _pageIndex == 0 ){
            if( _pageOffset < 0 ){
                _pageOffset = 0;
            }
        }else if( _pageIndex >= _pageDatas.size()-1 ){
            if( _pageOffset > 0 ){
                _pageOffset = 0;
            }
        }
    }
    
    //
    updateSpritePosition();
}

void ComicView::setContentSize(const Size& contentSize){
    Node::setContentSize(contentSize);
    
    if( _attribute->direction == Direction::Horizontal ){
        _pageSize = getContentSize().width;
    }else{
        _pageSize = getContentSize().height;
    }
}

void ComicView::setTouchEnabled(bool enabled){
    _touchEvent->setEnabled(enabled);
}

std::string ComicView::makePath(const std::string& url) const {
    if( url.find_first_of("http") == std::string::npos ){
        return FileUtils::getInstance()->fullPathForFilename(url);
    }else{
        std::string filename, dir = _attribute->cacheDir;
        const auto separator = url.rfind('/');
        if( separator != std::string::npos ){
            const std::string domain = url.substr(0, separator);
            if( !domain.empty() ){
                char* buf;
                base64Encode((const unsigned char*)domain.c_str(), (unsigned int)domain.length(), &buf);
                dir += buf;
                dir.push_back('/');
                free(buf);
            }
            filename = url.substr(separator+1, std::string::npos);
            if( !filename.empty() ){
                char* buf;
                base64Encode((const unsigned char*)filename.c_str(), (unsigned int)filename.length(), &buf);
                filename = buf;
                free(buf);
            }
        }
        if( filename.empty() ){
            filename = StringUtils::toString( std::hash<std::string>()(url) );
        }
        FileUtils::getInstance()->createDirectory(dir);
        return dir + filename;
    }
}

void ComicView::startDownload(PageData& pageData){
    auto request = new (std::nothrow) network::HttpRequest();
    request->setRequestType(network::HttpRequest::Type::GET);
    request->setUrl(pageData.url);
    request->setResponseCallback([this, &pageData](network::HttpClient* client, network::HttpResponse* response){
        if( response->isSucceed() ){
            pageData.data = std::move(*response->getResponseData());
            // ストレージへ保存
            _pageFileQueue.push(&pageData);
        }else{
            //TODO: retry
            CCLOG("ERROR[%ld] %s", response->getResponseCode(), response->getErrorBuffer());
        }
    });
    network::HttpClient::getInstance()->send(request);
    request->release();
}

void ComicView::updateSpritePosition(){
    const auto half = getContentSize() * 0.5f;
    if( _attribute->direction == Direction::Horizontal ){
        for( auto& pageView : _pageViews ){
            pageView.sprite->setPositionX(half.width + _pageOffset - _pageSize * pageView.offsetIndex);
            pageView.sprite->setPositionY(half.height);
            pageView.loadingNode->setPosition(pageView.sprite->getPosition());
        }
    }else{
        for( auto& pageView : _pageViews ){
            pageView.sprite->setPositionX(half.width);
            pageView.sprite->setPositionY(half.height + _pageOffset - _pageSize * pageView.offsetIndex);
            pageView.loadingNode->setPosition(pageView.sprite->getPosition());
        }
    }
}

void ComicView::updatePage(PageView& pageView, PageData& pageData){

    if( !pageData.texture ){
        pageData.texture = new (std::nothrow) Texture2D();
        CC_SCOPED_CLODK("***** updatePage");
        pageData.texture->initWithImage(pageData.image, Texture2D::PixelFormat::RGB888);
        CCLOG("*TexGen: %s", pageData.url.c_str());
    }
    pageView.sprite->setTexture(pageData.texture);
    
    const Size size = pageView.sprite->getTexture()->getContentSize();
    pageView.sprite->setTextureRect(Rect(Vec2::ZERO, size));
    pageView.sprite->setContentSize(size);
    
    const float w = getContentSize().width / size.width;
    const float h = getContentSize().height / size.height;
    pageView.sprite->setScale(std::min(w, h));
}

void ComicView::setPage(int32_t page){
    CC_SCOPED_CLODK("ComicView::setPage");
    
    const int32_t newPageIndex = std::max(0, std::min<int32_t>(page, (int)_pageDatas.size() - 1));
    if( _pageIndex == newPageIndex )
        return;
    _pageIndex = newPageIndex;
    
    if( _pageIndex == 0 || _pageIndex+1 == _pageDatas.size() ){
        _adjustmentTargetOffset = 0;
    }
    
    std::list<int32_t> lostIndices;
    for( const auto& pageView : _pageViews ){
        if( pageView.index >= 0 ){
            lostIndices.push_back(pageView.index);
        }
    }
    
    for( auto& pageView : _pageViews ){
        pageView.index = pageView.offsetIndex + _pageIndex;
        
        if( pageView.index >= 0 && pageView.index < _pageDatas.size() ){
            auto& pageData = _pageDatas[pageView.index];
            
            const bool ready( !pageData.initializing && pageData.image );
            if( ready ){
                pageView.sprite->setVisible(true);
                pageView.loadingNode->setVisible(false);
                updatePage(pageView, pageData);
            }else{
                if( pageData.data.empty() ){
                    if( !pageData.loading ){
                        pageData.loading = true;
                        if( FileUtils::getInstance()->isFileExist(pageData.storagePath) ){
                            _pageFileQueue.push(&pageData);
                        }else{
                            startDownload(pageData);
                        }
                    }
                }else{
                    if( !_pageImageQueue.exists(&pageData) ){
                        pageData.initializing = true;
                        _pageImageQueue.push(&pageData);
                    }
                }
                
                pageView.sprite->setVisible(false);
                pageView.loadingNode->setVisible(true);
                
                if( _attribute->onCreateLoadingNode ){
                    if( !pageView.loadingNode->getChildByTag(0xff) ){
                        auto node = _attribute->onCreateLoadingNode();
                        node->setTag(0xff);
                        pageView.loadingNode->addChild(node);
                    }
                }
            }
            
            lostIndices.remove(pageView.index);
        }else{
            // out of range
            pageView.index = -1;
            pageView.sprite->setVisible(false);
            pageView.loadingNode->setVisible(false);
        }
    }
    
    for( const int32_t lostIndex : lostIndices ){
        // 対象がいなくなっているのでグラフィックリソースを削除
        _pageDatas[lostIndex].clear();
    }
    
    if( _attribute->onUpdatePageIndex ){
        _attribute->onUpdatePageIndex(this);
    }
    
    updateSpritePosition();
}

#if COCOS2D_DEBUG > 0
ComicView* createComicViewSample(const std::vector<std::string>& urlList, bool vertical){
    auto labelPage = cocos2d::Label::createWithSystemFont("", "Helvetica", 32);
    labelPage->setPosition(cocos2d::Director::getInstance()->getWinSize().width * 0.5f, 32.0f);
    labelPage->setColor(cocos2d::Color3B::GREEN);
    
    std::unique_ptr<cocos2d::ComicView::Attribute> attr( new (std::nothrow) cocos2d::ComicView::Attribute() );
    attr->urlList = urlList;
    attr->cacheDir = "comic/";
    if( vertical ){
        attr->direction = ComicView::Direction::Vertical;
    }
    attr->onUpdatePageIndex = [labelPage](cocos2d::ComicView* sender){
        labelPage->setString(cocos2d::StringUtils::format("%d/%d", sender->getCurrentPage()+1, sender->getNumPages()));
    };
    attr->onCreateLoadingNode = [](){
        auto label = cocos2d::Label::createWithSystemFont("Loading...", "Helvetica", 48);
        label->setPositionX(-label->getContentSize().width * 0.5f);
        label->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
        cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;
        actions.pushBack(cocos2d::CallFunc::create([label](){ label->setString("Loading"); }));
        actions.pushBack(cocos2d::DelayTime::create(0.2f));
        actions.pushBack(cocos2d::CallFunc::create([label](){ label->setString("Loading."); }));
        actions.pushBack(cocos2d::DelayTime::create(0.2f));
        actions.pushBack(cocos2d::CallFunc::create([label](){ label->setString("Loading.."); }));
        actions.pushBack(cocos2d::DelayTime::create(0.2f));
        actions.pushBack(cocos2d::CallFunc::create([label](){ label->setString("Loading..."); }));
        actions.pushBack(cocos2d::DelayTime::create(0.2f));
        label->runAction(cocos2d::RepeatForever::create( cocos2d::Sequence::create(actions) ));
        return label;
    };
    
    auto view = cocos2d::ComicView::createWithAttribute(std::move(attr));
    view->addChild(labelPage);
    return view;
}
#endif

NS_CC_END
