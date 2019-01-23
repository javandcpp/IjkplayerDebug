package com;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.View;

import com.stone.media.MediaProcess;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import tv.danmaku.ijk.media.example.R;


public class MainActivity extends Activity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.d("hardcodec:",isSupportMediaCodecHardDecoder()+"");
    }



    public void compress(View view){

        MediaProcess.getMediaProcess().VideoCompress("/mnt/sdcard/test.mp4","/mnt/sdcard/output1.mp4",568,320);
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
}
