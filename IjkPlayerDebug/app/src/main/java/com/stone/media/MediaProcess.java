package com.stone.media;

import android.util.Log;

public final class MediaProcess {


    private MediaProcess() {

    }



    private static MediaProcess mInstance = null;

    public static MediaProcess getMediaProcess() {
        if (null == mInstance) {
            synchronized (MediaProcess.class) {
                if (null == mInstance) {
                    return (mInstance = new MediaProcess());
                }
            }
        }
        return mInstance;
    }


    public void VideoCompress(String url, String destPath,int width, int height) {
        final long start=System.currentTimeMillis();
        VideoCompress videoCompress = new VideoCompress();
        videoCompress.setCompressListener(new VideoCompress.CompressListener() {
            @Override
            public void complete() {
                long end=System.currentTimeMillis();
                Log.e("duration","duration:"+((end-start)/1000));
            }

            @Override
            public void isRunning() {

            }
        });
        videoCompress.videoCompress(url, width, height,destPath);
    }


}
