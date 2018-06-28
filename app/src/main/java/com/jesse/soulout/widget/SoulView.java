package com.jesse.soulout.widget;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.jesse.soulout.opengl.GLImage;
import com.jesse.soulout.opengl.GLPaint;
import com.jesse.soulout.opengl.ISurface;

import java.util.ArrayDeque;
import java.util.Queue;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by zhaoliangtai on 2018/6/25.
 */

public class SoulView extends GLSurfaceView implements GLSurfaceView.Renderer, ISurface {

    private static final String TAG = SoulView.class.getSimpleName();

    private GLPaint mGLPaint;
    // 视频数据
    private GLImage mImage;

    private Queue<byte[]> mVideoDataQueue;

    public SoulView(Context context) {
        this(context, null);
    }

    public SoulView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        setRenderer(this);
        // 连续模式 大概没16ms刷新一次
        setRenderMode(RENDERMODE_CONTINUOUSLY);
        mVideoDataQueue = new ArrayDeque<>();
        mGLPaint = new GLPaint();
        mImage = new GLImage();
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mGLPaint.onSurfaceCreated(getContext());
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mGLPaint.onSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        byte[] data = poll();
        if (null == data) {
            return;
        }

    }

    @Override
    public void offer(byte[] data) {

    }

    @Override
    public byte[] poll() {
        return new byte[0];
    }

    @Override
    public void setVideoParameters(int width, int height, int fps) {

    }

    @Override
    public void start() {

    }

    @Override
    public void stop() {

    }

    @Override
    public void tryStop() {

    }
}
