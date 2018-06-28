package com.jesse.soulout;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.jesse.soulout.util.VideoUtils;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        boolean result = VideoUtils.findHWDecoderByType("video/avc");
    }
}
