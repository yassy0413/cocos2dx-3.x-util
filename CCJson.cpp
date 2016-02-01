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
    if( p->initFromStr(str) ){
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

bool Json::initFromStr(const char* str){
    delete _document;
    _document = new (std::nothrow) rapidjson::Document();
    if( _document->Parse<rapidjson::kParseDefaultFlags>(str).HasParseError() ){
        CCLOG("JsonParseError [%s]", _document->GetParseError());
        return false;
    }
    return true;
}

bool Json::initFromValue(const Value& value){
    
    static const std::function<void(rapidjson::Document& document, const Value& in, rapidjson::Value& out)> convert[] = {
        // NONE
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetNull();
        },
        // BYTE
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetInt( in.asByte() );
        },
        // INTEGER
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetInt( in.asInt() );
        },
        // FLOAT
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetDouble( in.asFloat() );
        },
        // DOUBLE
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetDouble( in.asDouble() );
        },
        // BOOLEAN
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetBool( in.asBool() );
        },
        // STRING
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetString( in.asString().c_str(), static_cast<rapidjson::SizeType>(in.asString().length()) );
        },
        // VECTOR
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetArray();
            for( const auto& vv : in.asValueVector() ){
                rapidjson::Value v;
                convert[ static_cast<int>(vv.getType()) ]( document, vv, v );
                out.PushBack( v, document.GetAllocator() );
            }
        },
        // MAP
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
            out.SetObject();
            for( const auto& vv : in.asValueMap() ){
                rapidjson::Value v;
                convert[ static_cast<int>(vv.second.getType()) ]( document, vv.second, v );
                out.AddMember( vv.first.c_str(), v, document.GetAllocator() );
            }
        },
        // INT_KEY_MAP
        [](rapidjson::Document& document, const Value& in, rapidjson::Value& out){
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

const char* Json::getPrettyString(){
    CC_ASSERT(_document);
    
    delete _stringBuffer;
    _stringBuffer = new (std::nothrow) rapidjson::StringBuffer();
    
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( *_stringBuffer );
    writer.SetIndent( ' ', 2 );
    _document->Accept( writer );
    
    return _stringBuffer->GetString();
}

Value Json::getValue() const {
    CC_ASSERT(_document);
    
    static std::function<Value(const rapidjson::Value& in)> convert[] = {
        // kNullType
        [](const rapidjson::Value& in){
            return Value();
        },
        // kFalseType
        [](const rapidjson::Value& in){
            return Value(false);
        },
        // kTrueType
        [](const rapidjson::Value& in){
            return Value(true);
        },
        // kObjectType
        [](const rapidjson::Value& in){
            ValueMap result;
            for( rapidjson::Value::ConstMemberIterator it = in.MemberonBegin(); it != in.MemberonEnd(); ++it ){
                result.emplace( it->name.GetString(), convert[ it->value.GetType() ]( it->value ) );
            }
            return Value( std::move(result) );
        },
        // kArrayType
        [](const rapidjson::Value& in){
            ValueVector result;
            const rapidjson::SizeType size = in.Size();
            result.reserve( size );
            for( rapidjson::SizeType lp = 0; lp < size; ++lp ){
                result.emplace_back( convert[ in.GetType() ]( in[lp] ) );
            }
            return Value( std::move(result) );
        },
        // kStringType
        [](const rapidjson::Value& in){
            return Value( in.GetString() );
        },
        // kNumberType
        [](const rapidjson::Value& in){
            return in.IsDouble()? Value(in.GetDouble()) : Value(in.GetInt()) ;
        },
    };
    return convert[ _document->GetType() ]( *_document );
}

NS_CC_EXT_END
