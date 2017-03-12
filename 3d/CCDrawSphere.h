#ifndef __CCDRAWSPHERE_H__
#define __CCDRAWSPHERE_H__

#include "cocos2d.h"

NS_CC_BEGIN

/**
 * 球体描画
 */
class DrawSphere
: public cocos2d::Node
{
public:
    
    /**
     * Creates a wired sphere.
     */
    static DrawSphere* create(uint32_t detail, float radius, Color4B color = Color4B::GREEN);
    
    // cocos2d::Node
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    
CC_CONSTRUCTOR_ACCESS:
    DrawSphere();
    virtual ~DrawSphere();
    virtual bool init(uint32_t detail, float radius, Color4B color);
    
private:
    PrimitiveCommand _command[2];
    Primitive* _primitive[2];
};

NS_CC_END

#endif
