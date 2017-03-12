#include "CCDrawGrid.h"


NS_CC_BEGIN

DrawGrid* DrawGrid::create(uint32_t detail, float size, Color4B color){
    DrawGrid* p = new (std::nothrow) DrawGrid();
    if( p && p->init(detail, size, color) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

DrawGrid::DrawGrid(){
    _primitive = nullptr;
}

DrawGrid::~DrawGrid(){
    CC_SAFE_RELEASE(_primitive);
}

bool DrawGrid::init(uint32_t detail, float size, Color4B color){
    if( !Node::init() )
        return false;
    
    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));
    
    struct V3F_C4B {
        Vec3 vertices;
        Color4B colors;
        
        V3F_C4B(const Vec3& v, const Color4B& c)
        : vertices(v), colors(c)
        {}
    };
    std::vector<V3F_C4B> vertices;
    vertices.reserve(detail * 4 + 1);
    
    {
        const float s1 = size * -0.5f;
        const float s2 = size / detail;
        const Vec3 origin(s1, s1, s1);
        const Vec3 edgesize(s2, s2, s2);
        for( uint32_t lp = 0; lp <= detail; ++lp ){
            const float x = origin.x + edgesize.x * lp;
            const float z = origin.z + edgesize.z * lp;
            
            vertices.emplace_back(Vec3(x, 0.0f, origin.z       ), color);
            vertices.emplace_back(Vec3(x, 0.0f, origin.z + size), color);
            vertices.emplace_back(Vec3(origin.x,        0.0f, z), color);
            vertices.emplace_back(Vec3(origin.x + size, 0.0f, z), color);
        }
    }
    const int numVertices = static_cast<int>(vertices.size());
    
    auto vb = cocos2d::VertexBuffer::create(sizeof(V3F_C4B), numVertices);
    vb->updateVertices(&vertices[0], numVertices, 0);
    
    auto vd = cocos2d::VertexData::create();
    vd->setStream(vb, VertexStreamAttribute(0, GLProgram::VERTEX_ATTRIB_POSITION, GL_FLOAT, 3));
    vd->setStream(vb, VertexStreamAttribute(offsetof(V3F_C4B, colors), GLProgram::VERTEX_ATTRIB_COLOR, GL_UNSIGNED_BYTE, 4, true));
    
    _primitive = Primitive::create(vd, nullptr, GL_LINES);
    _primitive->setStart(0);
    _primitive->setCount(numVertices);
    _primitive->retain();
    
    return true;
}

void DrawGrid::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags){
    flags |= Node::FLAGS_RENDER_AS_3D;
    _command.init(0, 0, getGLProgramState(), BlendFunc::ALPHA_NON_PREMULTIPLIED, _primitive, _modelViewTransform, flags);
    _command.setTransparent(false);
    _command.set3D(true);
    renderer->addCommand(&_command);
}

NS_CC_END
