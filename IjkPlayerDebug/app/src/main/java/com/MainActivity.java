package com;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.View;

import com.stone.media.MediaProcess;

import tv.danmaku.ijk.media.example.R;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

    }



    public void compress(View view){
        MediaProcess.getMediaProcess().VideoCompress("/mnt/sdcard/video.mp4",120,20);
    }
}
