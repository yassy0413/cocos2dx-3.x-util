#ifndef __CCDRAWBOARD_H__
#define __CCDRAWBOARD_H__

#include "cocos2d.h"
#include <array>

NS_CC_BEGIN

/**
 * 板ポリゴン
 */
class DrawBoard
: public cocos2d::Node
{
public:
    
    /**
     * Creates a board.
     */
    static DrawBoard* createWithTexture(Texture2D *texture, const cocos2d::Size& size);
    static DrawBoard* createWithFile(const std::string& filename, const cocos2d::Size& size);
    
    //
    virtual void setTextureRect(const Rect& rect);
    
    bool isFlippedX() const;
    void setFlippedX(bool flippedX);
    
    virtual bool isDirty() const { return _dirty; }
    virtual void setDirty(bool dirty) { _dirty = dirty; }
    
    // cocos2d::Node
    virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags) override;
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    
CC_CONSTRUCTOR_ACCESS:
    DrawBoard();
    virtual ~DrawBoard();
    virtual bool initWithTexture(Texture2D *texture, const cocos2d::Size& size);
    virtual bool initWithFile(const std::string& filename, const cocos2d::Size& size);
    
private:
    std::array<V3F_T2F, 4> _vertices;
    VertexBuffer* _vertexBuffer;
    PrimitiveCommand _command;
    Primitive* _primitive;
    Texture2D* _texture;
    Rect _textureRect;
    bool _flippedX;
    bool _dirty;
};

NS_CC_END

#endif
