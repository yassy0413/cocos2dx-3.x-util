/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceCamera.h"

#import <AVFoundation/AVFoundation.h>

@interface CCDeviceCamera : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@property (assign, nonatomic) cocos2d::experimental::FrameBuffer* frameBuffer;
@property (retain, nonatomic) AVCaptureSession* avcSession;
@end

@implementation CCDeviceCamera

-(id)init:(cocos2d::DeviceCameraSprite::CaptureDevicePosition)pos quality:(cocos2d::DeviceCameraSprite::Quality)quality {
    // 入力デバイスの作成
    if( pos == cocos2d::DeviceCameraSprite::CaptureDevicePosition::Default ){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        pos = cocos2d::DeviceCameraSprite::CaptureDevicePosition::Back;
#else
        pos = cocos2d::DeviceCameraSprite::CaptureDevicePosition::Front;
#endif
    }
    const AVCaptureDevicePosition avCaptureDevicePosition[] = {
        AVCaptureDevicePositionFront,
        AVCaptureDevicePositionBack,
    };
    const AVCaptureDevicePosition position = avCaptureDevicePosition[static_cast<int>(pos)];
    
    AVCaptureDevice* device = nil;
    NSArray* videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for( AVCaptureDevice* videoDevice in videoDevices ){
        if( videoDevice.position == position ){
            device = videoDevice;
            break;
        }
    }
    if( !device ){
        device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
    if( device ){
        [device setActiveVideoMinFrameDuration:CMTimeMake(1, 30)];
        
        AVCaptureDeviceInput* input = [AVCaptureDeviceInput deviceInputWithDevice:device error:nil];
        if( input != nil ){
            // 出力デバイスの作成
            AVCaptureVideoDataOutput* output = [[[AVCaptureVideoDataOutput alloc] init] autorelease];
            output.videoSettings = @{(id)kCVPixelBufferPixelFormatTypeKey:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]};
            output.alwaysDiscardsLateVideoFrames = YES;
            [output setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
            // セッションの作成
            NSString* presets[] = {
                AVCaptureSessionPresetLow,
                AVCaptureSessionPresetMedium,
                AVCaptureSessionPresetHigh
            };
            _avcSession = [[AVCaptureSession alloc] init];
            _avcSession.sessionPreset = presets[static_cast<int>(quality)];
            [_avcSession addInput:input];
            [_avcSession addOutput:output];
        }
    }
    
    return self;
}

- (void)dealloc {
    if( _avcSession ){
        [_avcSession stopRunning];
        while( _avcSession.running ){ usleep(1000); }
        [_avcSession release];
    }
    [super dealloc];
}

-(void) toggleCameraFace {
    //    [_avcSession beginConfiguration];
    //    [_avcSession removeInput:[_avcSession.inputs objectAtIndex:0]];
    //    [_avcSession addInput:[self findAVCaptureDeviceInput:cameraDeivcePosition]];
    //    [_avcSession commitConfiguration];
}

-(void)startCamera {
    if( _avcSession ){
        [_avcSession startRunning];
    }
}

-(void)stopCamera {
    if( _avcSession ){
        [_avcSession stopRunning];
    }
}

// AVCSessionから毎フレーム呼び出される
-(void)captureOutput:(AVCaptureOutput*)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection*)connection {
//    connection.videoOrientation = AVCaptureVideoOrientationPortrait;
    CVImageBufferRef buffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(buffer, 0);
    
    void* base = CVPixelBufferGetBaseAddress(buffer);
    const int width = (int)CVPixelBufferGetWidth(buffer);
    const int height = (int)CVPixelBufferGetHeight(buffer);
    
    // BGRA -> RGBA
    uint8_t* pixel = reinterpret_cast<uint8_t*>(base);
    for( int lp = width * height; lp > 0; --lp ){
        std::swap(pixel[0], pixel[2]);
        pixel += 4;
    }
    
    if( !_frameBuffer ){
        _frameBuffer = cocos2d::experimental::FrameBuffer::create(1, width, height);
        _frameBuffer->retain();
        _frameBuffer->attachRenderTarget( cocos2d::experimental::RenderTarget::create(width, height, cocos2d::Texture2D::PixelFormat::RGBA8888) );
        _frameBuffer->attachDepthStencilTarget( nullptr );
    }else{
        _frameBuffer->getRenderTarget()->getTexture()->updateWithData(base, 0, 0, width, height);
    }
    
    // イメージバッファのアンロック
    CVPixelBufferUnlockBaseAddress( buffer, 0 );
}

@end

NS_CC_BEGIN

DeviceCameraSprite* DeviceCameraSprite::create(CaptureDevicePosition pos, Quality quality){
    auto pRet = new (std::nothrow) DeviceCameraSprite();
    if( pRet && pRet->init(pos, quality) ){
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

DeviceCameraSprite::DeviceCameraSprite()
: _internal(nullptr)
{}

DeviceCameraSprite::~DeviceCameraSprite(){
    if( CCDeviceCamera* internal = (CCDeviceCamera*)_internal ){
        [internal release];
    }
}

bool DeviceCameraSprite::init(CaptureDevicePosition pos, Quality quality){
    if( Sprite::init() ){
        _internal = [[CCDeviceCamera alloc] init:pos quality:quality];
        return true;
    }
    return false;
}

void DeviceCameraSprite::start(){
    if( CCDeviceCamera* internal = (CCDeviceCamera*)_internal ){
        [internal startCamera];
    }
}

void DeviceCameraSprite::stop(){
    if( CCDeviceCamera* internal = (CCDeviceCamera*)_internal ){
        [internal stopCamera];
    }
}

void DeviceCameraSprite::onEnter(){
    Sprite::onEnter();
    scheduleUpdate();
}

void DeviceCameraSprite::onExit(){
    Sprite::onExit();
    unscheduleUpdate();
}

void DeviceCameraSprite::update(float delta){
    if( CCDeviceCamera* internal = (CCDeviceCamera*)_internal ){
        if( getContentSize().equals(cocos2d::Size::ZERO) && internal.frameBuffer ){
            initWithTexture(internal.frameBuffer->getRenderTarget()->getTexture());
            setScale(cocos2d::Director::getInstance()->getWinSize().height / getContentSize().height);
            setRotation(90);
            setBlendFunc(cocos2d::BlendFunc::DISABLE);
            setPosition(cocos2d::Director::getInstance()->getWinSize() * 0.5f);
        }
    }
}

NS_CC_END
