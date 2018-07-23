package com.jesse.soulout;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;

import com.jesse.soulout.codec.VideoCodec;
import com.jesse.soulout.widget.SoulView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("movie_tools");
        System.loadLibrary("ffmpeg");
    }
    private SoulView mSoulView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSoulView = (SoulView) findViewById(R.id.soul_view);
        final VideoCodec videoCodec = new VideoCodec();
        videoCodec.setDisplay(mSoulView);
        String filePath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator
                + "DCIM/Camera/temp_uuid_2ca1950396b8219c788bc8c85bdb87fb.mp4";
        videoCodec.setDataSource(filePath);
        new Thread(new Runnable() {
            @Override
            public void run() {
                videoCodec.start();
            }
        }).start();
    }
}
