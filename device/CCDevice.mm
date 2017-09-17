/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDevice.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import <AVFoundation/AVFoundation.h>
#endif

NS_CC_EXT_BEGIN

std::string Device::getBundleVersion(){
    NSString* v = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
    return v? [v UTF8String] : "" ;
}

std::string Device::getBundleShortVersion(){
    NSString* v = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    return v? [v UTF8String] : "" ;
}

std::string Device::getSystemVersion(){
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return [[[UIDevice currentDevice] systemVersion] UTF8String];
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    const NSOperatingSystemVersion& v = [NSProcessInfo processInfo].operatingSystemVersion;
    return cocos2d::StringUtils::format("%ld.%ld.%ld", v.majorVersion, v.minorVersion, v.patchVersion);
#else
    return "";
#endif
}

std::string Device::getSystemName(){
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return [[UIDevice currentDevice].systemName UTF8String];
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    return [[NSHost currentHost].localizedName UTF8String];
#else
    return "";
#endif
}

bool Device::isPortrait(){
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation]);
#else
    return true;
#endif
}

uint64_t Device::getDiskFreeByates(){
    uint64_t result = 0;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSDictionary* dictionary = [[NSFileManager defaultManager] attributesOfFileSystemForPath:[paths lastObject] error:nil];
    if( dictionary ){
        result = [[dictionary objectForKey:NSFileSystemFreeSize] longLongValue];
    }
    return result;
}

void Device::clearUserDefaults(){
    NSString* bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
    [[NSUserDefaults standardUserDefaults] removePersistentDomainForName:bundleIdentifier];
}

NS_CC_EXT_END
