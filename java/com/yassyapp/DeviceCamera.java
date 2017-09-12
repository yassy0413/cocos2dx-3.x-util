/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
package com.yassyapp;

import org.cocos2dx.lib.Cocos2dxActivity;
import java.util.List;

import android.content.Context;
import android.hardware.Camera;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.util.Log;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.ViewGroup;

/**
 * http://techblog.kayac.com/android_slit_scan.html
 */
public class DeviceCamera implements Camera.PreviewCallback, Callback {

    private static DeviceCamera mInstance = null;

    private Camera mCamera = null;
    private byte[] mFrameBuffer = null;
    private boolean mFacingBack;
    private SurfaceView mSurfaceView = null;

    public native void updateFrameBuffer(byte[] bytes, int width, int height);

    private static boolean checkCameraHardware() {
        return Cocos2dxActivity.getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA);
    }

    public static void start(int width, int height, boolean back) {
        if (mInstance != null) {
            return;
        }
        mInstance = new DeviceCamera();
        mInstance.mFacingBack = back;

        Cocos2dxActivity activity = (Cocos2dxActivity)Cocos2dxActivity.getContext();
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // create surface view
                Cocos2dxActivity activity = (Cocos2dxActivity)Cocos2dxActivity.getContext();
                mInstance.mSurfaceView = new SurfaceView(activity);
                mInstance.mSurfaceView.getHolder().addCallback(mInstance);
                activity.getFrameLayout().addView(mInstance.mSurfaceView);
            }
        });
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int f, int w, int h) {
        Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
        int facing = mFacingBack? Camera.CameraInfo.CAMERA_FACING_BACK : Camera.CameraInfo.CAMERA_FACING_FRONT;
        int numberOfCameras = Camera.getNumberOfCameras();
        for( int lp = 0; lp < numberOfCameras; ++lp ){
            Camera.getCameraInfo(lp, cameraInfo);
            if( cameraInfo.facing == facing ){
                mCamera = Camera.open(lp);
                if( mCamera == null ){
                    return;
                }
                break;
            }
        }

        if( mCamera != null ){
            Camera.Parameters parameters = mCamera.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);
            List<Camera.Size> sizes = parameters.getSupportedPreviewSizes();
            for( int llp = 0; llp < sizes.size(); ++llp ) {
                Camera.Size optimalSize = sizes.get(llp);
                Log.d("yassyapp", "***** CameraSize! " + optimalSize.width + "," + optimalSize.height);
            }
            Camera.Size optimalSize = sizes.get(sizes.size()/2);
            int width = optimalSize.width;
            int height = optimalSize.height;
            parameters.setPreviewSize(width, height);
            mCamera.setParameters(parameters);

            int size = width * height * ImageFormat.getBitsPerPixel(parameters.getPreviewFormat()) / 8;
            mFrameBuffer = new byte[size];
            try {
                mCamera.setPreviewDisplay(mSurfaceView.getHolder());
                mCamera.addCallbackBuffer(mFrameBuffer);
                mCamera.setPreviewCallbackWithBuffer(mInstance);
                mCamera.startPreview();
            } catch(Exception e){
                e.printStackTrace();
            }
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }

    public static void stop(){
        if( mInstance == null || mInstance.mCamera == null ){
            return;
        }
        mInstance.mCamera.setPreviewCallbackWithBuffer(null);
        mInstance.mCamera.stopPreview();
        mInstance.mCamera.release();
        mInstance.mCamera = null;
        mInstance.mFrameBuffer = null;
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera){
        Camera.Size size = mCamera.getParameters().getPreviewSize();
        updateFrameBuffer(data, size.width, size.height);
        camera.addCallbackBuffer(mFrameBuffer);
    }

}
