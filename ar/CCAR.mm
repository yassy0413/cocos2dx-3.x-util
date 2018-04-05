/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCAR.h"
#include "../device/CCDevice.h"

#import <AVFoundation/AVFoundation.h>
#import <ARKit/ARKit.h>
#import <Endian.h>

API_AVAILABLE(ios(11.0)) API_UNAVAILABLE(macos, watchos, tvos)
@interface CCAR : NSObject <ARSessionDelegate>
@property (retain, nonatomic) ARSession* session;
@property (retain, nonatomic) ARWorldTrackingConfiguration* configuration;
@end

@implementation CCAR

-(id)init {
    _configuration = [[ARWorldTrackingConfiguration new] autorelease];
    _configuration.planeDetection = ARPlaneDetectionHorizontal;
    
    _session = [[ARSession new] autorelease];
    _session.delegate = self;
    [_session runWithConfiguration:_configuration];
    return self;
}

- (void)dealloc {
    [super dealloc];
}

- (void)pause {
    [_session pause];
}

- (void)resume {
    [_session runWithConfiguration:_configuration];
}

#pragma mark - ARSessionDelegate

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame {
    // kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
    CVPixelBufferLockBaseAddress(frame.capturedImage, kCVPixelBufferLock_ReadOnly);
    const int width = (int)CVPixelBufferGetWidth(frame.capturedImage);
    const int height = (int)CVPixelBufferGetHeight(frame.capturedImage);
    
    static uint32_t* rgba = nullptr;
    if( !rgba ){
        rgba = (uint32_t*)malloc(4 * width * height);
    }
    {// YCbCr_420_SP(NV21) -> RGBA
        //@see https://issuetracker.google.com/issues/36905470
        const uint8_t* yuv = reinterpret_cast<const uint8_t*>( CVPixelBufferGetBaseAddressOfPlane(frame.capturedImage, 0) );
        
        const int frameSize = width * height;
        
        for( int j = 0, yp = 0; j < height; ++j ){
            int uvp = frameSize + (j >> 1) * width;
            int u = 0;
            int v = 0;
            for( int i = 0; i < width; ++i, ++yp ){
                int y = yuv[yp] - 16;
                if (y < 0) y = 0;
                if ((i & 1) == 0) {
                    v = yuv[uvp++] - 128;
                    u = yuv[uvp++] - 128;
                }
                
                const int y1192 = 1192 * y;
                int r = (y1192 + 1634 * v);
                int g = (y1192 - 833 * v - 400 * u);
                int b = (y1192 + 2066 * u);
                
                if (r < 0) r = 0; else if (r > 262143) r = 262143;
                if (g < 0) g = 0; else if (g > 262143) g = 262143;
                if (b < 0) b = 0; else if (b > 262143) b = 262143;
                
                rgba[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
            }
        }
    }
    
    CVPixelBufferUnlockBaseAddress(frame.capturedImage, kCVPixelBufferLock_ReadOnly);
    cocos2d::extension::AR::getInstance()->applyImage(rgba, width, height);
    
    // デバイスカメラの姿勢を取得
    const auto& c = frame.camera.transform.columns;
    const cocos2d::Mat4 m(c[0][0], c[1][0], c[2][0], c[3][0],
                          c[0][1], c[1][1], c[2][1], c[3][1],
                          c[0][2], c[1][2], c[2][2], c[3][2],
                          c[0][3], c[1][3], c[2][3], c[3][3]);
    m.getTranslation(&cocos2d::extension::AR::getInstance()->getPosition());
    
    if( cocos2d::extension::Device::isPortrait() ){
        cocos2d::Quaternion q0, q1(m);
        cocos2d::Quaternion::createFromAxisAngle(cocos2d::Vec3::UNIT_Z, M_PI_2, &q0);
        cocos2d::extension::AR::getInstance()->getQuat() = q1 * q0;
    }else{
        cocos2d::extension::AR::getInstance()->getQuat().set(m);
    }
}

- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors {
    for( ARAnchor* anchor in anchors ){
        if( cocos2d::extension::AR::getInstance()->onAddPlane ){
            if( [anchor isKindOfClass:[ARPlaneAnchor class]] ){
                ARPlaneAnchor* v = (ARPlaneAnchor*)anchor;
                const auto& c = v.transform.columns;
                const cocos2d::Mat4 m(c[0][0], c[1][0], c[2][0], c[3][0],
                                      c[0][1], c[1][1], c[2][1], c[3][1],
                                      c[0][2], c[1][2], c[2][2], c[3][2],
                                      c[0][3], c[1][3], c[2][3], c[3][3]);
                cocos2d::extension::AR::getInstance()->onAddPlane([[anchor.identifier UUIDString] UTF8String], m);
            }
        }
    }
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors {
    for( ARAnchor* anchor in anchors ){
        if( cocos2d::extension::AR::getInstance()->onUpdatePlane ){
            if( [anchor isKindOfClass:[ARPlaneAnchor class]] ){
                ARPlaneAnchor* v = (ARPlaneAnchor*)anchor;
                const auto& c = v.transform.columns;
                const cocos2d::Mat4 m(c[0][0], c[1][0], c[2][0], c[3][0],
                                      c[0][1], c[1][1], c[2][1], c[3][1],
                                      c[0][2], c[1][2], c[2][2], c[3][2],
                                      c[0][3], c[1][3], c[2][3], c[3][3]);
                cocos2d::extension::AR::getInstance()->onUpdatePlane([[anchor.identifier UUIDString] UTF8String], m);
            }
        }
    }
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors {
    for( ARAnchor* anchor in anchors ){
        if( cocos2d::extension::AR::getInstance()->onRemovePlane ){
            if( [anchor isKindOfClass:[ARPlaneAnchor class]] ){
                cocos2d::extension::AR::getInstance()->onRemovePlane([[anchor.identifier UUIDString] UTF8String]);
            }
        }
    }
}

@end

NS_CC_EXT_BEGIN

bool AR::isSupported() const {
    if( @available(ios 11, *) ){
        return [ARWorldTrackingConfiguration isSupported];
    }
    return false;
}

void AR::pause(){
    if( @available(ios 11, *) ){
        if( CCAR* internal = (CCAR*)_internal ){
            [internal pause];
        }
    }
}

void AR::resume(){
    if( @available(ios 11, *) ){
        if( CCAR* internal = (CCAR*)_internal ){
            [internal resume];
        }
    }
}

void AR::start(){
    if( @available(ios 11, *) ){
        if( !_internal ){
            _internal = [[CCAR alloc] init];
        }
    }
}

void AR::stop(){
    if( @available(ios 11, *) ){
        if( CCAR* internal = (CCAR*)_internal ){
            [internal release];
            _internal = nullptr;
            CC_SAFE_RELEASE_NULL(_renderTarget);
        }
    }
}

void AR::update(float delta){
}

NS_CC_EXT_END
