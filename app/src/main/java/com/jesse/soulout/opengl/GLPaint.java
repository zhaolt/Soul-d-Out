package com.jesse.soulout.opengl;

import android.content.Context;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.util.Log;

import com.jesse.soulout.util.AssetsUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Created by zhaoliangtai on 2018/6/25.
 */

public class GLPaint {

    private static final String TAG = GLPaint.class.getSimpleName();

    private static final int FLOAT_SIZE = 4;

    private static final float[] VERTEX = {
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f
    };

    private static final float[] TEXTURE_COORDS = {
            0f, 1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private float[] mMvpMatrix = new float[16];
    private float[] mRotateMatrix = new float[16];


    // y、u、v三个纹理id
    private int[] mTextures;

    // 视频分辨率
    private int mWidth;
    private int mHeight;
    // 视频帧率
    private int mFps;

    // 着色器句柄
    private int mProgram;
    private int mPositionLoc;
    private int mTexCoordsLoc;
    private int mMatrixLoc;
    private int mYSampler;
    private int mUSampler;
    private int mVSampler;
    private int mAlphaLoc;

    private int mInterval;

    // 灵魂(虚影)数据
    private GLImage mSoulImg;


    private FloatBuffer mVertexBuffer;
    private FloatBuffer mCoordBuffer;


    public GLPaint() {
        mVertexBuffer = ByteBuffer.allocateDirect(VERTEX.length * FLOAT_SIZE)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(VERTEX);
        mVertexBuffer.position(0);
        mCoordBuffer = ByteBuffer.allocateDirect(TEXTURE_COORDS.length * FLOAT_SIZE)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(TEXTURE_COORDS);
        mCoordBuffer.position(0);
        mSoulImg = new GLImage();
    }

    public void setVideoParamters(int width, int height, int fps) {
        mWidth = width;
        mHeight = height;
        mFps = fps;
        mSoulImg.initSize(width, height);
    }


    public void onSurfaceCreated(Context context) {
        String vertexPath = "vertex.glsl";
        String fragmentPath = "fragment.glsl";
        mProgram = createProgram(
                AssetsUtils.getAssetsContents(context, vertexPath),
                AssetsUtils.getAssetsContents(context, fragmentPath));
        // 找到着色器变量句柄
        mPositionLoc = GLES20.glGetAttribLocation(mProgram, "vPosition");
        checkGlError("glGetAttribLocation vPosition");
        mTexCoordsLoc = GLES20.glGetAttribLocation(mProgram, "vCoord");
        checkGlError("glGetAttribLocation vCoord");
        mMatrixLoc = GLES20.glGetUniformLocation(mProgram, "vMatrix");
        checkGlError("glGetUniformLocation vMatrix");
        mYSampler = GLES20.glGetUniformLocation(mProgram, "sampler_y");
        checkGlError("glGetUniformLocation sampler_y");
        mUSampler = GLES20.glGetUniformLocation(mProgram, "sampler_u");
        checkGlError("glGetUniformLocation sampler_u");
        mVSampler = GLES20.glGetUniformLocation(mProgram, "sampler_v");
        checkGlError("glGetUniformLocation sampler_v");
        mAlphaLoc = GLES20.glGetUniformLocation(mProgram, "alpha");
        checkGlError("glGetUniformLocation alpha");
        GLES20.glUseProgram(mProgram);
        checkGlError("glUseProgram");
        GLES20.glVertexAttribPointer(mPositionLoc, 2, GLES20.GL_FLOAT, false,
                2 * FLOAT_SIZE, mVertexBuffer);
        checkGlError("glVertexAttribPointer vPosition");
        GLES20.glEnableVertexAttribArray(mPositionLoc);
        checkGlError("glEnableVertexAttribArray vPosition");
        GLES20.glVertexAttribPointer(mTexCoordsLoc, 2, GLES20.GL_FLOAT, false,
                2 * FLOAT_SIZE, mCoordBuffer);
        checkGlError("glVertexAttribPointer vCoord");
        GLES20.glEnableVertexAttribArray(mTexCoordsLoc);
        checkGlError("glEnableVertexAttribArray vCoord");
        mTextures = createTextures();
    }

    public void onSurfaceChanged(int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    public void clearSurface() {
        GLES20.glClearColor(0, 0, 0, 0);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        Matrix.setIdentityM(mMvpMatrix, 0);
//        Matrix.rotateM(mMvpMatrix, 0, 270, 0, 0, 1);
        GLES20.glUniformMatrix4fv(mMatrixLoc, 1, false, mMvpMatrix, 0);
        checkGlError("glUniformMatrix4fv matrix");
        GLES20.glUniform1f(mAlphaLoc, 1);
        checkGlError("glUniform1f");
    }

    /**
     * 绘制视频
     * 激活前三个纹理单元分别绑定yuv数据
     * @param img
     */
    public void onDraw(GLImage img) {
        Log.d(TAG, "clearSurface");
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        checkGlError("glActiveTexture 0");
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextures[0]);
        checkGlError("glBindTexture 0");
        img.getY().position(0);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, mWidth, mHeight, 0,
                GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, img.getY());
        checkGlError("glTexImage2D y");
        GLES20.glUniform1i(mYSampler, 0);
        checkGlError("glUniform1i y");
        Log.d(TAG, "onDraw update y sample");
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextures[1]);
        img.getU().position(0);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, mWidth / 2, mHeight / 2,
                0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, img.getU());
        GLES20.glUniform1i(mUSampler, 1);
        checkGlError("glUniform1i u");
        Log.d(TAG, "onDraw update u sample");
        GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextures[2]);
        img.getV().position(0);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, mWidth / 2, mHeight / 2,
                0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, img.getV());
        GLES20.glUniform1i(mVSampler, 2);
        checkGlError("glUniform1i v");
        Log.d(TAG, "onDraw update v sample");
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }

    public void onDrawSoul(byte[] data) {
        mInterval++;
        if (!mSoulImg.hasImage() || mInterval == mFps + 1) {
            mInterval = 1;
            mSoulImg.putYUV420PData(data);
        }
        if (!mSoulImg.hasImage()) {
            return;
        }
        // 开启混合模式 将灵魂混合到肉体上
        GLES20.glEnable(GLES20.GL_BLEND);
        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
        // 缩放 矩阵
        Matrix.setIdentityM(mMvpMatrix, 0);
//        Matrix.rotateM(mMvpMatrix, 0, 270, 0, 0, 1);
        float scale = 1.0f + mInterval / (mFps * 2f);
        Matrix.scaleM(mMvpMatrix, 0, scale, scale, 0);
        GLES20.glUniformMatrix4fv(mMatrixLoc, 1, false, mMvpMatrix, 0);
        GLES20.glUniform1f(mAlphaLoc, 0.2f + (mFps - mInterval) / 100f);
        onDraw(mSoulImg);
    }


    private int createProgram(String vertexSource, String fragmentSource) {
        // 加载顶点着色器代码
        int vertexShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        GLES20.glShaderSource(vertexShader, vertexSource);
        GLES20.glCompileShader(vertexShader);
        // 加载片元着色器代码
        int fragmentShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(fragmentShader, fragmentSource);
        GLES20.glCompileShader(fragmentShader);
        // 创建程序
        int program = GLES20.glCreateProgram();
        checkGlError("glCreateProgram");
        GLES20.glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        GLES20.glAttachShader(program, fragmentShader);
        checkGlError("glAttachShader");
        GLES20.glLinkProgram(program);
        // 检查状态
        int[] linkStatus = new int[1];
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
        if (linkStatus[0] != GLES20.GL_TRUE) {
            Log.e(TAG, "Could not link program: ");
            Log.e(TAG, GLES20.glGetProgramInfoLog(program));
            GLES20.glDeleteProgram(program);
            program = 0;
        }
        return program;
    }

    private int[] createTextures() {
        int[] textures = new int[3];
        GLES20.glGenTextures(3, textures, 0);
        for (int i = 0; i < 3; i++) {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[i]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_NEAREST);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        }
        return textures;
    }


    private int createTextureOES() {
        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);
        int texId = textures[0];
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texId);
        checkGlError("glBindTexture " + texId);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);
        return texId;
    }

    public void checkGlError(String op) {
        int error = GLES20.glGetError();
        if (error != GLES20.GL_NO_ERROR) {
            String msg = op + ": glError 0x" + Integer.toHexString(error);
            Log.e(TAG, msg);
            throw new RuntimeException(msg);
        }
    }

}
