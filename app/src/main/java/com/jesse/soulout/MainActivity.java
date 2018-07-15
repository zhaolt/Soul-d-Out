package com.jesse.soulout;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;

import com.jesse.soulout.codec.VideoCodec;
import com.jesse.soulout.widget.SoulView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private SoulView mSoulView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSoulView = (SoulView) findViewById(R.id.soul_view);
        VideoCodec videoCodec = new VideoCodec();
        videoCodec.setDisplay(mSoulView);
        String filePath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator
                + "DCIM/Camera/VID_20180715_212741.mp4";
        videoCodec.setDataSource(filePath);
        videoCodec.start();
    }
}
