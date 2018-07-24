package com.jesse.soulout;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;

import com.jesse.soulout.codec.VideoCodec;
import com.jesse.soulout.widget.GLSoulView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();

    static {
        System.loadLibrary("movie_tools");
        System.loadLibrary("ffmpeg");
    }
    private GLSoulView mGLSoulView;
    private VideoCodec mVideoCodec;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mGLSoulView = (GLSoulView) findViewById(R.id.soul_view);
        mVideoCodec = new VideoCodec();
        mVideoCodec.setDisplay(mGLSoulView);
        String filePath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator
                + "DCIM/Camera/temp_uuid_2ca1950396b8219c788bc8c85bdb87fb.mp4";
        mVideoCodec.setDataSource(filePath);
        mVideoCodec.start();
    }
}
