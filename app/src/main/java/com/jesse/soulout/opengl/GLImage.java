package com.jesse.soulout.opengl;

import java.nio.ByteBuffer;

/**
 * Created by zhaoliangtai on 2018/6/25.
 */

public class GLImage {
    // yuv数据长度
    private int mYLen;
    private int mUVLen;
    private byte[] mYBytes;
    private byte[] mUVBytes;
    private ByteBuffer mY;
    private ByteBuffer mU;
    private ByteBuffer mV;
    private boolean hasImage;

    /**
     * 这里的数据只接收yuv420p 所以数据长度的计算方式也是按照yuv420p
     * @param width
     * @param height
     */
    public void initSize(int width, int height) {
        mYLen = width * height;
        mUVLen = width / 2 * height / 2;
        mYBytes = new byte[mYLen];
        mUVBytes = new byte[mUVLen];
        mY = ByteBuffer.allocate(mYLen);
        mU = ByteBuffer.allocate(mUVLen);
        mV = ByteBuffer.allocate(mUVLen);
    }

    public void putYUV420PData(byte[] data) {
        System.arraycopy(data, 0, mY.array(), 0, mYLen);
        System.arraycopy(data, mYLen, mU.array(), 0, mUVLen);
        System.arraycopy(data, mYLen + mUVLen, mV.array(), 0, mUVLen);
        update(mY.array(), mU.array(), mV.array());
    }

    private void update(byte[] yData, byte[] uData, byte[] vData) {
        synchronized (this) {
            clearCache();
            mY.put(yData, 0, yData.length);
            mU.put(uData, 0, uData.length);
            mV.put(vData, 0, vData.length);
        }
    }

    private void clearCache() {
        mY.clear();
        mU.clear();
        mV.clear();
    }

    public ByteBuffer getY() {
        return mY;
    }

    public ByteBuffer getU() {
        return mU;
    }

    public ByteBuffer getV() {
        return mV;
    }
}
