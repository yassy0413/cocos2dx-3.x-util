#include "CCDrawSphere.h"


NS_CC_BEGIN

struct CircleTable {
    std::vector<float> sint1, sint2, cost1, cost2;
    
    CircleTable(uint32_t slices, uint32_t stacks){
        build(sint1, cost1, -static_cast<int32_t>(slices));
        build(sint2, cost2, stacks * 2);
    }
    
private:
    void build(std::vector<float>& sinT, std::vector<float>& cosT, int32_t n) const noexcept {
        const uint32_t size  = static_cast<uint32_t>( fabsf( static_cast<float>(n) ) );
        const float angle = 2.0f * M_PI / (float)((n==0)? 1 : n);
        
        sinT.resize(size + 1);
        cosT.resize(size + 1);
        
        sinT[0] = 0.0;
        cosT[0] = 1.0;
        
        for( uint32_t lp = 1; lp < size; ++lp ){
            const float v = angle * lp;
            sinT[lp] = sinf(v);
            cosT[lp] = cosf(v);
        }
        
        sinT[size] = sinT[0];
        cosT[size] = cosT[0];
    }
};


DrawSphere* DrawSphere::create(uint32_t detail, float radius, Color4B color){
    DrawSphere* p = new (std::nothrow) DrawSphere();
    if( p && p->init(detail, radius, color) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

DrawSphere::DrawSphere(){
    _primitive[0] = nullptr;
    _primitive[1] = nullptr;
}

DrawSphere::~DrawSphere(){
    CC_SAFE_RELEASE(_primitive[0]);
    CC_SAFE_RELEASE(_primitive[1]);
}

bool DrawSphere::init(uint32_t detail, float radius, Color4B color){
    if( !Node::init() )
        return false;
    
    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));
    
    CircleTable circletable(detail, detail);
    
    struct V3F_C4B {
        Vec3 vertices;
        Color4B colors;
        
        V3F_C4B(const Vec3& v, const Color4B& c)
        : vertices(v), colors(c)
        {}
    };
    std::vector<V3F_C4B> vertices;
    vertices.reserve(detail * (detail+1));
    
    const std::function<void()> setupVertices[2] =
    {
        [&](){
            vertices.clear();
            for( uint32_t lp = 1; lp <= detail; ++lp ){
                const float z = circletable.cost2[lp];
                const float r = circletable.sint2[lp];
                
                for( uint32_t lpp = 0; lpp <= detail; ++lpp ){
                    const float x = circletable.cost1[lpp];
                    const float y = circletable.sint1[lpp];
                    
                    vertices.emplace_back(cocos2d::Vec3(x*r, y*r, z) * radius, color);
                }
            }
        },
        [&](){
            vertices.clear();
            for( uint32_t lp = 1; lp <= detail; ++lp ){
                for( uint32_t lpp = 0; lpp <= detail; ++lpp ){
                    const float x = circletable.cost1[lp] * circletable.sint2[lpp];
                    const float y = circletable.sint1[lp] * circletable.sint2[lpp];
                    const float z = circletable.cost2[lpp];
                    
                    vertices.emplace_back(cocos2d::Vec3(x, y, z) * radius, color);
                }
            }
        }
    };
    
    const int drawtype[2] = {GL_LINE_LOOP, GL_LINE_STRIP};
    
    for( int lp = 0; lp < 2; ++lp ){
        
        setupVertices[lp]();
        const int numVertices = static_cast<int>(vertices.size());
        
        auto vb = cocos2d::VertexBuffer::create(sizeof(V3F_C4B), numVertices);
        vb->updateVertices(&vertices[0], numVertices, 0);
        
        auto vd = cocos2d::VertexData::create();
        vd->setStream(vb, VertexStreamAttribute(0, GLProgram::VERTEX_ATTRIB_POSITION, GL_FLOAT, 3));
        vd->setStream(vb, VertexStreamAttribute(offsetof(V3F_C4B, colors), GLProgram::VERTEX_ATTRIB_COLOR, GL_UNSIGNED_BYTE, 4, true));
        
        _primitive[lp] = Primitive::create(vd, nullptr, drawtype[lp]);
        _primitive[lp]->setStart(0);
        _primitive[lp]->setCount(numVertices);
        _primitive[lp]->retain();
    }
    return true;
}

void DrawSphere::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags){
    flags |= Node::FLAGS_RENDER_AS_3D;
    for( int lp = 0; lp < 2; ++lp ){
        _command[lp].init(0, 0, getGLProgramState(), BlendFunc::ALPHA_NON_PREMULTIPLIED, _primitive[lp], _modelViewTransform, flags);
        _command[lp].setTransparent(true);
        _command[lp].set3D(true);
        renderer->addCommand(&_command[lp]);
    }
}

NS_CC_END
