/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
package com.yassyapp;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import android.annotation.TargetApi;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.pm.PackageInfo;
import android.os.Build;
import android.os.StatFs;

/**
 * 端末操作
 */
public class Device {

    public static long getBundleVersion(){
        try {
            PackageInfo pi = Cocos2dxActivity.getContext().getPackageManager().getPackageInfo(Cocos2dxActivity.getContext().getPackageName(), 0);
            return pi.versionCode;
        } catch(Exception e) {
            return 0;
        }
    }

    public static String getBundleShortVersion(){
        try {
            PackageInfo pi = Cocos2dxActivity.getContext().getPackageManager().getPackageInfo(Cocos2dxActivity.getContext().getPackageName(), 0);
            return pi.versionName;
        } catch(Exception e) {
            return "";
        }
    }

    public static String getSystemVersion(){
        return Build.VERSION.RELEASE;
    }

    public static String getSystemName(){
        return Build.MODEL;
    }

    public static boolean isPortrait(){
        Configuration conf = Cocos2dxActivity.getContext().getResources().getConfiguration();
        return conf.orientation == Configuration.ORIENTATION_PORTRAIT;
    }

    public static long getDiskFreeByates(){
        if( Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2 ){
            StatFs statFs = new StatFs(Cocos2dxHelper.getCocos2dxWritablePath());
            return statFs.getAvailableBlocks() * statFs.getBlockSize();
        }
        return getDiskFreeByatesLong();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
    public static long getDiskFreeByatesLong() {
        StatFs statFs = new StatFs(Cocos2dxHelper.getCocos2dxWritablePath());
        return statFs.getAvailableBlocksLong() * statFs.getBlockSizeLong();
    }

    public static void clearUserDefaults(){
        SharedPreferences settings = Cocos2dxHelper.getActivity().getSharedPreferences("Cocos2dxPrefsFile", 0);
        SharedPreferences.Editor editor = settings.edit();
        editor.clear();
        editor.apply();
    }
}
