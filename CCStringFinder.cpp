/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCStringFinder.h"
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>


NS_CC_EXT_BEGIN

StringFinder* StringFinder::createWithStrings(const std::vector<std::string>& textList){
    StringFinder* pRet = new (std::nothrow) StringFinder();
    if( pRet && pRet->init() ){
        pRet->autorelease();
        pRet->setStrings(textList);
        return pRet;
    }
    delete pRet;
    return nullptr;
}

StringFinder* StringFinder::createWithStrings(std::vector<std::string>&& textList){
    StringFinder* pRet = new (std::nothrow) StringFinder();
    if( pRet && pRet->init() ){
        pRet->autorelease();
        pRet->setStrings(std::move(textList));
        return pRet;
    }
    delete pRet;
    return nullptr;
}

StringFinder::StringFinder()
: onStringSelectCallback(nullptr)
, _tableView(nullptr)
, _editBox(nullptr)
{
    _fontSize = cocos2d::Director::getInstance()->getWinSize().width / 20;
    _cellHeight = _fontSize + 16;
    setContentSize( cocos2d::Director::getInstance()->getWinSize() );
}

StringFinder::~StringFinder(){
}

void StringFinder::onEnter(){
    Node::onEnter();
    
    auto layer = cocos2d::LayerColor::create(cocos2d::Color4B(0, 0, 0, 100), getContentSize().width, getContentSize().height);
    addChild( layer );
    
    // Touch Guard
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches( true );
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event* event){ return isVisible(); };
    listener->onTouchMoved =
    listener->onTouchEnded =
    listener->onTouchCancelled = [this](cocos2d::Touch* touch, cocos2d::Event* event){};
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    //
    _tableView = TableView::create( this, getContentSize() );
    _tableView->setDelegate( this );
    _tableView->setDirection( TableView::Direction::VERTICAL );
    _tableView->setVerticalFillOrder( TableView::VerticalFillOrder::TOP_DOWN );
    _tableView->setBounceable( true );
    addChild( _tableView );
}

void StringFinder::onExit(){
    Node::onExit();
}

#pragma mark -- TableViewDataSource

Size StringFinder::tableCellSizeForIndex(TableView *table, ssize_t idx){
    return Size( getContentSize().width, _cellHeight );
}

TableViewCell* StringFinder::tableCellAtIndex(TableView *table, ssize_t idx){
    
    auto cell = (TableViewCell*)table->dequeueCell();
    if( !cell ){
        cell = new TableViewCell();
        cell->autorelease();
    }
    
    const Size s( getContentSize().width, _cellHeight );
    
    //
    auto container = (Label*)cell->getChildByTag( 0 );
    if( !container ){
        container = Label::createWithSystemFont("", "Arial", _fontSize);
        container->setAnchorPoint( Point::ANCHOR_MIDDLE );
        container->setPosition( Point(s.width * 0.5f, s.height * 0.5f) );
        container->setColor( Color3B::WHITE );
        container->setDimensions( s.width, s.height );
        container->setHorizontalAlignment( TextHAlignment::LEFT );
        container->setTag( 0 );
        cell->addChild( container );
    }
    
    //
    cell->setUserData( (void*)_filteredStrings[idx].c_str() );
    container->setString( _filteredStrings[idx] );
    container->setPositionY( container->getContentSize().height * 0.5f - 10 );
    
    return cell;
}

ssize_t StringFinder::numberOfCellsInTableView(TableView *table){
    return _filteredStrings.size();
}

#pragma mark -- TableViewDelegate

void StringFinder::tableCellTouched(TableView* table, TableViewCell* cell){
    
    if( !isVisible() )
        return;
    
    const auto string = static_cast<const char*>( cell->getUserData() );
    if( onStringSelectCallback ){
        onStringSelectCallback( this, string );
    }
}

#pragma mark -- EditBoxDelegate

void StringFinder::editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox){
}

void StringFinder::editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox){
    
}

void StringFinder::editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text){
    _editText = text;
    setFilter( _editText );
}

void StringFinder::editBoxReturn(cocos2d::ui::EditBox* editBox){
    
}

#pragma mark --

void StringFinder::setStrings(const std::vector<std::string>& textList){
    _strings = textList;
    setFilter("");
}
void StringFinder::setStrings(std::vector<std::string>&& textList){
    _strings = std::move(textList);
    setFilter("");
}

