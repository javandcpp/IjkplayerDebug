package com.stone.media;

public final class MediaProcess {


    private MediaProcess() {
        loadNativelibrary();
    }

    private void loadNativelibrary() {
        System.loadLibrary("coremedia");
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


    public void VideoCompress(String url, int width, int height) {

        new VideoCompress().videoCompress(url, width, height);
    }


}
