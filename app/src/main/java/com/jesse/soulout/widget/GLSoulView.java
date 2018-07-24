package com.jesse.soulout.widget;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

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

public class GLSoulView extends GLSurfaceView implements GLSurfaceView.Renderer, ISurface {

    private static final String TAG = GLSoulView.class.getSimpleName();

    private GLPaint mGLPaint;
    // 视频数据
    private GLImage mImage;

    private Queue<byte[]> mVideoDataQueue;

    public GLSoulView(Context context) {
        this(context, null);
    }

    public GLSoulView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        setRenderer(this);
        setRenderMode(RENDERMODE_WHEN_DIRTY);
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
        Log.d(TAG, "onDrawFrame data len : " + data.length);
        mImage.putYUV420PData(data);
        if (!mImage.hasImage()) {
            return;
        }
        mGLPaint.clearSurface();
        mGLPaint.onDraw(mImage);
        mGLPaint.onDrawSoul(data);
    }

    @Override
    public void offer(byte[] data) {
        mVideoDataQueue.offer(data);
        Log.d(TAG, "queue size : " + mVideoDataQueue.size());
        requestRender();
    }

    @Override
    public byte[] poll() {
        Log.d(TAG, "poll queue size : " + mVideoDataQueue.size());
        return mVideoDataQueue.poll();
    }

    @Override
    public void setVideoParameters(int width, int height, int fps) {
        mImage.initSize(width, height);
        mGLPaint.setVideoParamters(width, height, fps);
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