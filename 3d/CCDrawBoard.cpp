#include "CCDrawBoard.h"


NS_CC_BEGIN

DrawBoard* DrawBoard::createWithTexture(Texture2D *texture, const cocos2d::Size& size){
    DrawBoard* p = new (std::nothrow) DrawBoard();
    if( p && p->initWithTexture(texture, size) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

DrawBoard* DrawBoard::createWithFile(const std::string& filename, const cocos2d::Size& size){
    DrawBoard* p = new (std::nothrow) DrawBoard();
    if( p && p->initWithFile(filename, size) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

DrawBoard::DrawBoard()
: _texture(nullptr)
, _flippedX(false)
{}

DrawBoard::~DrawBoard(){
    CC_SAFE_RELEASE(_texture);
    CC_SAFE_RELEASE(_primitive);
}

bool DrawBoard::initWithTexture(Texture2D *texture, const cocos2d::Size& size){
    if( !Node::init() )
        return false;
    
    _dirty = true;
    
    CC_SAFE_RELEASE(_texture);
    _texture = texture;
    CC_SAFE_RETAIN(_texture);
    
    auto glProgramState = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_U_COLOR);
    setGLProgramState(glProgramState);
    
    {
        const float w = size.width * 0.5f;
        const float d = size.height;
        _vertices[0].vertices.set(+w, d, 0.0f); // RT
        _vertices[1].vertices.set(-w, d, 0.0f); // LT
        _vertices[2].vertices.set(+w, 0, 0.0f); // RB
        _vertices[3].vertices.set(-w, 0, 0.0f); // LB
        
        setTextureRect(Rect(0, 0, 1, 1));
    }
    
    _vertexBuffer = cocos2d::VertexBuffer::create(sizeof(V3F_T2F), 4);
    
    auto vd = cocos2d::VertexData::create();
    vd->setStream(_vertexBuffer, VertexStreamAttribute(0, GLProgram::VERTEX_ATTRIB_POSITION, GL_FLOAT, 3));
    vd->setStream(_vertexBuffer, VertexStreamAttribute(offsetof(V3F_T2F, texCoords), GLProgram::VERTEX_ATTRIB_TEX_COORD, GL_FLOAT, 2));
    
    _primitive = Primitive::create(vd, nullptr, GL_TRIANGLE_STRIP);
    _primitive->setStart(0);
    _primitive->setCount(4);
    _primitive->retain();
    
    return true;
}

bool DrawBoard::initWithFile(const std::string& filename, const cocos2d::Size& size){
    if( !Node::init() )
        return false;
    
    CCASSERT(filename.size()>0, "Invalid filename for sprite");
    
    if( Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename) ){
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        return initWithTexture(texture, size);
    }
    
    // don't release here.
    // when load texture failed, it's better to get a "transparent" sprite then a crashed program
    // this->release();
    return false;
}

void DrawBoard::setTextureRect(const Rect& rect){
    if( !_textureRect.equals(rect) ){
        _textureRect = rect;
        _dirty = true;
    }
}

bool DrawBoard::isFlippedX() const {
    return _flippedX;
}

void DrawBoard::setFlippedX(bool flippedX){
    if( _flippedX != flippedX ){
        _flippedX = flippedX;
    }
}

void DrawBoard::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags){
    if( _dirty ){
        _dirty = false;
        
        static const int indices0[4] = {0, 1, 2, 3};
        static const int indices1[4] = {1, 0, 3, 2};
        const int* indices = _flippedX? indices1 : indices0 ;
        _vertices[indices[0]].texCoords = Tex2F(_textureRect.getMinX(), _textureRect.getMinY());
        _vertices[indices[1]].texCoords = Tex2F(_textureRect.getMaxX(), _textureRect.getMinY());
        _vertices[indices[2]].texCoords = Tex2F(_textureRect.getMinX(), _textureRect.getMaxY());
        _vertices[indices[3]].texCoords = Tex2F(_textureRect.getMaxX(), _textureRect.getMaxY());
        _vertexBuffer->updateVertices(_vertices.data(), 4, 0);
    }
    Node::visit(renderer, parentTransform, parentFlags);
}

void DrawBoard::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags){
    getGLProgramState()->setUniformVec4("u_color", Vec4(getColor().r/0xff, getColor().g/0xff, getColor().b/0xff, getOpacity()/0xff));
    
    const auto blendFunc = _texture->hasPremultipliedAlpha()? BlendFunc::ALPHA_PREMULTIPLIED : BlendFunc::ALPHA_NON_PREMULTIPLIED ;
    _command.init(0, _texture->getName(), getGLProgramState(), blendFunc, _primitive, _modelViewTransform, flags | Node::FLAGS_RENDER_AS_3D);
//    _command.setTransparent(true);
    _command.set3D(true);
    renderer->addCommand(&_command);
}

NS_CC_END
