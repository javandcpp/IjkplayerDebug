package com.stone.media;

import android.util.Log;
import android.widget.Toast;

import tv.danmaku.ijk.media.player.annotations.CalledByNative;

public class VideoCompress {

    @CalledByNative
    public static void callbackFromNative(boolean isRunning){
        Log.d("callback",""+isRunning);
    }

    @CalledByNative
    public static void completeFromNative(String url){
        Log.d("callback",""+url);
    }

    public native boolean videoCompress(String srcUrl,int width,int height,String destUrl);
}
