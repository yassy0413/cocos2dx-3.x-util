/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CC_STRING_FINDER_H__
#define __CC_STRING_FINDER_H__

#include "cocos2d.h"
#include "cocos-ext.h"


NS_CC_EXT_BEGIN

/*
 * 文字列選択
 */
class StringFinder
: public Node
, public TableViewDataSource
, public TableViewDelegate
, public cocos2d::ui::EditBoxDelegate
{
public:
    CREATE_FUNC(StringFinder);
    StringFinder();
    virtual ~StringFinder();
    
    /**
     * 文字列リストを設定してオブジェクトを作成
     */
    static StringFinder* createWithStrings(const std::vector<std::string>& textList);
    static StringFinder* createWithStrings(std::vector<std::string>&& textList);
    
    // Node
    virtual void onEnter() override;
    virtual void onExit() override;
    
    // TableViewDataSource
    virtual Size tableCellSizeForIndex(extension::TableView *table, ssize_t idx) override;
    virtual extension::TableViewCell* tableCellAtIndex(extension::TableView *table, ssize_t idx) override;
    virtual ssize_t numberOfCellsInTableView(extension::TableView *table) override;
    
    // TableViewDelegate
    virtual void tableCellTouched(extension::TableView* table, extension::TableViewCell* cell) override;
    
    // EditBoxDelegate
    virtual void editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text) override;
    virtual void editBoxReturn(cocos2d::ui::EditBox* editBox) override;
    
public:
    
    /**
     * 文字列リストの設定
     */
    void setStrings(const std::vector<std::string>& textList);
    void setStrings(std::vector<std::string>&& textList);
    
    /**
     * セルの高さを設定
     */
    void setCellHeight(float height){ _cellHeight = height; }
    
    /**
     * フォントのサイズを設定
     */
    void setFontSize(float size){ _fontSize = size; }
    
    /**
     * 表示対象にフィルターをかける
     */
    void setFilter(const std::string& text);
    
    /**
     * 表示対象にかけるフィルターを編集するフォームを作成する
     */
    void enableFilterInputForm(cocos2d::ui::Scale9Sprite* sprite, float height);
    
    /**
     * 文字列が選択された時のコールバック
     */
    using stringSelectCallback = std::function<void(StringFinder* sender, const char* text)>;
    stringSelectCallback onStringSelectCallback;
    
private:
    TableView* _tableView;
    float _cellHeight;
    float _fontSize;
    
    cocos2d::ui::EditBox* _editBox;
    std::string _editText;
    
    std::vector<std::string> _strings;
    std::vector<std::string> _filteredStrings;
};

NS_CC_EXT_END

#endif