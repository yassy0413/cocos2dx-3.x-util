#ifndef __CCDRAWGRID_H__
#define __CCDRAWGRID_H__

#include "cocos2d.h"

NS_CC_BEGIN

/**
 * グリッド描画
 */
class DrawGrid
: public cocos2d::Node
{
public:
    
    /**
     * Creates a grid.
     */
    static DrawGrid* create(uint32_t detail, float size, Color4B color = Color4B::WHITE);
    
    // cocos2d::Node
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    
CC_CONSTRUCTOR_ACCESS:
    DrawGrid();
    virtual ~DrawGrid();
    virtual bool init(uint32_t detail, float size, Color4B color);
    
private:
    PrimitiveCommand _command;
    Primitive* _primitive;
};

NS_CC_END

#endif
