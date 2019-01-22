package com.stone.media;

import android.util.Log;
import android.widget.Toast;

import tv.danmaku.ijk.media.player.annotations.CalledByNative;

public class VideoCompress {


    private static CompressListener mCompressListener;

    @CalledByNative
    public static void callbackFromNative(boolean isRunning){
        Log.d("callback",""+isRunning);
    }

    @CalledByNative
    public static void completeFromNative(String url){
        Log.d("callback",""+url);
        if(null!=mCompressListener){
            mCompressListener.complete();
        }
    }

    public native boolean videoCompress(String srcUrl,int width,int height,String destUrl);

    public void setCompressListener(CompressListener compressListener){
        mCompressListener=compressListener;
    }

    public interface CompressListener{

        void complete();
        void isRunning();
    }
}
