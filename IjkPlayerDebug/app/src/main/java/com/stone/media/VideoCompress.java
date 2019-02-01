package com.stone.media;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import java.lang.ref.WeakReference;

import tv.danmaku.ijk.media.player.annotations.CalledByNative;

public class VideoCompress {


    private CompressListener mCompressListener;

    private static final int PROCESS_COMPLETE = 0;
    private static final int PROCESS_CANCEL = 1;
    private static final int PROCESS_RUNNING = 2;
    private static final int PROCESS_PROGESS = 3;
    private static EventHandler mEventHandler;

    public VideoCompress() {
        mEventHandler = new EventHandler(this);
    }

    @CalledByNative
    public static void callbackFromNative(boolean isRunning) {
        Log.d("callback", "" + isRunning);
        if (null != mEventHandler) {
            Message message = mEventHandler.obtainMessage();
            message.what = PROCESS_RUNNING;
            message.obj = isRunning;
            mEventHandler.sendMessage(message);
        }
    }

    @CalledByNative
    public static void completeFromNative(String url) {
        Log.d("callback", "" + url);
        if (null != mEventHandler) {
            Message message = mEventHandler.obtainMessage();
            message.what = PROCESS_COMPLETE;
            message.obj = url;
            mEventHandler.sendMessage(message);
        }
    }

    @CalledByNative
    public static void progressFromNative(int progress,int currentMills) {
        if (null != mEventHandler) {
            Message message = mEventHandler.obtainMessage();
            message.what = PROCESS_PROGESS;
            message.obj = progress;
            mEventHandler.sendMessage(message);
        }
    }


    public void videoCompress(String srcUrl,String destUrl, int width, int height,  CompressListener compressListener) {
        this.mCompressListener = compressListener;
        videoCompress(srcUrl, width, height, destUrl);
    }

    public void stopCompress(){
        stop();
    }


    public void setCompressListener(CompressListener compressListener) {
        mCompressListener = compressListener;
    }

    public interface CompressListener {

        void complete(String url);

        void isRunning(boolean isRunning);

        void progress(int progress);

    }


    private static class EventHandler extends Handler {
        private final WeakReference<VideoCompress> weakReference;
        private VideoCompress mVideoCompress;

        public EventHandler(VideoCompress videoCompress) {
            weakReference = new WeakReference<VideoCompress>(videoCompress);
            mVideoCompress = weakReference.get();
        }

        @Override
        public void handleMessage(Message msg) {
            if (null != mVideoCompress && null != mVideoCompress.mCompressListener)
                switch (msg.what) {
                    case PROCESS_COMPLETE:
                        String result = (String) msg.obj;
                        mVideoCompress.mCompressListener.complete(result);
                        break;
                    case PROCESS_CANCEL:
                        break;
                    case PROCESS_RUNNING:
                        boolean running = (boolean) msg.obj;
                        mVideoCompress.mCompressListener.isRunning(running);
                        break;
                    case PROCESS_PROGESS:
                        int progress = (int) msg.obj;
                        mVideoCompress.mCompressListener.progress(progress);
                        break;
                }
        }
    }



     private native boolean videoCompress(String srcUrl, int width, int height, String destUrl);

     private native void stop();


}

