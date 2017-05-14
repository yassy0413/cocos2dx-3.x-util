#include "CCCollectionView.h"
#include "CCTableViewLambda.h"

NS_CC_EXT_BEGIN

CollectionView::CollectionView()
: _tableView(nullptr)
, _numberOfItems(0)
, _numberOfItemNodes(0)
, _direction(Direction::VERTICAL)
{}

CollectionView::~CollectionView(){
}

bool CollectionView::init(){
    if( Node::init() ){
        return true;
    }
    return false;
}

void CollectionView::setNumberOfItems(int32_t num){
    _numberOfItems = num;
}

void CollectionView::setNumberOfItemsInRow(int32_t num){
    _numberOfItemsInRow = num;
}

void CollectionView::setItemSize(const Size& size){
    _itemSize = size;
}

void CollectionView::setDirection(Direction dir){
    _direction = dir;
}

void CollectionView::onEnter(){
    Node::onEnter();
    
    if( _tableView )
        return;
    
    if( getContentSize().equals(cocos2d::Size::ZERO) ){
        setContentSize(getParent()->getContentSize());
    }
    
    auto OnTableNodeAtIndex = [this](int idx, cocos2d::Node *node){
        
        if( !node ){
            node = Node::create();
        }
        
        idx *= _numberOfItemsInRow;
        
        if( _tableView->getDirection() == ScrollView::Direction::HORIZONTAL ){
            const float offset = (getContentSize().height - _itemSize.height * _numberOfItemsInRow) / (_numberOfItemsInRow + 1);
            const float unit = offset + _itemSize.height;
            for( int lp = 0; lp < _numberOfItemsInRow; ++lp ){
                auto itemNode = itemNodeAtIndex(idx, lp, node);
                itemNode->setPositionX(_itemSize.width * 0.5f);
                itemNode->setPositionY(_itemSize.height * 0.5f + offset + unit * (_numberOfItemsInRow - lp - 1));
            }
        }else{
            const float offset = (getContentSize().width - _itemSize.width * _numberOfItemsInRow) / (_numberOfItemsInRow + 1);
            const float unit = offset + _itemSize.width;
            for( int lp = 0; lp < _numberOfItemsInRow; ++lp ){
                auto itemNode = itemNodeAtIndex(idx, lp, node);
                itemNode->setPositionX(_itemSize.width * 0.5f + offset + unit * lp);
                itemNode->setPositionY(_itemSize.height * 0.5f);
            }
        }
        
        return node;
    };
    
    _tableView = TableViewEasyLambda::create(getContentSize(),
                                             [this](){ return _numberOfItemNodes; },
                                             [this](int idx){ return _itemNodeSize; },
                                             OnTableNodeAtIndex);
    reloadData();
    addChild(_tableView);
}

Node* CollectionView::itemNodeAtIndex(int idx, int localIdx, cocos2d::Node *parentNode){
    auto node0 = parentNode->getChildByTag(localIdx);
    auto node1 = onItemNodeAtIndex(idx + localIdx, node0);
    CC_ASSERT(node1);
    if( !node0 ){
        node1->setTag(localIdx);
        parentNode->addChild(node1);
    }
    return node1;
}

void CollectionView::reloadData(){
    CC_ASSERT(_tableView);
    
    _numberOfItemNodes = (_numberOfItems-1) / _numberOfItemsInRow + 1;
    
    if( _direction == Direction::VERTICAL ){
        _itemNodeSize.setSize(getContentSize().width, _itemSize.height);
        _tableView->setDirection(ScrollView::Direction::VERTICAL);
    }else{
        _itemNodeSize.setSize(_itemSize.width, getContentSize().height);
        _tableView->setDirection(ScrollView::Direction::HORIZONTAL);
    }
    
    _tableView->setVerticalFillOrder(TableView::VerticalFillOrder::TOP_DOWN);
    _tableView->reloadData();
}

#pragma mark -- sample
#if COCOS2D_DEBUG > 0

CollectionViewSample* CollectionViewSample::create(int fontSize){
    CollectionViewSample *p = new (std::nothrow) CollectionViewSample();
    if (p && p->init())
    {
        p->autorelease();
        p->_fontSize = fontSize;
        return p;
    }
    delete p;
    return nullptr;
}

void CollectionViewSample::onEnter(){
    Node::onEnter();
    
    if( getContentSize().equals(cocos2d::Size::ZERO) ){
        setContentSize(getParent()->getContentSize());
    }
    
    const int numItems = 999;
    
    auto collectionView = cocos2d::extension::CollectionView::create();
    collectionView->setNumberOfItems(numItems);
    collectionView->setNumberOfItemsInRow(5);
    collectionView->setItemSize(cocos2d::Size(100, 100));
    collectionView->onItemNodeAtIndex = [this](int idx, cocos2d::Node *node){
        auto item = (Label*)node;
        if( !item ){
            item = Label::createWithSystemFont("", "Helvetica-Bold", _fontSize);
        }
        if( idx < numItems ){
            item->setString(StringUtils::toString(idx+1));
            item->setVisible(true);
        }else{
            item->setVisible(false);
        }
        return item;
    };
    addChild(collectionView);
}
#endif

NS_CC_EXT_END
