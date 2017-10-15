/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCCOMICVIEW_H__
#define __CCCOMICVIEW_H__

#include <cocos2d.h>
#include "ExtensionMacros.h"
#include <array>


NS_CC_EXT_BEGIN

/**
 * 漫画的閲覧ビュー
 */
class ComicView
: public Node
{
public:
    
    /**
     * ページの進行方向
     */
    enum class Direction {
        /// 垂直方向
        Vertical,
         /// 水平方向
        Horizontal,
    };
    
    /**
     * 起動設定
     */
    struct Attribute {
        Attribute();
        
        /// 対象ファイルの場所リスト (StoragePath or NetworkURL)
        std::vector<std::string> urlList;
        /// ページの進行方向
        Direction direction;
        /// ダウンロードしたファイルの置き場所
        std::string cacheDir;
        /// 前後ページのキャッシュ数
        int32_t cacheRange;
        
        /// ページの自動位置補正
        bool pageAdjustment;
        /// ページ自動位置補正の速度倍率
        float pageAdjustmentSpeed;
        /// ページ自動位置補正の閾値 (0.0 - 1.0)
        float pageAdjustmentThreshold;
        /// ページ自動位置補正の最低速度
        float pageAdjustmentLowSpeed;
        /// 慣性速度の減衰率
        float inertiaDumpingForce;
        /// 両端のタッチ判定サイズ
        float edgeSize;
        /// フリック時の引っ掛かり時間
        float relationSeconds;
        /// TouchMoveでの自動送り判定の閾値 (0.0 - 1.0)
        float touchMoveAdjustmentThreshold;
        
        /// 閲覧ページの変更通知
        std::function<void(ComicView* sender)> onUpdatePageIndex;
        /// 最終ページを越えた時の通知
        std::function<void(ComicView* sender)> onPageOver;
        /// Loadingノードの生成
        std::function<Node*()> onCreateLoadingNode;
        /// 両端に引っかからずにタップ判定を得た時
        std::function<void(ComicView* sender)> onTapped;
    };
    
public:
    CREATE_FUNC(ComicView);
    ComicView();
    virtual ~ComicView();
    
    static ComicView* createWithAttribute(std::unique_ptr<Attribute> attribute);
    
    virtual bool initWithAttribute(std::unique_ptr<Attribute> attribute);
    
    // cocos2d::CCNode
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void update(float delta) override;
    virtual void setContentSize(const Size& contentSize) override;
    
    /**
     * タッチ判定の有効設定
     */
    virtual void setTouchEnabled(bool enabled);
    
    /**
     * 現在のページ番号を取得
     */
    inline int32_t getCurrentPage() const { return _pageIndex; }
    
    /**
     * 総ページ数を取得
     */
    inline int32_t getNumPages() const { return static_cast<int32_t>(_pageDatas.size()); }
    
    /**
     * ページ番号を指定
     */
    void setPage(int32_t page);
    
    /**
     * ページ番号を加算
     */
    inline void advancePage(int32_t add){
        setPage(_pageIndex + add);
    }
    
private:
    
    /**
     * スレッドセーフなキュー
     */
    template <class T>
    struct ThreadSafeQueue {
        std::list<T> _queue;
        mutable std::mutex _mutex;
        std::mutex _sleepMutex;
        std::condition_variable_any _sleepCondition;
        
        void push(T component);
        T pop();
        T pop_wait();
        bool empty() const;
        bool exists(T component) const;
        int32_t size() const;
    };
    
    /**
     * ページ情報
     */
    struct PageData {
        int32_t index;
        std::string url;
        std::string storagePath;
        std::vector<char> data;
        bool loading;
        bool initializing;
        bool http;
        Image* image;
        Texture2D* texture;
        
        ~PageData();
        
        /// グラフィックリソースの削除
        void clear();
        /// 先頭8bytesをビット反転して、OSのビューワー等でそのままでは見れないようにする
        void flipData();
    };
    
    /**
     * ページ表示情報
     */
    struct PageView {
        int32_t index;
        int32_t offsetIndex;
        Sprite* sprite;
        Node* loadingNode;
    };
    
    std::unique_ptr<Attribute> _attribute;
    
    std::vector<PageView> _pageViews;
    std::vector<PageData> _pageDatas;
    std::thread _pageFileThread;
    std::thread _pageImageThread;
    ThreadSafeQueue<PageData*> _pageFileQueue;
    ThreadSafeQueue<PageData*> _pageImageQueue;
    ThreadSafeQueue<PageData*> _pageImageReadyQueue;
    
    EventListener* _touchEvent;
    
    int32_t _pageIndex;
    bool _inertiaEnabled;
    float _inertiaSpeed;
    bool _adjustment;
    float _adjustmentTargetOffset;
    float _pageOffset;
    float _pageSize;
    float _touchLastPoint;
    float _touchMoved;
    bool _touching;
    float _touchingSeconds;
    
    std::string makePath(const std::string& url) const;
    void startDownload(PageData& pageData);
    void updatePage(PageView& pageView, PageData& pageData);
    void updateSpritePosition();
};

#if COCOS2D_DEBUG > 0
/**
 * 漫画的閲覧ビューの実装サンプル
 * @warning For only test.
 */
ComicView* createComicViewSample(const std::vector<std::string>& urlList, bool vertical, bool adjust);
#endif

NS_CC_EXT_END
#endif
