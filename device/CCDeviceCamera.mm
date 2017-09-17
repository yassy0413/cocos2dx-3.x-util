/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDeviceCamera.h"

#import <AVFoundation/AVFoundation.h>

@interface CCDeviceCamera : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@property (assign, nonatomic) cocos2d::extension::DeviceCamera* instance;
@property (retain, nonatomic) AVCaptureSession* avcSession;
@end

@implementation CCDeviceCamera

-(id)init:(cocos2d::extension::DeviceCamera::CaptureDevicePosition)pos
  quality:(cocos2d::extension::DeviceCamera::Quality)quality
   target:(cocos2d::extension::DeviceCamera*)target
{
    _instance = target;
    
    // 入力デバイスの作成
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
            // 開始
            [_avcSession startRunning];
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

// AVCSessionから毎フレーム呼び出される
-(void)captureOutput:(AVCaptureOutput*)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection*)connection {
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
    
    _instance->applyImage(base, width, height);
    
    CVPixelBufferUnlockBaseAddress(buffer, 0);
}

@end

NS_CC_EXT_BEGIN

void DeviceCamera::start(CaptureDevicePosition pos, Quality quality){
    if( !_internal ){
        setPosition(pos);
        cocos2d::Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
        _internal = [[CCDeviceCamera alloc] init:_position quality:quality target:this];
    }
}

void DeviceCamera::stop(){
    if( CCDeviceCamera* internal = (CCDeviceCamera*)_internal ){
        [internal release];
        _internal = nullptr;
        cocos2d::Director::getInstance()->getScheduler()->unscheduleUpdate(this);
    }
}

void DeviceCamera::update(float delta){
}

NS_CC_EXT_END
