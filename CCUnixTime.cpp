#include "CCUnixTime.h"
#include <new>

#if COCOS2D_DEBUG > 0
#include <cocos/network/HttpClient.h>
#include <external/json/document.h>
#endif

NS_CC_EXT_BEGIN

// singleton stuff
static UnixTime *s_Instance = nullptr;

UnixTime* UnixTime::getInstance()
{
    if (!s_Instance)
    {
        s_Instance = new (std::nothrow) UnixTime();
        CCASSERT(s_Instance, "FATAL: Not enough memory");
    }
    return s_Instance;
}

UnixTime::UnixTime()
: _capturedUnixTime(0)
{}

UnixTime::~UnixTime(){
}

void UnixTime::setUnixTime(int64_t unixTime){
    _capturedTimePoint = std::chrono::steady_clock::now();
    _capturedUnixTime = unixTime;
}

int64_t UnixTime::getUnixTime() const {
    CCASSERT(_capturedUnixTime > 0, "Not initialized!");
    const auto elapsed = std::chrono::steady_clock::now() - _capturedTimePoint;
    return _capturedUnixTime + std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
}

int64_t UnixTime::getRemainingSecondsFrom(int64_t unixTime) const {
    return unixTime - getUnixTime();
}

std::unique_ptr<tm> UnixTime::getGmTime() const {
    return getGmTimeAfter(0);
}

std::unique_ptr<tm> UnixTime::getGmTimeAfter(int64_t seconds) const {
    CCASSERT(_capturedUnixTime > 0, "Not initialized!");
    const time_t unixtime = static_cast<time_t>(getUnixTime() + seconds);
    tm* p = new (std::nothrow) tm( *gmtime(&unixtime) );
    p->tm_year += 1900;
    p->tm_mon += 1;
    return std::unique_ptr<tm>(p);
}

std::unique_ptr<tm> UnixTime::getLocalTime() const {
    return getLocalTimeAfter(0);
}

std::unique_ptr<tm> UnixTime::getLocalTimeAfter(int64_t seconds) const {
    CCASSERT(_capturedUnixTime > 0, "Not initialized!");
    const time_t unixtime = static_cast<time_t>(getUnixTime() + seconds);
    tm* p = new (std::nothrow) tm( *localtime(&unixtime) );
    p->tm_year += 1900;
    p->tm_mon += 1;
    return std::unique_ptr<tm>(p);
}

#if COCOS2D_DEBUG > 0
void setupUnixTimeSample(std::function<void(UnixTime* sender)> callback){
    auto req = new (std::nothrow) network::HttpRequest();
    req->setRequestType(network::HttpRequest::Type::GET);
    req->setUrl("https://ntp-a1.nict.go.jp/cgi-bin/json");
    req->setResponseCallback([callback](network::HttpClient* client, network::HttpResponse* response){
        response->getResponseData()->push_back('\0');
        rapidjson::Document document;
        document.ParseInsitu<rapidjson::kParseDefaultFlags>( response->getResponseData()->data() );
        CCASSERT(!document.HasParseError(), StringUtils::format("JsonParseError [%d]", document.GetParseError()).c_str());
        CC_ASSERT(document.IsObject());
        const auto it = document.FindMember("st");
        if( it != document.MemberEnd() ){
            CC_ASSERT(it->value.IsDouble());
            UnixTime::getInstance()->setUnixTime( static_cast<int64_t>(it->value.GetDouble()) );
            auto tm = UnixTime::getInstance()->getLocalTime();
            CCLOG("UNIXTime %04d/%02d/%02d %02d:%02d:%02d @local (%lld)",
                  tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
                  UnixTime::getInstance()->getUnixTime());
            tm = UnixTime::getInstance()->getGmTime();
            CCLOG("UNIXTime %04d/%02d/%02d %02d:%02d:%02d @gm (%lld)",
                  tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
                  UnixTime::getInstance()->getUnixTime());
            
        }
        if( callback ){
            callback(UnixTime::getInstance());
        }
    });
    network::HttpClient::getInstance()->sendImmediate( req );
    req->release();
}
#endif

NS_CC_EXT_END
