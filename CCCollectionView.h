/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCCOLLECTIONVIEW_H__
#define __CCCOLLECTIONVIEW_H__

#include <cocos2d.h>
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN
class TableViewEasyLambda;

/**
 * 格子状にアイテムを配置するビュー
 */
class CollectionView
: public Node
{
public:
    
    /**
     * CollectionView scroll direction type.
     */
    enum class Direction
    {
        VERTICAL,
        HORIZONTAL
    };
    
    /// callback
    using OnItemNodeAtIndex = std::function<cocos2d::Node*(int idx, cocos2d::Node *node)>;
    
public:
    CREATE_FUNC(CollectionView);
    
    virtual void onEnter() override;
    
    /**
     * アイテムの初期化、もしくは更新のコールバック
     */
    OnItemNodeAtIndex onItemNodeAtIndex;
    
    /**
     * 全体のアイテム数を設定
     */
    void setNumberOfItems(int32_t num);
    
    /**
     * 一行あたりのアイテム数を設定
     */
    void setNumberOfItemsInRow(int32_t num);
    
    /**
     * アイテム１つ分のサイズ
     */
    void setItemSize(const Size& size);
    
    /**
     * スクロールする方向
     */
    void setDirection(Direction dir);
    
    /**
     * データの再読込
     */
    void reloadData();
    
CC_CONSTRUCTOR_ACCESS:
    
    CollectionView();
    virtual ~CollectionView();
    
    virtual bool init() override;
    
private:
    TableViewEasyLambda* _tableView;
    int32_t _numberOfItems;
    int32_t _numberOfItemsInRow;
    int32_t _numberOfItemNodes;
    Size _itemNodeSize;
    Size _itemSize;
    Direction _direction;
    
    Node* itemNodeAtIndex(int idx, int localIdx, cocos2d::Node *parentNode);
};

#if COCOS2D_DEBUG > 0
/**
 * CollectionViewの実装サンプル
 */
class CollectionViewSample
: public Node
{
public:
    static CollectionViewSample* create(int fontSize = 32);
    virtual void onEnter() override;
private:
    int _fontSize;
};
#endif

NS_CC_EXT_END
#endif
