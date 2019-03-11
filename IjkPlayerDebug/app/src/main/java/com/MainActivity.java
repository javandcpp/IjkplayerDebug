package com;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.stone.media.MediaProcess;
import com.stone.media.VideoCompress;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import tv.danmaku.ijk.media.example.R;


public class MainActivity extends Activity {


    private TextView tvProgress;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String[] permissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE};
        Log.d("hardcodec:", isSupportMediaCodecHardDecoder() + "");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            int i = ContextCompat.checkSelfPermission(this, permissions[0]);
            // 权限是否已经 授权 GRANTED---授权  DINIED---拒绝
            if (i != PackageManager.PERMISSION_GRANTED) {
                // 如果没有授予该权限，就去提示用户请求
            }
        }
        tvProgress = findViewById(R.id.progress);
    }


    public void compress(View view) {

        MediaProcess.getMediaProcess().getVideoCompress().videoCompress("/mnt/sdcard/test4.mp4", "/mnt/sdcard/output1.mp4", 568, 320, new VideoCompress.CompressListener() {
            @Override
            public void complete(String url) {
                Toast.makeText(getApplicationContext(),url,Toast.LENGTH_SHORT).show();
            }

            @Override
            public void isRunning(boolean isRunning) {
                Toast.makeText(getApplicationContext(),isRunning+"",Toast.LENGTH_SHORT).show();
            }

            @Override
            public void progress(int progress) {
                tvProgress.setText(progress+"%");
            }
        });
    }

    public boolean isSupportMediaCodecHardDecoder() {
        boolean isHardcode = false;
        //读取系统配置文件/system/etc/media_codecc.xml
        File file = new File("/system/etc/media_codecs.xml");
        InputStream inFile = null;
        try {
            inFile = new FileInputStream(file);
        } catch (Exception e) {
            // TODO: handle exception
        }

        if (inFile != null) {
            XmlPullParserFactory pullFactory;
            try {
                pullFactory = XmlPullParserFactory.newInstance();
                XmlPullParser xmlPullParser = pullFactory.newPullParser();
                xmlPullParser.setInput(inFile, "UTF-8");
                int eventType = xmlPullParser.getEventType();
                while (eventType != XmlPullParser.END_DOCUMENT) {
                    String tagName = xmlPullParser.getName();
                    switch (eventType) {
                        case XmlPullParser.START_TAG:
                            if ("MediaCodec".equals(tagName)) {
                                String componentName = xmlPullParser.getAttributeValue(0);

                                if (componentName.startsWith("OMX.")) {
                                    if (!componentName.startsWith("OMX.google.")) {
                                        isHardcode = true;
                                    }
                                }
                            }
                    }
                    eventType = xmlPullParser.next();
                }
            } catch (Exception e) {
                // TODO: handle exception
            }
        }
        return isHardcode;
    }

    public void stop(View view) {
        MediaProcess.getMediaProcess().getVideoCompress().stopCompress();
    }

    public void compress2(View view) {

        MediaProcess.getMediaProcess().getVideoCompress().videoCompress("/mnt/sdcard/test.mp4", "/mnt/sdcard/output2.mp4", 568, 320, new VideoCompress.CompressListener() {
            @Override
            public void complete(String url) {
                Toast.makeText(getApplicationContext(),url,Toast.LENGTH_SHORT).show();
            }

            @Override
            public void isRunning(boolean isRunning) {
                Toast.makeText(getApplicationContext(),isRunning+"",Toast.LENGTH_SHORT).show();
            }

            @Override
            public void progress(int progress) {
                tvProgress.setText(progress+"%");
            }
        });
    }
}