void StringFinder::setFilter(const std::string& text){
    if( text.empty() ){
        _filteredStrings = _strings;
    }else{
        _filteredStrings.clear();
        std::string v1(text);
        std::transform( v1.begin(), v1.end(), v1.begin(), tolower );
        for( const auto& string : _strings ){
            std::string v0(string);
            std::transform( v0.begin(), v0.end(), v0.begin(), tolower );
            if( v0.find(v1) != std::string::npos ){// 部分一致
                _filteredStrings.push_back(string);
            }
        }
    }
    if( _tableView ){
        _tableView->reloadData();
    }
    
    _editText = text;
    if( _editBox ){
        if( _editText != _editBox->getText() ){
            _editBox->setText( text.c_str() );
        }
    }
}

void StringFinder::enableFilterInputForm(cocos2d::ui::Scale9Sprite* sprite, float height){
    
    CC_ASSERT(!_editBox);
    _editBox = cocos2d::ui::EditBox::create( cocos2d::Size(getContentSize().width, height), sprite );
    _editBox->setDelegate( this );
    _editBox->setPosition( cocos2d::Vec2( getContentSize().width * 0.5f, height * 0.5f ) );
    _editBox->setFontColor( cocos2d::Color3B::RED );
    _editBox->setFontSize( height );
    addChild( _editBox );
    
    auto listener = cocos2d::EventListenerKeyboard::create();
    listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keycode, cocos2d::Event* event){
        switch( keycode ){
            case cocos2d::EventKeyboard::KeyCode::KEY_A:
            case cocos2d::EventKeyboard::KeyCode::KEY_B:
            case cocos2d::EventKeyboard::KeyCode::KEY_C:
            case cocos2d::EventKeyboard::KeyCode::KEY_D:
            case cocos2d::EventKeyboard::KeyCode::KEY_E:
            case cocos2d::EventKeyboard::KeyCode::KEY_F:
            case cocos2d::EventKeyboard::KeyCode::KEY_G:
            case cocos2d::EventKeyboard::KeyCode::KEY_H:
            case cocos2d::EventKeyboard::KeyCode::KEY_I:
            case cocos2d::EventKeyboard::KeyCode::KEY_J:
            case cocos2d::EventKeyboard::KeyCode::KEY_K:
            case cocos2d::EventKeyboard::KeyCode::KEY_L:
            case cocos2d::EventKeyboard::KeyCode::KEY_M:
            case cocos2d::EventKeyboard::KeyCode::KEY_N:
            case cocos2d::EventKeyboard::KeyCode::KEY_O:
            case cocos2d::EventKeyboard::KeyCode::KEY_P:
            case cocos2d::EventKeyboard::KeyCode::KEY_Q:
            case cocos2d::EventKeyboard::KeyCode::KEY_R:
            case cocos2d::EventKeyboard::KeyCode::KEY_S:
            case cocos2d::EventKeyboard::KeyCode::KEY_T:
            case cocos2d::EventKeyboard::KeyCode::KEY_U:
            case cocos2d::EventKeyboard::KeyCode::KEY_V:
            case cocos2d::EventKeyboard::KeyCode::KEY_W:
            case cocos2d::EventKeyboard::KeyCode::KEY_X:
            case cocos2d::EventKeyboard::KeyCode::KEY_Y:
            case cocos2d::EventKeyboard::KeyCode::KEY_Z:
                _editText += 'a' + (static_cast<int>(keycode) - static_cast<int>(cocos2d::EventKeyboard::KeyCode::KEY_A));
                setFilter( _editText );
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_0:
            case cocos2d::EventKeyboard::KeyCode::KEY_1:
            case cocos2d::EventKeyboard::KeyCode::KEY_2:
            case cocos2d::EventKeyboard::KeyCode::KEY_3:
            case cocos2d::EventKeyboard::KeyCode::KEY_4:
            case cocos2d::EventKeyboard::KeyCode::KEY_5:
            case cocos2d::EventKeyboard::KeyCode::KEY_6:
            case cocos2d::EventKeyboard::KeyCode::KEY_7:
            case cocos2d::EventKeyboard::KeyCode::KEY_8:
            case cocos2d::EventKeyboard::KeyCode::KEY_9:
                _editText += '0' + (static_cast<int>(keycode) - static_cast<int>(cocos2d::EventKeyboard::KeyCode::KEY_0));
                setFilter( _editText );
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_DELETE:
            case cocos2d::EventKeyboard::KeyCode::KEY_BACKSPACE:
                if(!_editText.empty()){
                    _editText.erase(--_editText.end());
                    setFilter( _editText );
                }
                break;
            default:
                break;
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

NS_CC_EXT_END
