#include "CCTableViewLambda.h"

NS_CC_EXT_BEGIN

#pragma mark -- TableViewLambda

TableViewLambda* TableViewLambda::create(Size size,
                                         OnNumberOfCellsInTableView numberOfCellsInTableView,
                                         OnTableCellSizeForIndex tableCellSizeForIndex,
                                         OnTableCellAtIndex tableCellAtIndex)
{
    auto pRet = new (std::nothrow) TableViewLambda();
    if( pRet && pRet->initWithViewSize(size) ){
        pRet->autorelease();
        pRet->onNumberOfCellsInTableView = numberOfCellsInTableView;
        pRet->onTableCellSizeForIndex = tableCellSizeForIndex;
        pRet->onTableCellAtIndex = tableCellAtIndex;
        pRet->setDataSource(pRet);
        pRet->_updateCellPositions();
        pRet->_updateContentSize();
        return pRet;
    }
    delete pRet;
    return nullptr;
}


TableViewLambda::TableViewLambda()
: onNumberOfCellsInTableView(nullptr)
, onTableCellSizeForIndex(nullptr)
, onTableCellAtIndex(nullptr)
{}

TableViewLambda::~TableViewLambda(){
}


#pragma mark -- TableViewDataSource

ssize_t TableViewLambda::numberOfCellsInTableView(cocos2d::extension::TableView *table){
    CC_ASSERT( onNumberOfCellsInTableView );
    return onNumberOfCellsInTableView();
}

cocos2d::Size TableViewLambda::tableCellSizeForIndex(cocos2d::extension::TableView *table, ssize_t idx){
    CC_ASSERT( onNumberOfCellsInTableView );
    return onTableCellSizeForIndex(idx);
}

cocos2d::extension::TableViewCell* TableViewLambda::tableCellAtIndex(cocos2d::extension::TableView *table, ssize_t idx){
    CC_ASSERT( onNumberOfCellsInTableView );
    return onTableCellAtIndex(idx, table);
}


#pragma mark -- TableViewEasyLambda

TableViewEasyLambda* TableViewEasyLambda::create(Size size,
                                                 OnNumberOfCellsInTableView numberOfCellsInTableView,
                                                 OnTableCellSizeForIndex tableCellSizeForIndex,
                                                 OnTableNodeAtIndex tableNodeAtIndex)
{
    auto pRet = new (std::nothrow) TableViewEasyLambda();
    if( pRet && pRet->initWithViewSize(size) ){
        pRet->autorelease();
        pRet->onNumberOfCellsInTableView = numberOfCellsInTableView;
        pRet->onTableCellSizeForIndex = tableCellSizeForIndex;
        pRet->onTableNodeAtIndex = tableNodeAtIndex;
        pRet->setDataSource(pRet);
        pRet->_updateCellPositions();
        pRet->_updateContentSize();
        return pRet;
    }
    delete pRet;
    return nullptr;
}


TableViewEasyLambda::TableViewEasyLambda()
: onTableNodeAtIndex(nullptr)
{
    onTableCellAtIndex = [this](int idx, TableView *table){
        CC_ASSERT(onTableNodeAtIndex);
        
        auto cell = (TableViewCell*)table->dequeueCell();
        if( !cell ){
            cell = new (std::nothrow) TableViewCell();
            cell->autorelease();
        }
        
        auto container = (Node*)cell->getUserData();
        if( !container ){
            container = onTableNodeAtIndex(idx, nullptr);
            cell->setUserData( container );
            cell->addChild( container );
        }else{
            onTableNodeAtIndex(idx, container);
        }
        
        return cell;
    };
}

TableViewEasyLambda::~TableViewEasyLambda(){
}


NS_CC_EXT_END
