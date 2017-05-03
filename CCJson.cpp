/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCJson.h"
#include <sstream>
#include <iomanip>
#include <external/json/writer.h>
#include <external/json/prettywriter.h>


NS_CC_EXT_BEGIN

Json* Json::createFromStr(const char* str){
    auto p = new (std::nothrow) Json();
    if( p->initFromStr(str, false) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

Json* Json::createFromStrInsitu(const char* str){
    auto p = new (std::nothrow) Json();
    if( p->initFromStr(str, true) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

Json* Json::createFromValue(const Value& value){
    auto p = new (std::nothrow) Json();
    if( p->initFromValue(value) ){
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

Json::Json()
: _document(nullptr)
, _stringBuffer(nullptr)
{}

Json::~Json(){
    delete _stringBuffer;
    delete _document;
}

bool Json::initFromStr(const char* str, bool insitu){
    delete _document;
    _document = new (std::nothrow) rapidjson::Document();
    
    if( insitu ){
        _document->ParseInsitu<rapidjson::kParseDefaultFlags>(const_cast<char*>(str));
    }else{
        _document->Parse<rapidjson::kParseDefaultFlags>(str);
    }
    
    if( _document->HasParseError() ){
        CCLOG("JsonParseError [%d]", _document->GetParseError());
        return false;
    }
    return true;
}

bool Json::initFromValue(const Value& value){
    
    static const std::function<void(rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept> convert[] = {
        // NONE
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetNull();
        },
        // BYTE
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetInt( in.asByte() );
        },
        // INTEGER
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetInt( in.asInt() );
        },
        // UNSIGNED
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetUint( in.asUnsignedInt() );
        },
        // FLOAT
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetDouble( in.asFloat() );
        },
        // DOUBLE
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetDouble( in.asDouble() );
        },
        // BOOLEAN
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetBool( in.asBool() );
        },
        // STRING
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            const std::string v( in.asString() );
            out.SetString( v.c_str(), static_cast<rapidjson::SizeType>(v.length()), document.GetAllocator() );
        },
        // VECTOR
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetArray();
            for( const auto& vv : in.asValueVector() ){
                rapidjson::Value v;
                convert[ static_cast<int>(vv.getType()) ]( document, vv, v );
                out.PushBack( std::move(v), document.GetAllocator() );
            }
        },
        // MAP
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            out.SetObject();
            for( const auto& vv : in.asValueMap() ){
                rapidjson::Value v;
                convert[ static_cast<int>(vv.second.getType()) ]( document, vv.second, v );
                out.AddMember( rapidjson::StringRef(vv.first.c_str()), std::move(v), document.GetAllocator() );
            }
        },
        // INT_KEY_MAP
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out) noexcept {
            CCASSERT(0, "Not supported.");
        },
    };

//    std::vector<char> buffer;
//    buffer.resize( 4*1024*1024 );
//    rapidjson::MemoryPoolAllocator<> allocator( &buffer.front(), buffer.size() );
    
    delete _document;
    _document = new (std::nothrow) rapidjson::Document();
    convert[ static_cast<int>(value.getType()) ]( *_document, value, *_document );
    
    return true;
}

const char* Json::getString(){
    CC_ASSERT(_document);
    
    delete _stringBuffer;
    _stringBuffer = new (std::nothrow) rapidjson::StringBuffer();
    
    rapidjson::Writer<rapidjson::StringBuffer> writer( *_stringBuffer );
    _document->Accept( writer );
    
    return _stringBuffer->GetString();
}

const char* Json::getPrettyString(char indentChar, uint32_t indentCharCount){
    CC_ASSERT(_document);
    
    delete _stringBuffer;
    _stringBuffer = new (std::nothrow) rapidjson::StringBuffer();
    
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( *_stringBuffer );
    writer.SetIndent( indentChar, indentCharCount );
    _document->Accept( writer );
    
    return _stringBuffer->GetString();
}

Value Json::getValue() const {
    CC_ASSERT(_document);
    
    static const std::function<Value(const rapidjson::Value& in) noexcept> convert[] = {
        // kNullType
        [](const rapidjson::Value& in) noexcept {
            return Value();
        },
        // kFalseType
        [](const rapidjson::Value& in) noexcept {
            return Value(false);
        },
        // kTrueType
        [](const rapidjson::Value& in) noexcept {
            return Value(true);
        },
        // kObjectType
        [](const rapidjson::Value& in) noexcept {
            ValueMap result;
            for( auto it = in.MemberBegin(); it != in.MemberEnd(); ++it ){
                const auto& value = it->value;
                result.emplace( it->name.GetString(), convert[ value.GetType() ]( value ) );
            }
            return Value( std::move(result) );
        },
        // kArrayType
        [](const rapidjson::Value& in) noexcept {
            ValueVector result;
            const rapidjson::SizeType size = in.Size();
            result.reserve( size );
            for( rapidjson::SizeType lp = 0; lp < size; ++lp ){
                const auto& value = in[lp];
                result.emplace_back( convert[ value.GetType() ]( value ) );
            }
            return Value( std::move(result) );
        },
        // kStringType
        [](const rapidjson::Value& in) noexcept {
            return Value( in.GetString() );
        },
        // kNumberType
        [](const rapidjson::Value& in) noexcept {
            return in.IsDouble()? Value(in.GetDouble()) : (in.IsUint()? Value(in.GetUint()) : Value(in.GetInt())) ;
        },
    };
    return convert[ _document->GetType() ]( *_document );
}

NS_CC_EXT_END
