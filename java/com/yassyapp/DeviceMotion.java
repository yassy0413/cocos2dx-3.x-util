/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
package com.yassyapp;

import org.cocos2dx.lib.Cocos2dxActivity;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

/**
 *
 */
public class DeviceMotion implements SensorEventListener {

    private static DeviceMotion mInstance = null;

    private SensorManager mSensorManager = null;
    private final float[] mRotationQuat= new float[4];

    public native void updateRotationQuat(float[] quat);

    public static void start(){
        if( mInstance == null ){
            mInstance = new DeviceMotion();
            mInstance.mSensorManager = (SensorManager)Cocos2dxActivity.getContext().getSystemService(Context.SENSOR_SERVICE);
            resume();
        }
    }

    public static void stop(){
        if( mInstance != null ){
            pause();
            mInstance = null;
        }
    }

    public static void resume(){
        if( mInstance != null ) {
            Sensor sensor = mInstance.mSensorManager.getDefaultSensor(Sensor.TYPE_GAME_ROTATION_VECTOR);
            mInstance.mSensorManager.registerListener(mInstance, sensor, SensorManager.SENSOR_DELAY_GAME);
        }
    }

    static public void pause(){
        if( mInstance != null ) {
            mInstance.mSensorManager.unregisterListener(mInstance);
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy){
    }

    @Override
    public void onSensorChanged(SensorEvent event){
        if (event.sensor.getType() == Sensor.TYPE_GAME_ROTATION_VECTOR) {
            mSensorManager.getQuaternionFromVector(mRotationQuat, event.values);
            updateRotationQuat(mRotationQuat);
        }
    }

}
