/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCTABLEVIEW_LAMBDA_H__
#define __CCTABLEVIEW_LAMBDA_H__

#include "cocos2d.h"
#include "cocos-ext.h"


NS_CC_EXT_BEGIN

/**
 * コールバックをラムダ式で定義できる TableView
 */
class TableViewLambda
: public TableView
, public TableViewDataSource
{
public:
    // callbacks
    using OnNumberOfCellsInTableView = std::function<ssize_t()>;
    using OnTableCellSizeForIndex = std::function<cocos2d::Size(int idx)>;
    using OnTableCellAtIndex = std::function<cocos2d::extension::TableViewCell*(int idx, cocos2d::extension::TableView *sender)>;
    
    /**
     * Creates a table view with lambda.
     */
    static TableViewLambda* create(Size size,
                                   OnNumberOfCellsInTableView numberOfCellsInTableView,
                                   OnTableCellSizeForIndex tableCellSizeForIndex,
                                   OnTableCellAtIndex tableCellAtIndex);
    
    // TableViewDataSource
    virtual cocos2d::Size tableCellSizeForIndex(cocos2d::extension::TableView *table, ssize_t idx) override;
    virtual cocos2d::extension::TableViewCell* tableCellAtIndex(cocos2d::extension::TableView *table, ssize_t idx) override;
    virtual ssize_t numberOfCellsInTableView(cocos2d::extension::TableView *table) override;
    
CC_CONSTRUCTOR_ACCESS:
    TableViewLambda();
    virtual ~TableViewLambda();
	
protected:
    OnNumberOfCellsInTableView onNumberOfCellsInTableView;
    OnTableCellSizeForIndex onTableCellSizeForIndex;
    OnTableCellAtIndex onTableCellAtIndex;
};

/**
 * Cellのインスタンス管理を簡略化したコールバックを搭載
 *
 @sample
 const auto numberOfCellsInTableView = [](){
    return num;
 };
 const auto tableCellSizeForIndex = [](int idx){
    return size;
 };
 const auto tableNodeAtIndex = [](int idx, cocos2d::Node *node, cocos2d::extension::TableView *sender){
    if( !node ){
        node = Node::create()
    }
    node->setParam(...);
    return node;
 };
 auto table = cocos2d::extension::TableViewEasyLambda::create(numberOfCellsInTableView, tableCellSizeForIndex, tableNodeAtIndex);
 addChild(table)
 */
class TableViewEasyLambda
: public TableViewLambda
{
public:
    // callback
    using OnTableNodeAtIndex = std::function<cocos2d::Node*(int idx, cocos2d::Node *node, cocos2d::extension::TableView *sender)>;
    
    /**
     * Creates a table view with lambda.
     */
    static TableViewEasyLambda* create(Size size,
                                       OnNumberOfCellsInTableView numberOfCellsInTableView,
                                       OnTableCellSizeForIndex tableCellSizeForIndex,
                                       OnTableNodeAtIndex tableNodeAtIndex);
    
CC_CONSTRUCTOR_ACCESS:
    TableViewEasyLambda();
    virtual ~TableViewEasyLambda();
    
protected:
    OnTableNodeAtIndex onTableNodeAtIndex;
};

NS_CC_EXT_END

#endif
